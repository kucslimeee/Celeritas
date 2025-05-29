/*
 * QueueManager.c
 *
 *  Created on: May 14, 2025
 *      Author: hpraszpi
 */

#include "QueueManager.h"
#include "Flash.h"

// a random, but consistent value that validates the flash page at load
#define CURSOR_FLASH_VALIDITY 0x435FB103

#define CURSORS_LENGTH 2
volatile QueueCursor cursors[CURSORS_LENGTH];

/*
 * @note SHOULD BE CALLED AT SYSTEM INIT BEFORE ANY OTHER QUEUE's.
 */
void queue_manager_init() {
	// the load_length is calculated in flash read ops (1 read op = 4 byte) - see flash_load docs.
	// +1 is added to the length to load the check uint32_t from flash (that validates our cursors).
	uint16_t load_length = (CURSORS_LENGTH * sizeof(QueueCursor)) / 4 + 1;
	uint32_t load_data[load_length];
	flash_load((uint32_t *)QUEUE_MANAGER_ADDR, load_length, load_data);

	if(load_data[0] != CURSOR_FLASH_VALIDITY) {
		// flash invalid
		memset(cursors, 0, CURSORS_LENGTH * sizeof(QueueCursor)); // additional safety. makes sure the cursors empty
		return;
	}
	memcpy(cursors, load_data+1, CURSORS_LENGTH * sizeof(QueueCursor));
}

/*
 * @note The pointer that this method returns MUST BE HANDLED as a READ-ONLY reference!
 * @returns A READ-ONLY reference of the requested queue cursor.
 */
QueueCursor* queue_manager_get_cursor(QueueID queue_id){
	return cursors+queue_id;
}

/*
 * @brief steps head forward on requested cursor (used when querying the queue)
 * @param	queue_id	defines which queue cursor's modification is requested
 * @param	max_size	Head limit. If head reaches this boundary, we set it back to zero.
 */
void queue_manager_step_head(QueueID queue_id, uint16_t max_size){
	// inside queue manager modifications on pointers returned by queue_manager_get_cursor are allowed
	QueueCursor* cursor = queue_manager_get_cursor(queue_id);

	cursor->head++;
	if(cursor->head > max_size - 1){ // check if the queue head is not out of the queue's boundaries
		cursor->head = 0; //if the read position reaches its max, it starts from the beginning
	}
	if(cursor->size > 0) {cursor->size--;};	//additional safety

}

/**
 * @brief steps tail forward on requested cursor (used when a new item is pushed to queue)
 * @param	queue_id	defines which queue cursor's modification is requested
 * @param 	max_size	Tail limit. If tail reaches this boundary, we set it back to zero.
 */
void queue_manager_step_tail(QueueID queue_id, uint16_t max_size){
	// inside queue manager modifications on pointers returned by queue_manager_get_cursor are allowed
	QueueCursor* cursor = queue_manager_get_cursor(queue_id);

	cursor->tail++;
	if(cursor->tail > max_size - 1) {
		cursor->tail = 0; //if the queue reaches its max, it overflows from the beginning
	}

	if(cursor->size > max_size - 1) {
		cursor->size = max_size;
	}
	else {
		cursor->size++;
	}

}

void queue_manager_save(){
	// +2 is added to the length to save the check uint32_t from flash (that validates our cursors).
	uint16_t save_length = (CURSORS_LENGTH * sizeof(QueueCursor)) / 2 + 2;
	uint16_t save_data[save_length];

	save_data[0] = CURSOR_FLASH_VALIDITY & 0xFFFF;
	save_data[1] = CURSOR_FLASH_VALIDITY >> 16;
	memcpy(save_data+2, cursors, CURSORS_LENGTH * sizeof(QueueCursor));

	flash_save(QUEUE_MANAGER_ADDR, 1, save_length, (uint16_t *)&save_data);
}

void queue_manager_clear_saved(){
	QueueCursor* cursor = queue_manager_get_cursor(I2C_QUEUE);
	cursor->head = 0;
	cursor->tail = 0;
	cursor->size = 0;
	cursor = queue_manager_get_cursor(REQUEST_QUEUE);
	cursor->head = 0;
	cursor->tail = 0;
	cursor->size = 0;
	queue_manager_save();
}
