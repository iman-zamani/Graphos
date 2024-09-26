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
    socket.setBlocking(true);

    std::string message = "Hello from Graphos Desktop app";
    sf::IpAddress broadcastAddress = sf::IpAddress::Broadcast;
    unsigned short port = 5555;

    if (socket.send(message.c_str(), message.size(), broadcastAddress, port) != sf::Socket::Done) {
        std::cerr << "Error sending broadcast message.\n";
        return -1;
    }

    std::cout << "Broadcast message sent.\n";
    return 0;
}
