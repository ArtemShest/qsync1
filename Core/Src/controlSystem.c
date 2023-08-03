/*
 * controlSystem.c
 *
 *  Created on: Apr 25, 2023
 *      Author: artem
 */




#include "controlSystem.h"
#include "w5100s.h"
#include "puld.h"
#include "main.h"
#include "dht22.h"
#include "ds18b20.h"

#define SIZE_BF 32

extern W5100s PPU1;
extern W5100s PPU2;
extern t_PULD PULD;
extern t_CS CS;
extern TEMP_WATER temp_water;
extern TEMP_AIR temp_air;
extern uint8_t flag_temp_stab;
extern uint8_t flag_stab;

extern uint32_t count_stab;

uint8_t checkReadyNextMode(uint8_t cur_mode)
{
	switch(cur_mode)
	{
		case STANDY_MODE:
			/*
			if (flag_temp_stab && flag_stab &&
					PPU1.connect && PPU2.connect && !temp_air.hum_error &&
					!temp_air.temp_error && !temp_water.error)
			{
				return 1;
			}
			else return 0;
			*/
			/*
			if (!(temp_air.hum_error) && !(temp_air.temp_error))
			{
				return 1;
			}
			else return 0;
			*/
			return 1;
		break;
		case PREPARATION_MODE:
			return 1;
		break;
		default:
			return 1;
	}
}

void check_MainStatus(t_CS *CS) // статус зг и системы в целом
{
	uint32_t status = 0;

	status |= (1 << (CS->mode+1));

	if(checkReadyNextMode(CS->mode))
	{
		status |= (1 << SYS_RDY_BIT);

	}

	if(temp_water.error)
	{
		status |= (1 << SYS_WTEMP_ERR);
		status |= (1 << SYS_ERR_BIT);
	}

	if(temp_air.hum_error)
	{
		status |= (1 << SYS_AHUM_ERR);
		status |= (1 << SYS_ERR_BIT);
	}

	if(temp_air.temp_error)
	{
		status |= (1 << SYS_ATEMP_ERR);
		status |= (1 << SYS_ERR_BIT);
	}
/*
	if((PPU1.connect == 0) || (PPU2.connect == 0 ) || (PULD.connect == 0 ))
	{
		CS->error = 1;
		status |= (1 << SYS_ERR_BIT);
	}
*/
	switch(PULD.state)
	{
		case 0x00:
			// нет связи с зг
			status |= (1 << SYS_ERR_BIT);
			CS->error = 1;
		break;
		case 0x01:
			// подготовка
			CS->error = 1;
			// термостабилизация ЛД включена,
			// желаемая температура не достигнута
			break;
		case 0x02:
			// готов
			CS->error = 0;
			//термостабилизация ЛД включена,
			//желаемая температура  достигнута,
			//разрешено включать ток
			break;
		case 0x04:
			// выключен
			//термостабилизация ЛД выключена
			break;
		case 0x05:
			CS->error = 0;
			// включен
			// ток ЛД включен
			break;
		case 0x81:
			// перегрев ЛД
			// температура ЛД превысила 35 °С,
			// выключите ток, проверьте систему охлаждения и
			// условия окружающей среды
			status |= (1 << MO_LD_OVT_ERR);
			CS->error = 1;
			status |= (1 << SYS_ERR_BIT);
			break;
		case 0x82:
			// перегрузка по току
			status |= (1 << MO_OVС_ERR);
			CS->error = 1;
			status |= (1 << SYS_ERR_BIT);
			break;
		case 0x83:
			// Обрыв цепи
			status |= (1 << MO_BRK_ERR);
			CS->error = 1;
			status |= (1 << SYS_ERR_BIT);
			break;
		case 0x84:
			// перегрузка по напряжению
			status |= (1 << MO_OVV_ERR);
			CS->error = 1;
			status |= (1 << SYS_ERR_BIT);
			break;
		case 0x85:
			// короткое замыкание
			status |= (1 << MO_SHRT_ERR);
			CS->error = 1;
			status |= (1 << SYS_ERR_BIT);
			break;
		case 0x86:
			// перегрев радиатора
			status |= (1 << MO_HS_OVT_ERR);
			CS->error = 1;
			status |= (1 << SYS_ERR_BIT);
			break;
		case 0x87:
			// Т радиатора ниже норм
			status |= (1 << MO_HS_UDT_ERR);
			CS->error = 1;
			status |= (1 << SYS_ERR_BIT);
			break;
		case 0x88:
			// нет связи с подсистемами
			status |= (1 << MO_LINK_ERR);
			CS->error = 1;
			status |= (1 << SYS_ERR_BIT);
			break;
		case 0x89:
			// сбой микросхемы пельтье
			status |= (1 << MO_TEC_ERR);
			CS->error = 1;
			status |= (1 << SYS_ERR_BIT);
			break;
		case 0x8A:
			// перегрев пельтье
			status |= (1 << MO_TEC_OVT_ERR);
			CS->error = 1;
			status |= (1 << SYS_ERR_BIT);
			break;
		case 0x8B:
			// Т ЛД нестабильна
			status |= (1 << MO_LD_OVT_ERR);
			CS->error = 1;
			status |= (1 << SYS_ERR_BIT);
			break;
		case 0x8C:
			// Т печки 1 нестабильна
			status |= (1 << MO_HEAT1_ERR);
			CS->error = 1;
			status |= (1 << SYS_ERR_BIT);
			break;
		case 0x8D:
			// Т печки 2 нестабильна
			status |= (1 << MO_HEAT2_ERR);
			CS->error = 1;
			status |= (1 << SYS_ERR_BIT);
			break;
	}
	CS->status = status;
	//uint8_t st = (CS->status & 1);

	//if(st == 1)
	//{
	//	CS->error = 1;
	//}
}


uint8_t check_bit(uint16_t statusBlock, uint8_t bit) // 1 - если бит установлен
{
	if(statusBlock & (1 << bit))
	{
		return 1;
	}
	else return 0;
}


void CS_init(t_CS *CS, UART_HandleTypeDef *huart)
{
	CS->uart = huart;

	//HAL_UART_Receive_IT(CS->uart, CS->buff, 6);

	CS->mode = STANDY_MODE;
	CS->error = 1;
}

void CS_checkMsg(t_CS *CS)
{
	if(CS->newMsg == 1)
	{
		CS->newMsg = 0;
		// обработать пришедшее сообщение
		if(CS->buff[0] == 1) // 1. команда смены режима
		{
			CS_changeMode(CS->buff[4]);

			CS_createAns(1);
		}
		else if(CS->buff[0] == 2) // 2. запрос статуса устройства
		{
			// сreate status
			CS_createAns(2);
		}
	}
}


void CS_changeMode(uint8_t newMode)
{
	if(newMode == 0) // включить дежурный
	{
		// отправить БП команды выключения ( 2 )
		uint8_t comm[] = {2, 0, 0, 0};

		w5100s_sendMes(&PPU1, comm, 4);
		w5100s_sendMes(&PPU2, comm, 4);
		CS.mode = STANDY_MODE;
	}
	if((newMode == 1) && checkReadyNextMode(CS.mode)) // включить режим подготовки
	{
		//
		// если статус БП ОК отправить команду "включить"

		/*
			добавить проверку ошибок статусов
		*/
		CS.mode = PREPARATION_MODE;

		uint8_t comm[] = {1, 0, 0, 0};
		w5100s_sendMes(&PPU1, comm, 4);
		w5100s_sendMes(&PPU2, comm, 4);

		// выставить флаг включении импульсов
		CS.cur_time_PulseOn = count_stab;
		CS.flag_PulseOn = 1;
		// запомнить текущее время

	}
	if(newMode == 2) // включить рабочий режим
	{
		// если статус БП ОК отправить команду "включить"

		/*
			добавить проверку ошибок статусов
		*/

		if(CS.mode != 0)
		{
			CS.mode = OPERATING_MODE;
		}
		else return;
	}
}

void CS_Pulse_On()
{
	if((CS.flag_PulseOn == 1) && ((count_stab - CS.cur_time_PulseOn) >= 6))
	{
		CS.flag_PulseOn = 0;
		uint8_t comm[] = {3, 0, 0, 0};
		w5100s_sendMes(&PPU1, comm, 4);
		w5100s_sendMes(&PPU2, comm, 4);
	}
}

void CS_createAns(uint8_t command)
{
	if ((command == 1) || (command == 2)) // отправить ответ
	{
		uint32_t state0 = CS.status;


		uint32_t state1 = PPU1.all_status;
		uint32_t state2 = PPU2.all_status;

		//uint32_t state1 = CS_ProcessStatePPU(&PPU1);
		//uint32_t state2 = CS_ProcessStatePPU(&PPU2);

		//ответ вида { cm, статус зг, статус накачки 1, статус накачки 2, КС  }

		uint8_t cs = 0;

		uint8_t airTemp = (uint8_t)temp_air.temper;
		uint8_t airHum = (uint8_t)temp_air.hum;

		uint8_t data[] = {command, state0>>24,state0>>16,state0>>8,state0,
									state1>>24,state1>>16,state1>>8,state1,
									state2>>24,state2>>16,state2>>8,state2, airTemp, airHum, cs};

		for(uint8_t i = 0; i < 15; i++)
		{
			cs += data[i];
		}
		data[15] = cs;
		ans(data, 16);
	}
}


void ans(uint8_t *data, uint8_t dataSize)
{
	// отправка ответа
	HAL_GPIO_WritePin(RS485_DIR_GPIO_Port, RS485_DIR_Pin, GPIO_PIN_SET);
	HAL_UART_Transmit(CS.uart, data, dataSize, 100);
	HAL_GPIO_WritePin(RS485_DIR_GPIO_Port, RS485_DIR_Pin, GPIO_PIN_RESET);

}
