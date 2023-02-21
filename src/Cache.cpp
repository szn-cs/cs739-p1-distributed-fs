#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;
extern std::string cacheDirectory;

// Cache logic: storing status cache (not file cache)
class Cache {
 public:
  static std::unordered_map<std::string, std::string> get_local_cache(const std::string& cacheDirectory) {
    std::unordered_map<std::string, std::string> cache;
    // if no file for cache then create new file for keeping cache file.
    // after the file exists, read each line and add key value pair into cache in
    // memory.
    // File format [key;value\n]:
    //            /temp/path/to/file;ijio1290ej9fjio
    //            /temp/path/to/file2;ijio1290ej9fjio
    std::string local_cache_path = cacheDirectory;  // + "cache_file.txt";

    std::ifstream cache_file(local_cache_path);
    std::string line;
    while (std::getline(cache_file, line)) {
      // txt;sha
      size_t pos = line.find(";");
      if (pos == std::string::npos) {
        std::cout << "read cache file " << local_cache_path << " error: cannot find separator ; " << std::endl;
        return cache;
      }
      std::string key = line.substr(0, pos);
      std::string val = line.substr(pos + 1, line.size() - pos);
      cache[key] = val;
    }
    return cache;
  }

  static int fsync_cache(std::string& cacheDirectory, std::unordered_map<std::string, std::string> cache) {
    // update cache into local cache file.
    std::string local_cache_path = cacheDirectory;  // + "cache_file.txt";
    std::string tmp_local_cache_path = local_cache_path + ".TMP";
    std::ofstream tmp_cache_file(tmp_local_cache_path);
    if (tmp_cache_file.is_open()) {
      for (auto i = cache.begin(); i != cache.end(); i++) {
        tmp_cache_file << i->first << ";" << i->second << std::endl;
      }
    }
    rename(tmp_local_cache_path.c_str(), local_cache_path.c_str());
    return 0;
  }

  static int fsync_file(std::string& FilePath, std::string& buf) {
    std::string tmp_local_file_path = FilePath + ".TMP";
    std::ofstream tmp_file(tmp_local_file_path);
    if (tmp_file.is_open()) {
      tmp_file << buf << std::endl;
    }
    rename(tmp_local_file_path.c_str(), FilePath.c_str());
    return 0;
  }

  static std::string get_hash_path(const std::string& path) {
    unsigned char md_buf[SHA256_DIGEST_LENGTH];  // 32
    SHA256(reinterpret_cast<const unsigned char*>(path.c_str()), path.size(), md_buf);
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
      ss << std::hex << std::setw(2) << std::setfill('0') << (int)md_buf[i];
    }
    return ss.str();
  }
};