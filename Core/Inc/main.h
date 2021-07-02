/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
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
#include "stm32f0xx_hal.h"

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

#define SOURCE1 0
#define SOURCE2 1

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define Relay_AUX2_Pin GPIO_PIN_13
#define Relay_AUX2_GPIO_Port GPIOC
#define LED_S1_Pin GPIO_PIN_0
#define LED_S1_GPIO_Port GPIOA
#define LED_S2_Pin GPIO_PIN_1
#define LED_S2_GPIO_Port GPIOA
#define LED_HEALTY_Pin GPIO_PIN_2
#define LED_HEALTY_GPIO_Port GPIOA
#define Relay_AUX1_Pin GPIO_PIN_3
#define Relay_AUX1_GPIO_Port GPIOA
#define SPI1_CS1_Pin GPIO_PIN_4
#define SPI1_CS1_GPIO_Port GPIOA
#define LED_S2ON_Pin GPIO_PIN_4
#define LED_S2ON_GPIO_Port GPIOC
#define LED_Auto_Pin GPIO_PIN_5
#define LED_Auto_GPIO_Port GPIOC
#define LED_Manual_Pin GPIO_PIN_0
#define LED_Manual_GPIO_Port GPIOB
#define LED_S1ON_Pin GPIO_PIN_1
#define LED_S1ON_GPIO_Port GPIOB
#define LED_Fault_Pin GPIO_PIN_2
#define LED_Fault_GPIO_Port GPIOB
#define SPI2_CS_Pin GPIO_PIN_11
#define SPI2_CS_GPIO_Port GPIOB
#define LCD_D1_Pin GPIO_PIN_12
#define LCD_D1_GPIO_Port GPIOB
#define LCD_D2_Pin GPIO_PIN_13
#define LCD_D2_GPIO_Port GPIOB
#define LCD_D3_Pin GPIO_PIN_14
#define LCD_D3_GPIO_Port GPIOB
#define LCD_D4_Pin GPIO_PIN_15
#define LCD_D4_GPIO_Port GPIOB
#define LCD_E_Pin GPIO_PIN_6
#define LCD_E_GPIO_Port GPIOC
#define LCD_RS_Pin GPIO_PIN_7
#define LCD_RS_GPIO_Port GPIOC
#define LCD_BL_Pin GPIO_PIN_8
#define LCD_BL_GPIO_Port GPIOC
#define Buzzer_Pin GPIO_PIN_9
#define Buzzer_GPIO_Port GPIOC
#define RS485_Direction_Pin GPIO_PIN_8
#define RS485_Direction_GPIO_Port GPIOA
#define Digital_In1_Pin GPIO_PIN_15
#define Digital_In1_GPIO_Port GPIOA
#define Digital_In2_Pin GPIO_PIN_10
#define Digital_In2_GPIO_Port GPIOC
#define Digital_In3_Pin GPIO_PIN_11
#define Digital_In3_GPIO_Port GPIOC
#define Digital_In4_Pin GPIO_PIN_12
#define Digital_In4_GPIO_Port GPIOC
#define SW_Option1_Pin GPIO_PIN_2
#define SW_Option1_GPIO_Port GPIOD
#define SW_Option2_Pin GPIO_PIN_3
#define SW_Option2_GPIO_Port GPIOB
#define btn_MODE_Pin GPIO_PIN_4
#define btn_MODE_GPIO_Port GPIOB
#define btn_EN_Pin GPIO_PIN_5
#define btn_EN_GPIO_Port GPIOB
#define btn_UP_Pin GPIO_PIN_6
#define btn_UP_GPIO_Port GPIOB
#define btn_DW_Pin GPIO_PIN_7
#define btn_DW_GPIO_Port GPIOB
#define SOURCE1_Pin GPIO_PIN_8
#define SOURCE1_GPIO_Port GPIOB
#define SOURCE2_Pin GPIO_PIN_9
#define SOURCE2_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

#define RLY_GENS_Port Relay_AUX1_GPIO_Port
#define RLY_GENS_Pin	Relay_AUX1_Pin


/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
