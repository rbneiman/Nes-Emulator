#include <SFML/Window.hpp>
#include <atomic>
#include "screen.h"
#include <thread>
#include <iostream>
#include <unistd.h>
#include "NESSystem.h"

//16.63 milliseconds per frame on NTSC
//one cpu cycle every 558.73 ns


std::atomic<bool> pause{false};
std::atomic<bool> updated{false};
std::atomic<unsigned char> controller{0};
std::atomic<uint32_t> time_nanos{0};
[[noreturn]] void cpuTask(){

    NESSystem system{R"(C:\Users\alec\Documents\Programming\C++\Nes-Emulator\ROMS\Super Mario Bros. (Japan, USA).nes)"};
//NESSystem system{R"(C:\Users\alec\Documents\Programming\C++\Nes-Emulator\ROMS\Complete American Nintendo Entertainment System ROM set\Officially licensed games\The Legend of Zelda (USA).nes)"};
//NESSystem system{R"(C:\Users\alec\Documents\Programming\C++\Nes-Emulator\ROMS\nestest.nes)"};

    uint64_t count = 0;

    sf::Clock clock;
    while(true){
        if(updated){
            system.controller->updateState(controller);
            updated = false;
        }
        if(pause)
            continue;
        system.cpu->cycle(count * 20);
        system.ppu->cycle(count * 20);
        ++count;
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
    controller = 4;
    updated = true;
    sf::Clock clock;
    while (window->isOpen()){
        time_nanos += (clock.restart().asMilliseconds()*100000);
        sf::Event event{};
        while (window->pollEvent(event)){
            // "close requested" event: we close the window
            if (event.type == sf::Event::Closed)
                window->close();
            if(event.type == sf::Event::MouseButtonPressed){
                sf::Vector2i pos = sf::Mouse::getPosition(*window);
                pixelSet(pos.x/pixels_size, pos.y/pixels_size, sf::Color::Blue);
                printf("X:%d, Y:%d \n", pos.x/pixels_size, pos.y/pixels_size);
                fflush(stdout);
            }if(event.type == sf::Event::KeyPressed){
                sf::Event::KeyEvent key = event.key;
                switch(key.code){
                    case(sf::Keyboard::Right):
                        controller |= 0x01;
                        break;
                    case(sf::Keyboard::Left):
                        controller |= 0x02;
                        break;
                    case(sf::Keyboard::Down):
                        controller |= 0x04;
                        break;
                    case(sf::Keyboard::Up):
                        controller |= 0x08;
                        break;
                    case(sf::Keyboard::W): //start
                        controller |= 0x10;
                        break;
                    case(sf::Keyboard::Q): //select
                        controller |= 0x20;
                        break;
                    case(sf::Keyboard::A): //B
                        controller |= 0x40;
                        break;
                    case(sf::Keyboard::S): //A
                        controller |= 0x80;
                        break;
                    case(sf::Keyboard::Escape):
                        pause = !pause;
                        break;
                    default:
                        break;
                }
                updated = true;
            }if(event.type == sf::Event::KeyReleased){
                sf::Event::KeyEvent key = event.key;
                switch(key.code){
                    case(sf::Keyboard::Right):
                        controller &= ~0x01;
                        break;
                    case(sf::Keyboard::Left):
                        controller &= ~0x02;
                        break;
                    case(sf::Keyboard::Down):
                        controller &= ~0x04;
                        break;
                    case(sf::Keyboard::Up):
                        controller &= ~0x08;
                        break;
                    case(sf::Keyboard::W): //start
                        controller &= ~0x10;
                        break;
                    case(sf::Keyboard::Q): //select
                        controller &= ~0x20;
                        break;
                    case(sf::Keyboard::A): //B
                        controller &= ~0x40;
                        break;
                    case(sf::Keyboard::S): //A
                        controller &= 0x7F;
                        break;
                    default:
                        break;
                }
                updated = true;
            }


        }
    }
    return 0;
}
