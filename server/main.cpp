//g++ -o app drawConnection.cpp -lsfml-network -lsfml-system -lsfml-graphics -lsfml-window -lsfml-system -lX11 -lXext
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>
#include <memory>

// atomic flag to control the broadcasting
std::atomic<bool> keepBroadcasting(true);


std::shared_ptr<std::vector<char>> global_image_data = nullptr; 
std::mutex image_data_mutex; 
bool update;

void displayImage() {
    sf::VideoMode desktopMode = sf::VideoMode::getDesktopMode();
    sf::RenderWindow window(desktopMode, "Received Image", sf::Style::Fullscreen);

    sf::Image image;
    sf::Texture texture;
    sf::Sprite sprite;
    // Main loop for UI
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }
        {
            std::lock_guard<std::mutex> lock(image_data_mutex);
            if (update){
                update = false;
                const char * data = global_image_data->data();
                std::size_t size = global_image_data->size(); 
                if (!image.loadFromMemory(data, size)) {
                    std::cerr << "Error: Could not load image from memory.\n";
                    return;
                }
                if (!texture.loadFromImage(image)) {
                    std::cerr << "Error: Could not load texture from image.\n";
                    return;
                }
                sprite.setTexture(texture);

                // Resize the sprite to fit the window
                sf::Vector2u windowSize = window.getSize();
                sf::Vector2u imageSize = texture.getSize();
                float scaleX = static_cast<float>(windowSize.x) / imageSize.x;
                float scaleY = static_cast<float>(windowSize.y) / imageSize.y;
                sprite.setScale(scaleX, scaleY);
            }            
        }
        window.clear();
        window.draw(sprite);
        window.display();
    }
}

// Broadcast message to pair with Android device
void broadcastMessage() {
    sf::UdpSocket socket;
    socket.setBlocking(true);

    std::string message = "Hello from Graphos Desktop app";
    sf::IpAddress broadcastAddress = sf::IpAddress::Broadcast;
    unsigned short port = 5555;
    // we will broadcast each 100 millisecond until the connection is made
    while (keepBroadcasting) {
        if (socket.send(message.c_str(), message.size(), broadcastAddress, port) != sf::Socket::Done) {
            std::cerr << "Error sending broadcast message.\n";
        } else {
            std::cout << "Broadcast message sent.\n";
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));  
    }
}





void startServer() {
    sf::IpAddress serverIp = sf::IpAddress::Any;
    unsigned short serverPort = 5000;

    sf::TcpListener listener;

    // Binding the server to the port
    if (listener.listen(serverPort) != sf::Socket::Done) {
        std::cerr << "Error binding the server to port " << serverPort << std::endl;
        exit(-1);
    }

    std::cout << "Server listening on " << serverIp.toString() << ":" << serverPort << std::endl;

    while (true) {
        sf::TcpSocket client;


        if (listener.accept(client) != sf::Socket::Done) {
            std::cerr << "Error accepting connection" << std::endl;
            continue;
        }

        sf::IpAddress clientIp = client.getRemoteAddress();
        unsigned short clientPort = client.getRemotePort();
        std::cout << "Connection from " << clientIp.toString() << ":" << clientPort << std::endl;

        // Receive pairing message 
        char data[5000];
        std::size_t received;
        if (client.receive(data, sizeof(data), received) == sf::Socket::Done) {
            data[4999] = '\0';
            std::string receivedData(data, received);
            std::cout << "Received: " << receivedData << std::endl;

            // If paired, stop broadcasting
            if (receivedData == "Hello from Graphos android app") {
                keepBroadcasting = false;
                std::cout << "Pairing successful\n";
                client.disconnect();
                break; 
            }
        }

        client.disconnect();
    }

    // after pairing, wait for PNG files
    while (true) {
        sf::TcpSocket client;

        if (listener.accept(client) != sf::Socket::Done) {
            std::cerr << "Error accepting connection for PNG transfer" << std::endl;
            continue;
        }

        std::string size_str;
        char ch;
        while (true) {
            std::size_t receivedBytes = 0;
            if (client.receive(&ch, 1, receivedBytes) != sf::Socket::Done || ch == '\n') {
                break;
            }
            size_str += ch;
        }

        std::size_t image_size = std::stoul(size_str);
        std::cout << "Receiving image of size: " << image_size << " bytes\n";

        std::vector<char> image_data(image_size);
        std::size_t total_received = 0;
        
        // Wiating until we get the full image 
        while (total_received < image_size) {
            std::size_t received;
            if (client.receive(&image_data[total_received], image_size - total_received, received) != sf::Socket::Done) {
                std::cerr << "Error receiving image data.\n";
                break;
            }
            total_received += received;
        }

        if (total_received == image_size) {
            std::cout << "Image received successfully.\n";

            //changing the global image_data 
            {
                std::lock_guard<std::mutex> lock(image_data_mutex);
                update = true;
                global_image_data = std::make_shared<std::vector<char>>(image_data); // Store image data globally
            }
        } else {
            std::cerr << "Image receiving failed.\n";
        }

        client.disconnect();
    }
}

int main() {
    // Start server in a separate thread to handle connections
    std::thread serverThread(startServer);
    // Start broadcasting the pairing message
    broadcastMessage();
    //Start server in a separate thread 
    std::thread GUIThread(displayImage);

    serverThread.join();
    GUIThread.join();
    return 0;
}