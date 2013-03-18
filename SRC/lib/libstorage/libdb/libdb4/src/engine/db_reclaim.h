#ifndef __DB_RECLAIM_H__
#define __DB_RECLAIM_H__

#if defined(DB_MULTI_SECTOR)
INT32 DB_reclaim_operation();
INT32 DB_reclaim_data( );
#endif

INT32 DB_reclaim_write_header();

INT32 DB_reclaim(UINT8 mode);

#endif

