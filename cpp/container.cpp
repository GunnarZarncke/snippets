#include <vector>
#include <algorithm>
#include <iostream>
#include <optional>

using namespace std;

int main() {
    vector<int> v;
    
    // Add elements
    v.push_back(3);
    v.push_back(1);
    v.push_back(4);
    v.push_back(1);
    v.push_back(5);
    
    cout << "Vector after adding elements: ";
    for (const auto& val : v) {
        cout << val << " ";
    }
    cout << "\n";
    
    // Delete an element
    auto it = find(v.begin(), v.end(), 1);
    if (it != v.end()) {
        v.erase(it);
        cout << "After deleting first 1: ";
        for (const auto& val : v) {
            cout << val << " ";
        }
        cout << "\n";
    }
    
    // Get median
    if (v.size() > 0) {
        sort(v.begin(), v.end());
        int median = v.at((v.size() - 1) / 2);
        cout << "Median: " << median << "\n";
    }
    
    return 0;
}
