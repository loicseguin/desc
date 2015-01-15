#include <assert.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include "stats.h"

#define MAX_LINELENGTH 100
#define BASE_DATA_SIZE 32
#define BINMEDIAN_NBBINS 1001
#define BINMEDIAN_C 20
#define MICROSECS_PER_SEC 1000000

#define SWAP(a, b) temp=a; a=b; b=temp;

double quickselect(double *arr, int n, int k);

dataset* init_empty_dataset(size_t n)
{
    dataset *ds;

    ds = (dataset*)malloc(sizeof(dataset));
    if (ds == NULL) {
        perror("stats.c->init_empty_dataset");
        exit(1);
    }

    ds->data = (double*)malloc(n * sizeof(double));
    if (ds->data == NULL) {
        perror("stats.c->init_empty_dataset");
        exit(1);
    }

    ds->sum = 0;
    ds->ss = 0;
    ds->n = n;

    return ds;
}

dataset* create_dataset(double *array, size_t n)
{
    int i;
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
    FILE *fp = fopen(filename, "r");
    dataset *ds;
    char buffer[MAX_LINELENGTH];
    double datum;
    size_t n = 0, cur_data_size = BASE_DATA_SIZE;

    if (fp == NULL) {
        perror(filename);
        exit(1);
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
    fclose(fp);

    return ds;
}

void print_array(double *arr, size_t n)
{
    int i;
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

double binmedian(dataset *ds)
{
    // Implement the binmedian algorithm in
    //
    //   Tibshirani, Ryan J. "Fast Computation of the Median by Successive
    //   Binning." arXiv Preprint arXiv:0806.3301, 2008.
    //   http://arxiv.org/abs/0806.3301.
    //
    // Code taken in part from R. Tibshirani's website
    // http://www.stat.cmu.edu/~ryantibs/median/binmedian.c
    //
    double mu = mean(ds);
    double sigma = stddev(ds);

    // Bin x across the interval [mu-sigma, mu+sigma]
    int bottomcount = 0;
    int bincounts[BINMEDIAN_NBBINS] = {0};
    double scalefactor = (BINMEDIAN_NBBINS - 1) / (2 * sigma);
    double leftend =  mu - sigma;
    double rightend = mu + sigma;
    int bin;
    size_t i;
    size_t n = ds->n;
    double *x = ds->data;

    for (i = 0; i < n; i++) {
        if (x[i] < leftend) {
            bottomcount++;
        }
        else if (x[i] < rightend) {
            bin = (int)((x[i] - leftend) * scalefactor);
            bincounts[bin]++;
        }
    }

    // If n is odd
    if (n & 1) {
        // Recursive step
        int k, r, count, medbin;
        float oldscalefactor, oldleftend;
        int oldbin;
        float temp;

        k = (n + 1) / 2;  // number of data points to the left of or at the median
        r = 0;

        while(true) {
            // Find the bin that contains the median, and the order
            // of the median within that bin
            count = bottomcount;
            for (i = 0; i < BINMEDIAN_NBBINS; i++) {
                count += bincounts[i];

                if (count >= k) {
                    medbin = i;
                    k = k - (count - bincounts[i]);
                    break;
                }
            }

            bottomcount = 0;
            for (i = 0; i < BINMEDIAN_NBBINS; i++) {
                bincounts[i] = 0;
            }
            oldscalefactor = scalefactor;
            oldleftend = leftend;
            scalefactor = (BINMEDIAN_NBBINS - 1) * oldscalefactor;
            leftend = medbin / oldscalefactor + oldleftend;
            rightend = (medbin + 1) / oldscalefactor + oldleftend;

            // Determine which points map to medbin, and put
            // them in spots r, ..., n - 1
            i = r;
            r = n;
            while (i < r) {
                oldbin = (int)((x[i] - oldleftend) * oldscalefactor);

                if (oldbin == medbin) {
                    r--;
                    SWAP(x[i],x[r]);

                    // Re-bin on a finer scale
                    if (x[i] < leftend) {
                        bottomcount++;
                    }
                    else if (x[i] < rightend) {
                        bin = (int)((x[i]-leftend) * scalefactor);
                        bincounts[bin]++;
                    }
                } else {
                    i++;
                }
            }

            // Stop if all points in medbin are the same
            bool samepoints = true;
            for (i = r + 1; i < n; i++) {
                if (x[i] != x[r]) {
                    samepoints = false;
                    break;
                }
            }
            if (samepoints) {
                return x[r];
            }

            // Stop if there's <= 20 points left
            if (n - r <= 20) {
                break;
            }
        }

        // Perform insertion sort on the remaining points,
        // and then pick the kth smallest
        double a;
        int j;
        for (i = r + 1; i < n; i++) {
            a = x[i];
            for (j = i - 1; j >= r; j--) {
                if (x[j] > a) {
                    break;
                }
                x[j + 1] = x[j];
            }
            x[j + 1] = a;
        }

        return x[r - 1 + k];
    }

    // If n is even (not implemented yet)
    else {
        return 0;
    }
}

double quickselect(double *arr, int n, int k)
{
    // From Numerical Recipes in C.
    unsigned long i, ir, j, l, mid;
    double a, temp;

    l = 0;
    ir = n-1;
    while (true) {
        if (ir <= l + 1) {
            if (ir == l + 1 && arr[ir] < arr[l]) {
                SWAP(arr[l], arr[ir]);
            }
            return arr[k];
        }
        else {
            mid = (l + ir) >> 1;
            SWAP(arr[mid], arr[l + 1]);
            if (arr[l] > arr[ir]) {
                SWAP(arr[l], arr[ir]);
            }
            if (arr[l + 1] > arr[ir]) {
                SWAP(arr[l + 1], arr[ir]);
            }
            if (arr[l] > arr[l + 1]) {
                SWAP(arr[l], arr[l + 1]);
            }
            i = l + 1;
            j = ir;
            a = arr[l + 1];
            while (true) {
                do i++; while (arr[i] < a);
                do j--; while (arr[j] > a);
                if (j < i) break;
                SWAP(arr[i], arr[j]);
            }
            arr[l + 1] = arr[j];
            arr[j] = a;
            if (j >= k) ir = j - 1;
            if (j <= k) l = i;
        }
    }
}

double median(dataset *ds)
{
    // Compute the median using the quickselect algorithm. Basically, partially
    // order the dataset using quik sort and stop whenever the first half of
    // the dataset if sorted.

    double _median;
    double *arr = (double*)malloc(ds->n * sizeof(double));
    double high, low;

    if (arr == NULL) {
        perror("stats.c->median");
        exit(1);
    }

    // Create a copy of the original array to avoid tampering with implicit
    // ordering of the data.
    memcpy(arr, ds->data, ds->n * sizeof(double));
    high = quickselect(arr, ds->n, ds->n / 2);
    if (ds->n % 2 == 1) {
        _median = high;
    } else {
        low = quickselect(arr, ds->n, ds->n / 2 - 1);
        _median = 0.5 * (low + high);
    }
    free(arr);
    return _median;
}

double timeit(double (*datafunc)(dataset *), dataset *ds) {
    // Time the duration of a function. If the function executes in less than
    // 0.1 s, run it multiple times and return the average execution time.

    clock_t c0, c1;
    double time_s, time_us;
    int i, niter = 1;

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
    time_us = MICROSECS_PER_SEC * time_s / niter;

    return time_us;
}
