#include <dirent.h>
#include <errno.h>
#include <openssl/sha.h>
#include <stdio.h>
#include <string.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#ifdef HAVE_XATTR
#include <sys/xattr.h>
#endif /* HAVE_XATTR */

#ifdef linux
/* For pread()/pwrite()/utimensat() */
#define _XOPEN_SOURCE 700
#endif

#define ERRNO_NOOP -999

#include <fuse.h>

#ifndef CPP_WRAPPER_IN_C_H
#define CPP_WRAPPER_IN_C_H

#ifdef __cplusplus

#include "./grpc-client.h"
extern "C" {
#endif

// 11. mkdir(self, path, mode)
// 4. unlink(self, path)
// 12. rmdir(self, path)
// 1. getattr(self, path)
// 5. read(self, path, size, offset)
// 8. open(self, path, flags)
// 6. write(self, path, buf, offset)
// 2. readdir(self, path, offset)
// 3. mknod(self, path, mode, dev)
// 13. rename(self, pathfrom, pathto)
// 14. fsync(self, path, isfsyncfile
// 7. release(self, path, flags)
// 9. truncate(self, path, size)
// 10. utime(self, path, times)

int cppWrapper_lstat(const char* path, struct stat* buf);
int cppWrapper_getattr(const char* path, struct stat* buf);
int cppWrapper_readlink(const char* path, char* buf, size_t bufsiz);
int cppWrapper_mknod(const char* path, mode_t mode, dev_t dev);
int cppWrapper_mkdir(const char*, mode_t mode);
int cppWrapper_unlink(const char* path);
int cppWrapper_rmdir(const char* path);
int cppWrapper_symlink(const char* target, const char* linkpath);
int cppWrapper_rename(const char* oldpath, const char* newpath);
int cppWrapper_link(const char* oldpath, const char* newpath);
int cppWrapper_chmod(const char* path, mode_t mode);
int cppWrapper_chown(const char* path, uid_t owner, gid_t group);
int cppWrapper_truncate(const char* path, off_t length);
int cppWrapper_open(char* addrport, char* CacheDir, const char* path, struct fuse_file_info* fi);
int cppWrapper_read(char* addrport, char* CacheDir, const char* path, char* buf, size_t size, off_t offset, struct fuse_file_info* fi);
int cppWrapper_write(char* addrport, char* CacheDir, const char* path, const char* buf, size_t size, off_t offset, struct fuse_file_info* fi);
int cppWrapper_statfs(const char* path, struct statvfs* buf);
int cppWrapper_flush(const char* path, struct fuse_file_info* fi);
int cppWrapper_release(char* addrport, char* CacheDir, const char* path, struct fuse_file_info* fi);
int cppWrapper_fsync(const char* path, int datasync, struct fuse_file_info* fi);
int cppWrapper_setxattr(const char* path, const char* name, const char* value, size_t size, int flags);
int cppWrapper_getxattr(const char* path, const char* name, char* value, size_t size);
int cppWrapper_listxattr(const char* path, char* list, size_t size);
int cppWrapper_removexattr(const char* path, const char* name);
int cppWrapper_opendir(const char* path, struct fuse_file_info* fi);
int cppWrapper_readdir(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* fi);
int cppWrapper_releasedir(const char* path, struct fuse_file_info* fi);
int cppWrapper_fsyncdir(const char* path, int datasync, struct fuse_file_info* fi);
int cppWrapper_access(const char* path, int mode);
int cppWrapper_create(const char* path, mode_t mode, struct fuse_file_info* fi);
int cppWrapper_ftruncate(const char* path, off_t length, struct fuse_file_info* fi);
int cppWrapper_fgetattr(const char* path, struct stat* buf, struct fuse_file_info* fi);
int cppWrapper_lock(const char* path, struct fuse_file_info* fi, int cmd, struct flock* fl);
int cppWrapper_ioctl(const char* path, int cmd, void* arg, struct fuse_file_info* fi, unsigned int flags, void* data);
int cppWrapper_flock(const char* path, struct fuse_file_info* fi, int op);
int cppWrapper_fallocate(const char* path, int mode, off_t offset, off_t len, struct fuse_file_info* fi);
int cppWrapper_utimens(const char* path, const struct timespec ts[2]);

#ifdef __cplusplus
}
#endif

#endif /* CPP_WRAPPER_IN_C_H */
