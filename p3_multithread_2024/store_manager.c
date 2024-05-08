
//SSOO-P3 23/24

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stddef.h>
#include <sys/stat.h>
#include <pthread.h>
#include "queue.h"
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <errno.h>
#include <limits.h>

/* Constants _______________________________________________________________________________________________________ */

// #define DEBUG 1

#define MAX_THREADS 256
#define MAX_BUFFER 65536
#define LINE_SIZE 64

#define QUEUE_MUTEXNO 0
#define GETOPNUM_MUTEXNO 0
#define ENQUEUE_MUTEXNO 1
#define DEQUEUE_MUTEXNO 1
#define UPDATESTOCK_MUTEXNO 3

/* Global Variables_________________________________________________________________________________________________ */

char **operations;
long num_producers, num_consumers, buffer_size;

int op_count, op_num, elem_count,
  fd, 
  profits = 0,
  product_stock [5] = {0},
  purchase_rates [5] = { 2, 5, 15, 25, 100 },
  sale_rates [5] = { 3, 10, 20, 40, 125 };

pthread_mutex_t mutex[4];
pthread_cond_t non_full, non_empty;

queue *elem_queue;
struct element *elements;


/* Functions _______________________________________________________________________________________________________ */

int process_args(int argc, const char *argv[]);
int copy_file(const char *file_name);
void print_warnings();
int thread_manager();

int my_strtol(const char *string, long *number, char* strerr);
void print_result();

int store_element(struct element *elem, int id);
int process_element(struct element *elem);

void producer(void *id);
void consumer(void *id);

/* __________________________________________________________________________________________________________________ */





/**
 * It processes the arguments passed to the program
 * @param argc: number of arguments
 * @param argv: arguments array
 * @return -1 if error, 0 if successfull
*/

int process_args(int argc, const char * argv[]) {
  if (argc != 5) { 
    printf("Usage: ./store_manager <file name> <num producers> <num consumers> <buff size>\n");
    return -1;
  }

  char *strerr = NULL;
  if (my_strtol(argv[2], &num_producers, strerr) == -1) {
    fprintf(stderr, "ERROR: <num producers> %s\n", strerr);
    printf("Usage: ./store_manager <file name> <num producers> <num consumers> <buff size>\n");
    return -2;
  }
  if (my_strtol(argv[3], &num_consumers, strerr) == -1) {
    fprintf(stderr, "ERROR: <num consumers> %s\n", strerr);
    printf("Usage: ./store_manager <file name> <num producers> <num consumers> <buff size>\n");
    return -3;
  }
  if (my_strtol(argv[4], &buffer_size, strerr) == -1) {
    fprintf(stderr, "ERROR: <buff size> %s\n", strerr);
    printf("Usage: ./store_manager <file name> <num producers> <num consumers> <buff size>\n");
    return -4;
  }

  int err_count = 0;
  if (num_producers < 1) {
    fprintf(stderr, "ERROR: The number of producers must be greater than 0\n");
    err_count++;
  }
  if (num_consumers < 1) {
    fprintf(stderr, "ERROR: The number of consumers must be greater than 0\n");
    err_count++;
  }
  if (buffer_size < 1) {
    fprintf(stderr, "ERROR: The buffer size must be greater than 0\n");
    err_count++;
  }

  if (err_count > 0) {
    return -5;
  }

  return 0;
}






/***
 * It maps the file into memory
 * @param file_name: file name
 * @return -1 if error, 0 if success
*/
int copy_file(const char *file_name) {
  FILE* file = fopen(file_name, "r");
  if (file == NULL) { 
    perror("Error opening file");
    return -1;
  }

  if (fscanf(file, "%d", &op_num) != 1) { // Check the return value of fscanf
    perror("Error reading number of operations");
    fclose(file);
    return -1;
  }

  elements = malloc(op_num * sizeof(struct element));
  if (elements == NULL) { // Check the return value of malloc
    perror("Error allocating memory");
    fclose(file);
    return -1;
  }

  char tmp_op[9];
  int converted_num, invalid_operations = 0;
  for (int i = 0; i < op_num; i++) {
    converted_num = fscanf(file, "%d %s %d", &elements[i].product_id, tmp_op, &elements[i].units);
    
    if (converted_num == -1) {
      fprintf(stderr, "ERROR: There are less operations at the file than stated (%d)\n", op_num);
      free(elements);
      fclose(file);
      return -1;
    }
    else if (converted_num != 3) {
      invalid_operations++;
      continue;
    }
    
    if (strcmp(tmp_op, "PURCHASE") == 0) {
      elements[i].op = 0;
    } 
    else if (strcmp(tmp_op, "SALE") == 0) {
      elements[i].op = 1;
    }
    else {
      elements[i].op = -1;
      invalid_operations++;
    }
  }

  if (invalid_operations > 0) { // Print a warning message if there were any invalid operations
    fprintf(stderr, "WARNING: There were %d invalid operations\n", invalid_operations);
  }

  if (fclose(file) == -1) {
    perror("Error closing file");
    free(elements); // Free elements before returning -1
    return -1;  
  }

  return 0;
}




/**
 * Prints some warnings if the passed arguments are unnecessary big
*/
void print_warnings() {
  if (MAX_BUFFER < buffer_size) {
    printf("WARNING: The size of the buffer might be unnecessary big. It might hinder performance.\n");
  }
  if (MAX_THREADS < num_producers) {
    printf("WARNING: The number of producers might be unnecessary big. It might hinder performance.\n");
  }
  if (MAX_THREADS < num_consumers) {
    printf("WARNING: The number of consumers might be unnecessary big. It might hinder performance.\n");
  }
}





/**
 * @brief function that create, manage and destroy the threads
 * @return int: -1 if error, 0 if success 
 */
int thread_manager() {
  int *ids = (int *) malloc((num_consumers < num_producers ? num_producers : num_consumers) * sizeof(int));

  pthread_t *producers = (pthread_t *) malloc(num_producers * sizeof(pthread_t));
  if (producers == NULL) {
    perror("Error allocating memory for producer threads");
    return -1;
  }

  pthread_t *consumers = (pthread_t *) malloc(num_consumers * sizeof(pthread_t));
  if (consumers == NULL) {
    perror("Error allocating memory for consumer threads");
    free(producers);
    return -1;
  }


  for (int i = 0; i < 4; i++) {
    if (pthread_mutex_init(&mutex[i], NULL) != 0) {
      perror("Error initializing mutex");
      free(producers);
      free(consumers);
      return -1;
    }
  }

  if (pthread_cond_init(&non_full, NULL) != 0 || pthread_cond_init(&non_empty, NULL) != 0) {
    perror("Error initializing condition variable");
    for (int i = 0; i < 4; i++) {
      pthread_mutex_destroy(&mutex[i]);
    }
    free(producers);
    free(consumers);
    return -1;
  }

  for (int i = 0; i < num_producers; i++) {
    ids[i] = i;
    if (pthread_create(&producers[i], NULL, (void *) producer, (void *) &ids[i]) != 0) {
      perror("Error creating producer thread");
      for (int j = 0; j < i; j++) {
        pthread_cancel(producers[j]);
      }
      for (int i = 0; i < 4; i++) {
        pthread_mutex_destroy(&mutex[i]);
      }
      pthread_cond_destroy(&non_full);
      pthread_cond_destroy(&non_empty);
      free(producers);
      free(consumers);
      return -1;
    }
  }

  for (int i = 0; i < num_consumers; i++) {
    ids[i] = i;
    if (pthread_create(&consumers[i], NULL, (void *) consumer, (void *) &ids[i]) != 0) {
      perror("Error creating consumer thread");
      for (int j = 0; j < num_producers; j++) {
        pthread_cancel(producers[j]);
      }
      for (int j = 0; j < i; j++) {
        pthread_cancel(consumers[j]);
      }
      for (int i = 0; i < 4; i++) {
        pthread_mutex_destroy(&mutex[i]);
      }
      pthread_cond_destroy(&non_full);
      pthread_cond_destroy(&non_empty);
      free(producers);
      free(consumers);
      return -1;
    }
  }

  for (int i = 0; i < num_producers; i++) {
    pthread_join(producers[i], NULL);
  }

  for (int i = 0; i < num_consumers; i++) {
    pthread_join(consumers[i], NULL);
  }

  #ifdef DEBUG
  fprintf(stdout, "End of threads\n");
  #endif

  for (int i = 0; i < 4; i++) {
    pthread_mutex_destroy(&mutex[i]);
  }
  pthread_cond_destroy(&non_full);
  pthread_cond_destroy(&non_empty);

  free(producers);
  free(consumers);

  return 0;
}





/***
 * Conversion from string to long integer using strtol with some error handling
 * @param string: string to convert to long
 * @param number: pointer to store the result
 * @return Error -1 otherwise 0 
*/
int my_strtol(const char *string, long *number, char *strerr) {
    char *nptr, *endptr = NULL;
    nptr = (char *) string;
    errno = 0;
    *number = strtol(nptr, &endptr, 10);

    if (nptr && *endptr != 0) {
      strcpy(strerr, " is not an integer\n");
      return -1;
    }
    else if (errno == ERANGE && *number == LONG_MAX)
    {
      strcpy(strerr, " overflow\n");
      return -1;
    }
    else if (errno == ERANGE && *number == LONG_MIN)
    {
      strcpy(strerr, " underflow\n");
      return -1;
    }

    return 0;
}






/***
 * Prints the result
*/
void print_result() {
  printf("Total: %d euros\n", profits);
  printf("Stock:\n");
  printf("  Product 1: %d\n", product_stock[0]);
  printf("  Product 2: %d\n", product_stock[1]);
  printf("  Product 3: %d\n", product_stock[2]);
  printf("  Product 4: %d\n", product_stock[3]);
  printf("  Product 5: %d\n", product_stock[4]);
}





/***
 * It stores the information scrapped from the file inside an struct element and pushes it into the queue
 * @param elem: element to store
*/
int store_element(struct element *elem, int id) {
  #ifdef DEBUG
  int was_full = 0;
  #endif

  // !! Critical section <begin> !! -> thread pushes the element into the queue
  pthread_mutex_lock(&mutex[ENQUEUE_MUTEXNO]);
  while (queue_full(elem_queue) == 1) {
    #ifdef DEBUG
    was_full = 1;
    printf("\tproducer %d blocked\n", id);
    #endif

    pthread_cond_wait(&non_full, &mutex[ENQUEUE_MUTEXNO]);
  }

  #ifdef DEBUG
  if (was_full == 1)
    printf("\tproducer %d unblocked\n", id);
  #endif
  

  queue_put(elem_queue, elem);

  pthread_cond_signal(&non_empty);
  pthread_mutex_unlock(&mutex[ENQUEUE_MUTEXNO]);
  // !! Critical section <end> !!
  
  return 0;
};





/***
 * It processes the information inside an struct element and updats the product stock and profits
 * @param elem: element to process
*/
int process_element(struct element *elem) {
  // !! Critical section <begin> !! -> thread processes the element
  pthread_mutex_lock(&mutex[UPDATESTOCK_MUTEXNO]);

  if (elem->op == 0) { // Purchase
    product_stock[elem->product_id - 1] += elem->units;
    profits -= purchase_rates[elem->product_id - 1] * elem->units;
  } 
  else if (elem->op == 1) { // Sale
    product_stock[elem->product_id - 1] -= elem->units;
    profits += sale_rates[elem->product_id - 1] * elem->units;
  }
  pthread_mutex_unlock(&mutex[UPDATESTOCK_MUTEXNO]);
  // !! Critical section <end> !!
  
  return 0;
};




/***
 * Producer function for the producer thread
 * @return -1 if error, 0 if success
*/
void producer(void *id) {
  #ifdef DEBUG
  fprintf(stdout, "Start producer!\n");
  #endif

  struct element *elem;
  int op_index;
  while (op_count < op_num) {    
    #ifdef DEBUG
    fprintf(stdout, "I'm a producer!\n");
    #endif

    pthread_mutex_lock(&mutex[GETOPNUM_MUTEXNO]);
    if (op_count >= op_num) {
      pthread_mutex_unlock(&mutex[GETOPNUM_MUTEXNO]);
      break;
    }

    op_index = op_count++;    

    pthread_mutex_unlock(&mutex[GETOPNUM_MUTEXNO]);
    
    #ifdef DEBUG
    fprintf(stdout, "[producer %d begin]\n", op_index);
    #endif

    elem = &elements[op_index];

    store_element(elem, *(int*)id);

    #ifdef DEBUG
    fprintf(stdout, "[producer %d - elem %d end]\n", *(int *) id, op_index);
    #endif

    // pthread_mutex_lock(&mutex[ENQUEUE_MUTEXNO]);
    // while (queue_full(elem_queue)) {
    //   pthread_cond_wait(&non_full, &mutex[ENQUEUE_MUTEXNO]);
    // }

    // queue_put(elem_queue, &elem);

    // pthread_mutex_unlock(&mutex[ENQUEUE_MUTEXNO]);
    // pthread_cond_signal(&non_empty);

    #ifdef DEBUG
    fprintf(stdout, "[producer %d end]\n", op_index);
    #endif
  }

  #ifdef DEBUG
  fprintf(stdout, "\t\t\tEnd producer %d\n", *(int *) id);
  #endif

  pthread_exit(0);
}

/***
 * Consumer function for the consumer thread
 * @return -1 if error, 0 if success
*/
void consumer(void *id) {
  #ifdef DEBUG
  int was_empty = 0;
  fprintf(stdout, "Start consumer %d!\n", *(int *) id);
  #endif

  struct element *elem;
  int elem_index;
  while (elem_count < op_num) {
    // !! Critical section <begin> !! -> thread pops one element from the queue
    pthread_mutex_lock(&mutex[DEQUEUE_MUTEXNO]);
    elem_index = elem_count++; 
    if (elem_index >= op_num) {
      pthread_mutex_unlock(&mutex[DEQUEUE_MUTEXNO]);
      break;
    }

    #ifdef DEBUG
    fprintf(stdout, "[consumer %d - elem %d begin]\n", *(int *) id, elem_index);
    #endif


    while (queue_empty(elem_queue)) {      
      #ifdef DEBUG
      was_empty = 1;
      fprintf(stdout, "\tconsumer %d blocked\n", *(int *) id);
      #endif

      pthread_cond_wait(&non_empty, &mutex[DEQUEUE_MUTEXNO]);
    }

    #ifdef DEBUG
    if (was_empty == 1)
      fprintf(stdout, "\tconsumer %d unblocked\n", *(int *) id);
    #endif

    elem = queue_get(elem_queue);

    pthread_cond_signal(&non_full);
    pthread_mutex_unlock(&mutex[DEQUEUE_MUTEXNO]);

    process_element(elem);

    #ifdef DEBUG
    fprintf(stdout, "[consumer %d end]\n", elem_count);
    #endif
  }

  #ifdef DEBUG
  fprintf(stdout, "\t\t\tEnd consumer %d\n", *(int *) id);
  #endif

  pthread_exit(0);
}



/**
 * Main function _____________________________________________________________________________________________________
*/
int main (int argc, const char * argv[])
{
  // Checks whether arguments are correct or not
  if (process_args(argc, argv) < 0)
    return -1;

  // Copy the contents of the file into memory
  if (copy_file(argv[1]) < 0) {
    fprintf(stderr, "Error copying file\n");
    return -1;
  }

  #ifdef DEBUG
  fprintf(stderr, "Number of operations: %d\n", op_num);
  #endif

  // Warn user of big variables passed through the arguments array
  print_warnings();

  // Initialize the queue
  elem_queue = queue_init(buffer_size); 

  if (thread_manager() == -1) {
    fprintf(stderr, "Error in thread manager\n");
    free(elements); // Free the memory allocated for the elements array
    queue_destroy(elem_queue); // Destroy the queue
    return -1;
  }

  // Output
  print_result();

  free(elements); // Free the memory allocated for the elements array
  queue_destroy(elem_queue); // Destroy the queue

  return 0;
}
