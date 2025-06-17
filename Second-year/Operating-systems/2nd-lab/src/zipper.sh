#!/bin/bash

# Customizable variables - edit these values
SOURCE_DIR="../src/"
DEST_DIR="test/"
ZIP_NAME="ssoo_p2_100522253_100522146_100522100.zip"
FILE1="autores.txt"
FILE2="Makefile"
FILE3="mygrep.c"
FILE4="scripter.c"

# Check if all files exist
missing_files=0
for file in "$FILE1" "$FILE2" "$FILE3" "$FILE4"; do
    if [ ! -f "$file" ]; then
        echo "Error: File $file not found!"
        missing_files=1
    fi
done

# Exit if any files are missing
if [ $missing_files -ne 0 ]; then
    echo "Aborting: Some files are missing."
    exit 1
fi

# Create the zip archive
zip -r "$ZIP_NAME" "$FILE1" "$FILE2" "$FILE3" "$FILE4"

# Check if zip operation succeeded
if [ $? -eq 0 ]; then
    echo "Successfully created archive: $ZIP_NAME"
else
    echo "Failed to create archive"
    exit 1
fi

# Move the zip file to destination directory
mv "$ZIP_NAME" "../$DEST_DIR/"