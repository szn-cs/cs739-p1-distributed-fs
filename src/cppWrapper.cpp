/**
 * AFS Handlers: direct mapping from each of the UnreliableFS functions
 *
 */
#include "./cppWrapper.h"

#ifdef __cplusplus

namespace fs = std::filesystem;
using namespace std;
using termcolor::reset, termcolor::yellow, termcolor::red, termcolor::blue, termcolor::cyan;

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
        [x] fuse→mkdir()
        [x] fuse→rmdir()
        [x] fuse→readdir()
        [x] fuse→unlink()
        [x] fuse→open()
        [x!] fuse→read()
        [x!] fuse→release() //TODO dirty bit
        [x!] fuse→write()  // TODO- test further for edge cases

        [ ] fuse→fsync()
        [ ] fuse→truncate()
        [ ] fuse→mknod()

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
        [x] readdir():          fuse→readdir()


 * TODO: remove unnecessary platform specific implementations

*/

int cppWrapper_getattr(const char* path, struct stat* buf) {
  std::cout << blue << "cppWrapper_getattr" << reset << std::endl;
  std::string _path = Utility::constructRelativePath(path);
  std::cout << cyan << "path: " << path << reset << endl;
  int errornum, r;

  std::memset(buf, 0, sizeof(struct stat));

  r = grpcClient->getFileAttributes(_path, buf, errornum);

  return (r == -1) ? -errornum : 0;
}

int cppWrapper_open(const char* path, struct fuse_file_info* fi) {
  std::cout << blue << "\ncppWrapper_open" << reset << std::endl;
  int ret, _r, errornum = 0;
  std::string _path = Utility::constructRelativePath(path);
  struct stat serverAttr;

  Cache c(_path);

  // check is cache valid or stale
  _r = grpcClient->getFileAttributes(_path, &serverAttr, errornum);
  if (_r != 0)
    goto FetchToCache;

  /* if valid cache **/
  if (c.isCacheEntry() && c.isCacheValid(serverAttr))
    goto OpenCachedFile;

FetchToCache : {
  long timestamp;
  int numBytes;
  std::string buf;

  // fetch file
  ret = grpcClient->getFileContents(_path, numBytes, buf, timestamp);
  if (ret != 0) return ret;

  c.commitFileCache(buf);
  //c.commitStatusCache();
}

OpenCachedFile:  // open local cache file
  ret = open(c.fileCachePath.c_str(), fi->flags);
  if (ret == -1) return -errno;

  fi->fh = ret;

  return 0;
}

int cppWrapper_mkdir(const char* path, mode_t mode) {
  std::cout << blue << "\ncppWrapper_mkdir" << reset << std::endl;
  std::string _path = Utility::constructRelativePath(path);
  int errornum;

  int ret = grpcClient->createDirectory(_path, mode, errornum);
  if (ret == -1)
    return -errornum;

  return 0;
}

/** read triggers:
      cppWrapper_getattr
      cppWrapper_open
      cppWrapper_read
      cppWrapper_flush
      cppWrapper_lock
      cppWrapper_release
*/
int cppWrapper_read(const char* path, char* buf, size_t size, off_t offset, struct fuse_file_info* fi) {
  std::cout << blue << "\ncppWrapper_read" << reset << std::endl;
  int ret, fd;
  int free_mark = 0;

  if (fi == NULL) {
    //cout << red << "fh requested from open " << reset << endl;

    fi = new fuse_file_info();
    fi->flags = O_RDONLY;
    ret = cppWrapper_open(path, fi);
    if (ret == -1) ret = -errno;
    fd = fi->fh;
    free_mark = 1;
  } else {
    //cout << red << "fh already given fh " << fi->fh << reset << endl;

    fd = fi->fh;
  }
  if (fd == -1)
    return -errno;

  //cout << red << "read size: " << size << reset << endl;

  ret = pread(fd, buf, size, offset);
  if (ret == -1)
    ret = -errno;

  // if (fi == NULL) {
  if (free_mark == 1) {
    delete fi;
    close(fd);
  }

  return ret;
}

int cppWrapper_rmdir(const char* path) {
  std::cout << blue << "\ncppWrapper_rmdir" << reset << std::endl;
  std::string _path = Utility::constructRelativePath(path);

  int ret = grpcClient->removeDirectory(_path);

  if (ret != 0)
    return -ret;

  return 0;
}

int cppWrapper_unlink(const char* path) {
  std::cout << blue << "\ncppWrapper_unlink" << reset << std::endl;
  int ret;
  std::string _path = Utility::constructRelativePath(path);

  Cache c(_path);

  // delete local
  if (c.isCacheEntry()) {
    ret = c.deleteEntry();
    if (ret != 0)
      return -errno;
  }

  // delete on server
  ret = grpcClient->removeFile(_path);
  if (ret != 0) return -errno;

  return 0;
}

// TODO:
int cppWrapper_mknod(const char* path, mode_t mode, dev_t dev) {
  std::cout << blue << "\ncppWrapper_mknod" << reset << std::endl;
  path = Utility::constructRelativePath(path).c_str();

  int ret = mknod(path, mode, dev);
  if (ret == -1) {
    return -errno;
  }

  return 0;
}

int cppWrapper_rename(const char* oldpath, const char* newpath) {
  std::cout << blue << "\ncppWrapper_rename" << reset << std::endl;

  int ret = rename(oldpath, newpath);
  if (ret == -1) {
    return -errno;
  }

  return 0;
}

int cppWrapper_truncate(const char* path, off_t length) {
  std::cout << blue << "\ncppWrapper_truncate" << reset << std::endl;

  std::string _path = Utility::constructRelativePath(path);
  Cache c(_path);

  int ret = truncate(c.fileCachePath.c_str(), length);
  if (ret == -1) {
    return -errno;
  }

  return 0;
}

// trigger server create file -> download data
int cppWrapper_create(const char* path, mode_t mode, struct fuse_file_info* fi) {
  std::cout << termcolor::blue << "\ncppWrapper_create" << termcolor::reset << std::endl;
  int ret, errornum = 0;
  int consistence = -1;
  long timestamp;
  struct stat serverAttr;

  std::string _path = Utility::constructRelativePath(path);

  Cache c(_path);

  // check is cache valid or stale
  int _r = grpcClient->getFileAttributes(_path, &serverAttr, errornum);
  if (_r != 0)
    goto FetchToCache;

  /* if valid cache **/
  if (c.isCacheEntry() && c.isCacheValid(serverAttr))
    goto OpenCachedFile;

FetchToCache : {
  int numBytes;
  std::string buf;
  ret = grpcClient->OpenFile(_path, O_RDWR | O_CREAT, timestamp);
  std::cout << "ret " << ret << std::endl;
  if (ret != 0) return ret;
  ret = grpcClient->getFileContents(_path, numBytes, buf, timestamp);
  if (ret != 0) return ret;

  c.commitFileCache(buf);
  //c.commitStatusCache();
}

OpenCachedFile: {
  //  open local cache file
  ret = open(c.fileCachePath.c_str(), fi->flags, S_IRWXG | S_IRWXO | S_IRWXU);
  if (ret == -1) return -errno;

  fi->fh = ret;

  return 0;
}
  

Original : {
  // int ret = open(path, 32768, mode);
  int ret = open(path, fi->flags, mode);
  if (ret == -1) {
    return -errno;
  }
  fi->fh = ret;

  return 0;
}
}

int cppWrapper_write(const char* path, const char* buf, size_t size, off_t offset, struct fuse_file_info* fi) {
  std::cout << blue << "\ncppWrapper_write" << reset << std::endl;
  std::string _path = Utility::constructRelativePath(path);

  int ret, fd;
  int free_mark = 0;
  Cache c(_path);
  if (fi == NULL) {
    std::cout << "fi == NULL" << std::endl;
    fi = new fuse_file_info();
    fi->flags = O_WRONLY;
    ret = cppWrapper_open(path, fi);
    if (ret != 0) ret = -errno;
    fd = fi->fh;
    free_mark = 1;
  } else {
    fd = fi->fh;
  }

  if (fd == -1)
    return -errno;

  ret = pwrite(fd, buf, size, offset);
  if (ret == -1)
    ret = -errno;
  // if (fi == NULL) {
  if (free_mark == 1) {
    delete fi;
    close(fd);
  }
  if (!c.isDirty()) c.setDirtyBit();
  return ret;
}

int cppWrapper_flush(const char* path, struct fuse_file_info* fi) {
  std::cout << blue << "\ncppWrapper_flush" << reset << std::endl;
  std::string _path = Utility::constructRelativePath(path);
  int ret;
  int numOfBytes;
  long timestamp;
  std::ifstream is;

  Cache c(_path);

  // close file locally
  if (!c.isDirty()) {
    close(dup(fi->fh));
    return 0;  // by-pass
  }
  ret = close(dup(fi->fh));
  if (ret == -1)
    return -errno;

  // write to server...
  // stream file to server
  is.open(c.fileCachePath.c_str(), std::ios::binary | std::ios::in);
  is.seekg(0, is.end);
  int length = (int)is.tellg() > 0 ? (int)is.tellg() : 0;
  is.seekg(0, is.beg);
  if (length > 0) {
    std::string buf(length, '\0');
    is.read(&buf[0], length);
    ret = grpcClient->putFileContents(_path, buf, length, 0, numOfBytes, timestamp);
  }
  is.close();

  // reset DirtyBit
  c.resetDirtyBit();
  return 0;

Original : 
  ret = close(dup(fi->fh));
  if (ret == -1)
    return -errno;

  return 0;
}

int cppWrapper_release(const char* path, struct fuse_file_info* fi) {
  std::cout << blue << "\ncppWrapper_release" << reset << std::endl;

  std::string _path = Utility::constructRelativePath(path);
  int ret;
  int numOfBytes;
  long timestamp;
  std::ifstream is;

  Cache c(_path);

  // close file locally
  if (!c.isDirty()) {
    close(fi->fh);
    return 0;  // by-pass
  }
  ret = close(fi->fh);
  if (ret == -1)
    return -errno;

  // write to server...
  // stream file to server
  is.open(c.fileCachePath.c_str(), std::ios::binary | std::ios::in);
  is.seekg(0, is.end);
  int length = (int)is.tellg() > 0 ? (int)is.tellg() : 0;
  is.seekg(0, is.beg);
  if (length > 0) {
    std::string buf(length, '\0');
    is.read(&buf[0], length);
    ret = grpcClient->putFileContents(_path, buf, length, 0, numOfBytes, timestamp);
  }
  is.close();

  // reset DirtyBit
  c.resetDirtyBit();
  return 0;


  Original : 
  ret = close(fi->fh);
  if (ret == -1)
    return -errno;

  return 0;


  std::string _path = Utility::constructRelativePath(path);
  
  int numOfBytes;
  long timestamp;
  std::ifstream is;

  Cache c(_path);

  // close file locally
  if (!c.isDirty()) {
    close(fi->fh);
    return 0;  // by-pass
  }
  ret = close(fi->fh);

  if (ret == -1)
    return -errno;

  // write to server...
  // stream file to server
  is.open(c.fileCachePath.c_str(), std::ios::binary | std::ios::in);
  is.seekg(0, is.end);
  int length = (int)is.tellg() > 0 ? (int)is.tellg() : 0;
  is.seekg(0, is.beg);
  if (length > 0) {
    std::string buf(length, '\0');
    std::cout << blue << "length: " << length << std::endl;
    is.read(&buf[0], length);
    ret = grpcClient->putFileContents(_path, buf, length, 0, numOfBytes, timestamp);
  }
  is.close();

  // reset DirtyBit
  c.resetDirtyBit();
  return 0;
  
}

int cppWrapper_fsync(const char* path, int datasync, struct fuse_file_info* fi) {
  std::cout << blue << "\ncppWrapper_fsync" << reset << std::endl;
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

// ✅
int cppWrapper_opendir(const char* path, struct fuse_file_info* fi) {
  std::cout << blue << "\ncppWrapper_opendir" << reset << std::endl;
  return 0;  // by-pass
}

// ✅
int cppWrapper_releasedir(const char* path, struct fuse_file_info* fi) {
  std::cout << blue << "\ncppWrapper_releasedir" << reset << std::endl;
  return 0;  // by-pass
}

// ✅
int cppWrapper_readdir(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* fi) {
  std::cout << blue << "\ncppWrapper_readdir" << reset << std::endl;
  std::string _path = Utility::constructRelativePath(path);
  struct dirent de;
  int errornum = 0;
  std::vector<std::string> results;

  int ret = grpcClient->readDirectory(_path, errornum, results);
  if (ret != 0)
    return -errornum;

  for (auto result : results) {
    struct stat st;
    memset(&st, 0, sizeof(st));
    memcpy(&de, &result[0], result.size());
    st.st_ino = de.d_ino;
    st.st_mode = de.d_type << 12;
    std::cout << "de.d_name: " << de.d_name << std::endl;
    if (filler(buf, de.d_name, &st, 0)) break;
  }

  return 0;
}

int cppWrapper_fsyncdir(const char* path, int datasync, struct fuse_file_info* fi) {
  std::cout << blue << "\ncppWrapper_fsyncdir" << reset << std::endl;
Original:
  int ret;
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
    path = Utility::constructRelativePath(path).c_str();
  }
  closedir(dir);
  return 0;
}

int cppWrapper_access(const char* path, int mode) {
  std::cout << blue << "\ncppWrapper_access" << reset << std::endl;
  return 0;  // by-pass access control
}

int cppWrapper_ftruncate(const char* path, off_t length, struct fuse_file_info* fi) {
  std::cout << blue << "\ncppWrapper_ftruncate" << reset << std::endl;
Original:
  int ret = truncate(path, length);
  if (ret == -1) {
    return -errno;
  }

  return 0;
}

int cppWrapper_fgetattr(const char* path, struct stat* buf, struct fuse_file_info* fi) {
  std::cout << blue << "\ncppWrapper_fgetattr" << reset << std::endl;
Original:
  int ret = fstat((int)fi->fh, buf);
  if (ret == -1) {
    return -errno;
  }

  return 0;
}

int cppWrapper_lock(const char* path, struct fuse_file_info* fi, int cmd, struct flock* fl) {
  std::cout << blue << "\ncppWrapper_lock" << reset << std::endl;
Original:
  int ret = fcntl((int)fi->fh, cmd, fl);
  if (ret == -1) {
    return -errno;
  }

  return 0;
}

#ifdef HAVE_XATTR
int cppWrapper_setxattr(const char* path, const char* name, const char* value, size_t size, int flags) {
  std::cout << blue << "\ncppWrapper_setxattr" << reset << std::endl;
  std::string _path = Utility::constructRelativePath(path);
  Cache c(_path);

  int ret;
#ifdef __APPLE__
  ret = setxattr(c.fileCachePath.c_str(), name, value, size, 0, flags);
#else
  ret = setxattr(c.fileCachePath.c_str(), name, value, size, flags);
#endif /* __APPLE__ */
  if (ret == -1) {
    return -errno;
  }

  return 0;
}

int cppWrapper_getxattr(const char* path, const char* name, char* value, size_t size) {
  std::cout << blue << "\ncppWrapper_getxattr" << reset << std::endl;
    std::string _path = Utility::constructRelativePath(path);
  Cache c(_path);

Original:
  int ret;
  //ret = getxattr(path, name, value, size);
  ret = getxattr(c.fileCachePath.c_str(), name, value, size);
  if (ret == -1)
    return -errno;

  return 0;
}

int cppWrapper_listxattr(const char* path, char* list, size_t size) {
  std::cout << blue << "\ncppWrapper_listxattr" << reset << std::endl;
Original:
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
  std::cout << blue << "\ncppWrapper_removexattr" << reset << std::endl;
Original:
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

#ifdef HAVE_FLOCK
int cppWrapper_flock(const char* path, struct fuse_file_info* fi, int op) {
  std::cout << blue << "\ncppWrapper_flock" << reset << std::endl;
Original:
  int ret = flock(((int)fi->fh), op);
  if (ret == -1) {
    return -errno;
  }

  return 0;
}
#endif /* HAVE_FLOCK */

#ifdef HAVE_UTIMENSAT
int cppWrapper_utimens(const char* path, const struct timespec ts[2]) {
  std::cout << blue << "\ncppWrapper_utimens" << reset << std::endl;
Original:
  /* don't use utime/utimes since they follow symlinks */
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

#ifdef __cplusplus
}
#endif
