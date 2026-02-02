/**
 * Modern C++ Features Example
 * Demonstrates critical features missing from other examples:
 * - Smart pointers (unique_ptr, shared_ptr, weak_ptr)
 * - constexpr
 * - Structured bindings
 * - string_view
 * - variant
 * - tuple
 * - chrono
 * - Type traits
 */

#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <string_view>
#include <variant>
#include <tuple>
#include <chrono>
#include <type_traits>
#include <map>
#include <array>

// ============================================================================
// 1. SMART POINTERS - Most Critical Feature
// ============================================================================

class Resource {
public:
    Resource(int id) : id_(id) {
        std::cout << "  Resource " << id_ << " created\n";
    }
    ~Resource() {
        std::cout << "  Resource " << id_ << " destroyed\n";
    }
    int id() const { return id_; }
private:
    int id_;
};

void demonstrate_smart_pointers() {
    std::cout << "\n=== SMART POINTERS ===\n";
    
    // unique_ptr - exclusive ownership, zero overhead
    {
        std::cout << "\n1. unique_ptr (exclusive ownership):\n";
        auto ptr1 = std::make_unique<Resource>(1);
        // auto ptr2 = ptr1;  // ERROR - cannot copy unique_ptr
        auto ptr2 = std::move(ptr1);  // Transfer ownership
        std::cout << "  ptr1 is now " << (ptr1 ? "valid" : "null") << "\n";
        std::cout << "  ptr2 owns resource " << ptr2->id() << "\n";
    }  // Resource automatically destroyed here
    
    // shared_ptr - shared ownership with reference counting
    {
        std::cout << "\n2. shared_ptr (shared ownership):\n";
        std::shared_ptr<Resource> shared1 = std::make_shared<Resource>(2);
        {
            auto shared2 = shared1;  // Both share ownership
            std::cout << "  Reference count: " << shared1.use_count() << "\n";
        }  // shared2 destroyed, but resource still alive
        std::cout << "  Reference count: " << shared1.use_count() << "\n";
    }  // Resource destroyed when last shared_ptr goes out of scope
    
    // weak_ptr - breaks circular references
    {
        std::cout << "\n3. weak_ptr (non-owning reference):\n";
        std::shared_ptr<Resource> shared = std::make_shared<Resource>(3);
        std::weak_ptr<Resource> weak = shared;
        
        std::cout << "  shared count: " << shared.use_count() << "\n";
        std::cout << "  weak expired: " << (weak.expired() ? "yes" : "no") << "\n";
        
        if (auto locked = weak.lock()) {  // Try to get shared_ptr
            std::cout << "  Successfully locked, resource id: " << locked->id() << "\n";
        }
        
        shared.reset();  // Release ownership
        std::cout << "  After reset, weak expired: " << (weak.expired() ? "yes" : "no") << "\n";
    }
}

// ============================================================================
// 2. CONSTEXPR - Compile-time evaluation
// ============================================================================

constexpr int factorial(int n) {
    return n <= 1 ? 1 : n * factorial(n - 1);
}

template<typename T>
constexpr T square(T x) {
    return x * x;
}

// C++17: if constexpr
template<typename T>
auto process_value(T value) {
    if constexpr (std::is_integral_v<T>) {
        return value * 2;
    } else if constexpr (std::is_floating_point_v<T>) {
        return value * 1.5;
    } else {
        return std::string(value) + " processed";
    }
}

void demonstrate_constexpr() {
    std::cout << "\n=== CONSTEXPR ===\n";
    
    // Computed at compile time
    constexpr int fact_5 = factorial(5);
    constexpr int sq_10 = square(10);
    
    std::cout << "  factorial(5) = " << fact_5 << " (compile-time)\n";
    std::cout << "  square(10) = " << sq_10 << " (compile-time)\n";
    
    std::cout << "  process_value(5) = " << process_value(5) << "\n";
    std::cout << "  process_value(3.14) = " << process_value(3.14) << "\n";
}

// ============================================================================
// 3. STRUCTURED BINDINGS (C++17)
// ============================================================================

std::tuple<int, std::string, bool> get_user_info() {
    return {42, "Alice", true};
}

void demonstrate_structured_bindings() {
    std::cout << "\n=== STRUCTURED BINDINGS ===\n";
    
    // Unpack tuple
    auto [id, name, active] = get_user_info();
    std::cout << "  User: id=" << id << ", name=" << name 
              << ", active=" << active << "\n";
    
    // Unpack map entries
    std::map<std::string, int> scores = {{"Alice", 95}, {"Bob", 87}, {"Charlie", 92}};
    std::cout << "\n  Scores:\n";
    for (const auto& [player, score] : scores) {
        std::cout << "    " << player << ": " << score << "\n";
    }
    
    // Unpack array
    std::array<int, 3> coords = {10, 20, 30};
    auto [x, y, z] = coords;
    std::cout << "\n  Coordinates: (" << x << ", " << y << ", " << z << ")\n";
}

// ============================================================================
// 4. STRING_VIEW - Zero-copy string operations
// ============================================================================

void print_string(std::string_view sv) {
    std::cout << "  Length: " << sv.length() << ", Content: " << sv << "\n";
}

void demonstrate_string_view() {
    std::cout << "\n=== STRING_VIEW ===\n";
    
    std::string str = "Hello, World!";
    print_string(str);                    // Works with std::string
    print_string("Literal string");      // Works with literals (no temp string!)
    print_string(str.substr(0, 5));       // Works with substrings
    
    // string_view is just a view - doesn't own data
    std::string_view view = str;
    std::cout << "  View of string: " << view << "\n";
    
    // Warning: ensure source outlives the view!
    // std::string_view bad_view = std::string("temporary");  // DANGEROUS!
}

// ============================================================================
// 5. VARIANT - Type-safe union
// ============================================================================

using Value = std::variant<int, std::string, double>;

void print_value(const Value& v) {
    std::visit([](const auto& val) {
        std::cout << "  Value: " << val << " (type: " 
                  << typeid(val).name() << ")\n";
    }, v);
}

void demonstrate_variant() {
    std::cout << "\n=== VARIANT ===\n";
    
    Value v1 = 42;
    Value v2 = std::string("hello");
    Value v3 = 3.14;
    
    print_value(v1);
    print_value(v2);
    print_value(v3);
    
    // Type-safe access
    if (std::holds_alternative<int>(v1)) {
        std::cout << "  v1 contains int: " << std::get<int>(v1) << "\n";
    }
}

// ============================================================================
// 6. TUPLE - Multiple return values
// ============================================================================

std::tuple<bool, int, std::string> divide(int a, int b) {
    if (b == 0) {
        return {false, 0, "Division by zero"};
    }
    return {true, a / b, "Success"};
}

void demonstrate_tuple() {
    std::cout << "\n=== TUPLE ===\n";
    
    auto [success1, result1, message1] = divide(10, 2);
    std::cout << "  10 / 2: success=" << success1 << ", result=" << result1 
              << ", message=" << message1 << "\n";
    
    auto [success2, result2, message2] = divide(10, 0);
    std::cout << "  10 / 0: success=" << success2 << ", result=" << result2 
              << ", message=" << message2 << "\n";
}

// ============================================================================
// 7. CHRONO - Modern time handling
// ============================================================================

void demonstrate_chrono() {
    std::cout << "\n=== CHRONO ===\n";
    
    using namespace std::chrono;
    
    auto start = steady_clock::now();
    
    // Simulate some work
    for (int i = 0; i < 1000000; ++i) {
        volatile int x = i * 2;
    }
    
    auto end = steady_clock::now();
    auto duration = duration_cast<microseconds>(end - start);
    
    std::cout << "  Work took " << duration.count() << " microseconds\n";
    
    // Time literals (C++14)
    auto timeout = 500ms;
    std::cout << "  Timeout: " << timeout.count() << " milliseconds\n";
}

// ============================================================================
// 8. TYPE TRAITS - Compile-time type inspection
// ============================================================================

template<typename T>
void print_type_info() {
    std::cout << "  Type: " << typeid(T).name() << "\n";
    std::cout << "    is_integral: " << std::is_integral_v<T> << "\n";
    std::cout << "    is_pointer: " << std::is_pointer_v<T> << "\n";
    std::cout << "    is_const: " << std::is_const_v<T> << "\n";
}

template<typename T>
std::enable_if_t<std::is_integral_v<T>, T> double_value(T x) {
    return x * 2;
}

template<typename T>
std::enable_if_t<!std::is_integral_v<T>, T> double_value(T x) {
    return x + x;  // For strings, concatenate
}

void demonstrate_type_traits() {
    std::cout << "\n=== TYPE TRAITS ===\n";
    
    print_type_info<int>();
    print_type_info<int*>();
    print_type_info<const int>();
    
    std::cout << "\n  double_value(5) = " << double_value(5) << "\n";
    std::cout << "  double_value(\"hi\") = " << double_value(std::string("hi")) << "\n";
}

// ============================================================================
// 9. ARRAY vs VECTOR
// ============================================================================

void demonstrate_array() {
    std::cout << "\n=== ARRAY vs VECTOR ===\n";
    
    // std::array - fixed size, stack allocated
    std::array<int, 5> arr = {1, 2, 3, 4, 5};
    std::cout << "  std::array size: " << arr.size() << "\n";
    std::cout << "  arr[2] = " << arr[2] << "\n";
    
    // Bounds checking with .at()
    try {
        std::cout << "  arr.at(10) = " << arr.at(10) << "\n";
    } catch (const std::out_of_range& e) {
        std::cout << "  Caught: " << e.what() << "\n";
    }
    
    // std::vector - dynamic size, heap allocated
    std::vector<int> vec = {1, 2, 3, 4, 5};
    vec.push_back(6);  // Can grow
    std::cout << "  std::vector size: " << vec.size() << "\n";
}

// ============================================================================
// MAIN
// ============================================================================

int main() {
    std::cout << "Modern C++ Features Demonstration\n";
    std::cout << "==================================\n";
    
    demonstrate_smart_pointers();
    demonstrate_constexpr();
    demonstrate_structured_bindings();
    demonstrate_string_view();
    demonstrate_variant();
    demonstrate_tuple();
    demonstrate_chrono();
    demonstrate_type_traits();
    demonstrate_array();
    
    std::cout << "\n=== Summary ===\n";
    std::cout << "These are the most critical modern C++ features to learn.\n";
    std::cout << "Smart pointers should be your #1 priority!\n";
    
    return 0;
}
