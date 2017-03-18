#!/bin/bash

printf "Table Tennis Scoreboard Uninstaller\n\n"

if [ -d "lib/PIGPIO" ]; then
    echo "Uninstalling pigpio"
    cd "lib/PIGPIO"
    sudo make uninstall
    cd ../..
    sudo rm -rf "lib/PIGPIO"
else
    echo "It seems pigpio wasn't installed by me. Not uninstalling..."
fi
echo ""

echo "Removing font..."
rm "${HOME}/.fonts/Ozone.ttf"
rmdir "${HOME}/.fonts/" 2> /dev/null
echo ""

make uninstall
make clean
