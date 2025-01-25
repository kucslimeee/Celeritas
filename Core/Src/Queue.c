#include "Queue.h"
#include "Flash.h"
#include <string.h>

void queue_init(Queue* queue) {
	uint16_t load_lenght = queue->max_size * queue->item_size;
    queue->data = malloc(load_lenght);

    uint16_t loaded_data[load_lenght];
    flash_load(queue->flash_page, load_lenght, queue->data);
    while(1) {
    	bool is_valid = false;
    	for(uint16_t i = 0; i < queue->item_size; i++) {
    		if(*((uint16_t *)queue->data+(queue->tail*queue->item_size)+i) != 0xFFFF) {
    			is_valid = true;
    			break;
    		}
    	}
    	if(!is_valid) break;
    	queue->tail++;
    }
    queue->size = queue->tail;
}

void queue_push(Queue* queue, void* item) {

	memcpy(queue->data + queue->tail * queue->item_size, item, queue->item_size);
	queue->tail = (queue->tail + 1 + QUEUE_SIZE) % QUEUE_SIZE;
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

void queue_save(Queue* queue) {
	uint16_t save_lenght = queue->size * queue->item_size;
	uint16_t save_data [save_lenght];
	for(uint8_t idx = 0; idx < queue->size; idx++) {
		memcpy(save_data+queue->item_size*idx,
				queue->data+(queue->head+idx)*queue->item_size,
				queue->item_size);
	}
	flash_save(queue->flash_page, save_lenght, &save_data);
}
