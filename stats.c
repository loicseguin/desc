#include <assert.h>
#include <errno.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include "dbg.h"
#include "stats.h"

#define MAX_LINELENGTH 100
#define BASE_DATA_SIZE 32
#define MICROSECS_PER_SEC 1000000

#define SWAP(a, b) tmp=(a); a=(b); (b)=tmp;

double _select(double *list, size_t n, size_t k);

dataset* init_empty_dataset(size_t n)
{
    dataset *ds;

    ds = (dataset*)malloc(sizeof(dataset));
    check_mem(ds);

    ds->data = (double*)malloc(n * sizeof(double));
    check_mem(ds->data);

    ds->sum = 0;
    ds->ss = 0;
    ds->n = 0;
    ds->has_q1 = false;
    ds->has_q3 = false;
    ds->has_minmax = false;
    ds->streaming = false;

    return ds;

error:
    if (ds) {
        if (ds->data) free(ds->data);
        free(ds);
    }
    return NULL;
}

dataset* create_dataset(double *array, size_t n)
{
    size_t i;
    dataset *ds;

    ds = init_empty_dataset(n);
    check(ds, "Failed to create dataset.");

    for (i = 0; i < n; i++) {
        ds->data[i] = array[i];
        ds->sum += array[i];
        ds->ss += array[i] * array[i];
    }
    ds->n = n;

    return ds;

error:
    return NULL;
}

void add_sample(dataset *ds, double sample)
{
    // Add a single data point to the dataset. Used in the streaming version of
    // desc that does not keep track of the data, only of summary statistics.
    if (ds->n > 0) {
        if (sample < ds->min)
            ds->min = sample;
        else if (sample > ds->max)
            ds->max = sample;
    } else {
        ds->min = sample;
        ds->max = sample;
    }
    ds->has_minmax = true;
    ds->n++;
    ds->sum += sample;
    ds->ss += sample * sample;
}

void delete_dataset(dataset *ds)
{
    check(ds, "Can't delete non existent dataset.");
    free(ds->data);
    free(ds);

error:
    return;
}

size_t _grow_data(double **data, size_t n)
{
    // Try to allocate more memory for a data array.
    // First, try to double the size of the array. If that fails, increment the
    // size of the array by the greatest possible multiple of BASE_DATA_SIZE.
    //
    // NOTE: It is necessary to pass a pointer to the data pointer because C is
    // pass by value. If only the data pointer is passed, the assignment
    // data = newdata is only local to this function and the data pointer in
    // the calling function is not changed. This will blow up because realloc
    // frees the memory pointed to by data.
    unsigned int ntry, max_tries;
    size_t next_size = n * 2;

    ntry = max_tries = n / BASE_DATA_SIZE - 1;

    double *newdata = (double*)realloc(*data, next_size * sizeof(double));
    while (!newdata && ntry > 0) {
        if (ntry == max_tries) {
            log_warn("The dataset is large and you are "
                     "running low on memory.");
        }
        next_size = n + ntry * BASE_DATA_SIZE;
        ntry--;
        newdata = (double*)realloc(*data, next_size * sizeof(double));
    }
    check_debug(newdata, "Memory error");
    *data = newdata;
    return next_size;

error:
    if (newdata) free(newdata);
    return (size_t)-1;
}

dataset* read_data_file(char *filename, bool streaming)
{
    dataset *ds = NULL;
    char buffer[MAX_LINELENGTH];
    double datum;
    size_t n = 0, cur_data_size = BASE_DATA_SIZE;
    FILE *fp;
    char *endptr;

    if (filename == NULL) {
        fp = stdin;
    } else {
        fp = fopen(filename, "r");
        // Make sure file was opened.
        check(fp, "Failed to open %s.", filename);
    }

    // Start by creating an empty dataset of small size.
    ds = init_empty_dataset(BASE_DATA_SIZE);
    check_mem(ds);
    if (streaming)
        ds->streaming = true;

    while(fgets(buffer, MAX_LINELENGTH, fp) != NULL) {
        datum = strtod(buffer, &endptr);

        if (errno == ERANGE) {
            // Overflow or underflow occured, warn the user but keep going.
            log_warn("Results might not be correct.");
        }

        if (endptr == buffer) {
            // No conversion was performed. Go to to next line.
            continue;
        }

        if (!streaming && n >= cur_data_size) {
            // If the current dataset size if not big enough, try to grow.
            cur_data_size = _grow_data(&(ds->data), cur_data_size);
            check(cur_data_size != (size_t)-1, "Could not grow data.");
        }

        if (!streaming) {
            ds->data[n] = datum;
            ds->sum += datum;
            ds->ss += datum * datum;
            n++;
        } else {
            add_sample(ds, datum);
        }
    }

    if (filename)
        fclose(fp);

    // Free the unused memory at the end of the data array.
    if (!streaming) {
        ds->data = (double*)realloc(ds->data, n * sizeof(double));
        check_mem(ds->data);
        ds->n = n;
    }

    return ds;

error:
    if (filename && fp) fclose(fp);
    if (ds) delete_dataset(ds);
    return NULL;
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
    if (ds->streaming) return NAN;
    double high, low;

    high = _select(ds->data, ds->n, ds->n / 2);
    if (ds->n % 2 == 0) {
        // Use slightly convoluted formula to avoid overflow.
        low = _select(ds->data, ds->n, ds->n / 2 - 1);
        return low + 0.5 * (high - low);
    } else {
        return high;
    }
}

double first_quartile(dataset *ds)
{
    // Compute the first quartile using selection.
    if (ds->streaming) return NAN;
    if (ds->has_q1)
        return ds->q1;
    ds->q1 = percentile(ds, 25.0);
    ds->has_q1 = true;
    return ds->q1;
}


double third_quartile(dataset *ds)
{
    // Compute the third quartile using selection.
    if (ds->streaming) return NAN;
    if (ds->has_q3)
        return ds->q3;
    ds->q3 = percentile(ds, 75.0);
    ds->has_q3 = true;
    return ds->q3;
}

double percentile(dataset *ds, double q)
{
    // Find the qth percentile where q is a float between 0 and 100.
    // Using q = 0 gives the minimum, q = 50, the median, q = 100 the maximum.
    //
    // Inspired by the implementation in Numpy
    // http://github.com/numpy/numpy/blob/v1.9.1/numpy/lib/function_base.py#L2947

    if (ds->streaming) return NAN;
    check_debug(ds->n > 1, "Can't compute percentile dataset with less than 2 elements.");
    double index = q * (ds->n - 1) / 100.0;
    double weight_above, low, high;
    size_t index_below = (size_t)index;
    size_t index_above = index_below + 1;

    if (index_above > ds->n - 1) {
        index_above = ds->n - 1;
    }

    weight_above = index - index_below;
    low = _select(ds->data, ds->n, index_below);
    high = _select(ds->data, ds->n, index_above);
    return low * (1 - weight_above) + high * weight_above;

error:
#ifdef NAN
    return NAN;
#else
    return 0;
#endif
}

double interquartile_range(dataset *ds)
{
    // The interquartile range is the distance between the first and the third
    // quartiles.
    if (ds->streaming) return NAN;
    return third_quartile(ds) - first_quartile(ds);
}

void _minmax(dataset *ds)
{
    // Find the max and the min in one pass.
    check_debug(ds->n > 0, "Can't compute minimum/maximum of empty dataset.");
    double _min = ds->data[0];
    double _max = ds->data[0];
    size_t i, n = ds->n;
    double *data = ds->data;

    for (i = 0; i < n; i++) {
        if (data[i] < _min)
            _min = data[i];
        else if (data[i] > _max)
            _max = data[i];
    }
    ds->min = _min;
    ds->max = _max;
    ds->has_minmax = true;
    return;

error:
#ifdef NAN
    ds->min = NAN;
    ds->max = NAN;
#else
    ds->min = 0;
    ds->max = 0;
#endif
    ds->has_minmax = true;
    return;
}

double min(dataset *ds)
{
    // Find the minimum in the array.
    if (!ds->has_minmax)
        _minmax(ds);
    return ds->min;
}

double max(dataset *ds)
{
    // Find the maximum in the array.
    if (!ds->has_minmax)
        _minmax(ds);
    return ds->max;
}

double _select(double *list, size_t n, size_t k)
{
    // Given a list of size n, find the kth smallest value in the list.
    // This algorithm is based on the one found in Press et al. Numerical
    // Recipes in C, 2nd edition.
    size_t i, j;
    size_t left, mid, right;
    double a, tmp;

    check_debug(n > 0, "Can't select from empty dataset.");
    left = 0;
    right = n - 1;
    while (true) {
        if (right <= left + 1) {
            if (right == left + 1 && list[right] < list[left]) {
                SWAP(list[left], list[right]);
            }
            return list[k];
        } else {
            mid = left + (right - left) / 2;
            SWAP(list[mid], list[left + 1]);
            if (list[left] > list[right]) {
                SWAP(list[left], list[right]);
            }
            if (list[left + 1] > list[right]) {
                SWAP(list[left + 1], list[right]);
            }
            if (list[left] > list[left + 1]) {
                SWAP(list[left], list[left + 1]);
            }
            i = left + 1;
            j = right;
            a = list[left + 1];
            while (true) {
                do {
                    i++;
                } while (list[i] < a);
                do {
                    j--;
                } while (list[j] > a);
                if (j < i) break;
                SWAP(list[i], list[j]);
            }
            list[left + 1] = list[j];
            list[j] = a;
            if (j >= k) right = j - 1;
            if (j <= k) left = i;
        }
    }

error:
#ifdef NAN
    return NAN;
#else
    return 0;
#endif
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
