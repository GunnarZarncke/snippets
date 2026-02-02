/**
 * Catch2 testing framework example (Catch2 v3).
 * 
 * To compile and run with Catch2 v3 installed via Homebrew:
 *   g++ -std=c++17 -I/opt/homebrew/include -L/opt/homebrew/lib \
 *       -o catch2_example catch2_example.cpp -lCatch2 -lCatch2Main
 *   ./catch2_example
 * 
 * Or with Catch2 installed in /usr/local:
 *   g++ -std=c++17 -I/usr/local/include -L/usr/local/lib \
 *       -o catch2_example catch2_example.cpp -lCatch2 -lCatch2Main
 *   ./catch2_example
 */

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main()
#include <catch2/catch_all.hpp>
using namespace Catch;

#include <vector>
#include <algorithm>
#include <string>

// Simple function to test
int add(int a, int b) {
    return a + b;
}

// Function to find maximum in vector
int find_max(const std::vector<int>& vec) {
    if (vec.empty()) {
        return 0;
    }
    return *std::max_element(vec.begin(), vec.end());
}

// Function to reverse a string
std::string reverse_string(const std::string& str) {
    return std::string(str.rbegin(), str.rend());
}

TEST_CASE("Basic arithmetic", "[math]") {
    REQUIRE(add(2, 3) == 5);
    REQUIRE(add(-1, 1) == 0);
    REQUIRE(add(0, 0) == 0);
}

TEST_CASE("Vector operations", "[vector]") {
    std::vector<int> vec = {3, 1, 4, 1, 5, 9, 2, 6};
    
    SECTION("Find maximum") {
        REQUIRE(find_max(vec) == 9);
        REQUIRE(find_max({}) == 0);
        REQUIRE(find_max({42}) == 42);
    }
    
    SECTION("Vector size") {
        REQUIRE(vec.size() == 8);
        vec.push_back(7);
        REQUIRE(vec.size() == 9);
    }
    
    SECTION("Vector contains") {
        auto it = std::find(vec.begin(), vec.end(), 5);
        REQUIRE(it != vec.end());
        REQUIRE(*it == 5);
        
        it = std::find(vec.begin(), vec.end(), 99);
        REQUIRE(it == vec.end());
    }
}

TEST_CASE("String operations", "[string]") {
    SECTION("Reverse string") {
        REQUIRE(reverse_string("hello") == "olleh");
        REQUIRE(reverse_string("") == "");
        REQUIRE(reverse_string("a") == "a");
        REQUIRE(reverse_string("123") == "321");
    }
    
    SECTION("String comparison") {
        std::string str = "test";
        REQUIRE(str == "test");
        REQUIRE(str != "Test");
        REQUIRE(str.length() == 4);
    }
}

TEST_CASE("Approximate comparisons", "[math]") {
    double pi = 3.14159;
    double approx_pi = 3.1416;
    
    REQUIRE(pi == Approx(3.14159).margin(0.0001));
    REQUIRE(approx_pi == Approx(pi).epsilon(0.001));
}

TEST_CASE("Exception testing", "[exceptions]") {
    SECTION("No exception") {
        REQUIRE_NOTHROW(add(1, 2));
    }
    
    SECTION("Exception thrown") {
        auto throw_func = []() {
            throw std::runtime_error("Test exception");
        };
        REQUIRE_THROWS(throw_func());
        REQUIRE_THROWS_AS(throw_func(), std::runtime_error);
        REQUIRE_THROWS_WITH(throw_func(), "Test exception");
    }
}

TEST_CASE("Multiple assertions", "[assertions]") {
    int x = 5;
    int y = 10;
    
    CHECK(x < y);      // Continues even if fails
    CHECK(y > x);
    REQUIRE(x + y == 15);  // Stops if fails
    
    // More checks after REQUIRE
    CHECK(x * 2 == 10);
}
