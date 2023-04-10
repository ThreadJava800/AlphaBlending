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

-O0:

| Optimisation | Theoretical acceleration | Actual acceleration
| --- | --- | --- |
| `Naive` | 1x (1084us) | 1x (1084us) |
| `AVX256` | 8x (135.5us) | 6.3x (171.54us) |

-O1:

| Optimisation | Theoretical acceleration | Actual acceleration
| --- | --- | --- |
| `Naive` | 1x (580.22us) | 1x (580.22us) |
| `AVX256` | 8x (72.5us) | 5.5x (194.24us) |