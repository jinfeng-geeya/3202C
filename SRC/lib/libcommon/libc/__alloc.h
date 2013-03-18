#ifndef __ALLOC_H_BC8BC523_E0F8_49eb_8F06_E0C80BF73FE5_
#define __ALLOC_H_BC8BC523_E0F8_49eb_8F06_E0C80BF73FE5_


//functions for Win32 and TDS

#ifdef WIN32

	#define MEMCPY memcpy
	#define MEMSET memset
	#define MEMCMP memcmp
	#define MEMMOVE memmove
	#define ASSERTMSG(x, y) assert(0)
	typedef CRITICAL_SECTION lock;
	#define LOCK(l)	EnterCriticalSection(&l)
	#define UNLOCK(l) LeaveCriticalSection(&l)
#else
	typedef ID	lock;
	#define LOCK(l)	OS_LockMutex(l, -1)
	#define UNLOCK(l) OS_UnlockMutex(l)

#endif //WIN32




//==================================================================================================================
// internal used

#ifndef CHAR_BIT
#define	CHAR_BIT	8
#endif
#define INT_BIT		(CHAR_BIT * sizeof(int))
#define BLOCKLOG	(INT_BIT > 16 ? 12 : 9)
#define BLOCKSIZE	(1 << BLOCKLOG)
#define BLOCKIFY(SIZE)	(((SIZE) + BLOCKSIZE - 1) / BLOCKSIZE)

typedef void *POINTER;

/* Determine the amount of memory spanned by the initial heap table
(not an absolute limit).  */
#define HEAP		(INT_BIT > 16 ? 4194304 : 65536)

/* Number of contiguous free blocks allowed to build up at the end of
memory before they will be returned to the system.  */
#define FINAL_FREE_BLOCKS	8

/* Data structure giving per-block information.  */
typedef union
{
	/* Heap information for a busy block.  */
	struct
	{
		/* Zero for a large (multiblock) object, or positive giving the
		logarithm to the base two of the fragment size.  */
		int type;
		union
		{
			struct
			{
				__malloc_size_t nfree; /* Free frags in a fragmented block.  */
				__malloc_size_t first; /* First free fragment of the block.  */
			} frag;
			/* For a large object, in its first block, this has the number
			of blocks in the object.  In the other blocks, this has a
			negative number which says how far back the first block is.  */
			__malloc_ptrdiff_t size;
		} info;
	} busy;
	/* Heap information for a free block
	(that may be the first of a free cluster).  */
	struct
	{
		__malloc_size_t size;	/* Size (in blocks) of a free cluster.  */
		__malloc_size_t next;	/* Index of next free cluster.  */
		__malloc_size_t prev;	/* Index of previous free cluster.  */
	} free;
} malloc_info;

/* Address to block number and vice versa.  */
#define BLOCK(A)	(((char *) (A) - painfo->_heapbase) / BLOCKSIZE + 1)
#define ADDRESS(B)	((__ptr_t) (((B) - 1) * BLOCKSIZE + painfo->_heapbase))

/* Doubly linked lists of free fragments.  */
struct list
{
	struct list *next;
	struct list *prev;
};


#undef caddr_t
#define	caddr_t		void *


/* List of blocks allocated with `memalign' (or `valloc').  */
struct alignlist
{
	struct alignlist *next;
	__ptr_t aligned;		/* The address that memaligned returned.  */
	__ptr_t exact;		/* The address that malloc returned.  */
};


struct alloc_info
{
	
	struct alignlist *_aligned_blocks;
	char *_heapbase;					// Pointer to the base of the first block.
	malloc_info *_heapinfo;				// Block information table.  Allocated with align/__free (not malloc/free).

	__malloc_size_t heapsize;			// Number of info entries.	
	__malloc_size_t _heapindex;			// Search index in the info table.
	__malloc_size_t _heaplimit;			//Limit of valid info table indices. 
	

	struct list _fraghead[BLOCKLOG];	//Free lists for each fragment size. 
	
	__malloc_size_t _buf_size;
	__malloc_size_t _chunks_used;		//Instrumentation. 
	__malloc_size_t _bytes_used;
	__malloc_size_t _chunks_free;
	__malloc_size_t _bytes_free;

	int __malloc_initialized;	

	//the memory range
	unsigned char*	start_ptr;
	unsigned char*	end_ptr;
	unsigned char*	heap_ptr;

	//lock for the multi-thread
	lock a_lock;

};


//==================================================================================================================
// APIs

#define	__malloc_size_t		unsigned int
#define	__malloc_ptrdiff_t	int
#define	__ptr_t		void *

int __alloc_initialize (UINT32 *handle, __ptr_t start,  __ptr_t end);
int __alloc_cleanup (UINT32 handle);

__ptr_t __malloc (UINT32 handle, __malloc_size_t size);
__ptr_t __calloc(UINT32 handle, __malloc_size_t nelem, __malloc_size_t elsize);
void __free (UINT32 handle, __ptr_t ptr);
__ptr_t __realloc (UINT32 handle, __ptr_t ptr, __malloc_size_t size);




#endif //__ALLOC_INTERNAL_H_BC8BC523_E0F8_49eb_8F06_E0C80BF73FE5_


