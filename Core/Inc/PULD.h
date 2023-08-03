/*
 * PULD.h
 *
 *  Created on: Apr 26, 2023
 *      Author: artem
 */

#ifndef INC_PULD_H_
#define INC_PULD_H_

#include "stm32g0xx_hal.h"

typedef struct
{
	float temp; 		//температура ЛД
	float amperage; 	//амплитуда импульса
	signed char type_start; //тип запуска
	float frequency; 	// частота
	float width; 		//ширина
	float delay; 		// задержка(фронта импульса от запускающего)

} t_config;



typedef struct
{
	UART_HandleTypeDef *uart;
	t_config config_max;
	t_config config_min;
	t_config config;


	uint8_t buff[6];
	uint8_t buff1[2];
	uint8_t buff2[10];
	uint8_t newMsg;
	uint8_t state;
	uint8_t status;
	uint8_t connect_time;
	uint8_t connect;

} t_PULD; //power unit laser diode

#define SYS_ERR_BIT 0  //	 0 - ошибки нет; 1 - есть ошибка
#define SYS_STDBY_BIT 1 //	 0 - СУ не находится в режиме "Дежурный"; 1 - СУ находится в режиме "Дежурный"
#define SYS_PREP_BIT 2 //	 0 - СУ не находится в режиме "Подготовка"; 1 - СУ находится в режиме "Подготовка"
#define SYS_OP_BIT 3 // 	 0 - СУ не находится в режиме "Рабочий"; 1 - СУ находится в режиме "Рабочий"
#define SYS_RDY_BIT 4  //	 0 - СУ не готова перейти в следующий режим; 1 - СУ готова перейти в следующий режим
#define SYS_WFLOW_ERR 5 // 	 0 - есть напор в системе охлаждения; 1 - нет напора в системе охлаждения
#define SYS_WTEMP_ERR 6 //	 0 - температура охлаждающей жидкости в норме; 1 - температура охлаждающей жидкости выше нормы
#define SYS_AHUM_ERR 7 //	 0 - влажность воздуха в норме; 1 - влажность воздуха выше нормы
#define SYS_ATEMP_ERR 8 //	 0 - температура воздуха в норме; 1 - температура воздуха выше нормы
#define MO_LD_OVT_ERR 9 //	 0 - температура ЛД ЗГ в норме; 1 - температура ЛД ЗГ превысила 35 °С
#define MO_OVС_ERR 10 // 	 0 - ток ЗГ в норме; 1 - перегрузка ЗГ по току
#define MO_BRK_ERR 11 //     0 - нет обрыва цепей ЗГ; 1 - обрыв цепи ЗГ
#define MO_OVV_ERR 12 // 	 0 - напряжение ЗГ в норме; 1 - перегрузка ЗГ по напяжению
#define MO_SHRT_ERR 13 //	 0 - нет КЗ в цепях ЗГ; 1 - КЗ в цепях ЗГ
#define MO_HS_OVT_ERR 14 //	 0 - температура радиатора ЗГ не выше нормы; 1 - температура радиатора ЗГ выше нормы
#define MO_HS_UDT_ERR 15 //  0 - температура радиатора ЗГ не ниже нормы; 1 - температура радиатора ЗГ ниже нормы
#define MO_LINK_ERR 16 // 	 0 - есть связь с подсистемами ЗГ; 1 - нет связи с подсистемами ЗГ
#define MO_TEC_ERR 17 //	 0 - микросхема Пельтье ЗГ в норме; 1 - сбой микросхемы Пельтье ЗГ
#define MO_TEC_OVT_ERR 18 // 0 - температура элемента Пельтье ЗГ в норме; 1 - перегрев элемента Пельтье ЗГ
#define MO_TEC_UNST_ERR 19// 0 - температура элемента Пельтье ЗГ стабильна; 1 - температура элемента Пельтье ЗГ нестабильна
#define MO_HEAT1_ERR 20 //	 0 - температура печки1 ЗГ стабильна;  1 - температура печки1 ЗГ нестабильна
#define MO_HEAT2_ERR 21 //	 0 - температура печки2 ЗГ стабильна; 1 - температура печки2 ЗГ нестабильна
#define SYS_HEAT_ERR 22 //	 0 - нагреватель ТЭНП исправен 1 - неисправность нагревателя ТЭНП"


#define PULD_CONNECT_ERROR_TIME 20

////

void PULD_checkConnectTime(t_PULD *puld);
void PULD_checkMsg(t_PULD *puld);
void PULD_getStatus(t_PULD *puld);
void PULD_init(t_PULD *puld, UART_HandleTypeDef *huart);
void PULD_sendMessage(t_PULD *puld, uint8_t *data, uint8_t dataSize);


#endif /* INC_PULD_H_ */
