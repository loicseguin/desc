#ifndef TDIGEST_H
#define TDIGEST_H

#include "queue.h"
#include "tree.h"

LIST_HEAD(CentroidList, Centroid);

typedef struct Centroid {
    RB_ENTRY(Centroid) entry;
    LIST_ENTRY(Centroid) lentry;
    double mean;
    size_t count;
} Centroid;

int centroidcmp(Centroid *c1, Centroid *c2);

RB_HEAD(CentroidTree, Centroid);

typedef struct TDigest {
    struct CentroidTree *C;
    size_t count;
    size_t ncentroids;
    double delta;
    unsigned int K;
} TDigest;

RB_PROTOTYPE(CentroidTree, Centroid, entry, centroidcmp)
RB_GENERATE(CentroidTree, Centroid, entry, centroidcmp)

TDigest* TDigest_create(void);
void TDigest_destroy(TDigest* digest);
void TDigest_add(TDigest *digest, double x, size_t w);
int TDigest_find_closest_centroids(TDigest *digest, double x, struct CentroidList *closests);
void TDigest_compress(TDigest *digest);

Centroid* Centroid_create(double x, size_t w);
void Centroid_add(Centroid *c, double x, size_t w);
double Centroid_quantile(Centroid *c, TDigest *digest);

#endif
