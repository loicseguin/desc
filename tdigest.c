#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include "tdigest.h"
#include "queue.h"
#include "tree.h"

struct Centroid {
    RB_ENTRY(Centroid) entry;
    LIST_ENTRY(Centroid) lentry;
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

LIST_HEAD(CentroidList, Centroid);

RB_GENERATE(CentroidTree, Centroid, entry, centroidcmp)

TDigest* TDigest_create(void)
{
    TDigest *digest = calloc(1, sizeof(TDigest));
    RB_INIT(&(digest->C));
    digest->delta = 0.01;
    digest->K = 10;
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

void TDigest_add(TDigest *digest, double x, size_t w, size_t n)
{
    digest->count += w;
    struct CentroidList closests;
    LIST_INIT(&closests);
    int nclose = TDigest_find_closest_centroids(digest, x, &closests);
    size_t delta_w;
    Centroid *cj;

    while (!LIST_EMPTY(&closests) && w > 0) {
        // Select a centroid uniformly at random from the list.
        int i, j = arc4random_uniform(nclose);
        cj = LIST_FIRST(&closests);
        for (i = 0; i < j; i++) {
            cj = LIST_NEXT(cj, lentry);
        }
        double qcj = Centroid_quantile(cj, digest);
        double threshold = 4 * digest->count * digest->delta * qcj * (1 - qcj);

        if (cj->count + w <= threshold) {
            // Add the data point to the selected centroid.
            delta_w = w;
            Centroid_add(cj, x, delta_w);
            w -= delta_w;
        }
        LIST_REMOVE(cj, lentry);
        nclose--;
    }

    if (w > 0) {
        Centroid *c = Centroid_create(x, w);
        RB_INSERT(CentroidTree, &(digest->C), c);
        (digest->ncentroids)++;
    }

    // Empty and free the list. However, do not free the centroids, they are
    // still needed.
    while (!LIST_EMPTY(&closests)) {
        LIST_REMOVE(LIST_FIRST(&closests), lentry);
    }

    if (digest->ncentroids > digest->K / digest->delta) {
        TDigest_compress(digest);
    }
}

int TDigest_find_closest_centroids(TDigest *digest, double x, struct CentroidList *closests)
{
    // Find all the centroids whose mean is the closest to x. Return the number
    // of centroids that are closest to x.
    int nclose = 0;
    if (digest->ncentroids == 0) {
        return 0;
    }
    Centroid *res1, *res2, *find;
    find = Centroid_create(x, 0);
    res1 = RB_NFIND(CentroidTree, &(digest->C), find);
    double res_mean;
    
    if (res1 == NULL) {
        // No centroid with mean greater than or equal to x was found which
        // means all centroids have mean smaller than x.
        res1 = RB_MAX(CentroidTree, &(digest->C));
        res_mean = res1->mean;
        if (res1 == NULL) {
            return 0;
        }
        LIST_INSERT_HEAD(closests, res1, lentry);
        nclose++;
        for (res2 = RB_PREV(CentroidTree, &(digest->C), res1); res2 != NULL; res2 = RB_PREV(CentroidTree, &(digest->C), res2)) {
            if (res2->mean != res_mean)
                break;
            LIST_INSERT_HEAD(closests, res2, lentry);
            nclose++;
        }
        return nclose;
    }

    // A closest centroid was found. Search for nearby centroids at the same
    // distance.
    res_mean = res1->mean;
    double z = fabs(x - res_mean);
    LIST_INSERT_HEAD(closests, res1, lentry);
    nclose++;
    for (res2 = RB_PREV(CentroidTree, &(digest->C), res1); res2 != NULL; res2 = RB_PREV(CentroidTree, &(digest->C), res2)) {
        if (fabs(fabs(x - res2->mean) - z) > 1e-9)
            break;
        LIST_INSERT_HEAD(closests, res2, lentry);
        nclose++;
    }
    for (res2 = RB_NEXT(CentroidTree, &(digest->C), res1); res2 != NULL; res2 = RB_NEXT(CentroidTree, &(digest->C), res2)) {
        if (fabs(fabs(x - res2->mean) - z) > 1e-9)
            break;
        LIST_INSERT_HEAD(closests, res2, lentry);
        nclose++;
    }
    
    return nclose;
}

void TDigest_compress(TDigest *digest)
{
    static int comp_count = 0;
    printf("compression #%d\n", comp_count);
    TDigest *new_digest = TDigest_create();
    Centroid *c;
    size_t n = 1;

    while (!RB_EMPTY(&(digest->C))) {
        int i, j = arc4random_uniform(digest->ncentroids);
        c = RB_MIN(CentroidTree, &(digest->C));
        for (i = 0; i < j; i++) {
            c = RB_NEXT(CentroidTree, &(digest->C), c);
        }
        RB_REMOVE(CentroidTree, &(digest->C), c);
        digest->count -= c->count;
        digest->ncentroids -= 1;
        TDigest_add(new_digest, c->mean, c->count, n);
        n++;
        free(c);
    }

    digest = new_digest;
    comp_count++;
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

