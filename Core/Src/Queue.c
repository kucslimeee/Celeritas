#include "Queue.h"

void queue_init(Queue* queue) {
    queue->data = malloc(QUEUE_SIZE * queue->item_size);
}

void queue_push(Queue* queue, void* item, bool priority) {

 	if (priority){
 		queue->head = (queue->head - 1 + QUEUE_SIZE) % QUEUE_SIZE;
 		memcpy(queue->data + queue->head * queue->item_size, item, queue->item_size);
 	} else {
 		memcpy(queue->data + queue->tail * queue->item_size, item, queue->item_size);
 		queue->tail = (queue->tail + 1 + QUEUE_SIZE) % QUEUE_SIZE;
 	}
 	queue->size++;
}

bool queue_get(Queue* queue, void** data) {
    if (queue->size == 0){
 		data = 0;
 		return false;
 	}
 	void* item = queue->data+queue->head*queue->item_size;
 	queue->head = (queue->head + 1 + QUEUE_SIZE) % QUEUE_SIZE;
 	queue->size--;

 	*data = item;
 	return true;
}

void queue_clear(Queue* queue) {
    queue->head = 0;
 	queue->tail = 0;
 	queue->size = 0;
}

bool queue_delete(Queue* queue, bool (*condition)(void* item)) {
    for (int i = 0; i < queue->size; i++){
		uint8_t index = (queue->head+i) % QUEUE_SIZE;
		if (condition(queue->data+index)){
			for (int j = i; j < queue->size-1; j++){
				uint8_t current = (queue->head+j) % QUEUE_SIZE;
				uint8_t next = (queue->head+j+1) % QUEUE_SIZE;
				memcpy(queue->data+current*queue->item_size, queue->data+next*queue->item_size, queue->item_size); // move all requests on down
			}
			queue->tail = (queue->tail-1+QUEUE_SIZE) % QUEUE_SIZE;
			queue->size--;
			return true;
		}
	}
	return false; // Error: ID not found
}
