#include <stdio.h>    // i/o lib
#include <unistd.h>   // linux lib
#include <stdlib.h>   // dynamic mem lib
#include <sys/types.h>// ??
#include <sys/stat.h> // ??
#include <fcntl.h>    // ??
#include <string.h>   // string lib

#define EXPECTED_ARGC 4
#define MAX_STUDENTS 100

struct alumno{
	char nombre[50];
	int nota;
	int convocatoria;
};

int eq_alumno(struct alumno s1, struct alumno s2){
	/*
	Check if two students has the same name
	*/
	return strcmp(s1.nombre, s2.nombre) == 0;
}

int check_duplicated_alumno(struct alumno students[], int n){
	/*
	Checks if in an struct array there is a duplicate
	*/
	for (int i = 0; i < n; i++)
	{
		for (int j = i + 1; j < n; j++)
		{
			if (eq_alumno(students[i], students[j])) return 1;
		}
		
	}
	return 0;
}

void bubble_sort(struct alumno students[], int n) {
    /* 
    Bubble sort in ascending order to sort students
    */
    for (int i = 0; i < n-1; i++) {
        for (int j = 0; j < n-i-1; j++) {

            if (students[j].nota > students[j+1].nota){ // else if the convocatorias 
                // Changes the values in the array
                struct alumno temp = students[j];
                students[j] = students[j+1];
                students[j+1] = temp;
            }
        }
    }
}


void make_csv(const char csv_file_name[], struct alumno students[], int num_students){
	/*
	Creates a CSV file and populates it with the values requested by the statement
	*/
	int marks_count[5] = {0}; // Initialize all the counts in 0
	char char_mark[5] = {'M', 'S', 'N', 'A', 'F'};
	/*Indeces of marks_count: 0 -> M, 1 -> S, 2 -> N, 3 -> A, 4 -> F*/
	for (int i = 0; i < num_students; i++) // Counts every type of mark
	{
		switch (students[i].nota){
		case 10:
			marks_count[0]++; // M
			break;
		case 9:
			marks_count[1]++; // S
			break;
		case 8: case 7:
			marks_count[2]++; // N
			break;
		case 6: case 5: 
			marks_count[3]++; // A
			break;
		case 4: case 3: case 2: case 1: case 0:
			marks_count[4]++; // F
			break;
		default:
			printf("Error: Invalid mark.\n");
			printf("The mark: %d of the student: %s is not in the range of [0,10]\n", students[i].nota, students[i].nombre);
			exit(1);
		}
	}
	int csv = open(csv_file_name, O_CREAT | O_TRUNC | O_WRONLY, 0644);
	if (csv == -1){
		printf("Error creating file %s \n", csv_file_name);
		exit(1);
	}

	// Populates the .csv
	for (int i = 0; i < 5; i++){
		// The string is in the style, e.g: M;54;23.50%""
		char line_buffer[40]; // 40*8 bytes should be mem. enough to store a line, however as the size is not exact idk if it could make segmentation errors or fit in the file
		// Format the line of the .csv
		snprintf(line_buffer, sizeof(line_buffer), "%c;%d;%.2f%%\n", char_mark[i], marks_count[i], (marks_count[i] * 100.0) / num_students);

		size_t line_written = write(csv, line_buffer, strlen(line_buffer));
		if (line_written == (size_t)-1) {
            printf("Error writing file %s \n", csv_file_name); // Check errors
            close(csv); // Close the .csv file
            exit (1);
        } 
	}

	// Closes the file
	if (close(csv) < 0){ 
		printf("Error closing file %s\n", csv_file_name);
		exit(1);
	}

}

void read_file(const char arg_file[], struct alumno students[], int *num_students) {
	/*
	Opens the file and rad the data into the students array struct
	*/
    int file = open(arg_file, O_RDONLY); // Open the file
    if (file == -1) {
        printf("Error reading the file %s\n", arg_file);
        exit(1);
    }
    
    while (*num_students < MAX_STUDENTS) { // To check whether the number of students reach it's maximun
		ssize_t student_data = read(file, &students[*num_students], sizeof(struct alumno));
		if (student_data == -1) { // Problem reading the data
			printf("Error reading file\n");
			exit(1);
		}
		if (student_data == 0) {
			break; // End of file reached
		}
		printf("%d. %s, %d, %d\n", *num_students, students[*num_students].nombre, students[*num_students].nota, students[*num_students].convocatoria);
		if (student_data < (ssize_t)sizeof(struct alumno)) {  // Partial read (shouldn't happen normally)
            printf("Warning: Partial struct read. Data might be corrupted.\n");
            break;
        }
        (*num_students)++;
    }
    
    if (close(file)<0){ // Closes the file
		printf("close error\n");
		exit(1);
	}
}

void write_students(const char *arg_file, struct alumno *students, int num_students){
	/*
	Write the whole array of students in the output file
	*/
	int file = open(arg_file, O_WRONLY | O_CREAT | O_TRUNC, 0644); // Open the file
    if (file == -1) {
        printf("Error reading the file %s\n", arg_file);
        exit(1);
    }
    
    for (int i = 0; i < num_students; i++){ // Write each student in output file
        if (write(file, &students[i], sizeof(struct alumno)) == -1) {
            printf("Error writing file %s \n", arg_file); // Check errors
            close(file); // Close the file
            exit (1);
        }
    }
    
    if (close(file) < 0){ // Close the file, if error exit
		printf("close error\n");
		exit(1);
	}
}

int main(int argc, char *argv[]){

	if (argc != EXPECTED_ARGC) {
		// Check if the number of arguments is incorrect
		printf("Error: Invalid number of arguments.\n");
		printf("Program running with %d arguments, the program must tun with %d arguments.\n", argc - 1, EXPECTED_ARGC - 1);
        return -1;
    }
    
    struct alumno students[MAX_STUDENTS];
	int num_students = 0;

	read_file(argv[1], students, 	&num_students);
	read_file(argv[2], students, &num_students);
	if (num_students >= MAX_STUDENTS){ 
		// If any of the while loops of above is closed 
		// by reaching the students number limit an error is raised
		printf("Maximun number of students reached\n");
		return -1;
	}
	if (check_duplicated_alumno(students, num_students)){
		printf("There are duplicated students\n");
		return -1;
	}

    bubble_sort(students, num_students); // Sort the students in ascending order
	
	///*
	printf("-Debugging-\n");
	for (int i = 0; i < num_students; i++)
	{
		printf("%d. %s, %d, %d\n", i, students[i].nombre, students[i].nota, students[i].convocatoria);
	}
	//*/


	write_students(argv[3], students, num_students); // Writes the sorted students into the 3rd file

	make_csv("estadisticas.csv", students, num_students); // Creates the CSV and populates it with the statistics
	
	return 0;
}
