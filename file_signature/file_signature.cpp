#include <file_signature/file_signature.h>
#include <file_signature/file_signature_impl.h>

#include <boost/crc.hpp>
#include <fstream>
#include <future>
#include <ios>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace file_signature {

void generate(std::string input_file, std::string signature_file,
              int block_size) {
  generator g{input_file, signature_file, block_size};
  g.run();
}

//
// generator
//

generator::generator(std::string input_file, std::string signature_file,
                     int block_size)
    : input_file{input_file},
      signature_file{signature_file},
      block_size{block_size} {}

void generator::run() {
  try {
    writer_impl w{signature_file};
    hash_calc_impl h{w};
    reader r{input_file, block_size, h};

    auto reader_result = std::async(std::launch::async, [&]() { r.run(); });
    auto hash_calc_result = std::async(std::launch::async, [&]() { h.run(); });
    auto writer_result = std::async(std::launch::async, [&]() { w.run(); });

    reader_result.wait();
    hash_calc_result.wait();
    writer_result.wait();

    reader_result.get();
    hash_calc_result.get();
    writer_result.get();
  } catch (std::exception& e) {
    std::throw_with_nested(error("generate error: " + input_file));
  }
}

//
// reader
//

reader::reader(const std::string& input_file, int block_size, hash_calc& calc)
    : input_file{input_file}, block_size{block_size}, calc{calc} {}

void reader::run() {
  try {
    std::ifstream s;

    try {
      s.exceptions(std::ifstream::badbit | std::ios_base::failbit);
      s.open(input_file, std::ios::binary | std::ios::in);
    } catch (std::exception& e) {
      std::throw_with_nested(error("Couldn't open " + input_file));
    }

    try {
      s.exceptions(std::ifstream::badbit);

      while (s) {
        std::vector<char> buffer(block_size, 0);
        s.read(&buffer[0], buffer.size());

        if (s.eof()) {
          buffer.resize(s.gcount());
        }

        if (!buffer.empty()) {
          calc.on_read_block(std::move(buffer));
        }
      }
    } catch (std::exception& e) {
      std::throw_with_nested(
          error("Couldn't open " + input_file + ": " + e.what()));
    }
  } catch (std::exception& e) {
    calc.on_pipeline_failure();
    std::throw_with_nested(error(e.what()));
  }

  calc.on_finishing_reader();
}

//
// block_hash_calc_impl
//

hash_calc_impl::hash_calc_impl(writer& w)
    : writer_{w},
      reader_finished{false},
      failed{false},
      pipeline_failed{false} {}

void hash_calc_impl::on_read_block(file_block b) {
  std::unique_lock lk{mt};
  if (failed || pipeline_failed) {
    return;
  }

  blocks.push(std::move(b));
  lk.unlock();
  cv.notify_one();
}

void hash_calc_impl::on_pipeline_failure() {
  std::unique_lock lk{mt};
  pipeline_failed = true;
  lk.unlock();
  cv.notify_one();

  writer_.on_pipeline_failure();
}

void hash_calc_impl::on_finishing_reader() {
  std::unique_lock lk{mt};
  reader_finished = true;
  lk.unlock();
  cv.notify_one();
}

void hash_calc_impl::run() {
  try {
    file_block b;

    while (true) {
      std::unique_lock lk{mt};

      if (pipeline_failed) {
        break;
      }

      if (blocks.empty()) {
        if (reader_finished) {
          break;
        }

        cv.wait(lk);
        continue;
      }

      b = std::move(blocks.front());
      blocks.pop();

      lk.unlock();

      boost::crc_32_type result;
      result.process_bytes(b.data(), b.size());
      writer_.on_calc_block_hash(result.checksum());
    }
  } catch (const std::exception& e) {
    writer_.on_pipeline_failure();
    std::unique_lock lk{mt};
    failed = true;
    lk.unlock();

    std::throw_with_nested(error(e.what()));
  }

  writer_.on_finishing_hash_calc();
}

//
// writer_impl
//
writer_impl::writer_impl(const std::string& output_file)
    : output_file{output_file},
      hash_calc_finished{false},
      pipeline_failed{false} {}

void writer_impl::on_calc_block_hash(int hash) {
  std::unique_lock lk{mt};
  if (pipeline_failed) {
    return;
  }

  crc.push_back(hash);
  lk.unlock();
  cv.notify_one();
}

void writer_impl::on_finishing_hash_calc() {
  std::unique_lock lk{mt};
  hash_calc_finished = true;
  lk.unlock();
  cv.notify_one();
}

void writer_impl::on_pipeline_failure() {
  std::unique_lock lk{mt};
  pipeline_failed = true;
  lk.unlock();
  cv.notify_one();
}

void writer_impl::run() {
  try {
    while (true) {
      std::unique_lock lk{mt};
      if (pipeline_failed) {
        return;
      }

      if (!hash_calc_finished) {
        cv.wait(lk);
        continue;
      }
      break;
    }

    std::unique_lock lk{mt};

    std::ofstream s;

    try {
      s.exceptions(std::ifstream::badbit | std::ios_base::failbit);
      s.open(output_file, std::ios::trunc | std::ios::out);
    } catch (std::exception& e) {
      std::throw_with_nested(error("Couldn't open " + output_file));
    }

    for (auto i = crc.begin(); i != crc.end(); i++) {
      s << *i << '\n';
    }
    s.close();

    return;
  } catch (const std::exception& e) {
    std::throw_with_nested(error(e.what()));
  }
}

//
// error
//

error::error(const std::string& s) : std::runtime_error(s) {}

}  // namespace file_signature
