#include "./cppWrapper.h"

#ifdef __cplusplus
extern "C" {
#endif

int cppWrapper_mkdir(const char* path, mode_t mode) {
  std::cout << "⚫ cppWrapper_mkdir" << std::endl;

  int ret = mkdir(path, mode);
  if (ret == -1) {
    return -errno;
  }

  return 0;
}

#ifdef __cplusplus
}
#endif
