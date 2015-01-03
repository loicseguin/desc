#include <stdio.h>
#include "stats.h"

int main(int argc, const char *argv[])
{
    double data[5] = {0.0, -4.0, 5.6, 7.2, 8.6};
    size_t n = 5;

    dataset *ds = create_dataset(data, n);
    if (ds == NULL) {
        fprintf(stderr, "ERROR: Failed to allocate memory.");
        return 1;
    }

    printf("Mean:               %6.3f\n", mean(ds));
    printf("Variance:           %6.3f\n", var(ds));
    printf("Standard deviation: %6.3f\n", stddev(ds));
    printf("Median:             %6.3f\n", median(ds));

    printf("Data: ");
    print_dataset(ds);

    delete_dataset(ds);

    ds = read_data_file("test.dat");
    if (ds == NULL) {
        exit(1);
    }
    printf("Mean:               %6.3f\n", mean(ds));
    printf("Time for mean: ");
    timeit(mean, ds);
    printf("Variance:           %6.3f\n", var(ds));
    printf("Time for variance: ");
    timeit(var, ds);
    printf("Standard deviation: %6.3f\n", stddev(ds));
    printf("Time for standard deviation: ");
    timeit(stddev, ds);
    printf("Time for median: ");
    timeit(median, ds);
    printf("Median:             %6.3f\n", median(ds));
    printf("Time for median: ");
    timeit(median, ds);

    printf("Data size: %zu\n", ds->n);

    delete_dataset(ds);

    return 0;
}
