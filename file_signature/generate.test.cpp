#include <file_signature/file_signature.h>
#include <file_signature/file_signature.test.h>
#include <file_signature/file_signature_impl.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <ios>
#include <mutex>
#include <string>
#include <vector>

TEST(Generate, NotExistingFile) {
  try {
    file_signature::delete_file_for_reader(file_signature::default_input_file);
    file_signature::delete_file_for_reader(file_signature::default_output_file);

    file_signature::generate(file_signature::default_input_file,
                             file_signature::default_output_file, 10);
    FAIL() << "expected to throw an exception";
  } catch (std::exception& e) {
    SUCCEED();
  }

  try {
    file_signature::read_file(file_signature::default_output_file);
    FAIL() << "don't expect file";
  } catch (std::exception& e) {
    SUCCEED();
  }
}

TEST(Generate, EmptyFile) {
  try {
    file_signature::delete_file_for_reader(file_signature::default_input_file);
    file_signature::delete_file_for_reader(file_signature::default_output_file);

    file_signature::create_file_for_reader(file_signature::default_input_file,
                                           0, 'c');

    file_signature::generate(file_signature::default_input_file,
                             file_signature::default_output_file, 10);
    SUCCEED();
  } catch (std::exception& e) {
    FAIL() << e.what();
  }

  try {
    auto lines = file_signature::read_file(file_signature::default_output_file);
    SUCCEED();
    ASSERT_EQ(0, lines.size());
  } catch (std::exception& e) {
    FAIL() << e.what();
  }
}

TEST(Generate, FileWithOneBlock) {
  try {
    file_signature::create_file_for_reader(file_signature::default_input_file,
                                           1, 'c');
    file_signature::generate(file_signature::default_input_file,
                             file_signature::default_output_file, 10);

    auto lines = file_signature::read_file(file_signature::default_output_file);
    ASSERT_EQ(1, lines.size());
    EXPECT_EQ("112844655", lines[0]);
  } catch (std::exception& e) {
    FAIL() << e.what();
  }
}

TEST(Generate, FileWithTwoBlock) {
  try {
    file_signature::create_file_for_reader(file_signature::default_input_file,
                                           20, 'c');
    file_signature::generate(file_signature::default_input_file,
                             file_signature::default_output_file, 10);

    auto lines = file_signature::read_file(file_signature::default_output_file);
    ASSERT_EQ(2, lines.size());
    EXPECT_EQ(lines[1], lines[0]);
  } catch (std::exception& e) {
    FAIL() << e.what();
  }
}
