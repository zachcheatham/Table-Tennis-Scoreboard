#!/bin/bash

echo "Removing desktop icon..."
rm "${HOME}/.local/share/applications/table_tennis_scoreboard.desktop"

echo "Removing font..."
rm "${HOME}/.fonts/Ozone.ttf"
rmdir "${HOME}/.fonts/" 2> /dev/null

echo "Complete!"
