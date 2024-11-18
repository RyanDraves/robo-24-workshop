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
1. Setup the ESP IDF (v5.2.3). If using the web instructions, run the `./install.sh` script the args `./install.sh esp32c3 --enable-pytest`
  - Ubuntu & MacOS
    - Run `./setup.sh`. This will prompt for a `sudo` password early on, then can run in the background
    - [Web instructions if the script fails](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/linux-macos-setup.html#step-1-install-prerequisites)
  - Windows
    - [Instructions](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/windows-setup.html)
    - Stop at "Start a Project" (that is this repo)
2. With the ESP IDF environment activated, run:
```
pip install ipython==8.27.0
pip install nbformat==5.10.4
pip install pandas==2.2.3
pip install plotly==5.24.1
```
(you may skip step 2 if `./setup.sh` ran successfully)

# Usage
- Build: `idf.py build`
- Build & flash: `idf.py flash`
- Shell: `python shell.py`
- Test: `pytest pytest_dut.py`

The shell will display documentation on how to use it.
