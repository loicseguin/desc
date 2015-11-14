#include <stdio.h>
#include "dbg.h"
#include "stats.h"

void usage()
{
    fprintf(stderr, "usage: timings DATAFILE\n"
                    "       cat DATAFILE | timings\n"
                    "  DATAFILE has to contain a list of numbers, one per line.\n");
}

void timings(dataset *ds, char *filename)
{
    char *input;
    if (filename)
        input = filename;
    else
        input = "stdin";

    fprintf(stderr, "Timings for %s\n", input);
    fprintf(stderr, "  mean                %.3g µs\n", timeit(mean, ds, 0));
    fprintf(stderr, "  variance            %.3g µs\n", timeit(var, ds, 0));
    fprintf(stderr, "  standard deviation  %.3g µs\n", timeit(sd, ds, 0));
    fprintf(stderr, "  median              %.3g µs\n", timeit(median, ds, 1));
    fprintf(stderr, "  Q1                  %.3g µs\n", timeit(first_quartile, ds, 1));
    fprintf(stderr, "  Q3                  %.3g µs\n", timeit(third_quartile, ds, 1));
    fprintf(stderr, "  IQR                 %.3g µs\n", timeit(interquartile_range, ds, 1));
    fprintf(stderr, "  min                 %.3g µs\n", timeit(min, ds, 1));
    fprintf(stderr, "  max                 %.3g µs\n", timeit(max, ds, 1));
}

int main(int argc, char *argv[])
{
    dataset *ds;

    ds = read_data_file(argv[1], false);
    check(ds, "Could not read dataset.");
    if (ds->n < 2)
        log_warn("\n------ Less than two values in dataset: "
                 "most results will be wrong.");
    if (argc > 1)
        timings(ds, argv[1]);
    delete_dataset(ds);

    return 0;

error:
    usage();
    return 1;
}
