/*
 * Spinlock operations.  These were hand-coded specially for this
 * effort without reference to GPL code.
 *
 * Modified for CSC469 / CSC2208, Fall 2021.
 * Basic function prototypes retained, but bodies rewritten to use C11
 * atomic_flag type.
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
 * Copyright (c) 2003 IBM Corporation.
 * Copyright (c) 2021 Angela Demke Brown
 */

#ifndef __TICKETLOCK_H
#define __TICKETLOCK_H

#include <stdlib.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <assert.h>

/* Declare ticket lock structure */
typedef struct {
	atomic_ulong next_ticket;
	atomic_ulong now_serving;
} ticketlock_t;

/* Initialize a ticket lock to unlocked state. 
 */

static inline void
ticket_init(ticketlock_t *tlp)
{
	assert(tlp);
	atomic_init(&(tlp->next_ticket), 0);
	atomic_init(&(tlp->now_serving), 0);
}


/*
 * Acquire a ticket lock.
 */

static inline void
ticket_lock(ticketlock_t *tlp)
{
	atomic_ulong mytick = atomic_fetch_add_explicit(&(tlp->next_ticket), 1, memory_order_seq_cst);
	while (atomic_load_explicit(&(tlp->now_serving), memory_order_seq_cst) != mytick);
}

/*
 * Release a ticket lock.
 */

static inline void
ticket_unlock(ticketlock_t *tlp)
{
	atomic_fetch_add_explicit(&(tlp->now_serving), 1, memory_order_seq_cst);	
}


/* Define the generic lock functions in terms of the ticket lock functions */
#define spinlock_t ticketlock_t
#define spin_init(s) ticket_init(s)
#define spin_lock(s) ticket_lock(s)
#define spin_unlock(s) ticket_unlock(s)

#endif /* #ifndef __TICKETLOCK_H */
