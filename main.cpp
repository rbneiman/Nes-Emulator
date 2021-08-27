#include <cstdio>
#include <SFML/Window.hpp>
#include <atomic>
#include "screen.h"
#include <thread>
#include "NESSystem.h"


std::atomic<int> controller{0};
[[noreturn]] void cpuTask(){

    NESSystem system{R"(C:\Users\Alec\Documents\Programming\c++\Nes-Emulator\nestest.nes)"};

    int count = 0;
    while(true){
        system.cpu->cycle(count * 20);
        system.ppu->cycle(count * 20);
        system.memory->writeMemory8(0x4016, controller);
        count += 1;
#ifdef DEBUG_CPU
//        if(count % 100 == 0 && count != 0){
//            cpu.printMemoryDebug(0x00, 0xff);
//            count = 0;
//        }
#endif
    }
    system.cpu->printMemoryDebug(0x00, 0xff);
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
