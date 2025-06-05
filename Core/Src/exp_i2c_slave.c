/*
 * exp_i2c_slave.c
 *
 *  Created on: Aug 5, 2024
 *      Author: hadha
 */
#include "main.h"
#include "exp_i2c_slave.h"
#include <stdbool.h>
#include "Commands/Commands.h"
#include "RequestQueue.h"
#include "Request.h"
#include "Checksum.h"
#include "i2c_queue.h"
#include "Timer.h"
#include "Scheduler.h"
#include "Flash.h"
#include "Statusreport.h"

extern I2C_HandleTypeDef hi2c1;
extern ADC_HandleTypeDef hadc1;
extern Request current_request;

#define RxSIZE 8
#define TsyncSIZE 5
#define TxSIZE 16
static uint8_t RxData[RxSIZE];
static uint8_t TxData[TxSIZE] = {0x00};
//static uint8_t TX_TEMPLATE[TxSIZE] = {0x00};

uint8_t rxcount;
uint8_t txcount;
uint8_t bytesTransd = 0;
uint8_t counterror;
uint8_t startPosition;

void process_Command();

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
		bool result;
		uint8_t* packet = i2c_queue_get(&result);
		if(!result){
			packet = generate_status_report(false);
		}
		//if(packet[0] == 0x00) packet = TX_TEMPLATE; in the older version default packets were full of zeros
		for(int i = 0; i < TxSIZE; i++) {
			TxData[i] = packet[i];
		}
		HAL_I2C_Slave_Seq_Transmit_IT(hi2c, TxData+txcount, 1, I2C_FIRST_FRAME);
	}
	scheduler_on_i2c_communication();
}

bool isTimesyncCommand(){
 	return RxData[0] == 0x54;
}

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	rxcount++;
	if (rxcount < RxSIZE)
	{
		if (rxcount == TsyncSIZE-1 && isTimesyncCommand())
		{
			HAL_I2C_Slave_Sequential_Receive_IT(hi2c, RxData+rxcount, 1, I2C_LAST_FRAME);
		}
		else
		{
			HAL_I2C_Slave_Sequential_Receive_IT(hi2c, RxData+rxcount, 1, I2C_NEXT_FRAME);
		}
	}

	if (rxcount == TsyncSIZE && isTimesyncCommand())
	{
		process_TimesyncCommand();
	}
	else if (rxcount == RxSIZE)
	{
		process_Command();
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
		}
		else // error while slave is transmitting
		{
			bytesTransd = txcount-1;  // the txcount is 1 higher than the actual data transmitted
			txcount = 0;  // Reset the txcount for the next operation

		}
	}
	HAL_I2C_EnableListen_IT(hi2c);

	//BERR error occurs when the direction of communication changes
}

void process_TimesyncCommand(void)
{
	uint32_t timestamp = (RxData[4]<<24) | (RxData[3]<<16) | (RxData[2]<<8) | RxData[1];
	Set_SystemTime(timestamp);
	scheduler_on_timesync();
}

void process_Command()
{
	// Step 01: Message checksum checking
	if(calculate_checksum(RxData, 7) == RxData[7]){
		// Step 02: Separate the different parts of RxData
		uint8_t command_id = RxData[1];
		uint8_t command_dec[6];
		for (int i = 2; i < 9; i++)
			command_dec[i-2] = RxData[i];

		// Step 03: Find which command to execute
		switch(RxData[0]){
		case 0xE0:
			setDur(command_id, command_dec);
			break;
		case 0xD0:
			setScale(command_id, command_dec);
			break;
		case 0xCC:
			uint8_t* statuspacket = generate_status_report(true);
			i2c_queue_push(statuspacket, true, command_id);
			break;
		case 0x07:
			reMeasure(command_id, command_dec);
			break;
		case 0x06:
			reSelftest(command_id, command_dec);
			break;
		case 0x0F:
			scheduler_clear_all_flash(); //reset to zeros and save all in flash
			scheduler_restart();
			break;
		case 0x0E:
			scheduler_restart(); //restart
			break;
		case 0xAA:
			scheduler_save_all(); //save to flash
			break;
		case 0xBB:
			if (status == RUNNING) status = FINISHED; // stop measurement
			if (status == STARTING) current_request.start_time = 0;
			break;
		default:
			add_error(command_id, UNKNOWNCOMMAND);
			return;
		}
		scheduler_on_command();
	}
	else{
		add_error(RxData[1], CORRUPTED);
	}
}
