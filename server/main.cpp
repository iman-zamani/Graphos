//Unix : g++ -o app main.cpp -lsfml-network -lsfml-system
// windows : g++ -o app.exe main.cpp -lsfml-network -lsfml-system

#include <SFML/Network.hpp>
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>

// atomic flag to control the broadcasting
std::atomic<bool> keepBroadcasting(true);

// broadcast message
void broadcastMessage() {
    sf::UdpSocket socket;
    socket.setBlocking(true);

    std::string message = "Hello from Graphos Desktop app";
    sf::IpAddress broadcastAddress = sf::IpAddress::Broadcast;
    unsigned short port = 5555;
    // we will broadcast each second until the connection is made
    while (keepBroadcasting) {
        if (socket.send(message.c_str(), message.size(), broadcastAddress, port) != sf::Socket::Done) {
            std::cerr << "Error sending broadcast message.\n";
        } else {
            std::cout << "Broadcast message sent.\n";
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));  
    }
}

// incoming connections to server
void startServer() {
    sf::IpAddress serverIp = sf::IpAddress::Any;
    unsigned short serverPort = 5000;

    sf::TcpListener listener;

    // Binding
    if (listener.listen(serverPort) != sf::Socket::Done) {
        std::cerr << "Error binding the server to port " << serverPort << std::endl;
        exit(-1);
    }

    std::cout << "Server listening on " << serverIp.toString() << ":" << serverPort << std::endl;

    while (true) {
        sf::TcpSocket client;

        // waiting for connection
        if (listener.accept(client) != sf::Socket::Done) {
            std::cerr << "Error accepting connection" << std::endl;
            continue;
        }

        // get android device ip and port 
        sf::IpAddress clientIp = client.getRemoteAddress();
        unsigned short clientPort = client.getRemotePort();
        std::cout << "Connection from " << clientIp.toString() << ":" << clientPort << std::endl;

        // receive data from android app (it wont exceed 1024 chars )
        char data[1024];
        std::size_t received;
        if (client.receive(data, sizeof(data), received) == sf::Socket::Done) {
            std::string receivedData(data, received);
            std::cout << "Received: " << receivedData << std::endl;
            // stop broadcasting if we found the device
            if (receivedData == "Hello from Graphos android app") {
                keepBroadcasting = false;
                std::cout << "pairing successful" << std::endl;
            }
        }
        client.disconnect();
    }
}

int main() {
    std::thread serverThread(startServer);
    broadcastMessage();

    // this will never happens since the server runs indefinitely
    serverThread.join();

    return 0;
}
