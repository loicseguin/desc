# desc

A fast and simple descriptive statistics tool for the UNIX command line.

## Overview

**desc** computes and prints summary statistics (count, first quartile, mean,
median, third quartile, variance, interquartile range, standard deviation, min,
max) for a given data set. It can get its input from a file or standard input,
which means it composes well with other utilities.

**desc** is designed to be *fast* and to handle large datasets easily. Whatever
the size of your dataset, **desc** can quickly compute the results.

By default, **desc** computes exact statistics by considering the whole
dataset. If the dataset is too large to fit into memory or approximate
statistics are sufficient, **desc** can be executed in *streaming* mode, which
means the data will not be stored in memory.

## Installing

To install just clone the repository, `cd` into the directory and run

    make
    sudo make install

This will install **desc** in `/usr/local/bin` by default. You can also install
**desc** somewhere else by defining a `PREFIX` environment variable like so

    make
    PREFIX=~ make install

## Examples

Input data has to be a single column of numbers, one number per line. The
following file is an example of valid data.

    $ cat data/example.dat
    2
    22
    19
    29
    25
    26
    5
    20
    16
    21

Running **desc** on that data gives the following output

    $ desc data/example.dat
    count     10
    min       2
    Q1        16.75
    mean      18.5
    median    20.5
    Q3        24.25
    max       29
    IQR       7.5
    var       76.722
    sd        8.7591

It is also possible to use **desc** as part of a pipeline. A simple (and silly)
example is to `cat` the output of a file and feed it into **desc**

    $ cat data/example.dat | desc

A more interesting example is to compute statistics on the length of `.c` and
`.h` files in the Linux kernel. The following command does that for you

    $ find ~/linux-3.18.3 -name '*.[ch]' | xargs wc -l | grep -v total | desc
    count     36971
    min       0
    Q1        69
    mean      464.87
    median    193
    Q3        503
    max       29542
    IQR       434
    var       7.2064e+05
    sd        848.9

The above command runs in less than 1.5 second on a 2011 Macbook Pro.

To run **desc** in *streaming* mode, just add the command line option `-s`.
With this option turned on, the dataset is not stored into memory. Summary
statistics are computed in one pass on the data. This options allows to work
with very large datasets. The following is an example using a 2.3 GB dataset.

    $ time ./desc -s data/very_large.dat
    count     100000000
    min       2.2054e-06
    Q1        24.991
    median    50.124
    Q3        74.939
    max       100
    IQR       49.948
    mean      49.998
    var       833.34
    sd        28.868
    ./desc -s data/very_large.dat  616,20s user 4,54s system 99% cpu 10:24,51 total

The whole process takes less than 600 KB of memory. This is obviously much 
slower than not using streaming mode, but it has the advantage of working on 
datasets that can't fit into memory.

## Features/Limitations

  - Silently ignores missing/invalid values.
  - Uses only the first whitespace-separated token on each line of input, the
    rest of the line is ignored.
  - Get help: `-h` to print a short help message.
  - Run in streaming mode with command line option `-s`

    Using this mode, the throughput is roughly 3 MB/s (slightly faster for data 
    sets with less than a couple of gigabytes). The caveat is that the 
    percentiles (first quartile, median, third quartile) are approximated using 
    a t-digest [[1][dunning]]. The relative error is on the order of 0.5%.
    
    Due to the overhead of maintaining the data structure, this mode is much 
    slower than the normal mode and should only be used when data does not fit 
    into memory.

[dunning]: https://github.com/tdunning/t-digest "Dunning, T., Ertl, O. *Computing Extremely Accurate Quantiles Using t-Digests*"

## Benchmark

There are a plethora of other command line utilities for descriptive statistics
on the command line. Here are a few:

  - [st](https://github.com/nferraz/st)
  - [data_hacks](https://github.com/bitly/data_hacks)
  - [clistats](https://github.com/dpmcmlxxvi/clistats)
  - [qstats](https://github.com/tonyfischetti/qstats)

All of these tools have more features than **desc** but they are slower.
Here is a comparison of **qstats**, **st** and **desc** for calculations on a
240 MB file with 10,000,000 entries.

    $ time ../qstats/bin/qstats data/very_large.dat
    Min.     1.15151e-05
    1st Qu.  25.0243
    Median   50.031
    Mean     50.0183
    3rd Qu.  75.0133
    Max.     100
    Range    100
    Std Dev. 28.8645
    Length   10000000
    ../qstats/bin/qstats data/very_large.dat  11,12s user 0,23s system 99% cpu 11,463 total

    $ time ../st/script/st data/very_large.dat   
    N	min	max	sum	mean	stddev
    1e+07	1.15151e-05	100	5.00183e+08	50.0183	28.8645
    ../st/script/st data/very_large.dat  99,95s user 0,26s system 99% cpu 1:40,85 total

    $ time ./desc data/very_large.dat
    count     10000000
    min       1.1515e-05
    Q1        25.024
    mean      50.018
    median    50.031
    Q3        75.013
    max       100
    IQR       49.989
    var       833.16
    sd        28.865
    ./desc data/very_large.dat  6,09s user 0,15s system 99% cpu 6,254 total

**desc** is about twice as fast as **qstats** and fifteen times faster than
**st**.

## Author

Loïc Séguin-Charbonneau <lsc@loicseguin.com>

## License

MIT License, see LICENSE for details.
