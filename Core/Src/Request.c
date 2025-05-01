/*
 * Request.c
 *
 *  Created on: Mar 17, 2025
 *      Author: hpraszpi
 */

#include "Request.h"


bool check_request(Request request, uint32_t time) {
	if(request.type != SELFTEST && request.type != MAX_TIME && request.type != MAX_HITS)
		return false;		//return false if the measurement is neither of the specified ones
	if(request.start_time < time) return false;		//if the measurement was required to be carried out in the past, return false
	return true;	//other ways return true
}
