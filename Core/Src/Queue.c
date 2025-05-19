/**
 *             Queue.c/.h                                QueueManager.c/.h
 *	+-------------------------+         +----------------------------+
 *  |       QueueID (enum)    |         |    QueueCursor (struct)    |
 *	|-------------------------|         |----------------------------|
 *	| Values must be hardcoded|         | uint16_t head              |
 *	|-------------------------|			| uint16_t tail              |
 *	| I2C_QUEUE     = 0x0     |         | uint16_t size              |
 *	| REQUEST_QUEUE = 0x1     |         +----+--------+--------------+
 * 	+-----------+-------------+         	 |		  ^
 *  	        |                            |        |
 *      	    | identification              |        | (volatile QueueCursor[] cursors)
 *          	| for cursor ops             |        |
 *          	v                            |        |
 *	+-------------------------+   read-only  |        |
 *	|       Queue (struct)    |<-------------+		  |
 *	|-------------------------| link (QueueCursor*    |
 *	| Runtime-specific queue   |			Queue.cursor) |
 *	| data (RAM address or    |                       |
 *	| hardcoded information   |						  |
 *	| won’t be saved (except  |						  |
 *	| the CONTENT of data,    |						  |
 *	| which is handled        |						  |
 *	| separately))            |						  |
 *	|-------------------------|  provides         +---+------------------------+
 *	| void*        data       |<------------------| 	   QueueManager		   |
 *	| uint16_t     size       |  cursor access    |		   (subsystem)		   |
 *	| uint32_t     flash_page  |         		  |----------------------------|
 *	| uint8_t      n_pages    |------------------>| Manages cursors            |
 *	+-------------------------+	 asks for cursor  +----------------------------+
 *								 updates
 *
 */

#include "Queue.h"
#include "Flash.h"
#include <string.h>

/**
 * @note SHOULD BE ONLY CALLED by implementations (i2c and requests queues) AT SYSTEM INIT
 * 		 AFTER QUEUE_MANAGER's initialization.
 */
void queue_init(Queue* queue) {
	uint16_t load_length = (queue->max_size * queue->item_size);
    queue->data = malloc(load_length);

    // 1 flash read operation is 4 bytes (see flash_load docs).
    flash_load(queue->flash_page, load_length / 4, queue->data);
    queue->cursor = queue_manager_get_cursor(queue->ID);
}

void queue_push(Queue* queue, void* item) {
	memcpy(queue->data + queue->cursor->tail * queue->item_size, item, queue->item_size);
	queue_manager_step_tail(queue->ID, queue->max_size, false);
}

bool queue_get(Queue* queue, void** data) {
    if (queue->cursor->size == 0){
 		data = 0;
 		return false;
 	}

 	*data = queue->data+queue->cursor->head*queue->item_size;
    queue_manager_step_head(queue->ID, queue->max_size, false);

 	return true;
}

void queue_save(Queue* queue) {
	flash_save(queue->flash_page,
		queue->nf_pages,
		queue->item_size*queue->max_size/2, // 1 flash write operation is 2 bytes
		(uint16_t*)queue->data
	);
}
