# Graphos 

Graphos is a tool designed to assist in digital presentations by allowing users to draw and annotate directly on their PC screen from an Android device. It is particularly useful for educators and content creators who need to highlight or elaborate on visual elements during live or recorded sessions.

## Features

- **Direct Drawing**: Users can draw directly on their PC screen from their Android phone or tablet.
- **Overlay Capability**: Graph# Graphos

Graphos is a tool that allows users to draw and annotate directly on their computer screen from an Android device. This is useful for anyone who needs to make live annotations on a screen, such as educators or content creators, without relying on a dedicated drawing tablet or other expensive hardware.

## Purpose

Graphos addresses the need for flexible, low-cost screen annotation tools that work across various platforms. Whether you are teaching, presenting, or recording content, Graphos provides a way to interact with your computer's screen using your Android device.

## Key Features

* Draw and annotate on your computer screen using an Android device
* Overlay feature that doesn’t block other applications
* Compatible with Windows, macOS, and Linux systems
* In-progress tool that’s being refined for better performance and usability

## How It Works

Graphos uses a client-server setup:

* **Client (Android App)**: The app on your Android device tracks touch input and sends it to the server.
* **Server (PC Component)**: The server receives the input and renders the annotations as an overlay on your screen. The overlay is transparent, allowing you to draw on top of whatever is currently on the screen.

## Development Status

This project is a work in progress. The current focus is on refining the drawing functionality, improving stability, and ensuring compatibility across different platforms. It is available for testing, and contributions are welcome.


## Compilation

To compile Graphos, you'll need to build both the Android client and the server components. For the Android app, open the `android_client/` directory in Android Studio and build the project as you would any standard Android app. For the server, navigate to the `server/` directory and compile the code using a C++ compiler (such as GCC). Make sure you have the necessary dependencies installed for both components. Detailed instructions will be provided as the project progresses.


## Acknowledgments
- https://github.com/texus/TransparentWindows.git

## License

This project is licensed under a custom MIT-style license with an attribution requirement.
See [CUSTOM\_LICENSE](./LICENSE) for full terms.


