# Alpha blending
I took this simple algorithm to work more with intrinsics (shuffles particularly).\
This program just calculates formula (alphaFront * colorFront) + (1 - alphaFront) * colorBack

|  |  |
| --- | --- |
| `Compiler` | g++ (GCC) 12.2.1 |
| `Optimisation flags` | -mavx512f
| `OS` | Arch Linux x86_64 (6.2.7-arch1-1)|
| `CPU` | AMD Ryzen 5 5500U

I coded to versions of this program. One is [naive](https://github.com/ThreadJava800/AlphaBlending/blob/main/naive.cpp)\
And [second](https://github.com/ThreadJava800/AlphaBlending/blob/main/optim1.cpp) is with intrinsics (mostly using shuffles)

This is the comparison table for different versions:

| Optimisation | Theoretical acceleration | Actual acceleration
| --- | --- | --- |
| `Naive` | 1x (1.19ms) | 1x (1.19ms) |
| `AVX256` | 8x (0.15ms) | 5.7x (0.21ms) |