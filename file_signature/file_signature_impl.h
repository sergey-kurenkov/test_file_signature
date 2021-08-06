#ifndef FILE_SIGNATURE_FILE_SIGNATURE_IMPL_H_
#define FILE_SIGNATURE_FILE_SIGNATURE_IMPL_H_

#include <mutex>
#include <queue>
#include <string>
#include <vector>

namespace file_signature {

using file_block = std::vector<char>;

struct hash_calc {
  virtual ~hash_calc() = default;
  virtual void on_read_block(file_block) = 0;
  virtual void on_finishing_reader() = 0;
  virtual void on_pipeline_failure() = 0;
};

struct writer {
  virtual ~writer() = default;
  virtual void on_calc_block_hash(int hash) = 0;
  virtual void on_finishing_hash_calc() = 0;
  virtual void on_pipeline_failure() = 0;
};

class reader {
 public:
  reader(const std::string& input_file, int block_size, hash_calc&);
  void run();

 private:
  std::string input_file;
  hash_calc& calc;
  int block_size;
};

class hash_calc_impl : public hash_calc {
 public:
  explicit hash_calc_impl(writer&);
  void on_read_block(file_block) override;
  void on_finishing_reader() override;
  void on_pipeline_failure() override;
  void run();

 private:
  writer& writer_;
  std::mutex mt;
  std::condition_variable cv;
  std::queue<file_block> blocks;
  bool reader_finished;
  bool failed;
  bool pipeline_failed;
};

class writer_impl : public writer {
 public:
  explicit writer_impl(const std::string& output_file);
  void on_calc_block_hash(int hash) override;
  void on_finishing_hash_calc() override;
  void on_pipeline_failure() override;
  void run();

 private:
  std::string output_file;
  std::mutex mt;
  std::condition_variable cv;
  std::vector<int> crc;
  bool hash_calc_finished;
  bool pipeline_failed;
};

class generator {
 public:
  generator(std::string input_file, std::string signature_file, int block_size);
  void run();

 private:
  std::string input_file;
  std::string signature_file;
  int block_size;
};

}  // namespace file_signature

#endif  // FILE_SIGNATURE_FILE_SIGNATURE_IMPL_H_
