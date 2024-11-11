# About

This is a small hands-on project for the Robo 24 Workshop. 

# Wiring
The ESP32 is directly connected to the HC SR04 sensors. The trigger pin is connected to GPIO 1 and the echo pin is connected to GPIO 2.
```
    ----|o GPIO 1  |       |       5V o|----
    | --|o GPIO 2  |__USB__|      GND o|-- |
    | | |o                            o| | |
    | | |o                            o| | |
    | | |o                            o| | |
    | | |o                            o| | |
    | | |o                            o| | |
    | | |o                            o| | |
    | | |______________________________| | |
    | |                                  | |
    | |                                  | |
    | |                                  | |
    | |                                  | |
    | -----------------  ----------------- |
    ----------------- |  | -----------------
                    | |  | |
                    | |  | |
                  --|-|--|--
                  | |  \ |
                  | |  | |
                  | |  | |
                  | |  | |
                  | |  | |
        __________|_|__|_|___________
        |         o o  o o          |
        |   5V Trigger Echo GND     |
        |___________________________|
            |____|        |____|
```

# Installation
1. Setup the ESP IDF (v5.2.3). Stop at "Start a Project" (that is this repo)
  - [Ubuntu & MacOS install](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/linux-macos-setup.html#step-1-install-prerequisites)
  - [Windows](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/windows-setup.html)
2. With the ESP IDF environment activated, run:
```
pip install ipython=8.27.0
```

# Usage
- Build: `idf.py build`
- Build & flash: `idf.py flash`
- Shell: `python shell.py`

The shell will display documentation on how to use it.
