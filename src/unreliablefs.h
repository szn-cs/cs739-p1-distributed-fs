#ifndef UNRELIABLEFS_HH
#define UNRELIABLEFS_HH

#include <limits.h> /* PATH_MAX */
#include <pthread.h>

#include "cppWrapper.h"  // include cpp wrappers

#define DEFAULT_CONF_NAME "unreliablefs.conf"

typedef struct unreliablefs_config {
    struct err_inj_q *errors;
    char *basedir;
    char *config_path;
    unsigned int seed;
    unsigned int debug;
    pthread_mutex_t mutex;
    char *AddrPort;
    char *CacheDir;
} unreliablefs_config;

#endif /* UNRELIABLEFS_HH */
