#include "./cppWrapper.h"
#include "./grpc-client.h"

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}
#endif
