#include <SFML/Graphics.hpp>

#include "Memory.h"

int main()
{
    sf::RenderWindow window(sf::VideoMode(160, 144), "BoiBoy");
    sf::Vector2u size(480, 432);
    window.setSize(size);
    sf::Texture pixels;
    pixels.create(160, 144);
    //pixels.create(128, 128); // Map
    //pixels.create(256, 256); // FullScreen
    sf::Sprite screen;
    screen.setTexture(pixels);

    begin:
    //Cartridge cart("Tetris.gb");
    Cartridge cart("Dr. Mario.gb");
    //Cartridge cart("Super Mario Land.gb");

    BoiBoy boi(&cart);

    int cycles = 0;

    bool debugInstMode = false;
    bool debugFrameMode = false;
    bool debugCpu = false;

    bool nextInst = true;
    bool nextFrame = true;

    while (window.isOpen())
    {
        if (nextInst) {
            cycles += boi.Clock();
            if (debugInstMode)
                nextInst = false;
        }

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed) {
#pragma region joypad
                if (event.key.code == sf::Keyboard::D) {
                    boi.controllerInput &= ~0x01;
                }
                if (event.key.code == sf::Keyboard::A) {
                    boi.controllerInput &= ~0x02;
                }
                if (event.key.code == sf::Keyboard::W) {
                    boi.controllerInput &= ~0x04;
                }
                if (event.key.code == sf::Keyboard::S) {
                    boi.controllerInput &= ~0x08;
                }
                if (event.key.code == sf::Keyboard::M) {
                    boi.controllerInput &= ~0x10;
                }
                if (event.key.code == sf::Keyboard::N) {
                    boi.controllerInput &= ~0x20;
                }
                if (event.key.code == sf::Keyboard::Q) {
                    boi.controllerInput &= ~0x40;
                }
                if (event.key.code == sf::Keyboard::Enter) {
                    boi.controllerInput &= ~0x80;
                }
#pragma endregion
                if (event.key.code == sf::Keyboard::Escape) {
                    window.close();
                }
                if (event.key.code == sf::Keyboard::I) {
                    goto begin;
                }
                if (event.key.code == sf::Keyboard::F) {
                    if (debugFrameMode) {
                        std::cout << "Frame mode out" << std::endl;
                        debugFrameMode = false;
                        nextFrame = true;
                        nextInst = true;
                    }
                    else {
                        std::cout << "Frame mode on" << std::endl;
                        debugFrameMode = true;
                        nextFrame = false;
                        nextInst = false;
                    }
                }
                if (event.key.code == sf::Keyboard::P) {
                    if (debugInstMode) {
                        std::cout << "Individual opcode mode out" << std::endl;
                        debugInstMode = false;
                        nextInst = true;
                    }
                    else {
                        std::cout << "Individual opcode mode on" << std::endl;
                        debugInstMode = true;
                        nextInst = false;
                    }
                }
                if (event.key.code == sf::Keyboard::O) {
                    if (debugCpu) {
                        std::cout << "Cpu debug mode out" << std::endl;
                        debugCpu = false;
                        boi.cpu.debug = false;
                        boi.cpu.debugOp = false;
                    }
                    else {
                        std::cout << "Cpu debug mode on" << std::endl;
                        debugCpu = true;
                        boi.cpu.debug = true;
                        boi.cpu.debugOp = true;
                    }
                }
                if (event.key.code == sf::Keyboard::Space) {
                    if (debugInstMode)
                        nextInst = true;
                    else if (debugFrameMode) {
                        nextFrame = true;
                        nextInst = true;
                    }
                }
            }
            if (event.type == sf::Event::KeyReleased) {
                if (event.key.code == sf::Keyboard::D) {
                    boi.controllerInput |= 0x01;
                }
                if (event.key.code == sf::Keyboard::A) {
                    boi.controllerInput |= 0x02;
                }
                if (event.key.code == sf::Keyboard::W) {
                    boi.controllerInput |= 0x04;
                }
                if (event.key.code == sf::Keyboard::S) {
                    boi.controllerInput |= 0x08;
                }
                if (event.key.code == sf::Keyboard::M) {
                    boi.controllerInput |= 0x10;
                }
                if (event.key.code == sf::Keyboard::N) {
                    boi.controllerInput |= 0x20;
                }
                if (event.key.code == sf::Keyboard::Q) {
                    boi.controllerInput |= 0x40;
                }
                if (event.key.code == sf::Keyboard::Enter) {
                    boi.controllerInput |= 0x80;
                }
            }
        }

        if (/*cycles >= 17556*/boi.ppu.frameComplete && nextFrame) {

            window.clear();
            pixels.update(boi.ppu.GetScreen());
            screen.setTexture(pixels);
            window.draw(screen);
            window.display();

            boi.ppu.frameComplete = false;
            //cycles -= 17556;

            if (debugFrameMode) {
                nextInst = false;
                nextFrame = false;
            }
        }
        
    }

    return 0;
}