/*
 * ds18b20.c
 *
 *  Created on: Jun 1, 2023
 *      Author: Jora
 */

#include "ds18b20.h"

extern TEMP_WATER temp_water;


void tempWater_getData()
{
	if (temp_water.flag == 0)
	  {
		  ds18b20_MeasureTemperCmd();
		  temp_water.flag = 1;
	  }
	  else if(temp_water.flag == 1)
	  {
		  temp_water.flag = 0;
		  ds18b20_ReadStratcpad(temp_water.dt);
		  temp_water.raw_temper = ((uint16_t)temp_water.dt[1]<<8)|temp_water.dt[0];

		  //if(ds18b20_GetSign(temp_water.raw_temper)) temp_water.sign='1';
		  //else temp_water.sign='0';

		  temp_water.temper = ds18b20_Convert(temp_water.raw_temper);

		  // (temp_water.temper < temp_water.old_temper)

		  if(((temp_water.temper - temp_water.old_temper >= 5) && (temp_water.old_temper != 0)) || temp_water.temper < 0)
		  { // пришли неправильные показания, откатить обратно
			 temp_water.temper = temp_water.old_temper;
		  }
		  else
		  {
			temp_water.old_temper = temp_water.temper;
		  }
		  temp_water.temper = 20;
		  if((temp_water.temper >= 15) && (temp_water.temper <= 25)) temp_water.error = 0;
		  else temp_water.error = 1;
	  }
}

void ds18b20_MeasureTemperCmd()
{
  ds18b20_Reset();
  ds18b20_WriteByte(0xCC);
  ds18b20_WriteByte(0x44);

}

uint8_t ds18b20_Reset(void)
{
  uint16_t status;
  GPIOB->ODR &= ~GPIO_PIN_2;//низкий уровень
  DelayMicro(485);//задержка как минимум на 480 микросекунд
  GPIOB->ODR |= GPIO_PIN_2;//высокий уровень
  DelayMicro(65);//задержка как минимум на 60 микросекунд
  status = GPIOB->IDR & GPIO_PIN_2;//проверяем уровень
  DelayMicro(500);//задержка как минимум на 480 микросекунд
  return (status ? 1 : 0);//вернём результат
}

uint8_t ds18b20_ReadBit(void)
{
  uint8_t bit = 0;
  GPIOB->ODR &= ~ GPIO_PIN_2;//низкий уровень
  DelayMicro(2);
  GPIOB->ODR |= GPIO_PIN_2;//высокий уровень
  DelayMicro(13);
  bit = (GPIOB->IDR & GPIO_PIN_2 ? 1 : 0);
  DelayMicro(45);
  return bit;
}

void ds18b20_ReadStratcpad(uint8_t *Data)
{
  uint8_t i;
  ds18b20_Reset();
  ds18b20_WriteByte(0xCC);
  ds18b20_WriteByte(0xBE);
  for(i=0;i<8;i++)
  {
    Data[i] = ds18b20_ReadByte();
  }
}

uint8_t ds18b20_ReadByte(void)
{
  uint8_t data = 0;
  for (uint8_t i = 0; i <= 7; i++)
  data += ds18b20_ReadBit() << i;
  return data;
}

void ds18b20_WriteByte(uint8_t dt)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    ds18b20_WriteBit(dt >> i & 1);
    DelayMicro(5);
  }
}

void ds18b20_WriteBit(uint8_t bit)
{
  GPIOB->ODR &= ~GPIO_PIN_2; //низкий уровень
  DelayMicro(bit ? 3 : 65);
  GPIOB->ODR |= GPIO_PIN_2; //высокий уровень
  DelayMicro(bit ? 65 : 3);
}

uint8_t ds18b20_init()
{
  if(ds18b20_Reset()) return 1;

  ds18b20_WriteByte(0xCC);
  //WRITE SCRATCHPAD
    ds18b20_WriteByte(0x4E);
    //TH REGISTER 100 градусов
    ds18b20_WriteByte(0x64);
    //TL REGISTER - 30 градусов
    ds18b20_WriteByte(0x9E);
    //Resolution 12 bit
    ds18b20_WriteByte(0x7F);
  return 0;
}

uint8_t ds18b20_GetSign(uint16_t dt)
{
  //Проверим 11-й бит
  if (dt&(1<<11)) return 1;
  else return 0;
}

float ds18b20_Convert(uint16_t dt)
{
  float t;
  t = (float) ((dt&0x07FF)>>4); //отборосим знаковые и дробные биты
  //Прибавим дробную часть
  t += (float)(dt&0x000F) / 16.0f;
  return t;
}

//--------------------------------------------------
void DelayMicro(uint32_t micros)
{
	micros *= (SystemCoreClock / 1000000) / 9; //примерно микросекунда с частотой камня 64мгц
	while (micros--) ;
}
