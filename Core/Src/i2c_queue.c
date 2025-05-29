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
#include "Checksum.h"

#define ITEM_SIZE 16 // 15 elements + checksum

extern volatile uint8_t interrupt_counter;

volatile Queue i2c_queue = {
		.ID = I2C_QUEUE,
		.item_size = ITEM_SIZE,
		.max_size = 256,
		.flash_page = I2C_QUEUE_PAGE_1_ADDR,
		.nf_pages = 2,
};

void i2c_queue_init() {
	queue_init(&i2c_queue);
}

 void i2c_queue_push(uint8_t* item, bool checksum, uint8_t current_ID){
 	uint8_t new_item[ITEM_SIZE];
 	uint8_t copy_lenght = (checksum) ? ITEM_SIZE - 1 : ITEM_SIZE;

 	if (i2c_queue.cursor->size == i2c_queue.max_size - 1){ 	// push the QUEUE_OVERFLOW_ERROR packet and exit
		memset(new_item, 0, ITEM_SIZE);						//we do not want to call the add error recursively
 		new_item[0] = current_ID;
		new_item[13] = I2CQUEUEFULL;
		new_item[14] = 0xD5;
	} else {										// if the i2c queu is not full, then proceed
		for (uint8_t i = 0; i < copy_lenght; i++){
			new_item[i] = item[i];
		}
	}


 	if (checksum){
 		new_item[ITEM_SIZE-1] = calculate_checksum(item, ITEM_SIZE-1);
 	}

 	if (i2c_queue.cursor->size >= i2c_queue.max_size){
 		i2c_queue.cursor->size = i2c_queue.max_size;
 		return; 			// QUEUE_OVERFLOW_ERROR, do not push data
 	}

 	queue_push(&i2c_queue, new_item);
 }

 uint8_t* i2c_queue_get(bool* result){
	 uint8_t* data;
	 *result = queue_get(&i2c_queue, &data);
	 return data;
 }

 uint8_t* i2c_queue_fetch(uint8_t idx, bool* result) {
	 if(i2c_queue.cursor->size == 0) {
		 *result = false;
		 return NULL;
	 }

	 result = true;
	 return i2c_queue.data+(i2c_queue.cursor->head + idx)*ITEM_SIZE;
 }

uint8_t i2c_queue_count(bool (*filter)(uint8_t* item)) {
	if(i2c_queue.cursor->size == 0) {
		 return 0;
	}

	uint8_t filtered_count = 0;
	for (int i = i2c_queue.cursor->head; i != i2c_queue.cursor->tail; i++) {
		if (i >= i2c_queue.max_size){
			i = - 1;
		}
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

void i2c_queue_clear_saved() {
	queue_clear_saved(&i2c_queue);
}

void add_header(Request request, uint16_t duration){

	uint32_t localTime = Get_SystemTime();
	uint16_t v_int = get_refint_voltage();
	uint16_t temp = get_temperature(v_int); //measure temperature adc value and convert to Kelvin
											//also depends on internal reference voltage
	uint8_t headerData[ITEM_SIZE];

	headerData[0] = request.ID;
	headerData[1] = (uint8_t)(interrupt_counter & 0xFF);

	headerData[2] = temp >> 8;
	headerData[3] = temp & 0xFF;

	for(int i = 0; i < 4; i++){
		headerData[4+i] = (uint8_t)(localTime >> 24-i*8);
		localTime -= (headerData[i] << 24-i*8);
	}

	headerData[8] = (uint8_t)(request.resolution / 8);		//this means the number of packets
	headerData[9] = request.min_voltage >> 4;
	headerData[10] = ((request.min_voltage & 0xF) << 4) | (request.max_voltage >> 8);
	headerData[11] = request.max_voltage & 0xFF;

	headerData[12] = v_int >> 8;		//internal voltage reference in mV
	headerData[13] = v_int & 0xFF;

	headerData[14] = 0xFF;
	i2c_queue_push(headerData, true, request.ID);

}

void add_spectrum(uint16_t* spectrum, uint8_t meas_ID){
	i2c_queue_push(((uint8_t*)spectrum), false, meas_ID);
}

 void add_error(uint8_t request_id, ErrorType error_type){
	 uint8_t errorData[ITEM_SIZE] = {0};
	 errorData[0] = request_id;
	 errorData[13] = (uint8_t)error_type;
	 errorData[14] = 0xD5;
	 i2c_queue_push(errorData, true, request_id);
 }



