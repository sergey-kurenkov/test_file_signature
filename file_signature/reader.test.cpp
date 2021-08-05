#include <file_signature/file_signature_impl.h>
#include <file_signature/file_signature.h>
#include <file_signature/file_signature.test.h>

#include <gtest/gtest.h>

#include <filesystem>
#include <mutex>
#include <fstream>
#include <ios>
#include <string>
#include <vector>
#include <future>

TEST(Reader, StartStop) {
  try {
    file_signature::create_file_for_reader(
      file_signature::default_input_file, 1, 'c');
    file_signature::hash_mock hm;

    file_signature::reader r{file_signature::default_input_file, 10, hm};
    auto reader_result = std::async(std::launch::async, [&](){r.run();});

    reader_result.wait();
    reader_result.get();

    std::lock_guard lk{hm.mt};
    ASSERT_EQ(hm.blocks.size(), 1);
    EXPECT_EQ(hm.blocks[0][0], 'c');
    EXPECT_TRUE(hm.finished);
  } catch (std::exception& e) {
    FAIL() << e.what();
  }
}

TEST(Reader, ReadOneBlock) {
    try {
        file_signature::create_file_for_reader(
          file_signature::default_input_file, 1, 'c');
        file_signature::hash_mock hm;

        file_signature::reader r{file_signature::default_input_file, 10, hm};
        r.run();

        std::lock_guard lk{hm.mt};
        ASSERT_EQ(hm.blocks.size(), 1);
        EXPECT_EQ(hm.blocks[0][0], 'c');
    } catch (std::exception& e) {
        FAIL() << e.what();
    }
}

TEST(Reader, ReadOneFullBlock) {
  try {
    file_signature::create_file_for_reader(
      file_signature::default_input_file, 10, 'c');
    file_signature::hash_mock hm;

    file_signature::reader r{file_signature::default_input_file, 10, hm};
    r.run();

    std::lock_guard lk{hm.mt};
    ASSERT_EQ(hm.blocks.size(), 1);
    for (auto i = 0; i < 10; i++) {
      ASSERT_EQ(hm.blocks[0][i], 'c');
    }
  } catch (std::exception& e) {
    FAIL() << e.what();
  }
}

TEST(Reader, ReadTwoBlocks) {
  try {
    file_signature::create_file_for_reader(
      file_signature::default_input_file, 20, 'd');
    file_signature::hash_mock hm;

    file_signature::reader r{file_signature::default_input_file, 10, hm};
    r.run();

    std::lock_guard lk{hm.mt};
    ASSERT_EQ(hm.blocks.size(), 2);
    for (auto j = 0; j < 2; j++) {
      for (auto i = 0; i < 10; i++) {
        ASSERT_EQ(hm.blocks[0][i], 'd');
      }
    }
  } catch (std::exception& e) {
    FAIL() << e.what();
  }
}

TEST(Reader, NotExistingFile) {
  file_signature::hash_mock hm;

  try {
    file_signature::delete_file_for_reader(file_signature::default_input_file);

    file_signature::reader r{file_signature::default_input_file, 10, hm};
    r.run();

    FAIL();
  } catch (std::exception& e) {
    std::lock_guard lk{hm.mt};
    ASSERT_EQ(hm.blocks.size(), 0);
    ASSERT_TRUE(hm.pipeline_failure);
  }
}

TEST(Reader, EmptyFile) {
  file_signature::hash_mock hm;

  try {
    file_signature::delete_file_for_reader(
      file_signature::default_input_file);
    file_signature::create_file_for_reader(
      file_signature::default_input_file, 0, 'c');

    file_signature::reader r{file_signature::default_input_file, 10, hm};
    r.run();

    std::lock_guard lk{hm.mt};
    ASSERT_EQ(hm.blocks.size(), 0);
    ASSERT_FALSE(hm.pipeline_failure);
  } catch (std::exception& e) {
    FAIL() << e.what();
  }
}
