#include "alphablend.h"

void getBmpSize(int* x, int* y, int* channel, FILE* file) {
    if (!x || !y || !file) return;

    unsigned char intBuf[4] = {};

    fseek(file, 18, 0);     // set pointer to image width

    fread(intBuf, 4, 1, file);
    *x = intBuf[0] | (intBuf[1] << 8) | (intBuf[2] << 16) | (intBuf[3] << 24);

    fread(intBuf, 4, 1, file);
    *y = intBuf[0] | (intBuf[1] << 8) | (intBuf[2] << 16) | (intBuf[3] << 24);

    fseek(file, 28, 0);     // set pointer to bits per pixel

    fread(intBuf, 2, 1, file);
    *channel = (intBuf[0] | (intBuf[1] << 8)) >> 3; // bytes per pixel
}

char** picToArr(int x, int y, int channel, FILE* file) {
    if (x < 0 || y < 0 || !file) return NULL;

    fseek(file, 54, 0);     // move to start of binary array (skip signature)

    char** picArr = (char**) calloc(y, sizeof(char*));
    if (!picArr) return NULL;

    for (int i = 0; i < y; i++) {
        picArr[i] = (char*) calloc(x * channel, sizeof(char));
        if (!(picArr[i])) return NULL;
    }

    for (int i = y - 1; i >= 0; i--) {
        for (int j = 0; j < x * channel; j += channel) {
            if (channel == 4) {
                for (int m = 0; m < channel; m++) {
                    picArr[i][j + m] = fgetc(file);
                }
            } else {
                for (int m = channel - 1; m >= 0; m--) {
                    picArr[i][j + m] = fgetc(file);
                }
            }
        }
    }

    return picArr;
}

sf::Image getImageFromBmp(int x, int y, int channel, char** bmp) {
    sf::Image pixelImg;
    pixelImg.create(WINDOW_LENGTH, WINDOW_HEIGHT, sf::Color::Transparent);

    for (int i = 0; i < y; i++) {
        for (int j = 0; j < x * channel; j += channel) {
            if (channel == 3)
                pixelImg.setPixel(j / channel, i, sf::Color(
                    bmp[i][j],
                    bmp[i][j + 1],
                    bmp[i][j + 2]
                ));
            else 
                pixelImg.setPixel(j / channel, i, sf::Color(
                    bmp[i][j],
                    bmp[i][j + 1],
                    bmp[i][j + 2],
                    bmp[i][j + 3]
                ));
        }
    }

    return pixelImg;
}

void imposePics(sf::Image* top, sf::Image* back, sf::Image* pixels) {
    for (int i = 0; i < WINDOW_HEIGHT; i++) {
        for (int j = 0; j < WINDOW_LENGTH; j++) {
            int alpha = top->getPixel(j, i).a / 255;

            pixels->setPixel(j, i, sf::Color(
                top->getPixel(j, i).r * alpha + (1 - alpha) * back->getPixel(j, i).r,
                top->getPixel(j, i).g * alpha + (1 - alpha) * back->getPixel(j, i).g,
                top->getPixel(j, i).b * alpha + (1 - alpha) * back->getPixel(j, i).b,
                255
            ));
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

    FILE* picFile = fopen(frontPic, "rb");
    getBmpSize(&x, &y, &channel, picFile);
    char** picArr = picToArr(x, y, channel, picFile);
    sf::Image catImg = getImageFromBmp(x, y, channel, picArr);
    fclose(picFile);

    picFile = fopen(backPic, "rb");
    getBmpSize(&x, &y, &channel, picFile);
    picArr = picToArr(x, y, channel, picFile);
    sf::Image backImg = getImageFromBmp(x, y, channel, picArr);
    fclose(picFile);

    imposePics(&catImg, &backImg, &drawImg);

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
            imposePics(&catImg, &backImg, &drawImg);
            sprintf(fpsText, "%.2lf ms", ((double)clock() - (double)startTime) / CLOCKS_PER_SEC * 1000);  // ms
            text.setString(fpsText);

            drawTexture.loadFromImage(drawImg);
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