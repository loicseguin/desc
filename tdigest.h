/*
 * This is an implementation of the t-digest algorithm by Ted Dunning and Otmar
 * Ertl. The algorithm is detailed in https://github.com/tdunning/t-digest/blob/master/docs/t-digest-paper/histo.pdf
 * and a reference implementation in Java is available at https://github.com/tdunning/t-digest.
 *
 * The current implementation has also been inspired by the work of Cam
 * Davidson-Pilon: https://github.com/CamDavidsonPilon/tdigest.
 */

#ifndef TDIGEST_H
#define TDIGEST_H

#define DEFAULT_DELTA 0.01
#define DEFAULT_K 100

typedef struct TDigest TDigest;
typedef struct Centroid Centroid;

TDigest *TDigest_create(double delta, unsigned int K);
void TDigest_destroy(TDigest* digest);
void TDigest_add(TDigest **digest, double x, size_t w);
Centroid *TDigest_find_closest_centroid(TDigest *digest, double x, size_t w);
void TDigest_compress(TDigest **digest);
double TDigest_percentile(TDigest *digest, double q);
size_t TDigest_get_ncentroids(TDigest *digest);
Centroid *TDigest_get_centroid(TDigest *digest, size_t i);
size_t TDigest_get_ncompressions(TDigest *digest);
size_t TDigest_get_count(TDigest *digest);

Centroid* Centroid_create(double x, size_t w);
void Centroid_add(Centroid *c, double x, size_t w);
double Centroid_get_mean(Centroid *c);
size_t Centroid_get_count(Centroid *c);

#endif
