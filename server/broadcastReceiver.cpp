// Unix 
//g++ -o sender sender.cpp -lsfml-network
//g++ -o receiver receiver.cpp -lsfml-network
// windows 
//g++ -o sender sender.cpp -lsfml-network -lsfml-system
//g++ -o receiver receiver.cpp -lsfml-network -lsfml-system
#include <SFML/Network.hpp>
#include <iostream>

int main() {
    sf::UdpSocket socket;
    if (socket.bind(5555) != sf::Socket::Done) {
        std::cerr << "Error binding socket to port.\n";
        return -1;
    }

    char buffer[1024];
    std::size_t received = 0;
    sf::IpAddress sender;
    unsigned short senderPort;

    while (true) {
        if (socket.receive(buffer, sizeof(buffer), received, sender, senderPort) != sf::Socket::Done) {
            std::cerr << "Error receiving message.\n";
            continue;
        }

        std::string message(buffer, received);
        std::cout << "Received message: " << message << " from IP: " << sender.toString() << "\n";
    }

    return 0;
}
