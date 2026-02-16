/*-
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	from: @(#)random.c	8.1 (Berkeley) 6/10/93
 *      $Id: random.c,v 1.3 2005/06/18 18:26:32 tomhart Exp $
 */

/*
 * Modified 4/10/2003 to allow parallel operation.
 * Copyright (c) 2003 IBM Corporation.
 *
 * Modified 22/10/2020 to use GNU C re-entrant random functions.
 */

#include <string.h>
#include "test.h"

/*
 * Initialize the random-number generator.
 * Assumes nthreads is not yet known, so uses MAX_THREADS.
 * All per-thread state is stored in the struct per_thread threads[].
 */
void init_Random()
{
	//	assert(nthreads > 0 && nthreads <= MAX_THREADS);
	
	/* Associate a state buffer with each of the random_data buffers.
	 * Just use the same seed for all threads in initstate_r(). Tests can
	 * use sRandom() to assign a per-thread seed if they wish.
	 */ 
	for (int i = 0; i <= MAX_THREADS; i++) {
		struct random_data *rd_buf = &threads[i].rnd_databuf;
		char *r_state = threads[i].rnd_statebuf;
		memset(rd_buf, 0, sizeof(struct random_data));
		memset(r_state, 0, PRNG_BUFFER_SIZE);
		initstate_r(0, r_state, PRNG_BUFFER_SIZE, rd_buf);
	}
		
}

/*
 * Set the seed for the current thread.
 * Assumes per-thread rnd_statebuf and rnd_databuf already initialized.
 */
void sRandom(unsigned long seed)
{
	//assert(rnd_statebufs != NULL && rnd_databufs != NULL);
	srandom_r(seed, &threads[getTID()].rnd_databuf);
}

/*
 * Thread-safe pseudo-random number generator for randomizing the profiling 
 * clock, and whatever else we might use it for.  
 * Now just uses GNU C's random_r() function.
 */
unsigned long Random()
{
	int result;
	random_r(&threads[getTID()].rnd_databuf, &result);
	return (unsigned long)result;
}
