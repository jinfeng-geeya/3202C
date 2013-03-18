#ifndef _DIRENT_H
#define _DIRENT_H       

#include "types.h"


struct dirent 
{
	dev_t        d_dev;
	ino_t        d_ino;
	uint32		 d_type;
	off_t		 d_size;	
	unsigned short  d_reclen;
	char		d_name[1]; 
};


int fs_opendir(const char *path);
ssize_t fs_readdir(int fd, struct dirent *buffer);
int fs_rewinddir(int fd);
int fs_closedir(int fd);



#endif
