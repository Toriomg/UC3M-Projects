#!/usr/bin/env python3
import os
import random

def main():
    # Define the directory and file paths
    directory = os.path.join("..", "src", "t")
    filename = "output.txt"
    file_path = os.path.join(directory, filename)

    max_belts = 50
    

    # Create the directory if it doesn't exist
    os.makedirs(directory, exist_ok=True)

    # Define the content to write
    belts = []
    for i in range(max_belts):#random.randint(1,max_belts)):
        belt = [i]
        belt.append(random.randint(3, 6))
        belt.append(random.randint(5, 15))
        belts.append(belt)

    
    content = str(max_belts) + ' '
    for belt in belts:
        for numb in belt:
            content = content + str(numb) + ' '


    # Create and write to the file
    try:
        with open(file_path, "w") as f:
            f.write(content)
        print(f"File created successfully at: {file_path}")
    except Exception as e:
        print(f"An error occurred while creating the file: {e}")

if __name__ == "__main__":
    main()
