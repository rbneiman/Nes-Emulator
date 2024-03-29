#include <SFML/Window.hpp>
#include <atomic>
#include <thread>
#include <iostream>
#include "screen.h"
#include "NESSystem.h"
#include "ppu.h"

//16.63 milliseconds per frame on NTSC
//one cpu cycle every 558.73 ns



std::atomic<bool> pause{false};
std::atomic<bool> updated{false};
std::atomic<unsigned char> controller{0};
std::atomic<unsigned char> controller2{0};
std::atomic<uint32_t> time_nanos{0};
[[noreturn]] void cpuTask(const char* fileName){

    NESSystem system{fileName};
    uint64_t count = 4;

    sf::Clock clock;
    while(true){
        if(updated){
            system.controller->updateState(0, controller);
            system.controller->updateState(1, controller2);
            updated = false;
        }
        if(pause){
            sf::sleep(sf::milliseconds(20));
            continue;
        }

        system.cpu->cycle(count);
        system.ppu->cycle(count);
        count += 15;
    }
}

int main(int argc, char* argv[]) {
    sf::err().rdbuf(nullptr);
    if(argc < 2){
        std::cerr << "Please specify the path to the rom to load.";
        exit(1);
    }

    InitScreen();

//    pixelSet(1,1,sf::Color::Blue);

    std::thread cpuThread(cpuTask, argv[1]);
    cpuThread.detach();
    controller = 0;
    updated = true;
    sf::Clock clock;
    while (window->isOpen()){
        time_nanos += (clock.restart().asMilliseconds()*100000);
        sf::Event event{};
        while (window->pollEvent(event)){
            // "close requested" event: we close the window
            if (event.type == sf::Event::Closed)
                window->close();
            if(event.type == sf::Event::MouseButtonPressed) {  //for use with zapper
                sf::Vector2i pos = sf::Mouse::getPosition(*window);
                controller2 |= 0x18;
                pixelSet(pos.x / pixels_size, pos.y / pixels_size, sf::Color::Blue);
                printf("X:%d, Y:%d \n", pos.x / pixels_size, pos.y / pixels_size);
                std::cout << std::to_string(controller2) << std::endl;
                fflush(stdout);
                updated = true;
            }if(event.type == sf::Event::MouseButtonReleased){
                controller2 &= ~0x18;
                std::cout << std::to_string(controller2) << std::endl;
                updated = true;
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
            sf::sleep(sf::milliseconds(5));
        }
    }
    return 0;
}
