#include "Queue.h"
#include "Flash.h"
#include <string.h>

void queue_init(Queue* queue) {
	uint16_t load_length = (queue->max_size * queue->item_size);
    queue->data = malloc(load_length);
    memset(queue->data, 0, load_length);

    // 1 flash read operation is 4 bytes (see flash_load docs).
    flash_load(queue->flash_page, load_length / 4, queue->data);
    while(1) {
    	bool is_valid = false;
    	for(uint16_t i = 0; i < queue->item_size; i++) {
    		if(*((uint16_t *)queue->data+(queue->tail*queue->item_size / 2)+i) != 0xFFFF) {
    			is_valid = true;
    			break;
    		}
    	}
    	if(!is_valid) break;
    	// additional safety in order to prevent overloading the queue
    	if(queue->tail < queue->max_size) queue->tail++;
    	else break;
    }
    queue->size = queue->tail;

}

void queue_push(Queue* queue, void* item) {

	if(queue->tail > queue->max_size -1){queue->tail = 0;}; //if the queue reaches its max, it overflows from the beginning

	memcpy(queue->data + queue->tail * queue->item_size, item, queue->item_size);
	queue->tail = (queue->tail + 1 + QUEUE_SIZE) % QUEUE_SIZE;
 	if(queue->size > queue->max_size-1){queue->size = 128;} else {queue->size++;}; //additional safety
}

bool queue_get(Queue* queue, void** data) {
    if (queue->size == 0){
 		data = 0;
 		return false;
 	}
    if(queue->head > queue->max_size -1){queue->head = 0;}; //if the read position reaches its max, it starts from the beginning
 	void* item = queue->data+queue->head*queue->item_size;
 	queue->head = (queue->head + 1 + QUEUE_SIZE) % QUEUE_SIZE;;
 	if(queue->size > 0) {queue->size--;};	//additional safety

 	*data = item;
 	return true;
}

void queue_clear(Queue* queue) {
    queue->head = 0;
 	queue->tail = 0;
 	queue->size = 0;
}

void queue_save(Queue* queue) {
	flash_save(queue->flash_page,
		queue->size*queue->item_size/2, // 1 flash write operation is 2 bytes
		(uint16_t*)queue->data+(queue->head*queue->item_size/2)
	);
}
