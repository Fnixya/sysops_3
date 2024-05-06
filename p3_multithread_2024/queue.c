//SSOO-P3 23/24

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "queue.h"


/***
 * It creates a queue with a given size
 * @param size: size of the queue
 * @return queue object
*/
queue* queue_init(int size)
{
  fprintf(stderr, "queue_init()\n");
  queue *q;

  // Check if the memory allocation was successful
  if ((q = (queue *) malloc(sizeof(queue))) == NULL) {
    return NULL;  // Return NULL if the memory allocation failed
  }

  // Allocate memory for the array of elements
  // Check if the memory allocation was successful
  if ((q->array = (struct element *) malloc(size * sizeof(struct element))) == NULL) {
    free(q);  // Free the previously allocated memory for the queue
    return NULL;  // Return NULL if the memory allocation failed
  }

  // Initialize the count, front, and rear
  q->size = size;
  q->count = 0;
  q->front = 0;
  q->rear = 0;

  return q;
}

/***
 * To Enqueue an element
 * @param q: queue
 * @param x: element
 * @return 0 if the element was enqueued successfully, -1 otherwise
*/
int queue_put(queue *q, struct element* elem)
{
  fprintf(stderr, "queue_put() product_id: %d, op: %d, units: %d\n", elem->product_id, elem->op, elem->units);

  // Check if the queue is full
  if (q->count == q->size) {
      return -1;  // Return -1 or error code if the queue is full
  }

  // Increment rear and count
  q->array[q->rear] = *elem;
  q->rear = (q->rear+1)%q->size;
  q->count++;
  
  return 0;
}

/***
 * To Dequeue an element
 * @param q: queue
 * @return element
*/
struct element* queue_get(queue *q)
{
  // Check if the queue is empty
  if (q->count == 0) {
    return NULL;  
  }

  // Get the front element
  struct element* element = &(q->array[q->front]);

  // Increment front and decrement count
  q->front = (q->front+1)%q->size;
  q->count--;

  fprintf(stderr, "queue_get() product_id: %d, op: %d, units: %d\n", element->product_id, element->op, element->units);

  return element;  // Return the front element
}


/***
 * @param q: queue
 * @return 1 if the queue is empty, 0 otherwise
*/
int queue_empty(queue *q) {
  fprintf(stderr, "queue_empty()\n");

  return (q->count == 0);
}

/***
 * @param q: queue
 * @return 1 if the queue is full, 0 otherwise
*/
int queue_full(queue *q) {
  fprintf(stderr, "queue_full()\n");
  return (q->count == q->size);
}

/*** To destroy the queue and free the resources
 * @param q: queue
 * @return 0 if the queue was destroyed successfully
 */
int queue_destroy(queue *q) {
  fprintf(stderr, "queue_destroy()\n");

  // Deallocate the elementsi in queue
  free(q->array);
  // Deallocate the queue
  free(q);

  return 0;
}
