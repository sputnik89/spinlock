/* 
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
 *
 * Copyright (c) 2005 Tom Hart.
 * Copyright (c) 2021 Angela Demke Brown
 */

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <stdatomic.h>
#include "test.h"
#include "util.h"
#include "random.h"

#include <sys/syscall.h>
#include <sys/sysinfo.h>
#include <sched.h>

/* Global variables defined in test.h, used by multiple files. */

/* Duration of the test, in milliseconds. */
unsigned long nmilli; 

/* Number of threads for the test. */
unsigned long nthreads;

/* The key to retrieve pthread-specific thread ID (TID). */
pthread_key_t g_tid;

/* A pointer to the data structure on which the threads will work.*/
void *data_structure;

/* Per-thread data; MAX_THREADS lets us statically allocate it. 
 * Note -- the "+1" is for the parent thread.
 */
struct per_thread threads[MAX_THREADS+1];


/* Global flag set by the parent thread. 
 * Controls when the worker threads start and stop.
 */
#define TEST_NOT_STARTED 0
#define TEST_RUNNING 1
#define TEST_OVER 2
atomic_int test_state = TEST_NOT_STARTED;

void test();

double totals [NUM_TRIALS];
pthread_attr_t attr;
#define LOTS 1000000
atomic_ulong n_finished;

/* Round robin threads across processors */
int setCPU (unsigned long n) {
        /* Set CPU affinity to CPU n % numprocs only, skipping 0. */
	unsigned long nprocs = get_nprocs();
        pid_t tid = gettid();
        cpu_set_t mask;
        CPU_ZERO(&mask);

	unsigned long runon = (n % (nprocs - 1)) + 1;
        CPU_SET(runon, &mask);
        if (sched_setaffinity(tid, sizeof(cpu_set_t), &mask) != 0) {
                perror("sched_setaffinity failed");
		fprintf(stderr,"Could not bind thread %lu to CPU %lu\n",
			n, runon);			
		return -1;
        }
	return 0;
}

/* Stop running test if we get an unexpected signal.
 */
void sighandler(int a)
{
	fprintf(stderr, "T%lu received signal %d\n", getTID(), a);
	while (1) {continue;};
}

static void *thread_start(void *arg)
{
	struct sigaction new_action;
	
	/* Record this thread's TID. */
	pthread_setspecific(g_tid,arg);

	/* Bind thread to CPU */
	if (setCPU((unsigned long)arg) != 0) {
		fprintf(stderr,"Thread binding failed, bailing.\n");
		exit(-1);
	}

	/* Catch SIGSEGV so we can print an error message */
	memset(&new_action, 0, sizeof(struct sigaction));
	new_action.sa_handler = sighandler;
	sigaction(SIGSEGV, &new_action, NULL);

	/* Seed the random number generator. */
	sRandom((unsigned long)((arg + 1) + (time(0) << 4)));

	/* Wait for the parent to start the test. */
	while (atomic_load(&test_state) == TEST_NOT_STARTED) ;


	/* Execute the test-dependent main loop. */
	while (atomic_load(&test_state) == TEST_RUNNING) {
		testloop_body();
	}

	(void)atomic_fetch_add(&n_finished, 1);
	pthread_exit(NULL);
}

int runtest(int iteration)
{
	pthread_t thread[MAX_THREADS];   /* Our array of threads. */
	unsigned long i;                 /* Iterator variable. */
	struct timespec expt_time;       /* Test time, parent can sleep. */
	struct timespec time_left;       /* In case parent woken early. */
	double start, end;               /* Start and end times for the test. */
	double total_ops;                /* Total operations by all threads. */

	/* Make sure the threads don't start prematurely. */
	atomic_store(&test_state, TEST_NOT_STARTED);
	atomic_store(&n_finished, 0);

	/* Create our threads. */
	pthread_attr_setstacksize (&attr, LOTS);
	for (i=0; i < nthreads; i++) {
		threads[i].op_count = 0;
		if ( pthread_create(&thread[i], &attr, thread_start, (void *)i) ) {
			fprintf(stderr, "Error creating thread %lu (%d)\n", i, errno);
			exit(1);
		}
	}

	/* Start the test. */
	start = d_gettime();
	atomic_store(&test_state, TEST_RUNNING);

	/* Go to sleep for the specified amount of time. */
	expt_time.tv_sec= nmilli / 1000;
	expt_time.tv_nsec= (nmilli % 1000) * NSEC_PER_MSEC;
	while ((i = clock_nanosleep(CLOCK_REALTIME, 0, &expt_time, &time_left)) == EINTR) {
		/* Interrupted. Reset requested sleep time and try again. */
		expt_time.tv_sec = time_left.tv_sec;
		expt_time.tv_nsec = time_left.tv_nsec;
	} 

	/* Stop the test; wait for threads to terminate. */
	atomic_store(&test_state, TEST_OVER);
	while (atomic_load(&n_finished) < nthreads) { }

	end = d_gettime();

	/* Record the average time per operation for this trial. */
	total_ops = 0;
	for ( i = 0; i < nthreads; i++ ) {
		total_ops += threads[i].op_count;
	}
	printf("total ops = %g\n", total_ops);
	totals[iteration] = ((end - start)*NSEC_PER_SEC)/total_ops;

	/* Join all the threads before exiting, to avoid thread leaks. */
        for (i = 0; i < nthreads; i++) {
                pthread_join(thread[i], NULL);
        }

	return 0;
}

int main (int argc, char **argv)
{
	int i;                            /* Iterator variable. */
	double av, std, mx, mn;           /* Statistics. */
	unsigned long pTID = MAX_THREADS; /* Parent's TID. */

	/* Parent needs a space for some freelists for initializations. */
	pthread_key_create(&g_tid, NULL);
	pthread_setspecific(g_tid, (void *)pTID);

	/* Bind parent thread to CPU 0 */
	pid_t tid = gettid();
        cpu_set_t mask;
        CPU_ZERO(&mask);
        CPU_SET(0, &mask);
        if (sched_setaffinity(tid, sizeof(cpu_set_t), &mask) != 0) {
                perror("parent sched_setaffinity failed, bailing");
		exit(-1);
        } 

	/* Initialize the random number generator. */
        init_Random();

	setup_test(argc, argv);

	for (i = 0; i < NUM_TRIALS; i++) {
		printf("Iteration %d\n", i);
		runtest(i);
	}

	av = avg(totals, NUM_TRIALS);
	std = stdev(totals, NUM_TRIALS);
	mx = max(totals, NUM_TRIALS);
	mn = min(totals, NUM_TRIALS);

	printf("%s: avg = %g  max = %g  min = %g  std = %g\n",
	       argv[0], av, mx, mn, std);
}
