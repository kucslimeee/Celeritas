/*
 * RequestQueue.h
 *
 * Created on: Sep 4, 2024
 * 	   Author: badam
 */
#include "RequestQueue.h"

static Request request_queue[REQUEST_QUEUE_SIZE];
static uint8_t head = 0;
static uint8_t tail = 0;
static uint8_t size = 0;

void request_queue_put(Request request){
	if (size >= REQUEST_QUEUE_SIZE) return;

	/* Find insert position */
	uint8_t insert_pos = tail;
	for (int i = 0; i < size; i++){
		if (request_queue[i].start_time > request.start_time){
			insert_pos = i;
			break;
		}
	}

	for (int i = size; i > insert_pos; i--){
		request_queue[i] = request_queue[i-1];
	}
	request_queue[insert_pos] = request;
	tail = (tail+1) % REQUEST_QUEUE_SIZE;
	size++;
}

Request request_queue_get(void){
	if (size == 0){
		return (Request){ .start_time = 0 };
	}

	Request first_request = request_queue[head];
	head = (head+1) % REQUEST_QUEUE_SIZE;
	size--;

	return first_request;
}

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

void request_queue_clear(void){
	head = 0;
	tail = 0;
	size = 0;
}
