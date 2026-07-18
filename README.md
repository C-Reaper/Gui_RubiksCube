# Project README

## Overview
This project is a basic 3D graphics application using C/C++. It features camera controls, simple rendering, and basic transformations. The application uses basic matrix operations for camera manipulation and rendering.

## Features
- Basic 3D rendering
- Camera control with yaw, pitch, and translation
- Projection matrix setup

## Project Structure
```
<Project>/
├── build/              # .exe files produced by Main.c
└── src/                # source code
│   ├── Main.c          # Entry point
│   └── Camera.h        # Header file for camera operations
└── Makefile.linux      # Linux Build configuration
└── README.md           # This file
```

### Prerequisites
- C/C++ Compiler and Debugger (GCC, Clang)
- Standard development tools

## Build & Run
To build the project, navigate to the project directory and run:
```sh
make -f Makefile.linux all
```
This will compile the source code and generate an executable in the `build/` directory.

To execute the compiled application:
```sh
./build/Main
```

For a clean rebuild:
```sh
make -f Makefile.linux clean
make -f Makefile.linux all
```

If you encounter any issues, ensure your environment is properly set up with the required tools and libraries.