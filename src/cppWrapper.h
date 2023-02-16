#include <sys/stat.h>
#include <sys/types.h>

#ifndef AAA_C_CONNECTOR_H
#define AAA_C_CONNECTOR_H

#ifdef __cplusplus

#include <iostream>

extern "C" {
#endif

int cppWrapper_lstat();
int cppWrapper_getattr();
int cppWrapper_readlink();
int cppWrapper_mknod();
int cppWrapper_mkdir(const char*, mode_t);
int cppWrapper_unlink();
int cppWrapper_rmdir();
int cppWrapper_symlink();
int cppWrapper_rename();
int cppWrapper_link();
int cppWrapper_chmod();
int cppWrapper_chown();
int cppWrapper_truncate();
int cppWrapper_open();
int cppWrapper_read();
int cppWrapper_write();
int cppWrapper_statfs();
int cppWrapper_flush();
int cppWrapper_release();
int cppWrapper_fsync();
int cppWrapper_setxattr();
int cppWrapper_getxattr();
int cppWrapper_listxattr();
int cppWrapper_removexattr();
int cppWrapper_opendir();
int cppWrapper_readdir();
int cppWrapper_releasedir();
int cppWrapper_fsyncdir();
int cppWrapper_access();
int cppWrapper_create();
int cppWrapper_ftruncate();
int cppWrapper_fgetattr();
int cppWrapper_lock();
int cppWrapper_ioctl();
int cppWrapper_flock();
int cppWrapper_fallocate();
int cppWrapper_utimens();

#ifdef __cplusplus
}
#endif

#endif
