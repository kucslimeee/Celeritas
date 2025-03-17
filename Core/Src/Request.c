/*
 * Request.c
 *
 *  Created on: Mar 17, 2025
 *      Author: hpraszpi
 */

#include "Request.h"


bool check_request(Request request, uint32_t time) {
	if(request.type != SELFTEST && request.type != MAX_TIME && request.type != MAX_HITS)
		return false;
	if(request.start_time < time) return false;
	return true;
}
