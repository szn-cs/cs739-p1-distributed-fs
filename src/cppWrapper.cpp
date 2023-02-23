/**
 * AFS Handlers: direct mapping from each of the UnreliableFS functions
 * 
*/
#include "./cppWrapper.h"

#ifdef __cplusplus

namespace fs = std::filesystem;
using namespace std;
using termcolor::reset, termcolor::yellow, termcolor::red, termcolor::blue;

static GRPC_Client* grpcClient;
std::string cacheDirectory;
std::string statusCachePath;
std::string fsMountPath;
std::string fsRootPath;

extern "C" {
#endif

/** Mappings of FUSE to AFS handler logic
 * Main calls should be supported (check unreliablefs.c mapping)

** FUSE functions:
		[x] fuse→getattr() 
		[x!] fuse→open()  // TODO- cache validation logic
		[x] fuse→mkdir() 
		[x] fuse→rmdir()
		[x] fuse→unlink() 
		[ ] fuse→release() 
		[ ] fuse→readdir() 
		[ ] fuse→truncate() 
		[ ] fuse→fsync() 
		[ ] fuse→mknod() 
		[ ] fuse→read() 
		[ ] fuse→write() 

* check manual pages for POSIX functions details https://linux.die.net/man/2/
** POSIX→FUSE mapping:  FUSE operations that get triggered for each of the POSIX calls
		[x] open():             fuse→getattr(), fuse→open()
		[ ] close():            fuse→release()
		[ ] creat():            fuse→mknod()
		[x] unlink():           fuse→getattr(), fuse→unlink()
		[x] mkdir():            fuse→mkdir()
		[x] rmdir():            fuse→rmdir()
		[ ] read(), pread():    fuse→read()
		[ ] write(), pwrite():  fuse→write(), fuse→truncate()
    // https://linux.die.net/man/2/lstat
		[x] stat():             fuse→getattr()
    [ ] fsync():            fuse→fsync()
		[ ] readdir():          fuse→readdir()


 * TODO: remove unnecessary platform specific implementations

*/

int cppWrapper_getattr(const char* path, struct stat* buf) {
  std::cout << yellow << "cppWrapper_getattr" << reset << std::endl;

  int errornum, r;
  path = Utility::constructRelativePath(path).c_str();

  try {
    std::memset(buf, 0, sizeof(struct stat));

    r = grpcClient->getFileAttributes(path, buf, errornum);

    return (r == -1) ? -errornum : 0;
  } catch (...) {
    goto Original;
  }

Original:
  cout << red << "cppWrapper_getattr fallback to original implementation" << reset << endl;
  memset(buf, 0, sizeof(struct stat));
  if (lstat(path, buf) == -1) return -errno;
  return 0;
}

int cppWrapper_lstat(const char* path, struct stat* buf) {
  std::cout << yellow << "\ncppWrapper_lstat —forward→ cppWrapper_getattr" << reset << std::endl;
  return cppWrapper_getattr(path, buf);
}

int cppWrapper_open(const char* path, struct fuse_file_info* fi) {
  std::cout << yellow << "\ncppWrapper_open" << reset << std::endl;
  int ret;
  string _path = Utility::constructRelativePath(path);

  Cache c(_path);

  // check if cache entry for the path exists
  // TODO: and check is cache valid or stale
  if (c.isCacheEntry() && true /* if valid cache **/)
    goto OpenCachedFile;

FetchToCache : {
  long timestamp;
  int numBytes;
  std::string buf;

  // fetch file
  ret = grpcClient->getFileContents(_path, numBytes, buf, timestamp);
  if (ret != 0) return ret;

  c.commitFileCache(buf);
  c.commitStatusCache();
}

OpenCachedFile:  // open local cache file
  ret = open(c.fileCachePath.c_str(), fi->flags);
  if (ret == -1) return -errno;

  fi->fh = ret;

  return 0;

Original:
  ret = open(path, fi->flags);
  if (ret == -1) {
    return -errno;
  }
  fi->fh = ret;
}

int cppWrapper_mkdir(const char* path, mode_t mode) {
  std::cout << yellow << "\ncppWrapper_mkdir" << reset << std::endl;
  path = Utility::constructRelativePath(path).c_str();
  int errornum;

  int ret = grpcClient->createDirectory(path, mode, errornum);
  if (ret == -1)
    return -errornum;

  return 0;
}

int cppWrapper_rmdir(const char* path) {
  std::cout << yellow << "\ncppWrapper_rmdir" << reset << std::endl;
  path = Utility::constructRelativePath(path).c_str();

  int ret = grpcClient->removeDirectory(path);
  if (ret == -1)
    return -errno;

  return 0;
}

int cppWrapper_unlink(const char* path) {
  std::cout << yellow << "\ncppWrapper_unlink" << reset << std::endl;
  int ret;
  string _path = Utility::constructRelativePath(path);

  Cache c(_path);

  // delete on server
  ret = grpcClient->removeFile(_path);
  if (ret != 0) return -errno;

  // delete local
  if (c.isCacheEntry()) {
    ret = c.deleteEntry();
    if (ret != 0)
      return -errno;
  }

  return 0;
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

// trigger server create file -> download data
int cppWrapper_create(const char* path, mode_t mode, struct fuse_file_info* fi) {
  std::cout << termcolor::yellow << "\ncppWrapper_create" << termcolor::reset << std::endl;
  string _path = Utility::constructRelativePath(path);
  int ret;
  long timestamp;

  Cache c(_path);

  // if path not exist in the cache
  // TODO: and check is cache valid or stale
  if (c.isCacheEntry() && true /* if valid cache **/)
    goto OpenCachedFile;

FetchCache : {
  int numBytes;
  std::string buf;
  ret = grpcClient->OpenFile(_path, O_RDWR | O_CREAT, timestamp);
  if (ret != 0) return ret;
  ret = grpcClient->getFileContents(_path, numBytes, buf, timestamp);
  if (ret != 0) return ret;

  c.commitFileCache(buf);
  c.commitStatusCache();
}

OpenCachedFile:
  //  open local cache file
  ret = open(c.fileCachePath.c_str(), fi->flags, S_IRWXG | S_IRWXO | S_IRWXU);
  if (ret == -1) return -errno;

  fi->fh = ret;

  return 0;

Original : {
  int ret = open(path, fi->flags, mode);
  if (ret == -1) {
    return -errno;
  }
  fi->fh = ret;

  return 0;
}
}

int cppWrapper_read(const char* path, char* buf, size_t size, off_t offset, struct fuse_file_info* fi) {
  std::cout << yellow << "\ncppWrapper_read" << reset << std::endl;
  path = Utility::constructRelativePath(path).c_str();

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

  if (fd == -1)
    return -errno;

  std::cout << "cppWrapper_write buf: " << buf << " size: " << size << std::endl;
  ret = pwrite(fd, buf, size, offset);
  if (ret == -1)
    ret = -errno;

  // if (fi == NULL) {
  if (free_mark == 1) {
    delete fi;
    close(fd);
  }

  return 0;

Original : {
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

  ret = pwrite(fd, buf, size, offset);
  if (ret == -1) {
    ret = -errno;
  }

  if (fi == NULL) {
    close(fd);
  }

  return ret;
}
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
  std::ifstream is;

  Cache c(_path);

  // close file locally
  ret = close(fi->fh);
  if (ret == -1)
    return -errno;

  // stream file to server
  is.open(c.fileCachePath, std::ios::binary | std::ios::ate | std::ios::in | std::ios::out | std::ios::app);
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
  int ret = grpcClient->readDirectory(path, errornum, results);
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
  std::string _path = Utility::constructRelativePath(path);

  Cache c(_path);

  int ret = access(c.fileCachePath.c_str(), mode);
  if (ret == -1)
    return -errno;

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

/** ------------------------------------------------------------------------------------
 * AFS initialization functions 
 */
int cppWrapper_initialize(char* serverAddress, char* _cacheDirectory, char* argv[], char* _fsRootPath) {
  grpcClient = new GRPC_Client(grpc::CreateChannel(serverAddress, grpc::InsecureChannelCredentials()));
  cacheDirectory = _cacheDirectory;
  fsMountPath = argv[1];
  fsRootPath = _fsRootPath;

  // create direcotries
  fs::create_directories(cacheDirectory) == true ? cout << blue << "cacheDirectory created" << reset << endl : cout << blue << "cacheDirectory already exists" << reset << endl;
  fs::create_directories(fsMountPath) == true ? cout << blue << "fsMountPath created" << reset << endl : cout << blue << "fsMountPath already exists" << reset << endl;

  statusCachePath = Utility::concatenatePath(cacheDirectory, "cache_file.txt");

  cout << blue << "statusCachePath: " << statusCachePath << reset << endl;
  cout << blue << "serverAddress: " << serverAddress << reset << endl;
  cout << blue << "fsMountPath: " << fsMountPath << reset << endl;
  cout << blue << "fsRootPath: " << fsRootPath << reset << endl;
  cout << yellow << "cppWrapper_initialize complete" << reset << endl;

  return 0;
}

void cppWrapper_createDirectories(char* path) {
  fs::create_directories(path) == true ? cout << blue << path << " directory created" << reset << endl : cout << blue << path << " directory already exists" << reset << endl;
}

/** // testing:
  int main() {
    // std::unordered_map<std::string, std::string> Cache::get_local_cache(const
    // std::string& path) int Cache::commitStatusCache(std::string& path,
    // std::unordered_map<std::string, std::string> cache) std::string
    // hash_path(const std::string& path)
    // std::string test_cache_path("./test_cache.txt");
    // std::unordered_map<std::string, std::string> tmp_cache =
    //     Cache::get_local_cache(test_cache_path);
    // std::string test_path = "./test_pathh";
    // std::string test_hash = Cache::getPathHash(test_path);
    // std::cout << test_path << " " << test_hash << std::endl;
    // tmp_cache.insert(std::pair<std::string, std::string>(test_path,
    // test_hash)); Cache::commitStatusCache(test_cache_path, tmp_cache);
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
