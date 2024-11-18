#!/bin/bash
#
# This script sets up the environment for the project.
# It probably only works on Ubuntu.
#
# Based on
# https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/linux-macos-setup.html


set -e
set -o pipefail

APT_PACKAGES=(
    git
    wget
    flex
    bison
    gperf
    python3
    python3-pip
    python3-venv
    cmake
    ninja-build
    ccache
    libffi-dev
    libssl-dev
    dfu-util
    libusb-1.0-0
)

BREW_PACKAGES=(
    cmake
    ninja
    dfu-util
    python3
)

PYTHON_PACKAGES=(
    'ipython==8.27.0'
    'nbformat==5.10.4'
    'pandas==2.2.3'
    'plotly==5.24.1'
)

function install_apt_packages() {
    local packages=("$@")

    # Check if apt is available
    if ! check_command apt; then
        echo "apt not found"
        return 1
    fi

    # Check if apt packages are already installed
    all_installed=true
    missing_packages=()
    for package in "${packages[@]}"; do
        if dpkg -s "$package" &> /dev/null; then
            continue
        fi
        all_installed=false
        missing_packages+=("$package")
    done

    if $all_installed; then
        echo "All apt packages are already installed"
        return 0
    fi

    echo "Installing missing apt packages: ${missing_packages[@]}"

    # Install apt packages
    sudo apt update
    sudo apt install -y "${missing_packages[@]}"
}

function install_brew_packages() {
    local packages=("$@")

    # Check if brew is available
    if ! check_command brew; then
        echo "brew not found"
        return 1
    fi

    # Install brew packages
    brew install "${packages[@]}"
}

function clone_esp_idf() {
    mkdir -p ~/esp
    cd ~/esp

    # Check if the repo is already cloned
    if [ -d "esp-idf" ]; then
        echo "esp-idf is already cloned"
        return 0
    fi

    git clone -b v5.2.3 --recursive https://github.com/espressif/esp-idf.git
}

function install_tools() {
    cd ~/esp/esp-idf

    ./install.sh esp32c3
}

function setup_env() {
    maybe_add_to_profile "alias get_idf='. ~/esp/esp-idf/export.sh'"

    # Check if the IDF_PATH is already set
    if [ -n "$IDF_PATH" ]; then
        echo "IDF_PATH is already set"
        return 0
    fi

    # Source the export script for later use
    . ~/esp/esp-idf/export.sh
}

function maybe_add_to_profile() {
    local line=$1

    # Check the name of the current shell
    local shell_name=$(basename $SHELL)
    
    # Find a profile file suitable for the current shell
    if [ $shell_name == "bash" ]; then
        profile=~/.bash_aliases
    elif [ $shell_name == "zsh" ]; then
        profile=~/.zprofile
    else
        echo "Unsupported shell: $shell_name"
        return 1
    fi

    if ! grep -q "$line" $profile; then
        echo "$line" >> $profile
        echo "Added $line to $profile"
    fi
}

function install_python_packages() {
    local packages=("$@")

    # Check if pip is available
    if ! check_command pip; then
        echo "pip not found"
        return 1
    fi

    # Install python packages
    pip install "${packages[@]}"
}

function print_instructions() {
    local current_shell=$(basename $SHELL)

    # Find the file to source
    if [ $current_shell == "bash" ]; then
        profile=~/.bash_aliases
    elif [ $current_shell == "zsh" ]; then
        profile=~/.zprofile
    else
        echo "Unsupported shell: $current_shell"
        return 1
    fi

    echo ""
    echo "The environment is now set up."
    echo "Please run the following commands to activate the environment:"

    # Print in purple
    echo -e "\e[35m"
    echo "source $profile"
    echo "get_idf"
    echo -e "\e[0m"
}

function check_command() {
    local command=$1
    if ! command -v $command &> /dev/null; then
        echo "$command not found"
        return 1
    fi

    return 0
}

function is_macos() {
    [[ $(uname) == "Darwin" ]]
}

function success() {
    echo -e "\e[32m"
    echo "(っ◔◡◔)っ ♥success♥"
    echo -e "\e[0m"
}

# Check if we already have a virtual environment active
if [ -n "$VIRTUAL_ENV" ]; then
    echo "A virtual environment is already active. Please deactivate it first."
    return 1
fi

if is_macos; then
    install_brew_packages "${BREW_PACKAGES[@]}"
else
    install_apt_packages "${APT_PACKAGES[@]}"
fi
clone_esp_idf
install_tools
setup_env
install_python_packages "${PYTHON_PACKAGES[@]}"
print_instructions
success

