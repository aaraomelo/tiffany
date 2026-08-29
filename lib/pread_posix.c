/* pread_posix.c — pread/pwrite no Windows (erg.c, slot_mem.h). */
#ifdef _WIN32
#define _POSIX_C_SOURCE 200809L
#include <sys/types.h>
#include <io.h>
#include <unistd.h>

ssize_t pread(int fd, void *buf, size_t nbyte, off_t offset) {
    if (_lseeki64(fd, offset, SEEK_SET) < 0) return -1;
    return _read(fd, buf, (unsigned)nbyte);
}

ssize_t pwrite(int fd, const void *buf, size_t nbyte, off_t offset) {
    if (_lseeki64(fd, offset, SEEK_SET) < 0) return -1;
    return _write(fd, buf, (unsigned)nbyte);
}
#endif
