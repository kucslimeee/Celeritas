/*
  * i2c_queue.h
  *
  * Created on: Sep 11, 2024
  * 	   Author: badam
  */

#ifndef INC_QUEUE_H_
#define INC_QUEUE_H_

#define QUEUE_SIZE 256
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    void* 		data;         	// pointer to the data array
    uint16_t 	head;       	// starting index of the queue (0 - QUEUE_SIZE-1)
    uint16_t 	tail;       	// ending index of the queue (0 - QUEUE_SIZE-1)
    uint16_t	size;			// to seperate the number of new items from items that we can read (for example flash save)
    uint16_t	max_size;
    uint16_t 	item_size; // size of a signle item in queue (in bytes)
    uint32_t*	flash_page;
} Queue;

void queue_init(Queue* queue);
void queue_push(Queue* queue, void* item);
bool queue_get(Queue* queue, void** data);
void queue_clear(Queue* queue);
bool queue_delete(Queue* queue, bool (*condition)(void* item));
void queue_save(Queue* queue);

#endif /* INC_QUEUE_H_ */
