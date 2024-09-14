/*
 * RequestQueue.h
 *
 * Created on: Sep 4, 2024
 * 	   Author: badam
 */
#include "RequestQueue.h"

static Request request_queue[REQUEST_QUEUE_SIZE];
static uint8_t head = 0; // starting index of the queue (0 - REQUEST_QUEUE_SIZE-1)
static uint8_t tail = 0; // ending index of the queue (0 - REQUEST_QUEUE_SIZE-1)
static uint8_t size = 0; // number of elements in the queue

/** Finds the position to insert the new request based on its start time.
  * Starts at the head and goes until arriving at the tail. If head == tail (size == 0),
  * the index of the tail is returned.
  * @return The position to insert the new request
  */
static uint8_t find_insert_position(uint32_t time){
 	for (int i = head; i != tail; i++){
 		if (request_queue[i].start_time>time){
 			return i;
 		}
 	}
 	return tail;
 }

/**
  * Puts a request into the request queue, sorted by start time.
  * If the queue is full, the request is discarded.
  */
void request_queue_put(Request request){
	if (size >= REQUEST_QUEUE_SIZE) return;

	uint8_t insert_pos = find_insert_position(request.start_time);
	for (int i = size; i > insert_pos; i--){
		request_queue[i] = request_queue[i-1];
	}
	request_queue[insert_pos] = request;
	tail = (tail+1) % REQUEST_QUEUE_SIZE;
	size++;
}

/**
  * Returns the next request from the request queue, sorted by start time.
  * If the queue is empty, returns a default request with start time of 0.
  * Moves the head pointer and decrements the size of the queue.
  * @return The next request from the queue
  */
Request request_queue_get(void){
	if (size == 0){
		return (Request){ .start_time = 0 };
	}

	Request first_request = request_queue[head];
	head = (head+1) % REQUEST_QUEUE_SIZE;
	size--;

	return first_request;
}

/**
  * Deletes a request from the request queue by its ID.
  * If the queue is empty or the ID is not found, the function does nothing.
  */
void request_queue_delete(uint8_t id){
	for (int i = 0; i < size; i++){
		uint8_t index = (head+i) % REQUEST_QUEUE_SIZE;
		if (request_queue[index].ID == id){
			for (int j = i; j < size-1; j++){
				uint8_t current = (head+j) % REQUEST_QUEUE_SIZE;
				uint8_t next = (head+j+1) % REQUEST_QUEUE_SIZE;
				request_queue[current] = request_queue[next]; // move all requests on down
			}
			tail = (tail-1+REQUEST_QUEUE_SIZE) % REQUEST_QUEUE_SIZE;
			size--;
			return;
		}
	}
	return; // Error: ID not found
}

/**
  * Clears the request queue by resetting all pointers and the size.
  */
void request_queue_clear(void){
	head = 0;
	tail = 0;
	size = 0;
}
