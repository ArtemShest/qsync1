/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "controlSystem.h"
#include "w5100s.h"
#include "PULD.h"
#include "controlSystem.h"
#include "ds18b20.h"
#include "dht22.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
//#define TIME_STAB 60
//#define TIME_TEMP_STAB 60

#define TIME_STAB 0
#define TIME_TEMP_STAB 0
/////////////
#define BUFFER_SIZE 20
/////////////
//#define TIME_ERROR 1800
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

W5100s PPU1;
W5100s PPU2;
t_PULD PULD;
t_CS CS;

TEMP_WATER temp_water;
TEMP_AIR temp_air;

uint8_t WorkPeriodFlag; // взводится раз в секунду таймером

uint32_t count_temp_stab = 0;
uint32_t count_stab = 0;
uint32_t count_temp_error = 20;

uint8_t flag_temp_stab = 0;
uint8_t flag_stab = 0;

uint8_t mainStatus[12] = {0, };



////////////////////////////////////
uint8_t receive_buff[20];
////////////////////////////////////
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SPI1_Init();
  MX_SPI2_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_TIM17_Init();
  MX_TIM14_Init();
  MX_TIM16_Init();
  MX_TIM1_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */


  config_PPU_S();

  PULD_init(&PULD, &huart1);

  ds18b20_init();
  dht22_init();

  CS_init(&CS, &huart2);

	HAL_TIM_Base_Start_IT(&htim1); // запуск таймера

	__HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);

	HAL_UART_Receive_DMA(&huart2, (uint8_t*)receive_buff, 20);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  if(WorkPeriodFlag == 1)
	  {

		  WorkPeriodFlag = 0;

		  check_MainStatus(&CS); // проверяем статус зг и системы управления в целом


		  w5100s_readMsg(&PPU1); 	// есть ли ответ от блока питания накачки 1
		  w5100s_readMsg(&PPU2);	// есть ли ответ от блока питания накачки 2


		  // -------- temp water -----------////
		  tempWater_getData();
		  // -------- temp water -----------////

		  // -------------- temp air -----------//
		  tempAir_getData();
		  // -------------- temp air -----------//



		  // ---------- обработка времени общей стабилизации ---- //
		  if(count_stab >= TIME_STAB)
		  {
			  flag_stab = 1; // общая стабилизация ОК
		  }
		  count_stab++; // считаем каждую секунду

		  // ------ проверяем, есть ли коннект, если нет, взводим бит ошибки--------//
		  PULD_checkConnectTime(&PULD);

		  w5100s_checkConnectTime(&PPU1);
		  w5100s_checkConnectTime(&PPU1);


		  w5100s_getStatus(&PPU1); 	// отправляем запрос статуса на блок питания накачки 1
		  w5100s_getStatus(&PPU2);	// отправляем запрос статуса на блок питания накачки 2

		  PULD_getStatus(&PULD);	// отправляем запрос статуса на ЗГ

		  /*	проверка на наличие ошибок датчиков, общей стабилизации, общей ошибки системы управления
		   *
		  if ((!flag_stab || CS.error || temp_air.hum_error ||
		  				temp_air.temp_error || temp_water.error) && ((CS.mode == 1) || (CS.mode == 2)))
		  {
			  CS_changeMode(0); // переход в режим 0, если есть ошибки
		  }

		  */

		  changeLeds(); 	// поменять светодиоды в зависимости от текущего режима

	  }
	  CS_Pulse_On(); 			// включить импульсы блоков накачки через 6 сек, если была команда включения питания
	  PULD_checkMsg(&PULD);		//  проверяем, был ли ответ от ЗГ
	  CS_checkMsg(&CS);			// проверяем, была ли команда от АРМ

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 16;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

void changeLeds()
{
	if (CS.mode == 0) // дежурный
	{
		  HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, SET);
		  HAL_GPIO_WritePin(LED_YELLOW_GPIO_Port, LED_YELLOW_Pin, RESET);
		  HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, RESET);
	}
	else if (CS.mode == 1) // подготовка
	{
		  HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, RESET);
		  HAL_GPIO_WritePin(LED_YELLOW_GPIO_Port, LED_YELLOW_Pin, SET);
		  HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, RESET);
	}
	else if (CS.mode == 2) // рабочий
	{
		  HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, RESET);
		  HAL_GPIO_WritePin(LED_YELLOW_GPIO_Port, LED_YELLOW_Pin, RESET);
		  HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, SET);
	}

}

void config_PPU_S()
{
	 w5100s_init(&hspi2, &PPU1,
	  		  ETH1_MISO_GPIO_Port, ETH1_MISO_Pin,
	  		  ETH1_MOSI_GPIO_Port, ETH1_MOSI_Pin,
	  		  ETH1_RST_GPIO_Port, ETH1_RST_Pin,
	  		  ETH1_SCK_GPIO_Port, ETH1_SCK_Pin,
	  		  ETH1_INT_GPIO_Port, ETH1_INT_Pin,
	  		  ETH1_CS_GPIO_Port, ETH1_CS_Pin);

	    w5100s_init(&hspi1, &PPU2,
	  		  ETH2_MISO_GPIO_Port, ETH2_MISO_Pin,
	  		  ETH2_MOSI_GPIO_Port, ETH2_MOSI_Pin,
	  		  ETH2_RST_GPIO_Port, ETH2_RST_Pin,
	  		  ETH2_SCK_GPIO_Port, ETH2_SCK_Pin,
	  		  ETH2_INT_GPIO_Port, ETH2_INT_Pin,
	  		  ETH2_CS_GPIO_Port, ETH2_CS_Pin);


		// ip 192.168.10.5
	    PPU1.dest_ip[0] = 192; PPU1.dest_ip[1] = 168; PPU1.dest_ip[2] = 10; PPU1.dest_ip[3] = 5;
		// port 60 000
	    PPU1.dest_port[0] = 0xEA; PPU1.dest_port[1] = 0x60;

		// ip 192.168.10.6
		PPU2.dest_ip[0] = 192; PPU2.dest_ip[1] = 168; PPU2.dest_ip[2] = 10; PPU2.dest_ip[3] = 6;
		// port 60 000
		PPU2.dest_port[0] = 0xEA; PPU2.dest_port[1] = 0x60;


		w5100s_setDest(&PPU1);
		w5100s_setDest(&PPU2);
}

void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin)
{	// смотрим, какой синхроимпульс пришел
	if(GPIO_Pin == SYNC_MO_IN_Pin)
	{
		HAL_GPIO_WritePin(SYNC_MO_OUT_GPIO_Port, SYNC_MO_OUT_Pin, SET);
		HAL_TIM_Base_Start_IT(&htim14); // запускаем таймер 14
	}
	else if(GPIO_Pin == SYNC_OG_IN_Pin)
	{
		HAL_GPIO_WritePin(SYNC_OG_OUT_GPIO_Port, SYNC_OG_OUT_Pin, SET);
		HAL_TIM_Base_Start_IT(&htim16); // запускаем таймер 16
	}
	else if(GPIO_Pin == SYNC_PA_IN_Pin)
	{
		HAL_GPIO_WritePin(SYNC_PA_OUT_GPIO_Port, SYNC_PA_OUT_Pin, SET);
		HAL_TIM_Base_Start_IT(&htim17); // запускаем таймер 17
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{

	if(htim == &htim14) // блок зг 5 мкс
	{
		HAL_GPIO_WritePin(SYNC_MO_OUT_GPIO_Port, SYNC_MO_OUT_Pin, RESET);
		HAL_TIM_Base_Stop_IT(&htim14);
	}
	else if(htim ==  &htim16) // генерация 5мкс
	{
		HAL_GPIO_WritePin(SYNC_OG_OUT_GPIO_Port, SYNC_OG_OUT_Pin, RESET);
		HAL_TIM_Base_Stop_IT(&htim16);
	}
	else if(htim == &htim17) // блок накачки 250 мкс
	{
		HAL_GPIO_WritePin(SYNC_PA_OUT_GPIO_Port, SYNC_PA_OUT_Pin, RESET);
		HAL_TIM_Base_Stop_IT(&htim17);
	}
}





void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart == CS.uart)
	{
		//CS.newMsg = 1;
		//HAL_UART_Receive_IT(CS.uart, CS.buff, 6);
	}

	if(huart == PULD.uart)
	{
		PULD.newMsg = 1;
		HAL_UART_Receive_IT(PULD.uart, PULD.buff, 6);
	}
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{

}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
