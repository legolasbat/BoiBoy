#include <SFML/Graphics.hpp>
#include <chrono>

#include "Memory.h"

int main(int argc, char*argv[])
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
    //Cartridge cart(argv[1]);
    Cartridge cart("../BoiBoyCopia/Zelda.gb");

    BoiBoy boi(&cart);

    int cycles = 0;

    bool debugInstMode = false;
    bool debugFrameMode = false;
    bool debugCpu = false;

    bool nextInst = true;
    bool nextFrame = true;

    int frames = 0;

    std::chrono::steady_clock::time_point start = std::chrono::high_resolution_clock::now();

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
                // Channels enables
                if (event.key.code == sf::Keyboard::Num1) {
                    boi.spu.channel1 = !boi.spu.channel1;
                }
                if (event.key.code == sf::Keyboard::Num2) {
                    boi.spu.channel2 = !boi.spu.channel2;
                }
                if (event.key.code == sf::Keyboard::Num3) {
                    boi.spu.channel3 = !boi.spu.channel3;
                }
                if (event.key.code == sf::Keyboard::Num4) {
                    boi.spu.channel4 = !boi.spu.channel4;
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

        if (cycles >= 17556 /*boi.ppu.frameComplete*/ && nextFrame) {

            //while (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count() < 16.67) {
            //
            //}
            frames++;
            //std::cout << frames << std::endl;

            window.clear();
            pixels.update(boi.ppu.GetScreen());
            screen.setTexture(pixels);
            window.draw(screen);
            window.display();

            //boi.ppu.frameComplete = false;
            cycles -= 17556;

            if (debugFrameMode) {
                nextInst = false;
                nextFrame = false;
            }

            start = std::chrono::high_resolution_clock::now();
        }
        
    }

    return 0;
}