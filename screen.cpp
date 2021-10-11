
#include "screen.h"

void renderThread();



sf::RenderWindow* window;
sf::Thread* windowThread;
sf::VertexArray* pixels;

void InitScreen(){

    pixels = new sf::VertexArray(sf::Quads, pixels_width*pixels_height*4);
    for(int i=0; i<pixels_height; i++){
        for (int j = 0; j<pixels_width; j++) {
            //define counter clockwise starting top left
            (*pixels)[i*pixels_width*4+j*4].position = sf::Vector2f(j*pixels_size,i*pixels_size);
            (*pixels)[i*pixels_width*4+j*4+1].position = sf::Vector2f(j*pixels_size,i*pixels_size + pixels_size);
            (*pixels)[i*pixels_width*4+j*4+2].position = sf::Vector2f(j*pixels_size + pixels_size,i*pixels_size + pixels_size);
            (*pixels)[i*pixels_width*4+j*4+3].position = sf::Vector2f(j*pixels_size + pixels_size,i*pixels_size);
            (*pixels)[i*pixels_width*4+j*4].color = sf::Color::Black;
            (*pixels)[i*pixels_width*4+j*4+1].color = sf::Color::Black;
            (*pixels)[i*pixels_width*4+j*4+2].color = sf::Color::Black;
            (*pixels)[i*pixels_width*4+j*4+3].color = sf::Color::Black;
        }
    }

    window = new sf::RenderWindow(sf::VideoMode(pixels_width*pixels_size, pixels_height * pixels_size), "OpenGL");
    window->setActive(false);
    windowThread = new sf::Thread(&renderThread);
    windowThread->launch();
}

void pixelSet(int x, int y, sf::Color color){
    (*pixels)[y*pixels_width*4+x*4].color = color;
    (*pixels)[y*pixels_width*4+x*4+1].color = color;
    (*pixels)[y*pixels_width*4+x*4+2].color = color;
    (*pixels)[y*pixels_width*4+x*4+3].color = color;
}


sf::Color pixelGet(int x, int y){
    return (*pixels)[y*pixels_width*4+x*4].color;
}

uint8_t ready = 1; //thread semaphore
void renderThread(){
    while(!ready){;}
    ready = 0;
    window->setActive(true);
    // the rendering loop
    while (window->isOpen())
    {
        window->clear(sf::Color::Black);
        window->draw(*pixels);
        window->display();
        sf::sleep(sf::milliseconds(16));
    }
    ready = 1;
}


