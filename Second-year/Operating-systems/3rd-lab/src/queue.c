#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stddef.h>
#include "queue.h"

struct node {
    struct element *elem;
    struct node *next;
};

static struct node *head = NULL;
static struct node *tail = NULL;
static int queue_size = 0;
static int count = 0;

extern pthread_mutex_t q_mutex;
extern pthread_cond_t full_cv;
extern pthread_cond_t empty_cv;



//To create a queue
int queue_init(int size){
	/*
	function that creates the queue and reserves the size 
	specified as a parameter.
	*/
	if (size <= 0){
		// The print with id cannot be performed as queue_init neither the queue does not contain any id info.
		//printf("[ERROR][queue] There was an error while using queue with id: %d.\n", x->id_belt);
		return -1;
	}
	queue_size = size;
	head = tail = NULL;
	count = 0;
	return 0;
}


// To Enqueue an element
int queue_put(struct element* x) {
	/*
	function  that  inserts  elements  into  the  queue  if 
	space is available. If there is no space available, it must wait until the insertion can be 
	performed.
	*/
	struct node *new_node = malloc(sizeof(struct node));
	if (new_node == NULL){
		fprintf(stderr, "[ERROR][queue] There was an error while using queue with id: %d.\n", x->id_belt);
		return -1;
	}

	new_node->elem = x;
	new_node->next = NULL;

	if (tail != NULL){
		tail->elem->last = 0;
	}
	x->last = 1;

	if (head == NULL) {
		head = tail = new_node;
	}else{
		tail->next = new_node;
		tail = new_node;
	}

	count++;
	printf("[OK][queue] Introduced element with id %d in belt %d.\n", x->num_edition, x->id_belt);
	
	return 0;
}


// To Dequeue an element.
struct element* queue_get(void) {
	/*
	function that extracts elements from the queue if there 
	are elements in it. If the queue is empty, it must wait until an element is available.  
	*/
	struct node *temp = head;
	struct element *elem = temp->elem;

	head = head->next;
	if (head == NULL) {
		tail = NULL;
	}

	free(temp);
	count--;
	printf("[OK][queue] Obtained element with id %d in belt %d.\n", elem->num_edition, elem->id_belt);
	
	return elem;
}


//To check queue state
int queue_empty(void){
	/*
	function  that  queries  the  queue  status  and  determines  if  it  is 
	empty (return 1) or not (return 0).
	*/
	return count == 0;
}

int queue_full(void){
	/*
	function that queries the queue status and determines if the queue 
	is full (return 1) or still has available items (return 0).
	*/
	return count == queue_size;
}

//To destroy the queue and free the resources
int queue_destroy(void){
	/*
	function  that  destroys  the  queue  and  frees  all  allocated 
	resources.
	*/
	struct node *current = head;
	while (current != NULL) {
		struct node *next = current->next;
		free(current);
		current = next;
	}
	head = tail = NULL;
	count = queue_size = 0;
	return 0;
}