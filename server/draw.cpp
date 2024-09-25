#include <SFML/Graphics.hpp>
#include <iostream>

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Mouse Trail");
    window.setFramerateLimit(60);

    sf::RenderTexture renderTexture;
    if (!renderTexture.create(window.getSize().x, window.getSize().y)) {
        std::cerr << "Could not create the render texture!" << std::endl;
        return 1;
    }

    sf::Sprite canvasSprite(renderTexture.getTexture());  // Corrected this line

    sf::Vector2i lastMousePosition = sf::Mouse::getPosition(window);
    sf::Vector2i currentMousePosition;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        currentMousePosition = sf::Mouse::getPosition(window);
        if (lastMousePosition != currentMousePosition) {
            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f(lastMousePosition), sf::Color::Red),
                sf::Vertex(sf::Vector2f(currentMousePosition), sf::Color::Red)
            };

            renderTexture.draw(line, 2, sf::Lines);
            lastMousePosition = currentMousePosition;
        }

        window.clear();
        renderTexture.display();  // Necessary to update the texture of renderTexture
        canvasSprite.setTexture(renderTexture.getTexture());  // Update the texture
        window.draw(canvasSprite);
        window.display();
    }

    if (renderTexture.getTexture().copyToImage().saveToFile("mouse_trail.png")) {
        std::cout << "Saved mouse trail to 'mouse_trail.png'." << std::endl;
    } else {
        std::cerr << "Failed to save the image." << std::endl;
    }

    return 0;
}
