#pragma once

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <termcolor/termcolor.hpp>

#include "Utility.cpp"

using namespace std;
using termcolor::reset, termcolor::yellow, termcolor::red, termcolor::blue;

extern std::string statusCachePath;

/** Cache logic: 
 * Types of caches: 
 *   1. status cache (statusCachePath in AFS cache directory)
 *   2. file cache (fileCachePath in FUSE root directory) 
 */
class Cache {
 public:
  Cache(string relativePath) : relativePath(relativePath) {
    this->statusCache = Cache::getStatusCache();
    this->hash = Cache::getPathHash(relativePath);
    this->fileCachePath = Utility::concatenatePath(fsRootPath, hash);
  }

  // check if cache entry exists for the fileCachePath
  bool isCacheEntry() {
    return this->statusCache.find(this->relativePath) != this->statusCache.end();
  }

  // fsync commit fileCache to the root directory of FUSE/Unreliablefs FS
  int commitFileCache(std::string& buf) {
    std::string tmp_fileCachePath = this->fileCachePath + ".TMP";
    std::ofstream fileCacheStream(tmp_fileCachePath);

    if (!fileCacheStream.is_open())
      throw "Error @commitFileCache: openning fileCachePath: " + this->fileCachePath;

    fileCacheStream << buf << std::endl;

    rename(tmp_fileCachePath.c_str(), this->fileCachePath.c_str());

    this->statusCache.insert(std::pair<std::string, std::string>(this->relativePath, this->hash));

    return 0;
  }

  // fsync update cache into local cache file.
  int commitStatusCache() {
    std::string tmp_statusCachePath = statusCachePath + ".TMP";

    std::ofstream tmp_cache_file(tmp_statusCachePath);
    if (!tmp_cache_file.is_open())
      throw "Error @commitStatusCache: openning statusCachePath: " + statusCachePath;

    for (auto i = this->statusCache.begin(); i != this->statusCache.end(); i++)
      tmp_cache_file << i->first << ";" << i->second << std::endl;

    rename(tmp_statusCachePath.c_str(), statusCachePath.c_str());

    return 0;
  }

  int deleteEntry() {
    int ret = unlink(this->fileCachePath.c_str());
    if (ret != 0)
      return -errno;

    this->statusCache.erase(this->relativePath);
    this->commitStatusCache();

    return ret;
  }

 public:
  std::string relativePath;
  std::string fileCachePath;
  std::string hash;
  std::unordered_map<std::string, std::string> statusCache;  // in-memory copy from the statusCachePath contents

  // static members
  static std::unordered_map<std::string, std::string> getStatusCache();
  static std::string getPathHash(const std::string& path);
};

/* if no file for cache then create new file for keeping cache file.
     after the file exists, read each line and add key value pair into cache in memory.
     File format [key;value\n]:
                <relative path>;<hash>
                /temp/path/to/file;ijio1290ej9fjio
                /temp/path/to/file2;ijio1290ej9fjio
  */
std::unordered_map<std::string, std::string> Cache::getStatusCache() {
  std::unordered_map<std::string, std::string> statusCache;
  std::ifstream statusCacheStream(statusCachePath);
  std::string line;

  while (std::getline(statusCacheStream, line)) {
    size_t pos = line.find(";");  // find delimiter `;` from statuc cache entry:  <txt>;<sha>
    if (pos == std::string::npos)
      throw std::invalid_argument("Error @getStatusCache: bad format. Cannot find separator `;` in statusCacheFile:" + statusCachePath);

    std::string key = line.substr(0, pos);
    std::string val = line.substr(pos + 1, line.size() - pos);
    statusCache[key] = val;
  }

  return statusCache;
}

// create a hash from pathname
std::string Cache::getPathHash(const std::string& path) {
  unsigned char md_buf[SHA256_DIGEST_LENGTH];  // 32
  SHA256(reinterpret_cast<const unsigned char*>(path.c_str()), path.size(), md_buf);
  std::stringstream ss;
  for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
    ss << std::hex << std::setw(2) << std::setfill('0') << (int)md_buf[i];
  }
  return ss.str();
}
