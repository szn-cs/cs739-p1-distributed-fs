#include "./cppWrapper.h"
#include "./grpc-client.h"

#ifdef __cplusplus
extern "C" {
#endif

int cppWrapper_getattr() { return 0; }
int cppWrapper_readlink() { return 0; }
int cppWrapper_mknod() { return 0; }

int cppWrapper_mkdir(const char* path, mode_t mode) {
  std::cout << "⚫ cppWrapper_mkdir" << std::endl;

  std::string target_str = "localhost:50051";

  AFSClient client(grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));

  const std::string _path = "/tmp/fs";
  client.Mkdir(_path);

  int ret = mkdir(path, mode);
  if (ret == -1) {
    return -errno;
  }

  return 0;
}

int cppWrapper_unlink() { return 0; }
int cppWrapper_rmdir() { return 0; }
int cppWrapper_symlink() { return 0; }
int cppWrapper_rename() { return 0; }
int cppWrapper_link() { return 0; }
int cppWrapper_chmod() { return 0; }
int cppWrapper_chown() { return 0; }
int cppWrapper_truncate() { return 0; }
int cppWrapper_open() { return 0; }
int cppWrapper_read() { return 0; }
int cppWrapper_write() { return 0; }
int cppWrapper_statfs() { return 0; }
int cppWrapper_flush() { return 0; }
int cppWrapper_release() { return 0; }
int cppWrapper_fsync() { return 0; }
int cppWrapper_setxattr() { return 0; }
int cppWrapper_getxattr() { return 0; }
int cppWrapper_listxattr() { return 0; }
int cppWrapper_removexattr() { return 0; }
int cppWrapper_opendir() { return 0; }
int cppWrapper_readdir() { return 0; }
int cppWrapper_releasedir() { return 0; }
int cppWrapper_fsyncdir() { return 0; }
int cppWrapper_access() { return 0; }
int cppWrapper_create() { return 0; }
int cppWrapper_ftruncate() { return 0; }
int cppWrapper_fgetattr() { return 0; }
int cppWrapper_lock() { return 0; }
int cppWrapper_ioctl() { return 0; }
int cppWrapper_flock() { return 0; }
int cppWrapper_fallocate() { return 0; }
int cppWrapper_utimens() { return 0; }
int cppWrapper_lstat() { return 0; }

#ifdef __cplusplus
}
#endif
