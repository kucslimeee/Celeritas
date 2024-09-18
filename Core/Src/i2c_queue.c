/*
  * i2c_queue.c
  *
  * Created on: Sep 11, 2024
  * 	   Author: badam
  */

 #include "i2c_queue.h"

 #define QUEUE_SIZE 256
 #define ITEM_SIZE 16 // 15 elements + checksum

 typedef struct {
 	uint8_t data[QUEUE_SIZE][ITEM_SIZE];
 	uint8_t head;
 	uint8_t tail;
 	uint8_t size;
 } Queue;

 Queue queue = {.head = 0, .tail = 0, .size = 0};

 void queue_push(uint8_t item[ITEM_SIZE-1], bool priority, bool checksum){
 	uint8_t new_item[ITEM_SIZE];

 	for (uint8_t i = 0; i < ITEM_SIZE-1; i++){
 		new_item[i] = item[i];
 	}

 	if (checksum){
 		new_item[ITEM_SIZE-1] = calculate_checksum(item, ITEM_SIZE-1);
 	} else {
 		new_item[ITEM_SIZE-1] = 0;
 	}

 	if (queue.size == QUEUE_SIZE){
 		return; // QUEUE_OVERFLOW_ERROR
 	}

 	if (priority){
 		queue.head = (queue.head - 1 + QUEUE_SIZE) % QUEUE_SIZE;
 		strcpy(queue.data[queue.head], item);
 	} else {
 		strcpy(queue.data[queue.tail], item);
 		queue.tail = (queue.tail + 1 + QUEUE_SIZE) % QUEUE_SIZE;
 	}
 	queue.size++;
 }

 uint8_t* queue_get(void){
 	if (queue.size == 0){
 		return NULL;
 	}

 	uint8_t* item = queue.data[0];
 	queue.head = (queue.head + 1 + QUEUE_SIZE) % QUEUE_SIZE;
 	queue.size--;

 	return item;
 }

 void queue_clear(void){
 	queue.head = 0;
 	queue.tail = 0;
 	queue.size = 0;
 }
