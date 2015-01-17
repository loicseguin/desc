#include <assert.h>
#include <errno.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include "sort.h"
#include "stats.h"

#define MAX_LINELENGTH 100
#define BASE_DATA_SIZE 32
#define MICROSECS_PER_SEC 1000000

dataset* init_empty_dataset(size_t n)
{
    dataset *ds;

    ds = (dataset*)malloc(sizeof(dataset));
    if (ds == NULL) {
        perror("stats.c:init_empty_dataset");
        exit(1);
    }

    ds->data = (double*)malloc(n * sizeof(double));
    if (ds->data == NULL) {
        perror("stats.c:init_empty_dataset");
        exit(1);
    }

    ds->sum = 0;
    ds->ss = 0;
    ds->n = n;
    ds->sorted = false;

    return ds;
}

dataset* create_dataset(double *array, size_t n)
{
    size_t i;
    dataset *ds;

    ds = init_empty_dataset(n);

    for (i = 0; i < n; i++) {
        ds->data[i] = array[i];
        ds->sum += array[i];
        ds->ss += array[i] * array[i];
    }

    return ds;
}

void delete_dataset(dataset *ds)
{
    assert(ds != NULL);
    free(ds->data);
    free(ds);
}

dataset* read_data_file(char *filename) {
    dataset *ds;
    double *newdata;
    char buffer[MAX_LINELENGTH];
    double datum;
    size_t n = 0, cur_data_size = BASE_DATA_SIZE, next_size;
    unsigned int ntry;
    FILE *fp;

    if (filename == NULL) {
        fp = stdin;
    } else {
        fp = fopen(filename, "r");
    }

    // Make sure file was opened.
    if (fp == NULL) {
        perror(filename);
        exit(1);
    }

    // Start by creating an empty dataset of small size.
    ds = init_empty_dataset(BASE_DATA_SIZE);

    while(fgets(buffer, MAX_LINELENGTH, fp) != NULL) {
        datum = strtod(buffer, NULL);

        if (errno == ERANGE) {
            // Overflow or underflow occured, warn the user but keep going.
            perror("Warning");
        }

        if (n >= cur_data_size) {
            // If the current dataset size if not big enough, try to grow.
            ntry = 1;
            next_size = cur_data_size * 2;
            newdata = (double*)realloc(ds->data, next_size * sizeof(double));
            while (newdata == NULL && ntry < cur_data_size / BASE_DATA_SIZE) {
                perror("Warning");
                fprintf(stderr, "It seems the dataset is large and you are "
                                "running low on memory.");
                next_size = cur_data_size + BASE_DATA_SIZE;
                newdata = (double*)realloc(ds->data, next_size * sizeof(double));
            }
            if (newdata == NULL) {
                fprintf(stderr, "Error: failed to allocate memory.");
                exit(1);
            }
            ds->data = newdata;
            cur_data_size = next_size;
        }

        ds->data[n] = datum;
        ds->sum += datum;
        ds->ss += datum * datum;
        n++;
    }

    if (filename != NULL)
        fclose(fp);

    // Free the unused memory at the end of the data array.
    ds->data = (double*)realloc(ds->data, n * sizeof(double));
    ds->n = n;

    return ds;
}

void print_array(double *arr, size_t n)
{
    size_t i;
    for (i = 0; i < n; i++) {
        printf("%.3f", arr[i]);
        if (i < n - 1) {
            printf(", ");
        }
    }
    printf("\n");
}

void print_dataset(dataset *ds)
{
    print_array(ds->data, ds->n);
}

double mean(dataset *ds)
{
    return ds->sum / ds->n;
}

double var(dataset *ds)
{
    return (ds->ss - ds->sum * ds->sum / ds->n) / (ds->n - 1);
}

double sd(dataset *ds)
{
    return sqrt(var(ds));
}

double median(dataset *ds)
{
    // Compute the median using selection. This could be done using the
    // percentile function, but here we only perform one select call for arrays
    // of odd length.
    double high, low;

    high = select(ds->data, ds->n, ds->n / 2);
    if (ds->n % 2 == 0) {
        // Use slightly convoluted formula to avoid overflow.
        low = select(ds->data, ds->n, ds->n / 2 - 1);
        return low + 0.5 * (high - low);
    } else {
        return high;
    }
}

double first_quartile(dataset *ds)
{
    // Compute the first quartile using selection.
    return percentile(ds, 25.0);
}


double third_quartile(dataset *ds)
{
    // Compute the third quartile using selection.
    return percentile(ds, 75.0);
}

double percentile(dataset *ds, double q)
{
    // Find the qth percentile where q is a float between 0 and 100.
    // Using q = 0 gives the minimum, q = 50, the median, q = 100 the maximum.
    //
    // Inspired by the implementation in Numpy
    // http://github.com/numpy/numpy/blob/v1.9.1/numpy/lib/function_base.py#L2947

    double index = q * (ds->n - 1) / 100.0;
    double weight_above, low, high;
    int index_below = (int)index;
    int index_above = index_below + 1;

    if (index_above > ds->n - 1) {
        index_above = ds->n - 1;
    }

    weight_above = index - index_below;
    low = select(ds->data, ds->n, index_below);
    high = select(ds->data, ds->n, index_above);
    return low * (1 - weight_above) + high * weight_above;
}

double interquartile_range(dataset *ds)
{
    // The interquartile range is the distance between the first and the third
    // quartiles.
    return third_quartile(ds) - first_quartile(ds);
}

double min(dataset *ds)
{
    // Find the minimum in the array.
    double _min = ds->data[0];
    size_t i, n = ds->n;
    double *data = ds->data;

    for (i = 1; i < n; i++) {
        if (data[i] < _min)
            _min = data[i];
    }

    return _min;
}

double max(dataset *ds)
{
    // Find the maximum in the array.
    double _max = ds->data[0];
    size_t i, n = ds->n;
    double *data = ds->data;

    for (i = 1; i < n; i++) {
        if (data[i] > _max)
            _max = data[i];
    }

    return _max;
}

double timeit(double (*datafunc)(dataset *), dataset *ds, int n) {
    // Time the duration of a function. If the function executes in less than
    // 0.1 s, run it multiple times and return the average execution time.

    clock_t c0, c1;
    double time_s, time_us;
    int i, niter = 1;

    if (n != 0) {
        c0 = clock();
        for (i = 0; i < n; i++) {
            datafunc(ds);
        }
        c1 = clock();
        time_s = (double)(c1 - c0) / CLOCKS_PER_SEC;
    } else {
        do {
            c0 = clock();
            for (i = 0; i < niter; i++) {
                datafunc(ds);
            }
            c1 = clock();
            time_s = (double)(c1 - c0) / CLOCKS_PER_SEC;
            niter *= 10;
        } while (time_s < 0.1);

        niter /= 10;
    }

    time_us = MICROSECS_PER_SEC * time_s / niter;
    return time_us;
}
