/*
 *
 * process_manager.c
 *
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <unistd.h>
 #include <fcntl.h>
 #include <stddef.h>
 #include <pthread.h>
 #include "queue.h"
 #include <semaphore.h>
 
 #define NUM_THREADS 2
 
 
 extern sem_t *Finish_process_sem;
 extern sem_t *Waiting_process_sem;
 extern sem_t *Execute_process_sem;
 
 pthread_mutex_t q_mutex;
 pthread_cond_t full_cv;
 pthread_cond_t empty_cv;
 
 // Struct to hold thread arguments
 typedef struct {
	 int id;
	 int items_to_produce;
 } ConsProdArgs;
 
 typedef struct {
	 int id;
	 int belt_size;
	 int items_to_produce;
	 int sem_idx;
	 int process_numb;
 } ProcessArgs;
 
 //Thread functionS
 void *Producer(void *arg) {
	 // Take the arguments from the struct
	 ConsProdArgs *args = (ConsProdArgs *)arg;
	 int id = args->id;
	 int prod_elem = args->items_to_produce;
	 free(args); // Free the allocated memory
 
	 for (int i = 0; i < prod_elem; i++) {
		 // Enter critical part
		 pthread_mutex_lock(&q_mutex);
		 while (queue_full()){
			 pthread_cond_wait(&full_cv, &q_mutex);
		 }
		 // Allocate the element struct
		 struct element *elem = malloc(sizeof(struct element));
		 // The elements are freed in the consumer
		 if (elem == NULL) {
			 fprintf(stderr, "Failed to allocate element");
			 exit(EXIT_FAILURE);
		 }
 
		 // Flag to check if the queue was empty, to free the consumer after
		 // the execution of queue_put()
		 int was_empty = queue_empty();
 
		 // Pass the values to the struct
		 elem->num_edition = i;
		 elem->id_belt = id;
		 elem->last = 1;
		 queue_put(elem);
 
		 if (was_empty)
			 // Free the consumer
			 pthread_cond_signal(&empty_cv);
		 // Free the mutex
		 pthread_mutex_unlock(&q_mutex);
	 }
	 pthread_exit(0);
 }
 
 
 void *Consumer(void *arg) {
	 // Take the arguments from the struct
	 ConsProdArgs *args = (ConsProdArgs *)arg;
	 //int id = args->id; Unused var.
	 int cons_elem = args->items_to_produce;
	 free(args); // Free the allocated memory
 
	 for (int i = 0; i < cons_elem; i++) {
		 // Enter critical part
		 pthread_mutex_lock(&q_mutex);
		 while (queue_empty()){
			 pthread_cond_wait(&empty_cv, &q_mutex);
		 }
 
		 // Flag to check if the queue was full, to free the producer after
		 // the execution of queue_get()
		 int was_full = queue_full();
 
		 struct element *elem = queue_get();
		 // Free the allocated element
		 free(elem);
 
		 if (was_full)
			 // Free the consumer
			 pthread_cond_signal(&full_cv);
		 // Free the mutex
		 pthread_mutex_unlock(&q_mutex);
	 }
	 pthread_exit(0);
 }
 
 
 int process_manager (void* arg){
	 // Pass the arguments from the input struct to variables
	 ProcessArgs *data = (ProcessArgs*) arg;
	 int id = data->id; // Process id
	 int belt_size = data->belt_size; // Size of the queue
	 int items_to_produce = data->items_to_produce;
	 int sem_idx = data->sem_idx; // Index of the thread in the threads arrays for the semaphore management
	 int process_numb = data->process_numb; // Total number of procesess
 
	 // Wait to all the threads to be created
	 sem_wait(&Waiting_process_sem[sem_idx]);
 
	 if(belt_size < 0 || items_to_produce < 0){
		 fprintf(stderr, "[ERROR][process_manager] Arguments not valid.\n");
		 exit(-1);
	 }
 
	 printf("[OK][process_manager] Process_manager with id %d waiting to produce %d elements.\n", id, items_to_produce);
 
	 
	 // Free the next semaphore for printing that it's waiting
	 if (sem_idx + 1 < process_numb){
		 sem_post(&Waiting_process_sem[sem_idx + 1]);
	 } else {
		 // If it's the last thread, the first thread can start executing
		 sem_post(&Execute_process_sem[0]);
	 }
 
	 // Wait for execution
	 sem_wait(&Execute_process_sem[sem_idx]);
	 pthread_t threads[NUM_THREADS];
 
	 // Initialize the mutex and conditional variables
	 pthread_mutex_init(&q_mutex, NULL);
	 pthread_cond_init(&full_cv,  NULL);
	 pthread_cond_init(&empty_cv, NULL);
 
	 // Start the queue
	 if(queue_init(belt_size) < 0){
		 fprintf(stderr, "[ERROR][queue] There was an error while using queue with id: %d.\n", id);
		 sem_post(&Finish_process_sem[sem_idx]);
		 return -1;
	 };
 
	 printf("[OK][process_manager] Belt with id %d has been created with a maximum of %d elements.\n", id, belt_size);
 
	 // Struct for passing all the arguments to consumer/producer in the thread
	 // Allocate memory for the integer argument
	 // Twice allocation so the free in each thread can be done
	 ConsProdArgs *args1 = malloc(sizeof(ConsProdArgs));
	 if(args1 == NULL){
		 fprintf(stderr, "[ERROR][process_manager] There was an error executing process_manager with id %d.\n", id);
		 sem_post(&Finish_process_sem[sem_idx]);
		 return -1;
	 }
	 args1->id = id;
	 args1->items_to_produce = items_to_produce;
	 if(pthread_create(&threads[0], NULL, Producer, args1) < 0){
		 fprintf(stderr, "[ERROR][process_manager] There was an error executing process_manager with id %d.\n", id);
		 sem_post(&Finish_process_sem[sem_idx]);
		 return -1;
	 }
 
	 ConsProdArgs *args2 = malloc(sizeof(ConsProdArgs));
	 if(args2 == NULL){
		 fprintf(stderr, "[ERROR][process_manager] There was an error executing process_manager with id %d.\n", id);
		 sem_post(&Finish_process_sem[sem_idx]);
		 return -1;
	 }
	 args2->id = id;
	 args2->items_to_produce = items_to_produce;
	 if(pthread_create(&threads[1], NULL, Consumer, args2) < 0){
		 fprintf(stderr, "[ERROR][process_manager] There was an error executing process_manager with id %d.\n", id);
		 sem_post(&Finish_process_sem[sem_idx]);
		 return -1;
	 } 
 
	 // Wait for the threads to end
	 for (int i = 0; i < NUM_THREADS; i++) {	
		 pthread_join(threads[i], NULL);
	 }
	 
	 // Destroy the queue
	 queue_destroy();
	 
		printf("[OK][process_manager] Process_manager with id %d has produced %d elements.\n", id, items_to_produce);
	 // Free the finish semaphore, so factory_manager can print that it ended
	 sem_post(&Finish_process_sem[sem_idx]);
	 // Free the argument struct
	 free(data);
	 // Exit thread
	 //pthread_exit(0);
		return 0;
 }