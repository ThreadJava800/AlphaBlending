#include <immintrin.h>

#include "alphablend.h"

#define ON_ERROR(expr, errStr) {                         \
    if (expr) {                                           \
        fprintf(stderr, "FATAL ERROR: %s\n", errStr);      \
        abort();                                            \
    }                                                        \
}                                                             \


Pixel_t **imageFromArr(int x, int y, int channel, int offset, char* fPtr) {
    ON_ERROR(!fPtr, "Nullptr");

    Pixel_t** picArr = (Pixel_t**) calloc(y, sizeof(Pixel_t*));
    if (!picArr) return NULL;

    for (int i = 0; i < y; i++) {
        picArr[i] = (Pixel_t*) calloc(x, sizeof(Pixel_t));
        if (!(picArr[i])) return NULL;
    }

    int curPos = offset;

    for (int i = y - 1; i >= 0; i--) {
        for (int j = 0; j < x * channel; j += channel) {
            if (channel == 4) {
                picArr[i][j / channel] = {
                    fPtr[curPos++],
                    fPtr[curPos++],
                    fPtr[curPos++],
                    fPtr[curPos++]
                };
            } else {
                picArr[i][j / channel] = {
                    fPtr[curPos + 2],
                    fPtr[curPos + 1],
                    fPtr[curPos],
                };

                curPos += 3;
            }
        }
    }

    return picArr;
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

sf::Image imageFromPixels(int x, int y, int channel, Pixel_t **pixels) {
    sf::Image pixelImg;
    pixelImg.create(x, y, sf::Color::Transparent);

    for (int i = y - 1; i >= 0; i--) {
        for (int j = 0; j < x; j++) {
            if (channel == 4) {
                pixelImg.setPixel(j, i, sf::Color(
                    (pixels[i][j]).r,
                    (pixels[i][j]).g,
                    (pixels[i][j]).b,
                    (pixels[i][j]).a
                ));
            } else {
                pixelImg.setPixel(j, i, sf::Color(
                    (pixels[i][j]).r,
                    (pixels[i][j]).g,
                    (pixels[i][j]).b
                ));  
            }
        }
    }

    return pixelImg;
}

void imposePics(Pixel_t **top, int x, int y, int channel, Pixel_t **back, int backStartX, int backStartY, sf::Image *draw) {
    __m256 arr255 = _mm256_set1_ps(255);
    __m256 arr1   = _mm256_set1_ps(1);

    for (int i = 0; i < y; i++) {
        for (int j = 0; j < x; j += 8) {
            
            
            for (int m = 0; m < 8; m++, backX++) {
                draw->setPixel(backX, backY, sf::Color(
                    (unsigned char)r[m], (unsigned char)g[m], (unsigned char)b[m],
                    255
                ));
            }
        }
    }
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
    Pixel_t **backImgPixels = imageFromFile("assets/back.bmp", &backX, &backY, &backChannel);
    sf::Image backImg    = imageFromPixels(backX, backY, backChannel, backImgPixels);

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
            imposePics(catImgPixels, frontX, frontY, frontChannel, backImgPixels, 100, 100, &backImg);
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