/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 Copyright (C)
*
*    File:    sto_dev.h
*
*    Description:    This file contains all functions definition
*		             of storage driver.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	May.27.2003      Justin Wu       Ver 0.1    Create file.
*****************************************************************************/

#ifndef __HLD_STO_H__
#define __HLD_STO_H__

#include <sys_config.h>
#include <hld/sto/sto_dev.h>

/* lseek origin param */
#define STO_LSEEK_SET		0
#define STO_LSEEK_CUR		1
#define STO_LSEEK_END		2

#ifdef __cplusplus
extern "C"
{
#endif

INT32 sto_open(struct sto_device *dev);
INT32 sto_close(struct sto_device *dev);
INT32 sto_lseek(struct sto_device *dev, INT32 offset, int origin);
INT32 sto_write(struct sto_device *dev, UINT8 *data, INT32 len);
INT32 sto_read(struct sto_device *dev, UINT8 *data, INT32 len);
INT32 sto_io_control(struct sto_device *dev, INT32 cmd, UINT32 param);
INT32 sto_put_data(struct sto_device *dev, UINT32 offset, UINT8 *data, INT32 len);
INT32 sto_get_data(struct sto_device *dev, UINT8 *data, UINT32 offset, INT32 len);
#ifdef __cplusplus
}
#endif

#endif /* __HLD_STO_H__ */
