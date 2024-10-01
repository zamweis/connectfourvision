
# Connect4Vision

**Connect4Vision** is an AI-driven application designed to analyze Connect Four gameplay using video input. This project leverages computer vision techniques to detect played pieces, track the state of the board, and identify the winning player.

## Table of Contents

- [Introduction](#introduction)
- [Features](#features)
- [Installation](#installation)
- [Usage](#usage)
- [Project Structure](#project-structure)
- [How It Works](#how-it-works)
- [Contributing](#contributing)
- [License](#license)

## Introduction

Connect4Vision is built to offer a novel way to analyze Connect Four games by using video footage as input. It can detect the placement of pieces in real time, update the state of the game board, and determine the winning player with visual feedback. This project is ideal for developers and enthusiasts looking to integrate computer vision into classic games or for educational purposes.

## Features

- **Real-Time Analysis**: Analyze the game in real-time using video input.
- **Piece Detection**: Recognizes and identifies the placement of both players' pieces.
- **Game State Tracking**: Maintains the current state of the game and updates dynamically.
- **Winner Detection**: Automatically identifies when a player has won.
- **Graphical Interface**: Includes a user-friendly interface to display results.

## Installation

To get started with Connect4Vision, clone the repository and build the project using CMake.

```bash
git clone <repository_url>
cd connectfourvision-main
mkdir build && cd build
cmake ..
make
```

### Prerequisites

Ensure that the following dependencies are installed:

- C++11 or higher
- OpenCV library
- Qt5 framework
- CMake

To install OpenCV and Qt5:

```bash
sudo apt-get update
sudo apt-get install libopencv-dev qt5-default
```

## Usage

To run the application, navigate to the build directory and execute the binary:

```bash
./connect4vision
```

The interface allows you to load a video file and analyze the gameplay. You can use one of the provided test videos (`testvideo.mp4`, `testvideo2.mp4`, `testvideo3.mp4`) or upload your own.

## Project Structure

```
connectfourvision-main/
├── .gitignore           # Git ignore file
├── CMakeLists.txt       # CMake build script
├── README.md            # Project documentation
├── main.cpp             # Main application logic
├── mainwindow.cpp       # GUI logic for the main window
├── mainwindow.h         # Header file for main window class
├── mainwindow.ui        # Qt user interface file
├── testvideo.mp4        # Sample video for testing
├── testvideo2.mp4       # Additional sample video
└── testvideo3.mp4       # Additional sample video
```

## How It Works

Connect4Vision uses OpenCV to analyze video frames and detect the placement of pieces on the Connect Four board. The program tracks the state of the board and checks for winning conditions after every move. The following steps outline the core workflow:

1. **Load Video**: The user selects a video file to analyze or uses the camera directy.
2. **Frame Analysis**: Each frame of the video is processed using OpenCV to detect piece placement.
3. **Board Update**: The detected pieces are mapped onto the internal representation of the game board.
4. **Win Check**: After each move, the program checks for any winning combinations.
5. **Result Display**: The final game state and winning player (if any) are displayed on the GUI.

## Contributing

We welcome contributions to enhance the functionality and performance of Connect4Vision. Please fork the repository, create a feature branch, and submit a pull request for review.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
