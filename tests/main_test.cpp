#include <gtest/gtest.h>
#include <iostream>

int main(int argc, char **argv) {
    std::cout << "Running ToyC Compiler Tests..." << std::endl;

    ::testing::InitGoogleTest(&argc, argv);

    // 可以在這裡添加全局測試設定
    std::cout << "Initializing test environment..." << std::endl;

    int result = RUN_ALL_TESTS();

    if (result == 0) {
        std::cout << "All tests passed successfully!" << std::endl;
    } else {
        std::cout << "Some tests failed." << std::endl;
    }

    return result;
}
