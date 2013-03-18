#ifndef _UNISTD_H
#define _UNISTD_H


#include "types.h"


__BEGIN_DECLS


#ifndef SEEK_SET
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif

int fs_access(const char *path, int mode);
int fs_close(int fd);
off_t fs_lseek(int fd, off_t offset, int whence);
int fs_mount(const char *path, const char *device, const char *fsName, unsigned long flags, const char *args);
int fs_open(const char *path, int oflag, int perms);
ssize_t fs_read(int fd, void *buf, size_t nbyte);
int fs_rename(const char *oldPath, const char *newPath);
int fs_remove(const char* path);
int fs_rmdir(const char *path);
int fs_unmount(const char *path, unsigned long flags);
ssize_t fs_write(int fd, const void *buf, size_t nbyte);
int fs_sync(const char *path);
int fs_mkfs(const char *dev, const char* fsName);
int fs_ioctl(int fd, unsigned long op, void *buffer, size_t length);

int fs_symlink(const char *path1, const char *path2);
int fs_link(const char *path1, const char *path2);
ssize_t fs_readlink(const char *path, char *buf,  size_t bufsize);
int fs_unlink(const char *path);

__END_DECLS

#endif
