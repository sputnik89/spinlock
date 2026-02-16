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

#ifndef __SPINLOCK_H
#define __SPINLOCK_H

#include <stdlib.h>
#include <stdatomic.h>
#include <stdbool.h>

typedef atomic_flag spinlock_t;

/* Initialize a spinlock to unlocked state. 
 * Need either initializer or atomic_flag function depending on context. 
 */

#define SPIN_LOCK_INIT ATOMIC_FLAG_INIT

static inline void
spin_init(spinlock_t *slp)
{
	atomic_flag_clear(slp);
}


/*
 * Acquire a spinlock.
 */

/* Note: Default memory order is memory_order_seq_cst (sequential consistency).
 */

static inline void
spin_lock(spinlock_t *slp)
{
	while(atomic_flag_test_and_set(slp)) {}
	/* To try using acquire/release consistency instead, comment out the
	 * line above, and uncomment the one below.
	 */
	// while(atomic_flag_test_and_set_explicit(slp, memory_order_acq_rel)) {}	
}


/*
 * Release a spinlock.
 */

static inline void
spin_unlock(spinlock_t *slp)
{
	atomic_flag_clear(slp);
	/* To try using acquire/release consistency instead, comment out the
	 * line above, and uncomment the one below.
	 */
	/* atomic_flag_clear_explicit(slp, memory_order_release); */
}

#endif /* #ifndef __SPINLOCK_H */
