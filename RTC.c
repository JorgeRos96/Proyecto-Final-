/**
  ******************************************************************************
  * @file    Templates/Src/RTC.c 
  * @author  MCD Application Team
  * @brief   Fichero de inicialización  y configuración del RTC con el formato 
	*					 24 horas.  
	*
  * @note    modified by ARM
  *          The modifications allow to use this file as User Code Template
  *          within the Device Family Pack.
  ******************************************************************************
  *
  ******************************************************************************
  */

#include "RTC.h"
#include "mktime.h"


RTC_HandleTypeDef hrtc;
RTC_DateTypeDef sDate; 
RTC_TimeTypeDef sTime;

#define RTCCIF	0
#define RTCALF	1
#define s		0
#define mi	0
#define h		0
#define d		1
#define m		1
#define y		2020 //desde 1970
#define dw		1
#define wy		1

/**
	* @brief Función que resetea la hora del RTC y establece las 00:00:00 del 01/01/2020
	* @param None
  * @retval None
  */
void resetHoraFecha0(void){
	setHora(s,mi,h);
	setFecha(d,m,y);
}
/**
	* @brief Función que establece la hora del RTC con los parametros que se pasan a la función
	* @param seg: Segundos que se quieren establecer en el RTC
	*	@param min: Minutos que se quieren establecer en el RTC
	*	@param hor: Hora que se quiere establecer en el RTC	
  * @retval None
  */
void setHora(uint8_t seg, uint8_t min, uint8_t hor){
		

	sTime.Hours = hor; // set hours 
  sTime.Minutes = min; // set minutes
	sTime.Seconds = seg; // set seconds	
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE; 
  sTime.StoreOperation = RTC_STOREOPERATION_RESET; 
	if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK) 
  { 
		printf("Fallo al establecer la hora");
  } 

}
/**
	* @brief Función que establece la fecha del RTC con los parametros que se pasan a la función
	* @param dia: Día que se quiere establecer en el RTC
	*	@param mes: Mes que se quiere establecer en el RTC
	*	@param anio: Año que se quiere establecer en el RTC	
  * @retval None
  */
void setFecha(uint8_t dia, uint8_t mes, uint16_t anio ){
	
  sDate.Month = mes; // month 
  sDate.Date = dia; // date 
  sDate.Year = anio; // year 
	if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK) 
  { 
    printf("Fallo al establecer la fecha");
  }
}
/**
	* @brief Función que obtiene el valor de los segundos del RTC
	* @param 
  * @retval Devuelve valor de los segundos del RTC
  */
uint8_t getSeg(void){
	uint8_t seg = 0;
	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
	seg = sTime.Seconds;
	
	return seg;
}
/**
	* @brief Función que obtiene el valor de los minutos del RTC
	* @param 
  * @retval Devuelve valor de los minutos del RTC
  */
uint8_t getMin(void){
	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
  return sTime.Minutes;
}
/**
	* @brief Función que obtiene el valor de la hora del RTC
	* @param 
  * @retval Devuelve valor de la hora del RTC
  */
uint8_t getHora(void){
	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
  return sTime.Hours;
}
/**
	* @brief Función que obtiene el día del RTC
	* @param 
  * @retval Devuelve el día del RTC
  */
uint8_t getDia(void){
	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
  return sDate.Date;
}
/**
	* @brief Función que obtiene el mes del RTC
	* @param 
  * @retval Devuelve el mes del RTC
  */
uint8_t getMes(void){
	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
  return sDate.Month;
}
/**
	* @brief Función que obtiene el año del RTC
	* @param 
  * @retval Devuelve el año del RTC
  */
uint16_t getAnio(void){
	uint16_t ye;
	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
	ye = sDate.Year;
  return ye;
}
//devuelves segundos pasados desde 1 de enero de 1970
/**
	* @brief Función que obtiene el valor de los segundos pasados desde el 1 de Enero de 1970
	* @param 
  * @retval Devuelve valor de los seundos pasados desde el 1/1/1970
  */
uint64_t getTotalSeconds(void){
	uint64_t segundos;
	segundos=unix_time_in_seconds(getSeg(), getMin(), getHora(), getDia(), getMes(), getAnio()); 
	return segundos;
}
/**
	* @brief Función que inicializa el RTC con el formato 24h
	* @param None
  * @retval None
  */
void MX_RTC_Init(void)
{
  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    printf("Fallo en la inicializacion");
  }
  /** Enable Calibrartion
  */
  if (HAL_RTCEx_SetCalibrationOutPut(&hrtc, RTC_CALIBOUTPUT_1HZ) != HAL_OK)
  {
        printf("Fallo en la calibracion");

  }

}

