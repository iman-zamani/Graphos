//g++ -o mouse_position.out trackMouse.cpp -lsfml-system -lsfml-window
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <iostream>
#include <utility>  

//get the mouse position relative to the screen
std::pair<int, int> getMousePosition()
{
    // get the mouse position relative to the desktop (screen)
    sf::Vector2i mousePosition = sf::Mouse::getPosition();

    // return as a pair
    return std::make_pair(mousePosition.x, mousePosition.y);
}


int main()
{
    while (true)
    {

        std::pair<int, int> position = getMousePosition();
        std::cout << "Mouse Position: X = " << position.first << ", Y = " << position.second << std::endl;
        sf::sleep(sf::milliseconds(10));
    }

    return 0;
}