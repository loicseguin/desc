#ifndef STATS_H
#define STATS_H

#include <stdbool.h>
#include <stdlib.h>

typedef struct dataset {
    double *data;
    double *sorted_data;
    bool sorted;
    size_t n;
    double sum;
    double ss;
} dataset;

dataset* create_dataset(double *array, size_t n);
dataset* read_data_file(char *filename);
void delete_dataset(dataset *ds);
void print_array(double *arr, size_t n);
void print_dataset(dataset *ds);
double mean(dataset *ds);
double var(dataset *ds);
double stddev(dataset *ds);
double median(dataset *ds);
double timeit(double (*datafunc)(dataset *), dataset *ds);

#endif
