/*
 * dht22.c
 *
 *  Created on: May 29, 2023
 *      Author: Jora
 */
#include "dht22.h"

extern TEMP_AIR temp_air;

void tempAir_getData()
{
	if(dht22_GetData(temp_air.dt)) // если датчик отвечает
	{
		temp_air.temper = (float)((((uint16_t)temp_air.dt[2]<<8)|temp_air.dt[1]) & 0x3FFF) / 10;
		if((((uint16_t)temp_air.dt[2]<<8)|temp_air.dt[1]) & 0x8000) temp_air.temper *= -1.0;
		temp_air.hum = (float)(((uint16_t)temp_air.dt[4]<<8)|temp_air.dt[3]) / 10;

		//uint8_t aa = temp_air.temper - temp_air.old_temper;
		if(((abs(temp_air.temper - temp_air.old_temper) >= 5)
				&& (temp_air.temper < temp_air.old_temper)
				&& (temp_air.old_temper != 0))
					|| temp_air.temper < 0)
		{
			temp_air.temper = temp_air.old_temper; // пришли неправильные показания, откатить обратно
		}
		else
		{
			temp_air.old_temper = temp_air.temper;
		}
		if(((abs(temp_air.hum - temp_air.old_hum) >= 5)
				&& (temp_air.hum < temp_air.old_hum)
				&& (temp_air.old_hum != 0))
					|| temp_air.hum < 0)
		{
			temp_air.hum = temp_air.old_hum;  // пришли неправильные показания, откатить обратно
		}
		else
		{
			temp_air.old_hum = temp_air.hum;
		}

		if((temp_air.temper >= 10) && (temp_air.temper <= 35)) temp_air.temp_error = 0;
		else temp_air.temp_error = 1;

		if((temp_air.hum > 0) && (temp_air.hum <= 70)) temp_air.hum_error = 0;
		else temp_air.hum_error = 1;
	}

	else
	{
		temp_air.temp_error = 1;
		temp_air.hum_error = 1;
	}
}



uint8_t dht22_init(void)
{
	//HAL_Delay(2000);
    //GPIOC->ODR |= TEMP_AIR_Pin;	//высокий уровень
    HAL_GPIO_WritePin(GPIOC, TEMP_AIR_Pin, SET);
    return 0;
}

uint8_t dht22_GetData(uint8_t *data)
{
  uint8_t i, j = 0;
  //reset port
  GPIOC->ODR &= ~TEMP_AIR_Pin;//низкий уровень
  GPIOC->ODR |= TEMP_AIR_Pin;//высокий уровень
  DelayMicroSec(100000);
  GPIOC->ODR &= ~TEMP_AIR_Pin;//низкий уровень
  HAL_Delay(18);
  GPIOC->ODR |= TEMP_AIR_Pin;//высокий уровень
  DelayMicroSec(39);
  if(GPIOC->IDR & TEMP_AIR_Pin)  // если датчик не ответил притягиванем шины
  {
	return 0;
  }
  DelayMicroSec(80);

  //если датчик не отпустил шину, то ошибка
  if(!(GPIOC->IDR & TEMP_AIR_Pin))
  {
	return 0;
  }
  DelayMicroSec(80); // читаем данные
  for (j=0; j<5; j++)
  {
	data[4-j]=0;
	for(i=0; i<8; i++)
	{
		while(!(GPIOC->IDR & TEMP_AIR_Pin));
		DelayMicroSec(30);
		if(GPIOC->IDR & TEMP_AIR_Pin) //читаем результат по прошествии 30 микросекунд
		{
			data[4-j] |= (1<<(7-i));
		}
		while(GPIOC->IDR & TEMP_AIR_Pin); //ждём, пока датчик притянет шину (в случае единицы)
	}
  }
  return 1;
}

void DelayMicroSec(uint32_t micros)
{
	micros *= (SystemCoreClock / 1000000) / 9;
	while (micros--) ;
}

