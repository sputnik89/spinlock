/*
 * Written for CSC469 / CSC2208, Fall 2021.
 * Copyright (c) 2021 Angela Demke Brown
 */

#ifndef __PICKLOCK_H
#define __PICKLOCK_H

#include <stdlib.h>
#include <stdatomic.h>
#include <stdbool.h>

/* Use C preprocessor definitions to pick a lock type.
 * One, and only one, of these should be set in the compiler flags.
 * If multiple flags are set, the first one in this list will take effect. 
 */

/* Each lock type must define a type 'spinlock_t' and functions 
 * spin_init, spin_lock, and spin_unlock.
 */

#if defined PTHREADMUTEX
#include <pthread.h>
typedef pthread_mutex_t spinlock_t;
#define SPIN_LOCK_INIT PTHREAD_MUTEX_INITIALIZER;
static inline void
spin_init(spinlock_t *slp)
{
	pthread_mutex_init(slp, NULL);
}

static inline void
spin_lock(spinlock_t *slp)
{
	pthread_mutex_lock(slp);
}

static inline void
spin_unlock(spinlock_t *slp)
{
	pthread_mutex_unlock(slp);
}

#elif defined SPINLOCK
#include "spinlock.h"
#elif defined TICKETLOCK
#include "ticketlock.h"
#elif defined MCSLOCK
#include "mcslock.h"
#else
_Static_assert(0,"A lock type must be defined.");
#endif

#endif /* __PICKLOCK_H */
