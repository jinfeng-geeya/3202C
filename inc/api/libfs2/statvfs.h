#ifndef _SYS_STATFS_H
#define _SYS_STATFS_H


//#include "types.h"

struct statvfs
{
	unsigned long f_bsize;	 // File system block size. 
	unsigned long f_frsize;	 // Fundamental file system block size. 
	unsigned long f_blocks;	 // Total number of blocks on file system in units of f_frsize. 
	unsigned long f_bfree;	 // Total number of free blocks. 
	unsigned long f_bavail;	 // Number of free blocks available to 
							 //     non-privileged process. 
	unsigned long f_files;	 // Total number of file serial numbers. 
	unsigned long f_ffree;	 // Total number of free file serial numbers. 
	unsigned long f_favail;	 // Number of file serial numbers available to 
							 //     non-privileged process. 
	unsigned long f_fsid;	 // File system ID. 
	unsigned long f_flag;	 // Bit mask of f_flag values. 
	unsigned long f_namemax; // Maximum filename length. 

	char f_device_name[128];
	char f_volume_name[128];
	char f_fsh_name[128];
	
};	

#define		WFSSTAT_NAME	0x0001

int fs_statvfs(const char *path, struct statvfs *buf);
int fs_write_statvfs(const char *path, struct statvfs *buf, unsigned int mask);



#endif

