#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include <experimental/filesystem>
#include <iostream>
#include <list>
#include <unordered_map>
using namespace std;
namespace fs = std::experimental::filesystem;
#define CAP 20

class LRUCache {
  size_t m_capacity;
  string mountPoint;
  string cacheDir;
  unordered_map<string, list<pair<string, long>>::iterator>
      m_map;  // m_map_iter->first: key, m_map_iter->second: list iterator;
  list<pair<string, long>>
      m_list;  // m_list_iter->first: key, m_list_iter->second: value;
 public:
  LRUCache(size_t capacity) : m_capacity(capacity) {}
  void printCache() {
    for (auto itr = m_map.begin(); itr != m_map.end(); itr++)
      cout << itr->first << "\n";
  }
  bool exists(string key) {
    auto found_iter = m_map.find(key);
    if (found_iter == m_map.end())  // key doesn't exist
      return false;
    return true;
  }
  long get(string key) {
    auto found_iter = m_map.find(key);
    // if (found_iter == m_map.end()) // key doesn't exist
    //     return struct stat(0);
    m_list.splice(
        m_list.begin(), m_list,
        found_iter->second);  // move the node corresponding to key to front
    return found_iter->second->second;  // return value of the node
  }
  void set(string key, long value) {
    auto found_iter = m_map.find(key);
    if (found_iter != m_map.end())  // key exists
    {
      m_list.splice(
          m_list.begin(), m_list,
          found_iter->second);  // move the node corresponding to key to front
      found_iter->second->second = value;  // update value of the node
      return;
    }
    if (m_map.size() == m_capacity)  // reached capacity
    {
      string key_to_del = m_list.back().first;
      m_list.pop_back();        // remove node in list;
      m_map.erase(key_to_del);  // remove key in map
      int res = deleteCacheEntry(key_to_del);
    }
    m_list.emplace_front(key, value);  // create new node in list
    m_map[key] = m_list.begin();  // create correspondence between key and node
  }

  int deleteCacheEntry(string path) {
    std::error_code errorCode;
    if (!fs::remove(path, errorCode)) {
      std::cout << errorCode.message() << std::endl;
      return errorCode.value();
    }
    return 0;
  }
  void setMountPoint(string mnt) { mountPoint = mnt; }
  void setCachePath(string cache) { cacheDir = cache; }
  string getMountPoint() { return mountPoint; }
  string getCachePath() { return cacheDir; }
  string getFilePathCache(string path) { return cacheDir + path; }
};
