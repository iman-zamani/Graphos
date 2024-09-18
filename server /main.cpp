#include <SFML/Network.hpp>
#include <iostream>

void startServer() {

    // listens on 0.0.0.0. and port 5000 (temporary --> need better syncing system )
    sf::IpAddress serverIp = sf::IpAddress::Any;  
    unsigned short serverPort = 5000;


    sf::TcpListener listener;

    // bind the listener to specified port
    if (listener.listen(serverPort) != sf::Socket::Done) {
        std::cerr << "Error binding the server to port " << serverPort << std::endl;
        return;
    }

    std::cout << "Server listening on " << serverIp.toString() << ":" << serverPort << std::endl;

    // main loop 
    while (true) {
        sf::TcpSocket client;
        
        // waiting for a connection 
        if (listener.accept(client) != sf::Socket::Done) {
            std::cerr << "Error accepting connection" << std::endl;
            continue;
        }

        // get the client's IP and port
        sf::IpAddress clientIp = client.getRemoteAddress();
        unsigned short clientPort = client.getRemotePort();
        std::cout << "Connection from " << clientIp.toString() << ":" << clientPort << std::endl;

        // receive data from the client
        char data[1024];
        std::size_t received;
        if (client.receive(data, sizeof(data), received) == sf::Socket::Done) {
            std::string receivedData(data, received);
            std::cout << "Received: " << receivedData << std::endl;
        }

        // close connection
        client.disconnect();
    }
}

int main() {
    startServer();
    return 0;
}
