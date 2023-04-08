#include <SFML/Graphics.hpp>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

typedef unsigned char uchar;

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

int **imageFromArr(int x, int y, int channel, int offset, char* fPtr);
int **imageFromFile(const char *fileName, int *x, int *y, int *channel);
sf::Image imageFromPixels(int x, int y, int channel, int **pixels);

void mergeImposed(sf::Image *back, int *imposed, int startX, int startY, int x, int y);
void imposePics(int **top, int x, int y, int channel, int **back, int backStartX, int backStartY, int *draw);
void runMainCycle();