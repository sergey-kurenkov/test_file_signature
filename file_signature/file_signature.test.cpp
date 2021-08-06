#include <file_signature/file_signature.test.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <ios>
#include <string>
#include <vector>

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
namespace file_signature {

void create_file_for_reader(std::string name, int size, char ch) {
  std::ofstream f(name, std::ios::binary | std::ios::trunc | std::ios::out);
  const int block_size = 1000000;
  if (size >= block_size) {
    std::vector<char> block(block_size, ch);
    while (size > block_size) {
      f.write(&block[0], block_size);
      size -= block_size;
    }
  }

  while (size-- > 0) {
    f << ch;
  }

  f.close();
}

void delete_file_for_reader(std::string name) { std::filesystem::remove(name); }

std::vector<std::string> read_file(std::string input_file) {
  std::ifstream s;

  s.exceptions(std::ifstream::badbit | std::ios_base::failbit);
  s.open(input_file, std::ios::in);

  s.exceptions(std::ifstream::badbit);

  std::vector<std::string> result;

  while (s) {
    std::string line;
    std::getline(s, line);
    if (line.empty() && s.eof()) {
      break;
    }
    result.push_back(line);
  }

  return std::move(result);
}

}  // namespace file_signature
