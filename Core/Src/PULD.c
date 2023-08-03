/*
 * PULD.c
 *
 *  Created on: Apr 26, 2023
 *      Author: artem
 */

#include "PULD.h"
#include "main.h"

void PULD_init(t_PULD *puld, UART_HandleTypeDef *huart)
{
	puld->uart = huart;

	puld->config_min.amperage = 0; 	puld->config_max.amperage = 120;

	puld->config_min.delay = 0; 		puld->config_max.delay = 255;
	puld->config_min.frequency = 0; 	puld->config_max.frequency = 666;
	puld->config_min.type_start = 0; 	puld->config_max.type_start = 2;
	puld->config_min.width = 0; 		puld->config_max.width = 300;
	puld->config_min.temp = 15; 		puld->config_max.temp = 35;

    HAL_UART_Receive_IT(puld->uart, puld->buff, 6);
}

void PULD_checkConnectTime(t_PULD *puld)
{
	if(puld->connect_time < 250)  puld->connect_time++; // защита от переполнения :)


	// счетчик connect_time сбрасывается при приеме ответа от ЗГ
	if(puld->connect_time > PULD_CONNECT_ERROR_TIME)
	{
		puld->connect = 0; // нет связи с зг
		//puld->all_status = 1;
	}
}

void PULD_getStatus(t_PULD *puld)
{
	uint8_t mes[] = {0xA5, 0x2, 0x09, 0x10, 0x40};
	PULD_sendMessage(puld, mes, 5);
}

void PULD_sendMessage(t_PULD *puld, uint8_t *data, uint8_t dataSize)
{
	HAL_UART_Transmit(puld->uart , data, dataSize, 100);
}

void PULD_checkMsg(t_PULD *puld)
{
	if(puld->newMsg == 1)
	{
		puld->connect = 1;
		puld->newMsg = 0;
		// обработать пришедшее сообщение

		//   A5 03 05 10 05 3E  // ответ от бп на запрос статуса

/*		start – 1 байт, стартовый, равен 0xA5;
		len – 1 байт, длина значащей части сообщения (байтов данных + 2);
		INS – 2 байта, код сообщения, команда.
		data – размер произвольный, поле данных;
		CS – 1 байт, младший байт суммы всех предыдущих байтов
		*Обращаем внимание, что концом сообщения является байт не с величиной, равной CS,
			а с величиной равной разности 0x0000-CS.

		03 - длина значащей части
		05 10 - код сообщения (0x1005)
		05 - data
		3e - контрольная сумма (с2 +3е = 0)

*/
		if((puld->buff[2] == 0x05) && (puld->buff[3] == 0x10))
		{
			// пришел статус устройтсва
			puld->state = puld->buff[4];
			puld->connect_time = 0; // сбрасываем счетчик коннекта
		}

	}
}
