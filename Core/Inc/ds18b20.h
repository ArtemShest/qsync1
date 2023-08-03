/*
 * ds18b20.h
 *
 *  Created on: Jun 1, 2023
 *      Author: Jora
 */

#ifndef INC_DS18B20_H_
#define INC_DS18B20_H_

#include "stm32g0xx_hal.h"
#include "main.h"

typedef struct
{
	uint8_t dt[0];
	float temper;
	float old_temper;
	uint16_t raw_temper;
	uint8_t state;
	uint8_t flag;
	uint8_t sign;
	uint8_t error;
} TEMP_WATER;

void tempWater_getData();
uint8_t ds18b20_Reset(void);
uint8_t ds18b20_ReadByte(void);
uint8_t ds18b20_ReadBit(void);
void ds18b20_WriteByte(uint8_t dt);
void ds18b20_WriteBit(uint8_t bit);
uint8_t ds18b20_init();
uint8_t ds18b20_GetSign(uint16_t dt);
float ds18b20_Convert(uint16_t dt);
void DelayMicro(uint32_t micros);
void ds18b20_MeasureTemperCmd();
void ds18b20_ReadStratcpad(uint8_t *Data);
void ds18b20_MeasureTemperCmd();

#endif /* INC_DS18B20_H_ */
