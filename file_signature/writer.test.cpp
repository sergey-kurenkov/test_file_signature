#include <file_signature/file_signature.test.h>
#include <file_signature/file_signature_impl.h>
#include <gtest/gtest.h>

#include <future>
#include <string>

TEST(Writer, StartStop) {
  try {
    file_signature::delete_file_for_reader(file_signature::default_output_file);
    file_signature::writer_impl w{file_signature::default_output_file};

    auto writer_result = std::async(std::launch::async, [&]() { w.run(); });

    w.on_finishing_hash_calc();

    writer_result.wait();
    writer_result.get();
  } catch (std::exception& e) {
    FAIL() << e.what();
  }
}

TEST(Writer, WriteOneLine) {
  try {
    file_signature::delete_file_for_reader(file_signature::default_output_file);
    file_signature::writer_impl w{file_signature::default_output_file};

    auto writer_result = std::async(std::launch::async, [&]() { w.run(); });

    w.on_calc_block_hash(100);
    w.on_finishing_hash_calc();

    writer_result.wait();
    writer_result.get();

    auto lines = file_signature::read_file(file_signature::default_output_file);
    ASSERT_EQ(1, lines.size());
    EXPECT_EQ("100", lines[0]);
  } catch (std::exception& e) {
    FAIL() << e.what();
  }
}

TEST(Writer, WriteThreeLines) {
  try {
    file_signature::delete_file_for_reader(file_signature::default_output_file);
    file_signature::writer_impl w{file_signature::default_output_file};

    auto writer_result = std::async(std::launch::async, [&]() { w.run(); });

    w.on_calc_block_hash(100);
    w.on_calc_block_hash(-100);
    w.on_calc_block_hash(0);
    w.on_finishing_hash_calc();

    writer_result.wait();
    writer_result.get();

    auto lines = file_signature::read_file(file_signature::default_output_file);
    ASSERT_EQ(3, lines.size());
    EXPECT_EQ("100", lines[0]);
    EXPECT_EQ("-100", lines[1]);
    EXPECT_EQ("0", lines[2]);
  } catch (std::exception& e) {
    FAIL() << e.what();
  }
}

TEST(Writer, WriteOneLineThenPipelineFail) {
  try {
    file_signature::delete_file_for_reader(file_signature::default_output_file);
    file_signature::writer_impl w{file_signature::default_output_file};

    auto writer_result = std::async(std::launch::async, [&]() { w.run(); });

    w.on_calc_block_hash(100);
    w.on_pipeline_failure();

    writer_result.wait();
    writer_result.get();
  } catch (std::exception& e) {
    FAIL() << e.what();
  }

  try {
    auto lines = file_signature::read_file(file_signature::default_output_file);
    FAIL() << "not expected";
  } catch (std::exception& e) {
    SUCCEED();
  }
}
