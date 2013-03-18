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
#ifdef WIN32
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <errno.h>
#include <assert.h>
#include <windows.h>
#else
#include <api/libc/alloc.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <errno.h>
#endif //WIN32
#include "__alloc.h"

#ifdef WIN32

static int new_lock(lock *l)
{
	if (!InitializeCriticalSectionAndSpinCount(l, 0))
		return -1;
	else
		return 0;
}
static int free_lock(lock *l)
{
	DeleteCriticalSection(l);
	return 0;

}
#else
static int new_lock(lock *l)
{
	lock lk = (lock)OSAL_INVALID_ID;
	lk = (lock)osal_mutex_create();

	*l = lk;

	if (lk == OSAL_INVALID_ID)
		return -1;
	else
		return 0;
}
static int free_lock(lock *l)
{
	if (*l == OSAL_INVALID_ID)
	{
		return -1;
	}
	else
	{
		osal_mutex_delete(*l);
		*l = (lock)OSAL_INVALID_ID;
		return 0;

	}
}

#endif //WIN32

static void _free_internal (struct alloc_info *painfo, __ptr_t ptr);
static void __free2(struct alloc_info *painfo, __ptr_t ptr);

static caddr_t sbrk(struct alloc_info *painfo, int nbytes)
{
	caddr_t        base;

	//osal_interrupt_disable();

	if (painfo->end_ptr > painfo->heap_ptr + nbytes)
	{
		base = painfo->heap_ptr;
		painfo->heap_ptr += nbytes;
		//osal_interrupt_enable();
		return (base);
	} else
	{
		errno = ENOMEM;
		ASSERTMSG(0, "Heap overflow!");
		return ((caddr_t)-1);
	}
}

static POINTER (*__morecore) (struct alloc_info *, int) = sbrk;


/* Aligned allocation.  */
static __ptr_t align (struct alloc_info *painfo, __malloc_size_t size)
{
	__ptr_t result;
	unsigned long int adj;

	result = (*__morecore) (painfo, size);
	adj = (unsigned long int) ((unsigned long int) ((char *) result -
							   (char *) NULL)) % BLOCKSIZE;
	if (adj != 0)
	{
		adj = BLOCKSIZE - adj;
		(void) (*__morecore) (painfo, adj);
		result = (char *) result + adj;
	}

	return result;
}



int __alloc_initialize (UINT32 *handle, caddr_t start, caddr_t end)
{
	struct alloc_info *painfo = MALLOC(sizeof(struct alloc_info));
	MEMSET(painfo, 0, sizeof(struct alloc_info));
		
	painfo->heap_ptr = painfo->start_ptr = (caddr_t)((unsigned long)start & 0X8FFFFFFF);
	painfo->end_ptr = (caddr_t)((unsigned long)end & 0X8FFFFFFF);
	painfo->_buf_size = (unsigned long)end - (unsigned long)start;
	painfo->heapsize = painfo->_buf_size / BLOCKSIZE + 1;
	painfo->_heapinfo = (malloc_info *) align (painfo, painfo->heapsize * sizeof (malloc_info));
	if (painfo->_heapinfo == NULL)
	{
		FREE(painfo);
		return 0;
	}
	MEMSET (painfo->_heapinfo, 0, painfo->heapsize * sizeof (malloc_info));
	painfo->_heapinfo[0].free.size = 0;
	painfo->_heapinfo[0].free.next = painfo->_heapinfo[0].free.prev = 0;
	painfo->_heapindex = 0;
	painfo->_heapbase = (char *) painfo->_heapinfo;

	/* Account for the _heapinfo block itself in the statistics.  */
	painfo->_bytes_used = painfo->heapsize * sizeof (malloc_info);
	painfo->_chunks_used = 1;

	painfo->__malloc_initialized = 1;
	new_lock(&painfo->a_lock); 
	*handle = (UINT32)painfo;
	return 1;
}

int __alloc_cleanup (UINT32 handle)
{
	struct alloc_info *painfo = (struct alloc_info *)handle;
	free_lock(&painfo->a_lock); 
	MEMSET(painfo, 0, sizeof(struct alloc_info));
	FREE(painfo);
	return 0;
}


/* Get neatly aligned memory, initializing or
growing the heap info table as necessary. */
static __ptr_t morecore (struct alloc_info *painfo, __malloc_size_t size)
{
	__ptr_t result;
	malloc_info *newinfo, *oldinfo;
	__malloc_size_t newsize;

	result = align (painfo, size);
	if (result == NULL)
		return NULL;

	/* Check if we need to grow the info table.  */
	if ((__malloc_size_t) BLOCK ((char *) result + size) > painfo->heapsize)
	{
		newsize = painfo->heapsize;
		while ((__malloc_size_t) BLOCK ((char *) result + size) > newsize)
			newsize *= 2;
		newinfo = (malloc_info *) align (painfo, newsize * sizeof (malloc_info));
		if (newinfo == NULL)
		{
			(*__morecore) (painfo, -size);
			return NULL;
		}
		MEMCPY (newinfo, painfo->_heapinfo, painfo->heapsize * sizeof (malloc_info));
		MEMSET (&newinfo[painfo->heapsize], 0,
				(newsize - painfo->heapsize) * sizeof (malloc_info));
		oldinfo = painfo->_heapinfo;
		newinfo[BLOCK (oldinfo)].busy.type = 0;
		newinfo[BLOCK (oldinfo)].busy.info.size = BLOCKIFY (painfo->heapsize * sizeof (malloc_info));
		painfo->_heapinfo = newinfo;
		/* Account for the painfo->_heapinfo block itself in the statistics.  */
		painfo->_bytes_used += newsize * sizeof (malloc_info);
		++painfo->_chunks_used;
		_free_internal (painfo, oldinfo);
		painfo->heapsize = newsize;
	}

	painfo->_heaplimit = BLOCK ((char *) result + size);
	return result;
}


/* Allocate memory from the heap.  */
static __ptr_t _malloc_internal(struct alloc_info *painfo, __malloc_size_t size)
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

	if (size == 0)
	{
	}
	if (!painfo->__malloc_initialized)
	{
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
		next = painfo->_fraghead[log].next;
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
			if (--painfo->_heapinfo[block].busy.info.frag.nfree != 0)
				painfo->_heapinfo[block].busy.info.frag.first = (unsigned long int)
														((unsigned long int) ((char *) next->next - (char *) NULL)
														 % BLOCKSIZE) >> log;

			/* Update the statistics.  */
			++painfo->_chunks_used;
			painfo->_bytes_used += 1 << log;
			--painfo->_chunks_free;
			painfo->_bytes_free -= 1 << log;
		}
		else
		{
			/* No free fragments of the desired size, so get a new block
			and break it into fragments, returning the first.  */
			result = _malloc_internal (painfo, BLOCKSIZE);	//3 only once recursive self-call
			if (result == NULL)
			{
				return NULL;
			}

			/* Link all fragments but the first into the free list.  */
			for (i = 1; i < (__malloc_size_t) (BLOCKSIZE >> log); ++i)
			{
				next = (struct list *) ((char *) result + (i << log));
				next->next = painfo->_fraghead[log].next;
				next->prev = &painfo->_fraghead[log];
				next->prev->next = next;
				if (next->next != NULL)
					next->next->prev = next;
			}

			/* Initialize the nfree and first counters for this block.  */
			block = BLOCK (result);
			painfo->_heapinfo[block].busy.type = log;
			painfo->_heapinfo[block].busy.info.frag.nfree = i - 1;
			painfo->_heapinfo[block].busy.info.frag.first = i - 1;

			painfo->_chunks_free += (BLOCKSIZE >> log) - 1;
			painfo->_bytes_free += BLOCKSIZE - (1 << log);
			painfo->_bytes_used -= BLOCKSIZE - (1 << log);
		}
	}
	else	//2 need one or more blocks
	{
		/* Large allocation to receive one or more blocks.
		Search the free list in a circle starting at the last place visited.
		If we loop completely around without finding a large enough
		space we will have to get more memory from the system.  */
		blocks = BLOCKIFY (size);
		start = block = painfo->_heapindex;
		while (painfo->_heapinfo[block].free.size < blocks)
		{
			block = painfo->_heapinfo[block].free.next;
			if (block == start)
			{
				/* Need to get more from the system.  Check to see if
				the new core will be contiguous with the final free
				block; if so we don't need to get as much.  */
				block = painfo->_heapinfo[0].free.prev;
				lastblocks = painfo->_heapinfo[block].free.size;
				if (painfo->_heaplimit != 0 && block + lastblocks == painfo->_heaplimit &&
						(*__morecore) (painfo, 0) == ADDRESS (block + lastblocks) &&
						(morecore (painfo, (blocks - lastblocks) * BLOCKSIZE)) != NULL)
				{
					/* Which block we are extending (the `final free
					block' referred to above) might have changed, if
					it got combined with a freed info table.  */
					block = painfo->_heapinfo[0].free.prev;
					painfo->_heapinfo[block].free.size += (blocks - lastblocks);
					painfo->_bytes_free += (blocks - lastblocks) * BLOCKSIZE;
					continue;
				}
				result = morecore (painfo, blocks * BLOCKSIZE);
				if (result == NULL)
				{
					return NULL;
				}
				block = BLOCK (result);
				painfo->_heapinfo[block].busy.type = 0;
				painfo->_heapinfo[block].busy.info.size = blocks;
				++painfo->_chunks_used;
				painfo->_bytes_used += blocks * BLOCKSIZE;
				return result;
			}
		}

		/* At this point we have found a suitable free list entry.
		Figure out how to remove what we need from the list. */
		result = ADDRESS (block);
		if (painfo->_heapinfo[block].free.size > blocks)
		{
			/* The block we found has a bit left over,
			so relink the tail end back into the free list. */
			painfo->_heapinfo[block + blocks].free.size = painfo->_heapinfo[block].free.size - blocks;
			painfo->_heapinfo[block + blocks].free.next = painfo->_heapinfo[block].free.next;
			painfo->_heapinfo[block + blocks].free.prev = painfo->_heapinfo[block].free.prev;
			painfo->_heapinfo[painfo->_heapinfo[block].free.prev].free.next 
				= painfo->_heapinfo[painfo->_heapinfo[block].free.next].free.prev
				= painfo->_heapindex = block + blocks;
		}
		else
		{
			/* The block exactly matches our requirements,
			so just remove it from the list. */
			painfo->_heapinfo[painfo->_heapinfo[block].free.next].free.prev 
				= painfo->_heapinfo[block].free.prev;
			painfo->_heapinfo[painfo->_heapinfo[block].free.prev].free.next
				= painfo->_heapindex = painfo->_heapinfo[block].free.next;
			--painfo->_chunks_free;
		}

		painfo->_heapinfo[block].busy.type = 0;
		painfo->_heapinfo[block].busy.info.size = blocks;
		++painfo->_chunks_used;
		painfo->_bytes_used += blocks * BLOCKSIZE;
		painfo->_bytes_free -= blocks * BLOCKSIZE;

		/* Mark all the blocks of the object just allocated except for the
		first with a negative number so you can find the first block by
		adding that adjustment.  */
		while (--blocks > 0)
			painfo->_heapinfo[block + blocks].busy.info.size = -blocks;
	}

	
	return result;
}

__ptr_t __malloc (UINT32 handle, __malloc_size_t size)
{
	__ptr_t ptr;

	struct alloc_info *painfo = (struct alloc_info *)handle;
	LOCK(painfo->a_lock);
	ptr = _malloc_internal(painfo, size);
	UNLOCK(painfo->a_lock);
	return ptr;
	
}

static void _free_internal (struct alloc_info *painfo, __ptr_t ptr)
{
	int type;
	__malloc_size_t block, blocks;
	register __malloc_size_t i;
	struct list *prev, *next;

	block = BLOCK (ptr);

	type = painfo->_heapinfo[block].busy.type;
	switch (type)
	{
	case 0:
		/* Get as many statistics as early as we can.  */
		--painfo->_chunks_used;
		painfo->_bytes_used -= painfo->_heapinfo[block].busy.info.size * BLOCKSIZE;
		painfo->_bytes_free += painfo->_heapinfo[block].busy.info.size * BLOCKSIZE;

		/* Find the free cluster previous to this one in the free list.
		Start searching at the last block referenced; this may benefit
		programs with locality of allocation.  */
		i = painfo->_heapindex;
		if (i > block)
			while (i > block)
				i = painfo->_heapinfo[i].free.prev;
		else
		{
			do
				i = painfo->_heapinfo[i].free.next;
			while (i > 0 && i < block);
			i = painfo->_heapinfo[i].free.prev;
		}

		/* Determine how to link this block into the free list.  */
		if (block == i + painfo->_heapinfo[i].free.size)
		{
			/* Coalesce this block with its predecessor.  */
			painfo->_heapinfo[i].free.size += painfo->_heapinfo[block].busy.info.size;
			block = i;
		}
		else
		{
			/* Really link this block back into the free list.  */
			painfo->_heapinfo[block].free.size = painfo->_heapinfo[block].busy.info.size;
			painfo->_heapinfo[block].free.next = painfo->_heapinfo[i].free.next;
			painfo->_heapinfo[block].free.prev = i;
			painfo->_heapinfo[i].free.next = block;
			painfo->_heapinfo[painfo->_heapinfo[block].free.next].free.prev = block;
			++painfo->_chunks_free;
		}

		/* Now that the block is linked in, see if we can coalesce it
		with its successor (by deleting its successor from the list
		and adding in its size).  */
		if (block + painfo->_heapinfo[block].free.size == painfo->_heapinfo[block].free.next)
		{
			painfo->_heapinfo[block].free.size
				+= painfo->_heapinfo[painfo->_heapinfo[block].free.next].free.size;
			painfo->_heapinfo[block].free.next
				= painfo->_heapinfo[painfo->_heapinfo[block].free.next].free.next;
			painfo->_heapinfo[painfo->_heapinfo[block].free.next].free.prev = block;
			--painfo->_chunks_free;
		}

		/* Now see if we can return stuff to the system.  */
		blocks = painfo->_heapinfo[block].free.size;
		if (blocks >= FINAL_FREE_BLOCKS && block + blocks == painfo->_heaplimit
				&& (*__morecore) (painfo, 0) == ADDRESS (block + blocks))
		{
			register __malloc_size_t bytes = blocks * BLOCKSIZE;
			painfo->_heaplimit -= blocks;
			(*__morecore) (painfo, -bytes);
			painfo->_heapinfo[painfo->_heapinfo[block].free.prev].free.next
				= painfo->_heapinfo[block].free.next;
			painfo->_heapinfo[painfo->_heapinfo[block].free.next].free.prev
				= painfo->_heapinfo[block].free.prev;
			block = painfo->_heapinfo[block].free.prev;
			--painfo->_chunks_free;
			painfo->_bytes_free -= bytes;
		}

		/* Set the next search to begin at this block.  */
		painfo->_heapindex = block;
		break;

	default:
		/* Do some of the statistics.  */
		--painfo->_chunks_used;
		painfo->_bytes_used -= 1 << type;
		++painfo->_chunks_free;
		painfo->_bytes_free += 1 << type;

		/* Get the address of the first free fragment in this block.  */
		prev = (struct list *) ((char *) ADDRESS (block) +
								(painfo->_heapinfo[block].busy.info.frag.first << type));

		if (painfo->_heapinfo[block].busy.info.frag.nfree == (__malloc_size_t)((BLOCKSIZE >> type) - 1))
		{
			/* If all fragments of this block are free, remove them
			from the fragment list and free the whole block.  */
			next = prev;
			for (i = 1; i < (__malloc_size_t) (BLOCKSIZE >> type); ++i)
				next = next->next;
			prev->prev->next = next;
			if (next != NULL)
				next->prev = prev->prev;
			painfo->_heapinfo[block].busy.type = 0;
			painfo->_heapinfo[block].busy.info.size = 1;

			/* Keep the statistics accurate.  */
			++painfo->_chunks_used;
			painfo->_bytes_used += BLOCKSIZE;
			painfo->_chunks_free -= BLOCKSIZE >> type;
			painfo->_bytes_free -= BLOCKSIZE;

			__free2 (painfo, ADDRESS (block));
		}
		else if (painfo->_heapinfo[block].busy.info.frag.nfree != 0)
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
			++painfo->_heapinfo[block].busy.info.frag.nfree;
		}
		else
		{
			/* No fragments of this block are free, so link this
			fragment into the fragment list and announce that
			it is the first free fragment of this block. */
			prev = (struct list *) ptr;
			painfo->_heapinfo[block].busy.info.frag.nfree = 1;
			painfo->_heapinfo[block].busy.info.frag.first = (unsigned long int)
													((unsigned long int) ((char *) ptr - (char *) NULL)
													 % BLOCKSIZE >> type);
			prev->next = painfo->_fraghead[type].next;
			prev->prev = &painfo->_fraghead[type];
			prev->prev->next = prev;
			if (prev->next != NULL)
				prev->next->prev = prev;
		}
		break;
	}
}

/* Return memory to the heap.  */
static void __free2(struct alloc_info *painfo, __ptr_t ptr)
{
	register struct alignlist *l;
	
	if(( ptr < (void *)(painfo->_heapbase + painfo->heapsize * sizeof (malloc_info)))||( ptr >= (__ptr_t)painfo->end_ptr))
	{
		return;
	}
	
	if (ptr == NULL)
	{
		return;
	}

	for (l = painfo->_aligned_blocks; l != NULL; l = l->next)
	{
		if (l->aligned == ptr)
		{
			l->aligned = NULL;	/* Mark the slot in the list as free.  */
			ptr = l->exact;
			break;
		}
	}
	
	_free_internal (painfo, ptr);

	
}

void __free (UINT32 handle, __ptr_t ptr)
{
	struct alloc_info *painfo = (struct alloc_info *)handle;

	LOCK(painfo->a_lock);

	__free2(painfo, ptr);
	
	UNLOCK(painfo->a_lock);
}


/* Resize the given region to the new size, returning a pointer
to the (possibly moved) region.  This is optimized for speed;
some benchmarks seem to indicate that greater compactness is
achieved by unconditionally allocating and copying to a
new region.  This module has incestuous knowledge of the
internals of both free and malloc. */
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))
__ptr_t __realloc (UINT32 handle, __ptr_t ptr, __malloc_size_t size)
{
	__ptr_t result;
	int type;
	__malloc_size_t block, blocks, oldlimit;
	struct alloc_info *painfo = (struct alloc_info *)handle;

	if (size == 0)
	{
		__free ((UINT32)painfo, ptr);
		return __malloc ((UINT32)painfo, 0);
	}
	else if (ptr == NULL)
		return __malloc ((UINT32)painfo, size);

	LOCK(painfo->a_lock);
	
	block = BLOCK (ptr);

	type = painfo->_heapinfo[block].busy.type;
	switch (type)
	{
	case 0:
		/* Maybe reallocate a large block to a small fragment.  */
		if (size <= BLOCKSIZE / 2)
		{
			result = _malloc_internal(painfo, size);
			if (result != NULL)
			{
				MEMCPY (result, ptr, size);
				_free_internal (painfo, ptr);
				UNLOCK(painfo->a_lock);
				return result;
			}
		}

		/* The new size is a large allocation as well;
		see if we can hold it in place. */
		blocks = BLOCKIFY (size);
		if (blocks < (__malloc_size_t)painfo->_heapinfo[block].busy.info.size)
		{
			/* The new size is smaller; return
			excess memory to the free list. */
			painfo->_heapinfo[block + blocks].busy.type = 0;
			painfo->_heapinfo[block + blocks].busy.info.size
				= painfo->_heapinfo[block].busy.info.size - blocks;
			painfo->_heapinfo[block].busy.info.size = blocks;
			/* We have just created a new chunk by splitting a chunk in two.
			Now we will free this chunk; increment the statistics counter
			so it doesn't become wrong when _free_internal decrements it.  */
			++painfo->_chunks_used;
			_free_internal (painfo, ADDRESS (block + blocks));
			result = ptr;
		}
		else if (blocks == (__malloc_size_t)painfo->_heapinfo[block].busy.info.size)
			/* No size change necessary.  */
			result = ptr;
		else
		{
			/* Won't fit, so allocate a new region that will.
			Free the old region first in case there is sufficient
			adjacent free space to grow without moving. */
			blocks = painfo->_heapinfo[block].busy.info.size;
			/* Prevent free from actually returning memory to the system.  */
			oldlimit = painfo->_heaplimit;
			painfo->_heaplimit = 0;
			_free_internal (painfo, ptr);
			painfo->_heaplimit = oldlimit;
			result = _malloc_internal (painfo, size);
			if (result == NULL)
			{
				/* Now we're really in trouble.  We have to unfree
				the thing we just freed.  Unfortunately it might
				have been coalesced with its neighbors.  */
				if (painfo->_heapindex == block)
					(void) _malloc_internal(painfo, blocks * BLOCKSIZE);
				else
				{
					__ptr_t previous = _malloc_internal(painfo, (block - painfo->_heapindex) * BLOCKSIZE);
					(void) _malloc_internal(painfo, blocks * BLOCKSIZE);
					_free_internal (painfo, previous);
				}
				UNLOCK(painfo->a_lock);
				return NULL;
			}
			if (ptr != result)
				MEMMOVE (result, ptr, blocks * BLOCKSIZE);
		}
		UNLOCK(painfo->a_lock);
		break;

	default:
		/* Old size is a fragment; type is logarithm
		to base two of the fragment size.  */
		UNLOCK(painfo->a_lock);
		if (size > (__malloc_size_t) (1 << (type - 1)) &&
				size <= (__malloc_size_t) (1 << type))
			/* The new size is the same kind of fragment.  */
			result = ptr;
		else
		{
			/* The new size is different; allocate a new space,
			and copy the lesser of the new size and the old. */
			result = __malloc ((UINT32)painfo, size);
			if (result == NULL)
				return NULL;
			MEMCPY (result, ptr, MIN (size, (__malloc_size_t) 1 << type));
			__free ((UINT32)painfo, ptr);
		}
		break;
	}

	return result;
}


__ptr_t __calloc(UINT32 handle, __malloc_size_t nelem, __malloc_size_t elsize)
{
	struct alloc_info *painfo = (struct alloc_info *)handle;
	unsigned char *ptr = __malloc((UINT32)painfo, nelem * elsize);
	if(ptr)
	{
		MEMSET(ptr, 0, nelem *	elsize);
	}
	return ptr;
}

int __get_free_ramsize(UINT32 handle)
{
	struct alloc_info *painfo = (struct alloc_info *)handle;
	int free_ram_size = painfo->_buf_size-painfo->_bytes_used;
	return free_ram_size;
}

int __get_used_ramsize(UINT32 handle)
{
	struct alloc_info *painfo = (struct alloc_info *)handle;
	return painfo->_bytes_used;
}


////////////////////////////////////////////////////////////////////
