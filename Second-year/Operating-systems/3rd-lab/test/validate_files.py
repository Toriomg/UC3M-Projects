import sys
from collections import Counter


def compare_files(file1_path, file2_path):
    # Read the first file and store its lines
    with open(file1_path, 'r') as file1:
        lines1 = file1.readlines()
    
    # Read the second file and store its lines
    with open(file2_path, 'r') as file2:
        lines2 = file2.readlines()
    
    # Strip whitespace and newlines for accurate comparison
    stripped_lines1 = [line.strip() for line in lines1]
    stripped_lines2 = [line.strip() for line in lines2]
    
    # Check if the sets of lines are equal (order-independent)
    #if set(stripped_lines1) == set(stripped_lines2):
    if Counter(stripped_lines1) == Counter(stripped_lines2):
        print("The files have the same lines.")
        return 0
    else:
        print("The files do not have the same lines.")        
        return 1

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python validate_files.py <file1> <file2>")
        sys.exit(1)
    
    file1 = sys.argv[1]
    file2 = sys.argv[2]
    
    ret = compare_files(file1, file2)
    sys.exit(ret)
