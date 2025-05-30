/*
 * Request.h
 *
 * Created on: Sep 2, 2024
 * 	   Author: badam
 */

#ifndef INC_REQUEST_HANDLER_H_
#define INC_REQUEST_HANDLER_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum class {
	UNKNOWN = 0x00,
	SELFTEST = 0x01,
	MAX_TIME = 0x02,
	MAX_HITS = 0x03
} RequestType;


typedef struct {
	uint8_t ID;				// (new) measurement ID
	RequestType type;
	bool is_okay;
	bool is_header;
	bool continue_with_full_channel;
	uint16_t limit;
	uint32_t start_time;
	uint16_t min_voltage;
	uint16_t max_voltage;
	uint8_t samples;
	uint16_t resolution;
} Request;

bool check_request(Request request, uint32_t time);

#endif /* INC_REQUEST_HANDLER_H_ */
