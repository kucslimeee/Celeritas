/*
  * i2c_queue.c
  *
  * Created on: Sep 11, 2024
  * 	   Author: badam
  */

 #include "i2c_queue.h"

 #define QUEUE_SIZE 256
 #define ITEM_SIZE 16 // 15 elements + checksum

#define TIMEOUT 0xFD
#define INTERRUPT 0xFB
#define CORRUPTED 0xF7

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


void add_header(Request request, uint16_t duration){
	 uint8_t headerData[ITEM_SIZE];
	 uint16_t localDur = duration;
	 uint32_t localTime = request.start_time;
	 headerData[0] = request.ID;
	 //headearData[1] = temperature;
	 headerData[2] = (uint8_t)(localDur >> 8);
	 localDur -= (headerData[2] << 8);
	 headerData[3] = (uint8_t)(localDur);
	 for(int i = 0; i < 4; i++){
		 headerData[4+i] = (uint8_t)(localTime >> 24-i*8);
		 localTime -= (headerData[i] << 24-i*8);
	 }
	 //headerData[9] = request.number_of_Intervals;
	 headerData[10] = request.min_voltage;
	 headerData[11] = request.max_voltage;
	 headerData[12] = 0;
	 headerData[13] = 0;
	 headerData[14] = 0;
	 headerData[15] = 0xFF;
	 //queue_push(headerData, priority???, checksum???);

 }


 void add_spectrum(Request request, unsigned int* spectrum, uint8_t resolution){
	 const uint8_t everyBit = 15*8; //120
	 uint8_t importantBits = everyBit/resolution;
	 uint8_t bitArr[everyBit]; //120 byte
	 uint8_t data[ITEM_SIZE-1];
	 for(int i = 0; i < resolution; i++){
		 for(int j = 0; j < importantBits; j++){
			 if(spectrum[i] % 2 == 0){
				 spectrum[i] /= 2;
				 bitArr[(i+1)*importantBits-j-1] = 0;
			 }
			 else{
				 spectrum[i] = (spectrum[i]-1)/2;
				 bitArr[(i+1)*importantBits-j-1] = 1;
			 }
		 }
	 }

	 for(int k = 0; k < ITEM_SIZE-1; k++){
		 for(int m = 0; m < 8; m++){
			 if(bitArr[k*8+m] == 1){
				 data[k] += pow(2, 7-m);
			 }
		 }
	 }
	 //queue_push(data, priority???, checksum???);



}

 void add_error(Request request, uint8_t error_type){
	 uint8_t errorData[15] = {0};
	 if(error_type == TIMEOUT){
		 errorData[14] = 0xFD;
	 }
	 if(error_type == INTERRUPT){
		 errorData[14] = 0xFB;
	 }
	 if(error_type == CORRUPTED){
	 	errorData[14] = 0xF7;
	 }
	 //queue_push(errorData, priority???, checksum???);
 }
