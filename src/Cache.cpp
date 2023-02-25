#pragma once

#include <fstream>
#include <iomanip>
#include <iostream>
#include <shared_mutex>
#include <sstream>
#include <string>
#include <termcolor/termcolor.hpp>
#include <tuple>

#include "Utility.cpp"

using namespace std;
using termcolor::reset, termcolor::yellow, termcolor::red, termcolor::blue, termcolor::cyan;

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
    if (isCacheEntry()) {
      std::string hash_;
      int dirtyBit_;
      int clock_;
      tie(hash_, dirtyBit_, clock_) = this->statusCache[relativePath];
      this->dirtyBit = dirtyBit_;
      this->clock = clock_;
    } else {
      this->dirtyBit = 0;  // init
      this->clock = 0;     // TODO:
    }
  }
  // class functions
  bool isCacheValid(struct stat serverAttr) {
    struct stat localAttr;

    // check if cache entry for the path exists
    if (lstat(this->fileCachePath.c_str(), &localAttr) != 0)
      return false;

    // stale cache, need to fetch
    if (serverAttr.st_mtime > localAttr.st_mtime)
      return false;

    // local cache was stored after last server-modified (may have same content) or could be newer/locally-modified
    // if (serverAttr.st_mtime < localAttr.st_mtime) {
    //   goto OpenCachedFile;
    // }

    return true;
  }
  // check if cache entry exists for the fileCachePath
  bool isCacheEntry() {
    return this->statusCache.find(this->relativePath) != this->statusCache.end();
  }

  bool isDirty() {
    return this->dirtyBit;
  }

  void setDirtyBit() {
    this->dirtyBit = 1;
    updateCache();
    commitStatusCache();
  }

  void resetDirtyBit() {
    this->dirtyBit = 0;
    updateCache();
    commitStatusCache();
  }

  bool getClock() {
    return this->clock;
  }

  void syncClock(int time) {
    this->clock = time;
    updateCache();
    commitStatusCache();
  }

  void updateCache() {
    auto it = this->statusCache.find(this->relativePath);
    if (it != this->statusCache.end())
      it->second = make_tuple(this->hash, this->dirtyBit, this->clock);
  }

  // fsync commit fileCache to the root directory of FUSE/Unreliablefs FS
  int commitFileCache(std::string& buf) {
    std::string tmp_fileCachePath = this->fileCachePath + ".TMP";
    std::ofstream fileCacheStream(tmp_fileCachePath);

    if (!fileCacheStream.is_open())
      throw "Error @commitFileCache: openning fileCachePath: " + this->fileCachePath;

    fileCacheStream << buf << std::endl;

    if (rename(tmp_fileCachePath.c_str(), this->fileCachePath.c_str()) != 0) {
      std::cout << red << "rename fail" << reset << std::endl;
    }

    this->statusCache[this->relativePath] = make_tuple(this->hash, this->dirtyBit, this->clock);
    return 0;
  }

  // fsync update cache into local cache file.
  int commitStatusCache() {
    std::string tmp_statusCachePath = statusCachePath + ".TMP";

    std::ofstream tmp_cache_file(tmp_statusCachePath);
    if (!tmp_cache_file.is_open())
      throw "Error @commitStatusCache: openning statusCachePath: " + statusCachePath;

    for (auto i = this->statusCache.begin(); i != this->statusCache.end(); i++) {
      std::string hash_;
      int dirtyBit_;
      int clock_;
      tie(hash_, dirtyBit_, clock_) = i->second;
      tmp_cache_file << i->first << ";" << hash_ << ";" << dirtyBit_ << ";" << clock_ << "\n";
    }

    if (rename(tmp_statusCachePath.c_str(), statusCachePath.c_str()) != 0) {
      std::cout << red << "rename fail" << reset << std::endl;
    }

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
  int dirtyBit;
  int clock;
  //                                        hash, dirtybit, logical clock
  std::unordered_map<std::string, std::tuple<std::string, int, int>> statusCache;  // in-memory copy from the statusCachePath contents

  // static members
  static std::unordered_map<std::string, std::tuple<std::string, int, int>> getStatusCache();
  std::string getPathHash(const std::string& path);
};

/* if no file for cache then create new file for keeping cache file.
     after the file exists, read each line and add key value pair into cache in memory.
     File format [key;value\n]:
                <relative path>;<hash>
                /temp/path/to/file;ijio1290ej9fjio
                /temp/path/to/file2;ijio1290ej9fjio
  */
std::unordered_map<std::string, std::tuple<std::string, int, int>> Cache::getStatusCache() {
  std::unordered_map<std::string, std::tuple<std::string, int, int>> statusCache;
  std::ifstream statusCacheStream(statusCachePath);
  std::string line;

  while (std::getline(statusCacheStream, line)) {
    size_t pos = line.find(";");  // find delimiter `;` from statuc cache entry:  <txt>;<sha>
    std::string key = line.substr(0, pos);
    line.erase(0, pos + 1);

    pos = line.find(";");
    std::string hash = line.substr(0, pos);
    line.erase(0, pos + 1);

    pos = line.find(";");
    std::string dirtyBit_ = line.substr(0, pos);
    int dirtyBit = stoi(dirtyBit_);
    line.erase(0, pos + 1);

    int clock = stoi(line);
    statusCache[key] = make_tuple(hash, dirtyBit, clock);
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
