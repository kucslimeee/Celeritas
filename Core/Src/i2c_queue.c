/*
  * i2c_queue.c
  *
  * Created on: Sep 11, 2024
  * 	   Author: badam
  */

 #include "i2c_queue.h"
#include "Request.h"
#include "Scheduler.h"
#include "Queue.h"
#include "Flash.h"
#include "Timer.h"

#define ITEM_SIZE 16 // 15 elements + checksum

extern volatile uint8_t interrupt_counter;

volatile Queue i2c_queue = {
		.item_size = 16,
		.head = 0,
		.tail = 0,
		.size = 0,
		.max_size = 128,
		.flash_page = I2C_QUEUE_ADDR,
};

void i2c_queue_init() {
	queue_init(&i2c_queue);
}

 void i2c_queue_push(uint8_t* item, bool checksum){
 	uint8_t new_item[ITEM_SIZE];
 	uint8_t copy_lenght = (checksum) ? ITEM_SIZE - 1 : ITEM_SIZE;
 	for (uint8_t i = 0; i < copy_lenght; i++){
 		new_item[i] = item[i];
 	}

 	if (checksum){
 		new_item[ITEM_SIZE-1] = calculate_checksum(item, ITEM_SIZE-1);
 	}

 	if (i2c_queue.size == 256){
 		return; // QUEUE_OVERFLOW_ERROR
 	}

 	queue_push(&i2c_queue, new_item);
 }

 uint8_t* i2c_queue_get(bool* result){
	 uint8_t* data;
	 *result = queue_get(&i2c_queue, &data);
	 return data;
 }

 void i2c_queue_clear(void){
	queue_clear(&i2c_queue);
}

 uint8_t* i2c_queue_fetch(uint8_t idx, bool* result) {
	 if(i2c_queue.size == 0) {
		 *result = false;
		 return NULL;
	 }

	 result = true;
	 return i2c_queue.data+(i2c_queue.head + idx)*ITEM_SIZE;
 }

 uint8_t i2c_queue_count(bool (*filter)(uint8_t* item)) {
	 if(i2c_queue.size == 0) {
		 return NULL;
	 }

	 uint8_t filtered_count = 0;
	 for (int i = 0; i < i2c_queue.size; i++) {
		 bool res;
		 uint8_t* item = i2c_queue_fetch(i, &res);
		 if(res)
			 if (filter(item)) filtered_count++;
	 }
	 return filtered_count;
 }

 void i2c_queue_save() {
	 queue_save(&i2c_queue);
 }

void add_header(Request request, uint16_t duration){
	 uint8_t headerData[ITEM_SIZE];

	 headerData[0] = request.ID;
	 headerData[1] = (uint8_t)(interrupt_counter & 0xFF);

	 uint16_t temp = get_temperature();
	 headerData[2] = temp >> 8;
	 headerData[3] = temp & 0xFF;

	 uint32_t localTime = Get_SystemTime();
	 for(int i = 0; i < 4; i++){
		 headerData[4+i] = (uint8_t)(localTime >> 24-i*8);
		 localTime -= (headerData[i] << 24-i*8);
	 }

	 headerData[8] = request.resolution;
	 headerData[9] = request.min_voltage >> 4;
	 headerData[10] = ((request.min_voltage & 0xF) << 4) | (request.max_voltage >> 8);
	 headerData[11] = request.max_voltage & 0xFF;

	 uint16_t vbat = get_refint_voltage();
	 headerData[12] = vbat >> 8;
	 headerData[13] = vbat & 0xFF;

	 headerData[14] = 0xFF;
	 i2c_queue_push(headerData, true);

 }

void add_spectrum(uint16_t* spectrum){
	i2c_queue_push(((uint8_t*)spectrum), false);
}

 void add_error(uint8_t request_id, ErrorType error_type){
	 uint8_t errorData[15] = {0};
	 errorData[0] = request_id;
	 errorData[14] = 0xD5;
   errorData[15] = (uint8_t)error_type;
	 i2c_queue_push(errorData, true);
 }
