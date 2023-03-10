#pragma once

#include <algorithm>
// #include <boost/filesystem.hpp>
#include <filesystem>
#include <iostream>

using namespace std;
namespace fs = std::filesystem;
// namespace _fs = boost::filesystem;

extern std::string fsMountPath;
extern std::string fsRootPath;

class Utility {
 public:
  // construct a relative path
  static std::string constructRelativePath(std::string path) {
    string relativePath;
    if (!(fs::path(path)).is_absolute())
      path = fs::canonical(path);

    relativePath = (fs::relative(path, fsRootPath)).generic_string();

    // trim slashes or `./` from trailing/leading ends for hashing to be consistent between server and client
    // TODO: for caching
    // cout << "📃 relative path has dots ?: " << relativePath << " from path: " << path << endl;

    return relativePath;
  }

  static std::string concatenatePath(std::string base, std::string path) {
    fs::path concatenated;
    fs::path _base(base), _path(path);
    concatenated = fs::absolute(_base / _path);

    return concatenated.generic_string();
  }
};
