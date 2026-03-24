#ifndef RANDOM_H
#define RANDOM_H

/* Capitalize "Random" so we don't conflict with stdlib.h. */

#define PRNG_BUFFER_SIZE 32 /* see man pg for initstate(), must be at least 8 */

void init_Random();
void sRandom(unsigned long);
unsigned long Random();

#endif
