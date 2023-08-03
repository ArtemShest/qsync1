/*
 * dht22.h
 *
 *  Created on: May 29, 2023
 *      Author: Jora
 */

#include "main.h"

#ifndef INC_DHT22_H_
#define INC_DHT22_H_

typedef struct
{
	float temper;
	float old_temper;
	float hum;
	float old_hum;
	uint8_t dt[5];
	uint8_t temp_error;
	uint8_t hum_error;
} TEMP_AIR;

void tempAir_getData();
uint8_t dht22_init(void);
uint8_t dht22_GetData(uint8_t *data);
void DelayMicroSec(uint32_t micros);

#endif /* INC_DHT22_H_ */
