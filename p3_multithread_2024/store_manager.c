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

#define MAX_THREADS 256
#define MAX_BUFFER 65536
#define LINE_SIZE 64
 
#define READFILE_NO 0
#define ENQUEUE_NO 1
#define DEQUEUE_NO 2
#define UPDATE_NO 3

/* Global Variables_________________________________________________________________________________________________ */

char **operations;

int op_count, op_num, elem_count,
  fd, 
  profits = 0,
  product_stock [5] = {0},
  purchase_rates [5] = { -2, -5, -15, -25, -100 },
  sale_rates [5] = { 3, 10, 20, 40, 125 };

pthread_mutex_t mutex[4];
pthread_mutex_t read_file_mutex, update_stock_mutex, read_elem_mutex, write_elem_mutex;
pthread_cond_t non_full, non_empty;

queue elem_queue;

/* Functions _______________________________________________________________________________________________________ */

int read_file();
int my_strtol(const char *string, long *number);
void print_result();

int store_element(struct element *elem);
int process_element(struct element *elem);

void producer();
void consumer();

/* __________________________________________________________________________________________________________________ */


/***
 * It maps the file into memory
 * @param file_name: file name
 * @return -1 if error, 0 if success
*/
int read_file() {
  operations = (char **) malloc(sizeof(char) * LINE_SIZE * op_num);
  return 0;
}

/***
 * Conversion from string to long integer using strtol with some error handling
 * @param string: string to convert to long
 * @param number: pointer to store the result
 * @return Error -1 otherwise 0 
*/
int my_strtol(const char *string, long *number) {
    // https://stackoverflow.com/questions/8871711/atoi-how-to-identify-the-difference-between-zero-and-error
    char *nptr, *endptr = NULL;                            /* pointer to additional chars  */
    nptr = (char *) string;
    endptr = NULL;
    errno = 0;
    *number = strtol(nptr, &endptr, 10);

    // Error extracting number (it is not an integer)
    if (nptr && *endptr != 0) {
      fprintf(stdout, "[ERROR] Not an integer\n");
      return -1;
    }
    // Overflow
    else if (errno == ERANGE && *number == LONG_MAX)
    {
      fprintf(stdout, "[ERROR] Overflow\n");
      return -1;
    }
    // Underflow
    else if (errno == ERANGE && *number == LONG_MIN)
    {
      fprintf(stdout, "[ERROR] Underflow\n");
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
int store_element(struct element *elem) {
  // Critical section !! -> thread pushes the element into the queue
  pthread_mutex_lock(&mutex[ENQUEUE_NO]);
  while (queue_full(&elem_queue)) {
    pthread_cond_wait(&non_full, &mutex[ENQUEUE_NO]);
  }

  queue_put(&elem_queue, elem);

  pthread_cond_signal(&non_empty);
  pthread_mutex_unlock(&mutex[ENQUEUE_NO]);
  
  return 0;
};

/***
 * It processes the information inside an struct element and updats the product stock and profits
 * @param elem: element to process
*/
int process_element(struct element *elem) {
  // rate: it gets the purchase or sale rate of the product (depending on the operation code) 
  int rate = (elem->op == 0) ? purchase_rates[elem->product_id-1] : sale_rates[elem->product_id-1],
      multiplier = (elem->op == 0) ? 1 : -1;

  // Critical section !! -> thread updates common variables: product_stock[] and profits
  pthread_mutex_lock(&mutex[UPDATE_NO]);
  product_stock[elem->product_id-1] += multiplier * elem->units;
  profits = rate * elem->units;
  pthread_mutex_unlock(&mutex[UPDATE_NO]);

  return 0;
};


/***
 * Producer function for the producer thread
 * @return -1 if error, 0 if success
*/
void producer() {
  struct element elem;
  char *op_str = NULL;
  int op_index;
  int product_id, units;
  while (op_count < op_num) {    
    // Critical section !! -> assign operation to thread
    pthread_mutex_lock(&mutex[READFILE_NO]);
    if (op_count >= op_num) {
      pthread_mutex_unlock(&mutex[READFILE_NO]);
      break;
    }
    op_index = op_count++;
    pthread_mutex_unlock(&mutex[READFILE_NO]);

    // Extract the information from the operation
    sscanf(operations[op_index], "%d %s %d", &product_id, op_str, &units);

    // Transform the extracted information as an element
    if (strcmp(op_str, "PURCHASE") == 0)
      elem.op = 0;
    else if (strcmp(op_str, "SALE") == 0)
      elem.op = 1;
    else 
      elem.op = -1;

    elem.product_id = product_id;
    elem.units = units;

    // Store the element inside the queue
    store_element(&elem);
  }

  pthread_exit(0);
  return;
}

/***
 * Consumer function for the consumer thread
 * @return -1 if error, 0 if success
*/
void consumer() {
  struct element *elem;
  while (elem_count < op_num) {
    // Critical section !! -> thread pops the element from the queue
    pthread_mutex_lock(&mutex[DEQUEUE_NO]);
    while (queue_empty(&elem_queue)) {
      pthread_cond_wait(&non_empty, &mutex[DEQUEUE_NO]);
    }

    elem = queue_get(&elem_queue);
    elem_count++;

    pthread_cond_signal(&non_empty);
    pthread_mutex_unlock(&mutex[DEQUEUE_NO]);

    // Process the element and update the common variables
    process_element(elem);
  }
  pthread_exit(0);
  return;
}



/***
 * Main function _____________________________________________________________________________________________________
*/
int main (int argc, const char * argv[])
{
  // Check the number of arguments
  if (argc != 5) {
    printf("ERROR: The program must be called with 4 arguments <file> <producers> <consermers> <buffer>\n");
    return -1;
  }

  const char *file_name = argv[1];
  long num_producers, num_consumers, buffer_size;

  // Open the file
  if ((fd = open(file_name, O_RDONLY)) == -1) {
    perror("ERROR opening file\n");
    return -1;
  }

  // Convert all the arguments to long
  if (my_strtol(argv[2], &num_producers) == -1) {
    fprintf(stderr, "ERROR converting string to long\n");
    return -1;
  }
  if (my_strtol(argv[3], &num_consumers) == -1) {
    fprintf(stderr, "ERROR converting string to long\n");
    return -1;
  }
  if (my_strtol(argv[4], &buffer_size) == -1) {
    fprintf(stderr, "ERROR converting string to long\n");
    return -1;
  }


  // Check the number of producers
  if (num_producers < 1) {
    printf("ERROR: The number of producers must be greater than 0\n");
    return -1;
  }
  // Check the number of consumers  
  if (num_consumers < 1) {
    printf("ERROR: The number of consumers must be greater than 0\n");
    return -1;
  }
  // Check the number of consumers  
  if (buffer_size < 1) {
    printf("ERROR: The buffer size must be greater than 0\n");
    return -1;
  }


  // Warn user of big variables
  if (MAX_BUFFER < buffer_size) {
    printf("WARNING: The size of the buffer might be unnecessary big. It might affect performance.\n");
    return -1;
  }
  if (MAX_THREADS < num_producers) {
    printf("WARNING: The number of producers might be unnecessary big. It might affect performance.\n");
    return -1;
  }
  if (MAX_THREADS < num_consumers) {
    printf("WARNING: The number of consumers might be unnecessary big. It might affect performance.\n");
    return -1;
  }

  // pthread variables initialization
  for (int i = 0; i < 4; i++) {
    pthread_mutex_init(&mutex[i], NULL);
  }
  pthread_cond_init(&non_full, NULL);
  pthread_cond_init(&non_empty, NULL);

  pthread_t *producer_thread = (pthread_t *) malloc(sizeof(pthread_t) * num_producers);
  pthread_t *consumer_thread = (pthread_t *) malloc(sizeof(pthread_t) * num_consumers);


  // Create the producer and consumer threads
  for (int i = 0; i < num_producers; i++) {
    pthread_create(&(producer_thread[i]), NULL, (void *) producer, NULL);
  }
  for (int i = 0; i < num_consumers; i++) {
    pthread_create(&(consumer_thread[i]), NULL, (void *) consumer, NULL);
  }

  // Join the producer and consumer threads
  for (int i = 0; i < num_producers; i++) {
    pthread_join(producer_thread[i], NULL);
  }
  for (int i = 0; i < num_consumers; i++) {
    pthread_join(consumer_thread[i], NULL);
  }

  for (int i = 0; i < 4; i++) {
    pthread_mutex_destroy(&mutex[i]);
  }
  pthread_cond_destroy(&non_full);
  pthread_cond_destroy(&non_empty);

  free(producer_thread);
  free(consumer_thread);

  // Close the file
  if (close(fd) == -1) {
    perror("ERROR closing file");
    return -1;
  }

  // Output
  print_result(product_stock);

  exit(0);
}
