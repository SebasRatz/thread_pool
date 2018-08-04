#include <vector>

#include "gtest/gtest.h"
#include "thread_pool.h"

template<typename T>
class PoolTest : public thread_pool::Pool<T> {
public:
    PoolTest(int num_workers) : thread_pool::Pool<T>(num_workers) {}
    std::queue<T> getWork() {
        std::lock_guard<std::mutex>(this->work_m_);
        return this->work_;
    }
};

TEST(PoolTesting, addIntToQueue) {
    PoolTest<int> pool(3);
    pool.addWork(3);
    int b = 4;
    pool.addWork(b);
    ASSERT_EQ(pool.getWork().size(), 2);
}

TEST(PoolTesting, addVectorToQueue) {
    using StrVec = std::vector<std::string>;

    const int num_second_vectors = 88;
    const int num_first_vectors = 3;
    PoolTest<StrVec> pool(5);

    std::vector<StrVec> vec;
    for (int i=0; i<num_first_vectors; ++i) {
    vec.push_back(StrVec{"Hello", "Bybye", "Foo"});
    }
    pool.addWork(vec);
    ASSERT_EQ(pool.getWork().size(), num_first_vectors);

    std::vector<StrVec> vec2;
    for (int i=0; i<num_second_vectors; ++i) {
    vec2.push_back(StrVec{"Hello", "Bybye", "Foo"});
    }

    pool.addWork(std::move(vec2));
    ASSERT_EQ(pool.getWork().size(), num_first_vectors + num_second_vectors);
}

TEST(PoolTesting, EndToEndTest) {
    // dummy method, caluclate sum of a and see if its the same as b
    auto foo = [](std::vector<int> a, int b){
        int sum = 0;
        for(const int element : a) {
            sum+= element;
        }
        if (sum == b) {
            std::cout << "sum is correct" << std::endl;
        }
    };

    int num_work = 500;
    PoolTest<std::vector<int>> pool(5);

    for(int i=0; i<num_work; ++i) {
        pool.addWork(std::vector<int>{1,2,3,5,6,7});
    }
    ASSERT_GT(pool.getWork().size(), 0);
    pool.run(foo, 2.0);
    ASSERT_EQ(pool.getWork().size(), 0);
}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
