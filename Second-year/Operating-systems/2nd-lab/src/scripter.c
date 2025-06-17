#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

/* CONST VARS */
const int max_line = 1024;
const int max_commands = 10;
#define max_redirections 3 // stdin, stdout, stderr
#define max_args 15

/* VARS TO BE USED FOR THE STUDENTS */

// structure equivalent to the "argv" that stores the command line when
// executing a program.
char *argvv[max_args];
// redirections array. If a redirection is detected, the file name is referenced
// at the corresponding position
char *filev[max_redirections];
// indicates whether a command or command sequence is to be executed in
// foreground (0) or bg (1).
int background = 0;

int max_lines = 10;

int tokenizar_linea(char *linea, char *delim, char *tokens[], int max_tokens) {
    /*
    This function splits a char* line into different tokens based on a given
    character
    @return Number of tokens
    */
    int i = 0;
    char *token = strtok(linea, delim);
    while (token != NULL && i < max_tokens - 1) {
        tokens[i++] = token;
        token = strtok(NULL, delim);
    }
    tokens[i] = NULL;
    return i;
}

void procesar_redirecciones(char *args[]) {
    /*
    This function processes the command line to evaluate if there are
    redirections. If any redirection is detected, the destination file is
    indicated in filev[i] array. filev[0] for STDIN filev[1] for STDOUT filev[2]
    for STDERR
    */

    // initialization for every command
    filev[0] = NULL;
    filev[1] = NULL;
    filev[2] = NULL;
    // Store the pointer to the filename if needed.
    // args[i] set to NULL once redirection is processed
    for (int i = 0; i < max_args; i++) {
        if (args[i] == NULL)
            continue;
        if (strcmp(args[i], "<") == 0) {
            filev[0] = args[i + 1];
            args[i] = NULL;
            args[i + 1] = NULL;
        } else if (strcmp(args[i], ">") == 0) {
            filev[1] = args[i + 1];
            args[i] = NULL;
            args[i + 1] = NULL;
        } else if (strcmp(args[i], "!>") == 0) {
            filev[2] = args[i + 1];
            args[i] = NULL;
            args[i + 1] = NULL;
        }
    }
}

void print_commands() {
    /*
    Delete for submission !!!!
    */
    printf("Comando = %s\n", argvv[0]);
    for (int arg = 1; arg < max_args; arg++)
        if (argvv[arg] != NULL)
            printf("Args = %s\n", argvv[arg]);

    printf("Background = %d\n", background);
    if (filev[0] != NULL)
        printf("Redir [IN] = %s\n", filev[0]);
    if (filev[1] != NULL)
        printf("Redir [OUT] = %s\n", filev[1]);
    if (filev[2] != NULL)
        printf("Redir [ERR] = %s\n", filev[2]);
}

void execute_command(int num_commands, int iter_command, int *prev_pipe_fd) {
    int fd[2] = {0};
    pid_t pid;
    int input_fd = STDIN_FILENO, output_fd = STDOUT_FILENO;

    // If the numb of commands is >1 that means that there are pipes in the line
    if (num_commands > 1 && iter_command != num_commands - 1) {
        if (pipe(fd) == -1) {
            perror("Error creating pipe");
            exit(-1);
        }
    }

    pid = fork();

    if (pid < 0) {
        perror("Error creating the process");
        exit(-1);
    }
    if (pid == 0) {
        // --CHILD PROCESS--
        // Handle input redirection
        if (filev[0]) {
            if (iter_command == 0) {
                input_fd = open(filev[0], O_RDONLY);
                if (input_fd == -1) {
                    perror("Error opening input file for read");
                    exit(-1);
                }
            } else {
                errno = EINVAL;
                perror("Commands between pipes cannot have redirections");
                exit(-1);
            }

        } else if (*prev_pipe_fd != -1) {
            input_fd = *prev_pipe_fd; // Use previous pipe's read end
        }

        if (input_fd != STDIN_FILENO) {
            if (dup2(input_fd, STDIN_FILENO) < 0) {
                perror("Error dup2 input file descriptor");
                exit(-1);
            } // Redirect stdin to fd
            close(input_fd);
        }

        // Handle output redirection
        if (filev[1]) {
            if (iter_command == num_commands - 1) {
                output_fd = open(filev[1], O_WRONLY | O_CREAT | O_TRUNC, 0660);
                if (output_fd == -1) {
                    perror("Error creating output file");
                    exit(-1);
                }
            } else {
                errno = EINVAL;
                perror("Commands between pipes cannot have redirections");
                exit(-1);
            }
        } else if (num_commands > 1 && iter_command < num_commands - 1) {
            output_fd = fd[1];
        }

        if (output_fd != STDOUT_FILENO) {
            if (dup2(output_fd, STDOUT_FILENO) < 0) {
                perror("Error dup2 output file descriptor");
                exit(-1);
            }
            close(output_fd);
        }

        // Handle error redirection
        if (filev[2]) {
            int error_fd = open(filev[2], O_RDWR | O_CREAT | O_TRUNC, 0660);
            if (error_fd == -1) {
                perror("Error creating error file");
                exit(-1);
            }
            if (dup2(error_fd, STDERR_FILENO) < 0) {
                perror("Error dup2 error file descriptor");
                exit(-1);
            }
            close(error_fd);
        }

        // Close unnecessary pipes
        if (num_commands > 1 && iter_command != num_commands - 1) {
            close(fd[0]);
        }

        // execute the command
        execvp(argvv[0], argvv);
        perror("Command execution failed");
        exit(-1);

    } else {
        // --PARENT PROCESS--
        if (*prev_pipe_fd != -1) {
            close(*prev_pipe_fd); // Close previous read end
        }
        if (num_commands > 1 && iter_command < num_commands - 1) {
            close(fd[1]);          // Close write end
            *prev_pipe_fd = fd[0]; // Pass read end to next process
        }

        if (!background) {
            int status;
            waitpid(pid, &status,
                    0); // Wait for child process if not background
        } else {
            printf("%d",pid); // Background (&) activated, print the child pid
        }
        // End parent
    }
}

void remove_quotes_from_args(char *argv[]) {
    for (int i = 1; argv[i] != NULL; i++) {
        int len = strlen(argv[i]);
        if (len > 0 && argv[i][0] == '"') {
            // Remove leading quote by shifting the string to the left.
            memmove(argv[i], argv[i] + 1, len);
            len--;
        }
        if (len > 0 && argv[i][len - 1] == '"') {
            argv[i][len - 1] = '\0';
        }
    }
}

int procesar_linea(char *linea) {
    /*
    This function processes the input command line and returns in global
    variables: argvv -- command an args as argv filev -- files for redirections.
    NULL value means no redirection. background -- 0 means foreground; 1
    background.
    */
    if (strspn(linea, " ") == strlen(linea)) {
        errno = ENOEXEC;
        perror("Empty line in the script");
        exit(-1);
    }
    char *comandos[max_commands];
    int num_comandos = tokenizar_linea(linea, "|", comandos,
                                       max_commands); // Split line commands

    // Check if background is indicated
    if (strchr(comandos[num_comandos - 1], '&')) {
        background = 1;
        char *pos = strchr(comandos[num_comandos - 1], '&');
        // remove character
        *pos = '\0';
    } else {
        background = 0;
    }

    // Finish processing

    int prev_pipe_fd = -1;
    for (int i = 0; i < num_comandos; i++) {
        tokenizar_linea(comandos[i], " \t\n", argvv, max_args);
        procesar_redirecciones(argvv);
        remove_quotes_from_args(argvv);
        /*
                |       My code     |
                V                   V
        */
        //print_commands(); //!!! Delete for submission
        execute_command(num_comandos, i, &prev_pipe_fd);
    }
    return num_comandos;
}

int read_script(char *script_path, char ***lines, int *line_count) {
    /*
    Read the script and parse the lines
    */
    int file = open(script_path, O_RDONLY);
    if (file == -1) {
        perror("Error opening input script");
        return -1;
    }

    char char_read;
    int line_index = 0;
    ssize_t bytes_read;

    // Read the whole script
    while ((bytes_read = read(file, &char_read, sizeof(char))) > 0) {
        // Check if it's the end of a line
        if (char_read == '\n') {
            if (!(*lines)[*line_count]) { // Ensure memory is allocated
                (*lines)[*line_count] = malloc(max_line * sizeof(char));
                if (!(*lines)[*line_count]) {
                    errno = ENOMEM;
                    perror("Memory allocation failed");
                    return -1;
                }
            }

            // End the line in the array
            (*lines)[*line_count][line_index] = '\0';
            (*line_count)++;
            line_index = 0;

            if (*line_count >= max_lines) { // Reallocate memory for more lines
                max_lines += 1024;          // Add another KB of memory
                char **temp = realloc(*lines, max_lines * sizeof(char *));
                if (!temp) {
                    errno = ENOMEM;
                    perror("Memory allocation failed");
                    return -1;
                }
                *lines = temp;

                // Allocate memory for new lines
                for (int i = *line_count; i < max_lines; i++) {
                    (*lines)[i] = malloc(max_line * sizeof(char));
                    if (!(*lines)[i]) {
                        errno = ENOMEM;
                        perror("Memory allocation failed for new lines");
                        return -1;
                    }
                }
            }
        } else {
            if (!(*lines)[*line_count]) { // Ensure memory is allocated
                (*lines)[*line_count] = malloc(max_line * sizeof(char));
                if (!(*lines)[*line_count]) {
                    errno = ENOMEM;
                    perror("Memory allocation failed");
                    return -1;
                }
            }

            (*lines)[*line_count][line_index] = char_read;
            line_index++;

            if (line_index >= max_line - 1) {
                perror("Exceeded maximum line length"); // S
                break;
            }
        }
    }

    // Check for errors during reading
    if (bytes_read == -1) {
        perror("Error reading the input script");
        return -1;
    }

    if (close(file) < 0) {
        perror("Error closing input file");
        return -1;
    }

    // Handle the last line if it didn’t end with '\n'
    if (line_index > 0) {
        (*lines)[*line_count][line_index] = '\0';
        (*line_count)++;
    }
    return 0; // Success
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        // Check arguments
        errno = EINVAL;
        perror("Usage: scripter <ruta_fichero>");
        return 1;
    }

    char **lines; // Array that store the lines of the script
    int line_count = 0;
    lines = (char **)malloc(max_lines * sizeof(char *));
    if (lines == NULL) {
        perror("Error allocating the memory");
        return 1;
    }

    for (int i = 0; i < max_lines; i++) {
        lines[i] = (char *)malloc(
            max_line * sizeof(char)); // Allocate memory for each line
        if (lines[i] == NULL) {
            errno = ENOMEM;
            perror("Memory allocation failed for a line");
            return 1;
        }
    }

    if (read_script(argv[1], &lines, &line_count) < 0) {
        for (int i = 0; i < max_lines; i++) {
            free(lines[i]);
        }
        free(lines);
        return 1;
    }

    // Check ##Script de SSOO
    if (strcmp(lines[0], "## Script de SSOO") != 0) {
        errno = ENOEXEC;
        perror("The script does not start with: ## Script de SSOO"); // S
        return 1;
    }

    // Process the lines
    for (int i = 1; i < line_count; i++) {
        // char example_line[] = "ls -l | pwd &";//"ls -l > pepe.txt | grep
        // scripter | wc -l !> redir_out.txt &";
        procesar_linea(lines[i]);
    }

    for (int i = 0; i < max_lines; i++) {
        free(lines[i]); // Free each line
    }
    free(lines);

    return 0;
}