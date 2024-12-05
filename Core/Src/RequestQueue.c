/*
 * RequestQueue.h
 *
 * Created on: Sep 4, 2024
 * 	   Author: badam
 */
#include "RequestQueue.h"
#include "Queue.h"
#include "Flash.h"
#define REQUEST_QUEUE_SIZE QUEUE_SIZE

volatile Queue request_queue = {
		.item_size = sizeof(Request),
		.head = 0,
		.tail = 0,
		.size = 0,
		.max_size = 100,
		.flash_page = REQ_QUEUE_ADDR,
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
 	for (int i = request_queue.head; i != request_queue.tail; i++){
 		if (((Request* )request_queue.data+i)->start_time > time){
 			return i;
 		}
 	}
 	return request_queue.tail;
 }

/**
  * Puts a request into the request queue, sorted by start time.
  * If the queue is full, the request is discarded.
  */
void request_queue_put(Request request){
	if (request_queue.size >= REQUEST_QUEUE_SIZE) return;

	uint8_t insert_pos = find_insert_position(request.start_time);
	for (int i = request_queue.tail; i > insert_pos; i--){
		memcpy(request_queue.data+i, request_queue.data+(i-1)*request_queue.item_size, request_queue.item_size);
	}
	memcpy(request_queue.data+insert_pos*request_queue.item_size, &request, request_queue.item_size);
	request_queue.tail = (request_queue.tail+1) % REQUEST_QUEUE_SIZE;
	request_queue.size++;
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
	return *request;
}

/**
  * Deletes a request from the request queue by its ID.
  * If the queue is empty or the ID is not found, the function does nothing.
  */
void request_queue_delete(uint8_t id){
	bool condition(void* item){
		return ((Request* )item)->ID == id;
	}

	queue_delete(&request_queue, condition);
}

/**
  * Clears the request queue by resetting all pointers and the size.
  */
void request_queue_clear(void){
	request_queue.head = 0;
	request_queue.tail = 0;
	request_queue.size = 0;
}

void request_queue_save(){
	queue_save(&request_queue);
}
