/*
 * exp_i2c_slave.c
 *
 *  Created on: Aug 5, 2024
 *      Author: hadha
 */


/*
 * i2c_slave.c
 *
 *  Created on: Aug 1, 2024
 *      Author: hadha
 */


#include "main.h"
#include "exp_i2c_slave.h"
#include "Commands/Commands.h"
#include "RequestQueue.h"
#include "Checksum.h"
#include "i2c_queue.h"

extern I2C_HandleTypeDef hi2c1;
extern ADC_HandleTypeDef hadc1;

#define RxSIZE 8
#define TxSIZE 16
static uint8_t RxData[RxSIZE];
static uint8_t TxData[TxSIZE] = {0x00};
static uint8_t TX_TEMPLATE[TxSIZE] = {0xFF};

uint8_t rxcount;
uint8_t txcount;
uint8_t bytesTransd = 0;
uint8_t counterror;
uint8_t startPosition;



int prc = 1;

extern void HAL_I2C_ListenCpltCallback (I2C_HandleTypeDef *hi2c)
{
	HAL_I2C_EnableListen_IT(hi2c);
}

extern void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode)
{

	if(TransferDirection == I2C_DIRECTION_TRANSMIT)  // if the master wants to transmit the data
	{
		rxcount = 0;
		HAL_I2C_Slave_Sequential_Receive_IT(hi2c, RxData+rxcount, 1, I2C_FIRST_FRAME);

	}
	else
	{
		txcount = 0;
		uint8_t * packet = queue_get();
		if(!packet) packet = TX_TEMPLATE;
		else {
			for(int i = 0; i < TxSIZE; i++) {
				TxData[i] = packet[i];
			}
		}
		HAL_I2C_Slave_Seq_Transmit_IT(hi2c, TxData+txcount, 1, I2C_FIRST_FRAME);
	}
}

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	rxcount++;
	if (rxcount < RxSIZE)
	{
		if (rxcount == RxSIZE-1)
		{
			HAL_I2C_Slave_Sequential_Receive_IT(hi2c, RxData+rxcount, 1, I2C_LAST_FRAME);
		}
		else
		{
			HAL_I2C_Slave_Sequential_Receive_IT(hi2c, RxData+rxcount, 1, I2C_NEXT_FRAME);
		}
	}

	if (rxcount == RxSIZE)
	{
		process_RxData();
	}
}

void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	txcount++;
	if(txcount == TxSIZE-1){
		HAL_I2C_Slave_Seq_Transmit_IT(hi2c, TxData+txcount, 1, I2C_LAST_FRAME);
	}
	else{
		HAL_I2C_Slave_Seq_Transmit_IT(hi2c, TxData+txcount, 1, I2C_NEXT_FRAME);
	}

}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c) //Bus Error / Berror??????
{
	uint32_t errorcode = HAL_I2C_GetError(hi2c);
	if (errorcode == 4)  // AF error
	{
		if (txcount == 0)  // error is while slave is receiving
		{
			//bytesRrecvd = rxcount-1;  // the first byte is the register address
			rxcount = 0;  // Reset the rxcount for the next operation
			process_RxData();
		}
		else // error while slave is transmitting
		{
			bytesTransd = txcount-1;  // the txcount is 1 higher than the actual data transmitted
			txcount = 0;  // Reset the txcount for the next operation

		}
	}
	HAL_I2C_EnableListen_IT(hi2c);

	//BERR error akkor fordul elő ha változik a kommunikáció iránya
}

void process_RxData()
{
	// Step 01: Message checksum checking
	if(1){
		// Step 02: Separate the different parts of RxData
		uint8_t command_id = RxData[1];
		uint8_t command_dec[5];
		for (int i = 2; i < 7; i++) command_dec[i-2] = RxData[i];

		// Step 03: Find which command to execute
		switch(RxData[0]){
		case '0xE0':
			//set_duration(uint8_t * message);
			break;
		case '0xD0':
			//set_scale(uint8_t * message);
			break;
		case 0x07:
			reMeasure(command_id, command_dec);
			break;
		case '0x06':
			//RequestSelfTest(uint8_t * message);
			break;
		case '0x0F':
			//ABReset();
			break;
		case '0x0E':
			//Restart();
			break;
		case '0x0D':
			//Delete_all_measurements();
			break;
		case '0x0C' :
			//Delete_measurements(uint8_t * message); //ide jön a teljes üzenet
			break;
		case '0x0B':
			//Delete_all_request();
			break;
		case '0x0A':
			//Delete_requests(uint8_t * message); //ide jön a teljes üzenet
			break;
		default:
			break;
		}
	}
	else{
		//invalid command error
	}

}
