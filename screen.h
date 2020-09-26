

#ifndef EMULATORTEST_SCREEN_H
#define EMULATORTEST_SCREEN_H

#define pixels_width 256
#define pixels_height 240
#define pixels_size 4 //virtual pixel size in real pixels

#include <SFML/Graphics.hpp>

extern sf::RenderWindow* window;

void InitScreen();
void pixelSet(int x, int y, sf::Color color);

#endif //EMULATORTEST_SCREEN_H
