#!/bin/bash

printf "Table Tennis Scoreboard Installer\n\n"

echo "This script will also install the following requirements from the internet:"
echo " - libgtk-3-dev"
echo " - The Ozone font"
echo " - pigpio"
echo ""
echo "If you wish to install only Table Tennis Scoreboard, run \"make && make install\" instead of this script."
echo ""
read -r -p "Would you like to continue? [y/N] " response
case "$response" in
    [yY])
        ;;
    *)
        exit 0;
        ;;
esac

echo "Downloading the Ozone font..."
wget -O "/tmp/ozone.zip" "http://dl.dafont.com/dl/?f=ozone"
echo "Installing the Ozone font..."
unzip -o "/tmp/ozone.zip" "Ozone.ttf" -d "/tmp/"
mkdir -p "${HOME}/.fonts/"
mv -f "/tmp/Ozone.ttf" "${HOME}/.fonts/Ozone.ttf"
rm "/tmp/ozone.zip"
echo ""

if [ ! -d "/usr/include/gtk-3.0" ]; then
    if [ -f "/usr/bin/apt-get" ]; then
        sudo apt-get update
        sudo apt-get install libgtk-3-dev
    else
        echo "It seems we don't have apt. You must install gtk-3-dev on your own to build this program."
    fi
else
    echo "libgtk-3-dev is already installed. Skipping..."
fi
echo ""

if [[ "$(uname -m)" == "arm"* ]]; then
    if [ ! -f "/usr/local/lib/libpigpio.so" ]; then
        echo "Downloading pigpio..."
        wget -O "/tmp/pigpio.zip" "http://abyz.co.uk/rpi/pigpio/pigpio.zip"
        echo "Building pigpio... (THIS MAY TAKE A WHILE)"
        unzip -o "/tmp/pigpio.zip" -d "lib/"
        rm "/tmp/pigpio.zip"
        cd "lib/PIGPIO"
        make -j4
        echo "Installing pigpio..."
        sudo make install
        cd ../..
    else
        echo "pigpio is already installed. Skipping..."
    fi
else
    echo "Skipping installation of pigpio since it seems we're not on a Pi."
fi
echo ""

echo "Building Table Tennis Scoreboard..."
make
make install
echo ""

echo "Installation complete."
