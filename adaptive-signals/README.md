credits:
This Project Was Made By 4  students :

houssam bengourou
ayoub sacha
chaimae zaki
hajar tigharmte


🚦 Intelligent Traffic Management System (ITMS)

![alt text](https://img.shields.io/badge/C%2B%2B-17-blue.svg)


![alt text](https://img.shields.io/badge/UI-Raylib-orange.svg)


![alt text](https://img.shields.io/badge/Build-CMake-green.svg)

An advanced traffic control simulation designed to reduce urban congestion. This system replaces traditional fixed-timer traffic lights with adaptive logic that adjusts green light durations based on real-time vehicle queue lengths and traffic density.

✨ Key Features

Adaptive Timing Logic: Automatically adjusts green light cycles from 10s up to 90s based on traffic flow (Low, Medium, High, or Jam).

Multi-Intersection Coordination: A centralized Coordinator manages a grid of intersections (up to 10x10) to synchronize traffic flow.

Special Modes:

🚨 Emergency Mode: Instantly grants priority to specific directions for emergency vehicles.

🌙 Night Mode: Detects low traffic volumes and switches lights to a "flashing yellow" state for efficiency.

Realistic 2D Simulation:

Vehicle behaviors including lane following and turning logic.

Collision avoidance and stop-line detection.

Interactive menu to customize the city grid size.

Unit Testing: Comprehensive test suite for sensors and decision logic.

🛠 Prerequisites

Ensure you have the following installed:

Compiler: GCC 9+ or Clang (supporting C++17)

Build System: CMake 3.10+

Library: Raylib (Must be installed on your system)

Installing Raylib (Ubuntu/Debian)
code
Bash
download
content_copy
expand_less
sudo apt install libraylib-dev
🚀 Installation & Building

Clone the repository

code
Bash
download
content_copy
expand_less
git clone https://github.com/houssam-icon/ADAPTIVE_SIGNAL_CONTROL.git
cd intelligent-traffic-system

Build the project

code
Bash
download
content_copy
expand_less
mkdir build && cd build
cmake ..
make

Run the Simulation

code
Bash
download
content_copy
expand_less
./run_tests # To run logic verification
./Project    # To launch the graphical simulation
📖 How it Works

The system is built on a modular architecture:

Capteur (Sensors): Monitors specific lanes, calculates queue lengths, and reports "Vehicles per Minute" (flux).

LogiqueAdaptative: The brain of the system. It processes sensor data to calculate the optimal duration for the next green phase using specific adjustment factors.

IntersectionController: Manages the state machine for an individual intersection (Green -> Yellow -> Red -> All-Red).

Coordinator: Synchronizes multiple controllers to ensure smooth transitions across the city grid.

📂 Project Structure
code
Text
download
content_copy
expand_less
├── src/
│   ├── main.cpp                # Raylib simulation and UI logic
│   ├── Sensor.cpp              # Traffic detection implementation
│   ├── AdaptiveLogic.cpp       # Decision-making algorithms
│   ├── IntersectionController.cpp # Signal phase management
│   ├── Coordinator.cpp         # Multi-intersection sync
│   └── trafficlights.cpp       # Traffic light state objects
├── include/                    # Header files
├── tests/
│   └── testcapteur.cpp         # Unit tests for logic validation
└── CMakeLists.txt              # Build configuration
🧪 Running Tests

Validation of the adaptive logic is handled via assertions. To verify the logic:

code
Bash
download
content_copy
expand_less
cd build
./run_tests

Successful output will show: Tous les tests sont PASSÉS !

🎮 Controls

Menu: Use the + / - buttons to set the number of rows and columns for your city.

In-Game:

M: Return to main menu.

Esc: Exit application.

📄 License

This project is open-source. See the LICENSE file for details.

✉️ Contact

Project Link: https://github.com/houssam-icon/ADAPTIVE_SIGNAL_CONTROL
