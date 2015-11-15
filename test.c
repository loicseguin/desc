#include <math.h>
#include <stdio.h>
#include "tdigest.h"
#include "dbg.h"
#include "stats.h"

#define EPSILON 1e-8


#define mu_start() char *message = NULL

#define mu_run_test(test) debug("\n-----%s", " " #test); \
    message = test(); tests_run++; if (message) return message

#define mu_assert(test, message) if (!(test)) { log_err(message); return message; }

#define test_statistic(stat, i, tol) mu_assert(check_answer(stat(ds), answer[i], tol), \
                                          "failed to compute " #stat ".");

#define test_dataset(tol) char *msg = test_describe(ds, answer, tol); \
                       delete_dataset(ds); if (msg) return msg; return NULL;

int tests_run = 0;


int check_answer(double computed, double answer, double tol)
{
    if (isnan(computed) && isnan(answer)) {
        return 1;
    }
    if (computed < answer + tol && computed > answer - tol) {
        return 1;
    } else {
        fprintf(stderr, "[ERROR] expected %.9g, got %.9g\n", answer, computed);
        return 0;
    }
}

char *test_describe(dataset *ds, double *answer, double tol)
{
    test_statistic(median, 0, tol);
    test_statistic(mean, 1, tol);
    test_statistic(var, 2, tol);
    test_statistic(sd, 3, tol);
    test_statistic(first_quartile, 4, tol);
    test_statistic(third_quartile, 5, tol);
    test_statistic(interquartile_range, 6, tol);
    test_statistic(min, 7, tol);
    test_statistic(max, 8, tol);
    return NULL;
}

char *test_odd1()
{
    double data[7] = {9.4, 2.1, -6.5, 34.2, 3.34, 67.5, 8.64};
    size_t n = 7;
    double answer[9] = {8.64, 16.954285714285714, 655.76342857142856,
                        25.607878252042447, 2.72, 21.8, 19.08, -6.5, 67.5};

    dataset *ds = create_dataset(data, n);
    test_dataset(EPSILON);
}

char *test_odd2()
{
    double data[101] = {
        0.43237019,  0.89965341,  0.65736954,  0.29787347,  0.97940454,
        0.68124639,  0.10636585,  0.26271744,  0.32726637,  0.84379331,
        0.25945488,  0.36626302,  0.28545607,  0.96567511,  0.05816944,
        0.48142022,  0.46270003,  0.35002848,  0.48814735,  0.29582646,
        0.24433933,  0.43193555,  0.33852267,  0.47881679,  0.53685681,
        0.02080174,  0.69240203,  0.00851312,  0.67130511,  0.91025214,
        0.76341898,  0.97894942,  0.79185358,  0.70467176,  0.50039602,
        0.78577565,  0.38596683,  0.17184017,  0.3115971 ,  0.64044818,
        0.42768467,  0.70312878,  0.64437839,  0.82133105,  0.03765682,
        0.50174467,  0.8016999 ,  0.91263549,  0.00700852,  0.47038832,
        0.58153316,  0.21695557,  0.1800288 ,  0.75913846,  0.76955199,
        0.32775094,  0.89597533,  0.64899747,  0.76109793,  0.08578548,
        0.6346401 ,  0.9931002 ,  0.41520079,  0.87434325,  0.09935491,
        0.50075671,  0.86616643,  0.87013226,  0.06687713,  0.81737008,
        0.3404168 ,  0.01019728,  0.03138638,  0.29660639,  0.88462848,
        0.02436522,  0.40903969,  0.73306587,  0.47339932,  0.36121191,
        0.29101213,  0.58949457,  0.14527204,  0.84933733,  0.34961894,
        0.76631946,  0.04105252,  0.56109999,  0.87730681,  0.65769674,
        0.62805237,  0.69491854,  0.15972331,  0.33908443,  0.07314813,
        0.18694128,  0.08702012,  0.54743775,  0.1845326 ,  0.56460996,
        0.81548316};
    size_t n = 101;

    double answer[9] = {0.48142021970636995,
                        0.4904530462376237, 0.08335251150960539,
                        0.28870835025957492, 0.28545607, 0.75913846,
                        0.47368239, 0.00700852, 0.9931002};

    dataset *ds = create_dataset(data, n);
    test_dataset(EPSILON);
}

char *test_odd3()
{
    dataset *ds = read_data_file("data/odd.dat", false);

    double answer[9] = {0.50382482225561787,
                        0.50233294716282062, 0.082897444134665946,
                        0.28791916249993843, 0.25413486746800,
                        0.75404246086600, 0.499907593398,
                        0.00019540681109, 0.99961258962500};
    test_dataset(EPSILON);
}

char *test_odd3_streaming()
{
    dataset *ds = read_data_file("data/odd.dat", true);

    double answer[9] = {0.50382482225561787,
                        0.50233294716282062, 0.082897444134665946,
                        0.28791916249993843, 0.25413486746800,
                        0.75404246086600, 0.499907593398,
                        0.00019540681109, 0.99961258962500};
    test_dataset(1e-2);
}

char *test_odd4()
{
    double data[7] = {8.64, 9.4, 2.1, -6.5, 34.2, 3.34, 67.5};
    size_t n = 7;
    double answer[9] = {8.64, 16.954285714285714, 655.76342857142856,
                        25.607878252042447, 2.72, 21.8, 19.08, -6.5, 67.5};

    dataset *ds = create_dataset(data, n);
    test_dataset(EPSILON);
}

char *test_odd5()
{
    double data[11] = {6, 7, 15, 36, 39, 40, 41, 42, 43, 47, 49};
    size_t n = 11;
    double answer[9] = {40.0, 33.181818182, 251.96363636364, 15.873362478178,
                        25.5, 42.5, 17.0, 6.0, 49.0};

    dataset *ds = create_dataset(data, n);
    test_dataset(EPSILON);
}

char *test_even1()
{
    double data[8] = {4.3, 9.4, 2.1, -6.5, 34.2, 3.34, 67.5, 8.64};
    size_t n = 8;
    double answer[9] = {6.47, 15.3725, 582.09930714285713,
                        24.126734282593183, 3.03, 15.6, 12.57, -6.5, 67.5};

    dataset *ds = create_dataset(data, n);
    test_dataset(EPSILON);
}

char *test_even2()
{
    double data[100] = {
        0.27564984,  0.76216363,  0.17482944,  0.42485547,  0.26271342,
        0.08006306,  0.44788837,  0.68474791,  0.24736164,  0.82006618,
        0.92724928,  0.94786928,  0.31081874,  0.63527509,  0.54334513,
        0.35131215,  0.31973438,  0.19724116,  0.05609514,  0.94613421,
        0.58635689,  0.08175372,  0.55780855,  0.2621798 ,  0.07965311,
        0.91239451,  0.6968671 ,  0.24828693,  0.9034816 ,  0.3625052 ,
        0.1786429 ,  0.9241733 ,  0.41879173,  0.9675375 ,  0.22868655,
        0.32843568,  0.14586676,  0.03701525,  0.7135808 ,  0.34343475,
        0.71237876,  0.91353507,  0.55883932,  0.18121403,  0.44049463,
        0.42779077,  0.083098  ,  0.51829406,  0.61061055,  0.56288862,
        0.45446935,  0.46948462,  0.81136468,  0.08685398,  0.95479866,
        0.13773993,  0.80417514,  0.59397999,  0.45370185,  0.02643887,
        0.67834174,  0.17681015,  0.81512603,  0.69894483,  0.49324567,
        0.43104551,  0.24529282,  0.2186107 ,  0.36801189,  0.51962121,
        0.70257311,  0.87483376,  0.19393657,  0.52055607,  0.96756828,
        0.12514523,  0.29526195,  0.58954618,  0.7091562 ,  0.59563842,
        0.8050604 ,  0.44021433,  0.01790693,  0.98626531,  0.32092172,
        0.61701703,  0.86518485,  0.28019361,  0.82510572,  0.37655538,
        0.5342091 ,  0.19874851,  0.04020226,  0.87420673,  0.30328376,
        0.83395533,  0.86146147,  0.2151674 ,  0.86681485,  0.85074555};
    size_t n = 100;

    double answer[9] = {0.461976985, 0.4952750359,
                        0.08234955831149722, 0.28696612746367334,
                        0.2480556075, 0.7257265075, 0.4776709,
                        0.0179069300, 0.9862653100};

    dataset *ds = create_dataset(data, n);
    test_dataset(EPSILON);
}

char *test_even3()
{
    double answer[9] = {0.50056922106177648,
                        0.50006327290531249, 0.08331229878740451,
                        0.28863869939321113, 0.25445019760600, 0.74980086801600,
                        0.49535067041, 0.00007428522194, 0.99986769224100};

    dataset *ds = read_data_file("data/even.dat", false);
    test_dataset(EPSILON);
}

char *test_even3_streaming()
{
    double answer[9] = {0.50056922106177648,
                        0.50006327290531249, 0.08331229878740451,
                        0.28863869939321113, 0.25445019760600, 0.74980086801600,
                        0.49535067041, 0.00007428522194, 0.99986769224100};

    dataset *ds = read_data_file("data/even.dat", true);
    test_dataset(1e-2);
}

char *test_even4()
{
    double data[6] = {7, 15, 36, 39, 40, 41};
    size_t n = 6;
    double answer[9] = {37.5, 29.6666666666667, 218.26666666666665,
                        14.773850773128402, 20.25, 39.75, 19.5, 7.0, 41.0};

    dataset *ds = create_dataset(data, n);
    test_dataset(EPSILON);
}

char *test_small_streaming()
{
    double answer[9] = {49.080551821884,  // median
                        47.27102612042 ,  // mean
                        823.12519278139,  // var
                        28.690158465603,  // sd
                        22.0141822990,    // first_quartile
                        69.3273725100,    // third_quartile
                        47.313190210849,  // interquartile_range
                        6.5767552732,     // min
                        88.1707378980     // max
                        };

    dataset *ds = read_data_file("data/small.dat", true);
    test_dataset(7);
}

char *test_empty()
{
    double data[0] = {};
    size_t n = 0;

    dataset *ds = create_dataset(data, n);
    mu_assert(ds != NULL, "Could not create empty dataset");
    median(ds);
    mean(ds);
    var(ds);
    sd(ds);
    first_quartile(ds);
    third_quartile(ds);
    interquartile_range(ds);
    min(ds);
    max(ds);

    delete_dataset(ds);

    return NULL;
}

char *test_centroid()
{
    Centroid *c = Centroid_create(4.5, 3);
    Centroid_add(c, -9.8, 8);
    mu_assert(check_answer(Centroid_get_mean(c), -5.9, EPSILON), "Incorrect centroid mean");
    mu_assert(Centroid_get_count(c) == 11, "Incorrect centroid count");
    free(c);
    return NULL;
}

char *test_create_destroy_tdigest()
{
    TDigest *t = TDigest_create(0.01, 100);
    TDigest_destroy(t);
    return NULL;
}

char *test_tdigest_add()
{
    int i, nx = 7;
    double x[7] = {5.4, -3.2, 2.0, 2.5, 2.7, 9.8, -5.6};
    double xsorted[7] = {-5.6, -3.2, 2.0, 2.5, 2.7, 5.4, 9.8};
    TDigest *t = TDigest_create(0.01, 100);
    for (i = 0; i < nx; i++)
        TDigest_add(&t, x[i], 1);
    size_t j, ncentroids = TDigest_get_ncentroids(t);
    Centroid *c;
    for (j = 0; j < ncentroids; j++) {
        c = TDigest_get_centroid(t, j);
        mu_assert(Centroid_get_count(c) == 1, "Incorrect centroid count");
        mu_assert(check_answer(Centroid_get_mean(c), xsorted[j], EPSILON), "Incorrect centroid mean");
    }
    return NULL;
}

char *test_nofile()
{
    dataset *ds = read_data_file("imnotthere.dat", false);
    mu_assert(ds == NULL, "Read non existent file");
    return NULL;
}

char *all_tests()
{
    mu_start();

    mu_run_test(test_odd1);
    mu_run_test(test_odd2);
    mu_run_test(test_odd3);
    mu_run_test(test_odd3_streaming);
    mu_run_test(test_odd4);
    mu_run_test(test_odd5);
    mu_run_test(test_even1);
    mu_run_test(test_even2);
    mu_run_test(test_even3);
    mu_run_test(test_even3_streaming);
    mu_run_test(test_even4);
    mu_run_test(test_empty);
    mu_run_test(test_nofile);
    mu_run_test(test_small_streaming);
    mu_run_test(test_centroid);
    mu_run_test(test_create_destroy_tdigest);
    mu_run_test(test_tdigest_add);

    return NULL;
}

int main(void)
{
    char *result = all_tests();
    if (result != 0)
        printf("FAILED: %s\n", result);
    else
        printf("All tests passed.\n");
    printf("Ran %d tests.\n", tests_run);
    return (result != 0);
}
