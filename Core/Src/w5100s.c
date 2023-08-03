/*
 * w5100s.c
 *
 *  Created on: Jan 20, 2023
 *      Author: artem
 */

#include "w5100s.h"
#include "main.h"


void w5100s_checkConnectTime(W5100s *w5100s)
{
	if(w5100s->connect_time < 250) // защита от переполнения :)
	{
		w5100s->connect_time++;
	}


	// счетчик connect_time сбрасывается при приеме ответа от ЗГ
	if(w5100s->connect_time > PPU_CONNECT_ERROR_TIME)
	{
		w5100s->connect = 0;
		//w5100s->all_status = (1 << 31);
	}
}


void w5100s_ProcessState(W5100s *w5100s)
{
	uint32_t state = 0;
	uint8_t statusBit = 0;

	for(uint8_t i = 7; i <= 13; i++)
	{
		state |= (((uint32_t)check_bit(w5100s->status_block, i)) << statusBit);
		statusBit++;
	}

	for(uint8_t i = 0; i < 6; i++) // для каждого из 6 каналов
	{
		for(uint8_t j = 4; j <= 7; j++) // для каждого бита ошибки
		{
			if(j==7)
			{
				state |= (((uint32_t)( 1 - check_bit(w5100s->status_channel[i], j))) << statusBit);
			}
			else
			{
				state |= (((uint32_t)check_bit(w5100s->status_channel[i], j)) << statusBit);
			}
			statusBit++;
		}
	}
	w5100s->all_status = state;
}

void w5100s_readMsg(W5100s *w5100s)
{
	w5100s->recieve_msg_size = w5100s_getSizeMsg(w5100s);
	if(w5100s->recieve_msg_size > 0)
	{
		uint16_t offset;
		//uint16_t offset_masked;
		uint16_t start_address;

		offset =  w5100s_readReg(w5100s, REG_S0_RX_RD0) << 8;
		offset += w5100s_readReg(w5100s, REG_S0_RX_RD1);

		//offset_masked &= S0_RX_MASK;
		start_address = RX_BASE_ADDR ; // + offset_masked;

		if(w5100s->recieve_msg_size > 30)
		{
			w5100s_initUDP(w5100s);
			w5100s_setDest(w5100s);
			w5100s->tx_rd = 0;
		}


			for (uint16_t i = 0; i< w5100s->recieve_msg_size; i++)
			{
				w5100s->recieve_msg[i] = w5100s_readReg(w5100s, start_address + i + offset);
			}
			offset += w5100s->recieve_msg_size;
		// }



		w5100s->is_new_command = 1;
		w5100s_writeReg(w5100s, REG_S0_RX_RD0, offset >> 8);
		w5100s_writeReg(w5100s, REG_S0_RX_RD1, offset);

		w5100s_writeReg(w5100s, REG_S0_CR, SOCKET_RECV);

		if(offset >= 8*1024 - w5100s->recieve_msg_size)
		{

			w5100s_initUDP(w5100s);
			w5100s_setDest(w5100s);
			w5100s->tx_rd = 0;
		}

		w5100s_parseMessage(w5100s);
	}
}

void w5100s_parseMessage(W5100s *w5100s)
{
	if (w5100s->recieve_msg[8] == 24) //ответ на запрос статуса:
	{
		w5100s->status_block = w5100s->recieve_msg[11] << 8;
		w5100s->status_block += w5100s->recieve_msg[10];
//		if ((w5100s->status_block & 0x3F80) > 0) w5100s->error = 1;
		for(uint8_t iter = 0; iter<6; iter++ )
		{
			w5100s->status_channel[iter] = w5100s->recieve_msg[12+iter];
//			if(((w5100s->status_channel[iter] & 0xF0) > 0) && (iter != 6)) w5100s->error = 1; //????
		}

		w5100s->connect = 1;
		w5100s->connect_time = 0; // сброс счетчика коннекта

		w5100s_ProcessState(w5100s);
	}
}

void w5100s_setDest(W5100s *w5100s) // задаем целевой адрес
{
	w5100s_writeReg(w5100s, S0_DIPR0, w5100s->dest_ip[0]);
	w5100s_writeReg(w5100s, S0_DIPR1, w5100s->dest_ip[1]);
	w5100s_writeReg(w5100s, S0_DIPR2, w5100s->dest_ip[2]);
	w5100s_writeReg(w5100s, S0_DIPR3, w5100s->dest_ip[3]);

	w5100s_writeReg(w5100s, REG_S0_DPORTR0, w5100s->dest_port[0]);
	w5100s_writeReg(w5100s, REG_S0_DPORTR1, w5100s->dest_port[1]);


}

void w5100s_getStatus(W5100s *w5100s)
{
	uint8_t data[] = {24, 0, 0, 0};
	w5100s_sendMes(w5100s, data, 4);
}

void w5100s_sendMes(W5100s *w5100s, uint8_t *data, uint8_t dataSize)
{

	uint8_t buf_data[dataSize];
	for(int i = 0; i < dataSize; i++)
	{
		buf_data[i] = *data;
		data++;
	}

	// для любого кол-ва байт
	for(uint8_t i = 0; i < dataSize; i++)
	{
		w5100s_writeReg(w5100s, 0x4000 + i + w5100s->tx_rd, buf_data[i]);
	}
	w5100s->tx_rd += dataSize;


	w5100s_writeReg(w5100s, REG_S0_TX_WR0, w5100s->tx_rd >> 8/*data_length >> 8*/);
	w5100s_writeReg(w5100s, REG_S0_TX_WR1, w5100s->tx_rd/*data_length*/);

	if(w5100s->tx_rd >= 8*1024-dataSize)
	{

		w5100s_initUDP(w5100s);
		w5100s_setDest(w5100s);
		w5100s->tx_rd = 0;
	}
	w5100s_writeReg(w5100s, REG_S0_CR, SOCKET_SEND);
}

uint16_t w5100s_getSizeMsg(W5100s* w5100s)
{
	uint16_t dataSize = 0;
	dataSize = (uint16_t)(w5100s_readReg(w5100s, REG_S0_RX_RSR0));
	dataSize <<= 8;
	dataSize += (uint16_t)(w5100s_readReg(w5100s, REG_S0_RX_RSR1));
	return dataSize;
}

uint8_t w5100s_socketState(W5100s* w5100s) //состояние сокета
{
	return w5100s_readReg(w5100s, REG_S0_SR);
}

void w5100s_socketReOpen(W5100s *w5100s)
{
	w5100s_socketClose(w5100s);
	w5100s_socketOpen(w5100s);
}

uint8_t w5100s_readReg(W5100s *w5100s, uint16_t address)
{
	uint8_t buf[] = {W5100_READ, address >> 8, address};
	uint8_t rbyte;
    SS_SELECT(w5100s);
    HAL_SPI_Transmit(w5100s->spi, buf, 3, 1000);
	HAL_SPI_Receive(w5100s->spi, &rbyte, 1, 1000);
	SS_DESELECT(w5100s);
	return rbyte;
}

void SS_SELECT(W5100s *w5100s)
{
	HAL_GPIO_WritePin(w5100s->cs_port, w5100s->cs_pin, GPIO_PIN_RESET);
}

void SS_DESELECT(W5100s *w5100s)
{
	HAL_GPIO_WritePin(w5100s->cs_port, w5100s->cs_pin, GPIO_PIN_SET);
}

void w5100s_writeReg(W5100s *w5100s, uint16_t address, uint8_t data)
{
	uint8_t buf[] = {W5100_WRITE, address >> 8, address, data};
	SS_SELECT(w5100s);
	HAL_SPI_Transmit(w5100s->spi, buf, 4, 0xFFFFFFFF);
	SS_DESELECT(w5100s);
}


void w5100s_setMainBuf(W5100s *w5100s, uint8_t byte)
{

}

void w5100s_init(SPI_HandleTypeDef *hspi, W5100s *w5100s,
		GPIO_TypeDef* miso_port, uint16_t miso_pin,
		GPIO_TypeDef* mosi_port, uint16_t mosi_pin,
		GPIO_TypeDef* rst_port, uint16_t rst_pin,
		GPIO_TypeDef* sck_port, uint16_t sck_pin,
		GPIO_TypeDef* int_port, uint16_t int_pin,
		GPIO_TypeDef* cs_port, uint16_t cs_pin)
{

	w5100s->ipaddr[0] = 192; w5100s->ipaddr[1]=168; w5100s->ipaddr[2]= 10; w5100s->ipaddr[3] = 2;
	w5100s->ipgate[0] = 192; w5100s->ipgate[1]=168; w5100s->ipgate[2]= 10; w5100s->ipgate[3] = 1;
	w5100s->ipmask[0] = 255; w5100s->ipmask[1] = 255; w5100s->ipmask[2] = 255; w5100s->ipmask[3] = 0;
	w5100s->mac_addr[0] = 0x1; w5100s->mac_addr[1] = 0x2; w5100s->mac_addr[2] = 0x3; w5100s->mac_addr[3] = 0x4; w5100s->mac_addr[4] = 0x5; w5100s->mac_addr[5] = 0x6;

	w5100s->local_port = 80;

	w5100s->all_status = (1 << 31);

	w5100s->spi = hspi;
	w5100s->miso_port = miso_port;
	w5100s->miso_pin = miso_pin;

	w5100s->mosi_port = mosi_port;
	w5100s->mosi_pin = mosi_pin;

	w5100s->rst_port = rst_port;
	w5100s->rst_pin = rst_pin;

	w5100s->sck_port = sck_port;
	w5100s->sck_pin = sck_pin;

	w5100s->int_port = int_port;
	w5100s->int_pin = int_pin;

	w5100s->cs_port = cs_port;
	w5100s->cs_pin = cs_pin;

	w5100s_initUDP(w5100s);

}

void w5100s_initUDP(W5100s *w5100s)
{
	//----------------- hard reset ------------------------

	HAL_GPIO_WritePin(w5100s->rst_port, w5100s->rst_pin, RESET);
//	HAL_Delay(1);
	HAL_Delay(10);
	HAL_GPIO_WritePin(w5100s->rst_port, w5100s->rst_pin, SET);
	//-----------------------------------------------------

//	HAL_Delay(65);
	HAL_Delay(100);


	//-------- 1. Mode Register : soft reset --------------
	w5100s_writeReg(w5100s, REG_MR, 0x80); HAL_Delay(1);
	//-----------------------------------------------------

	//---- 2. Interrupt Mask Register : interrupt enable --
	w5100s_writeReg(w5100s, REG_IMR, S0_INT); HAL_Delay(1);
    //-----------------------------------------------------

	//---------- 3. Retry Time-value Register -------------
	w5100s_writeReg(w5100s, REG_RTR0, 0x0f); HAL_Delay(1);
    w5100s_writeReg(w5100s, REG_RTR1, 0xA0); HAL_Delay(1);
    //-----------------------------------------------------

	//---------- 4. Retry Count Register ------------------
	w5100s_writeReg(w5100s, REG_RCR, 0x01); HAL_Delay(1);
    //-----------------------------------------------------

	//---------------- 5. IP-gate address -----------------
	w5100s_writeReg(w5100s, REG_GWR0, w5100s->ipgate[0]); HAL_Delay(1);
	w5100s_writeReg(w5100s, REG_GWR1, w5100s->ipgate[1]); HAL_Delay(1);
	w5100s_writeReg(w5100s, REG_GWR2, w5100s->ipgate[2]); HAL_Delay(1);
	w5100s_writeReg(w5100s, REG_GWR3, w5100s->ipgate[3]); HAL_Delay(1);
	//-----------------------------------------------------

	//---------------- 6. MAC address ---------------------
	w5100s_writeReg(w5100s, REG_SHAR0, w5100s->mac_addr[0]); HAL_Delay(1);
	w5100s_writeReg(w5100s, REG_SHAR1, w5100s->mac_addr[1]); HAL_Delay(1);
	w5100s_writeReg(w5100s, REG_SHAR2, w5100s->mac_addr[2]); HAL_Delay(1);
	w5100s_writeReg(w5100s, REG_SHAR3, w5100s->mac_addr[3]); HAL_Delay(1);
	w5100s_writeReg(w5100s, REG_SHAR4, w5100s->mac_addr[4]); HAL_Delay(1);
	w5100s_writeReg(w5100s, REG_SHAR5, w5100s->mac_addr[5]); HAL_Delay(1);
	//-----------------------------------------------------

	//---------------- 7. Mask address --------------------
	w5100s_writeReg(w5100s, REG_SUBR0, w5100s->ipmask[0]); HAL_Delay(1);
	w5100s_writeReg(w5100s, REG_SUBR1, w5100s->ipmask[1]); HAL_Delay(1);
	w5100s_writeReg(w5100s, REG_SUBR2, w5100s->ipmask[2]); HAL_Delay(1);
	w5100s_writeReg(w5100s, REG_SUBR3, w5100s->ipmask[3]); HAL_Delay(1);
	//-----------------------------------------------------

	//----------------- 8. IP address ---------------------
	w5100s_writeReg(w5100s, REG_SIPR0, w5100s->ipaddr[0]); HAL_Delay(1);
	w5100s_writeReg(w5100s, REG_SIPR1, w5100s->ipaddr[1]); HAL_Delay(1);
	w5100s_writeReg(w5100s, REG_SIPR2, w5100s->ipaddr[2]); HAL_Delay(1);
	w5100s_writeReg(w5100s, REG_SIPR3, w5100s->ipaddr[3]); HAL_Delay(1);
	//-----------------------------------------------------



	//--- 10. RX memory of Socket_0 = 8kB, others = 0kB ---
	w5100s_writeReg(w5100s, REG_RMSR, 0x03); HAL_Delay(1); // RX
	w5100s_writeReg(w5100s, REG_TMSR, 0x03); HAL_Delay(1); // TX


	w5100s_writeReg(w5100s, 0x041E, 0x08); HAL_Delay(1); // RX
	w5100s_writeReg(w5100s, 0x051E, 0); HAL_Delay(1); // RX
	w5100s_writeReg(w5100s, 0x061E, 0); HAL_Delay(1); // RX
	w5100s_writeReg(w5100s, 0x071E, 0); HAL_Delay(1); // RX

	w5100s_writeReg(w5100s, 0x041F, 0x08); HAL_Delay(1); // TX
	w5100s_writeReg(w5100s, 0x051F, 0); HAL_Delay(1); // TX
	w5100s_writeReg(w5100s, 0x061F, 0); HAL_Delay(1); // TX
	w5100s_writeReg(w5100s, 0x071F, 0); HAL_Delay(1); // TX

	//-----------------------------------------------------

	//-------------- 11. Interrupt enable -----------------
	//w5100s_writeReg(REG_S0_IMR, 0x01); HAL_Delay(1);
	//-----------------------------------------------------

	//----------- 12. Setting TCP protocol ----------------
	w5100s_writeReg(w5100s, REG_S0_MR, UDP_MODE); HAL_Delay(1);

	//----------------- 9. Setting port -------------------
	w5100s_writeReg(w5100s, REG_S0_PORT0, 0x13); HAL_Delay(1);
	w5100s_writeReg(w5100s, REG_S0_PORT1, 0x88); HAL_Delay(1);
	//-----------------------------------------------------
	//-----------------------------------------------------

	//---------------- 13. Socket OPEN  -------------------
	w5100s_writeReg(w5100s, REG_S0_CR, SOCKET_OPEN); HAL_Delay(50);
	//-----------------------------------------------------
}
void w5100s_socketOpen(W5100s *w5100s)
{
	w5100s_writeReg(w5100s, REG_S0_CR, SOCKET_OPEN);
}

