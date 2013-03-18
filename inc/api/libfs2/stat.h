#ifndef _SYS_STAT_H
#define _SYS_STAT_H


#include "types.h"




struct stat 
{
	uint32_t	st_dev;
	long		st_pad1[3];		/* Reserved for network id */
	ino_t		st_ino;
	uint32_t	st_mode;
	int32_t		st_nlink;
	int32_t		st_uid;
	int32_t		st_gid;
	uint32_t	st_rdev;
	long		st_pad2[2];
	off_t		st_size;
	long		st_pad3;
	/*
	 * Actually this should be timestruc_t st_atime, st_mtime and st_ctime
	 * but we don't have it under Linux.
	 */
	time_t		st_atime;
	long		reserved0;
	time_t		st_mtime;
	long		reserved1;
	time_t		st_ctime;
	long		reserved2;
	long		st_blksize;
	long		st_blocks;
	char		st_fstype[16];	/* Filesystem type name */
	long		st_pad4[8];
	/* Linux specific fields */
	uint32_t	st_flags;
	uint32_t	st_gen;

	//for some customer use the win32 similar file system. 
	//and they want to get the attribute of the file or directory
	unsigned long st_attribute;
};



#define S_IFMT	    00170000
#define S_IFSOCK	0140000
#define S_IFLNK		0120000
#define S_IFREG		0100000
#define S_IFBLK		0060000
#define S_IFDIR		0040000
#define S_IFCHR		0020000
#define S_IFIFO		0010000
#define S_ISUID		0004000
#define S_ISGID		0002000
#define S_ISVTX		0001000

#define S_ISLNK(m)	(((m) & S_IFMT) == S_IFLNK)
#define S_ISREG(m)	(((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m)	(((m) & S_IFMT) == S_IFDIR)
#define S_ISCHR(m)	(((m) & S_IFMT) == S_IFCHR)
#define S_ISBLK(m)	(((m) & S_IFMT) == S_IFBLK)
#define S_ISFIFO(m)	(((m) & S_IFMT) == S_IFIFO)
#define S_ISSOCK(m)	(((m) & S_IFMT) == S_IFSOCK)

#define S_IRWXU 00700
#define S_IRUSR 00400
#define S_IWUSR 00200
#define S_IXUSR 00100

#define S_IRWXG 00070
#define S_IRGRP 00040
#define S_IWGRP 00020
#define S_IXGRP 00010

#define S_IRWXO 00007
#define S_IROTH 00004
#define S_IWOTH 00002
#define S_IXOTH 00001


#define S_IREAD S_IRUSR
#define S_IWRITE S_IWUSR
#define S_IEXEC S_IXUSR

#define		WSTAT_MODE		0x0001
#define		WSTAT_UID		0x0002
#define		WSTAT_GID		0x0004
#define		WSTAT_SIZE		0x0008
#define		WSTAT_ATIME		0x0010
#define		WSTAT_MTIME		0x0020
#define		WSTAT_CRTIME	0x0040


//Windows file attribute
enum {
    FILE_ATTRIBUTE_NORMAL = 0x00,
    FILE_ATTRIBUTE_RDONLY = 0x01,
    FILE_ATTRIBUTE_HIDDEN = 0x02,
    FILE_ATTRIBUTE_SYSTEM = 0x04,
    FILE_ATTRIBUTE_VOLUME = 0x08,
    FILE_ATTRIBUTE_DIRENT = 0x10,
    FILE_ATTRIBUTE_ARCHIVE =0x20
};


int fs_stat(const char *path, struct stat *buf);
int fs_fstat(int fd, struct stat *buf);
int fs_wstat(const char *path, struct stat *stat, unsigned long mask);
int fs_mkdir(const char *path, int perms);


#endif
