
#include "unreliablefs_ops.h"
#include "unreliablefs.h"

const char *fuse_op_name[] = {
    "getattr",
    "readlink",
    "mknod",
    "mkdir",
    "unlink",
    "rmdir",
    "symlink",
    "rename",
    "link",
    "chmod",
    "chown",
    "truncate",
    "open",
    "read",
    "write",
    "statfs",
    "flush",
    "release",
    "fsync",
#ifdef HAVE_XATTR
    "setxattr",
    "getxattr",
    "listxattr",
    "removexattr",
#endif /* HAVE_XATTR */
    "opendir",
    "readdir",
    "releasedir",
    "fsyncdir",
    "access",
    "creat",
    "ftruncate",
    "fgetattr",
    "lock",
#if !defined(__OpenBSD__)
    "ioctl",
#endif /* __OpenBSD__ */
#ifdef HAVE_FLOCK
    "flock",
#endif /* HAVE_FLOCK */
#ifdef HAVE_FALLOCATE
    "fallocate",
#endif /* HAVE_FALLOCATE */
#ifdef HAVE_UTIMENSAT
    "utimens",
#endif /* HAVE_UTIMENSAT */
    "lstat"};
//
extern struct unreliablefs_config conf;
//
extern int error_inject(const char *path, fuse_op operation);

int unreliable_lstat(const char *path, struct stat *buf) {
    int ret = error_inject(path, OP_LSTAT);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    return cppWrapper_lstat(path, buf);
}

int unreliable_getattr(const char *path, struct stat *buf) {
    int ret = error_inject(path, OP_GETATTR);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    return cppWrapper_getattr(path, buf);
}

int unreliable_readlink(const char *path, char *buf, size_t bufsiz) {
    int ret = error_inject(path, OP_READLINK);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    return cppWrapper_readlink(path, buf, bufsiz);
}

int unreliable_mknod(const char *path, mode_t mode, dev_t dev) {
    int ret = error_inject(path, OP_MKNOD);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    return cppWrapper_mknod(path, mode, dev);
}

int unreliable_mkdir(const char *path, mode_t mode) {
    int ret = error_inject(path, OP_MKDIR);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    return cppWrapper_mkdir(path, mode);
}

int unreliable_unlink(const char *path) {
    int ret = error_inject(path, OP_UNLINK);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    return cppWrapper_unlink(path);
}

int unreliable_rmdir(const char *path) {
    int ret = error_inject(path, OP_RMDIR);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    return cppWrapper_rmdir(path);
}

int unreliable_symlink(const char *target, const char *linkpath) {
    int ret = error_inject(target, OP_SYMLINK);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    return cppWrapper_symlink(target, linkpath);
}

int unreliable_rename(const char *oldpath, const char *newpath) {
    int ret = error_inject(oldpath, OP_RENAME);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    return cppWrapper_rename(oldpath, newpath);
}

int unreliable_link(const char *oldpath, const char *newpath) {
    int ret = error_inject(oldpath, OP_LINK);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    return cppWrapper_link(oldpath, newpath);
}

int unreliable_chmod(const char *path, mode_t mode) {
    int ret = error_inject(path, OP_CHMOD);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    return cppWrapper_chmod(path, mode);
}

int unreliable_chown(const char *path, uid_t owner, gid_t group) {
    int ret = error_inject(path, OP_CHOWN);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    return cppWrapper_chown(path, owner, group);
}

int unreliable_truncate(const char *path, off_t length) {
    int ret = error_inject(path, OP_TRUNCATE);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    return cppWrapper_truncate(path, length);
}

int unreliable_open(const char *path, struct fuse_file_info *fi) {
    int ret = error_inject(path, OP_OPEN);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    // original
    // return cppWrapper_open(path, fi);
    // Temporary fix
    char *s = "";
    return cppWrapper_open(s, s, path, fi);
}

int unreliable_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    int ret = error_inject(path, OP_READ);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    return cppWrapper_read(conf.AddrPort, conf.CacheDir, path, buf, size, offset, fi);
}

int unreliable_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    int ret = error_inject(path, OP_WRITE);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    // original
    // return cppWrapper_write(path, buf, size, offset, fi);
    // temporary fix
    char *s = "";
    return cppWrapper_write(s, s, path, buf, size, offset, fi);
}

int unreliable_statfs(const char *path, struct statvfs *buf) {
    int ret = error_inject(path, OP_STATFS);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    return cppWrapper_statfs(path, buf);
}

int unreliable_flush(const char *path, struct fuse_file_info *fi) {
    int ret = error_inject(path, OP_FLUSH);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    return cppWrapper_flush(path, fi);
}

int unreliable_release(const char *path, struct fuse_file_info *fi) {
    int ret = error_inject(path, OP_RELEASE);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    // original
    // return cppWrapper_release(path, fi);
    // temporary fix
    char *s = "";
    return cppWrapper_release(s, s, path, fi);
}

int unreliable_fsync(const char *path, int datasync, struct fuse_file_info *fi) {
    int ret = error_inject(path, OP_FSYNC);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    return cppWrapper_fsync(path, datasync, fi);
}

#ifdef HAVE_XATTR
int unreliable_setxattr(const char *path, const char *name, const char *value, size_t size, int flags) {
    int ret = error_inject(path, OP_SETXATTR);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    return cppWrapper_setxattr(path, name, value, size, flags);
}

int unreliable_getxattr(const char *path, const char *name, char *value, size_t size) {
    int ret = error_inject(path, OP_GETXATTR);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    return cppWrapper_getxattr(path, name, value, size);
}

int unreliable_listxattr(const char *path, char *list, size_t size) {
    int ret = error_inject(path, OP_LISTXATTR);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    cppWrapper_listxattr(path, list, size);

    return ret;
}

int unreliable_removexattr(const char *path, const char *name) {
    int ret = error_inject(path, OP_REMOVEXATTR);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    return cppWrapper_removexattr(path, name);
}
#endif /* HAVE_XATTR */

int unreliable_opendir(const char *path, struct fuse_file_info *fi) {
    int ret = error_inject(path, OP_OPENDIR);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    return cppWrapper_opendir(path, fi);
}

int unreliable_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    int ret = error_inject(path, OP_READDIR);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    return cppWrapper_readdir(path, buf, filler, offset, fi);
}

int unreliable_releasedir(const char *path, struct fuse_file_info *fi) {
    int ret = error_inject(path, OP_RELEASEDIR);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    return cppWrapper_releasedir(path, fi);
}

int unreliable_fsyncdir(const char *path, int datasync, struct fuse_file_info *fi) {
    int ret = error_inject(path, OP_FSYNCDIR);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    return cppWrapper_fsyncdir(path, datasync, fi);
}

int unreliable_access(const char *path, int mode) {
    int ret = error_inject(path, OP_ACCESS);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    return cppWrapper_access(path, mode);
}

int unreliable_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    int ret = error_inject(path, OP_CREAT);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    return cppWrapper_create(path, mode, fi);
}

int unreliable_ftruncate(const char *path, off_t length, struct fuse_file_info *fi) {
    int ret = error_inject(path, OP_FTRUNCATE);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    return cppWrapper_ftruncate(path, length, fi);
}

int unreliable_fgetattr(const char *path, struct stat *buf, struct fuse_file_info *fi) {
    int ret = error_inject(path, OP_FGETATTR);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    return cppWrapper_fgetattr(path, buf, fi);
}

int unreliable_lock(const char *path, struct fuse_file_info *fi, int cmd, struct flock *fl) {
    int ret = error_inject(path, OP_LOCK);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    return cppWrapper_lock(path, fi, cmd, fl);
}

void *unreliable_init(struct fuse_conn_info *conn) {
    return NULL;
}

void unreliable_destroy(void *private_data) {
}

#if !defined(__OpenBSD__)
int unreliable_ioctl(const char *path, int cmd, void *arg, struct fuse_file_info *fi, unsigned int flags, void *data) {
    int ret = error_inject(path, OP_IOCTL);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    return cppWrapper_ioctl(path, cmd, arg, fi, flags, data);
}
#endif /* __OpenBSD__ */

#ifdef HAVE_FLOCK
int unreliable_flock(const char *path, struct fuse_file_info *fi, int op) {
    int ret = error_inject(path, OP_FLOCK);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    return cppWrapper_flock(path, fi, op);
}
#endif /* HAVE_FLOCK */

#ifdef HAVE_FALLOCATE
int unreliable_fallocate(const char *path, int mode, off_t offset, off_t len, struct fuse_file_info *fi) {
    int ret = error_inject(path, OP_FALLOCATE);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    return cppWrapper_fallocate(path, mode, offset, len, fi);
}
#endif /* HAVE_FALLOCATE */

#ifdef HAVE_UTIMENSAT
int unreliable_utimens(const char *path, const struct timespec ts[2]) {
    int ret = error_inject(path, OP_UTIMENS);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    return cppWrapper_utimens(path, ts);
}
#endif /* HAVE_UTIMENSAT */
