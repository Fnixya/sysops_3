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


/* Functions _______________________________________________________________________________________________________ */
int read_file(char *file_name);
int my_strtol(char *string, long *number);
int producer();
int consumer();

/***
 * Read the file
 * @param file_name: file name
 * @return -1 if error, 0 if success
*/
int read_file(char *file_name) {
  return -1;
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
        if (mode == 0)
            fprintf(stdout, "[ERROR] Not an integer\n");
        return -1;
    }
    // Overflow
    else if (errno == ERANGE && *number == LONG_MAX)
    {
        if (mode == 0)
            fprintf(stdout, "[ERROR] Overflow\n", var);
        return -1;
    }
    // Underflow
    else if (errno == ERANGE && *number == LONG_MIN)
    {
        if (mode == 0)
            fprintf(stdout, "[ERROR] Underflow\n", var);
        return -1;
    }

    return 0;
}

/***
 * Producer function
 * @return -1 if error, 0 if success
*/
int producer() {
  pthread_exit(0);
  return 0;
}

/***
 * Consumer function
 * @return -1 if error, 0 if success
*/
int consumer() {
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

  if (my_strtol(argv[2], &num_producers) == -1) return -1;
  if (my_strtol(argv[3], &num_consumers) == -1) return -1;
  if (my_strtol(argv[4], &buffer_size) == -1) return -1;

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
