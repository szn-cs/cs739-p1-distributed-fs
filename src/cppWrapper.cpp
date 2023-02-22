/**
 * AFS Handlers: direct mapping from each of the UnreliableFS functions
 * 
*/

#include "./cppWrapper.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <termcolor/termcolor.hpp>

#ifdef __cplusplus

using namespace std;
using termcolor::reset, termcolor::yellow;

static AFS_Client* grpcClient;
static std::string cacheDirectory;
static std::string cacheFile;
static std::string fsMountPath;
static std::string fsRootPath;

#include "./Cache.cpp"
#include "./Utility.cpp"

extern "C" {
#endif

int cppWrapper_initialize(char* serverAddress, char* _cacheDirectory, char* argv[], char* _fsRootPath) {
  grpcClient = new AFS_Client(grpc::CreateChannel(serverAddress, grpc::InsecureChannelCredentials()));
  cacheDirectory = _cacheDirectory;
  // TODO: create cache direcotry.
  fsMountPath = argv[1];
  fsRootPath = _fsRootPath;
  cacheFile = cacheDirectory + "cache_file.txt";
  std::cout << "⚫ cppWrapper initialized" << std::endl;
  std::cout << "⚫ cacheDirectory path: " << cacheDirectory << std::endl;
  std::cout << "⚫ cacheFile path: " << cacheFile << std::endl;
  std::cout << "⚫ serverAddress path: " << serverAddress << std::endl;
  std::cout << "@cppWrapper_initialize fsMountPath: " << fsMountPath << std::endl;
  std::cout << "@cppWrapper_initialize fsRootPath: " << fsRootPath << endl;

  return 0;
}

// ----------------------------------------------------------------------------
/** Mappings of FUSE to AFS handler logic
 * Main calls should be supported (check unreliablefs.c mapping)

** FUSE functions:
		fuse→open() 
		fuse→release() 
		fuse→readdir() 
		fuse→truncate() 
		fuse→fsync() 
		fuse→mknod() 
		fuse→getattr() 
		fuse→mkdir() 
		fuse→unlink() 
		fuse→read() 
		fuse→write() 
		fuse→rmdir() 
  
** POSIX→FUSE mapping:  FUSE operations that get triggered for each of the POSIX calls
		- open(): fuse→getattr(), fuse→open()
		- close(): fuse→release()
		- creat(): fuse→mknod()
		- unlink(): fuse→getattr(), fuse→unlink()
		- mkdir(): fuse→mkdir()
		- rmdir(): fuse→rmdir()
		- read(), pread(): fuse→read()
		- write(), pwrite(): fuse→write(), fuse→truncate()
		- stat(): fuse→getattr()
		- fsync(): fuse→fsync()
		- readdir(): fuse→readdir()


 * TODO: remove unnecessary platform specific implementations


# client side functions:

- [ ] ls
- [ ] stat
- [ ] rmdir
- [ ] mkdir
- [ ] touch(create)
- [ ] read
- [ ] write
- [ ] unlink

*/

int cppWrapper_lstat(const char* path, struct stat* buf) {
  std::cout << yellow << "\ncppWrapper_lstat" << reset << std::endl;

  path = Utility::constructRelativePath(path).c_str();

  memset(buf, 0, sizeof(struct stat));
  if (lstat(path, buf) == -1) return -errno;

  return 0;
}

int cppWrapper_getattr(const char* path, struct stat* buf) {
  std::cout << yellow << "cppWrapper_getattr" << reset << std::endl;
  std::cout << yellow << "before: " << path << std::endl;
  path = Utility::constructRelativePath(path).c_str();
  std::cout << yellow << "after: " << path << reset << std::endl;
  try {
    int errornum;

    std::memset(buf, 0, sizeof(struct stat));

    int ret = grpcClient->GetAttribute(path, buf, errornum);
    if (ret == -1) return -errornum;
    return 0;
  } catch (...) {
    cout << "⚫ fallback to original implementation" << endl;
    // original:
    memset(buf, 0, sizeof(struct stat));
    if (lstat(path, buf) == -1) return -errno;
    return 0;
  }
}

int cppWrapper_readlink(const char* path, char* buf, size_t bufsiz) {
  std::cout << yellow << "\ncppWrapper_readlink" << reset << std::endl;

  path = Utility::constructRelativePath(path).c_str();

  int ret = readlink(path, buf, bufsiz);
  if (ret == -1) {
    return -errno;
  }
  buf[ret] = 0;

  return 0;
}

int cppWrapper_mknod(const char* path, mode_t mode, dev_t dev) {
  std::cout << yellow << "\ncppWrapper_mknod" << reset << std::endl;

  path = Utility::constructRelativePath(path).c_str();

  int ret = mknod(path, mode, dev);
  if (ret == -1) {
    return -errno;
  }

  return 0;
}

int cppWrapper_mkdir(const char* path, mode_t mode) {
  std::cout << yellow << "\ncppWrapper_mkdir" << reset << std::endl;

  path = Utility::constructRelativePath(path).c_str();

  int errornum;
  int ret = grpcClient->MakeDirectory(path, mode, errornum);
  if (ret == -1) {
    return -errornum;
  }
  return 0;
}

int cppWrapper_unlink(const char* path) {
  std::cout << yellow << "\ncppWrapper_unlink" << reset << std::endl;

  std::string _path = Utility::constructRelativePath(path);
  int ret;
  // delete server
  ret = grpcClient->Unlink(_path);
  // delete local
  std::string path_str(_path);
  std::unordered_map<std::string, std::string> cache = Cache::get_local_cache(cacheFile);
  std::string sha_path = Cache::get_hash_path(path_str);
  std::string local_cache_file = fsRootPath + "/" + sha_path;

  ret = unlink(local_cache_file.c_str());
  if (ret == -1) {
    return -errno;
  }

  cache.erase(_path);
  Cache::fsync_cache(cacheFile, cache);

  return 0;
}

int cppWrapper_rmdir(const char* path) {
  std::cout << yellow << "\ncppWrapper_rmdir" << reset << std::endl;

  path = Utility::constructRelativePath(path).c_str();

  int ret = rmdir(path);
  if (ret == -1) {
    return -errno;
  }

  return 0;
}

int cppWrapper_symlink(const char* target, const char* linkpath) {
  std::cout << yellow << "\ncppWrapper_symlink" << reset << std::endl;

  int ret = symlink(target, linkpath);
  if (ret == -1) {
    return -errno;
  }

  return 0;
}

int cppWrapper_rename(const char* oldpath, const char* newpath) {
  std::cout << yellow << "\ncppWrapper_rename" << reset << std::endl;

  int ret = rename(oldpath, newpath);
  if (ret == -1) {
    return -errno;
  }

  return 0;
}

int cppWrapper_link(const char* oldpath, const char* newpath) {
  std::cout << yellow << "\ncppWrapper_link" << reset << std::endl;

  int ret = link(oldpath, newpath);
  if (ret < 0) {
    return -errno;
  }

  return 0;
}

int cppWrapper_chmod(const char* path, mode_t mode) {
  std::cout << yellow << "\ncppWrapper_chmod" << reset << std::endl;

  path = Utility::constructRelativePath(path).c_str();

  int ret = chmod(path, mode);
  if (ret < 0) {
    return -errno;
  }

  return 0;
}

int cppWrapper_chown(const char* path, uid_t owner, gid_t group) {
  std::cout << yellow << "\ncppWrapper_chown" << reset << std::endl;

  path = Utility::constructRelativePath(path).c_str();

  int ret = chown(path, owner, group);
  if (ret == -1) {
    return -errno;
  }

  return 0;
}

int cppWrapper_truncate(const char* path, off_t length) {
  std::cout << yellow << "\ncppWrapper_truncate" << reset << std::endl;

  path = Utility::constructRelativePath(path).c_str();

  int ret = truncate(path, length);
  if (ret == -1) {
    return -errno;
  }

  return 0;
}

int cppWrapper_open(const char* path, struct fuse_file_info* fi) {
  std::cout << yellow << "\ncppWrapper_open" << reset << std::endl;

  const char* _path = Utility::constructRelativePath(path).c_str();

  std::string local_cache_file(path);
  std::string path_str(_path);
  std::unordered_map<std::string, std::string> cache = Cache::get_local_cache(cacheFile);
  std::string sha_path = Cache::get_hash_path(path_str);
  local_cache_file = fsRootPath + "/" + sha_path;

  int ret;
  long timestamp;

  if (cache.find(path_str) == cache.end()) {
    // path not exist in the cache
    ret = grpcClient->OpenFile(path_str, O_RDWR | O_CREAT | S_IRWXU, timestamp);
    if (ret != 0) return ret;
    int numBytes;
    std::string buf;
    ret = grpcClient->ReadFile(path_str, numBytes, buf, timestamp);
    if (ret != 0) return ret;

    Cache::fsync_file(local_cache_file, buf);

    cache.insert(std::pair<std::string, std::string>(path_str, sha_path));
    Cache::fsync_cache(cacheFile, cache);
  }
  // path exist, then check version, fetch updated data
  // grpcClient->GetAttribute()
  //
  // open local cache file
  // ret = open(path, fi->flags);
  ret = open(local_cache_file.c_str(), fi->flags);
  if (ret == -1) return -errno;

  fi->fh = ret;

  return 0;
}

int cppWrapper_create(const char* path, mode_t mode, struct fuse_file_info* fi) {
  std::cout << termcolor::yellow << "\ncppWrapper_create" << termcolor::reset << std::endl;
  // trigger server create file
  // -> download data
  const char* _path = Utility::constructRelativePath(path).c_str();

  std::string local_cache_file(path);
  std::string path_str(_path);
  std::unordered_map<std::string, std::string> cache = Cache::get_local_cache(cacheFile);
  std::string sha_path = Cache::get_hash_path(path_str);
  local_cache_file = fsRootPath + "/" + sha_path;

  int ret;
  long timestamp;

  if (cache.find(path_str) == cache.end()) {
    // path not exist in the cache
    ret = grpcClient->OpenFile(path_str, O_RDWR | O_CREAT, timestamp);
    if (ret != 0) return ret;
    int numBytes;
    std::string buf;
    ret = grpcClient->ReadFile(path_str, numBytes, buf, timestamp);
    if (ret != 0) return ret;

    Cache::fsync_file(local_cache_file, buf);
    // std::cout << "---------------------------------------fsync_file" << std::endl;
    cache.insert(std::pair<std::string, std::string>(path_str, sha_path));
    Cache::fsync_cache(cacheFile, cache);
  }
  //  path exist, then check version, fetch updated data
  //  grpcClientInstance->clientGetAttr()
  //
  //  open local cache file

  ret = open(local_cache_file.c_str(), fi->flags, S_IRWXG | S_IRWXO | S_IRWXU);
  // std::cout << "---------------------------------------_file" << std::endl;
  if (ret == -1) return -errno;

  fi->fh = ret;

  return 0;
  //
  // _path = Utility::constructRelativePath(path).c_str();

  // int ret = open(path, fi->flags, mode);
  // if (ret == -1) {
  //   return -errno;
  // }
  // fi->fh = ret;

  // return 0;
}

int cppWrapper_read(const char* path, char* buf, size_t size, off_t offset, struct fuse_file_info* fi) {
  std::cout << yellow << "\ncppWrapper_read" << reset << std::endl;

  path = Utility::constructRelativePath(path).c_str();

  // std::string local_cache_dir(cacheDirectory);
  // std::string path_str(path);
  int ret, fd;
  int free_mark = 0;

  if (fi == NULL) {
    fi = new fuse_file_info();
    fi->flags = O_RDONLY;
    fd = cppWrapper_open(path, fi);
    free_mark = 1;
  } else {
    fd = fi->fh;
  }

  if (fd == -1) {
    return -errno;
  }

  ret = pread(fd, buf, size, offset);
  if (ret == -1) {
    ret = -errno;
  }

  // if (fi == NULL) {
  if (free_mark == 1) {
    delete fi;
    close(fd);
  }

  return 0;
}

int cppWrapper_write(const char* path, const char* buf, size_t size, off_t offset, struct fuse_file_info* fi) {
  std::cout << yellow << "\ncppWrapper_write" << reset << std::endl;

  const char* _path = Utility::constructRelativePath(path).c_str();

  std::string local_cache_dir(cacheDirectory);

  int ret, fd;
  int free_mark = 0;

  if (fi == NULL) {
    std::cout << "fi == NULL" << std::endl;
    fi = new fuse_file_info();
    fi->flags = O_WRONLY;
    fd = cppWrapper_open(_path, fi);
    free_mark = 1;
  } else {
    fd = fi->fh;
  }

  if (fd == -1) {
    return -errno;
  }
  std::cout << "cppWrapper_write buf: " << buf << " size: " << size
            << std::endl;
  ret = pwrite(fd, buf, size, offset);
  if (ret == -1) {
    ret = -errno;
  }

  // if (fi == NULL) {
  if (free_mark == 1) {
    delete fi;
    close(fd);
  }

  /*
  int fd;
  (void)fi;
  if (fi == NULL) {
    fd = open(path, O_WRONLY);
  } else {
    fd = fi->fh;
  }

  if (fd == -1) {
    return -errno;
  }

  int ret = pwrite(fd, buf, size, offset);
  if (ret == -1) {
    ret = -errno;
  }

  if (fi == NULL) {
    close(fd);
  }
  */
  return 0;
}

int cppWrapper_statfs(const char* path, struct statvfs* buf) {
  std::cout << yellow << "\ncppWrapper_statfs" << reset << std::endl;

  const char* _path = Utility::constructRelativePath(path).c_str();

  int ret = statvfs(_path, buf);
  if (ret == -1) {
    return -errno;
  }

  return 0;
}

int cppWrapper_flush(const char* path, struct fuse_file_info* fi) {
  std::cout << yellow << "\ncppWrapper_flush" << reset << std::endl;

  path = Utility::constructRelativePath(path).c_str();

  int ret = close(dup(fi->fh));
  if (ret == -1) {
    return -errno;
  }

  return 0;
}

int cppWrapper_release(const char* path, struct fuse_file_info* fi) {
  std::cout << yellow << "\ncppWrapper_release" << reset << std::endl;

  std::string _path = Utility::constructRelativePath(path).c_str();

  int ret;
  int numOfBytes;
  long timestamp;
  std::string path_str(_path);

  ret = close(fi->fh);
  if (ret == -1) {
    return -errno;
  }

  std::string local_cache_dir(fsRootPath);
  std::unordered_map<std::string, std::string> cache = Cache::get_local_cache(cacheFile);
  std::string sha_path = Cache::get_hash_path(path_str);
  std::string local_cache_file = local_cache_dir + "/" + sha_path;
  std::ifstream is;
  is.open(local_cache_file, std::ios::binary | std::ios::ate | std::ios::in | std::ios::out | std::ios::app);
  is.seekg(0, is.end);
  int length = (int)is.tellg() > 0 ? (int)is.tellg() : 0;

  is.seekg(0, is.beg);
  if (length >= 0) {
    std::string buf(length, '\0');
    is.read(&buf[0], length);
    ret = grpcClient->WriteFile(_path, buf, length, 0, numOfBytes, timestamp);
  }
  is.close();
  return 0;
}

int cppWrapper_fsync(const char* path, int datasync, struct fuse_file_info* fi) {
  std::cout << yellow << "\ncppWrapper_fsync" << reset << std::endl;

  path = Utility::constructRelativePath(path).c_str();

  int ret;
  if (datasync) {
    ret = fdatasync(fi->fh);
    if (ret == -1) {
      return -errno;
    }
  } else {
    ret = fsync(fi->fh);
    if (ret == -1) {
      return -errno;
    }
  }

  return 0;
}

#ifdef HAVE_XATTR
int cppWrapper_setxattr(const char* path, const char* name, const char* value, size_t size, int flags) {
  std::cout << yellow << "\ncppWrapper_setxattr" << reset << std::endl;

  path = Utility::constructRelativePath(path).c_str();

  int ret;
#ifdef __APPLE__
  ret = setxattr(path, name, value, size, 0, flags);
#else
  ret = setxattr(path, name, value, size, flags);
#endif /* __APPLE__ */
  if (ret == -1) {
    return -errno;
  }

  return 0;
}

int cppWrapper_getxattr(const char* path, const char* name, char* value, size_t size) {
  std::cout << yellow << "\ncppWrapper_getxattr" << reset << std::endl;

  path = Utility::constructRelativePath(path).c_str();

  int ret;
#ifdef __APPLE__
  ret = getxattr(path, name, value, size, 0, XATTR_NOFOLLOW);
#else
  ret = getxattr(path, name, value, size);
#endif /* __APPLE__ */
  if (ret == -1) {
    return -errno;
  }

  return 0;
}

int cppWrapper_listxattr(const char* path, char* list, size_t size) {
  std::cout << yellow << "\ncppWrapper_listxattr" << reset << std::endl;

  path = Utility::constructRelativePath(path).c_str();

  int ret;
#ifdef __APPLE__
  ret = listxattr(path, list, size, XATTR_NOFOLLOW);
#else
  ret = listxattr(path, list, size);
#endif /* __APPLE__ */
  if (ret == -1) {
    return -errno;
  }

  return 0;
}

int cppWrapper_removexattr(const char* path, const char* name) {
  std::cout << yellow << "\ncppWrapper_removexattr" << reset << std::endl;

  path = Utility::constructRelativePath(path).c_str();

  int ret;
#ifdef __APPLE__
  ret = removexattr(path, name, XATTR_NOFOLLOW);
#else
  ret = removexattr(path, name);
#endif /* __APPLE__ */
  if (ret == -1) {
    return -errno;
  }

  return 0;
}
#endif /* HAVE_XATTR */

int cppWrapper_opendir(const char* path, struct fuse_file_info* fi) {
  std::cout << yellow << "\ncppWrapper_opendir" << reset << std::endl;

  path = Utility::constructRelativePath(path).c_str();

  DIR* dir = opendir(path);

  if (!dir) {
    return -errno;
  }
  fi->fh = (int64_t)dir;

  return 0;
}

int cppWrapper_readdir(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* fi) {
  std::cout << yellow << "\ncppWrapper_readdir" << reset << std::endl;

  path = Utility::constructRelativePath(path).c_str();
  /*
  DIR* dp = opendir(path);
  if (dp == NULL) {
    return -errno;
  }
  struct dirent* de;

  (void)offset;
  (void)fi;

  while ((de = readdir(dp)) != NULL) {
    struct stat st;
    memset(&st, 0, sizeof(st));
    st.st_ino = de->d_ino;
    st.st_mode = de->d_type << 12;
    if (filler(buf, de->d_name, &st, 0)) break;
  }
  closedir(dp);
  */
  struct dirent de;
  int errornum = 0;
  std::vector<std::string> results;
  int ret = grpcClient->ReadDirectory(path, errornum, results);
  if (ret != 0) {
    return -errornum;
  }

  for (auto result : results) {
    struct stat st;
    memset(&st, 0, sizeof(st));
    memcpy(&de, &result[0], result.size());
    st.st_ino = de.d_ino;
    st.st_mode = de.d_type << 12;
    std::cout << "de.d_name: " << de.d_name << std::endl;
    if (filler(buf, de.d_name, &st, 0)) break;
  }
  // closedir(dp);
  return 0;
}

int cppWrapper_releasedir(const char* path, struct fuse_file_info* fi) {
  std::cout << yellow << "\ncppWrapper_releasedir" << reset << std::endl;

  path = Utility::constructRelativePath(path).c_str();

  DIR* dir = (DIR*)fi->fh;

  int ret = closedir(dir);
  if (ret == -1) {
    return -errno;
  }

  return 0;
}

int cppWrapper_fsyncdir(const char* path, int datasync, struct fuse_file_info* fi) {
  std::cout << yellow << "\ncppWrapper_fsyncdir" << reset << std::endl;

  path = Utility::constructRelativePath(path).c_str();

  int ret;
  // TODO:
  DIR* dir = opendir(path);
  if (!dir) {
    return -errno;
  }

  if (datasync) {
    ret = fdatasync(dirfd(dir));
    if (ret == -1) {
      return -errno;
    }
  } else {
    ret = fsync(dirfd(dir));
    std::cout << yellow << "before: " << path << std::endl;
    path = Utility::constructRelativePath(path).c_str();
    std::cout << yellow << "after: " << path << reset << std::endl;
  }
  closedir(dir);
  return 0;
}

int cppWrapper_access(const char* path, int mode) {
  std::cout << yellow << "\ncppWrapper_access" << reset << std::endl;
  std::cout << "path: " << path << std::endl;
  std::string _path = Utility::constructRelativePath(path);

  std::string local_cache_dir(fsRootPath);
  std::unordered_map<std::string, std::string> cache = Cache::get_local_cache(cacheFile);
  std::string sha_path = Cache::get_hash_path(_path);
  std::string local_cache_file = local_cache_dir + "/" + sha_path;

  int ret = access(local_cache_file.c_str(), mode) || access(path, mode);
  if (ret == -1) {
    return -errno;
  }

  return 0;
}

int cppWrapper_ftruncate(const char* path, off_t length, struct fuse_file_info* fi) {
  std::cout << yellow << "\ncppWrapper_ftruncate" << reset << std::endl;

  path = Utility::constructRelativePath(path).c_str();

  int ret = truncate(path, length);
  if (ret == -1) {
    return -errno;
  }

  return 0;
}

int cppWrapper_fgetattr(const char* path, struct stat* buf, struct fuse_file_info* fi) {
  std::cout << yellow << "\ncppWrapper_fgetattr" << reset << std::endl;

  path = Utility::constructRelativePath(path).c_str();

  int ret = fstat((int)fi->fh, buf);
  if (ret == -1) {
    return -errno;
  }

  return 0;
}

int cppWrapper_lock(const char* path, struct fuse_file_info* fi, int cmd, struct flock* fl) {
  std::cout << yellow << "\ncppWrapper_lock" << reset << std::endl;

  path = Utility::constructRelativePath(path).c_str();

  int ret = fcntl((int)fi->fh, cmd, fl);
  if (ret == -1) {
    return -errno;
  }

  return 0;
}

#if !defined(__OpenBSD__)
int cppWrapper_ioctl(const char* path, int cmd, void* arg, struct fuse_file_info* fi, unsigned int flags, void* data) {
  std::cout << yellow << "\ncppWrapper_ioctl" << reset << std::endl;

  path = Utility::constructRelativePath(path).c_str();

  int ret = ioctl(fi->fh, cmd, arg);
  if (ret == -1) {
    return -errno;
  }

  return 0;
}
#endif /* __OpenBSD__ */

#ifdef HAVE_FLOCK
int cppWrapper_flock(const char* path, struct fuse_file_info* fi, int op) {
  std::cout << yellow << "\ncppWrapper_flock" << reset << std::endl;

  path = Utility::constructRelativePath(path).c_str();

  int ret = flock(((int)fi->fh), op);
  if (ret == -1) {
    return -errno;
  }

  return 0;
}
#endif /* HAVE_FLOCK */

#ifdef HAVE_FALLOCATE
// TODO:
int cppWrapper_fallocate(const char* path, int mode, off_t offset, off_t len, struct fuse_file_info* fi) {
  std::cout << yellow << "\ncppWrapper_fallocate" << reset << std::endl;

  path = Utility::constructRelativePath(path).c_str();

  int ret;
  int fd;
  (void)fi;

  if (mode) {
    return -EOPNOTSUPP;
  }

  if (fi == NULL) {
    fd = open(path, O_WRONLY);
  } else {
    fd = fi->fh;
  }

  if (fd == -1) {
    return -errno;
  }

  ret = fallocate((int)fi->fh, mode, offset, len);
  if (ret == -1) {
    return -errno;
  }

  if (fi == NULL) {
    close(fd);
  }

  return 0;
}
#endif /* HAVE_FALLOCATE */

#ifdef HAVE_UTIMENSAT
int cppWrapper_utimens(const char* path, const struct timespec ts[2]) {
  std::cout << yellow << "\ncppWrapper_utimens" << reset << std::endl;

  path = Utility::constructRelativePath(path).c_str();

  /* don't use utime/utimes since they follow symlinks */
  // TODO:
  int ret = utimensat(0, path, ts, AT_SYMLINK_NOFOLLOW);
  if (ret == -1) {
    return -errno;
  }

  return 0;
}
#endif /* HAVE_UTIMENSAT */

/** // testing:
  int main() {
    // std::unordered_map<std::string, std::string> Cache::get_local_cache(const
    // std::string& path) int Cache::fsync_cache(std::string& path,
    // std::unordered_map<std::string, std::string> cache) std::string
    // hash_path(const std::string& path)
    // std::string test_cache_path("./test_cache.txt");
    // std::unordered_map<std::string, std::string> tmp_cache =
    //     Cache::get_local_cache(test_cache_path);
    // std::string test_path = "./test_pathh";
    // std::string test_hash = Cache::get_hash_path(test_path);
    // std::cout << test_path << " " << test_hash << std::endl;
    // tmp_cache.insert(std::pair<std::string, std::string>(test_path,
    // test_hash)); Cache::fsync_cache(test_cache_path, tmp_cache);
    // std::string AddrPort_ = "localhost:50051";
    // std::string cacheDirectory_ = "/tmp/cache/";

    //  1.) with fh (data in local cache)
    //  2.) without fh
    //                -> (data in/not in local cache)   -> wrapper_open()

    struct fuse_file_info t1;
    struct fuse_file_info t2;
    t1.flags = O_RDWR;
    t2.flags = O_RDWR;
    struct fuse_file_info* p1 = &t1;
    struct fuse_file_info* p2 = &t2;
    std::string buf = "I want to go home.\0";
    char* pbuf = &buf[0];
    // char buf1[20];
    // buf1[0] = '\0';
    // char buf2[20];
    // buf2[0] = '\0';
    // cppWrapper_open("localhost:50051", "/tmp/cache/", "test2.txt", p1);
    // cppWrapper_read("localhost:50051", "/tmp/cache/", "test2.txt", buf1, 20,
    5,
    //                 p1);
    // std::cout << "with fh " << buf1 << std::endl;
    // cppWrapper_read("localhost:50051", "/tmp/cache/", "test1.txt", buf2, 20,
    5,
    //                 NULL);
    // std::cout << "without fh " << buf2 << std::endl;
    cppWrapper_open("localhost:50051", "/tmp/cache/", "test1.txt", p1);

    cppWrapper_write("localhost:50051", "/tmp/cache/", "test1.txt", pbuf,
                    buf.size(), 0, p1);

    cppWrapper_write("localhost:50051", "/tmp/cache/", "test2.txt", pbuf,
                    buf.size(), 0, NULL);
    cppWrapper_release("localhost:50051", "/tmp/cache/", "test1.txt", p1);
    return 0;
  }
*/

#ifdef __cplusplus
}
#endif
