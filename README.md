# Alpha blending
> This program just calculates formula (alphaFront * colorFront) + (1 - alphaFront) * colorBack for each pixel.

## Table of Contents
1. [General information](#general)
2. [Progress](#progress)
3. [Comparison table](#compare)
4. [Dive into assembly](#assemble)
5. [Conclusion](#conclusion)

## General information <a name="general"></a>

I took this simple algorithm to work more with intrinsics (shuffles particularly).

Here's the result of alpha blending of 2 pictures: \
![Alpha Blending](https://github.com/ThreadJava800/AlphaBlending/blob/main/readmepics/result.png)

|  |  |
| --- | --- |
| `Compiler` | g++ (GCC) 12.2.1 |
| `Optimisation flags` | -mavx2
| `OS` | Arch Linux x86_64 (6.2.7-arch1-1)|
| `CPU` | AMD Ryzen 5 5500U

## Progress <a name="progress"></a>

I coded two versions of this program. One is [naive](https://github.com/ThreadJava800/AlphaBlending/blob/main/naive.cpp)\
And [second](https://github.com/ThreadJava800/AlphaBlending/blob/main/optim1.cpp) is with intrinsics (mostly using shuffles).

## Comparison table <a name="compare"></a>

This is the comparison table for different versions:

-O2:

| Optimisation | Theoretical acceleration | Actual acceleration |
| --- | --- | --- |
| `Naive` | 1x (3505 us) | 1x (3505 ± 34.8 us) |
| `AVX256` | 8x (438.1 us) | 7x (497 ± 60.5 us) |

-O3:

| Optimisation | Theoretical acceleration | Actual acceleration |
| --- | --- | --- |
| `Naive` | 1x (3552.5 us) | 1x (3552.5 ± 34 us) |
| `AVX256` | 8x (444 us) | 7x (506.7 ± 79.6 us) |

-Ofast:

| Optimisation | Theoretical acceleration | Actual acceleration |
| --- | --- | --- |
| `Naive` | 1x (3555 us) | 1x (3555 ± 39.1 us) |
| `AVX256` | 8x (444.4 us) | 6.9x (511.6 ± 84 us) |

## Dive into assembly <a name="assemble"></a>

