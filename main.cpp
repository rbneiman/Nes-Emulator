#include <cstdio>
#include <cstdint>
#include "cpu.h"
#include "ppu.h"
#include <SFML/Window.hpp>
#include "screen.h"
#include <iostream>
#include "rom.h"
#include <thread>

[[noreturn]] void cpuTask(){
    auto* testRom = new RomFile(R"(C:\Users\Alec\Documents\Programming\c++\Nes-Emulator\nestest.nes)");


    CPU6502 cpu{};
    cpu.setRom(testRom);
    InitPpu();

    int count = 0;
    while(true){
        cpu.cycle(100);
        count += 1;
        if(count % 100 == 0 && count != 0){
//            cpu.printStatus();
            cpu.printMemoryDebug(0x00, 0xff);
            count = 0;
            fflush(stdout);
        }
    }

}

int main() {
    sf::err().rdbuf(nullptr);



    InitScreen();

    pixelSet(1,1,sf::Color::Blue);


    std::thread cpuThread(cpuTask);
    cpuThread.detach();
    while (window->isOpen()){

        sf::Event event{};
        while (window->pollEvent(event)){
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
