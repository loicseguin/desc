#ifndef STATS_H
#define STATS_H

#include <stdlib.h>

typedef struct dataset {
    double *data;
    size_t n;
    double sum;
    double ss;
} dataset;

dataset* create_dataset(double *array, size_t n);
dataset* read_data_file(char *filename);
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
