/*
 * QueueManager.h
 *
 *  Created on: May 14, 2025
 *      Author: hpraszpi
 */

#ifndef INC_QUEUEMANAGER_H_
#define INC_QUEUEMANAGER_H_

#include <stdint.h>
#include <stdbool.h>
#include "QueueID.h"


typedef struct {
    uint16_t 	head;       	// starting index of the queue (0 - QUEUE_SIZE-1)
    uint16_t 	tail;       	// ending index of the queue (0 - QUEUE_SIZE-1)
    uint16_t	size;			// to seperate the number of new items from items that we can read (for example flash save)
} QueueCursor;


void queue_manager_init();
QueueCursor* queue_manager_get_cursor(QueueID queue_id);
void queue_manager_step_head(QueueID queue_id, uint16_t max_size);
void queue_manager_step_tail(QueueID queue_id, uint16_t max_size);
void queue_manager_save();
void queue_manager_clear_saved();

#endif /* INC_QUEUEMANAGER_H_ */
