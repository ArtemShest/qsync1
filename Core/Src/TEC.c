/*
 * TEC.c
 *
 *  Created on: Apr 26, 2023
 *      Author: artem
 */
#include "TEC.h"
#include "main.h"


void TEC_init(t_TEC *tec,
	GPIO_TypeDef* error_port, uint16_t error_pin,
	GPIO_TypeDef* ready_port, uint16_t ready_pin )
{
	tec->error_port = error_port;
	tec->error_pin = error_pin;
	tec->ready_port = ready_port;
	tec->ready_pin = ready_pin;
}


void TEC_getValue(t_TEC *tec)
{
	tec->state &= ~(HAL_GPIO_ReadPin(tec->error_port, tec->error_pin) << ERROR_BIT);
	tec->state &= ~(HAL_GPIO_ReadPin(tec->ready_port, tec->ready_pin) << READY_BIT);
}
