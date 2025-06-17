#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

int size = 1024;

int main(int argc, char ** argv) {
    if (argc != 3) { // Check the correct number of inputs
        errno = EINVAL;
        perror("Usage: mygrep <ruta_fichero> <cadena_busqueda>"); //s
        return -1;
    }

    // Open the file
    int fd = open(argv[1], O_RDWR);
    if (fd == -1){
        // Check if the file was opended correctly
        perror("Error opening input script");
        return -1;
    }

    char *buffer = (char*)calloc(size, sizeof(char)); // clear all the space with calloc
    if(!buffer){
        perror("Error allocating the buffer");
        exit(-1);
    }

    int line_idx = 0;
    size_t byte_read;
    char char_read;     // Char read by the read syscall
    char string_found_in_file = 0;  // Flag whether for the seeked string found

    while ((byte_read = read(fd, &char_read, sizeof(char))) > 0){
        if(char_read == '\n'){
            if (strstr(buffer,argv[2])){
                printf("%s\n", buffer);
                string_found_in_file = 1;
            }
            memset(buffer, 0, size);
            line_idx = 0;
        } else {    
            buffer[line_idx++] = char_read;
        }

        if (line_idx >= size){
            size += 1024;
            char *temp = realloc(buffer, size * sizeof(char));;
                if (!temp) { // Error reallocating
                    if (!string_found_in_file){
                        printf("%s not found\n", argv[2]);
                    }
                    perror("Memory allocation failed");
                    close(fd);
                    free(buffer);
                    return -1;
                }
                buffer = temp; // set pointer
        }

    }
     // Final check to print the buffer if string was found in the last line
    if (strstr(buffer,argv[2])){
        printf("%s\n", buffer);
        string_found_in_file = 1;
    }
    if(byte_read == (size_t)-1){
        perror("Error reading the file");
        close(fd);
        return -1;
    }


    if (close(fd) < 0){
        perror("Error closing the input file");
        return -1;
    }

    if (!string_found_in_file){ // String not found then message displayed
        printf("%s not found\n", argv[2]); // Don`t sure how to put the quotes MUST ASK ABOUT
    }
    free(buffer);
    return 0;
}