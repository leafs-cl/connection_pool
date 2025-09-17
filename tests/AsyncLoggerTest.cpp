/*
* @Description: Test Logger Base Implement
* @Author: abellli
* @Date: 2025-09-15
* @LastEditTime: 2025-09-15
*/


#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>
#include <fmt/core.h>
#include "Logger.hpp"

/**
 * @class AsyncLoggerTest
 * Test class for verifying AsyncLogger functionality
 */
class AsyncLoggerTest {
public:
    /**
     * Run all test cases
     */
    static void runAllTests() {
        std::cout << "Starting AsyncLogger tests...\n";
        
        testBasicLogging();
        testLogLevelFiltering();
        testMultiThreadedLogging();
        testQueueFullBehavior();
        testFileRotation();
        testConsoleOutputToggle();
        
        std::cout << "All tests completed successfully!\n";
    }

private:
    /**
     * Test basic logging functionality with different levels
     */
    static void testBasicLogging() {
        std::cout << "Testing basic logging...\n";
        
        // Initialize logger with specific test file
        AsyncLogger::instance().init("test_basic.log", 1 * 1024 * 1024, 100, true, false);
        
        // Log messages at different levels
        DEBUG_LOG("This is a DEBUG message - should be visible if LOG_LEVEL is DEBUG");
        INFO_LOG("This is an INFO message");
        WARN_LOG("This is a WARN message");
        ERROR_LOG("This is an ERROR message");
        FATAL_LOG("This is a FATAL message");
        
        // Allow time for logs to be processed
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        std::cout << "Basic logging test completed.\n";
    }

    /**
     * @brief Test log level filtering functionality
     */
    static void testLogLevelFiltering() {
        std::cout << "Testing log level filtering...\n";
        
        // Note: This test assumes LOG_LEVEL is set to INFO by default
        // Messages below INFO level should be filtered out
        DEBUG_LOG("This DEBUG message should be filtered out (not appear in log)");
        INFO_LOG("This INFO message should appear");
        WARN_LOG("This WARN message should appear");
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "Log level filtering test completed.\n";
    }

    /**
     * @brief Test logging from multiple concurrent threads
     */
    static void testMultiThreadedLogging() {
        std::cout << "Testing multi-threaded logging...\n";
        
        constexpr int num_threads = 5;
        constexpr int messages_per_thread = 10;
        std::vector<std::thread> threads;
        std::atomic<int> messages_logged{0};
        
        AsyncLogger::instance().init("test_threaded.log", 2 * 1024 * 1024, 500, true, false);
        
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back([i, &messages_logged]() {
                for (int j = 0; j < messages_per_thread; ++j) {
                    INFO_LOG("Thread " << i << " message " << j);
                    messages_logged++;
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
            });
        }
        
        for (auto& thread : threads) {
            thread.join();
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(200)); // Allow final logs to process
        std::cout << "Multi-threaded test completed. Logged " << messages_logged << " messages.\n";
    }

    /**
     * @brief Test behavior when log queue becomes full
     */
    static void testQueueFullBehavior() {
        std::cout << "Testing queue full behavior...\n";
        
        // Initialize with very small queue to trigger full condition quickly
        AsyncLogger::instance().init("test_queue_full.log", 1 * 1024 * 1024, 10, true, false);
        
        // Rapidly log many messages to fill the queue
        for (int i = 0; i < 20; ++i) {
            INFO_LOG("Filling queue with message " << i);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        std::cout << "Queue full test completed. Check for drop warnings in console.\n";
    }

    /**
     * @brief Test file rotation functionality
     */
    static void testFileRotation() {
        std::cout << "Testing file rotation...\n";
        
        // Initialize with very small size to trigger rotation quickly
        AsyncLogger::instance().init("test_rotation.log", 100, 50, true, false);
        
        // Log enough messages to trigger file rotation
        for (int i = 0; i < 30; ++i) {
            INFO_LOG("This is message " << i << " designed to trigger file rotation when accumulated");
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        std::cout << "File rotation test completed.\n";
    }

    /**
     * @brief Test console output enable/disable functionality
     */
    static void testConsoleOutputToggle() {
        std::cout << "Testing console output toggle...\n";
        
        AsyncLogger::instance().init("test_console.log", 1 * 1024 * 1024, 50, true, true);
        
        INFO_LOG("This message should appear in console (console output enabled)");
        
        AsyncLogger::instance().enable_console_output(false);
        INFO_LOG("This message should NOT appear in console (console output disabled)");
        
        AsyncLogger::instance().enable_console_output(true);
        INFO_LOG("This message should appear again in console (console output re-enabled)");
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "Console output toggle test completed.\n";
    }
};

/**
 * Main function to run all tests
 */
int main() {
    try {
        AsyncLoggerTest::runAllTests();
        
        // Properly shutdown the logger after all tests
        AsyncLogger::instance().shutdown();
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}