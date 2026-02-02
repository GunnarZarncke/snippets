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
#include <ctime>

namespace fs = std::filesystem;

// LRU cache with file-based eviction (similar to functools.lru_cache behavior)
class ImageCache {
private:
    fs::path cache_dir;
    size_t max_size;
    CURL* curl;
    
    // Track access order for LRU eviction
    std::list<std::string> access_order;
    std::unordered_map<std::string, typename std::list<std::string>::iterator> url_to_iterator;
    
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
    
    void evict_oldest_file() {
        // Get all files with their modification times
        std::vector<std::pair<std::time_t, fs::path>> files;
        for (const auto& entry : fs::directory_iterator(cache_dir)) {
            if (entry.is_regular_file()) {
                auto mtime = fs::last_write_time(entry.path());
                auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                    mtime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
                auto time_t = std::chrono::system_clock::to_time_t(sctp);
                files.push_back({time_t, entry.path()});
            }
        }
        
        if (files.size() > max_size) {
            std::sort(files.begin(), files.end());
            fs::remove(files[0].second);
            std::cout << "Evicted LRU file: " << files[0].second << std::endl;
        }
    }
    
    void touch_url(const std::string& url) {
        // Move to end (most recently used)
        auto it = url_to_iterator.find(url);
        if (it != url_to_iterator.end()) {
            access_order.erase(it->second);
        }
        access_order.push_back(url);
        url_to_iterator[url] = std::prev(access_order.end());
    }
    
    std::optional<fs::path> get_impl(const std::string& url) {
        fs::path cache_path = get_cache_path(url);
        
        if (fs::exists(cache_path)) {
            touch_url(url);
            std::cout << "Using cached image: " << cache_path << std::endl;
            return cache_path;
        }
        
        return fetch_impl(url);
    }
    
    std::optional<fs::path> fetch_impl(const std::string& url) {
        fs::path cache_path = get_cache_path(url);
        
        // Evict oldest file if cache is full
        evict_oldest_file();
        
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
                
                touch_url(url);
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
        : cache_dir(cache_dir), max_size(max_size) {
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
        if (force_refresh) {
            // Clear from access tracking
            auto it = url_to_iterator.find(url);
            if (it != url_to_iterator.end()) {
                access_order.erase(it->second);
                url_to_iterator.erase(it);
            }
            return fetch_impl(url);
        }
        return get_impl(url);
    }
    
    std::optional<fs::path> get(const std::string& url) {
        return get_impl(url);
    }
    
    void clear() {
        for (const auto& entry : fs::directory_iterator(cache_dir)) {
            if (entry.is_regular_file()) {
                fs::remove(entry.path());
            }
        }
        access_order.clear();
        url_to_iterator.clear();
        std::cout << "Cleared cache directory: " << cache_dir << std::endl;
    }
    
    size_t get_cache_size() const {
        return access_order.size();
    }
    
    size_t get_hits() const {
        // Simplified: in real implementation, track hits/misses
        return 0;
    }
    
    size_t get_misses() const {
        // Simplified: in real implementation, track hits/misses
        return 0;
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
        
        std::cout << "LRU cache stats: " << cache.get_hits() << " hits, " 
                  << cache.get_misses() << " misses, " 
                  << cache.get_cache_size() << "/3 entries" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        curl_global_cleanup();
        return 1;
    }
    
    curl_global_cleanup();
    return 0;
}
