#!/bin/bash

echo "Installing desktop icon..."
mkdir -p "${HOME}/.local/share/applications/"
cp -f "table_tennis_scoreboard.desktop" "${HOME}/.local/share/applications/table_tennis_scoreboard.desktop"
sed -i "s/%PATH%/${PWD//\//\\/}/g" "${HOME}/.local/share/applications/table_tennis_scoreboard.desktop"

echo "Downloading scoreboard font..."
wget -O "ozone.zip" "http://dl.dafont.com/dl/?f=ozone"

echo "Installing scoreboard font..."
unzip -o "ozone.zip" "Ozone.ttf"
mkdir -p "${HOME}/.fonts/"
mv -f "Ozone.ttf" "${HOME}/.fonts/Ozone.ttf"
rm "ozone.zip"

echo "Complete!"
