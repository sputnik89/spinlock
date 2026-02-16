#include "util.h"
#include <math.h>
#include <stdlib.h>
#include <sys/mman.h>

double d_gettime() {
	int retval;
	struct timespec now;
	
	retval = clock_gettime(CLOCK_REALTIME, &now);
	if (retval != 0) {
		perror("clock_gettime");
		exit(-1);
	}
	return (now.tv_sec + ((double)now.tv_nsec) / 1000000000.);
}

double avg(double *data, int samples)
{
  int i;
  double sum = 0;

  for (i = 0; i < samples; i++)
    sum += data[i];

  return sum/(double)samples;
}

double stdev(double *data, int samples)
{
  int i;
  double sumsq = 0;
  double av;

  for (i = 0; i < samples; i++)
    sumsq += data[i] * data[i];
  av = avg(data, samples);

  return sqrt(sumsq/samples - av*av);
}

double max(double *data, int samples)
{
  int i;
  double m = data[0];

  for (i = 1; i < samples; i++)
    if (data[i] > m) m = data[i];

  return m;
}

double min(double *data, int samples)
{
  int i;
  double m = data[0];

  for (i = 1; i < samples; i++)
    if (data[i] < m) m = data[i];

  return m;
}

