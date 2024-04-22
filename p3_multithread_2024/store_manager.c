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


/* Constants _______________________________________________________________________________________________________ */

#define MAX_THREADS 16

/* Global Variables_________________________________________________________________________________________________ */

int fd, 
  profits = 0,
  product_stock [5] = {0},
  purchase_rates [5] = { 2, 5, 15, 25, 100 },
  sale_rates [5] = { 3, 10, 20, 40, 125 };

/* Functions _______________________________________________________________________________________________________ */

int read_line();
int my_strtol(char *string, long *number);
void print_result();

int store_element();
int process_element(struct element *elem);

int producer();
int consumer();



/***
 * Reads a line of the file
 * @param file_name: file name
 * @return -1 if error, 0 if success
*/
int read_line() {
  return 0;
}

/***
 * Conversion from string to long integer using strtol with some error handling
 * @param string: string to convert to long
 * @param number: pointer to store the result
 * @return Error -1 otherwise 0 */
int my_strtol(char *string, long *number) {
    // https://stackoverflow.com/questions/8871711/atoi-how-to-identify-the-difference-between-zero-and-error
    char *nptr, *endptr = NULL;                            /* pointer to additional chars  */
    
    nptr = string;
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
      fprintf(stdout, "[ERROR] Overflow\n", var);
      return -1;
    }
    // Underflow
    else if (errno == ERANGE && *number == LONG_MIN)
    {
      fprintf(stdout, "[ERROR] Underflow\n", var);
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



int store_element() {
  return 0;
};

/***
 * It processes the information inside an struct element and updats the product stock and profits
*/
int process_element(struct element *elem) {
  int id = elem.product_id - 1;

  if (strcmp(elem.op, "PURCHASE") == 0) {
    product_stock[id] += elem.units;
    profits -= purchase_rates[id] * elem.units;
  } 
  else if (strcmp(elem.op, "SALE") == 0) {
    product_stock[id] -= elem.units;
    profits += sale_rates[id] * elem.units;
  }

  return -1;
};



/***
 * Producer function for the producer thread
 * @return -1 if error, 0 if success
*/
int producer() {
  // while (1) {
  //   store_element();
  // }
  pthread_exit(0);
  return 0;
}

/***
 * Consumer function for the consumer thread
 * @return -1 if error, 0 if success
*/
int consumer() {
  // while (1) {
  //   process_element();
  // }
  pthread_exit(0);
  return 0;
}



/***
 * Main function _____________________________________________________________________________________________________
*/
int main (int argc, const char * argv[])
{
  // Check the number of arguments
  if (argc != 5) {
    printf("ERROR: The program must be called with 4 arguments\n");
    return -1;
  }

  char *file_name = argv[1];
  long num_producers, num_consumers, buffer_size;

  // Open the file
  if ((fd = open(file_name, O_RDONLY)) == -1) {
    perror("ERROR opening file");
    return -1;
  }

  // Convert all the arguments to long
  if (my_strtol(argv[2], &num_producers) == -1) {
    perror("ERROR converting string to long");
    return -1;
  }
  if (my_strtol(argv[3], &num_consumers) == -1) {
    perror("ERROR converting string to long");
    return -1;
  }
  if (my_strtol(argv[4], &buffer_size) == -1) {
    perror("ERROR converting string to long");
    return -1;
  }

  // fscanf("%d", )

  // Check the number of consumers  
  if (num_consumers < 1) {
    printf("ERROR: The number of consumers must be greater than 0\n");
    return -1;
  }

  // Check the number of producers
  if (num_producers < 1) {
    printf("ERROR: The number of producers must be greater than 0\n");
    return -1;
  }

  // Malloc
  pthread_t producer_thread = malloc(sizeof(pthread_t) * num_producers);
  pthread_t consumer_thread = malloc(sizeof(pthread_t) * num_consumers);

  for (int i = 0; i < num_producers; i++) {
    pthread_create(&producer_thread[i], NULL, producer, NULL);
  }
  
  for (int i = 0; i < num_consumer; i++) {
    pthread_create(&consumer_thread[i], NULL, consumer, NULL);
  }

  for (int i = 0; i < num_producers; i++) {
    pthread_join(&producer_thread[i], NULL, producer, NULL);
  }
  
  for (int i = 0; i < num_consumer; i++) {
    pthread_join(&consumer_thread[i], NULL, consumer, NULL);
  }

  // block main process until all threads are finished

  free(producer_thread);
  free(consumer_thread);

  // Close the file
  if (close(fd) == -1) {
    perror("ERROR closing file");
    return -1;
  }

  // Output
  print_result(product_stock);

  return 0;
}
