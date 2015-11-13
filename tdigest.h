#ifndef TDIGEST_H
#define TDIGEST_H

typedef struct TDigest TDigest;
typedef struct Centroid Centroid;
struct CentroidList;

TDigest* TDigest_create(void);
void TDigest_destroy(TDigest* digest);
void TDigest_add(TDigest *digest, double x, size_t w, size_t n);
int TDigest_find_closest_centroids(TDigest *digest, double x, struct CentroidList *closests);
void TDigest_compress(TDigest *digest);
double TDigest_percentile(TDigest *digest, double q);
size_t TDigest_get_ncentroids(TDigest *digest);
Centroid *TDigest_get_centroid(TDigest *digest, size_t i);

Centroid* Centroid_create(double x, size_t w);
void Centroid_add(Centroid *c, double x, size_t w);
double Centroid_get_mean(Centroid *c);
double Centroid_get_count(Centroid *c);
double Centroid_quantile(Centroid *c, TDigest *digest);

#endif
