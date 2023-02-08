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

// operations to implement from https://github.com/libfuse/libfuse/blob/master/include/fuse.h#L310

// 1. getattr(self, path)

// 2. readdir(self, path, offset)

// 3. mknod(self, path, mode, dev)

// 4. unlink(self, path)

// 5. read(self, path, size, offset)

// 6. write(self, path, buf, offset)

// 7. release(self, path, flags)

// 8. open(self, path, flags)

// 9. truncate(self, path, size)

// 10. utime(self, path, times)

// 11. mkdir(self, path, mode)

// 12. rmdir(self, path)

// 13. rename(self, pathfrom, pathto)

// 14. fsync(self, path, isfsyncfile
