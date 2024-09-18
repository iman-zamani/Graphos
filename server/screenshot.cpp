//g++ -o screenshot_program screenshot.cpp `pkg-config --cflags --libs opencv4` -lX11
//g++ -o screenshot_program screenshot.cpp `pkg-config --cflags --libs opencv4` -framework ApplicationServices
//g++ -o screenshot_program screenshot.cpp -lopencv_core -lopencv_imgcodecs -lopencv_highgui -lgdi32
#include <opencv2/opencv.hpp>
#include <iostream>

#ifdef _WIN32
    #include <Windows.h>
#elif __linux__
    #include <X11/Xlib.h>
    #include <X11/Xutil.h>
#elif __APPLE__
    #include <ApplicationServices/ApplicationServices.h>
#endif

cv::Mat captureScreen(int width, int height) {
#ifdef _WIN32
    // Windows screenshot
    HDC hScreenDC = GetDC(nullptr);   // Get the device context of the screen
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);  // Create a compatible DC

    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, width, height);
    HGDIOBJ oldBitmap = SelectObject(hMemoryDC, hBitmap);

    BitBlt(hMemoryDC, 0, 0, width, height, hScreenDC, 0, 0, SRCCOPY);  // Copy screen to memory DC
    hBitmap = (HBITMAP)SelectObject(hMemoryDC, oldBitmap);

    BITMAPINFOHEADER bi;
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = width;
    bi.biHeight = -height;  // Negative to ensure top-down bitmap
    bi.biPlanes = 1;
    bi.biBitCount = 24;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    cv::Mat mat(height, width, CV_8UC3);
    GetDIBits(hMemoryDC, hBitmap, 0, height, mat.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    DeleteObject(hBitmap);
    DeleteDC(hMemoryDC);
    ReleaseDC(nullptr, hScreenDC);

    return mat;

#elif __linux__
    // Linux screenshot using X11
    Display* display = XOpenDisplay(nullptr);
    Window root = DefaultRootWindow(display);
    XWindowAttributes attributes = {0};
    XGetWindowAttributes(display, root, &attributes);

    XImage* img = XGetImage(display, root, 0, 0, width, height, AllPlanes, ZPixmap);
    cv::Mat mat(height, width, CV_8UC4, img->data);
    cv::Mat matRGB;
    cv::cvtColor(mat, matRGB, cv::COLOR_BGRA2BGR);  // Convert from BGRA to BGR

    XDestroyImage(img);
    XCloseDisplay(display);

    return matRGB;

#elif __APPLE__
    // macOS screenshot using Quartz
    CGImageRef image = CGDisplayCreateImage(kCGDirectMainDisplay);
    int imgWidth = CGImageGetWidth(image);
    int imgHeight = CGImageGetHeight(image);
    
    cv::Mat mat(imgHeight, imgWidth, CV_8UC4);  // Create a matrix for the image
    CGContextRef context = CGBitmapContextCreate(mat.data, imgWidth, imgHeight, 8, imgWidth * 4,
                                                 CGImageGetColorSpace(image), kCGImageAlphaNoneSkipLast | kCGBitmapByteOrder32Little);
    CGContextDrawImage(context, CGRectMake(0, 0, imgWidth, imgHeight), image);
    CGContextRelease(context);
    CGImageRelease(image);

    cv::Mat matRGB;
    cv::cvtColor(mat, matRGB, cv::COLOR_BGRA2BGR);  // Convert from BGRA to BGR

    return matRGB;
#endif
}

int main() {
#ifdef _WIN32
    int width = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);
#elif __linux__
    Display* display = XOpenDisplay(nullptr);
    Screen* screen = DefaultScreenOfDisplay(display);
    int width = screen->width;
    int height = screen->height;
    XCloseDisplay(display);
#elif __APPLE__
    int width = CGDisplayPixelsWide(kCGDirectMainDisplay);
    int height = CGDisplayPixelsHigh(kCGDirectMainDisplay);
#endif

    cv::Mat screenshot = captureScreen(width, height);
    if (screenshot.empty()) {
        std::cerr << "Failed to capture screen!" << std::endl;
        return -1;
    }

    // Save the screenshot as PNG
    if (cv::imwrite("screenshot.png", screenshot)) {
        std::cout << "Screenshot saved to screenshot.png" << std::endl;
    } else {
        std::cerr << "Error saving screenshot" << std::endl;
    }

    return 0;
}
