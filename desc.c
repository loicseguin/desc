#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "dbg.h"
#include "stats.h"

void usage()
{
    fprintf(stderr,
            "usage: desc DATAFILE\n\n"
            "desc analyse the data in DATAFILE and prints summary statistics\n"
            "that describe the data.\n\n"
            "The input must consist of one number per line. If no DATAFILE is\n"
            "provided, desc reads from standard input which means it can be\n"
            "used with pipes.\n\n"
            "Examples\n"
            "--------\n"
            "cat data/large.dat | desc\n"
            );

    exit(1);
}


int main(int argc, char *argv[])
{
    int ch;
    bool streaming = false;

	while ((ch = getopt(argc, argv, "hs")) != -1)
		switch (ch) {
		case 'h':
            usage();
			break;
        case 's':
            streaming = true;
            break;
		default:
			usage();
		}
	argv += optind;

    dataset *ds = read_data_file(argv[0], streaming);
    if (!ds) {
        fprintf(stderr, "\n");
        usage();
        return 1;
    }

    printf("count     %zu\n", ds->n);
    printf("min       %.5g\n", min(ds));
    if (!streaming) printf("Q1        %.5g\n", first_quartile(ds));
    printf("mean      %.5g\n", mean(ds));
    if (!streaming) printf("median    %.5g\n", median(ds));
    if (!streaming) printf("Q3        %.5g\n", third_quartile(ds));
    printf("max       %.5g\n", max(ds));
    if (!streaming) printf("IQR       %.5g\n", interquartile_range(ds));
    printf("var       %.5g\n", var(ds));
    printf("sd        %.5g\n", sd(ds));
    
    return 0;
}
