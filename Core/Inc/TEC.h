/*
 * TEC.h
 *
 *  Created on: Apr 26, 2023
 *      Author: artem
 */

#ifndef INC_TEC_H_
#define INC_TEC_H_

#include "stm32g0xx_hal.h"

#define ERROR_BIT 0
#define READY_BIT 1

typedef struct
{
	GPIO_TypeDef* error_port;
		uint16_t error_pin;
	GPIO_TypeDef* ready_port;
		uint16_t ready_pin;
	uint8_t state;
} t_TEC;


void TEC_getValue(t_TEC *tec);

void TEC_init(t_TEC *tec,
	GPIO_TypeDef* error_port, uint16_t error_pin,
	GPIO_TypeDef* ready_port, uint16_t ready_pin );


#endif /* INC_TEC_H_ */
