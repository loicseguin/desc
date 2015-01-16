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
    // Given a list of size n, find the kth smallest value in the list.
    // This algorithm is based on the one found in Press et al. Numerical
    // Recipes in C, 2nd edition.
    size_t i, j;
    size_t left, mid, right;
    double a, tmp;

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
}
