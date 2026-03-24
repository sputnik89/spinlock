/*
 * Utility functions used by the program.
 */

#ifndef UTIL_H
#define UTIL_H

#include <time.h>
#include <stdio.h>
#include <errno.h>

/* Get time of day as a decimal fraction. Units are seconds.
 */

double d_gettime();

double avg(double *data, int samples);
double stdev(double *data, int samples);
double max(double *data, int samples);
double min(double *data, int samples);

#endif
