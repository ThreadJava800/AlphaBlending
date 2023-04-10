# Alpha blending
I took this simple algorithm to work more with intrinsics (shuffles particularly).\
This program just calculates formula (alphaFront * colorFront) + (1 - alphaFront) * colorBack

|  |  |
| --- | --- |
| `Compiler` | g++ (GCC) 12.2.1 |
| `Optimisation flags` | -msse4.2 (-mavx2)
| `OS` | Arch Linux x86_64 (6.2.7-arch1-1)|
| `CPU` | AMD Ryzen 5 5500U

I coded to versions of this program. One is [naive](https://github.com/ThreadJava800/AlphaBlending/blob/main/naive.cpp)\
And [second](https://github.com/ThreadJava800/AlphaBlending/blob/main/optim1.cpp) is with intrinsics (mostly using shuffles)

This is the comparison table for different versions:

-O2:

| Optimisation | Theoretical acceleration | Actual acceleration
| --- | --- | --- |
| `Naive` | 1x (3497.5us) | 1x (3497.5us) |
| `AVX256` | 8x (437.2us) | 7.33x (476.9us) |

-O3:

| Optimisation | Theoretical acceleration | Actual acceleration
| --- | --- | --- |
| `Naive` | 1x (3421.9us) | 1x (3421.9us) |
| `AVX256` | 8x (427.7us) | 7.15x (478.9us) |

-Ofast:

| Optimisation | Theoretical acceleration | Actual acceleration
| --- | --- | --- |
| `Naive` | 1x (3531.8us) | 1x (3531.8us) |
| `AVX256` | 8x (441.5us) | 7.36x (479.9us) |