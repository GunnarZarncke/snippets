# Critical Modern C++ Features Missing from Examples

## Overview
Your examples demonstrate: `auto`, range-based for loops, lambdas, `std::optional`, `enum class`, move semantics, deleted functions, raw string literals, `std::filesystem`, and `nullptr`.

Here are the **most important** features since 2000 that you should absolutely know:

---

## 🔴 CRITICAL - Must Know (C++11/14)

### 1. **Smart Pointers** (`std::unique_ptr`, `std::shared_ptr`, `std::weak_ptr`)
**Why critical:** Replaces raw pointers and manual memory management. Prevents memory leaks.

```cpp
#include <memory>

// unique_ptr - exclusive ownership, no overhead
std::unique_ptr<int> ptr = std::make_unique<int>(42);
auto ptr2 = std::make_unique<std::vector<int>>(10, 5);

// shared_ptr - shared ownership with reference counting
std::shared_ptr<Database> db = std::make_shared<Database>("tasks.db");
auto db2 = db;  // Both share ownership

// weak_ptr - breaks circular references
std::weak_ptr<Database> weak_db = db;
if (auto locked = weak_db.lock()) {
    // Use locked
}
```

**Key points:**
- `make_unique`/`make_shared` are preferred (exception-safe, more efficient)
- `unique_ptr` is move-only (like your Database class)
- `shared_ptr` has reference counting overhead
- Use `weak_ptr` to break cycles

---

### 2. **constexpr** (C++11) and **if constexpr** (C++17)
**Why critical:** Compile-time evaluation and conditional compilation.

```cpp
// constexpr functions - evaluated at compile time
constexpr int factorial(int n) {
    return n <= 1 ? 1 : n * factorial(n - 1);
}
constexpr int fact_10 = factorial(10);  // Computed at compile time

// if constexpr - compile-time if (C++17)
template<typename T>
auto process(T value) {
    if constexpr (std::is_integral_v<T>) {
        return value * 2;
    } else {
        return value + " processed";
    }
}
```

---

### 3. **Structured Bindings** (C++17)
**Why critical:** Clean syntax for unpacking tuples, pairs, structs.

```cpp
#include <tuple>
#include <map>

// Unpack tuples
auto [x, y, z] = std::make_tuple(1, 2.5, "hello");

// Unpack pairs (e.g., from map iteration)
std::map<std::string, int> m = {{"a", 1}, {"b", 2}};
for (const auto& [key, value] : m) {
    std::cout << key << ": " << value << "\n";
}

// Unpack structs
struct Point { int x, y; };
Point p{10, 20};
auto [px, py] = p;
```

---

### 4. **std::string_view** (C++17)
**Why critical:** Zero-copy string operations, better performance than `const std::string&`.

```cpp
#include <string_view>

// No allocation - just a view into existing string
void print(std::string_view sv) {
    std::cout << sv << "\n";
}

std::string s = "hello world";
print(s);                    // Works
print("literal string");     // Works - no temporary string created
print(s.substr(0, 5));      // Works - but be careful of lifetime!

// string_view doesn't own the data - ensure it outlives the view
```

---

### 5. **std::variant** and **std::optional** (C++17)
**Why critical:** Type-safe alternatives to unions and nullable types.

```cpp
#include <variant>
#include <optional>

// variant - type-safe union
std::variant<int, std::string, double> v = 42;
v = "hello";
v = 3.14;

// Visit pattern
std::visit([](auto& val) {
    std::cout << val << "\n";
}, v);

// optional - you have this, but more patterns:
std::optional<int> find_value(const std::vector<int>& vec, int target) {
    auto it = std::find(vec.begin(), vec.end(), target);
    return (it != vec.end()) ? std::make_optional(*it) : std::nullopt;
}

auto result = find_value(vec, 5);
if (result.has_value()) {
    std::cout << *result << "\n";
}
// Or: if (result) { ... }
```

---

## 🟡 IMPORTANT - Should Know (C++11/14/17)

### 6. **std::array** vs `std::vector`
**Why important:** Stack-allocated fixed-size array, better performance.

```cpp
#include <array>

// Fixed size, stack allocated (like C array but safer)
std::array<int, 5> arr = {1, 2, 3, 4, 5};
// vs std::vector<int> vec = {1, 2, 3, 4, 5};  // heap allocated

// Has .size(), iterators, bounds checking with .at()
arr.at(10);  // Throws std::out_of_range
arr[10];     // Undefined behavior (like C array)
```

---

### 7. **std::tuple** and **std::tie**
**Why important:** Returning multiple values, structured data.

```cpp
#include <tuple>

// Return multiple values
std::tuple<int, std::string, bool> get_info() {
    return {42, "hello", true};
}

auto [id, name, active] = get_info();  // Structured binding

// Or with std::tie (C++11)
int id;
std::string name;
bool active;
std::tie(id, name, active) = get_info();
```

---

### 8. **std::chrono** (C++11)
**Why important:** Modern time handling, replaces C time functions.

```cpp
#include <chrono>
#include <thread>

using namespace std::chrono;

// Time points and durations
auto start = steady_clock::now();
// ... do work ...
auto end = steady_clock::now();
auto duration = duration_cast<milliseconds>(end - start);
std::cout << "Took " << duration.count() << " ms\n";

// Sleep
std::this_thread::sleep_for(100ms);
std::this_thread::sleep_until(steady_clock::now() + 1s);
```

---

### 9. **Lambda Improvements** (C++14/17)
**Why important:** More powerful lambdas.

```cpp
// C++14: Generic lambdas
auto lambda = [](auto x, auto y) { return x + y; };

// C++14: Capture with initializers
auto ptr = std::make_unique<int>(42);
auto lambda2 = [ptr = std::move(ptr)]() { return *ptr; };

// C++17: constexpr lambdas
constexpr auto square = [](int x) { return x * x; };
static_assert(square(5) == 25);
```

---

### 10. **Type Traits** (`<type_traits>`)
**Why important:** Template metaprogramming, SFINAE, concepts foundation.

```cpp
#include <type_traits>

// Check types at compile time
static_assert(std::is_integral_v<int>);
static_assert(!std::is_pointer_v<int>);

// Enable/disable function overloads
template<typename T>
std::enable_if_t<std::is_integral_v<T>, T> process(T value) {
    return value * 2;
}

// C++20 concepts are cleaner, but type traits still useful
```

---

## 🟢 NICE TO KNOW (C++17/20)

### 11. **std::any** (C++17)
Type-erased container for any type.

### 12. **std::regex** (C++11)
Regular expressions (though often slower than alternatives).

### 13. **std::thread**, **std::async**, **std::future** (C++11)
Concurrency primitives.

### 14. **std::mutex**, **std::lock_guard**, **std::unique_lock** (C++11)
Thread synchronization.

### 15. **Concepts** (C++20)
Better than SFINAE for template constraints.

### 16. **Ranges** (C++20)
Modern, composable algorithms.

### 17. **Coroutines** (C++20)
Asynchronous programming.

---

## Quick Reference: What to Use When

| Old Way (pre-2000) | Modern Way (C++11+) |
|-------------------|---------------------|
| `int* ptr = new int(42); delete ptr;` | `auto ptr = std::make_unique<int>(42);` |
| `const char*` parameters | `std::string_view` |
| C arrays `int arr[10]` | `std::array<int, 10>` |
| `NULL` | `nullptr` |
| Manual memory management | Smart pointers |
| C-style casts | `static_cast`, `dynamic_cast`, etc. |
| `#define` constants | `constexpr` |
| Function pointers | Lambdas |
| `std::pair` for 2 values | `std::tuple` for N values |
| `void*` for type erasure | `std::any` or `std::variant` |

---

## Recommended Learning Order

1. **Smart pointers** - Most critical, use everywhere
2. **constexpr** - Performance and compile-time safety
3. **Structured bindings** - Cleaner code
4. **string_view** - Better string handling
5. **variant/optional** - Type safety
6. **chrono** - Modern time handling
7. **Type traits** - Template programming foundation

Then explore C++17/20 features as needed.
