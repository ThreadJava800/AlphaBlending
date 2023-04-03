#include <SFML/Graphics.hpp>

const int WINDOW_LENGTH = 1920;
const int WINDOW_HEIGHT = 1080;

char fpsText[2000] = "";

const char* backPic  = "assets/back.bmp";
const char* frontPic = "assets/front.bmp";

void getBmpSize(int* x, int* y, int* channel, FILE* file);
char** picToArr(int x, int y, int channel, FILE* file);
sf::Image getImageFromBmp(int x, int y, int channel, char** bmp);
void imposePics(sf::Image* top, sf::Image* back, sf::Image* pixels);
void runMainCycle();