#ifndef HEADER_FILE
#define HEADER_FILE

struct element {
  // represents the order of creation within the belt.
  int num_edition;
  // represents  the  ID  of  the  belt  in  which  the  object  is  created  (the 
  // factory_manager indicates it to the process by parameter). 
  int id_belt;
  // it will be 0 if the inserted element is not the last one, and it will be 1 if the 
  // object is the last one to be created by the producer.
  int last;
};

int queue_init (int size);
int queue_destroy (void);
int queue_put (struct element* elem);
struct element * queue_get(void);
int queue_empty (void);
int queue_full(void);

#endif
