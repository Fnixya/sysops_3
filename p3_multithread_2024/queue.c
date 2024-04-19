//SSOO-P3 23/24

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "queue.h"



//To create a queue
queue* queue_init(int size)
{

  queue * q = (queue *)malloc(sizeof(queue));

  // Check if the memory allocation was successful
  if (q == NULL) {
    return NULL;  // Return NULL if the memory allocation failed
  }

  // Allocate memory for the array of elements
  q->array = (struct element *)malloc(size * sizeof(struct element));

  // Check if the memory allocation was successful
  if (q->array == NULL) {
    free(q);  // Free the previously allocated memory for the queue
    return NULL;  // Return NULL if the memory allocation failed
  }

  // Initialize the count, front, and rear
  q->size == size;
  q->count = 0;
  q->front = 0;
  q->rear = -1;

  return q;
}

// To Enqueue an element
int queue_put(queue *q, struct element* x)
{
    // Check if the queue is full
    if (q->count == q->size) {
        return -1;  // Return -1 or error code if the queue is full
    }

    // Increment rear and count
    q->rear++;
    q->count++;
    q->array[q->rear] = *x;
  return 0;
}

// To Dequeue an element.
struct element* queue_get(queue *q)
{
  // Check if the queue is empty
  if (q->count == 0) {
    return NULL;  
  }

  // Get the front element
  struct element* element = &(q->array[q->front]);

  // Increment front and decrement count
  q->front++;
  q->count--;

  return element;  // Return the front element
}





//To check queue state
int queue_empty(queue *q) {
  return (q->count == 0);
}

int queue_full(queue *q) {
  return (q->count == q->size);
}

//To destroy the queue and free the resources
int queue_destroy(queue *q) {
  //Deallocate the elementsi in queue
  free(q->array);
  // Deallocate the queue
  free(q);

  return 0;
}
