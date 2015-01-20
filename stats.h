#ifndef STATS_H
#define STATS_H

#include <stdbool.h>
#include <stdlib.h>

typedef struct dataset {
    double *data;
    size_t n;
    double sum;
    double ss;
    double q1;
    double q3;
    double min;
    double max;
    bool has_q1;
    bool has_q3;
    bool has_minmax;
    bool streaming;
} dataset;

dataset* create_dataset(double *array, size_t n);
dataset* read_data_file(char *filename, bool streaming);
void delete_dataset(dataset *ds);
double mean(dataset *ds);
double var(dataset *ds);
double sd(dataset *ds);
double median(dataset *ds);
double percentile(dataset *ds, double q);
double first_quartile(dataset *ds);
double third_quartile(dataset *ds);
double interquartile_range(dataset *ds);
double min(dataset *ds);
double max(dataset *ds);
double timeit(double (*datafunc)(dataset *), dataset *ds, int n);

#endif
