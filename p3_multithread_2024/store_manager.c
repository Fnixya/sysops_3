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

// #define PRODUCER_DEBUG 1
// #define DEBUG 1

#define MAX_THREADS 256
#define MAX_BUFFER 65536
#define LINE_SIZE 64

#define MUTEX_SIZE 3
#define GETOPNUM_MUTEXNO 0
#define QUEUE_MUTEXNO 1
#define UPDATESTOCK_MUTEXNO 2

/* Global Variables_________________________________________________________________________________________________ */

char **operations;
long num_producers, num_consumers, buffer_size;

int op_count, op_num, elem_count,
  fd, 
  profits = 0,
  product_stock [5] = {0},
  purchase_rates [5] = { 2, 5, 15, 25, 100 },
  sale_rates [5] = { 3, 10, 20, 40, 125 };

// Used for debugging
int debug_profits = 0,
  debug_stock[5] = {0},
  debug_count = 0;

pthread_mutex_t mutex[MUTEX_SIZE];
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

void debug_print_result();
int debug_process_element(struct element *elem);

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
  // Check if the number of arguments is correct
  if (argc != 5) { 
    printf("Usage: ./store_manager <file name> <num producers> <num consumers> <buff size>\n");
    return -1;
  }

  char *strerr = malloc(64 * sizeof(char));
  if (my_strtol(argv[2], &num_producers, strerr) == -1) {
    fprintf(stderr, "ERROR: <num producers> %s", strerr);
    printf("Usage: ./store_manager <file name> <num producers> <num consumers> <buff size>\n");
    free(strerr);
    return -2;
  }
  // Convert the number of consumers from a string to an integer
  if (my_strtol(argv[3], &num_consumers, strerr) == -1) {
    fprintf(stderr, "ERROR: <num consumers> %s", strerr);
    printf("Usage: ./store_manager <file name> <num producers> <num consumers> <buff size>\n");
    free(strerr);
    return -3;
  }
  // Convert the buffer size from a string to an integer
  if (my_strtol(argv[4], &buffer_size, strerr) == -1) {
    fprintf(stderr, "ERROR: <buff size> %s", strerr);
    printf("Usage: ./store_manager <file name> <num producers> <num consumers> <buff size>\n");
    free(strerr);
    return -4;
  }
  free(strerr);

  int err_count = 0;
  // Check if the number of producers is greater than 0
  if (num_producers < 1) {
    fprintf(stderr, "ERROR: The number of producers must be greater than 0\n");
    err_count++;
  }
  // Check if the number of consumers is greater than 0
  if (num_consumers < 1) {
    fprintf(stderr, "ERROR: The number of consumers must be greater than 0\n");
    err_count++;
  }
  // Check if the buffer size is greater than 0 and less than or equal to INT_MAX
  if (buffer_size < 1) {
    fprintf(stderr, "ERROR: The buffer size must be greater than 0\n");
    err_count++;
  }
  else if (buffer_size > INT_MAX) {
    fprintf(stderr, "ERROR: The buffer size is too big, introduce a number lower than %d\n", INT_MAX);
    err_count++;
  }

  // If there were any errors, return -5
  if (err_count > 0) {
    return -5;
  }

  // If everything was successful, return 0
  return 0;
}






/***
 * It maps the file into memory
 * @param file_name: file name
 * @return -1 if error, 0 if success
*/
int copy_file(const char *file_name) {
  // Open the file for reading
  FILE* file = fopen(file_name, "r");
  if (file == NULL) { 
    // If the file could not be opened, print an error message and return -1
    perror("Error opening file");
    return -1;
  }

  // Read the number of operations from the file
  if (fscanf(file, "%d", &op_num) != 1) { 
    // If the number of operations could not be read, print an error message, close the file, and return -1
    perror("Error reading number of operations");
    fclose(file);
    return -1;
  }

  // Allocate memory for the elements array
  elements = (struct element *) malloc(op_num * sizeof(struct element));
  if (elements == NULL) { 
    // If memory could not be allocated, print an error message, close the file, and return -1
    perror("Error allocating memory");
    fclose(file);
    return -1;
  }

  // Scan each line of the file, convert it to a struct element, and store it in the elements array
  char tmp_op[9];
  int converted_num, invalid_operations = 0;
  for (int i = 0; i < op_num; i++) {
    converted_num = fscanf(file, "%d %s %d", &elements[i].product_id, tmp_op, &elements[i].units);
    
    // If there are less operations in the file than stated, print an error message, free the elements array, close the file, and return -1
    if (converted_num == -1) {
      fprintf(stderr, "ERROR: There are less operations at the file than stated (N=%d but there are %d operations)\n", op_num, i);
      free(elements);
      fclose(file);
      return -1;
    }
    // If an operation could not be read correctly, increment the count of invalid operations and continue to the next operation
    else if (converted_num != 3) {
      invalid_operations++;
      continue;
    }
    
    // If the operation is a purchase, set the operation type to 0
    if (strcmp(tmp_op, "PURCHASE") == 0) {
      elements[i].op = 0;
    } 
    // If the operation is a sale, set the operation type to 1
    else if (strcmp(tmp_op, "SALE") == 0) {
      elements[i].op = 1;
    }
    // If the operation is not a purchase or a sale, set the operation type to -1 and increment the count of invalid operations
    else {
      elements[i].op = -1;
      invalid_operations++;
    }
  }

  // If there were any invalid operations, print a warning message
  if (invalid_operations > 0) {
    fprintf(stderr, "WARNING: There are %d invalid operations in %s which will be ignored\n", invalid_operations, file_name);
  }

  // Close the file
  if (fclose(file) == -1) {
    // If the file could not be closed, print an error message, free the elements array, and return -1
    perror("Error closing file");
    free(elements);
    return -1;  
  }

  // If everything was successful, return 0
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
  // Allocate memory for thread IDs
  int *ids = (int *) malloc((num_consumers < num_producers ? num_producers : num_consumers) * sizeof(int));

  // Allocate memory for producer threads
  pthread_t *producers = (pthread_t *) malloc(num_producers * sizeof(pthread_t));
  if (producers == NULL) {
    perror("Error allocating memory for producer threads");
    return -1;
  }

  // Allocate memory for consumer threads
  pthread_t *consumers = (pthread_t *) malloc(num_consumers * sizeof(pthread_t));
  if (consumers == NULL) {
    perror("Error allocating memory for consumer threads");
    free(producers);
    return -1;
  }

  // Initialize mutexes
  for (int i = 0; i < MUTEX_SIZE; i++) {
    if (pthread_mutex_init(&mutex[i], NULL) != 0) {
      perror("Error initializing mutex");
      free(producers);
      free(consumers);
      return -1;
    }
  }

  // Initialize condition variables
  if (pthread_cond_init(&non_full, NULL) != 0 || pthread_cond_init(&non_empty, NULL) != 0) {
    perror("Error initializing condition variable");
    for (int i = 0; i < MUTEX_SIZE; i++) {
      pthread_mutex_destroy(&mutex[i]);
    }
    free(producers);
    free(consumers);
    return -1;
  }

  // Create producer threads
  for (int i = 0; i < num_producers; i++) {
    ids[i] = i;
    if (pthread_create(&producers[i], NULL, (void *) producer, (void *) &ids[i]) != 0) {
      perror("Error creating producer thread");
      for (int j = 0; j < i; j++) {
        pthread_cancel(producers[j]);
      }
      for (int i = 0; i < MUTEX_SIZE; i++) {
        pthread_mutex_destroy(&mutex[i]);
      }
      pthread_cond_destroy(&non_full);
      pthread_cond_destroy(&non_empty);
      free(producers);
      free(consumers);
      return -1;
    }
  }

  // Create consumer threads
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
      for (int i = 0; i < MUTEX_SIZE; i++) {
        pthread_mutex_destroy(&mutex[i]);
      }
      pthread_cond_destroy(&non_full);
      pthread_cond_destroy(&non_empty);
      free(producers);
      free(consumers);
      return -1;
    }
  }

  // Wait for all producer threads to finish
  for (int i = 0; i < num_producers; i++) {
    pthread_join(producers[i], NULL);
  }

  // Wait for all consumer threads to finish
  for (int i = 0; i < num_consumers; i++) {
    pthread_join(consumers[i], NULL);
  }

  // Destroy mutexes and condition variables
  for (int i = 0; i < MUTEX_SIZE; i++) {
    pthread_mutex_destroy(&mutex[i]);
  }
  pthread_cond_destroy(&non_full);
  pthread_cond_destroy(&non_empty);

  // Free allocated memory for threads
  free(producers);
  free(consumers);
  free(ids);

  return 0;
}





/***
 * Conversion from string to long integer using strtol with some error handling
 * @param string: string to convert to long
 * @param number: pointer to store the result
 * @return Error -1 otherwise 0 
*/
int my_strtol(const char *string, long *number, char *strerr) {
    // Declare pointers for the start and end of the string
    char *nptr, *endptr = NULL;
    // Initialize the start pointer to the start of the string
    nptr = (char *) string;
    // Reset the error number
    errno = 0;
    // Convert the string to a long integer
    *number = strtol(nptr, &endptr, 10);

    // If the end pointer is not at the end of the string, the string is not a valid integer
    if (nptr && *endptr != 0) {
      strcpy(strerr, " is not an integer\n");
      return -1;
    }
    // If the error number is ERANGE and the number is equal to LONG_MAX, an overflow occurred
    else if (errno == ERANGE && *number == LONG_MAX)
    {
      strcpy(strerr, " overflow\n");
      return -1;
    }
    // If the error number is ERANGE and the number is equal to LONG_MIN, an underflow occurred
    else if (errno == ERANGE && *number == LONG_MIN)
    {
      strcpy(strerr, " underflow\n");
      return -1;
    }

    // If no errors occurred, return 0
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

void debug_print_result() {
  printf("Total operations: %d \n", debug_count);
  printf("Total: %d euros\n", debug_profits);
  printf("Stock:\n");
  printf("  Product 1: %d\n", debug_stock[0]);
  printf("  Product 2: %d\n", debug_stock[1]);
  printf("  Product 3: %d\n", debug_stock[2]);
  printf("  Product 4: %d\n", debug_stock[3]);
  printf("  Product 5: %d\n", debug_stock[4]);
}





/***
 * It stores the information scrapped from the file inside an struct element and pushes it into the queue
 * @param elem: element to store
*/
int store_element(struct element *elem, int thread_id) {
  // Debugging flag
  #ifdef DEBUG
  int was_full = 0;
  #endif

  // !! Critical section <begin> !! -> thread pushes the element into the queue
  // Lock the mutex to ensure that only one thread can access the queue at a time
  pthread_mutex_lock(&mutex[QUEUE_MUTEXNO]);
  // If the queue is full, wait until it's not full
  while (queue_full(elem_queue) == 1) {
    #ifdef DEBUG
    was_full = 1;
    printf("\tproducer %d blocked\n", thread_id);
    #endif

    pthread_cond_wait(&non_full, &mutex[QUEUE_MUTEXNO]);
  }

  #ifdef DEBUG
  if (was_full == 1)
    printf("\tproducer %d unblocked\n", thread_id);
  #endif

  // Put the element into the queue
  if (queue_put(elem_queue, elem) == -1)
    return -1;

  // Signal that the queue is not empty
  pthread_cond_signal(&non_empty);
  // Unlock the mutex to allow other threads to access the queue
  pthread_mutex_unlock(&mutex[QUEUE_MUTEXNO]);
  // !! Critical section <end> !!
  
  return 0;
};





/***
 * It processes the information inside an struct element and updats the product stock and profits
 * @param elem: element to process
*/
int process_element(struct element *elem) {
  // Lock the mutex to ensure that only one thread can update the stock and profits at a time
  pthread_mutex_lock(&mutex[UPDATESTOCK_MUTEXNO]);

  // If the operation is a purchase
  if (elem->op == 0) { 
    // Increase the stock of the product by the number of units purchased
    product_stock[elem->product_id - 1] += elem->units;
    // Decrease the profits by the cost of the units purchased
    profits -= purchase_rates[elem->product_id - 1] * elem->units;
  } 
  // If the operation is a sale
  else if (elem->op == 1) { 
    // Decrease the stock of the product by the number of units sold
    product_stock[elem->product_id - 1] -= elem->units;
    // Increase the profits by the revenue from the units sold
    profits += sale_rates[elem->product_id - 1] * elem->units;
  }

  // Unlock the mutex to allow other threads to update the stock and profits
  pthread_mutex_unlock(&mutex[UPDATESTOCK_MUTEXNO]);
  
  return 0;
};


int debug_process_element(struct element *elem) {
  // Lock the mutex to ensure that only one thread can update the stock and profits at a time
  pthread_mutex_lock(&mutex[UPDATESTOCK_MUTEXNO]);

  // Increment the count of processed elements
  debug_count++;

  // If the operation is a purchase
  if (elem->op == 0) { 
    // Increase the stock of the product by the number of units purchased
    debug_stock[elem->product_id - 1] += elem->units;
    // Decrease the profits by the cost of the units purchased
    debug_profits -= purchase_rates[elem->product_id - 1] * elem->units;
  } 
  // If the operation is a sale
  else if (elem->op == 1) { 
    // Decrease the stock of the product by the number of units sold
    debug_stock[elem->product_id - 1] -= elem->units;
    // Increase the profits by the revenue from the units sold
    debug_profits += sale_rates[elem->product_id - 1] * elem->units;
  }

  // Unlock the mutex to allow other threads to update the stock and profits
  pthread_mutex_unlock(&mutex[UPDATESTOCK_MUTEXNO]);
  
  return 0;
};



/***
 * Producer function for the producer thread
 * @return -1 if error, 0 if success
*/
void producer(void *id) {
  // Debugging flag
  #ifdef DEBUG
  fprintf(stdout, "Start producer!\n");
  #endif

  struct element elem;
  int op_index;
  // The producer will keep producing until it has produced all the operations
  while (op_count < op_num) {    
    #ifdef DEBUG
    fprintf(stdout, "I'm a producer!\n");
    #endif

    // Lock the mutex to ensure that only one thread can increment the operation count at a time
    pthread_mutex_lock(&mutex[GETOPNUM_MUTEXNO]);
    // If all operations have been produced, unlock the mutex and break the loop
    if (op_count >= op_num) {
      pthread_mutex_unlock(&mutex[GETOPNUM_MUTEXNO]);
      break;
    }
    // Increment the count of produced operations
    op_index = op_count++;    
    pthread_mutex_unlock(&mutex[GETOPNUM_MUTEXNO]);
    
    #ifdef DEBUG
    fprintf(stdout, "[producer %d begin]\n", op_index);
    #endif

    // Get the operation from the elements array
    elem = elements[op_index];

    // Store the operation in the queue
    if (store_element(&elem, *(int*)id) == -1) {
      fprintf(stderr, "Error storing element\n");
      pthread_exit((void *) -1);
    }

    #ifdef PRODUCER_DEBUG
    debug_process_element(&elem);
    #endif 

    #ifdef DEBUG
    fprintf(stdout, "[producer %d - elem %d end]\n", *(int *) id, op_index);
    #endif
  }

  #ifdef DEBUG
  fprintf(stdout, "\t\t\tEnd producer %d\n", *(int *) id);
  #endif

  // Exit the thread
  pthread_exit(0);
}

/***
 * Consumer function for the consumer thread
 * @return -1 if error, 0 if success
*/
void consumer(void *id) {
  // Debugging flag
  #ifdef DEBUG
  int was_empty = 0;
  fprintf(stdout, "Start consumer %d!\n", *(int *) id);
  #endif

  struct element elem, *elem_ptr;
  int elem_index;
  // The consumer will keep consuming until it has consumed all the operations
  while (elem_count < op_num) {
    // Lock the mutex to ensure that only one thread can access the queue at a time
    pthread_mutex_lock(&mutex[QUEUE_MUTEXNO]);
    // Increment the count of consumed elements
    elem_index = elem_count++; 
    // If all operations have been consumed, unlock the mutex and break the loop
    if (elem_index >= op_num) {
      pthread_mutex_unlock(&mutex[QUEUE_MUTEXNO]);
      break;
    }

    #ifdef DEBUG
    fprintf(stdout, "[consumer %d - elem %d begin]\n", *(int *) id, elem_index);
    #endif

    // If the queue is empty, wait until it's not empty
    while (queue_empty(elem_queue) == 1) {      
      #ifdef DEBUG
      was_empty = 1;
      fprintf(stdout, "\tconsumer %d blocked\n", *(int *) id);
      #endif

      pthread_cond_wait(&non_empty, &mutex[QUEUE_MUTEXNO]);
    }

    #ifdef DEBUG
    if (was_empty == 1)
      fprintf(stdout, "\tconsumer %d unblocked\n", *(int *) id);
    #endif

    // Get an element from the queue
    if ((elem_ptr = queue_get(elem_queue)) == NULL) {
      fprintf(stderr, "Error getting element from queue\n");
      pthread_exit((void *) -1);
    }
    elem = *elem_ptr;

    // Signal that the queue is not full
    pthread_cond_signal(&non_full);
    // Unlock the mutex to allow other threads to access the queue
    pthread_mutex_unlock(&mutex[QUEUE_MUTEXNO]);

    // Process the consumed element
    process_element(&elem);

    #ifdef DEBUG
    fprintf(stdout, "[consumer %d - elem %d end]\n", *(int *) id, elem_index);
    #endif
  }

  #ifdef DEBUG
  fprintf(stdout, "\t\t\tEnd consumer %d\n", *(int *) id);
  #endif

  // Exit the thread
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
  fprintf(stdout, "Number of operations: %d\n", op_num);
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
  
  free(elements); // Free the memory allocated for the elements array
  queue_destroy(elem_queue); // Destroy the queue

  // Output
  #ifdef PRODUCER_DEBUG
  debug_print_result();
  #endif

  print_result();


  return 0;
}
