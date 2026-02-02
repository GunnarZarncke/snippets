#include <sqlite3.h>
#include <iostream>
#include <vector>
#include <map>
#include <memory>
#include <stdexcept>

// RAII wrapper for SQLite database connection
class Database {
private:
    sqlite3* db;
    
public:
    Database(const std::string& filename) : db(nullptr) {
        if (sqlite3_open(filename.c_str(), &db) != SQLITE_OK) {
            throw std::runtime_error("Cannot open database: " + std::string(sqlite3_errmsg(db)));
        }
    }
    
    ~Database() {
        if (db) {
            sqlite3_close(db);
        }
    }
    
    // Non-copyable
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    
    // Movable
    Database(Database&& other) noexcept : db(other.db) {
        other.db = nullptr;
    }
    
    sqlite3* get() { return db; }
    
    void execute(const std::string& sql) {
        char* errMsg = nullptr;
        if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
            std::string error = errMsg ? errMsg : "Unknown error";
            sqlite3_free(errMsg);
            throw std::runtime_error("SQL error: " + error);
        }
    }
    
    void execute(const std::string& sql, const std::vector<std::string>& params) {
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            throw std::runtime_error("Cannot prepare statement: " + std::string(sqlite3_errmsg(db)));
        }
        
        for (size_t i = 0; i < params.size(); ++i) {
            sqlite3_bind_text(stmt, i + 1, params[i].c_str(), -1, SQLITE_STATIC);
        }
        
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            throw std::runtime_error("Cannot execute statement: " + std::string(sqlite3_errmsg(db)));
        }
        
        sqlite3_finalize(stmt);
    }
    
    int64_t execute_insert(const std::string& sql, const std::vector<std::string>& params) {
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            throw std::runtime_error("Cannot prepare statement: " + std::string(sqlite3_errmsg(db)));
        }
        
        for (size_t i = 0; i < params.size(); ++i) {
            sqlite3_bind_text(stmt, i + 1, params[i].c_str(), -1, SQLITE_STATIC);
        }
        
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            throw std::runtime_error("Cannot execute statement: " + std::string(sqlite3_errmsg(db)));
        }
        
        int64_t id = sqlite3_last_insert_rowid(db);
        sqlite3_finalize(stmt);
        return id;
    }
    
    std::vector<std::map<std::string, std::string>> query(const std::string& sql, 
                                                          const std::vector<std::string>& params = {}) {
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            throw std::runtime_error("Cannot prepare statement: " + std::string(sqlite3_errmsg(db)));
        }
        
        for (size_t i = 0; i < params.size(); ++i) {
            sqlite3_bind_text(stmt, i + 1, params[i].c_str(), -1, SQLITE_STATIC);
        }
        
        std::vector<std::map<std::string, std::string>> results;
        
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::map<std::string, std::string> row;
            int colCount = sqlite3_column_count(stmt);
            for (int i = 0; i < colCount; ++i) {
                const char* colName = sqlite3_column_name(stmt, i);
                const unsigned char* colValue = sqlite3_column_text(stmt, i);
                row[colName] = colValue ? reinterpret_cast<const char*>(colValue) : "";
            }
            results.push_back(row);
        }
        
        sqlite3_finalize(stmt);
        return results;
    }
    
    int total_changes() {
        return sqlite3_total_changes(db);
    }
    
    void begin_transaction() {
        execute("BEGIN TRANSACTION");
    }
    
    void commit() {
        execute("COMMIT");
    }
    
    void rollback() {
        execute("ROLLBACK");
    }
};

void init_db(Database& db) {
    db.execute(R"(
        CREATE TABLE IF NOT EXISTS tasks (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            title TEXT NOT NULL,
            completed INTEGER DEFAULT 0
        )
    )");
}

// CREATE
int64_t create_task(Database& db, const std::string& title) {
    return db.execute_insert("INSERT INTO tasks (title) VALUES (?)", {title});
}

// READ
std::vector<std::map<std::string, std::string>> get_all_tasks(Database& db) {
    return db.query("SELECT * FROM tasks");
}

std::map<std::string, std::string> get_task(Database& db, int64_t task_id) {
    auto results = db.query("SELECT * FROM tasks WHERE id = ?", {std::to_string(task_id)});
    return results.empty() ? std::map<std::string, std::string>() : results[0];
}

// UPDATE
bool update_task(Database& db, int64_t task_id, 
                 const std::string& title = "", bool has_title = false,
                 bool completed = false, bool has_completed = false) {
    std::vector<std::string> updates;
    std::vector<std::string> params;
    
    if (has_title) {
        updates.push_back("title = ?");
        params.push_back(title);
    }
    if (has_completed) {
        updates.push_back("completed = ?");
        params.push_back(std::to_string(completed ? 1 : 0));
    }
    
    if (updates.empty()) {
        return false;
    }
    
    std::string sql = "UPDATE tasks SET " + 
                     [&]() {
                         std::string result;
                         for (size_t i = 0; i < updates.size(); ++i) {
                             if (i > 0) result += ", ";
                             result += updates[i];
                         }
                         return result;
                     }() + 
                     " WHERE id = ?";
    
    params.push_back(std::to_string(task_id));
    
    int changes_before = db.total_changes();
    db.execute(sql, params);
    return db.total_changes() > changes_before;
}

// DELETE
bool delete_task(Database& db, int64_t task_id) {
    int changes_before = db.total_changes();
    db.execute("DELETE FROM tasks WHERE id = ?", {std::to_string(task_id)});
    return db.total_changes() > changes_before;
}

int main() {
    try {
        Database db("tasks.db");
        init_db(db);
        
        // Create
        int64_t task_id = create_task(db, "Learn C++");
        std::cout << "Created task with id: " << task_id << std::endl;
        
        // Read
        auto tasks = get_all_tasks(db);
        std::cout << "All tasks:" << std::endl;
        for (const auto& task : tasks) {
            std::cout << "  ID: " << task.at("id") 
                      << ", Title: " << task.at("title")
                      << ", Completed: " << task.at("completed") << std::endl;
        }
        
        // Update
        update_task(db, task_id, "", false, true, true);
        auto task = get_task(db, task_id);
        std::cout << "Updated task:" << std::endl;
        std::cout << "  ID: " << task.at("id") 
                  << ", Title: " << task.at("title")
                  << ", Completed: " << task.at("completed") << std::endl;
        
        // Delete
        delete_task(db, task_id);
        std::cout << "Task deleted" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
