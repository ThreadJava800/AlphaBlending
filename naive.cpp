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

    for (int i = 0; i < sizeY; i++) {
        for (int j = 0; j < sizeX; j++) {
            int alpha = top->getPixel(j, i).a / 255;
            int backX = j + backStartX;
            int backY = i + backStartY;

            back->setPixel(backX, backY, sf::Color(
                top->getPixel(j, i).r * alpha + (1 - alpha) * back->getPixel(backX, backY).r,
                top->getPixel(j, i).g * alpha + (1 - alpha) * back->getPixel(backX, backY).g,
                top->getPixel(j, i).b * alpha + (1 - alpha) * back->getPixel(backX, backY).b,
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
            imposePics(&catImg, &backImg, 100, 100);
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