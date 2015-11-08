#ifndef TDIGEST_H
#define TDIGEST_H

typedef struct Centroid Centroid;
typedef struct TDigest TDigest;
struct CentroidList;

TDigest* TDigest_create(void);
void TDigest_destroy(TDigest* digest);
void TDigest_add(TDigest *digest, double x, size_t w);
int TDigest_find_closest_centroids(TDigest *digest, double x, struct CentroidList *closests);
void TDigest_compress(TDigest *digest);
double TDigest_percentile(TDigest *digest, double q);

Centroid* Centroid_create(double x, size_t w);
void Centroid_add(Centroid *c, double x, size_t w);
double Centroid_quantile(Centroid *c, TDigest *digest);

#endif
