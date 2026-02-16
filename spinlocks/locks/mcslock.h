/* 
 * MCS (Mellor-Crummey and Scott) queueing locks.
 *
 * See:
 * "Algorithms for Scalable Synchronization on Shared-Memory Multiprocessors," 
 *  by J. M. Mellor-Crummey and M. L. Scott. ACM Trans. on Computer Systems, 
 *  Feb. 1991.
 *
 * Modified for CSC469 / CSC2208 Fall 2021 to use C11 atomics.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Copyright (c) 2002 IBM Corporation.
 * Copyright (c) 2005 Thomas E. Hart.
 * Copyright (c) 2021 Angela Demke Brown
 */

#ifndef __MCSLOCK_H
#define __MCSLOCK_H

#include <stdlib.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <assert.h>
#include "../test.h" /* For per-thread qnodes */


/* An mcslock is a pointer to the qnode at the tail of a queue.
 * It is shared and will be modified by multiple threads, so it should be
 * an _Atomic pointer.
 */
  
typedef struct qnode * _Atomic mcslock_t;

static inline void
mcs_init(mcslock_t *lock)
{
	atomic_store(lock, NULL);
}

static inline void
mcs_lock(mcslock_t *lock, struct qnode *n)
{
	struct qnode* pred;
	atomic_store_explicit(&(n->next), NULL, memory_order_seq_cst);
	if ((pred = atomic_exchange_explicit(lock, n, memory_order_seq_cst))) {
		/* not the first node, wait for predecessor to link to itself */ 
		atomic_store_explicit(&(n->locked), true, memory_order_seq_cst);
		atomic_store_explicit(&(pred->next), n, memory_order_seq_cst);
		while (atomic_load_explicit(&(n->locked), memory_order_seq_cst));
	} else {
		/* node list is empty, n is the first node */
	}
}

static inline void
mcs_unlock(mcslock_t *lock, struct qnode *n)
{
	if (!atomic_load_explicit(&(n->next), memory_order_seq_cst)) {
		/* no known next node, first check if tail is set to n itself */
		struct qnode* tmp = n;
		if (atomic_compare_exchange_strong(lock, &tmp, NULL)) return; 
		while (!atomic_load_explicit(&(n->next), memory_order_seq_cst));
	}
	atomic_store_explicit(&(n->next->locked), false, memory_order_seq_cst);
}


/* Define the generic lock functions in terms of the mcs lock functions */
/* The lookup of the thread ID to index into the per-thread data is the price
 * of hiding the different mcs lock interface, which requires an extra arg. 
 */
#define spinlock_t mcslock_t
#define spin_init(s) mcs_init(s)
#define spin_lock(s) mcs_lock(s, &threads[getTID()].my_node);
#define spin_unlock(s) mcs_unlock(s, &threads[getTID()].my_node);

#endif
