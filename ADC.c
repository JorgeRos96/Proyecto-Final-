/**
  ******************************************************************************
  * @file    Templates/Src/Potenciometro.c
  * @author  MCD Application Team
  * @brief   Fichero de inicialización y cofiguración del potenciometro 1 de la 					 
	*					 tarjeta de aplicaciones. Se utiliza el ADC 1 canal 3 para obtener 
	*					 el valor del potenciometro. El ADC se configura con una resolución 
	*					 de 12 bits y el pin al que se conecta el potenciometro es:
	*
	*					 - Pin POT1: PA3 
	*						
	*					 La lectura del potenciometro obtiene los valores de entre 0 y 3.3 
	*					 voltios.
  *
  * @note    modified by ARM
  *          The modifications allow to use this file as User Code Template
  *          within the Device Family Pack.
  ******************************************************************************
  * 
  ******************************************************************************
  */

#include "Board_ADC.h"

ADC_HandleTypeDef hadc1;

/**
  * @brief Función de inicialización del canal 3 del ADC1 con una resolucion de 12 bits
  * @param None
  * @retval None
  */
int ADC1_Init(void)
{
  ADC_ChannelConfTypeDef sConfig = {0};

	hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCKPRESCALER_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.NbrOfDiscConversion = 1;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = EOC_SINGLE_CONV;
  HAL_ADC_Init(&hadc1);
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_3;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
	HAL_ADC_ConfigChannel(&hadc1, &sConfig);
	
	return 0;

}

/**
  * @brief Función de lectura del valor del ADC y convierte el valor en la tensión equivalente a la salida del potenciometro 
  * @param None
	* @retval voltaje: Tensión a la salida del potenciometro qu regula la tensión de 3.3 V
  */
int32_t lectura(void)
	{
		HAL_StatusTypeDef status;
		uint32_t conversion = 0;
		
		/*Se inicia la conversión a traves del ADC*/
		HAL_ADC_Start(&hadc1);
				status = HAL_ADC_PollForConversion(&hadc1, 0);
		while (status != HAL_OK)
			status = HAL_ADC_PollForConversion(&hadc1, 0);
		/*Se obtiene el valor a la salida del potenciometro*/
		conversion = (int32_t) HAL_ADC_GetValue(&hadc1);
	
		return conversion;		
}

