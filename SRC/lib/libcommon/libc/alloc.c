/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 Copyright (C)
*
*    File:    alloc.c
*
*    Description:    This file contains all functions definition
*		             of memory operations.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	Dec.23.2002       Justin Wu       Ver 0.1    Create file.
*	2.
*****************************************************************************/

#include <api/libc/alloc.h>
#include <api/libc/string.h>
#include <osal/osal.h>

/*
 * 	Name		:   libc_malloc()
 *	Description	:
 *	Parameter	:
 *
 *	Return		:
 *
 */
#if 0
void *libc_malloc(unsigned int len)
{
	void *addr;

	osal_interrupt_disable();
	addr = (void *)malloc(len);
	osal_interrupt_enable();

	return addr;
}

/*
 * 	Name		:   libc_realloc()
 *	Description	:
 *	Parameter	:
 *
 *	Return		:
 *
 */
void *libc_realloc(void *ptr, unsigned int len)
{
	void *addr;

	osal_interrupt_disable();
	addr = (void *)realloc(ptr, len);
	osal_interrupt_enable();

	return addr;
}

/*
 * 	Name		:   libc_mfree()
 *	Description	:
 *	Parameter	:
 *
 *	Return		:
 *
 */
void libc_mfree(void *addr)
{
	osal_interrupt_disable();
	free(addr);
	osal_interrupt_enable();
}

////////////////////////////////////////////////////////////////////

#else	// __USE_LIBC_MM__


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
#ifdef SYS_SDRAM_SIZE
#if(SYS_SDRAM_SIZE>=64)
#define HEAP		0x2000000
#else
#define HEAP		(INT_BIT > 16 ? 4194304 : 65536)
#endif
#else
#define HEAP		(INT_BIT > 16 ? 4194304 : 65536)
#endif

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
#define BLOCK(A)	(((char *) (A) - _heapbase) / BLOCKSIZE + 1)
#define ADDRESS(B)	((__ptr_t) (((B) - 1) * BLOCKSIZE + _heapbase))

/* Current search index for the heap table.  */
extern __malloc_size_t _heapindex;

/* Limit of valid info table indices.  */
extern __malloc_size_t _heaplimit;

/* Doubly linked lists of free fragments.  */
struct list
{
	struct list *next;
	struct list *prev;
};

/* Free list headers for each fragment size.  */
extern struct list _fraghead[];

/* List of blocks allocated with `memalign' (or `valloc').  */
struct alignlist
{
	struct alignlist *next;
	__ptr_t aligned;		/* The address that memaligned returned.  */
	__ptr_t exact;		/* The address that malloc returned.  */
};
struct alignlist *_aligned_blocks;

/* Pointer to the base of the first block.  */
char *_heapbase;

/* Block information table.  Allocated with align/__free (not malloc/free).  */
malloc_info *_heapinfo;

/* Number of info entries.  */
static __malloc_size_t heapsize;

/* Search index in the info table.  */
__malloc_size_t _heapindex;

/* Limit of valid info table indices.  */
__malloc_size_t _heaplimit;

/* Free lists for each fragment size.  */
struct list _fraghead[BLOCKLOG];

/* Instrumentation.  */
__malloc_size_t _chunks_used;
__malloc_size_t _bytes_used;
__malloc_size_t _chunks_free;
__malloc_size_t _bytes_free;

/* Are you experienced?  */
void* sbrk(int nbytes);

int __malloc_initialized = 0;
POINTER (*__morecore) () = sbrk;

static void _free_internal (__ptr_t ptr);


//hook functions
__ptr_t (*__malloc_hook)(__malloc_size_t __size);
void (*__free_hook)(__ptr_t __ptr);
__ptr_t (*__realloc_hook)(__ptr_t __ptr, __malloc_size_t __size);



/* Aligned allocation.  */
static __ptr_t align (__malloc_size_t size)
{
	__ptr_t result;
	unsigned long int adj;

	result = (*__morecore) (size);
	adj = (unsigned long int) ((unsigned long int) ((char *) result -
							   (char *) NULL)) % BLOCKSIZE;
	if (adj != 0)
	{
		adj = BLOCKSIZE - adj;
		(void) (*__morecore) (adj);
		result = (char *) result + adj;
	}

	return result;
}

#define	caddr_t		void *
extern caddr_t RAMSIZE;

int see_heaptop_init(UINT32 addr)
{
	RAMSIZE = (caddr_t)(addr & 0x8FFFFFFF);

	return 0;
}

#define DRAM_SPLIT_CTRL_BASE 0xb8041000
#define PVT_S_ADDR 0x10
#define SHR_S_ADDR 0x18
static int initialize ()
{
#ifdef BL_SECURITY_CONFIG
    #ifdef SEE_CPU
        RAMSIZE = ((*(UINT32 *)(DRAM_SPLIT_CTRL_BASE+SHR_S_ADDR)|0x80000000)& 0X8FFFFFFF);  
    #else 
        RAMSIZE = ((*(UINT32 *)(DRAM_SPLIT_CTRL_BASE+PVT_S_ADDR)|0x80000000)& 0X8FFFFFFF);  
    #endif
#else
#ifndef SEE_CPU	
	RAMSIZE = (caddr_t)(__MM_HEAP_TOP_ADDR & 0X8FFFFFFF);
#else
	RAMSIZE = 0X8FFFFFFF;
#endif
#endif
	heapsize = HEAP / BLOCKSIZE;
	_heapinfo = (malloc_info *) align (heapsize * sizeof (malloc_info));
	if (_heapinfo == NULL)
		return 0;
	MEMSET (_heapinfo, 0, heapsize * sizeof (malloc_info));
	_heapinfo[0].free.size = 0;
	_heapinfo[0].free.next = _heapinfo[0].free.prev = 0;
	_heapindex = 0;
	_heapbase = (char *) _heapinfo;

	/* Account for the _heapinfo block itself in the statistics.  */
	_bytes_used = heapsize * sizeof (malloc_info);
	_chunks_used = 1;

	__malloc_initialized = 1;
	return 1;
}

/* Get neatly aligned memory, initializing or
growing the heap info table as necessary. */
static __ptr_t morecore (__malloc_size_t size)
{
	__ptr_t result;
	malloc_info *newinfo, *oldinfo;
	__malloc_size_t newsize;

	result = align (size);
	if (result == NULL)
		return NULL;

	/* Check if we need to grow the info table.  */
	if ((__malloc_size_t) BLOCK ((char *) result + size) > heapsize)
	{
		newsize = heapsize;
		while ((__malloc_size_t) BLOCK ((char *) result + size) > newsize)
			newsize *= 2;
		newinfo = (malloc_info *) align (newsize * sizeof (malloc_info));
		if (newinfo == NULL)
		{
			(*__morecore) (-size);
			return NULL;
		}
		MEMCPY (newinfo, _heapinfo, heapsize * sizeof (malloc_info));
		MEMSET (&newinfo[heapsize], 0,
				(newsize - heapsize) * sizeof (malloc_info));
		oldinfo = _heapinfo;
		newinfo[BLOCK (oldinfo)].busy.type = 0;
		newinfo[BLOCK (oldinfo)].busy.info.size
		= BLOCKIFY (heapsize * sizeof (malloc_info));
		_heapinfo = newinfo;
		/* Account for the _heapinfo block itself in the statistics.  */
		_bytes_used += newsize * sizeof (malloc_info);
		++_chunks_used;
		_free_internal (oldinfo);
		heapsize = newsize;
	}

	_heaplimit = BLOCK ((char *) result + size);
	return result;
}

/*
void ddd_test()
{
int state = 0;

state = read_cpu_state();

if (state & 0x6)
PRINTF("called in intr or exp!!!\n");

}
*/

/* Allocate memory from the heap.  */
__ptr_t malloc (__malloc_size_t size)
{
	__ptr_t result;
	__malloc_size_t block, blocks, lastblocks, start;
	register __malloc_size_t i;
	struct list *next;

	/* ANSI C allows `malloc (0)' to either return NULL, or to return a
	valid address you can realloc and free (though not dereference).

	It turns out that some extant code (sunrpc, at least Ultrix's version)
	expects `malloc (0)' to return non-NULL and breaks otherwise.
	Be compatible.  */

///
//ddd_test();
///

#if	0
	if (size == 0)
		return NULL;
#endif
	if (size == 0)
	{
		;//libc_printf("malloc size: 0, please check the application!\n");
	}
	osal_task_dispatch_off();

	if (__malloc_hook != NULL)
	{
		result =  (*__malloc_hook) (size);
		osal_task_dispatch_on();
		return result;
	}


	if (!__malloc_initialized)
		if (!initialize ())
		{
			osal_task_dispatch_on();
			return NULL;
		}

	if (size < sizeof (struct list))	// round up
		size = sizeof (struct list);

#ifdef SUNOS_LOCALTIME_BUG
	if (size < 16)
		size = 16;
#endif

	/* Determine the allocation policy based on the request size.  */
	if (size <= BLOCKSIZE / 2)	//2 need fragment of a block
	{
		/* Small allocation to receive a fragment of a block.
		Determine the logarithm to base two of the fragment size. */
		register __malloc_size_t log = 1;
		--size;
		while ((size /= 2) != 0)
			++log;

		/* Look in the fragment lists for a
		free fragment of the desired size. */
		next = _fraghead[log].next;
		if (next != NULL)
		{
			/* There are free fragments of this size.
			Pop a fragment out of the fragment list and return it.
			Update the block's nfree and first counters. */
			result = (__ptr_t) next;
			next->prev->next = next->next;
			if (next->next != NULL)
				next->next->prev = next->prev;
			block = BLOCK (result);
			if (--_heapinfo[block].busy.info.frag.nfree != 0)
				_heapinfo[block].busy.info.frag.first = (unsigned long int)
														((unsigned long int) ((char *) next->next - (char *) NULL)
														 % BLOCKSIZE) >> log;

			/* Update the statistics.  */
			++_chunks_used;
			_bytes_used += 1 << log;
			--_chunks_free;
			_bytes_free -= 1 << log;
		}
		else
		{
			/* No free fragments of the desired size, so get a new block
			and break it into fragments, returning the first.  */
			result = malloc (BLOCKSIZE);	//3 only once recursive self-call
			if (result == NULL)
			{
				osal_task_dispatch_on();
				return NULL;
			}

			/* Link all fragments but the first into the free list.  */
			for (i = 1; i < (__malloc_size_t) (BLOCKSIZE >> log); ++i)
			{
				next = (struct list *) ((char *) result + (i << log));
				next->next = _fraghead[log].next;
				next->prev = &_fraghead[log];
				next->prev->next = next;
				if (next->next != NULL)
					next->next->prev = next;
			}

			/* Initialize the nfree and first counters for this block.  */
			block = BLOCK (result);
			_heapinfo[block].busy.type = log;
			_heapinfo[block].busy.info.frag.nfree = i - 1;
			_heapinfo[block].busy.info.frag.first = i - 1;

			_chunks_free += (BLOCKSIZE >> log) - 1;
			_bytes_free += BLOCKSIZE - (1 << log);
			_bytes_used -= BLOCKSIZE - (1 << log);
		}
	}
	else	//2 need one or more blocks
	{
		/* Large allocation to receive one or more blocks.
		Search the free list in a circle starting at the last place visited.
		If we loop completely around without finding a large enough
		space we will have to get more memory from the system.  */
		blocks = BLOCKIFY (size);
		start = block = _heapindex;
		while (_heapinfo[block].free.size < blocks)
		{
			block = _heapinfo[block].free.next;
			if (block == start)
			{
				/* Need to get more from the system.  Check to see if
				the new core will be contiguous with the final free
				block; if so we don't need to get as much.  */
				block = _heapinfo[0].free.prev;
				lastblocks = _heapinfo[block].free.size;
				if (_heaplimit != 0 && block + lastblocks == _heaplimit &&
						(*__morecore) (0) == ADDRESS (block + lastblocks) &&
						(morecore ((blocks - lastblocks) * BLOCKSIZE)) != NULL)
				{
					/* Which block we are extending (the `final free
					block' referred to above) might have changed, if
					it got combined with a freed info table.  */
					block = _heapinfo[0].free.prev;
					_heapinfo[block].free.size += (blocks - lastblocks);
					_bytes_free += (blocks - lastblocks) * BLOCKSIZE;
					continue;
				}
				result = morecore (blocks * BLOCKSIZE);
				if (result == NULL)
				{
					osal_task_dispatch_on();
					return NULL;
				}
				block = BLOCK (result);
				_heapinfo[block].busy.type = 0;
				_heapinfo[block].busy.info.size = blocks;
				++_chunks_used;
				_bytes_used += blocks * BLOCKSIZE;
				osal_task_dispatch_on();
				return result;
			}
		}

		/* At this point we have found a suitable free list entry.
		Figure out how to remove what we need from the list. */
		result = ADDRESS (block);
		if (_heapinfo[block].free.size > blocks)
		{
			/* The block we found has a bit left over,
			so relink the tail end back into the free list. */
			_heapinfo[block + blocks].free.size
			= _heapinfo[block].free.size - blocks;
			_heapinfo[block + blocks].free.next
			= _heapinfo[block].free.next;
			_heapinfo[block + blocks].free.prev
			= _heapinfo[block].free.prev;
			_heapinfo[_heapinfo[block].free.prev].free.next
			= _heapinfo[_heapinfo[block].free.next].free.prev
			  = _heapindex = block + blocks;
		}
		else
		{
			/* The block exactly matches our requirements,
			so just remove it from the list. */
			_heapinfo[_heapinfo[block].free.next].free.prev
			= _heapinfo[block].free.prev;
			_heapinfo[_heapinfo[block].free.prev].free.next
			= _heapindex = _heapinfo[block].free.next;
			--_chunks_free;
		}

		_heapinfo[block].busy.type = 0;
		_heapinfo[block].busy.info.size = blocks;
		++_chunks_used;
		_bytes_used += blocks * BLOCKSIZE;
		_bytes_free -= blocks * BLOCKSIZE;

		/* Mark all the blocks of the object just allocated except for the
		first with a negative number so you can find the first block by
		adding that adjustment.  */
		while (--blocks > 0)
			_heapinfo[block + blocks].busy.info.size = -blocks;
	}

	osal_task_dispatch_on();
	return result;
}


static void _free_internal (__ptr_t ptr)
{
	int type;
	__malloc_size_t block, blocks;
	register __malloc_size_t i;
	struct list *prev, *next;

	block = BLOCK (ptr);

	type = _heapinfo[block].busy.type;
	switch (type)
	{
	case 0:
		/* Get as many statistics as early as we can.  */
		--_chunks_used;
		_bytes_used -= _heapinfo[block].busy.info.size * BLOCKSIZE;
		_bytes_free += _heapinfo[block].busy.info.size * BLOCKSIZE;

		/* Find the free cluster previous to this one in the free list.
		Start searching at the last block referenced; this may benefit
		programs with locality of allocation.  */
		i = _heapindex;
		if (i > block)
			while (i > block)
				i = _heapinfo[i].free.prev;
		else
		{
			do
				i = _heapinfo[i].free.next;
			while (i > 0 && i < block);
			i = _heapinfo[i].free.prev;
		}

		/* Determine how to link this block into the free list.  */
		if (block == i + _heapinfo[i].free.size)
		{
			/* Coalesce this block with its predecessor.  */
			_heapinfo[i].free.size += _heapinfo[block].busy.info.size;
			block = i;
		}
		else
		{
			/* Really link this block back into the free list.  */
			_heapinfo[block].free.size = _heapinfo[block].busy.info.size;
			_heapinfo[block].free.next = _heapinfo[i].free.next;
			_heapinfo[block].free.prev = i;
			_heapinfo[i].free.next = block;
			_heapinfo[_heapinfo[block].free.next].free.prev = block;
			++_chunks_free;
		}

		/* Now that the block is linked in, see if we can coalesce it
		with its successor (by deleting its successor from the list
		and adding in its size).  */
		if (block + _heapinfo[block].free.size == _heapinfo[block].free.next)
		{
			_heapinfo[block].free.size
			+= _heapinfo[_heapinfo[block].free.next].free.size;
			_heapinfo[block].free.next
			= _heapinfo[_heapinfo[block].free.next].free.next;
			_heapinfo[_heapinfo[block].free.next].free.prev = block;
			--_chunks_free;
		}

		/* Now see if we can return stuff to the system.  */
		blocks = _heapinfo[block].free.size;
		if (blocks >= FINAL_FREE_BLOCKS && block + blocks == _heaplimit
				&& (*__morecore) (0) == ADDRESS (block + blocks))
		{
			register __malloc_size_t bytes = blocks * BLOCKSIZE;
			_heaplimit -= blocks;
			(*__morecore) (-bytes);
			_heapinfo[_heapinfo[block].free.prev].free.next
			= _heapinfo[block].free.next;
			_heapinfo[_heapinfo[block].free.next].free.prev
			= _heapinfo[block].free.prev;
			block = _heapinfo[block].free.prev;
			--_chunks_free;
			_bytes_free -= bytes;
		}

		/* Set the next search to begin at this block.  */
		_heapindex = block;
		break;

	default:
		/* Do some of the statistics.  */
		--_chunks_used;
		_bytes_used -= 1 << type;
		++_chunks_free;
		_bytes_free += 1 << type;

		/* Get the address of the first free fragment in this block.  */
		prev = (struct list *) ((char *) ADDRESS (block) +
								(_heapinfo[block].busy.info.frag.first << type));

		if (_heapinfo[block].busy.info.frag.nfree == (__malloc_size_t)((BLOCKSIZE >> type) - 1))
		{
			/* If all fragments of this block are free, remove them
			from the fragment list and free the whole block.  */
			next = prev;
			for (i = 1; i < (__malloc_size_t) (BLOCKSIZE >> type); ++i)
				next = next->next;
			prev->prev->next = next;
			if (next != NULL)
				next->prev = prev->prev;
			_heapinfo[block].busy.type = 0;
			_heapinfo[block].busy.info.size = 1;

			/* Keep the statistics accurate.  */
			++_chunks_used;
			_bytes_used += BLOCKSIZE;
			_chunks_free -= BLOCKSIZE >> type;
			_bytes_free -= BLOCKSIZE;

			free (ADDRESS (block));
		}
		else if (_heapinfo[block].busy.info.frag.nfree != 0)
		{
			/* If some fragments of this block are free, link this
			fragment into the fragment list after the first free
			fragment of this block. */
			next = (struct list *) ptr;
			next->next = prev->next;
			next->prev = prev;
			prev->next = next;
			if (next->next != NULL)
				next->next->prev = next;
			++_heapinfo[block].busy.info.frag.nfree;
		}
		else
		{
			/* No fragments of this block are free, so link this
			fragment into the fragment list and announce that
			it is the first free fragment of this block. */
			prev = (struct list *) ptr;
			_heapinfo[block].busy.info.frag.nfree = 1;
			_heapinfo[block].busy.info.frag.first = (unsigned long int)
													((unsigned long int) ((char *) ptr - (char *) NULL)
													 % BLOCKSIZE >> type);
			prev->next = _fraghead[type].next;
			prev->prev = &_fraghead[type];
			prev->prev->next = prev;
			if (prev->next != NULL)
				prev->next->prev = prev;
		}
		break;
	}
}

/* Return memory to the heap.  */
void free (__ptr_t ptr)
{
	register struct alignlist *l;

///
//ddd_test();
///
	if(( ptr < (void *)(_heapbase + heapsize * sizeof (malloc_info)))||( ptr >= RAMSIZE))
	{
		return;
	}
	osal_task_dispatch_off();

	if (ptr == NULL)
	{
		osal_task_dispatch_on();
		return;
	}

	for (l = _aligned_blocks; l != NULL; l = l->next)
	{
		if (l->aligned == ptr)
		{
			l->aligned = NULL;	/* Mark the slot in the list as free.  */
			ptr = l->exact;
			break;
		}
	}

	if (__free_hook != NULL)
		(*__free_hook) (ptr);
	else
		_free_internal (ptr);
	
	osal_task_dispatch_on();
}

/* Resize the given region to the new size, returning a pointer
to the (possibly moved) region.  This is optimized for speed;
some benchmarks seem to indicate that greater compactness is
achieved by unconditionally allocating and copying to a
new region.  This module has incestuous knowledge of the
internals of both free and malloc. */
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))
__ptr_t realloc (__ptr_t ptr, __malloc_size_t size)
{
	__ptr_t result;
	int type;
	__malloc_size_t block, blocks, oldlimit;

	if (size == 0)
	{
		free (ptr);
		return malloc (0);
	}
	else if (ptr == NULL)
		return malloc (size);

	if (__realloc_hook != NULL)
	{
		return (*__realloc_hook) (ptr, size);
		
	}


	block = BLOCK (ptr);

	type = _heapinfo[block].busy.type;
	switch (type)
	{
	case 0:
		/* Maybe reallocate a large block to a small fragment.  */
		if (size <= BLOCKSIZE / 2)
		{
			result = malloc (size);
			if (result != NULL)
			{
				MEMCPY (result, ptr, size);
				_free_internal (ptr);
				return result;
			}
		}

		/* The new size is a large allocation as well;
		see if we can hold it in place. */
		blocks = BLOCKIFY (size);
		if (blocks < (__malloc_size_t)_heapinfo[block].busy.info.size)
		{
			/* The new size is smaller; return
			excess memory to the free list. */
			_heapinfo[block + blocks].busy.type = 0;
			_heapinfo[block + blocks].busy.info.size
			= _heapinfo[block].busy.info.size - blocks;
			_heapinfo[block].busy.info.size = blocks;
			/* We have just created a new chunk by splitting a chunk in two.
			Now we will free this chunk; increment the statistics counter
			so it doesn't become wrong when _free_internal decrements it.  */
			++_chunks_used;
			_free_internal (ADDRESS (block + blocks));
			result = ptr;
		}
		else if (blocks == (__malloc_size_t)_heapinfo[block].busy.info.size)
			/* No size change necessary.  */
			result = ptr;
		else
		{
			/* Won't fit, so allocate a new region that will.
			Free the old region first in case there is sufficient
			adjacent free space to grow without moving. */
			blocks = _heapinfo[block].busy.info.size;
			/* Prevent free from actually returning memory to the system.  */
			oldlimit = _heaplimit;
			_heaplimit = 0;
			_free_internal (ptr);
			_heaplimit = oldlimit;
			result = malloc (size);
			if (result == NULL)
			{
				/* Now we're really in trouble.  We have to unfree
				the thing we just freed.  Unfortunately it might
				have been coalesced with its neighbors.  */
				if (_heapindex == block)
					(void) malloc (blocks * BLOCKSIZE);
				else
				{
					__ptr_t previous = malloc ((block - _heapindex) * BLOCKSIZE);
					(void) malloc (blocks * BLOCKSIZE);
					_free_internal (previous);
				}
				return NULL;
			}
			if (ptr != result)
				MEMMOVE (result, ptr, blocks * BLOCKSIZE);
		}
		break;

	default:
		/* Old size is a fragment; type is logarithm
		to base two of the fragment size.  */
		if (size > (__malloc_size_t) (1 << (type - 1)) &&
				size <= (__malloc_size_t) (1 << type))
			/* The new size is the same kind of fragment.  */
			result = ptr;
		else
		{
			/* The new size is different; allocate a new space,
			and copy the lesser of the new size and the old. */
			result = malloc (size);
			if (result == NULL)
				return NULL;
			MEMCPY (result, ptr, MIN (size, (__malloc_size_t) 1 << type));
			free (ptr);
		}
		break;
	}

	return result;
}


__ptr_t calloc(__malloc_size_t nelem, __malloc_size_t elsize)
{
	unsigned char *ptr = malloc(nelem *	elsize);
	if(ptr)
	{
		libc_memset(ptr, 0, nelem *	elsize);
	}
	return ptr;
}


#endif	//__USE_LIBC_MM__

////////////////////////////////////////////////////////////////////
