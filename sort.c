#include <stdbool.h>
#include <stdlib.h>
#include "sort.h"

#define SWAP(a, b) tmp=(a); a=(b); (b)=tmp;

void insertion_sort(double *list, int n)
{
    // Insertion sort is a O(n^2) algorithm; use it only for n < 20.
    int i, j;
    double tmp;

    for (i = 1; i < n; i++) {
        tmp = list[i];
        j = i - 1;
        while (tmp < list[j] && j >= 0) {
            list[j + 1] = list[j];
            j--;
        }
        list[j + 1] = tmp;
    }

    return;
}

void shell_sort(double *list, int n)
{
    // Shell sort is a O(n^3/2) algorithm; use it for n < 50.
    int i, j, inc;
    float v;

    // Find the starting increment.
    inc = 1;
    do {
        inc *= 3;
        inc++;
    } while (inc < n);

    // Perform partial sorts.
    do {
        inc /= 3;
        // Perform insertion sort on the sub array.
        for (i = inc; i < n; i++) {
            v = list[i];
            j = i;
            while (list[j - inc] > v) {
                list[j] = list[j - inc];
                j -= inc;
                if (j <= inc) break;
            }
            list[j] = v;
        }
    } while (inc > 1);

    return;
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

void sort(double *list, size_t n)
{
    // Sort the array using the most appropriate sorting algorithm based on
    // length.
    if (n < 20) {
        insertion_sort(list, n);
    } else if (n < 50) {
        shell_sort(list, n);
    } else {
        qsort(list, n, sizeof(double), compare);
    }
    return;
}

double select(double *list, size_t n, size_t k)
{
    size_t i, ir, j, l, mid;
    double a, tmp;

    l = 0;
    ir = n - 1;
    while (true) {
        if (ir <= l + 1) {
            if (ir == l + 1 && list[ir] < list[l]) {
                SWAP(list[l], list[ir]);
            }
            return list[k];
        } else {
            mid = (l + ir) >> 1;
            SWAP(list[mid], list[l + 1]);
            if (list[l] > list[ir]) {
                SWAP(list[l], list[ir]);
            }
            if (list[l + 1] > list[ir]) {
                SWAP(list[l + 1], list[ir]);
            }
            if (list[l] > list[l + 1]) {
                SWAP(list[l], list[l + 1]);
            }
            i = l + 1;
            j = ir;
            a = list[l + 1];
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
            list[l + 1] = list[j];
            list[j] = a;
            if (j >= k) ir = j - 1;
            if (j <= k) l = i;
        }
    }
}
