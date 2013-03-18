/*
 * Copyright (C) ALi Shanghai Corp. 2005
 *
 * sie.h
 *
 * SI engine interface header file.
 *
 * History:
 * 1. 20051129	0.1	Zhengdao Li	SIE 2.0 header file.
 */
#ifndef __SIE_H__
#define __SIE_H__

#include <types.h>
#include <sys_config.h>

#include <hld/hld_dev.h>
#include <hld/dmx/dmx.h>


#if (SYS_SDRAM_SIZE<4)
#define SIE_MINIMUM_MODEL
#endif


/*
 * this enumerates defines the value provides by external modules.
 */
enum {
//	SIE_MODULE_MAX_FILTER_NUMBER	= 4, 
	SIE_MODULE_MAX_FILTER_NUMBER	= MAX_MV_NUM, /*sync with dmx*/
};

/*
 * this enumerate defines the standard section parser return value.
 */
typedef enum {
	sie_started			= 0x00000001UL, /* section should be activatived */
	sie_stopped			= 0x00000002UL, /* section should be idlized  */
	sie_freed			= 0x00000004UL, /* section should be freed */
}sie_status_t;

/*
  * This enumerate defines the event/handler trigger reason.
*/
enum {
	SIE_REASON_FILTER_TIMEOUT	= 0x08, /* section retrieve timeout*/
	SIE_REASON_FILTER_OVERLAP = 0x10, /* section is overlapped */
	SIE_REASON_RINGBUF_FULL	= 0x20, 
	SIE_REASON_FILTER_HIT		= 0x40,	
	SIE_REASON_ALL			= 0xF8,	/* any reason mask */
};

/*
 * This enumerate defines the attribute of section syntax.
 */
enum {
	SI_ATTR_HAVE_CRC32		= 0x01, /* has CRC32 field in each section end.	*/
	SI_ATTR_HAVE_HEADER		= 0x02, /* has 8-byte of section header structure. */
	SI_ATTR_IS_ISO13818		= 0x04, /* is defined in ISO-13818 specifications. */
};

struct si_filter_t {
	struct dmx_device *dmx;	/* the dmx that the filter belongs to */
	void *priv[SIE_MODULE_MAX_FILTER_NUMBER];
};

typedef BOOL(*si_dmx_event_t)(UINT16 pid, struct si_filter_t *, UINT8, UINT8 *, INT32);
typedef sie_status_t(* si_handler_t)(UINT16 pid, struct si_filter_t *, UINT8, UINT8 *, INT32);

/*
  * This structure provides the basic section filter accessing fields.
*/
struct si_filter_param {
	UINT16 attr[SIE_MODULE_MAX_FILTER_NUMBER]; 	/* combination of SIE_ATTR_xxxx */
	UINT32 timeout; 				/* timeout period value. */
	struct restrict mask_value; 			/* the mask&value pairs. */
	si_dmx_event_t section_event; 			/* callback, could be NULL for single section. */
	si_handler_t section_parser;			/* callback, should not be NULL. */
	void *priv; 					/* for requester use only. */
};


/*
 * This enumerates defines the standard module status.
 */
enum {
	SIE_STATUS_RELEASED			= 0x00000000UL, /* release mode */
	SIE_STATUS_INITED			= 0x00000001UL,	/* initial mode */
	SIE_STATUS_OPENED			= 0x00000002UL,	/* open mode */
};

#ifdef __cplusplus
extern "C"{
#endif

/*
  * Description
  * This function will change the module from released mode into
  * initialized mode.
  * Parameters
  * Returns
  * INT32
  * Return Value List
  * SI_SUCCESS :  The SI engine module is changed to initialized mode.
  * SI_ESTAT :    The SI engine is not in released mode when this function is
  *               called.
  * Remarks
  * This function is thread-safe.                                            
*/
INT32 sie_module_init();

INT32 sie_open(struct dmx_device *dmx, INT32 max_filter_nr, UINT8 *buffer, INT32 buflen);

INT32 sie_open_dmx(struct dmx_device *dmx, INT32 max_filter_nr, UINT8 *buffer, INT32 buflen);

/*
  * Summary
  * This function will change the SI engine module into intialized status.
  * Description
  * This function returns SI_SUCCESS only when the module is in open mode.
  * Return Value List
  * SI_SUCCESS :  The module is closed.
  * SI_ESTAT :    The module is not in open state when this function is
  *               called.
  * Remarks
  * This function is thread-safe.                                         
*/
INT32 sie_close();

INT32 sie_close_dmx(struct dmx_device *dmx);

struct si_filter_t *sie_alloc_filter(UINT16 pid, UINT8 *buffer, UINT32 buffer_len, UINT16 max_sec_length);

struct si_filter_t *sie_alloc_filter_ext(struct dmx_device *dmx, UINT16 pid, UINT8 *buffer, UINT32 length, 
	UINT16 max_sec_length, UINT8 mode);

INT32 sie_enable_filter(struct si_filter_t *filter);

INT32 sie_config_filter(struct si_filter_t *filter, struct si_filter_param *fparam);

INT32 sie_copy_config(struct si_filter_t *filter, struct si_filter_param *fparam);

INT32 sie_abort(UINT16 pid, struct restrict *mask_value);

INT32 sie_abort_filter(struct si_filter_t *filter);
INT32 sie_abort_ext(struct dmx_device *dmx, UINT8 **buff, UINT16 pid, struct restrict *mask_value);

#ifndef SIE_MINIMUM_MODEL
INT32 sie_query_stat();

INT32 sie_get_pid(struct si_filter_t *filter);
#endif


#endif /* __SIE_H__ */

