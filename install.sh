#!/bin/bash

printf "Table Tennis Scoreboard Installer\n\n"

echo "This script will also install the following requirements from the internet:"
echo " - The OZone font"
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
echo ""

echo "Downloading the Ozone font..."
wget -O "/tmp/ozone.zip" "http://dl.dafont.com/dl/?f=ozone"
echo "Installing the Ozone font..."
unzip -o "/tmp/ozone.zip" "Ozone.ttf" -d "/tmp/"
mkdir -p "${HOME}/.fonts/"
mv -f "/tmp/Ozone.ttf" "${HOME}/.fonts/Ozone.ttf"
rm "/tmp/ozone.zip"
echo ""

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
echo ""

echo "Building Table Tennis Scoreboard..."
make
make install
echo ""

echo "Installation complete."
