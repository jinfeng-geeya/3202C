/*!
	* \file 
	*/
/*********************************************************************
    Copyright (c) 2005 Embedded Internet Solutions, Inc
    All rights reserved. You are not allowed to copy or distribute
    the code without permission.
    There are the NVRAM Porting APIs needed by iPanel MiddleWare. 
    Note: the "int" in the file is 32bits
    $ver0.0.0.1 $author Zouxianyun 2005/04/28
*********************************************************************/
#ifndef _IPANEL_MIDDLEWARE_PORTING_NVRAM_API_FUNCTOTYPE_H_
#define _IPANEL_MIDDLEWARE_PORTING_NVRAM_API_FUNCTOTYPE_H_
#ifdef __cplusplus
extern "C" {
#endif
/*NVRAM information -- address and size, usually is FLASH Memory*/
/*!  
 *  \brief 获得Flash Memory（断电保存区）区的起始地址、块数和每块的长度，建议给iPanel MiddleWare用作NVRAM的Flash Memory每块合适尺寸是32k/64k/128k/256kBytes。但NVRAM的最小空间不得小于64k。注意：如果当该函数得不到nvram的地址和尺寸时， 镜像不必实施，其他nvram接口函数可以放空。
 *  \author
 *  @param[in] addr 保存缓冲区起始地址的变量的地址；
 *  @param[in] numberofsections 保存块数的地址
 *  @param[in] size 保存缓冲区长度的变量的地址。
 *  @param[out] Flash的起始地址及块数、每块的大小
 *  \return 0 – 成功，-1 – 失败
 *  \version 
 *  \date     
 *  \warning 
 *  \sa \#define NVRAMSIZE  0x8000 \n
Static unsigned char *flash_Base; //实际NVRAM地址 \n
int ipanel_porting_nvram_info(unsigned char ** addr, int *num, int *size) \n
{ \n
*addr = flash_Base; \n
*num  = 4; \n
*size = NVRAMSIZE; \n
If ( (*num == 0 || *addr == NULL) || \n
((*size != 0x8000) && (*size != 0x10000) && (*size != 0x20000) && (*size != 0x40000)) || \n
(*num * *size < 0x10000) ) { \n
*addr = NULL; \n
*num = 0; \n
*size = 0; \n
return -1; \n
} \n
Return 0; \n
} \n
  *  \deprecated
 */
int ipanel_porting_nvram_info(unsigned char **addr, int *numberofsection, int *sect_size);
/*Read NVRAM flash data, the function is belong to NVRAM part.*/
/*!  
 *  \brief 从Flash Memory(掉电保护)中读取指定的字节数到数据缓存区中，实现时注意操作空间越界。
 *  \author
 *  @param[in] flash_addr 想要读取的目标Flash Memory的起始地址；
 *  @param[in] buff_addr 保存读取数据的缓冲区。
 *  @param[in] nbytes 想要读取的字节数。
 *  @param[out] 读取的数据
 *  \return 实际读取长度，如果失败，返回-1
 *  \version 
 *  \date     
 *  \warning 
 *  \sa Int ipanel_porting_nvram_read(unsigned int flash_addr, unsigned char *buff_addr, int nbytes) \n
{ \n
if (flash_addr < flash_Base || flash_addr > flash_Base + NVRAMSIZE – nbytes) { \n
memset(buff_addr, 0, nbytes); \n
return -1; \n
} \n
memcpy (buff_addr, (char *)flash_addr, nbytes); \n
return nbytes; \n
} \n
  *  \deprecated
 */
int ipanel_porting_nvram_read(unsigned int address, unsigned char *buf, int nbytes);
/*Write data to NVRAM flash , the function is belong to NVRAM part.*/
/*!  
 *  \brief 将数据写入Flash Memory(掉电保护)空间缓冲区中，实现时注意操作空间越界问题。并注意异步速度问题， 建议使用新的线程来同步速度，并且该函数必须立即返回，不能阻塞。在实际处理时，通常会建立一个镜像SDRAM区，先将数据写到SDRAM镜像去，立即返回，然后后台进程/任务/线程将镜像SDRAM区中的数据写到NVRAM中。需要注明的是：该函数返回后，内存buff_addr并不会立即被释放，而是等到调用状态函数ipanel_porting_nvram_status(unsigned int flash_addr, int len)返回写成功后才会释放该buff_addr，porting程序员可以充分利用这点。
 *  \author
 *  @param[in] flash_addr 想要写入的目标起始地址；
 *  @param[in] buff_addr 写入数据块的起始地址。
 *  @param[in] len 想要写入的字节数。
 *  @param[out] 无
 *  \return 实际写入数据，失败返回-1
 *  \version 
 *  \date     
 *  \warning 
 *  \sa int ipanel_porting_nvram_burn(unsigned int flash_addr, const char *buff_addr, int len) \n
{ \n
if (flash_addr< flash_Base|| flash_addr>flash_Base + NVRAMSIZE–len) { \n
return -1; \n
} \n
post msg[buffer address] to queque \n
return len; \n
} \n
In other task/process/pthread \n
Void write_data from_mirror_to_flash(void) \n
{ \n
Get msg from queque; \n
Write status as Write data to flash \n
} \n
  *  \deprecated
 */
int ipanel_porting_nvram_burn(unsigned int address, const char *buf, int len);
/*return the status of the NVRAM block.*/
/*!  
 *  \brief 判断真正的Flash Memory的写入状态。
 *  \author
 *  @param[in] address burn的起始地址；
 *  @param[in] len     写入的字节数
 *  @param[out] 无
 *  \return 0：正在写； 1：已经写成功；－1：写失败
 *  \version 
 *  \date     
 *  \warning 
 *  \sa ipanel_nvram_demo.c
  *  \deprecated
 */
int ipanel_porting_nvram_status(unsigned int address, int len);
#ifdef __cplusplus
}
#endif
#endif
