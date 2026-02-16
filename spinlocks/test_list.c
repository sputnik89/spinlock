#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include "test.h"
#include "list.h"
#include "random.h"

unsigned long nsearch;
unsigned long nmodify;
unsigned long nelements;
unsigned long nkeys;

void testloop_body()
{
	int i;
	int n = 100;
	/* Get all our randomness out of r and split it up. 
	 * Should have enough bits
	 */
	unsigned long r;
	unsigned long action;
	unsigned long ins_del;
	long key;

	struct list *l = (struct list *)data_structure;

	for (i = 0; i < n; i++) {
		r = Random();
		ins_del = r & 1;
		r >>= 1;
		action = ((r & ((1 << 10) - 1)) % (nsearch+nmodify)) + 1;
		r >>= 10;
		key = r % nkeys;

		if (action <= nsearch) {
			search(l, key);
		} else if (ins_del == 0) {
			delete(l, key);
		} else {
			insert(l, key);
		}
	}

	/* Record another n operations. */
	threads[getTID()].op_count += n;
}

void usage(char *progname)
{
	fprintf(stderr, "Usage: %s -t <num millisecs> -u <update fraction> -e <num elements> -n <num threads>\n", progname);
	exit(-1);
}


void parse_arguments(int argc, char **argv) {

	int opt;
	double update_fraction;
	
	/* Defaults */
	
	/* Run for 10s. Change test run time with -t <num milliseconds> */
	nmilli = 10000;
	
	/* 50/50 search/modify operations. Change with -u <update fraction> */
	update_fraction = 0.5;

	/* Number of elements in the list initially. Change with -e <nelem> */
	nelements = 10;

	/* Number of threads. Change with -n <num threads> */
	nthreads = 2;
	
	while ((opt = getopt(argc, argv, "t:u:e:n:h")) != -1) {
		switch (opt) {
		case 't':
			errno = 0;
			nmilli = strtoul(optarg, NULL, 0);
			if (errno != 0) {
				perror("Error converting -t argument:");
				usage(argv[0]);
				exit(1);
			}
			if (nmilli == 0 || nmilli > MAX_TIME) {
				fprintf(stderr,"Argument to -t must be between 1 and %u milliseconds.\n", MAX_TIME);
				usage(argv[0]);
				exit(1);
			}
			break;
		case 'u':
			errno = 0;
			update_fraction = strtod(optarg, NULL);
			if (errno != 0) {
				perror("Error converting -u argument:");
				usage(argv[0]);
				exit(1);
			}
			if (update_fraction < 0.0 || update_fraction > 1.0) {
				fprintf(stderr,"Argument to -u must be between 0 and 1\n");
				usage(argv[0]);
				exit(1);
			}
			break;
		case 'e':
			errno = 0;
			nelements = strtoul(optarg, NULL, 0);
			if (errno != 0) {
				perror("Error converting -e argument:");
				usage(argv[0]);
				exit(1);
			}
			/* Allow any number of elements? */
			break;
		case 'n':
			errno = 0;
			nthreads = strtoul(optarg, NULL, 0);
			if (errno != 0) {
				perror("Error converting -n argument:");
				usage(argv[0]);
				exit(1);
			}
			if (nthreads < 1 || nthreads > MAX_THREADS) {
				printf("Argument to -n must be between 1 and %u\n", MAX_THREADS);
				usage(argv[0]);
				exit(1);
			}
			break;
		case 'h':
			fprintf(stderr,"HELP REQUESTED\n");
			usage(argv[0]);
			break;
		case '?':
			fprintf(stderr,"UNRECOGNIZED ARGUMENT\n");
			usage(argv[0]);
			break;
		default:
			fprintf(stderr,"Unexpected error parsing args\n");
			usage(argv[0]);
			break;		      
		}
	}

	/* Fix up parameters that depend on arguments */
	nkeys = nthreads * 2;
	nmodify = (1 << 10) * update_fraction;
	nsearch = (1 << 10) - nmodify;

}

void setup_test(int argc, char **argv)
{
	int i;

	if (argc > 10) {
		usage(argv[0]);
	}

	parse_arguments(argc, argv);
	
	/* Initialize data_structure. */
	list_init( (struct list **) &data_structure );

	/* Populate our list. */
	for (i = 0; i < nelements; i++)
		insert((struct list *)data_structure, Random() % nkeys);

	/* Summarize our parameters. */
	printf("%s: nmilli: %lu update/total: %lu/%lu nelements: %lu nthreads: %lu\n",
		  argv[0], nmilli, nmodify, nsearch + nmodify, nelements, nthreads);

}
