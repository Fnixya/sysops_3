//SSOO-P3 23/24

#ifndef HEADER_FILE
#define HEADER_FILE


struct element {
  int product_id; //Product identifier
  int op;         //Operation: 0 for purchase, 1 for sale
  int units;      //Product units
};

typedef struct queue {
  // Define the struct yourself
  int size, front, rear, count; 
  struct element *array;
}queue;

queue* queue_init (int size);
int queue_destroy (queue *q);
int queue_put (queue *q, struct element* elem);
struct element * queue_get(queue *q);
int queue_empty (queue *q);
int queue_full(queue *q);

#endif
