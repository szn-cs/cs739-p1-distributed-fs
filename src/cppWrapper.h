#include <sys/stat.h>
#include <sys/types.h>

#ifndef AAA_C_CONNECTOR_H
#define AAA_C_CONNECTOR_H

#ifdef __cplusplus
#include <iostream>
extern "C" {
#endif

int cppWrapper_mkdir(const char *, mode_t);

#ifdef __cplusplus
}
#endif

#endif
