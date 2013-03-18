#include <hld/sto/sto_dev.h>
#include <bus/erom/erom.h>

/**************************************************************
 * Function:
 * 	sto_remote_flash_sync()
 * Description:
 * 	Create and init remote flash device.
 * Inpute:
 *	param --- Parameters to sync with slaves.
 * Return Values:
 *	Return SUCCESS for successfully create and init the
 *      device.
 ***************************************************************/
INT32 sto_remote_flash_sync(struct sto_flash_info *param)
{
	return 	erom_sync(param->sync_tmo,param->mode);
}
