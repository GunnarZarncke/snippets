#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <iomanip>
#include <nlohmann/json.hpp>
#include <cxxopts.hpp>
#include <cstdlib>

using json = nlohmann::json;
namespace fs = std::filesystem;

class Library {
private:
    fs::path data_file;
    
    json load_data() {
        if (fs::exists(data_file)) {
            std::ifstream file(data_file);
            json data;
            file >> data;
            return data;
        }
        return json{{"books", json::array()}, {"borrowed", json::object()}};
    }
    
    void save_data(const json& data) {
        std::ofstream file(data_file);
        file << data.dump(2);
    }
    
public:
    Library() {
        const char* home = std::getenv("HOME");
        if (home) {
            data_file = fs::path(home) / ".library_data.json";
        } else {
            data_file = ".library_data.json";
        }
    }
    
    int add_book(const std::string& title, const std::string& author, 
                 const std::string& isbn = "") {
        json data = load_data();
        json book;
        book["id"] = data["books"].size() + 1;
        book["title"] = title;
        book["author"] = author;
        book["isbn"] = isbn.empty() ? 
            "ISBN-" + std::string(4 - std::to_string(data["books"].size() + 1).length(), '0') + 
            std::to_string(data["books"].size() + 1) : isbn;
        book["available"] = true;
        
        data["books"].push_back(book);
        save_data(data);
        
        std::cout << "Added book: " << title << " by " << author 
                  << " (ID: " << book["id"] << ")" << std::endl;
        return book["id"];
    }
    
    void list_books(bool available_only = false) {
        json data = load_data();
        std::vector<json> books = data["books"];
        
        if (available_only) {
            books.erase(
                std::remove_if(books.begin(), books.end(),
                    [](const json& b) { return !b["available"].get<bool>(); }),
                books.end()
            );
        }
        
        if (books.empty()) {
            std::cout << "No books found." << std::endl;
            return;
        }
        
        std::cout << "\n" << std::left << std::setw(5) << "ID" 
                  << std::setw(30) << "Title" 
                  << std::setw(25) << "Author" 
                  << std::setw(15) << "ISBN" 
                  << std::setw(10) << "Status" << std::endl;
        std::cout << std::string(85, '-') << std::endl;
        
        for (const auto& book : books) {
            std::string status = book["available"].get<bool>() ? "Available" : "Borrowed";
            std::cout << std::left << std::setw(5) << book["id"].get<int>()
                      << std::setw(30) << book["title"].get<std::string>()
                      << std::setw(25) << book["author"].get<std::string>()
                      << std::setw(15) << book["isbn"].get<std::string>()
                      << std::setw(10) << status << std::endl;
        }
    }
    
    void borrow_book(int book_id, const std::string& borrower) {
        json data = load_data();
        json* book = nullptr;
        
        for (auto& b : data["books"]) {
            if (b["id"].get<int>() == book_id) {
                book = &b;
                break;
            }
        }
        
        if (!book) {
            std::cout << "Error: Book with ID " << book_id << " not found." << std::endl;
            return;
        }
        
        if (!(*book)["available"].get<bool>()) {
            std::cout << "Error: Book '" << (*book)["title"].get<std::string>() 
                      << "' is already borrowed." << std::endl;
            return;
        }
        
        (*book)["available"] = false;
        data["borrowed"][std::to_string(book_id)] = borrower;
        save_data(data);
        
        std::cout << "'" << (*book)["title"].get<std::string>() 
                  << "' borrowed by " << borrower << std::endl;
    }
    
    void return_book(int book_id) {
        json data = load_data();
        json* book = nullptr;
        
        for (auto& b : data["books"]) {
            if (b["id"].get<int>() == book_id) {
                book = &b;
                break;
            }
        }
        
        if (!book) {
            std::cout << "Error: Book with ID " << book_id << " not found." << std::endl;
            return;
        }
        
        if ((*book)["available"].get<bool>()) {
            std::cout << "Error: Book '" << (*book)["title"].get<std::string>() 
                      << "' is already available." << std::endl;
            return;
        }
        
        std::string borrower = data["borrowed"].contains(std::to_string(book_id)) ?
            data["borrowed"][std::to_string(book_id)].get<std::string>() : "Unknown";
        
        data["borrowed"].erase(std::to_string(book_id));
        (*book)["available"] = true;
        save_data(data);
        
        std::cout << "'" << (*book)["title"].get<std::string>() 
                  << "' returned by " << borrower << std::endl;
    }
    
    void remove_book(int book_id) {
        json data = load_data();
        json* book = nullptr;
        size_t index = 0;
        
        for (size_t i = 0; i < data["books"].size(); ++i) {
            if (data["books"][i]["id"].get<int>() == book_id) {
                book = &data["books"][i];
                index = i;
                break;
            }
        }
        
        if (!book) {
            std::cout << "Error: Book with ID " << book_id << " not found." << std::endl;
            return;
        }
        
        if (!(*book)["available"].get<bool>()) {
            std::cout << "Error: Cannot remove '" << (*book)["title"].get<std::string>() 
                      << "' - it is currently borrowed." << std::endl;
            return;
        }
        
        data["books"].erase(data["books"].begin() + index);
        save_data(data);
        
        std::cout << "Removed book: '" << (*book)["title"].get<std::string>() << "'" << std::endl;
    }
    
    void search_books(const std::string& query) {
        json data = load_data();
        std::string query_lower = query;
        std::transform(query_lower.begin(), query_lower.end(), query_lower.begin(), ::tolower);
        
        std::vector<json> matches;
        for (const auto& book : data["books"]) {
            std::string title = book["title"].get<std::string>();
            std::string author = book["author"].get<std::string>();
            std::transform(title.begin(), title.end(), title.begin(), ::tolower);
            std::transform(author.begin(), author.end(), author.begin(), ::tolower);
            
            if (title.find(query_lower) != std::string::npos || 
                author.find(query_lower) != std::string::npos) {
                matches.push_back(book);
            }
        }
        
        if (matches.empty()) {
            std::cout << "No books found matching '" << query << "'" << std::endl;
            return;
        }
        
        std::cout << "\nFound " << matches.size() << " book(s) matching '" << query << "':" << std::endl;
        std::cout << std::left << std::setw(5) << "ID" 
                  << std::setw(30) << "Title" 
                  << std::setw(25) << "Author" 
                  << std::setw(10) << "Status" << std::endl;
        std::cout << std::string(70, '-') << std::endl;
        
        for (const auto& book : matches) {
            std::string status = book["available"].get<bool>() ? "Available" : "Borrowed";
            std::cout << std::left << std::setw(5) << book["id"].get<int>()
                      << std::setw(30) << book["title"].get<std::string>()
                      << std::setw(25) << book["author"].get<std::string>()
                      << std::setw(10) << status << std::endl;
        }
    }
};

int main(int argc, char* argv[]) {
    Library library;
    
    cxxopts::Options options("library", "Library Management System CLI");
    
    options.add_options()
        ("a,add", "Add a new book", cxxopts::value<std::vector<std::string>>())
        ("l,list", "List all books")
        ("available", "Show only available books")
        ("b,borrow", "Borrow a book", cxxopts::value<std::vector<std::string>>())
        ("r,return", "Return a borrowed book", cxxopts::value<int>())
        ("remove", "Remove a book", cxxopts::value<int>())
        ("s,search", "Search for books", cxxopts::value<std::string>())
        ("h,help", "Print help");
    
    try {
        auto result = options.parse(argc, argv);
        
        if (result.count("help") || argc == 1) {
            std::cout << options.help() << std::endl;
            return 0;
        }
        
        if (result.count("add")) {
            auto args = result["add"].as<std::vector<std::string>>();
            if (args.size() >= 2) {
                std::string isbn = args.size() > 2 ? args[2] : "";
                library.add_book(args[0], args[1], isbn);
            } else {
                std::cout << "Error: add requires title and author" << std::endl;
            }
        } else if (result.count("list")) {
            library.list_books(result.count("available") > 0);
        } else if (result.count("borrow")) {
            auto args = result["borrow"].as<std::vector<std::string>>();
            if (args.size() >= 2) {
                library.borrow_book(std::stoi(args[0]), args[1]);
            } else {
                std::cout << "Error: borrow requires book_id and borrower" << std::endl;
            }
        } else if (result.count("return")) {
            library.return_book(result["return"].as<int>());
        } else if (result.count("remove")) {
            library.remove_book(result["remove"].as<int>());
        } else if (result.count("search")) {
            library.search_books(result["search"].as<std::string>());
        }
        
    } catch (const cxxopts::exceptions::exception& e) {
        std::cerr << "Error parsing options: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
