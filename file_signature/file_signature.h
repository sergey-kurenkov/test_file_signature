#ifndef FILE_SIGNATURE_FILE_SIGNATURE_H_
#define FILE_SIGNATURE_FILE_SIGNATURE_H_

#include <string>
#include <stdexcept>

namespace file_signature {

struct error : public std::runtime_error {
  explicit error(const std::string& s);
};

void generate(std::string input_file,
              std::string signature_file,
              int block_size);

}  // namespace file_signature

#endif  // FILE_SIGNATURE_FILE_SIGNATURE_H_
