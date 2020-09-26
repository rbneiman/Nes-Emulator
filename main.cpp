#include <cstdio>
#include <cstdint>
#include "cpu.h"
#include "ppu.h"
#include <SFML/Window.hpp>
#include "screen.h"
#include <iostream>


int main() {
    sf::err().rdbuf(nullptr);

    InitCPU();
    InitScreen();
    InitPpu();
    cycleCPU(100);
    pixelSet(1,1,sf::Color::Blue);
    while (window->isOpen())
    {
        sf::Event event;
        while (window->pollEvent(event))
        {
            // "close requested" event: we close the window
            if (event.type == sf::Event::Closed)
                window->close();
            if(event.type == sf::Event::MouseButtonPressed){
                sf::Vector2i pos = sf::Mouse::getPosition(*window);
                pixelSet(pos.x/pixels_size, pos.y/pixels_size, sf::Color::Blue);
                printf("X:%d, Y:%d \n", pos.x/pixels_size, pos.y/pixels_size);
            }
        }
    }
    return 0;
}
