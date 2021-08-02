#ifndef __mbedAppBoard_PINOUT_H
#define __mbedAppBoard_PINOUT_H


#include "stm32f429xx.h"
#include "stm32f4xx_hal_gpio.h"


typedef struct {
	GPIO_TypeDef *Port;
	__IO uint16_t IO;
} GPIO;
																							// Arduino Uno V3 CN7 & CN8 connectors TOP to BOTTOM

GPIO XBEE_Tx 			= {GPIOG, GPIO_PIN_9};			//	D0
GPIO XBEE_Rx 			= {GPIOG, GPIO_PIN_14};			//	D1
GPIO XBEE_STATUS 	= {GPIOF, GPIO_PIN_15};			//	D2
GPIO XBEE_NRST 		= {GPIOE, GPIO_PIN_13};			//	D3
GPIO SW_CENTER 		= {GPIOF, GPIO_PIN_14};			//	D4
GPIO LED_RED 			= {GPIOE, GPIO_PIN_11};			//	D5
GPIO SPEAKER 			= {GPIOE, GPIO_PIN_9};			//	D6
GPIO LCD_A0 			= {GPIOF, GPIO_PIN_13};			//	D7
GPIO LED_BLUE 		= {GPIOE, GPIO_PIN_13};			//	D8
GPIO LED_GREEN 		= {GPIOD, GPIO_PIN_15};			//	D9
GPIO LCD_CS_N 		= {GPIOD, GPIO_PIN_14};			//	D10
GPIO LCD_MOSI 		= {GPIOB, GPIO_PIN_5};			//	D11
GPIO LCD_RESET 		= {GPIOA, GPIO_PIN_6};			//	D12
GPIO LCD_SCK 			= {GPIOA, GPIO_PIN_5};			//	D13
GPIO SDA 					= {GPIOB, GPIO_PIN_9};			//	D14
GPIO SCL 					= {GPIOB, GPIO_PIN_8}; 			//	D15

GPIO SW_RIGHT 		= {GPIOF, GPIO_PIN_10};			//	A5
GPIO SW_LEFT 			= {GPIOF, GPIO_PIN_5};			//	A4
GPIO SW_DOWN 			= {GPIOF, GPIO_PIN_3};			//	A3
GPIO SW_UP 				= {GPIOF, GPIO_PIN_2};			//	A2
GPIO POT_2 				= {GPIOC, GPIO_PIN_0};			//	A1
GPIO POT_3 				= {GPIOA, GPIO_PIN_3};			//	A0


#endif