#ifndef __DB_RETURN_VALUE_H__
#define __DB_RETURN_VALUE_H__

#include <retcode.h>

#define DB_SUCCES			SUCCESS
/*invalid parameter*/
#define DBERR_PARAM		-1
/*flash reclaim fail*/
#define DBERR_RECLAIM		-2
/*flash full of valid data*/
#define DBERR_FLASH_FULL	-3
/*data already exist*/
#define DBERR_EXSITS		-4
/*command buffer full*/
#define DBERR_CMDBUF_FULL	-5
/*view buffer full*/
#define DBERR_VIEW_FULL	-6
/**/
#define DBERR_INIT			-7
/*to max number*/
#define DBERR_MAX_LIMIT	-8
/*update fail*/
#define DBERR_UPDATE		-9		

#define DBERR_ID_ASSIGN		-10

/*BASIC OPERTION LAYER*/
/*flash wirte error*/
#define DBERR_BO_WRITE			-11
/*flash read error*/
#define DBERR_BO_READ			-12
/*flash erase error*/
#define DBERR_BO_ERASE			-13
/*node header errupted*/
#define DBERR_BO_BAD_HEADER	-14
/*node header errupted*/
#define DBERR_BO_THRESHOLD	-15
/*need reclaim*/
#define DBERR_BO_NEED_RACLAIM	-16
/*addr init failed*/
#define DBERR_BO_INIT			-17

/*DATA OBJECT LAYER*/
/*create table failed*/
#define DBERR_DO_TABLE		-18
/*create view failed*/
#define DBERR_DO_VIEW		-19
/* reclaim failed*/
#define DBERR_DO_RECLAIM	-20
/*cmd buf*/
#define DBERR_DO_CMDBUF	-21

#define DBERR_DO_PROCESS_OP	-22


/*API LAYER*/
/* node not found*/
#define DBERR_API_NOFOUND	-23
#define DBERR_PACK		-24

/*buffer not enough or overflow*/
#define DBERR_BUF		-25

#define DBERR_DEFAULT	-26

/*use the db3 return value*/
#define NODE_POOL_FULL				DBERR_CMDBUF_FULL	
#define STORE_SPACE_FULL			DBERR_FLASH_FULL
#define BACKUP_DATA_FAIL			DBERR_RECLAIM
#define UPDATE_DATA_FAIL			DBERR_UPDATE			
#define NODE_ALREADY_EXIST			DBERR_EXSITS



#endif

