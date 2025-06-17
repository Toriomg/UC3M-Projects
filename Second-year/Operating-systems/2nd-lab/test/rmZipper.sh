#!/bin/bash



NAME="ssoo_p2_100522253_100522146_100522100"

# Remove the zip file if it exists
rm -f "${NAME}.zip"

# Remove the directory and its contents recursively
rm -rf "${NAME}/"

echo "Removed ${NAME}.zip and ${NAME}/ (if they existed)"