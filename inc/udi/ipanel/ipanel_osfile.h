/*!
	* \file 
	*/
/*********************************************************************
    Copyright (c) 2005 Embedded Internet Solutions, Inc
    All rights reserved. You are not allowed to copy or distribute
    the code without permission.
    There are the OSFile Porting APIs needed by iPanel MiddleWare. 
    Note: the "int" in the file is 32bits
    $ver0.0.0.1 $author Zouxianyun 2005/04/28
*********************************************************************/
#ifndef _IPANEL_MIDDLEWARE_PORTING_OSFILE_API_FUNCTOTYPE_H_
#define _IPANEL_MIDDLEWARE_PORTING_OSFILE_API_FUNCTOTYPE_H_
#ifdef __cplusplus
extern "C" {
#endif
/*open a OS file*/
int ipanel_porting_localfile_open(const char *filename, const char *mode);
/*read data from a OS file*/
int ipanel_porting_localfile_read(int fd, char* buffer, int nbytes);
/*close a OS file*/
int ipanel_porting_localfile_close(int fd);
/*open a OS directory*/
/*!  
 *  \brief 打开指定路径的OS目录。
 *  \author
 *  @param[in] path 指定目录，不带驱动器符，后缀可带/，绝对路径
 *  @param[out] 无
 *  \return 目录标示符， -1 – 失败。
 *  \version 
 *  \date     
 *  \warning 
 *  \sa ipanel_osfile_demo.c
  *  \deprecated
 */
int ipanel_porting_localdir_open(const char *path);
/*close a OS directory*/
/*!  
 *  \brief 关闭OS目录。
 *  \author
 *  @param[in] fd 有效的目录标示符
 *  @param[out] 无
 *  \return 0 – 成功， -1 – 失败。
 *  \version 
 *  \date     
 *  \warning 
 *  \sa ipanel_osfile_demo.c
  *  \deprecated
 */
int ipanel_porting_localdir_close(int fd);
/**directory struct define*/
#define MAX_SUB_DIRS   (20)
#define FILE_DIR_NAME_LEN  (20)
#define MAX_FILES_IN_DIR   (80)
typedef struct tagEisDir {
	char dirname[20];
	unsigned int numberofsubdirs;
	struct tagEisDir *subdir[MAX_SUB_DIRS];
	unsigned int numberoffiles;
	char filename[MAX_FILES_IN_DIR][FILE_DIR_NAME_LEN];
}EisDir;
/*read data from a OS directory*/
/*!  
 *  \brief 读取标示符fd中的目录信息。其中结构EisDir的定义如下：\par
typedef struct tagEisDir { \n
char dirname[20]; //当前目录名字 \n
unsigned int numberofsubdirs; //目录里实际的子目录数 \n
struct tagEisDir *subdir[20];   //存放子目录信息，最大20个子目录 \n
unsigned int numberoffiles;   //目录里文件个数 \n
char filename[80][20];       //目录里最大80个文件，每个文件的名字不超过20个字符 \n
}EisDir; \n
 *  \author
 *  @param[in] fd 目录标示符，存储目录结构的内存空间
 *  @param[out] dir 目录结构信息
 *  \return 0 -- 成功， -1 – 失败
 *  \version 
 *  \date     
 *  \warning 
 *  \sa ipanel_osfile_demo.c
  *  \deprecated
 */
int ipanel_porting_localdir_read(int fd, EisDir **dir);
/*create a new OS directory*/
/*!  
 *  \brief 在路径path下创建dir新目录。
 *  \author
 *  @param[in] 路径path，目录名dir
 *  @param[out] 无
 *  \return 0 – 成功， -1 – 失败。
 *  \version 
 *  \date     
 *  \warning 
 *  \sa ipanel_osfile_demo.c
  *  \deprecated
 */
int ipanel_porting_localdir_mkdir(const char *path, const char *dir);
#ifdef __cplusplus
}
#endif
#endif
