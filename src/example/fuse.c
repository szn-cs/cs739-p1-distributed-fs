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

// operations to implement from https://github.com/libfuse/libfuse/blob/master/include/fuse.h#L310

#define FUSE_USE_VERSION 35
#include <fuse.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

TODO:
    // https://wiki.osdev.org/FUSE
    // https://libfuse.github.io/doxygen/fuse_8h.html
