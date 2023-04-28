# Alpha blending
> Эта работа является логическим продолжением задачи с [множеством Мандельброта](https://github.com/ThreadJava800/Mandelbrot)

## Содержание
1. [Общая инфомация](#general)
2. [Ход работы](#progress)
3. [Результаты](#compare)
4. [Углубимся в ассемблер](#assemble)
5. [Заключение](#conclusion)

## Общая информация <a name="general"></a>
Цель программы - слить два изображения. \
Например:\
![Alpha Blending](https://github.com/ThreadJava800/AlphaBlending/blob/main/readmepics/result.png)

Слияние происходит при помощи такого процесса, как <em>alpha blending</em>.\
Его суть заключается в том, чтобы рассчитать каждый пиксель нового изображения по следующему правилу:\
*Цвет кодируется при помощи 4 каналов (alpha, red, green , blue)*\
*Back и front - фоновое изображение и переднее.*\
$result.alpha = 255$\
$result.red = front.red * front.alpha + back.red * (1 - front.alpha)$\
Аналогично вычисляются result.green и result.blue.


Я решил использовать этот алгоритм, чтобы глубже разобраться в <em>intrinsic</em> функциях и <em>пареллелизме уровня инструкций</em>. В частности, в <em>shuffle-ах</em>. 


|  |  |
| --- | --- |
| `Компилятор` | g++ (GCC) 12.2.1 |
| `Флаги оптимизации` | -mavx2
| `ОС` | Arch Linux x86_64 (6.2.7-arch1-1)|
| `Процессор` | AMD Ryzen 5 5500U

## Ход работы <a name="progress"></a>

Я реализовал две версии программы.

Первая - [наивная](https://github.com/ThreadJava800/AlphaBlending/blob/main/naive.cpp)\
В двойном цикле вычисляется цвет для каждого пикселя по формуле, приведённой выше. Массивом, хранящим результат, здесь является `draw`, который передаётся как параметр функции.\
Front и top же - двумерные массивы, хранящие цвета пикселей для передней и фоновой картинки соответственно.\
`Pixel_t` - структура из 4-х unsigned char-ов, каждый из которых соотвествует одному из 4-х каналов.\
![naive1](https://github.com/ThreadJava800/AlphaBlending/blob/main/readmepics/naive1.png)\
![naive2](https://github.com/ThreadJava800/AlphaBlending/blob/main/readmepics/naive2.png)

Вторая - с [использованием intrinsic функций](https://github.com/ThreadJava800/AlphaBlending/blob/main/optim1.cpp).\
Используя функцию [`_mm256_shuffle_epi8`](https://www.laruence.com/sse/#text=_mm256_shuffle_epi8&expand=5156) я сдвигаю вправо на байт все байты на нечётных местах (сами нечётные байты заполняются нулями). Таким образом я отдельно работаю с красным+голубым и альфа+зелёным каналами.\
![optim1](https://github.com/ThreadJava800/AlphaBlending/blob/main/readmepics/optim1.png)\
![optim2](https://github.com/ThreadJava800/AlphaBlending/blob/main/readmepics/optim2.png)

## Результаты <a name="compare"></a>

Как и в [Мандельброте](https://github.com/ThreadJava800/Mandelbrot) измерялось лишь время расчётов и считалось среднеквадратичное отклонение.

`-O2`:

| Реализация | Теоретическое ускорение | Фактическое ускорение |
| --- | --- | --- |
| `Наивная` | 1x (3505 us) | 1x (3505 ± 34.8 us) |
| `AVX2` | 8x (438.1 us) | 7x (497 ± 60.5 us) |

`-O3`:

| Реализация | Теоретическое ускорение | Фактическое ускорение |
| --- | --- | --- |
| `Наивная` | 1x (3552.5 us) | 1x (3552.5 ± 34 us) |
| `AVX2` | 8x (444 us) | 7x (506.7 ± 79.6 us) |

`-Ofast`:

| Реализация | Теоретическое ускорение | Фактическое ускорение |
| --- | --- | --- |
| `Наивная` | 1x (3555 us) | 1x (3555 ± 39.1 us) |
| `AVX2` | 8x (444.4 us) | 6.9x (511.6 ± 84 us) |

## Углубимся в ассемблер <a name="assemble"></a>
Далее, я решил продолжить изучение оптимизаций `g++`.

Помимо наблюдений из [Мандельброта](https://github.com/ThreadJava800/Mandelbrot), выяснилось, что `-O1` анализирует код и не считает одно и тоже значение дважды .

Ссылка на [godbolt.org](https://godbolt.org/z/eoa9Kc49v).

Код на си:\
![C code](https://github.com/ThreadJava800/AlphaBlending/blob/main/readmepics/c_code.png)\
Ассемблерный код (`-O0`):\
![O0](https://github.com/ThreadJava800/AlphaBlending/blob/main/readmepics/o0.png)\
Ассемблерный код (`-O1`):\
![O1](https://github.com/ThreadJava800/AlphaBlending/blob/main/readmepics/o1.png)

`-O2` переставляет команды таким образом, что вычисления происходят быстрее (благодаря вычислительному конвейеру). 

Код на Си:\
![c_code2](https://github.com/ThreadJava800/AlphaBlending/blob/main/readmepics/c_code2.png)\
Ассемблерный код (`-O2`):\
![O2](https://github.com/ThreadJava800/AlphaBlending/blob/main/readmepics/o2.png)

`-O3` и `-Ofast` имеют одинаковый эффект.
Они меняют команды местами, чтобы использовать возможности вычислительного конвейера для ускорения вычислений.\
Также, важно отметить, что -O3 и -Ofast разворачивают циклы гораздо сильнее, чем другие оптимизации.\
Примечательно, что эти оптимизации работают за то же время, что и `-O2`.\
Это не удивительно, ведь в ассемблерном коде (`-O3`) 1039 ассемблерных команд, в то время как в (`-O2`) их всего 88.\
Т.е., несмотря на использование более эффективных инструкций (по конвейеризуемости и количеству тактов на команду), колоссальное их количество берёт своё.

Код на Си:\
![C code](https://github.com/ThreadJava800/AlphaBlending/blob/main/readmepics/c_code3.png)

Ассемблерный код (`-Ofast`):\
![O3](https://github.com/ThreadJava800/AlphaBlending/blob/main/readmepics/o3.png)

Примечательно, что ни одна из оптимизаций не смогла "додуматься" до использования <em>shuffle-ов</em>.

## Заключение <a name="conclusion"></a>
Как мы можем видеть, даже `-Ofast` не способен использовать интринсики на полную (компилятор, например, не решается использовать <em>shuffle-ы</em> для оптимизации). Учитывая, что использование <em>intrinsic-ов</em> способно ускорить программу в разы (в данном случае - в 7 раз), в будущем я планирую использовать эту технологию для оптимизаций.
