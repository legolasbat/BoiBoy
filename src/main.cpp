#include <SFML/Graphics.hpp>

#include "Memory.h"

int main()
{
    sf::RenderWindow window(sf::VideoMode(160, 144), "BoiBoy");
    sf::CircleShape shape(50.f);
    shape.setFillColor(sf::Color::Green);

    //Cartridge cart("cpu_instrs/individual/02-interrupts.gb");
    //Cartridge cart("cpu_instrs/cpu_instrs.gb");
    Cartridge cart("cpu_instrs/instr_timing.gb");
    //Cartridge cart("Tetris.gb");

    //BoiBoy boi;
    //boi.InsertCart(&cart);

    BoiBoy boi(&cart);

    while (window.isOpen())
    {
        boi.Clock();
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        //window.clear();
        //window.draw(shape);
        //window.display();
    }

    return 0;
}