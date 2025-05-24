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
#include "QueueID.h"
#include "QueueManager.h"

typedef struct {
	QueueID			ID;
    void* 			data;         	// pointer to the data array
    QueueCursor*	cursor;
    uint16_t		max_size;
    uint16_t 		item_size; 		// size of a signle item in queue (in bytes)
    uint32_t*		flash_page;
    uint8_t 		nf_pages;		// number of flash pages
} Queue;

void queue_init(Queue* queue);
void queue_push(Queue* queue, void* item);
bool queue_get(Queue* queue, void** data);
bool queue_delete(Queue* queue, bool (*condition)(void* item));
void queue_save(Queue* queue);
void queue_clear_saved(Queue* queue);

#endif /* INC_QUEUE_H_ */
