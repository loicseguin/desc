#include <math.h>
#include <stdio.h>
#include "stats.h"

#define EPSILON 1e-8

void check_answer(double computed, double answer, double tol)
{
    if (computed < answer + tol && computed > answer - tol) {
        fprintf(stderr, "ok\n");
    } else {
        fprintf(stderr, "error (expected %.9g, got %.9g)\n", answer, computed);
    }
}

void test_describe(char *testname, dataset *ds, double *answer)
{
    fprintf(stderr, "%s: median: ", testname);
    check_answer(median(ds), answer[1], EPSILON);

    fprintf(stderr, "%s: mean: ", testname);
    check_answer(mean(ds), answer[2], EPSILON);

    fprintf(stderr, "%s: var: ", testname);
    check_answer(var(ds), answer[3], EPSILON);

    fprintf(stderr, "%s: sd: ", testname);
    check_answer(sd(ds), answer[4], EPSILON);
}

void test_odd1()
{
    double data[7] = {9.4, 2.1, -6.5, 34.2, 3.34, 67.5, 8.64};
    size_t n = 7;
    double answer[5] = {8.64, 8.64, 16.954285714285714, 655.76342857142856,
                       25.607878252042447};

    dataset *ds = create_dataset(data, n);
    test_describe("test_odd1", ds, answer);
    delete_dataset(ds);
}

void test_odd2()
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

    double answer[5] = {0.48142021970636995, 0.48142021970636995,
                       0.4904530462376237, 0.08335251150960539,
                       0.28870835025957492};

    dataset *ds = create_dataset(data, n);
    test_describe("test_odd2", ds, answer);
    delete_dataset(ds);
}

void test_odd3()
{
    dataset *ds = read_data_file("test_odd.dat");

    double answer[5] = {0.50382482225561787, 0.50382482225561787,
                       0.50233294716282062, 0.082897444134665946,
                       0.28791916249993843};
    test_describe("test_odd3", ds, answer);
    delete_dataset(ds);
}

void test_odd4()
{
    double data[7] = {8.64, 9.4, 2.1, -6.5, 34.2, 3.34, 67.5};
    size_t n = 7;
    double answer[5] = {8.64, 8.64, 16.954285714285714, 655.76342857142856,
                       25.607878252042447};

    dataset *ds = create_dataset(data, n);
    test_describe("test_odd4", ds, answer);
    delete_dataset(ds);
}

void test_even1()
{
    double data[8] = {4.3, 9.4, 2.1, -6.5, 34.2, 3.34, 67.5, 8.64};
    size_t n = 8;
    double answer[5] = {6.47, 6.47, 15.3725, 582.09930714285713,
                       24.126734282593183};

    dataset *ds = create_dataset(data, n);
    test_describe("test_even1", ds, answer);
    delete_dataset(ds);
}

void test_even2()
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

    double answer[5] = {0.461976985, 0.461976985, 0.4952750359,
                       0.08234955831149722, 0.28696612746367334};

    dataset *ds = create_dataset(data, n);
    test_describe("test_even2", ds, answer);
    delete_dataset(ds);
}

void test_even3()
{
    double answer[5] = {0.50056922106177648, 0.50056922106177648,
                       0.50006327290531249, 0.08331229878740451,
                       0.28863869939321113};

    dataset *ds = read_data_file("test_even.dat");
    test_describe("test_even3", ds, answer);
    delete_dataset(ds);
}

void timings(dataset *ds)
{
    fprintf(stderr, "Timings\n");
    fprintf(stderr, "  mean                  %.3g µs\n", timeit(mean, ds, 0));
    fprintf(stderr, "  variance              %.3g µs\n", timeit(var, ds, 0));
    fprintf(stderr, "  standard deviation    %.3g µs\n", timeit(sd, ds, 0));
    fprintf(stderr, "  median                %.3g µs\n", timeit(median, ds, 0));
}

int main(int argc, const char *argv[])
{
    dataset *ds;

    ds = read_data_file("test_odd.dat");
    fprintf(stderr, "Odd Data Set ");
    timings(ds);
    delete_dataset(ds);

    ds = read_data_file("test_even.dat");
    fprintf(stderr, "\nEven Data Set ");
    timings(ds);
    delete_dataset(ds);
    fprintf(stderr, "\n");

    test_odd1();
    test_odd2();
    test_odd3();
    test_odd4();
    test_even1();
    test_even2();
    test_even3();

    // Test reading nonexisting file.
    ds = read_data_file("imnotthere.dat");

    return 0;
}
