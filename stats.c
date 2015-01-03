#include <assert.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include "stats.h"

#define MAX_LINELENGTH 100
#define BASE_DATA_SIZE 32

dataset* init_empty_dataset(size_t n)
{
    dataset *ds;

    ds = (dataset*)malloc(sizeof(dataset));
    if (ds == NULL) return NULL;

    ds->data = (double*)malloc(n * sizeof(double));
    if (ds->data == NULL) return NULL;

    ds->sum = 0;
    ds->ss = 0;
    ds->n = n;
    ds->sorted = false;
    ds->sorted_data = NULL;

    return ds;
}

dataset* create_dataset(double *array, size_t n)
{
    int i;
    dataset *ds;

    ds = init_empty_dataset(n);
    if (ds == NULL) return NULL;

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
    if (ds->sorted_data != NULL) {
        free(ds->sorted_data);
    }
    free(ds->data);
    free(ds);
}

dataset* read_data_file(char *filename) {
    FILE *fp = fopen(filename, "r");
    dataset *ds;
    char buffer[MAX_LINELENGTH];
    double datum;
    size_t n = 0, cur_data_size = BASE_DATA_SIZE;

    if (fp == NULL) {
        fprintf(stderr, "Can't open file %s\n", filename);
        return NULL;
    }

    ds = init_empty_dataset(BASE_DATA_SIZE);

    while(fgets(buffer, MAX_LINELENGTH, fp) != NULL) {
        datum = strtod(buffer, NULL);
        
        if (n >= cur_data_size) {
            cur_data_size += BASE_DATA_SIZE;
            ds->data = (double*)realloc(ds->data, cur_data_size * sizeof(double));
        }

        ds->data[n] = datum;
        ds->sum += datum;
        ds->ss += datum * datum;
        n++;
    }

    ds->n = n;

    return ds;
}

void print_array(double *arr, size_t n) {
    int i;
    for (i = 0; i < n; i++) {
        printf("%.3f", arr[i]);
        if (i < n - 1) {
            printf(", ");
        }
    }
    printf("\n");
}

void print_dataset(dataset *ds) {
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

double stddev(dataset *ds)
{
    return sqrt(var(ds));
}

int compare (const void *a, const void *b)
{
    if (*(double*)a < *(double*)b)
        return -1;
    else if (*(double*)a == *(double*)b)
        return 0;
    else
        return 1;
}

double median(dataset *ds)
{
    double _median;

    // Create a copy to avoid silently changing the ordering in the data. This
    // is less efficient memory-wise, but prevents surprises if the order of
    // the data is important.
    if (!ds->sorted) {
        ds->sorted_data = (double*)malloc(ds->n * sizeof(double));
        memcpy(ds->sorted_data, ds->data, ds->n * sizeof(double));
        qsort(ds->sorted_data, ds->n, sizeof(double), compare);
        ds->sorted = true;
    }

    if (ds->n % 2 == 0) {
        _median = 0.5 * (ds->sorted_data[(ds->n)/2 - 1] + ds->sorted_data[(ds->n)/2]);
    } else {
        _median = ds->sorted_data[(ds->n - 1) / 2];
    }
    return _median;
}

double timeit(double (*datafunc)(dataset *), dataset *ds) {
    double val;
    clock_t c1, c0 = clock();
    val = datafunc(ds);
    c1 = clock();
    fprintf(stdout, "%.3g us\n", 1000000.0 * (c1 - c0) / CLOCKS_PER_SEC);
    return val;
}
