/**
  ******************************************************************************
  * @file    Templates/Src/HTTP_Server.c 
  * @author  MCD Application Team
  * @brief   Fichero donde se realiza la gestión de los hilos. En ellos se realiza
	*					 el manejo de los LEDs del microcontrolador, el control de la escritura
	*					 por pantall, el control de la hora en el RTC y el envío de la hora 
	*					 actualizada a traves de la USART.
	*
  * @note    modified by ARM
  *          The modifications allow to use this file as User Code Template
  *          within the Device Family Pack.
  ******************************************************************************
  *
  ******************************************************************************
  */
#include <stdio.h>

#include "main.h"
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
#include "Watchdog.h"

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
osThreadId_t TID_USART;
osTimerId_t Timer_USART;
													 
/* Thread declarations */
static void BlinkLed (void *arg);
static void Display  (void *arg);
static void Timer_USART_callback(void *args);
int getNumber(char caracter);

__NO_RETURN void app_main (void *arg);
__NO_RETURN static void Rtc_setTime  (void *arg);
__NO_RETURN static void Rtc_setDate  (void *arg);
__NO_RETURN static void USART_thread(void*args);

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




/**
  * @brief Hilo de gestión de la pantalla. Inicialmente se representa el comienzo del programa. 
	*				 Una vez iniciado representa la IP del servidor web en la pantalla y espera a que se
	*				 se actualice desde la página web lo que se desea representar.
	* @param arg
  * @retval None
  */
static __NO_RETURN void Display (void *arg) {
   static uint8_t ip_addr[NET_ADDR_IP6_LEN];
  static char    ip_ascii[40];
  static char    buf[20+1];
	uint32_t flag;
  GPIO_INIT();
	LCD_reset();
	lcd_clean();
  sprintf (lcd_text[0], "Starting...");
  sprintf (lcd_text[1], "Now");
	actualizar(lcd_text);
	osDelay(1000); 
	/* Representa la IP del servidor web*/
	netIF_GetOption (NET_IF_CLASS_ETH,netIF_OptionIP4_Address, ip_addr, sizeof(ip_addr));
	sprintf (buf, "IP4:%-16s",ip_ascii);
	sprintf (lcd_text[0],"IP address:");
	sprintf (lcd_text[1],"%s", netIP_ntoa (NET_ADDR_IP4, ip_addr, ip_ascii, sizeof(ip_ascii)));
	actualizar(lcd_text);
  while(1) {
    /* Se espera a que se actualice la pantalla desde el servidor web */
    flag = osThreadFlagsWait (0x01U, osFlagsWaitAny, 100);
		/*Si se actualiza la pantalla desde la página web*/
		if (flag == 0x01){
			lcd_clean();
			/*Se representa lo que se envía desde la página web*/
			actualizar(lcd_text);
			/*Se envía un mensaje a traves de la USARt indicando que se ha escrito en la pantalla desde la página web*/
			size = sprintf(buf,"\rSe escribe en pantalla\n");
			tx_USART(buf, size);
		}
		reset_Watchdog();
  }
}

/**
  * @brief Hilo de gestión de los LEDs del microcontrolador según lo que se configura desde la página web. Además
	*				 se realiza el manejo del RGB en función de la temperatura muestra un color.
	*						- Si t<25 se enciende de color azul
	*						- Si 25<t<30 se endiende de amarillo
	*						- Si t>30 se enciende de color rojo
	* @param arg 
  * @retval None
  */
static __NO_RETURN void BlinkLed (void *arg) {
  const uint8_t led_val[6] = { 0x1,0x3,0x6,0x4,0x6,0x3};
  uint32_t cnt = 0U;

  (void)arg;

  LEDrun = true;
  while(1) {
    /* Si se encuentra activo el modo running lights se realiza el encendido de un LED cada 100 ms*/
    if (LEDrun == true) {
      LED_SetOut (led_val[cnt]);
      if (++cnt >= sizeof(led_val)) {
        cnt = 0U;
      }
    }
		/*Si la temperatura es superior a los 30ºC*/
		if(leerTemperatura() > 30){
			/*Se enciende el RGB de color rojo*/
			apagar_LED_azul();
			apagar_LED_verde();
			encender_LED_rojo(40000);
		}
		/*Si la temperatura es superior a los 25ºC y menor de 30 ºC*/
		else if (leerTemperatura() > 25) {					
			/*Se enciende el RGB de color amarillo*/
			encender_LED_verde(40000);
			encender_LED_rojo(40000);		
			apagar_LED_azul();
		}	
		/*Si la temperatura es menor de 25ºC*/
		else{
			/*Se enciende el RGB de color azul*/			
			apagar_LED_rojo();
			apagar_LED_verde();
			encender_LED_azul(40000);			
		}
    osDelay (100);
		reset_Watchdog();
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
	uint32_t flag;
	
	while(1){

		/*Se espera a recibir el flag de que se ha establecido la hora desde la página web*/
		flag = osThreadFlagsWait (0x20U, osFlagsWaitAny, 100);
		
		if (flag == 0x20){
			/*Establece la hora en el RTC*/
			sprintf (date, "%s",time_text[0]);
			date[2] = date[5] = '\0';
			hor=10*getNumber(date[0])+getNumber(date[1]);
			if(hor >24 || hor<1)
				hor = 1;
			min=10*getNumber(date[3])+getNumber(date[4]);
			if(min >59|| min<1)
				min = 0;
			seg=10*getNumber(date[6])+getNumber(date[7]);
			if(seg >59|| seg<1)
				seg = 0;
			setHora(seg, min, hor);
		}
		reset_Watchdog();
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
	uint32_t flag;
	
	while(1){

		/*Se espera a recibir el flag de que se ha establecido la fecha desde la página web*/
		flag = osThreadFlagsWait (0x20U, osFlagsWaitAny, 100);
				
		if (flag == 0x20){
			/*Establece la fecha en el RTC*/
			sprintf (date, "%s",time_text[1]);
			date[2] = date[5] = '\0';
			dia=10*getNumber(date[0])+getNumber(date[1]);
			if (dia > 31)
				dia = 1;
			mes=10*getNumber(date[3])+getNumber(date[4]);
			if (mes > 12 || mes < 1)
				mes = 1;
			anio=10*getNumber(date[8])+getNumber(date[9]);			
			setFecha(dia, mes, anio);
		}
		reset_Watchdog();
	}
}


/**
  * @brief Función de callback del Timer que envía una señal para que se envíe la hora a traves de la USART.
	* @param arg
  * @retval None
  */
static  void Timer_USART_callback(void *args){

	osThreadFlagsSet (TID_USART, 0x04);
}

/**
  * @brief Función que espera al envío de la señal desde el Timer para enviar la hora a traves de la USART al terminal.
	* @param arg
  * @retval None
  */
static __NO_RETURN void USART_thread(void *args){
	uint32_t flag;
	
	while(1) {
		flag = osThreadFlagsWait (0x04U, osFlagsWaitAny, 100);
		if (flag == 0x04){
			size = sprintf(buf,"\rSon las %d:%d:%d del %d/%d/%d\n", getHora(),getMin(),getSeg(),getDia(),getMes(),getAnio());
			tx_USART(buf, size);
		}
		reset_Watchdog();
  }
}

/**
  * @brief Hilo main donde se incializa n los perifericos como los LEDs, el sensoor de temperatura, el potenciometro, la USART, el RGB y el RTC.
	*				 Ademas se establece la hora incial del RTC y se crean los hilos y el Timer. 
	* @param arg
  * @retval None
  */
__NO_RETURN void app_main (void *arg) {
  (void)arg;
	uint32_t exec1=1U;
	uint32_t time;
	
	/*Inicialización de los LEDs del microcontrolador*/
  LED_Initialize();
	/*Inicialización del sensor de temperatura*/
  initI2c();
	/*Inicialización del potenciometro*/
  ADC1_Init();
	/*Inicialización de la USART*/
	init_USART();
	/*Inicialización del RGB*/
	initRGB();
	/*Inicialización del componente de red*/
  netInitialize ();
	
	/*Inicializació del RTC*/
	MX_RTC_Init();
		
	/*Se establece la hora y fecha incial*/
	setHora(00,20,22);
	setFecha(31,8,21);

	/*Se crean los hilos*/
  TID_Led     = osThreadNew (BlinkLed, NULL, NULL);
  TID_Display = osThreadNew (Display,  NULL, NULL);
	TID_Rtc_setTime		= osThreadNew (Rtc_setTime, 		 NULL, NULL);
	TID_Rtc_setDate		= osThreadNew (Rtc_setDate, 		 NULL, NULL);
	TID_USART				= osThreadNew (USART_thread, NULL, NULL); 

	/*Se crea el Timer para enviar por la USART la fecha y hora cada 1 min*/
	Timer_USART=osTimerNew(Timer_USART_callback, osTimerPeriodic, &exec1, NULL);
	if (Timer_USART != NULL)  {
		time = 60000U; //1min
	  osStatus_t status = osTimerStart(Timer_USART, time);       // start timer
  }	
  osThreadExit();
}


/**
  * @brief Función que transforma el carácter en número.
	* @param caracter: se quiere transformar a número
  * @retval Devuelve el número correspondiente
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
