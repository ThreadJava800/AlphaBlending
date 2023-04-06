#include <immintrin.h>

#include "alphablend.h"

#define ON_ERROR(expr, errStr) {                         \
    if (expr) {                                           \
        fprintf(stderr, "FATAL ERROR: %s\n", errStr);      \
        abort();                                            \
    }                                                        \
}                                                             \


sf::Image imageFromArr(int x, int y, int channel, int offset, char* fPtr) {
    ON_ERROR(!fPtr, "Nullptr");

    sf::Image pixelImg;
    fprintf(stderr, "%d %d\n", x, y);
    pixelImg.create(x, y, sf::Color::White);

    int curPos = offset;
    channel >>= 3;

    for (int i = y - 1; i >= 0; i--) {
        for (int j = 0; j < x; j++) {
            if (channel == 4) {
                pixelImg.setPixel(j, i, sf::Color(
                    fPtr[curPos],
                    fPtr[curPos + 1],
                    fPtr[curPos + 2],
                    fPtr[curPos + 3]
                ));

                curPos += 4;
            } else {
                pixelImg.setPixel(j, i, sf::Color(
                    fPtr[curPos++],
                    fPtr[curPos++],
                    fPtr[curPos++]
                ));  
            }
        }
    }

    return pixelImg;
}

sf::Image imageFromFile(const char *fileName) {
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

    int fOffset = 0, sizeX = 0, sizeY = 0, channel = 0;

    memcpy(&fOffset, fPtr + OFFSET_START, 4);
    memcpy(&sizeX, fPtr + SIZEX_START, 4);
    memcpy(&sizeY, fPtr + SIZEY_START, 4);
    memcpy(&channel, fPtr + CHANNEL_ST, 2);

    sf::Image image = imageFromArr(sizeX, sizeY, channel, fOffset, fPtr);
    close(fileDescr);

    return image;
}

void imposePics(sf::Image* top, sf::Image* back, int backStartX, int backStartY) {
    int sizeY = top->getSize().y;
    int sizeX = top->getSize().x;

    __m256 arr255 = _mm256_set1_ps(255);
    __m256 arr1   = _mm256_set1_ps(1);

    for (int i = 0; i < sizeY; i++) {
        for (int j = 0; j < sizeX - 7; j += 8) {
            int backX = j + backStartX;
            int backY = i + backStartY;

            __m256 topAlpha = _mm256_set_ps(
                    top->getPixel(j + 7, i).a, top->getPixel(j + 6, i).a,
                    top->getPixel(j + 5, i).a, top->getPixel(j + 4, i).a,
                    top->getPixel(j + 3, i).a, top->getPixel(j + 2, i).a,
                    top->getPixel(j + 1, i).a, top->getPixel(j, i).a
                );

            topAlpha = _mm256_div_ps(topAlpha, arr255);

            __m256 topRed = _mm256_set_ps(
                    top->getPixel(j + 7, i).r, top->getPixel(j + 6, i).r,
                    top->getPixel(j + 5, i).r, top->getPixel(j + 4, i).r,
                    top->getPixel(j + 3, i).r, top->getPixel(j + 2, i).r,
                    top->getPixel(j + 1, i).r, top->getPixel(j, i).r
                );

            __m256 topGreen = _mm256_set_ps(
                    top->getPixel(j + 7, i).g, top->getPixel(j + 6, i).g,
                    top->getPixel(j + 5, i).g, top->getPixel(j + 4, i).g,
                    top->getPixel(j + 3, i).g, top->getPixel(j + 2, i).g,
                    top->getPixel(j + 1, i).g, top->getPixel(j, i).g
                );

            __m256 topBlue = _mm256_set_ps(
                    top->getPixel(j + 7, i).b, top->getPixel(j + 6, i).b,
                    top->getPixel(j + 5, i).b, top->getPixel(j + 4, i).b,
                    top->getPixel(j + 3, i).b, top->getPixel(j + 2, i).b,
                    top->getPixel(j + 1, i).b, top->getPixel(j, i).b
                );


            __m256 backRed = _mm256_set_ps(
                    back->getPixel(backX + 7, backY).r, back->getPixel(backX + 6, backY).r,
                    back->getPixel(backX + 5, backY).r, back->getPixel(backX + 4, backY).r,
                    back->getPixel(backX + 3, backY).r, back->getPixel(backX + 2, backY).r,
                    back->getPixel(backX + 1, backY).r, back->getPixel(backX, backY).r
                );

            __m256 backGreen = _mm256_set_ps(
                    back->getPixel(backX + 7, backY).g, back->getPixel(backX + 6, backY).g,
                    back->getPixel(backX + 5, backY).g, back->getPixel(backX + 4, backY).g,
                    back->getPixel(backX + 3, backY).g, back->getPixel(backX + 2, backY).g,
                    back->getPixel(backX + 1, backY).g, back->getPixel(backX, backY).g
                );

            __m256 backBlue = _mm256_set_ps(
                    back->getPixel(backX + 7, backY).b, back->getPixel(backX + 6, backY).b,
                    back->getPixel(backX + 5, backY).b, back->getPixel(backX + 4, backY).b,
                    back->getPixel(backX + 3, backY).b, back->getPixel(backX + 2, backY).b,
                    back->getPixel(backX + 1, backY).b, back->getPixel(backX, backY).b
                );

            __m256 topArrR = _mm256_mul_ps(topRed, topAlpha);
            __m256 topArrG = _mm256_mul_ps(topGreen, topAlpha);
            __m256 topArrB = _mm256_mul_ps(topBlue, topAlpha);

            topAlpha = _mm256_sub_ps(arr1, topAlpha);       // 1 - alpha

            __m256 backArrR = _mm256_mul_ps(backRed, topAlpha);
            __m256 backArrG = _mm256_mul_ps(backGreen, topAlpha);
            __m256 backArrB = _mm256_mul_ps(backBlue, topAlpha);

            __m256 resR     = _mm256_add_ps(topArrR, backArrR);
            __m256 resG     = _mm256_add_ps(topArrG, backArrG);
            __m256 resB     = _mm256_add_ps(topArrB, backArrB);

            float *r          = (float*) (&resR);
            float *g          = (float*) (&resG);
            float *b          = (float*) (&resB);
            
            for (int m = 0; m < 8; m++, backX++) {
                back->setPixel(backX, backY, sf::Color(
                    (unsigned char)r[m], (unsigned char)g[m], (unsigned char)b[m],
                    255
                ));
            }
        }
    }
}

void runMainCycle() {
    int x = 0, y = 0, channel = 0;

    sf::Texture drawTexture;
    sf::Sprite  drawSp;
    sf::Image   drawImg;
    drawImg.create(WINDOW_LENGTH, WINDOW_HEIGHT, sf::Color::Transparent);

    sf::Font font;
    font.loadFromFile("assets/font.ttf");

    sf::Text    text;
    text.setFont(font);
    text.setFillColor(sf::Color::White);

    sf::Image catImg  = imageFromFile("assets/front.bmp");
    sf::Image backImg = imageFromFile("assets/back.bmp");

    sf::RenderWindow window(sf::VideoMode(WINDOW_LENGTH, WINDOW_HEIGHT), "Alpha blending");
    window.setPosition(sf::Vector2i(0, 0));

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
            imposePics(&catImg, &backImg, 0, 0);
            sprintf(fpsText, "%.2lf ms", ((double)clock() - (double)startTime) / CLOCKS_PER_SEC * 1000);  // ms
            text.setString(fpsText);

            drawTexture.loadFromImage(backImg);
            drawSp.     setTexture   (drawTexture);

            window.clear();
            window.draw(drawSp);
            window.draw(text);
            window.display();
        }
    }
}

int main() {
    runMainCycle();

    return 0;
}