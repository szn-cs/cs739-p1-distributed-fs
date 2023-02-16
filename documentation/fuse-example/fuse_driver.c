/**
 * FUSE driver: used to mount FS with custom hooks registered. 
*/

#define FUSE_USE_VERSION FUSE_MAKE_VERSION(3, 10)  // https://github.com/libfuse/libfuse/blob/master/ChangeLog.rst

// C includes
#include <fuse.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

void* private_data = NULL;  // FS initial context
char* fs_path = NULL;       // name of the device or image file where the file system resides
// FUSE handlers: https://libfuse.github.io/doxygen/structfuse__operations.html
static struct fuse_operations custom_operations = {};

int main(int argc, char* argv[]) {
    int i, _status = 0;

    /**
   *  get the device or image filename from arguments and remove it before initializing FUSE library
   */
    for (i = 1; i < argc && (argv[i][0] == '-'); i++)
        ;
    if (i < argc) {
        fs_path = realpath(argv[i], NULL);  // convert to absolute path
        memcpy(&argv[i], &argv[i + 1], (argc - i) * sizeof(argv[0]));
        argc--;
    } else {
        printf("❌ missing arguments or unknown\n");
        return -1;
    }

    printf("path: %s\n", fs_path);
    printf("⚫ Mounting file system.\n");

    // register FUSE handlers
    _status = fuse_main(argc, argv, &custom_operations, private_data);

    printf("fuse_main call return status: %d\n", _status);
    return _status;
};


