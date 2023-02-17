#include "./cppWrapper.h"

#ifdef __cplusplus
extern "C" {
#endif

int cppWrapper_lstat(const char* path, struct stat* buf) {
  memset(buf, 0, sizeof(struct stat));
  if (lstat(path, buf) == -1) {
    return -errno;
  }
  return 0;
}

int cppWrapper_getattr(const char* path, struct stat* buf) {
  memset(buf, 0, sizeof(struct stat));
  if (lstat(path, buf) == -1) {
    return -errno;
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
  std::cout << "⚫cppWrapper_mkdir" << std::endl;

  std::string target_str = "localhost:50051";

  AFSClient client(
      grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));

  const std::string _path = "/tmp/fs";
  client.clientMkdir(_path);

  int ret = mkdir(path, mode);
  if (ret == -1) {
    return -errno;
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

int cppWrapper_open(const char* path, struct fuse_file_info* fi) {
  int ret = open(path, fi->flags);
  if (ret == -1) {
    return -errno;
  }
  fi->fh = ret;

  return 0;
}

int cppWrapper_read(const char* path, char* buf, size_t size, off_t offset,
                    struct fuse_file_info* fi) {
  std::cout << "👍cppWrapper_read" << std::endl;
  std::string target_str = "localhost:50051";
  AFSClient client(
      grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));
  int fd;   // file handle id
  int ret;  // return

  /*
  int fd;

  if (fi == NULL) {
    fd = open(path, O_RDONLY);
  } else {
    fd = fi->fh;
  }

  if (fd == -1) {
    return -errno;
  }

  int ret = pread(fd, buf, size, offset);
  if (ret == -1) {
    ret = -errno;
  }

  if (fi == NULL) {
    close(fd);
  }
  */

  return 0;
}

int cppWrapper_write(const char* path, const char* buf, size_t size,
                     off_t offset, struct fuse_file_info* fi) {
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

int cppWrapper_release(const char* path, struct fuse_file_info* fi) {
  int ret = close(fi->fh);
  if (ret == -1) {
    return -errno;
  }

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

#ifdef __cplusplus
}
#endif

int main(int argc, char* argv[]) {
  cppWrapper_mkdir("test_dir", 777);
  return 0;
}