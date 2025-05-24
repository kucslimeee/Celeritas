/*
 * RequestQueue.h
 *
 * Created on: Sep 4, 2024
 * 	   Author: badam
 */

#ifndef INC_REQUESTQUEUE_H
#define INC_REQUESTQUEUE_H

#include "Request.h"

void request_queue_init();
void request_queue_put(Request request);
Request request_queue_get(void);

void request_queue_save();
void request_queue_clear_saved();

#endif // INC_REQUESTQUEUE_H
