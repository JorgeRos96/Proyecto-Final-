/*------------------------------------------------------------------------------
 * MDK Middleware - Component ::Network
 * Copyright (c) 2004-2019 Arm Limited (or its affiliates). All rights reserved.
 *------------------------------------------------------------------------------
 * Name:    HTTP_Server.c
 * Purpose: HTTP Server example
 *----------------------------------------------------------------------------*/

#include <stdio.h>

#include "main.h"
#include "SNTP.h"
#include "RTC.h"
#include "rl_net.h"                     // Keil.MDK-Pro::Network:CORE
#include <time.h>
#include "stm32f4xx_hal.h"              // Keil::Device:STM32Cube HAL:Common
#include "Board_LED.h"                  // ::Board Support:LED
#include "Temp.h"
#include "Board_ADC.h"                  // ::Board Support:A/D Converter
#include "LCD.h"                 // ::Board Support:Graphic LCD
#include "USART.h"
#include "RGB.h"

// Main stack size must be multiple of 8 Bytes
#define APP_MAIN_STK_SZ (1024U)
uint64_t app_main_stk[APP_MAIN_STK_SZ / 8];
const osThreadAttr_t app_main_attr = {
  .stack_mem  = &app_main_stk[0],
  .stack_size = sizeof(app_main_stk)
};

extern char time_text[2][20+1];
char time_text[2][20+1] = { "XX:XX:XX",
                           "XX/XX/XXXX" };
extern uint16_t AD_in          (uint32_t ch);
extern void     netDHCP_Notify (uint32_t if_num, uint8_t option, const uint8_t *val, uint32_t len);

extern bool LEDrun;
extern char lcd_text[2][20+1];

extern osThreadId_t TID_Display;
extern osThreadId_t TID_Led;


char buf[100];
int size = 0;
bool LEDrun;
char lcd_text[2][20+1] = { "LCD line 1",
                           "LCD line 2" };

/* Thread IDs */
osThreadId_t TID_Display;
osThreadId_t TID_Led;
osThreadId_t TID_Rtc_setTime;
osThreadId_t TID_Rtc_setDate;
osThreadId_t TID_SNTP;
osTimerId_t Timer_SNTP;
													 
/* Thread declarations */
static void BlinkLed (void *arg);
static void Display  (void *arg);
static void Timer_SNTP_callback(void *args);
int getNumber(char caracter);

__NO_RETURN void app_main (void *arg);
__NO_RETURN static void Rtc_setTime  (void *arg);
__NO_RETURN static void Rtc_setDate  (void *arg);
__NO_RETURN static void SNTP_thread(void*args);

/* Read analog inputs */
uint16_t AD_in (uint32_t ch) {
  int32_t val = 0;

  if (ch == 0) {
    val = lectura();
  }
  return ((uint16_t)val);
}

float temperatura(){
	float tem;
	
	tem = leerTemperatura();

	return tem; 
}


/* IP address change notification */
void netDHCP_Notify (uint32_t if_num, uint8_t option, const uint8_t *val, uint32_t len) {

  (void)if_num;
  (void)val;
  (void)len;

  if (option == NET_DHCP_OPTION_IP_ADDRESS) {
    /* IP address change, trigger LCD update */
		sprintf (lcd_text[0],"IP address:");
    sprintf (lcd_text[1],"%s", val);
    osThreadFlagsSet (TID_Display, 0x01);
  }
}

/*----------------------------------------------------------------------------
  Thread 'Display': LCD display handler
 *---------------------------------------------------------------------------*/
static __NO_RETURN void Display (void *arg) {
   static uint8_t ip_addr[NET_ADDR_IP6_LEN];
  static char    ip_ascii[40];
  static char    buf[20+1];
  GPIO_INIT();
	LCD_reset();
	lcd_clean();
  sprintf (lcd_text[0], "Starting...");
  sprintf (lcd_text[1], "Now");
	actualizar(lcd_text);
	 /* Retrieve and print IPv4 address */
	osDelay(1000); 
	netIF_GetOption (NET_IF_CLASS_ETH,netIF_OptionIP4_Address, ip_addr, sizeof(ip_addr));
	sprintf (buf, "IP4:%-16s",ip_ascii);
	sprintf (lcd_text[0],"IP address:");
	sprintf (lcd_text[1],"%s", netIP_ntoa (NET_ADDR_IP4, ip_addr, ip_ascii, sizeof(ip_ascii)));
	actualizar(lcd_text);
  while(1) {
    /* Wait for signal from DHCP */
    osThreadFlagsWait (0x01U, osFlagsWaitAny, osWaitForever);
		lcd_clean();
		actualizar(lcd_text);
		size = sprintf(buf,"\rSe escribe en pantalla\n");
		tx_USART(buf, size);
  }
}

/*----------------------------------------------------------------------------
  Thread 'BlinkLed': Blink the LEDs on an eval board
 *---------------------------------------------------------------------------*/
static __NO_RETURN void BlinkLed (void *arg) {
  const uint8_t led_val[6] = { 0x1,0x3,0x6,0x4,0x6,0x3};
  uint32_t cnt = 0U;

  (void)arg;

  LEDrun = true;
  while(1) {
    /* Every 100 ms */
    if (LEDrun == true) {
      LED_SetOut (led_val[cnt]);
      if (++cnt >= sizeof(led_val)) {
        cnt = 0U;
      }
    }
		if(temperatura() > 30){
			apagar_LED_azul();
			encender_LED_rojo(40000);
		}
		else {
			apagar_LED_rojo();
			encender_LED_azul(40000);			
		}
    osDelay (100);
  }
}

/**
  * @brief Hilo de gestión de la hora donde se espera a la actualzación de la hora desde la página web y
	*				 se establece en el RTC.
	* @param arg
  * @retval None
  */
static __NO_RETURN void Rtc_setTime (void *arg) {

	char date[8];
	uint8_t   min, hor ;
	int seg;
	while(1){

		/*Se espera a recibir el flag de que se ha establecido la hora desde la página web*/
		osThreadFlagsWait (0x20U, osFlagsWaitAny, osWaitForever);
		
		/*Establece la hora en el RTC*/
		sprintf (date, "%s",time_text[0]);
		date[2] = date[5] = '\0';
		hor=10*getNumber(date[0])+getNumber(date[1]);
		min=10*getNumber(date[3])+getNumber(date[4]);
		seg=10*getNumber(date[6])+getNumber(date[7]);
		setHora(seg, min, hor);
	}
	

}

/**
  * @brief Hilo de gestión de la fecha donde se espera a la actualzación de la fecha desde la página web y
	*				 se establece en el RTC.
	* @param arg
  * @retval None
  */
static __NO_RETURN void Rtc_setDate (void *arg) {

	char date[10];
	uint8_t  dia, mes;
	uint16_t anio;
	while(1){

		/*Se espera a recibir el flag de que se ha establecido la fecha desde la página web*/
		osThreadFlagsWait (0x20U, osFlagsWaitAny, osWaitForever);
				
		/*Establece la fecha en el RTC*/
		sprintf (date, "%s",time_text[1]);
		date[2] = date[5] = '\0';
		dia=10*getNumber(date[0])+getNumber(date[1]);
		mes=10*getNumber(date[3])+getNumber(date[4]);
		anio=10*getNumber(date[8])+getNumber(date[9]);
		setFecha(dia, mes, anio);
	}
}


/**
  * @brief Función de callback del Timer que envía una señal para que se obtenga la hora del servidor SNTP
	* @param arg
  * @retval None
  */
static  void Timer_SNTP_callback(void *args){

	osThreadFlagsSet (TID_SNTP, 0x04);
}

/**
  * @brief Función que espera al envío de la señal desde el Timer para realizar la comunicación con el servidor SNTP 
	*				 para recibir la hora.
	* @param arg
  * @retval None
  */
static __NO_RETURN void SNTP_thread(void *args){

	while(1) {
		osThreadFlagsWait (0x04U, osFlagsWaitAny, osWaitForever);
//		size = sprintf(buf,"\rSon las %d:%d:%d del %d/%d/%d\n", getHora(),getMin(),getSeg(),getDia(),getMes(),getAnio());
//		tx_USART(buf, size);
  }
}

/*----------------------------------------------------------------------------
  Main Thread 'main': Run Network
 *---------------------------------------------------------------------------*/
__NO_RETURN void app_main (void *arg) {
  (void)arg;
	uint32_t exec1=1U;
	uint32_t time;
	
  LED_Initialize();
  initI2c();
  ADC1_Init();
	init_USART();
	initRGB();
  netInitialize ();
	
	/*Inicializació del RTC*/
	MX_RTC_Init();
		
	/*Se establece la hora y fecha incial*/
	setHora(00,20,22);
	setFecha(31,8,21);

  TID_Led     = osThreadNew (BlinkLed, NULL, NULL);
  TID_Display = osThreadNew (Display,  NULL, NULL);
	TID_Rtc_setTime		= osThreadNew (Rtc_setTime, 		 NULL, NULL);
	TID_Rtc_setDate		= osThreadNew (Rtc_setDate, 		 NULL, NULL);
	TID_SNTP				= osThreadNew (SNTP_thread, NULL, NULL); 

	/*Se crea el Timer para enviar por pantalla la hora cada 1min*/
	Timer_SNTP=osTimerNew(Timer_SNTP_callback, osTimerPeriodic, &exec1, NULL);
	 if (Timer_SNTP != NULL)  {
    time = 10000U; //10s 
	  osStatus_t status = osTimerStart(Timer_SNTP, time);       // start timer
  }	
	 osThreadFlagsWait (0x80U, osFlagsWaitAny, 30000);//Esperamos 13 segundoss
	 osThreadFlagsSet (TID_SNTP, 0x04);//mandamos señal para actualizar hora del sntp

  osThreadExit();
}


/**
  * @brief Función que transforma el carácter en número.
	* @param arg
  * @retval None
  */
int getNumber(char caracter){
	int ret=0;
	switch (caracter){
		case '1':
			ret= 1;
			break;
		case '2':
			ret= 2;
			break;
		case '3':
			ret= 3;
				break;
		case '4':
			ret= 4;
				break;
		case '5':
			ret= 5;
				break;
		case '6':
			ret= 6;
				break;
		case '7':
			ret= 7;
				break;
		case '8':
			ret= 8;	
				break;
		case '9':
			ret= 9;			
				break;
		default: 
			ret= 0;
			break;
	}
	return ret;
}
