#pragma once

#include <stdio.h>

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
 *
 * Notes: supporting multiple processes/threads using file-level locking is not required.
 * 
 * Client-side must tolerate a crash using local update protocol for caches.
 */
class Cache {
 public:
  Cache(std::string relativePath) : relativePath(relativePath) {
    //this->statusCache = Cache::getStatusCache();
    this->hash = Cache::getPathHash(relativePath);
    this->fileCachePath = Utility::concatenatePath(fsRootPath, hash);
    if (isCacheEntry()) {
      std::string hash_;
      int dirtyBit_;
      int clock_;
      tie(hash_, dirtyBit_, clock_) = statusCache[relativePath];
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
    return statusCache.find(this->relativePath) != statusCache.end();
  }

  bool isDirty() {
    return this->dirtyBit;
  }

  void setDirtyBit() {
    this->dirtyBit = 1;
    updateCache();
    //commitStatusCache();
  }

  void resetDirtyBit() {
    this->dirtyBit = 0;
    updateCache();
    //commitStatusCache();
  }

  bool getClock() {
    return this->clock;
  }

  void syncClock(int time) {
    this->clock = time;
    updateCache();
    //commitStatusCache();
  }

  void updateCache() {
    auto it = statusCache.find(relativePath);
    if (it != statusCache.end())
      it->second = make_tuple(this->hash, this->dirtyBit, this->clock);
    /*
    auto it = this->statusCache.find(this->relativePath);
    if (it != this->statusCache.end())
      it->second = make_tuple(this->hash, this->dirtyBit, this->clock);
    */
  }

  // fsync commit fileCache to the root directory of FUSE/Unreliablefs FS
  int commitFileCache(std::string& buf) {
    std::cout << "commitFileCache" << std::endl;
    FILE* fp;
    if ((fp = fopen(this->fileCachePath.c_str(), "w+")) == NULL) {
      std::cout << red << "open file failed" << reset << std::endl;
      exit(1);
    }

    if (buf.size() != 0) {
      if (fputs(buf.c_str(), fp) == EOF) {
        std::cout << red << "EOF" << reset << std::endl;
      }
      fclose(fp);
    } else {
      std::cout << "buf size = 0" << std::endl;
      fclose(fp);
    }

    /*
    std::string tmp_fileCachePath = this->fileCachePath + ".TMP";
    std::cout << "tmp_fileCachePath "<<tmp_fileCachePath << std::endl;
    FILE *fp;
    if ((fp = fopen(tmp_fileCachePath.c_str(), "w+")) != NULL) {
      if (buf.size() != 0){
        if (fputs(buf.c_str(), fp) == EOF) {
          std::cout << red << "EOF" << reset << std::endl;
        }
        fclose(fp);
      } else {
        std::cout << "buf size = 0" << std::endl;
        fclose(fp);
      }
    } else {
      std::cout << red  << "open tmp file failed" << reset  << std::endl;
    }
    

    FILE * fp1 = fopen(this->fileCachePath.c_str(), "r");
    if ( fp1 != NULL ) {
      std::cout  << "unlink this->fileCachePath.c_str()"  << std::endl;
      fclose(fp1);
      unlink(this->fileCachePath.c_str());
    }

    fp1 = fopen(tmp_fileCachePath.c_str(), "r");
    if ( fp1 == NULL ) {
      std::cout << red  << "tmp file not exist" << reset  << std::endl;
    } else {
      fclose(fp1);
    }
    
    if (rename(tmp_fileCachePath.c_str(), this->fileCachePath.c_str()) != 0) {
      std::cout << red << "rename fail" << reset << std::endl;
    }
    */
    /*
    std::ofstream fileCacheStream(tmp_fileCachePath.c_str(), std::ofstream::out | std::ofstream::binary);

    // don't use fileCacheStream.is_open()
    // if (!fileCacheStream.is_open())
    //   throw "Error @commitFileCache: openning fileCachePath: " + this->fileCachePath;
    if (buf.size() != 0) {
       fileCacheStream << buf;
    }
    fileCacheStream.close();

    unlink(this->fileCachePath.c_str());
    if (rename(tmp_fileCachePath.c_str(), this->fileCachePath.c_str()) != 0) {
      std::cout << red << "rename fail" << reset << std::endl;
    }
    */

    statusCache[this->relativePath] = make_tuple(this->hash, this->dirtyBit, this->clock);
    fsync(fileno(fp));
    return 0;
  }

  // fsync update cache into local cache file.
  int commitStatusCache() {
    return 0;

    cout << "begin commitStatusCache" << endl;
    std::string tmp_statusCachePath = statusCachePath + ".TMP";

    std::ofstream tmp_cache_file(tmp_statusCachePath.c_str(), std::ofstream::out);
    // if (!tmp_cache_file.is_open())
    //   throw "Error @commitStatusCache: openning statusCachePath: " + statusCachePath;
    cout << "tmp_cache_file opened" << endl;
    for (auto i = this->statusCache.begin(); i != this->statusCache.end(); i++) {
      std::string hash_;
      int dirtyBit_;
      int clock_;
      tie(hash_, dirtyBit_, clock_) = i->second;
      tmp_cache_file << i->first << ";" << hash_ << ";" << std::to_string(dirtyBit_) << ";" << std::to_string(clock_) << "\n";
    }
    tmp_cache_file.close();
    tmp_cache_file.flush();  // TODO: c++ implementation doesn't call fsync necessarily
    unlink(this->fileCachePath.c_str());
    if (rename(tmp_statusCachePath.c_str(), statusCachePath.c_str()) != 0) {
      std::cout << red << "rename fail" << reset << std::endl;
    }
    cout << "end commitStatusCache" << endl;
    return 0;
  }

  int deleteEntry() {
    int ret = unlink(this->fileCachePath.c_str());
    if (ret != 0)
      return -errno;

    this->statusCache.erase(this->relativePath);
    //this->commitStatusCache();

    return ret;
  }

 public:
  std::string relativePath;
  std::string fileCachePath;
  std::string hash;
  int dirtyBit;
  int clock;
  //                                        hash, dirtybit, logical clock
  static std::unordered_map<std::string, std::tuple<std::string, int, int>> statusCache;  // in-memory copy from the statusCachePath contents

  // static members
  static std::unordered_map<std::string, std::tuple<std::string, int, int>> getStatusCache();
  std::string getPathHash(const std::string& path);
};
std::unordered_map<std::string, std::tuple<std::string, int, int>> Cache::statusCache;

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
  statusCacheStream.close();
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
