#include "./cppWrapper.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#ifdef __cplusplus
extern "C" {
#endif

std::unordered_map<std::string, std::string> get_local_cache(
    const std::string& CacheDir) {
  std::unordered_map<std::string, std::string> cache;
  // if no file for cache then create new file for keeping cache file.
  // after the file exists, read each line and add key value pair into cache in
  // memory.
  // File format [key;value\n]:
  //            /temp/path/to/file;ijio1290ej9fjio
  //            /temp/path/to/file2;ijio1290ej9fjio
  std::string local_cache_path = CacheDir + "cache_file.txt";

  std::ifstream cache_file(local_cache_path);
  std::string line;
  while (std::getline(cache_file, line)) {
    // txt;sha
    size_t pos = line.find(";");
    if (pos == std::string::npos) {
      std::cout << "read cache file " << local_cache_path
                << " error: cannot find separator ; " << std::endl;
      return cache;
    }
    std::string key = line.substr(0, pos);
    std::string val = line.substr(pos + 1, line.size() - pos);
    cache[key] = val;
  }
  return cache;
}

int fsync_cache(std::string& CacheDir,
                std::unordered_map<std::string, std::string> cache) {
  // update cache into local cache file.
  std::string local_cache_path = CacheDir + "cache_file.txt";
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

int fsync_file(std::string& FilePath, std::string& buf) {
  std::string tmp_local_file_path = FilePath + ".TMP";
  std::ofstream tmp_file(tmp_local_file_path);
  if (tmp_file.is_open()) {
    tmp_file << buf << std::endl;
  }
  rename(tmp_local_file_path.c_str(), FilePath.c_str());
  return 0;
}

std::string get_hash_path(const std::string& path) {
  unsigned char md_buf[SHA256_DIGEST_LENGTH];  // 32
  SHA256(reinterpret_cast<const unsigned char*>(path.c_str()), path.size(),
         md_buf);
  std::stringstream ss;
  for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
    ss << std::hex << std::setw(2) << std::setfill('0') << (int)md_buf[i];
  }
  return ss.str();
}

int cppWrapper_lstat(const char* path, struct stat* buf) {
  memset(buf, 0, sizeof(struct stat));
  if (lstat(path, buf) == -1) {
    return -errno;
  }
  return 0;
}

int cppWrapper_getattr(const char* path, struct stat* buf) {
  std::cout << "⚫ cppWrapper_getattr" << std::endl;

  std::string target_str = "localhost:50051";

  AFSClient client(
      grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));

  const std::string _path(path);
  int errornum;
  std::memset(buf, 0, sizeof(struct stat));
  int ret = client.clientGetAttr(_path, buf, errornum);
  if (ret == -1) {
    return -errornum;
  }
  return 0;
}

int cppWrapper_readlink(const char* path, char* buf, size_t bufsiz) {
  int ret = readlink(path, buf, bufsiz);
  if (ret == -1) {
    return -errno;
  }
  buf[ret] = 0;

  return 0;
}

int cppWrapper_mknod(const char* path, mode_t mode, dev_t dev) {
  int ret = mknod(path, mode, dev);
  if (ret == -1) {
    return -errno;
  }

  return 0;
}

int cppWrapper_mkdir(const char* path, mode_t mode) {
  std::cout << "⚫ cppWrapper_mkdir" << std::endl;

  std::string target_str = "localhost:50051";

  AFSClient client(
      grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));

  const std::string _path(path);
  int errornum;
  int ret = client.clientMkdir(_path, mode, errornum);
  if (ret == -1) {
    return -errornum;
  }
  return 0;
}

int cppWrapper_unlink(const char* path) {
  int ret = unlink(path);
  if (ret == -1) {
    return -errno;
  }

  return 0;
}

int cppWrapper_rmdir(const char* path) {
  int ret = rmdir(path);
  if (ret == -1) {
    return -errno;
  }

  return 0;
}

int cppWrapper_symlink(const char* target, const char* linkpath) {
  int ret = symlink(target, linkpath);
  if (ret == -1) {
    return -errno;
  }

  return 0;
}

int cppWrapper_rename(const char* oldpath, const char* newpath) {
  int ret = rename(oldpath, newpath);
  if (ret == -1) {
    return -errno;
  }

  return 0;
}

int cppWrapper_link(const char* oldpath, const char* newpath) {
  int ret = link(oldpath, newpath);
  if (ret < 0) {
    return -errno;
  }

  return 0;
}

int cppWrapper_chmod(const char* path, mode_t mode) {
  int ret = chmod(path, mode);
  if (ret < 0) {
    return -errno;
  }

  return 0;
}

int cppWrapper_chown(const char* path, uid_t owner, gid_t group) {
  int ret = chown(path, owner, group);
  if (ret == -1) {
    return -errno;
  }

  return 0;
}

int cppWrapper_truncate(const char* path, off_t length) {
  int ret = truncate(path, length);
  if (ret == -1) {
    return -errno;
  }

  return 0;
}

int cppWrapper_open(char* addrport, char* CacheDir, const char* path,
                    struct fuse_file_info* fi) {
  std::cout << "👍cppWrapper_open" << std::endl;

  std::string local_cache_dir(CacheDir);
  std::string path_str(path);
  std::unordered_map<std::string, std::string> cache =
      get_local_cache(local_cache_dir);
  std::string sha_path = get_hash_path(path_str);
  std::string local_cache_file = local_cache_dir + sha_path;
  AFSClient client(
      grpc::CreateChannel(addrport, grpc::InsecureChannelCredentials()));
  int ret;
  long timestamp;

  if (cache.find(path_str) == cache.end()) {
    // path not exist in the cache
    ret = client.clientOpen(path_str, O_RDWR | O_CREAT, timestamp);
    if (ret != 0) {
      return ret;
    }
    int numBytes;
    std::string buf;
    ret = client.clientRead(path, numBytes, buf, timestamp);
    if (ret != 0) {
      return ret;
    }
    fsync_file(local_cache_file, buf);
    cache.insert(std::pair<std::string, std::string>(path_str, sha_path));
    fsync_cache(local_cache_dir, cache);
  }
  // path exist, then check version, fetch updated data
  // client.clientGetAttr()
  //
  // open local cache file
  ret = open(local_cache_file.c_str(), fi->flags);
  if (ret == -1) {
    return -errno;
  }
  fi->fh = ret;

  return 0;
}

int cppWrapper_read(char* addrport, char* CacheDir, const char* path, char* buf,
                    size_t size, off_t offset, struct fuse_file_info* fi) {
  std::cout << "👍cppWrapper_read" << std::endl;
  std::string local_cache_dir(CacheDir);
  std::string path_str(path);
  int ret, fd;
  int free_mark = 0;

  if (fi == NULL) {
    fi = new fuse_file_info();
    fi->flags = O_RDONLY;
    fd = cppWrapper_open(addrport, CacheDir, path, fi);
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

int cppWrapper_write(char* addrport, char* CacheDir, const char* path,
                     const char* buf, size_t size, off_t offset,
                     struct fuse_file_info* fi) {
  std::cout << "👍cppWrapper_write" << std::endl;
  std::string local_cache_dir(CacheDir);
  std::string path_str(path);
  int ret, fd;
  int free_mark = 0;

  if (fi == NULL) {
    fi = new fuse_file_info();
    fi->flags = O_WRONLY;
    fd = cppWrapper_open(addrport, CacheDir, path, fi);
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
  int ret = statvfs(path, buf);
  if (ret == -1) {
    return -errno;
  }

  return 0;
}

int cppWrapper_flush(const char* path, struct fuse_file_info* fi) {
  int ret = close(dup(fi->fh));
  if (ret == -1) {
    return -errno;
  }

  return 0;
}

int cppWrapper_release(char* addrport, char* CacheDir, const char* path,
                       struct fuse_file_info* fi) {
  std::cout << "👍cppWrapper_release" << std::endl;
  int ret;
  int numOfBytes;
  long timestamp;
  std::string path_str(path);
  AFSClient client(
      grpc::CreateChannel(addrport, grpc::InsecureChannelCredentials()));
  ret = close(fi->fh);
  if (ret == -1) {
    return -errno;
  }

  std::string local_cache_dir(CacheDir);
  std::unordered_map<std::string, std::string> cache =
      get_local_cache(local_cache_dir);
  std::string sha_path = get_hash_path(path_str);
  std::string local_cache_file = local_cache_dir + sha_path;
  std::ifstream is;
  is.open(local_cache_file, std::ios::binary | std::ios::ate);
  is.seekg(0, is.end);
  int length = is.tellg();
  is.seekg(0, is.beg);
  if (length != 0) {
    std::string buf(length, '\0');
    is.read(&buf[0], length);
    ret = client.clientWrite(path_str, buf, length, 0, numOfBytes, timestamp);
  }
  is.close();
  return 0;
}

int cppWrapper_fsync(const char* path, int datasync,
                     struct fuse_file_info* fi) {
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
int cppWrapper_setxattr(const char* path, const char* name, const char* value,
                        size_t size, int flags) {
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

int cppWrapper_getxattr(const char* path, const char* name, char* value,
                        size_t size) {
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
  DIR* dir = opendir(path);

  if (!dir) {
    return -errno;
  }
  fi->fh = (int64_t)dir;

  return 0;
}

int cppWrapper_readdir(const char* path, void* buf, fuse_fill_dir_t filler,
                       off_t offset, struct fuse_file_info* fi) {
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

  return 0;
}

int cppWrapper_releasedir(const char* path, struct fuse_file_info* fi) {
  DIR* dir = (DIR*)fi->fh;

  int ret = closedir(dir);
  if (ret == -1) {
    return -errno;
  }

  return 0;
}

int cppWrapper_fsyncdir(const char* path, int datasync,
                        struct fuse_file_info* fi) {
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
    if (ret == -1) {
      return -errno;
    }
  }
  closedir(dir);

  return 0;
}

int cppWrapper_access(const char* path, int mode) {
  int ret = access(path, mode);
  if (ret == -1) {
    return -errno;
  }

  return 0;
}

int cppWrapper_create(const char* path, mode_t mode,
                      struct fuse_file_info* fi) {
  int ret = open(path, fi->flags, mode);
  if (ret == -1) {
    return -errno;
  }
  fi->fh = ret;

  return 0;
}

int cppWrapper_ftruncate(const char* path, off_t length,
                         struct fuse_file_info* fi) {
  int ret = truncate(path, length);
  if (ret == -1) {
    return -errno;
  }

  return 0;
}

int cppWrapper_fgetattr(const char* path, struct stat* buf,
                        struct fuse_file_info* fi) {
  int ret = fstat((int)fi->fh, buf);
  if (ret == -1) {
    return -errno;
  }

  return 0;
}

int cppWrapper_lock(const char* path, struct fuse_file_info* fi, int cmd,
                    struct flock* fl) {
  int ret = fcntl((int)fi->fh, cmd, fl);
  if (ret == -1) {
    return -errno;
  }

  return 0;
}

#if !defined(__OpenBSD__)
int cppWrapper_ioctl(const char* path, int cmd, void* arg,
                     struct fuse_file_info* fi, unsigned int flags,
                     void* data) {
  int ret = ioctl(fi->fh, cmd, arg);
  if (ret == -1) {
    return -errno;
  }

  return 0;
}
#endif /* __OpenBSD__ */

#ifdef HAVE_FLOCK
int cppWrapper_flock(const char* path, struct fuse_file_info* fi, int op) {
  int ret = flock(((int)fi->fh), op);
  if (ret == -1) {
    return -errno;
  }

  return 0;
}
#endif /* HAVE_FLOCK */

#ifdef HAVE_FALLOCATE
int cppWrapper_fallocate(const char* path, int mode, off_t offset, off_t len,
                         struct fuse_file_info* fi) {
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
  /* don't use utime/utimes since they follow symlinks */
  int ret = utimensat(0, path, ts, AT_SYMLINK_NOFOLLOW);
  if (ret == -1) {
    return -errno;
  }

  return 0;
}
#endif /* HAVE_UTIMENSAT */
int main() {
  // std::unordered_map<std::string, std::string> get_local_cache(const
  // std::string& path) int fsync_cache(std::string& path,
  // std::unordered_map<std::string, std::string> cache) std::string
  // hash_path(const std::string& path)
  // std::string test_cache_path("./test_cache.txt");
  // std::unordered_map<std::string, std::string> tmp_cache =
  //     get_local_cache(test_cache_path);
  // std::string test_path = "./test_pathh";
  // std::string test_hash = get_hash_path(test_path);
  // std::cout << test_path << " " << test_hash << std::endl;
  // tmp_cache.insert(std::pair<std::string, std::string>(test_path,
  // test_hash)); fsync_cache(test_cache_path, tmp_cache);
  // std::string AddrPort_ = "localhost:50051";
  // std::string CacheDir_ = "/tmp/cache/";

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
  // cppWrapper_read("localhost:50051", "/tmp/cache/", "test2.txt", buf1, 20, 5,
  //                 p1);
  // std::cout << "with fh " << buf1 << std::endl;
  // cppWrapper_read("localhost:50051", "/tmp/cache/", "test1.txt", buf2, 20, 5,
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

#ifdef __cplusplus
}
#endif
