/*
 * This is an implementation of the t-digest algorithm by Ted Dunning and Otmar
 * Ertl. The algorithm is detailed in https://github.com/tdunning/t-digest/blob/master/docs/t-digest-paper/histo.pdf
 * and a reference implementation in Java is available at https://github.com/tdunning/t-digest.
 *
 * The current implementation has also been inspired by the work of Cam
 * Davidson-Pilon: https://github.com/CamDavidsonPilon/tdigest.
 */

#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include "tdigest.h"
#include "tree.h"

struct Centroid {
    RB_ENTRY(Centroid) entry;
    double mean;
    size_t count;
};

int centroidcmp(Centroid *c1, Centroid *c2)
{
    return (c1->mean < c2->mean ? -1 : 1);
}

struct TDigest {
    RB_HEAD(CentroidTree, Centroid) C;
    size_t count;
    size_t ncentroids;
    double delta;
    unsigned int K;
};

RB_GENERATE(CentroidTree, Centroid, entry, centroidcmp)

TDigest* TDigest_create(double delta, unsigned int K)
{
    TDigest *digest = calloc(1, sizeof(TDigest));
    RB_INIT(&(digest->C));
    digest->delta = delta;
    digest->K = K;
    digest->count = 0;
    digest->ncentroids = 0;
    return digest;
}

void TDigest_destroy(TDigest* digest)
{
    Centroid *c, *nxt;

    for (c = RB_MIN(CentroidTree, &(digest->C)); c != NULL; c = nxt) {
        nxt = RB_NEXT(CentroidTree, &(digest->C), c);
        RB_REMOVE(CentroidTree, &(digest->C), c);
        free(c);
    }
    free(digest);
}

void TDigest_add(TDigest **digest, double x, size_t w)
{
    Centroid *cj;
    cj = TDigest_find_closest_centroid(*digest, x, w);

    if (cj != NULL) {
        // Add the data point to the selected centroid.
        Centroid_add(cj, x, w);
    } else {
        Centroid *c = Centroid_create(x, w);
        RB_INSERT(CentroidTree, &((*digest)->C), c);
        ((*digest)->ncentroids)++;
    }

    (*digest)->count += w;

    if ((*digest)->ncentroids > (*digest)->K / (*digest)->delta) {
        TDigest_compress(digest);
    }
}

Centroid *TDigest_find_closest_centroid(TDigest *digest, double x, size_t w)
{
    // Find all the centroids whose mean is the closest to x. Return the number
    // of centroids that are closest to x.
    if (digest->ncentroids == 0) {
        return NULL;
    }
    double upper_mean;
    Centroid *upper_c, *res, *find, *c = NULL;
    find = Centroid_create(x, 1);

    // Find the first centroid with mean greater than or equal to x.
    upper_c = RB_NFIND(CentroidTree, &(digest->C), find);
    free(find);

    // Find the last centroid with mean greater than or equal to x.
    if (upper_c != NULL) {
        upper_mean = upper_c->mean;
        for (res = RB_NEXT(CentroidTree, &(digest->C), upper_c);
             res != NULL;
             res = RB_NEXT(CentroidTree, &(digest->C), res)) {
            if (res->mean > upper_mean)
                break;
            upper_c = res;
        }
    } else {
        // No centroid with mean greater than or equal to x was found which
        // means all centroids have mean smaller than x.
        upper_c = RB_MAX(CentroidTree, &(digest->C));
    }

    double n = 0;
    double qc, threshold;
    
    if (upper_c == NULL) {
        return NULL;
    }
    upper_mean = upper_c->mean;
    for (res = upper_c; res != NULL; res = RB_PREV(CentroidTree, &(digest->C), res)) {
        qc = Centroid_quantile(res, digest);
        threshold = 4 * digest->count * digest->delta * qc * (1 - qc);
        if (res->mean < upper_mean)
            break;
        if (res->count + w <= threshold) {
            n++;
            if (rand() / (double)RAND_MAX < 1 / n) {
                c = res;
            }
        }
    }
    return c;
}

void TDigest_compress(TDigest **digest)
{
    TDigest *digestp = *digest;
    TDigest *new_digest = TDigest_create(digestp->delta, digestp->K);
    Centroid *c;
    int i, j;

    size_t ncold, ncnew;
    ncold = digestp->ncentroids;

    while (!RB_EMPTY(&(digestp->C))) {
        j = arc4random_uniform(digestp->ncentroids);
        c = RB_MIN(CentroidTree, &(digestp->C));
        for (i = 0; i < j; i++) {
            c = RB_NEXT(CentroidTree, &(digestp->C), c);
        }
        RB_REMOVE(CentroidTree, &(digestp->C), c);
        digestp->count -= c->count;
        digestp->ncentroids -= 1;
        TDigest_add(&new_digest, c->mean, c->count);
        free(c);
    }

    ncnew = new_digest->ncentroids;
    printf("Compression: %.4lf%%\n", (ncold - ncnew) / (double)ncold * 100.0);

    TDigest_destroy(digestp);
    *digest = new_digest;
}

double TDigest_percentile(TDigest *digest, double q)
{
    double delta, t = 0;
    bool first = true;
    Centroid *c;
    q *= digest->count;
    RB_FOREACH(c, CentroidTree, &(digest->C)) {
        if (q < t + c->count) {
            if (first) {
                delta = RB_NEXT(CentroidTree, &(digest->C), c)->mean - c->mean;
            } else if (c == RB_MAX(CentroidTree, &(digest->C))) {
                delta = c->mean - RB_PREV(CentroidTree, &(digest->C), c)->mean;
            } else {
                delta = RB_NEXT(CentroidTree, &(digest->C), c)->mean - RB_PREV(CentroidTree, &(digest->C), c)->mean;
            }
            return c->mean + ((q - t) / c->count - 0.5) * delta;
        }
        t += c->count;
        first = false;
    }
    return RB_MAX(CentroidTree, &(digest->C))->mean;
}

size_t TDigest_get_ncentroids(TDigest *digest)
{
    return digest->ncentroids;
}

Centroid *TDigest_get_centroid(TDigest *digest, size_t i)
{
    Centroid *c = RB_MIN(CentroidTree, &(digest->C));
    size_t j;

    for (j = 0; j < i; j++) {
        c = RB_NEXT(CentroidTree, &(digest->C), c);
    }
    
    return c;
}

Centroid* Centroid_create(double x, size_t w)
{
    Centroid *centroid = malloc(sizeof(Centroid));
    centroid->count = w;
    centroid->mean = x;
    return centroid;
}

void Centroid_add(Centroid *c, double x, size_t w)
{
    c->count += w;
    c->mean += w * (x - c->mean) / c->count;
}

double Centroid_quantile(Centroid *c, TDigest *digest)
{
    Centroid *cj;
    double quantile = c->count / 2.0;
    for (cj = RB_PREV(CentroidTree, &(digest->C), c); cj != NULL; cj = RB_PREV(CentroidTree, &(digest->C), cj)) {
        quantile += cj->count;
    }
    return quantile / digest->count;
}

double Centroid_get_mean(Centroid *c)
{
    return c->mean;
}

double Centroid_get_count(Centroid *c)
{
    return c->count;
}

