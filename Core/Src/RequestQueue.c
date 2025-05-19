/*
 * RequestQueue.h
 *
 * Created on: Sep 4, 2024
 * 	   Author: badam
 */
#include "RequestQueue.h"
#include "Queue.h"
#include "Flash.h"

volatile Queue request_queue = {
		.ID = REQUEST_QUEUE,
		.item_size = sizeof(Request),
		.max_size = 102,
		.flash_page = REQ_QUEUE_ADDR,
		.nf_pages = 1,
};

void request_queue_init() {
	queue_init(&request_queue);
}

/** Finds the position to insert the new request based on its start time.
  * Starts at the head and goes until arriving at the tail. If head == tail (size == 0),
  * the index of the tail is returned.
  * @return The position to insert the new request
  */
static uint8_t find_insert_position(uint32_t time){
 	for (int i = request_queue.cursor->head; i != request_queue.cursor->tail; i++){
 		if (((Request* )request_queue.data+i)->start_time > time){
 			return i;
 		}
 	}
 	return request_queue.cursor->tail;
 }

/**
  * Puts a request into the request queue, sorted by start time.
  * If the queue is full, the request is discarded.
  */
void request_queue_put(Request request){
	if (request_queue.cursor->size >= request_queue.max_size) {
		request_queue.cursor->size = request_queue.max_size;
		return;
	}

	uint8_t insert_pos = find_insert_position(request.start_time);
	for (int i = request_queue.cursor->tail; i > insert_pos; i--){
		memcpy(request_queue.data+i, request_queue.data+(i-1)*request_queue.item_size, request_queue.item_size);
	}
	memcpy(request_queue.data+insert_pos*request_queue.item_size, &request, request_queue.item_size);
	queue_manager_step_tail(request_queue.ID, request_queue.max_size, false);
}

/**
  * Returns the next request from the request queue, sorted by start time.
  * If the queue is empty, returns a default request with start time of 0.
  * Moves the head pointer and decrements the size of the queue.
  * @return The next request from the queue
  */
Request request_queue_get(void) {
	Request* request = &empty_request;
	queue_get(&request_queue, &request);
	Request request_clone = *request;
	memset(request, 0, request_queue.item_size);
	return request_clone;
}

void request_queue_save(){
	queue_save(&request_queue);
}
