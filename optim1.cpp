#include <immintrin.h>

#include "alphablend.h"

__m256i maskR_B = _mm256_set1_epi32(0x00FF00FF);
__m256i SHUFFLE_A_G    = _mm256_set_epi8(   0x80, 31, 0x80, 29,
                                            0x80, 27, 0x80, 25,
                                            0x80, 23, 0x80, 21,
                                            0x80, 19, 0x80, 17,
                                            0x80, 15, 0x80, 13,
                                            0x80, 11, 0x80, 9,
                                            0x80, 7,  0x80, 5,
                                            0x80, 3,  0x80, 1);

__m256i SHUFFLE_ONLY_A = _mm256_set_epi8(   0x80, 31, 0x80, 31,
                                            0x80, 27, 0x80, 27,
                                            0x80, 23, 0x80, 23,
                                            0x80, 19, 0x80, 19,
                                            0x80, 15, 0x80, 15,
                                            0x80, 11, 0x80, 11,
                                            0x80, 7,  0x80, 7,
                                            0x80, 3,  0x80, 3);

__m256i ALL_255        = _mm256_set1_epi16(255);
__m256i ZERO_SECOND    = _mm256_set1_epi32(0xFF00FF00);


#define ON_ERROR(expr, errStr) {                         \
    if (expr) {                                           \
        fprintf(stderr, "FATAL ERROR: %s\n", errStr);      \
        abort();                                            \
    }                                                        \
}                                                             \


Pixel_t **imageFromArr(int x, int y, int channel, int offset, char* fPtr) {
    ON_ERROR(!fPtr, "Nullptr");

    int** picArr = (int**) calloc(y, sizeof(int*));
    if (!picArr) return NULL;

    for (int i = 0; i < y; i++) {
        picArr[i] = (int*) calloc(x, sizeof(int));
        if (!(picArr[i])) return NULL;
    }

    int curPos = offset;

    for (int i = y - 1; i >= 0; i--) {
        for (int j = 0; j < x * channel; j += channel) {
            if (channel == 4) {
                memcpy(&(picArr[i][j / channel]), fPtr + curPos, 4);
                curPos += 4;
            } else {
                memcpy(&(picArr[i][j / channel]), fPtr + curPos, 3);
                picArr[i][j / channel] |= (255 << 24);
                curPos += 3;
            }
        }
    }

    return (Pixel_t**) picArr;
}

Pixel_t **imageFromFile(const char *fileName, int *x, int *y, int *channel) {
    ON_ERROR(!fileName, "Nullptr");

    // index to an entry in the process's table of open file descriptors
    int fileDescr = open(fileName, O_RDONLY);
    struct stat fileStat;
    int fErr = fstat(fileDescr, &fileStat);
    ON_ERROR(fErr < 0, "Couldn't open file");

    char *fPtr = (char*) mmap(NULL, fileStat.st_size, PROT_READ, MAP_SHARED, fileDescr, 0);
    ON_ERROR(fPtr == MAP_FAILED, "Mapping not succeeded");

    int signature = fPtr[1] << 8 | fPtr[0];
    ON_ERROR(signature != CMP_SIGN, "That is not BMP file!!!");

    int fOffset = 0;

    memcpy(&fOffset, fPtr + OFFSET_START, 4);
    memcpy(x, fPtr + SIZEX_START, 4);
    memcpy(y, fPtr + SIZEY_START, 4);
    memcpy(channel, fPtr + CHANNEL_ST, 2);
    (*channel) >>= 3;

    Pixel_t **image = imageFromArr(*x, *y, *channel, fOffset, fPtr);
    close(fileDescr);

    return image;
}

void mergeImposed(sf::Image *back, Pixel_t *imposed, int startX, int startY, int x, int y) {
    ON_ERROR(!back || !imposed, "Nullptr");

    for (int i = 0; i < y; i++) {
        for (int j = 0; j < x; j++) {
            back->setPixel(startX + j, startY + i, sf::Color(
                imposed[i * x + j].r,
                imposed[i * x + j].g,
                imposed[i * x + j].b,
                imposed[i * x + j].a
            ));
        }
    }
}

sf::Image imageFromPixels(int x, int y, int channel, Pixel_t **pixels) {
    ON_ERROR(!pixels, "Nullptr");

    sf::Image pixelImg;
    pixelImg.create(x, y, sf::Color::Transparent);

    for (int i = y - 1; i >= 0; i--) {
        ON_ERROR(!pixels[i], "Nullptr");

        for (int j = 0; j < x; j++) {
            pixelImg.setPixel(j, i, sf::Color(
                pixels[i][j].r,
                pixels[i][j].g,
                pixels[i][j].b,
                pixels[i][j].a
            ));
        }
    }

    return pixelImg;
}

void imposePics(Pixel_t **top, int x, int y, int channel, Pixel_t **back, int backStartX, int backStartY, Pixel_t *draw) {
    for (int i = 0; i < y; i++) {
        for (int j = 0; j < x - 7; j += 8) {
            // float alpha = ((top[i][j] & 0xFF000000)>>24) / 255;
            int backX = j + backStartX;
            int backY = i + backStartY;

            __m256i topPic  = _mm256_loadu_si256((__m256i*) &(top[i][j]));
            __m256i backPic = _mm256_loadu_si256((__m256i*) &(back[backY][backX]));

            // taking red and blues (others set to 0)
            __m256i topR_B  = _mm256_and_si256(maskR_B, topPic);
            __m256i backR_B = _mm256_and_si256(maskR_B, backPic);

            // taking alphas and green (others set to 0)
            __m256i topA_G  = _mm256_shuffle_epi8(topPic,  SHUFFLE_A_G);
            __m256i backA_G = _mm256_shuffle_epi8(backPic, SHUFFLE_A_G);

            // alphas on second byte
            __m256i topA    = _mm256_shuffle_epi8(topPic,  SHUFFLE_ONLY_A);

            // 256 - alpha on second byte
            __m256i topA256 = _mm256_sub_epi16(ALL_255, topA);
            __m256i resA_G  = _mm256_add_epi8(_mm256_mullo_epi16(topA, topA_G), _mm256_mullo_epi16(topA256, backA_G));
            __m256i resR_B  = _mm256_add_epi8(_mm256_mullo_epi16(topA, topR_B), _mm256_mullo_epi16(topA256, backR_B));

            resR_B          = _mm256_shuffle_epi8(resR_B, SHUFFLE_A_G);
            resA_G          = _mm256_and_si256   (resA_G, ZERO_SECOND);
            __m256i res     = _mm256_or_si256    (resR_B, resA_G);

            _mm256_storeu_si256((__m256i *) (draw + i * x + j), res);
        }
    }

    for (int i = 0; i < y; i++) {
        for (int j = x - 7; j < x; j++) {
            float alpha = top[i][j].a / 255;
            int backX = j + backStartX;
            int backY = i + backStartY;

            draw[i * x + j].a = 255;
            draw[i * x + j].r = top[i][j].r * alpha + (1 - alpha) * back[backY][backX].r;
            draw[i * x + j].g = top[i][j].g * alpha + (1 - alpha) * back[backY][backX].g;
            draw[i * x + j].b = top[i][j].b * alpha + (1 - alpha) * back[backY][backX].b;
        }
    }
}

void freeDoubleArr(Pixel_t **arr, int x, int y) {
    if (!arr) return;

    for (int i = 0; i < y; i++) {
        if (arr[i]) free(arr[i]);
    }
    free(arr);
}

void runMainCycle() {
    sf::Texture drawTexture;
    sf::Sprite  drawSp;
    sf::Image   drawImg;
    drawImg.create(WINDOW_LENGTH, WINDOW_HEIGHT, sf::Color::Transparent);

    sf::Font font;
    font.loadFromFile("assets/font.ttf");

    sf::Text    text;
    text.setFont(font);
    text.setFillColor(sf::Color::White);

    int frontX = 0, frontY = 0, frontChannel = 0;
    Pixel_t **catImgPixels = imageFromFile("assets/front.bmp", &frontX, &frontY, &frontChannel);

    int backX = 0, backY = 0, backChannel = 0;
    Pixel_t **backImgPixels  = imageFromFile("assets/back.bmp", &backX, &backY, &backChannel);
    sf::Image backImg    = imageFromPixels(backX, backY, backChannel, backImgPixels);

    sf::RenderWindow window(sf::VideoMode(WINDOW_LENGTH, WINDOW_HEIGHT), "Alpha blending");
    window.setPosition(sf::Vector2i(0, 0));

    Pixel_t* picArr = (Pixel_t*) calloc(frontY * frontX, sizeof(Pixel_t));
    if (!picArr) return;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed || 
                (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)) {
                
                window.close();
            }

            clock_t startTime = clock();
            imposePics(catImgPixels, frontX, frontY, frontChannel, backImgPixels, 100, 100, picArr);
            sprintf(fpsText, "%.2lf ms", ((double)clock() - (double)startTime) / CLOCKS_PER_SEC * 1000);  // ms
            text.setString(fpsText);

            mergeImposed(&backImg, picArr, 100, 100, frontX, frontY);

            drawTexture.loadFromImage(backImg);
            drawSp.     setTexture   (drawTexture);

            window.clear();
            window.draw(drawSp);
            window.draw(text);
            window.display();
        }
    }

    freeDoubleArr(catImgPixels, frontX, frontY);
    freeDoubleArr(backImgPixels, backX, backY);
    free(picArr);
}

int main() {
    runMainCycle();

    return 0;
}