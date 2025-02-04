
// linux: g++ -o Graphos main.cpp -lsfml-network -lsfml-system -lsfml-graphics -lsfml-window -lsfml-system -lX11 -lXext
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <cstring>
#include <fstream>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>
#include <memory>

#include "blockingconcurrentqueue.h"

#define debug false
void print(std::string str)
{
    if (debug)
    {
        std::cout << str << std::endl;
    }
    return;
}

int windowSizeX;
int windowSizeY;

// atomic flag to control the broadcasting
std::atomic<bool> keepBroadcasting(true);

// Alias for byte buffer (contiguous, movable)
using ByteBuffer = std::vector<std::byte>;

// Create A queue for getting image data
moodycamel::BlockingConcurrentQueue<ByteBuffer> global_image_data_queue;

bool windowVisible;
#if defined(SFML_SYSTEM_WINDOWS)
#include <windows.h>

bool setShape(HWND hWnd, const sf::Image &image)
{
    const sf::Uint8 *pixelData = image.getPixelsPtr();

    HRGN hRegion = CreateRectRgn(0, 0, image.getSize().x, image.getSize().y);
    bool transparentPixelFound = false;
    unsigned int rectLeft = 0;
    for (unsigned int y = 0; y < image.getSize().y; y++)
    {
        for (unsigned int x = 0; x < image.getSize().x; x++)
        {
            const bool isTransparentPixel = (pixelData[y * image.getSize().x * 4 + x * 4 + 3] == 0);
            if (isTransparentPixel && !transparentPixelFound)
            {
                transparentPixelFound = true;
                rectLeft = x;
            }
            else if (!isTransparentPixel && transparentPixelFound)
            {
                HRGN hRegionPixel = CreateRectRgn(rectLeft, y, x, y + 1);
                CombineRgn(hRegion, hRegion, hRegionPixel, RGN_XOR);
                DeleteObject(hRegionPixel);
                transparentPixelFound = false;
            }
        }
        if (transparentPixelFound)
        {
            HRGN hRegionPixel = CreateRectRgn(rectLeft, y, image.getSize().x, y + 1);
            CombineRgn(hRegion, hRegion, hRegionPixel, RGN_XOR);
            DeleteObject(hRegionPixel);
            transparentPixelFound = false;
        }
    }

    SetWindowRgn(hWnd, hRegion, true);
    DeleteObject(hRegion);
    return true;
}

bool setTransparency(HWND hWnd, unsigned char alpha)
{
    SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
    SetLayeredWindowAttributes(hWnd, 0, alpha, LWA_ALPHA);
    return true;
}

#elif defined(SFML_SYSTEM_LINUX)
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/shape.h>

bool setShape(Window wnd, const sf::Image &image)
{
    Display *display = XOpenDisplay(NULL);
    int event_base, error_base;
    if (!XShapeQueryExtension(display, &event_base, &error_base))
    {
        XCloseDisplay(display);
        return false;
    }

    const sf::Uint8 *pixelData = image.getPixelsPtr();
    Pixmap pixmap = XCreatePixmap(display, wnd, image.getSize().x, image.getSize().y, 1);
    GC gc = XCreateGC(display, pixmap, 0, NULL);

    XSetForeground(display, gc, 1);
    XFillRectangle(display, pixmap, gc, 0, 0, image.getSize().x, image.getSize().y);

    XSetForeground(display, gc, 0);
    bool transparentPixelFound = false;
    unsigned int rectLeft = 0;
    for (unsigned int y = 0; y < image.getSize().y; y++)
    {
        for (unsigned int x = 0; x < image.getSize().x; x++)
        {
            const bool isTransparentPixel = (pixelData[y * image.getSize().x * 4 + x * 4 + 3] == 0);
            if (isTransparentPixel && !transparentPixelFound)
            {
                transparentPixelFound = true;
                rectLeft = x;
            }
            else if (!isTransparentPixel && transparentPixelFound)
            {
                XFillRectangle(display, pixmap, gc, rectLeft, y, x - rectLeft, 1);
                transparentPixelFound = false;
            }
        }
        if (transparentPixelFound)
        {
            XFillRectangle(display, pixmap, gc, rectLeft, y, image.getSize().x - rectLeft, 1);
            transparentPixelFound = false;
        }
    }

    XShapeCombineMask(display, wnd, ShapeBounding, 0, 0, pixmap, ShapeSet);
    XFreeGC(display, gc);
    XFreePixmap(display, pixmap);
    XFlush(display);
    XCloseDisplay(display);
    return true;
}

bool setTransparency(Window wnd, unsigned char alpha)
{
    Display *display = XOpenDisplay(NULL);
    unsigned long opacity = (0xffffffff / 0xff) * alpha;
    Atom property = XInternAtom(display, "_NET_WM_WINDOW_OPACITY", false);
    if (property != None)
    {
        XChangeProperty(display, wnd, property, XA_CARDINAL, 32, PropModeReplace, (unsigned char *)&opacity, 1);
        XFlush(display);
        XCloseDisplay(display);
        return true;
    }
    else
    {
        XCloseDisplay(display);
        return false;
    }
}

#undef None
#elif defined(SFML_SYSTEM_MACOS)
#include <objc/objc.h>
#include <objc/message.h>
#include <objc/runtime.h>
bool setShape(sf::WindowHandle handle, const sf::Image &image);
bool setTransparency(sf::WindowHandle handle, unsigned char alpha);
#else
bool setShape(sf::WindowHandle handle, const sf::Image &image)
{
    return false;
}

bool setTransparency(sf::WindowHandle handle, unsigned char alpha)
{
    return false;
}
#endif

void setWindowAlwaysOnTop(sf::RenderWindow &window)
{
#ifdef _WIN32
    // Windows: Use SetWindowPos to make the window always on top
    HWND hwnd = window.getSystemHandle();
    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
#elif __APPLE__
    // macOS: Use Objective-C runtime to set the NSWindow level to NSStatusWindowLevel
    void *nsWindow = window.getSystemHandle();
    id windowObj = reinterpret_cast<id>(nsWindow);
    SEL setLevelSel = sel_registerName("setLevel:");
    int NSStatusWindowLevel = 24;
    ((void (*)(id, SEL, int))objc_msgSend)(windowObj, setLevelSel, NSStatusWindowLevel);
#elif __linux__
    // Linux: Use X11 to set the _NET_WM_STATE_ABOVE property
    Display *display = XOpenDisplay(NULL);
    if (display)
    {
        Window xWindow = window.getSystemHandle();
        Atom wmState = XInternAtom(display, "_NET_WM_STATE", False);
        Atom wmStateAbove = XInternAtom(display, "_NET_WM_STATE_ABOVE", False);

        XEvent xEvent;
        memset(&xEvent, 0, sizeof(xEvent));
        xEvent.type = ClientMessage;
        xEvent.xclient.window = xWindow;
        xEvent.xclient.message_type = wmState;
        xEvent.xclient.format = 32;
        xEvent.xclient.data.l[0] = 1;
        xEvent.xclient.data.l[1] = wmStateAbove;
        xEvent.xclient.data.l[2] = 0;
        xEvent.xclient.data.l[3] = 1;

        XSendEvent(display, DefaultRootWindow(display), False, SubstructureNotifyMask | SubstructureRedirectMask, &xEvent);
        XFlush(display);
        XCloseDisplay(display);
    }
#endif
}
void displayDrawing()
{
    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    windowSizeX = desktop.width;
    windowSizeY = desktop.height;
    sf::VideoMode defaultMode = sf::VideoMode(windowSizeX, windowSizeY);
    sf::Image image;
    // just for avoiding some bugs
    image.loadFromFile("default.png");
    sf::RenderWindow window(sf::VideoMode(image.getSize().x, image.getSize().y, 32), "Graphos", sf::Style::None);
    window.setPosition(sf::Vector2i((sf::VideoMode::getDesktopMode().width - image.getSize().x) / 2,
                                    (sf::VideoMode::getDesktopMode().height - image.getSize().y) / 2));
    window.setFramerateLimit(60);

    sf::Texture texture;
    sf::Sprite sprite;
    // Main loop for UI
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        // this code is dequeing
        ByteBuffer buff;
        global_image_data_queue.wait_dequeue(buff);

        const std::byte *data = buff.data();
        std::size_t size = buff.size();
        if (!image.loadFromMemory(data, size))
        {
            std::cerr << "Error: Could not load image from memory.\n";
            return;
        }
        if (!texture.loadFromImage(image))
        {
            std::cerr << "Error: Could not load texture from image.\n";
            return;
        }
        sprite.setTexture(texture);

        // Resize the sprite to fit the window
        sf::Vector2u windowSize(windowSizeX, windowSizeY);
        sf::Vector2u imageSize = texture.getSize();
        float scaleX = static_cast<float>(windowSizeX) / imageSize.x;
        float scaleY = static_cast<float>(windowSizeY) / imageSize.y;
        sprite.setScale(scaleX, scaleY);
        setShape(window.getSystemHandle(), image);
        window.setSize(sf::Vector2u(image.getSize().x, image.getSize().y));
        // make the window to be always in top
        setWindowAlwaysOnTop(window);

        window.setVisible(windowVisible);

        window.clear(sf::Color::Transparent);
        window.draw(sprite);
        window.display();
    }
}

// Broadcast message to pair with Android device
void broadcastMessage()
{
    sf::UdpSocket socket;
    socket.setBlocking(true);

    std::string message = "Hello from Graphos Desktop app";
    sf::IpAddress broadcastAddress = sf::IpAddress::Broadcast;
    unsigned short port = 5555;
    // we will broadcast each 100 millisecond until the connection is made
    while (keepBroadcasting)
    {
        if (socket.send(message.c_str(), message.size(), broadcastAddress, port) != sf::Socket::Done)
        {
            std::cerr << "Error sending broadcast message.\n";
        }
        else
        {
            print("Broadcast message sent.");
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void startServer()
{
    sf::IpAddress serverIp = sf::IpAddress::Any;
    unsigned short serverPort = 5000;

    sf::TcpListener listener;
    listener.setBlocking(false);
    // Binding the server to the port
    if (listener.listen(serverPort) != sf::Socket::Done)
    {
        std::cerr << "Error binding the server to port " << serverPort << std::endl;
        exit(-1);
    }

    listener.setBlocking(true);
    print("Server listening on " + serverIp.toString() + ":" + std::to_string(serverPort));

    while (true)
    {
        sf::TcpSocket client;

        if (listener.accept(client) != sf::Socket::Done)
        {
            std::cerr << "Error accepting connection" << std::endl;
            continue;
        }

        sf::IpAddress clientIp = client.getRemoteAddress();
        unsigned short clientPort = client.getRemotePort();
        print("Connection from " + clientIp.toString() + ":" + std::to_string(clientPort));

        // Receive pairing message
        char data[5000];
        std::size_t received;
        if (client.receive(data, sizeof(data), received) == sf::Socket::Done)
        {
            data[4999] = '\0';
            std::string receivedData(data, received);
            print("Received: " + receivedData);

            // If paired, stop broadcasting
            if (receivedData == "Hello from Graphos android app")
            {
                keepBroadcasting = false;
                print("Pairing successful");
                client.disconnect();
                break;
            }
        }

        client.disconnect();
    }

    // after pairing, wait for PNG files
    while (true)
    {
        sf::TcpSocket client;

        if (listener.accept(client) != sf::Socket::Done)
        {
            std::cerr << "Error accepting connection for PNG transfer" << std::endl;
            continue;
        }

        std::string size_str;
        char ch;
        while (true)
        {
            std::size_t receivedBytes = 0;
            if (client.receive(&ch, 1, receivedBytes) != sf::Socket::Done || ch == '\n')
            {
                break;
            }
            size_str += ch;
        }

        std::size_t image_size = std::stoul(size_str);

        // what hell bro? just plz use size_str it's already fucking shows the size in string!!!!
        print("Receiving image of size: " + std::to_string(image_size) + " bytes");

        // std::vector<char> image_data(image_size);
        ByteBuffer image_data(image_size);
        std::size_t total_received = 0;

        // Waiting until we get the full image
        while (total_received < image_size)
        {
            std::size_t received;
            if (client.receive(&image_data[total_received], image_size - total_received, received) != sf::Socket::Done)
            {
                std::cerr << "Error receiving image data.\n";
                break;
            }
            total_received += received;
        }

        if (total_received == image_size)
        {
            print("Image received successfully");

            // changing the global image_data
            {
                if (image_size != 0)
                {
                    global_image_data_queue.enqueue(image_data);
                    windowVisible = true;
                }
                else
                {
                    windowVisible = false;
                }
            }
        }
        else
        {
            std::cerr << "Image receiving failed.\n";
        }

        client.disconnect();
    }
}

int main()
{
    std::thread serverThread(startServer);
    broadcastMessage();
    std::thread GUIThread(displayDrawing);

    serverThread.join();
    GUIThread.join();
    return 0;
}