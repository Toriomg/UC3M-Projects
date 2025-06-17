/*
 *
 * factory_manager.c
 *
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <unistd.h>
 #include <fcntl.h>
 #include <stddef.h>
 #include <semaphore.h>
 #include <sys/stat.h>
 #include <pthread.h>
 #define BUFFER_SIZE 1024
 
 sem_t *Finish_process_sem;
 sem_t *Execute_process_sem;
 sem_t *Waiting_process_sem;
 
 typedef struct {
	 int id;
	 int belt_size;
	 int items_to_produce;
	 int sem_idx;
	 int process_numb;
 } ProcessArgs;
 
 void* process_manager(void* arg);
 
 void print_processes(int **proc_managers, int max_belts){
	/*Just for debugging*/
	 for (int i = 0; i < max_belts; i++){
		 for (int j = 0; j < 3; j++){
			 printf("%d\t", proc_managers[i][j]);
		 }
		 printf("\n");
	 }
 }
 
 int* tokenize_file(const char* filename, size_t *argc) {
	 int fd = open(filename, O_RDONLY);
	 if (fd == -1) {
		 fprintf(stderr, "[ERROR][factory_manager] Invalid file.\n");
		 return NULL;
	 }
 
	 // Initialize buffer and read file incrementally
	 size_t buffer_size = 128; // Initial buffer size
	 size_t total_read = 0;
	 char *buffer = malloc(buffer_size);
	 if (buffer == NULL) {
		 fprintf(stderr, "[ERROR][factory_manager] Invalid file.\n");
		 return NULL;
	 }
 
	 while (1) {
		 // Read into remaining space in buffer
		 ssize_t n = read(fd, buffer + total_read, buffer_size - total_read);
		 if (n == -1) {
			 fprintf(stderr, "[ERROR][factory_manager] Invalid file.\n");
			 return NULL;
		 }
		 if (n == 0) break; // Reached EOF
 
		 total_read += n;
 
		 // Double buffer size if full
		 if (total_read == buffer_size) {
			 buffer_size *= 2;
			 char *temp = realloc(buffer, buffer_size);
			 if (temp == NULL) {
				 fprintf(stderr, "[ERROR][factory_manager] Invalid file.\n");
				 return NULL;
			 }
			 buffer = temp;
		 }
	 }
 
	 // Ensure space for null terminator
	 if (total_read >= buffer_size) {
		 buffer_size = total_read + 1;
		 char *temp = realloc(buffer, buffer_size);
		 if (temp == NULL) {
			 fprintf(stderr, "[ERROR][factory_manager] Invalid file.\n");
			 return NULL;
		 }
		 buffer = temp;
	 }
	 buffer[total_read] = '\0';
 
	 // Truncate at newline if present
	 char *newline_ptr = strchr(buffer, '\n');
	 if (newline_ptr != NULL) {
		 *newline_ptr = '\0';
	 }
 
	 close(fd);
 
	 // Tokenize the buffer into argvv array
	 int *argvv = NULL;
	 size_t argvv_capacity = 16;
 
	 argvv = malloc(argvv_capacity * sizeof(int));
	 if (argvv == NULL) {
		 fprintf(stderr, "[ERROR][factory_manager] Invalid file.\n");
		 return NULL;
	 }
 
	 char *token;
	 char *saveptr = NULL;
	 token = strtok_r(buffer, " ", &saveptr);
 
	 while (token != NULL) {
		 char *endptr;
		 long value = strtol(token, &endptr, 10);
	 
		 // Check if token is a valid integer
		 if (*endptr != '\0') {
			 fprintf(stderr, "[ERROR][factory_manager] Invalid file.\n");  // Not a valid number
			 return NULL;
		 }
	 
		 // Reallocate the array
		 if (*argc >= argvv_capacity) {
			 argvv_capacity *= 2;
			 int *temp = realloc(argvv, argvv_capacity * sizeof(int));
			 if (temp == NULL) {
				 fprintf(stderr, "[ERROR][factory_manager] Invalid file.\n");
				 return NULL;
			 }
			 argvv = temp;
		 }
	 
		 argvv[(*argc)++] = (int)value;
	 
		 token = strtok_r(NULL, " ", &saveptr);
	 }
 
	 
	 // Cleanup
	 free(buffer);
 
	 return argvv;
 }
 
 int parse_file(const char* file_path, int*** proc_managers, int* max_belts, int* process_numb){
	 
	 size_t argc = 0;
	 int *argvv;
	 if((argvv = tokenize_file(file_path, &argc)) == NULL){
		 return(-1);
	 }
 
		 
	 *max_belts = argvv[0];
	  // Check the constraints on the arguments of the file
	 if (*max_belts < 1 || (argc - 1) % 3 != 0 || (argc - 1) /3 > argvv[0]){
		 fprintf(stderr, "[ERROR][factory_manager] Invalid file.\n");
		 return(-1);
	 }
 
	 
	 // Allocate the matrix for the process managers
	 *proc_managers = (int**) malloc(*max_belts * sizeof(int*));
	 if (!*proc_managers){
		 fprintf(stderr, "[ERROR][factory_manager] Invalid file.\n");
		 return(-1);
	 }
 
	 
	 for (int i = 0; i < *max_belts; i++){
		 (*proc_managers)[i] = malloc(3 * sizeof(int));
		 if (!(*proc_managers)[i]){
			 fprintf(stderr, "[ERROR][factory_manager] Invalid file.\n");
			 return(-1);
		 }
	 }
 
	 //Populate the matrix with the values from argvv
	 for (int i = 0; i < *max_belts; i++) {
		 for (int j = 0; j < 3; j++) {
 
			 int number = 1 + i * 3 + j;
			 if (number < argc) {
				 // Check repeated ID's
				 if(j == 0){
					 for (int k = 0; k < i; k++){
						 if ((*proc_managers)[k][0] == argvv[number]) {
							 fprintf(stderr, "[ERROR][factory_manager] Invalid file.\n");
							 return(-1);
						 }
					 }
				 }
				 (*proc_managers)[i][j] = argvv[number];
 
			 } else {
				 (*proc_managers)[i][j] = 0; // Empty row
 
			 }
		 }
	 }
	 *process_numb = (argc -1)/3;
	 free(argvv);
	 return 0;
 }
 
 void create_sync_struct(int **proc_managers, int max_belts, int process_numb){
	 
	 // Start and allocate semaphores	
	 Waiting_process_sem = malloc(process_numb * sizeof(sem_t));
	 for (int i = 0; i < process_numb; i++) {
		 sem_init(&Waiting_process_sem[i], 0, (i == 0) ? 1 : 0);
	 }
 
	 Execute_process_sem = malloc(process_numb * sizeof(sem_t));
	 for (int i = 0; i < process_numb; i++) {
		 sem_init(&Execute_process_sem[i], 0, 0);
	 }
 
	 Finish_process_sem = malloc(process_numb * sizeof(sem_t));
	 for (int i = 0; i < process_numb; i++) {
		 sem_init(&Finish_process_sem[i], 0, 0);
	 }
 
	 pthread_t threads[process_numb]; // Declare threads
 
	 sem_wait(&Waiting_process_sem[0]);
	 for (int i = 0; i < process_numb; ++i) {
		 // Struct for passing all the arguments to process_manager in the thread
		 // Allocate struct
		 ProcessArgs *pArgs = malloc(sizeof(ProcessArgs));
		 if (!pArgs) {
			 perror("Error al asignar memoria para ProcessArgs");
			 exit(EXIT_FAILURE);
		 }
		 // Pass values to struct
		 pArgs->id = proc_managers[i][0];
		 pArgs->belt_size = proc_managers[i][1];
		 pArgs->items_to_produce = proc_managers[i][2];
		 pArgs->sem_idx = i;
		 pArgs->process_numb = process_numb;
 
		 // Thread creation
		 if (pthread_create(&threads[i], NULL, process_manager, pArgs) != 0) {
			 fprintf(stderr, "[ERROR][factory_manager] Failed to create process_manager with id %d.\n", proc_managers[i][0]);
		 } else {
			 printf("[OK][factory_manager] Process_manager with id %d has been created.\n", proc_managers[i][0]);
		 }
	 }
	 // Signal the semaphore, so the process can print: 'waiting'
	 sem_post(&Waiting_process_sem[0]);
 
	 void *status; // Stores the result of the thread
	 // Join all the threads to the factory manager
	 for (int i = 0; i < process_numb; ++i) {
		 //Wait for a process_manager finishing, so print: 'finished' can be done.
		 sem_wait(&Finish_process_sem[i]);
		 pthread_join(threads[i], &status);
 
		 if (status != 0){
			 fprintf(stderr, "[ERROR][factory_manager] Process_manager with id %d has finished with errors.\n", proc_managers[i][0]);
		 } else {
			 printf("[OK][factory_manager] Process_manager with id %d has finished.\n", proc_managers[i][0]);
		 }
		 if (i+1 < process_numb) // If it isn't last process, execute the next process_manager
			 sem_post(&Execute_process_sem[i+1]);
	 }
 
	 
	 // Close semaphores
	 for (int i = 0; i < process_numb; i++) {
		 sem_destroy(&Waiting_process_sem[i]);
		 sem_destroy(&Execute_process_sem[i]);
		 sem_destroy(&Finish_process_sem[i]);
	 }
	 // Free the semaphore arrays
	 free(Finish_process_sem);
	 free(Execute_process_sem);
	 free(Waiting_process_sem);
 }
 
 int main (int argc, const char * argv[] ){
	 if (argc != 2){
		 fprintf(stderr, "[ERROR][factory_manager] Invalid file.\n");
		 return(-1);
	 };
 
	 // Create variables
	 int **proc_managers; 	// Store the data of the file
	 int max_belts;			// Max numb. of belts
	 int process_numb;		// The process number
	 // Parse the file, so the value of the file are passed into proc_managers
	 if(parse_file(argv[1], &proc_managers, &max_belts, &process_numb) < 0) {
		 return(-1);
	 }
 
	 create_sync_struct(proc_managers, max_belts, process_numb);
	 
	 // Free the matrix
	 for (int i = 0; i < max_belts; i++){
		 free(proc_managers[i]);
	 }
	 free(proc_managers);
	 // Print end of factory_manager
	 printf("[OK][factory_manager] Finishing.\n");
	 return 0;
 }