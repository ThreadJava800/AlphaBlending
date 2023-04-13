# Alpha blending
I took this simple algorithm to work more with intrinsics (shuffles particularly).\
This program just calculates formula (alphaFront * colorFront) + (1 - alphaFront) * colorBack

Here's the result of alpha blending of 2 pictures: \
![Alpha Blending](https://github.com/ThreadJava800/AlphaBlending/blob/main/result.png)

|  |  |
| --- | --- |
| `Compiler` | g++ (GCC) 12.2.1 |
| `Optimisation flags` | -mavx2
| `OS` | Arch Linux x86_64 (6.2.7-arch1-1)|
| `CPU` | AMD Ryzen 5 5500U

I coded two versions of this program. One is [naive](https://github.com/ThreadJava800/AlphaBlending/blob/main/naive.cpp)\
And [second](https://github.com/ThreadJava800/AlphaBlending/blob/main/optim1.cpp) is with intrinsics (mostly using shuffles).

This is the comparison table for different versions:

-O2:

| Optimisation | Theoretical acceleration | Actual acceleration | STD Deviation
| --- | --- | --- | --- |
| `Naive` | 1x (3505us) | 1x (3505us) | 34.8 |
| `AVX256` | 8x (438.1us) | 7x (497us) | 60.5 |

-O3:

| Optimisation | Theoretical acceleration | Actual acceleration | STD Deviaton
| --- | --- | --- | --- |
| `Naive` | 1x (3552.5us) | 1x (3552.5us) | 34 |
| `AVX256` | 8x (444us) | 7x (506.7us) | 79.6 |

-Ofast:

| Optimisation | Theoretical acceleration | Actual acceleration | STD Deviation
| --- | --- | --- | --- |
| `Naive` | 1x (3555us) | 1x (3555us) | 39.1 |
| `AVX256` | 8x (444.4us) | 6.9x (511.6us) | 84 |