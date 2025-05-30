/*
 * RequestQueue.h
 *
 * Created on: Sep 4, 2024
 * 	   Author: badam
 */
#include "RequestQueue.h"
#include "Queue.h"
#include "Flash.h"
#include "i2c_queue.h"

extern Request empty_request;

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

	int i = request_queue.cursor->head; //start from the head position

	i--; //to compensate for the first increment in the dowhile loop

	do {
		i++;										//increment the position i
		if (i >= request_queue.max_size){i = 0;}; 	//if this overflows, then start from the beginning of the buffer
		if (((Request* )(request_queue.data + i * request_queue.item_size))->start_time > time){ //Check the start time of to stored request to the new one
		 	return i;	//return this insert position
		 }

	}
	while (i != request_queue.cursor->tail); //do this until reaching the tail position

	return request_queue.cursor->tail;	//if the tail position was reached, this means none of the already stored requests have later start time than the new request
}										//by default the insert position is in the tail position


/**
  * Puts a request into the request queue, sorted by start time.
  * If the queue is full, the request is discarded.
  */
void request_queue_put(Request request){

	if (request_queue.cursor->size >= request_queue.max_size) { //if the request queue is full, then register an error in i2c queue and exit the function
		request_queue.cursor->size = request_queue.max_size;
		add_error(request.ID, REQUESTQUEUEFULL);

		return;
	}

	uint8_t insert_pos = find_insert_position(request.start_time); //find the position to insert the new request

	for (int i = request_queue.cursor->tail; i != insert_pos; i--){ //this for loop pushes all requests starting from the insert position and ending at the tail, one block of memory higher

		if (i < 0 || i >= request_queue.max_size) { //if i wonders away from the queue location by accident, we do not want to sort memory outside
			add_error(request.ID, REQUESTSORT);
			return;
		}

		if (i != 0) { // copy position i-1 to position i
			memcpy(request_queue.data + (i * request_queue.item_size), request_queue.data + (i-1) * request_queue.item_size, request_queue.item_size);
		}
		else { //if the position is 0, then copy the highest position to i=0
			memcpy(request_queue.data, request_queue.data + (request_queue.max_size - 1)*request_queue.item_size, request_queue.item_size);
			i = request_queue.max_size; // go to the highest position
		}

	}
	memcpy(request_queue.data+insert_pos*request_queue.item_size, &request, request_queue.item_size); //finally copy the new request into the insert position
	queue_manager_step_tail(request_queue.ID, request_queue.max_size); //step the tail
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

void request_queue_clear_saved(){
	queue_clear_saved(&request_queue);
}
