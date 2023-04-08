#include "alphablend.h"

#define ON_ERROR(expr, errStr) {                         \
    if (expr) {                                           \
        fprintf(stderr, "FATAL ERROR: %s\n", errStr);      \
        abort();                                            \
    }                                                        \
}                                                             \


int **imageFromArr(int x, int y, int channel, int offset, char* fPtr) {
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
                picArr[i][j / channel] <<= 8;
                picArr[i][j / channel] |= 255;
                curPos += 3;
            }
        }
    }

    return picArr;
}

int **imageFromFile(const char *fileName, int *x, int *y, int *channel) {
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

    int **image = imageFromArr(*x, *y, *channel, fOffset, fPtr);
    close(fileDescr);

    return image;
}

sf::Image imageFromPixels(int x, int y, int channel, int **pixels) {
    ON_ERROR(!pixels, "Nullptr");

    sf::Image pixelImg;
    pixelImg.create(x, y, sf::Color::Transparent);

    for (int i = y - 1; i >= 0; i--) {
        ON_ERROR(!pixels[i], "Nullptr");
        
        for (int j = 0; j < x; j++) {
            pixelImg.setPixel(j, i, sf::Color(
                pixels[i][j]
            ));
        }
    }

    return pixelImg;
}

void mergeImposed(sf::Image *back, int *imposed, int startX, int startY, int x, int y) {
    ON_ERROR(!back || !imposed, "Nullptr");

    for (int i = 0; i < y; i++) {
        for (int j = 0; j < x; j++) {
            back->setPixel(startX + j, startY + i, sf::Color(
                imposed[i * x + j]
            ));
        }
    }
}

void imposePics(int **top, int x, int y, int frontChannel, int **back, int backStartX, int backStartY, int *draw) {
    for (int i = 0; i < y; i++) {
        for (int j = 0; j < x; j++) {
            float alpha = ((top[i][j] & 0xFF000000)>>24) / 255;
            int backX = j + backStartX;
            int backY = i + backStartY;

            draw[i * x + j] = (
                ((uchar) (((top[i][j] & 0xFF0000) >> 16) * alpha + (1 - alpha) * ((back[backY][backX] & 0xFF000000) >> 24)) << 24) |
                ((uchar) (((top[i][j] & 0xFF00) >> 8)    * alpha + (1 - alpha) * ((back[backY][backX] & 0xFF0000) >> 16)) << 16)   |
                ((uchar) (((top[i][j] & 0xFF))           * alpha + (1 - alpha) * ((back[backY][backX] & 0xFF00) >> 8)) << 8)       |
                255);
        }
    }
}

void freeDoubleArr(int **arr, int x, int y) {
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
    int **catImgPixels = imageFromFile("assets/front.bmp", &frontX, &frontY, &frontChannel);

    int backX = 0, backY = 0, backChannel = 0;
    int **backImgPixels = imageFromFile("assets/back.bmp", &backX, &backY, &backChannel);
    sf::Image backImg    = imageFromPixels(backX, backY, backChannel, backImgPixels);

    sf::RenderWindow window(sf::VideoMode(WINDOW_LENGTH, WINDOW_HEIGHT), "Alpha blending");
    window.setPosition(sf::Vector2i(0, 0));

    int* picArr = (int*) calloc(frontY * frontX, sizeof(int));
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