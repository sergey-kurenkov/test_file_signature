#ifndef FILE_SIGNATURE_FILE_SIGNATURE_TEST_H_
#define FILE_SIGNATURE_FILE_SIGNATURE_TEST_H_

#include <file_signature/file_signature.h>
#include <file_signature/file_signature_impl.h>

#include <filesystem>
#include <fstream>
#include <ios>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

namespace file_signature {

const char default_input_file[] = "test.txt";
const char default_output_file[] = "test.signature";

void create_file_for_reader(std::string name, int size, char ch);

void delete_file_for_reader(std::string name);

std::vector<std::string> read_file(std::string name);

struct hash_mock : public hash_calc {
  std::mutex mt;
  std::vector<file_signature::file_block> blocks;
  bool finished = false;
  bool pipeline_failure = false;

  void on_read_block(file_signature::file_block fb) override {
    std::lock_guard<std::mutex> lk{mt};
    blocks.push_back(std::move(fb));
  }

  void on_finishing_reader() override {
    std::lock_guard lk{mt};
    finished = true;
  }

  void on_pipeline_failure() override {
    std::lock_guard lk{mt};
    pipeline_failure = true;
  }
};

struct writer_mock : public writer {
  std::mutex mt;
  std::vector<int> data;
  bool finished = false;
  bool pipeline_failure = false;

  void on_calc_block_hash(int hash) override {
    std::lock_guard lk{mt};
    data.push_back(hash);
  }

  void on_finishing_hash_calc() override {
    std::lock_guard lk{mt};
    finished = true;
  }

  void on_pipeline_failure() override {
    std::lock_guard lk{mt};
    pipeline_failure = true;
  }
};

}  // namespace file_signature

#endif  // FILE_SIGNATURE_FILE_SIGNATURE_TEST_H_
