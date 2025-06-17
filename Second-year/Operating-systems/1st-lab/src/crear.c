#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define EXPECTED_ARGC 3 //crear , fichero and modo are the arguments

mode_t get_mode(char *arg_mode){
    /*
    Gets the octal mode from the string
    */
    // Check if length =3 or =4 with a 0 at the begining
    size_t len = strlen(arg_mode);
    if (!(len == 3 || (len == 4 && arg_mode[0] == '0'))) return -1;

    for (size_t i = (len == 4 ? 1 : 0); i < len; i++) // Check if the octal code has the correct characters
    {
        if (arg_mode[i] < '0' || arg_mode[i] > '7') return -1;
    }
    return strtol(arg_mode, NULL, 8); //convert string to long base 8
}


int main(int argc, char *argv[]) {
    if (argc != EXPECTED_ARGC) {
		// Check if the number of arguments is incorrect
        printf("Error: Invalid number of arguments.\n");
		printf("Program executing with %d arguments, the program must run with %d arguments.\n", argc - 1, EXPECTED_ARGC - 1);
        return -1;
    }
    char *filename = argv[1];
    mode_t mode = get_mode(argv[2]); // Convert from string to octal

    if (mode == (mode_t)-1) {
        printf("the argument: %s, is not an octal number with just 3 digits\n", argv[2]);
        return -1;
    }    
    
    // Save the mask and set it to zero
    mode_t old_umask = umask(0);
    
    // Try to open the file/ create the file
    int fd = open(filename, O_CREAT | O_EXCL | O_TRUNC, mode);
    if (fd == -1) {
        printf("Error creating file %s\n", filename);
        umask(old_umask); // Restore the previous mask
        return -1;
    }
    if (close(fd)<0){
        printf("close error\n");
        return -1;
    }
    
    // Restore the previous mask
    umask(old_umask);
    
    return 0;
}


