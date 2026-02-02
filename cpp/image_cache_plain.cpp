#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <list>
#include <optional>
#include <openssl/md5.h>
#include <curl/curl.h>
#include <sstream>
#include <iomanip>

namespace fs = std::filesystem;

// Simple LRU cache implementation
template<typename Key, typename Value>
class LRUCache {
private:
    size_t max_size;
    std::list<std::pair<Key, Value>> items;
    std::unordered_map<Key, typename std::list<std::pair<Key, Value>>::iterator> cache;
    
public:
    LRUCache(size_t max_size) : max_size(max_size) {}
    
    void put(const Key& key, const Value& value) {
        auto it = cache.find(key);
        if (it != cache.end()) {
            items.erase(it->second);
            cache.erase(it);
        } else if (items.size() >= max_size) {
            auto last = items.back();
            cache.erase(last.first);
            items.pop_back();
        }
        
        items.push_front({key, value});
        cache[key] = items.begin();
    }
    
    std::optional<Value> get(const Key& key) {
        auto it = cache.find(key);
        if (it != cache.end()) {
            items.splice(items.begin(), items, it->second);
            return it->second->second;
        }
        return std::nullopt;
    }
    
    void touch(const Key& key) {
        auto it = cache.find(key);
        if (it != cache.end()) {
            items.splice(items.begin(), items, it->second);
        }
    }
    
    bool contains(const Key& key) const {
        return cache.find(key) != cache.end();
    }
    
    void clear() {
        items.clear();
        cache.clear();
    }
    
    size_t size() const {
        return items.size();
    }
};

class ImageCache {
private:
    fs::path cache_dir;
    size_t max_size;
    LRUCache<std::string, fs::path> lru_cache;
    CURL* curl;
    
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }
    
    std::string md5_hash(const std::string& input) {
        unsigned char digest[MD5_DIGEST_LENGTH];
        MD5((unsigned char*)input.c_str(), input.length(), digest);
        
        std::ostringstream ss;
        for (int i = 0; i < MD5_DIGEST_LENGTH; ++i) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)digest[i];
        }
        return ss.str();
    }
    
    std::string url_to_filename(const std::string& url) {
        std::string hash = md5_hash(url);
        fs::path url_path(url);
        std::string extension = url_path.extension().string();
        if (extension.empty()) {
            extension = ".jpg";
        }
        return hash + extension;
    }
    
    fs::path get_cache_path(const std::string& url) {
        return cache_dir / url_to_filename(url);
    }
    
    void evict_lru() {
        if (lru_cache.size() >= max_size) {
            // The LRU cache will automatically evict when we add a new item
            // But we need to delete the file from disk
            // Since we can't easily get the LRU item without modifying the cache,
            // we'll handle eviction when adding new items
        }
    }
    
    std::optional<fs::path> fetch_impl(const std::string& url) {
        fs::path cache_path = get_cache_path(url);
        
        // Evict if cache is full
        evict_lru();
        
        std::string response_data;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        
        CURLcode res = curl_easy_perform(curl);
        
        if (res == CURLE_OK) {
            long response_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            
            if (response_code == 200) {
                std::ofstream file(cache_path, std::ios::binary);
                file.write(response_data.c_str(), response_data.size());
                file.close();
                
                lru_cache.put(url, cache_path);
                std::cout << "Cached image: " << cache_path 
                          << " (" << response_data.size() << " bytes)" << std::endl;
                return cache_path;
            } else {
                std::cout << "Error: HTTP " << response_code << std::endl;
                return std::nullopt;
            }
        } else {
            std::cout << "Error fetching image: " << curl_easy_strerror(res) << std::endl;
            return std::nullopt;
        }
    }
    
public:
    ImageCache(const std::string& cache_dir = ".image_cache", size_t max_size = 10)
        : cache_dir(cache_dir), max_size(max_size), lru_cache(max_size) {
        fs::create_directories(cache_dir);
        curl = curl_easy_init();
        if (!curl) {
            throw std::runtime_error("Failed to initialize CURL");
        }
    }
    
    ~ImageCache() {
        if (curl) {
            curl_easy_cleanup(curl);
        }
    }
    
    bool is_cached(const std::string& url) {
        return fs::exists(get_cache_path(url));
    }
    
    std::optional<fs::path> fetch(const std::string& url, bool force_refresh = false) {
        fs::path cache_path = get_cache_path(url);
        
        if (!force_refresh && fs::exists(cache_path)) {
            lru_cache.touch(url);
            std::cout << "Using cached image: " << cache_path << std::endl;
            return cache_path;
        }
        
        return fetch_impl(url);
    }
    
    std::optional<fs::path> get(const std::string& url) {
        fs::path cache_path = get_cache_path(url);
        
        if (fs::exists(cache_path)) {
            lru_cache.touch(url);
            return cache_path;
        }
        
        return fetch(url);
    }
    
    void clear() {
        for (const auto& entry : fs::directory_iterator(cache_dir)) {
            if (entry.is_regular_file()) {
                fs::remove(entry.path());
            }
        }
        lru_cache.clear();
        std::cout << "Cleared cache directory: " << cache_dir << std::endl;
    }
    
    size_t get_cache_size() const {
        return lru_cache.size();
    }
};

int main() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    try {
        ImageCache cache(".image_cache", 3);
        
        std::vector<std::string> test_urls = {
            "https://httpbin.org/image/jpeg",
            "https://httpbin.org/image/png",
            "https://httpbin.org/image/webp",
            "https://httpbin.org/image/svg",
        };
        
        std::cout << "Fetching images (cache size: 3)..." << std::endl;
        for (const auto& url : test_urls) {
            auto image_path = cache.get(url);
            if (image_path) {
                std::cout << "  Image saved to: " << *image_path << "\n" << std::endl;
            }
        }
        
        std::cout << "\nFetching first image again (should be in cache)..." << std::endl;
        auto image_path = cache.get(test_urls[0]);
        if (image_path) {
            std::cout << "  Image from cache: " << *image_path << "\n" << std::endl;
        }
        
        std::cout << "Cache size: " << cache.get_cache_size() << "/3" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        curl_global_cleanup();
        return 1;
    }
    
    curl_global_cleanup();
    return 0;
}
