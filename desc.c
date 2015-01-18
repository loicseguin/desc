#include <stdio.h>
#include "dbg.h"
#include "stats.h"

int main(int argc, char *argv[])
{
    dataset *ds = read_data_file(argv[1]);
    if (!ds)
        return 1;

    printf("count     %zu\n", ds->n);
    printf("min       %.5g\n", min(ds));
    printf("Q1        %.5g\n", first_quartile(ds));
    printf("mean      %.5g\n", mean(ds));
    printf("median    %.5g\n", median(ds));
    printf("Q3        %.5g\n", third_quartile(ds));
    printf("max       %.5g\n", max(ds));
    printf("IQR       %.5g\n", interquartile_range(ds));
    printf("var       %.5g\n", var(ds));
    printf("sd        %.5g\n", sd(ds));
    
    return 0;
}
