/* holes - find holes (= runs of zero bytes) in input files */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

ssize_t minlen = 64;
int ret;
char *argv0;

void
holes(FILE *input, char *filename)
{
	off_t offset = 0;
	off_t run = 0;	
	int ch;

	while ((ch = getc_unlocked(input)) != EOF) {
		if (ch == 0) {
			run++;
		} else {
			if (run >= minlen) {
				if (filename)
					printf("%s: ", filename);
				printf("%08lx %ld\n", offset - run, run);
			}
			run = 0;
		}
		offset++;
	}
	if (ferror(input)) {
		fprintf(stderr, "%s: can't read '%s': %s\n",
		    argv0, filename ? filename : "-", strerror(errno));
		ret = 1;
		return;
	}

	if (offset == 0 ||	// empty file
	    run >= minlen) {
		if (filename)
			printf("%s: ", filename);
		printf("%08lx %ld\n", offset - run, run);
	}
}

int
main(int argc, char *argv[])
{
	int c, i;

	argv0 = argv[0];

	while ((c = getopt(argc, argv, "n:")) != -1)
		switch(c) {
                case 'n':
			minlen = atoll(optarg);
			break;
                default:
                        fprintf(stderr,
			    "Usage: %s [-n MINLEN] [FILES...]\n", argv0);
			exit(2);
		}

	if (minlen < 1) {
		fprintf(stderr, "%s: MINLEN must not be smaller than 1.\n",
		    argv0);
		exit(2);
	}

	if (optind == argc)
		holes(stdin, 0);
	else
		for (i = optind; i < argc; i++) {
			FILE *input = (strcmp(argv[i], "-") == 0) ?
			    stdin : fopen(argv[i], "rb");
			
			if (!input) {
				fprintf(stderr, "%s: can't open file '%s': %s\n",
				    argv0, argv[i], strerror(errno));
				ret = 1;
			} else {
				holes(input, argc - optind > 1 ? argv[i] : 0);
			}
		}

	return ret;
}