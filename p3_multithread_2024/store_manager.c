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

#define MAX_THREADS 16

// Functions
int read_file(char *file_name);
int producer();
int consumer();

/***
 * Read the file
*/
int read_file(char *file_name) {
  return -1;
}

/***
 * Producer function
*/
int producer() {
  pthread_exit(0);
}

/***
 * Consumer function
*/
int consumer() {
  pthread_exit(0);
}



/***
 * Main function
*/
int main (int argc, const char * argv[])
{
  // Check the number of arguments
  if (argc != 5) {
    printf("ERROR: The program must be called with 4 arguments\n");
    return -1;
  }

  char *file_name = argv[1];

  // Change to strtol in the future
  int num_producers = atoi(argv[2]),
    num_consumers = atoi(argv[3]),
    buffer = atoi(argv[4]);

  if (N < 1) {
    printf("ERROR: The number of consumers must be greater than 0\n");
    return -1;
  }

  if (M < 1) {
    printf("ERROR: The number of producers must be greater than 0\n");
    return -1;
  }
  
  // printf("WARNING: Overflow buffer\n");

  // Variables
  int profits = 0;
  int product_stock [5] = {0};

  // malloc
  pthread_t producer_thread;
  pthread_t consumer_thread;

  // First one thread for the producer 
  pthread_create(&producer_thread, NULL, producer, NULL);
  pthread_create(&producer_thread, NULL, consumer, NULL);
  pthread_join(&producer_thread, NULL, producer, NULL);
  pthread_exit(&producer_thread, NULL, producer, NULL);


  // Output
  printf("Total: %d euros\n", profits);
  printf("Stock:\n");
  printf("  Product 1: %d\n", product_stock[0]);
  printf("  Product 2: %d\n", product_stock[1]);
  printf("  Product 3: %d\n", product_stock[2]);
  printf("  Product 4: %d\n", product_stock[3]);
  printf("  Product 5: %d\n", product_stock[4]);

  return 0;
}
