#include <file_signature/file_signature.test.h>
#include <file_signature/file_signature_impl.h>
#include <gtest/gtest.h>

#include <future>
#include <string>

TEST(HashCalc, RunStop) {
  try {
    file_signature::writer_mock w;
    file_signature::hash_calc_impl h{w};

    auto hash_calc_result = std::async(std::launch::async, [&]() { h.run(); });

    h.on_finishing_reader();

    hash_calc_result.wait();
    hash_calc_result.get();

    std::lock_guard lk{w.mt};
    ASSERT_EQ(w.data.size(), 0);
  } catch (std::exception& e) {
    FAIL() << e.what();
  }
}

TEST(HashCalc, OneBlock) {
  try {
    file_signature::writer_mock w;
    file_signature::hash_calc_impl h{w};

    h.on_read_block({'c', 'c', 'c'});
    h.on_finishing_reader();

    h.run();

    std::lock_guard lk{w.mt};
    ASSERT_EQ(w.data.size(), 1);
  } catch (std::exception& e) {
    FAIL() << e.what();
  }
}

TEST(HashCalc, TwoBlocks) {
  try {
    file_signature::writer_mock w;
    file_signature::hash_calc_impl h{w};

    h.on_read_block({'c', 'c', 'c'});
    h.on_read_block({'c', 'c', 'c'});
    h.on_finishing_reader();

    h.run();

    std::lock_guard lk{w.mt};
    ASSERT_EQ(w.data.size(), 2);
    EXPECT_EQ(w.data[0], w.data[1]);
  } catch (std::exception& e) {
    FAIL() << e.what();
  }
}

TEST(HashCalc, TwoBlocksAsyncHandling) {
  try {
    file_signature::writer_mock w;
    file_signature::hash_calc_impl h{w};

    auto hash_calc_result = std::async(std::launch::async, [&]() { h.run(); });

    h.on_read_block({'c', 'c', 'c'});
    h.on_read_block({'c', 'c', 'c'});
    h.on_finishing_reader();

    hash_calc_result.wait();
    hash_calc_result.get();

    std::lock_guard lk{w.mt};
    ASSERT_EQ(w.data.size(), 2);
    EXPECT_EQ(w.data[0], w.data[1]);
  } catch (std::exception& e) {
    FAIL() << e.what();
  }
}

TEST(HashCalc, OnPipelineFailed) {
  try {
    file_signature::writer_mock w;
    file_signature::hash_calc_impl h{w};

    auto hash_calc_result = std::async(std::launch::async, [&]() { h.run(); });

    h.on_read_block({'c', 'c', 'c'});
    h.on_pipeline_failure();

    hash_calc_result.wait();
    hash_calc_result.get();

    std::lock_guard lk{w.mt};
    ASSERT_LE(w.data.size(), 1);
    ASSERT_TRUE(w.pipeline_failure);
  } catch (std::exception& e) {
    FAIL() << e.what();
  }
}
