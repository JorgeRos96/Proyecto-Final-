/**
  ******************************************************************************
  * @file    Templates/Src/SNTP.c 
  * @author  MCD Application Team
  * @brief   Fichero que contiene las funciones necesarias para obtener el valor
	*					 de la hora y fecha del servidor SNTP y establecerla en el RTC.
	*
  * @note    modified by ARM
  *          The modifications allow to use this file as User Code Template
  *          within the Device Family Pack.
  ******************************************************************************
  *
  ******************************************************************************
  */


#include "SNTP.h"
#include "cmsis_os2.h"                 
#include "rl_net.h"
#include "RTC.h"
#include <time.h>
#include <stdio.h>

const NET_ADDR4 ntp_server = { NET_ADDR_IP4, 0, 130, 206, 3, 166}; //Servidores SNTP	130.206.3.166 	150.214.94.5 193.78.240.12 167.86.76.95
static void time_callback (uint32_t s, uint32_t seconds_fraction);
 

/**
	* @brief Función que sirve para establecer la comunicación con el servidor SNTP. Se realiza a traves de la función
	*				 de CMSIS netSNTPc_GetTime donde se pasa por parametro la dirección IP del servidor SNTP y la función de 
	*				 callback a la que se llama cuando el servidor SNTP envía los datos.
	* @param None
  * @retval None
  */
void get_time(void) {
	
  if (netSNTPc_GetTime ((NET_ADDR *)&ntp_server, time_callback) == netOK) {
   printf ("SNTP request sent.\n");
  }
  else {
   printf ("SNTP not ready or bad parameters.\n");
  }
}
 
/**
	* @brief Función de callback a la que se llama cuando el servidor SNTP envía los datos de los segundos 
	*				 en fotmato UTC y la fracción de los segundos.
	* @param s: segundos en formato UTC que se recibe desde el servidor SNTP
	*	@param seconds_fraction: fracción de segundos que se recibe del servidor SNTP
  * @retval Devuelve valor de los seundos del RTC
  */
 static void time_callback (uint32_t s, uint32_t seconds_fraction) {
	
	uint8_t seg, min, hor, dia, mes;
	uint16_t anio;
	time_t now=s+3600;//Para obtener utc+1
	struct tm *ts;

	/*Si los segundos recibidos del SNTP estan a cero ha habido un fallo en la comunicación*/
	if (s == 0) {
    printf ("Server not responding or bad response.\n");
		/*Se envía la señal para que se pueda volver a realizar la petición al servidor SNTP*/
		osThreadFlagsSet (TID_SNTP, 0x10);
  }
	/*Si se reciben correctamente los segundos se establecen en el RTC*/
	else {
		ts=localtime(&now); 
		seg=ts->tm_sec;
		min= ts->tm_min;
		hor= ts->tm_hour;
		dia= ts->tm_mday;
		mes= (ts->tm_mon)+1;	//Enero =0 
		anio= (ts->tm_year)-100; //Desde 1900;
		setHora(seg,  min,  hor);
		setFecha( dia,  mes,  anio); 
		/*Se envía la señal para que se pueda volver a realizar la petición al servidor SNTP*/
		osThreadFlagsSet (TID_SNTP, 0x10);
  }
}