#ifndef TEST_H
#define TEST_H

#include <pthread.h>
#include <stdlib.h>
#include <stdatomic.h>
#include "random.h"

/****************************************************************************
 *
 *   Definitions of constants.
 *
 ****************************************************************************/

#ifndef NULL
#define NULL ((void *)0)
#endif

#define MAX_THREADS   32
#define MAX_TIME      100000 /* 100k milliseconds == 100 seconds */
#define NUM_TRIALS    5
#define NSEC_PER_SEC  1000000000
#define NSEC_PER_MSEC 1000000
#define NSEC_PER_USEC 1000
#define CACHESIZE     128           /* Should cover most machines. */


/****************************************************************************
 *
 *   Data structures.
 *
 ****************************************************************************/

/* For MCS locks, each thread needs a qnode structure, and it is easiest to 
 * put that in the per_thread struct. So we'll define the struct qnode here
 * and let mcslock.h include test.h.
 */
struct qnode {
	atomic_bool locked;
	struct qnode * _Atomic next;
};

struct per_thread {
	double op_count;

	/* For thread-specific random number seeds */
	struct random_data rnd_databuf;
	char rnd_statebuf[PRNG_BUFFER_SIZE];
	
	/* Just include data for all the lock schemes. */

	/* MCS locks need a per-thread qnode structure */
	struct qnode my_node;

};

/****************************************************************************
 *
 *   Variables common to all tests.
 *
 ****************************************************************************/

/* Duration of the test, in milliseconds. */
extern unsigned long nmilli;

/* Number of threads for the test. */
extern unsigned long nthreads;

/* The key to retrieve pthread-specific thread ID (TID). */
extern pthread_key_t g_tid;

/* A pointer to the data structure on which the threads will work. We're using 
 * C's weak typing as poor man's polymorphism.
 */
extern void *data_structure;

/* Per-thread data; MAX_THREADS lets us statically allocate it. 
 * Note -- the "+1" is for the parent thread.
 */
extern struct per_thread threads[MAX_THREADS+1];

/****************************************************************************
 *
 *   Utility macros and functions.
 *
 ****************************************************************************/

/* NB: It looks like calling pthread_getspecific() has about 20ns overhead.
 *     this sort of thing can make a difference as to which algoithm "wins"
 *     in some situations.
 */
#define getTID() ((unsigned long)pthread_getspecific(g_tid))

/****************************************************************************
 *
 *   Interface to test functions.
 *
 ****************************************************************************/

/* Sets up the test, in a test-dependent way. Parses the argv. Instantiates 
 * data_structure to point to the right data structure.
 */
void setup_test(int argc, char **argv);

/* Different data structures -- ie. linked list, queue -- must  provide 
 * implementations of this. It's the workhorse function of the performance test.
 */
void testloop_body();

#endif
