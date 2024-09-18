/*
 * RequestQueue.h
 *
 * Created on: Sep 4, 2024
 * 	   Author: badam
 */

#ifndef INC_REQUESTQUEUE_H
#define INC_REQUESTQUEUE_H

#include "Request.h"

#define REQUEST_QUEUE_SIZE 256

void request_queue_put(Request request);
Request request_queue_get(void);
void request_queue_delete(uint8_t id);
void request_queue_clear(void);

#endif // INC_REQUESTQUEUE_H
