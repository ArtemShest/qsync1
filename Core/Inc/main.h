/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void config_PPU_S();
void changeLeds();
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define TEMP_AIR_Pin GPIO_PIN_13
#define TEMP_AIR_GPIO_Port GPIOC
#define PT100_Pin GPIO_PIN_1
#define PT100_GPIO_Port GPIOA
#define RS485_TX_Pin GPIO_PIN_2
#define RS485_TX_GPIO_Port GPIOA
#define RS485_RX_Pin GPIO_PIN_3
#define RS485_RX_GPIO_Port GPIOA
#define RS485_DIR_Pin GPIO_PIN_4
#define RS485_DIR_GPIO_Port GPIOA
#define SYNC_PA_IN_Pin GPIO_PIN_5
#define SYNC_PA_IN_GPIO_Port GPIOA
#define SYNC_PA_IN_EXTI_IRQn EXTI4_15_IRQn
#define SYNC_OG_IN_Pin GPIO_PIN_6
#define SYNC_OG_IN_GPIO_Port GPIOA
#define SYNC_OG_IN_EXTI_IRQn EXTI4_15_IRQn
#define HEATER_Pin GPIO_PIN_7
#define HEATER_GPIO_Port GPIOA
#define SYNC_MO_IN_Pin GPIO_PIN_0
#define SYNC_MO_IN_GPIO_Port GPIOB
#define SYNC_MO_IN_EXTI_IRQn EXTI0_1_IRQn
#define SYNC_OG_OUT_Pin GPIO_PIN_1
#define SYNC_OG_OUT_GPIO_Port GPIOB
#define TEMP_WATER_Pin GPIO_PIN_2
#define TEMP_WATER_GPIO_Port GPIOB
#define LED_GREEN_Pin GPIO_PIN_10
#define LED_GREEN_GPIO_Port GPIOB
#define LED_YELLOW_Pin GPIO_PIN_11
#define LED_YELLOW_GPIO_Port GPIOB
#define LED_RED_Pin GPIO_PIN_12
#define LED_RED_GPIO_Port GPIOB
#define SYNC_PA_OUT_Pin GPIO_PIN_8
#define SYNC_PA_OUT_GPIO_Port GPIOA
#define RS232_TX_Pin GPIO_PIN_9
#define RS232_TX_GPIO_Port GPIOA
#define SYNC_MO_OUT_Pin GPIO_PIN_6
#define SYNC_MO_OUT_GPIO_Port GPIOC
#define RS232_RX_Pin GPIO_PIN_10
#define RS232_RX_GPIO_Port GPIOA
#define ETH2_CS_Pin GPIO_PIN_15
#define ETH2_CS_GPIO_Port GPIOA
#define ETH1_CS_Pin GPIO_PIN_0
#define ETH1_CS_GPIO_Port GPIOD
#define ETH1_SCK_Pin GPIO_PIN_1
#define ETH1_SCK_GPIO_Port GPIOD
#define ETH1_RST_Pin GPIO_PIN_2
#define ETH1_RST_GPIO_Port GPIOD
#define ETH1_MISO_Pin GPIO_PIN_3
#define ETH1_MISO_GPIO_Port GPIOD
#define ETH2_SCK_Pin GPIO_PIN_3
#define ETH2_SCK_GPIO_Port GPIOB
#define ETH2_MISO_Pin GPIO_PIN_4
#define ETH2_MISO_GPIO_Port GPIOB
#define ETH2_MOSI_Pin GPIO_PIN_5
#define ETH2_MOSI_GPIO_Port GPIOB
#define ETH2_RST_Pin GPIO_PIN_6
#define ETH2_RST_GPIO_Port GPIOB
#define ETH1_MOSI_Pin GPIO_PIN_7
#define ETH1_MOSI_GPIO_Port GPIOB
#define ETH1_INT_Pin GPIO_PIN_8
#define ETH1_INT_GPIO_Port GPIOB
#define ETH2_INT_Pin GPIO_PIN_9
#define ETH2_INT_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
