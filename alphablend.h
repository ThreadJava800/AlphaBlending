#include <SFML/Graphics.hpp>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

typedef unsigned char uchar;

struct Pixel_t {
    uchar b = 0;
    uchar g = 0;
    uchar r = 0;
    uchar a = 0;
};

const int WINDOW_LENGTH = 1920;
const int WINDOW_HEIGHT = 1080;

const int CMP_SIGN     = 0x4D42;    // bmp file signature
const int OFFSET_START = 10;
const int SIZEX_START  = 18;
const int SIZEY_START  = 22;
const int CHANNEL_ST   = 28;

char fpsText[2000] = "";

const char* backPic  = "assets/back.bmp";
const char* frontPic = "assets/front.bmp";

Pixel_t **imageFromArr(int x, int y, int channel, int offset, char* fPtr);
Pixel_t **imageFromFile(const char *fileName, int *x, int *y, int *channel);
sf::Image imageFromPixels(int x, int y, int channel, Pixel_t **pixels);

void mergeImposed(sf::Image *back, int *imposed, int startX, int startY, int x, int y);
void imposePics(Pixel_t **top, int x, int y, int channel, Pixel_t **back, int backStartX, int backStartY, Pixel_t *draw);
void freeDoubleArr(Pixel_t **arr, int x, int y);
void runMainCycle();