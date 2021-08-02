/**
  ******************************************************************************
  * @file    Templates/Src/RGB.c
  * @author  MCD Application Team
  * @brief   Fichero de inicializacion y configuración del LED RGB de la tarjeta 
	*					 de aplicaciones. En esta librería se encuentran las funciones necesarias
	*					 para encender/apagar y variar la intensidad del RGB.
	*					 Para utilizar el LED RGB se emplea una señal PWM que se genera a partir
	*					 de un Timer.
	*
	*					 Se han configurado los pines de la siguiente manera:
	*					 - LED RGB rojo: Timer 1 Canal 2 pin PE11
	*					 - LED RGB verde: Timer 4 Canal 4 pin PD15
	*					 - LED RGB azul: Timer 1 Canal 3 pin PE13
	*					
  *					 La frecuencia de la señal PWM dependerá de la frecuencia del sistema
	*					 configurada para cada proyecto siendo:
	*					 
	*				 	 F(PWM) = f(sistema)/65536
	*
	*					 Para imponer la intensidad en el RGB dependerá del ciclo de trabajo 
	*					 de la señal PWM, por lo que a mayor ciclo de trabajo menor intensidad
	*					 y viceversa. 
	*					 El ciclo de trabajo se calcula de la siguiente manera:
	*					 
	*					CT(%) = CCRx/ARR 
	*		
	*					Siendo CCRx la intensidad que se pasa por parametro al llamar a la 
	*				  función y ARR el valor definido de 65536. Por lo que la intensidad 
	*					puede tener un valor máximo de 65535.
	*
  * @note    modified by ARM
  *          The modifications allow to use this file as User Code Template
  *          within the Device Family Pack.
  ******************************************************************************
  * 
  ******************************************************************************
  */
	
#include "RGB.h"

int initRGB (void);

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim4;

/**
  * @brief Función de inicialización del LED RGB, incializando los Timers 1 y 4.
	*				 Se configura el canal 2 y 3 del Timer 1 y el canal 4 del Timer 4.
	*				 Se configuran los pines de la siguiente manera:
	*				 - LED RGB rojo: Timer 1 Canal 2 pin PE11
	*				 - LED RGB verde: Timer 4 Canal 4 pin PD15
	*				 - LED RGB azul: Timer 1 Canal 3 pin PE13
	*				 Todas las señales PWM estan configuradas para tener una frecuencia: 
	*				 F(PWM) = f(sistema)/65536			
	* @param None
  * @retval None
  */
int initRGB (void){
	TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

	/*Inicialización del Timer 1*/
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    return -1;
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    return -1;
  }
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
		return -1;  
	}
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
		 return -1;  
	}
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 65536/2;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    return -1;
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    return -1;
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    return -1;
  }

  HAL_TIM_MspPostInit(&htim1);
	
	/*Configuración del Timer 4*/
	htim4.Instance = TIM4;
  htim4.Init.Prescaler = 0;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 65535;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    return -1;
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    return -1;
  }
  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
  {
    return -1;
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    return -1;
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 65536/2;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    return -1;
  }

  HAL_TIM_MspPostInit(&htim4);
	
	return 0;
}

/**
  * @brief Función para encender el LED rojo con la intensidad que se pasa por parametro
	*				 activando la señal PWM a traves del Timer 1 canal 2.
	* @param intensidad: Intensidad que se quiere establecer en el LED rojo.
  * @retval None
  */
void encender_LED_rojo ( int intensidad){
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
	htim1.Instance->CCR2 = intensidad;
}

/**
  * @brief Función para encender el LED azul con la intensidad que se pasa por parametro
	*				 activando la señal PWM a traves del Timer 1 canal 3.
	* @param intensidad: Intensidad que se quiere establecer en el LED azul.
  * @retval None
  */
void encender_LED_azul (int intensidad){
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
	htim1.Instance->CCR3 = intensidad;
}

/**
  * @brief Función para encender el LED verde con la intensidad que se pasa por parametro
	*				 activando la señal PWM a traves del Timer 4 canal 4.
	* @param intensidad: Intensidad que se quiere establecer en el LED verde.
  * @retval None
  */
void encender_LED_verde (int intensidad){
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4);
	htim4.Instance->CCR4 = intensidad;
}

/**
  * @brief Función para apagar el LED rojo 
	* @param None
  * @retval None
  */
void apagar_LED_rojo (){
	HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
}

/**
  * @brief Función para apagar el LED azul 
	* @param None
  * @retval None
  */
void apagar_LED_azul (){
	HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);
}

/**
  * @brief Función para apagar el LED verde 
	* @param None
  * @retval None
  */
void apagar_LED_verde (){
	HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_4);
}

/**
  * @brief Función para modificar la intensidad del LED rojo 
	* @param intensidad: Nueva intensidad quese desea establecer en el Led rojo
	*				 El parámetro tiene que ser inferior a 65536.
  * @retval None
  */
void intensidad_LED_rojo (int intensidad){
		htim1.Instance->CCR2 = intensidad;
}

/**
  * @brief Función para modificar la intensidad del LED azul 
	* @param intensidad: Nueva intensidad quese desea establecer en el Led azul
	*				 El parámetro tiene que ser inferior a 65536.
  * @retval None
  */
void intensidad_LED_azul (int intensidad){
		htim1.Instance->CCR3 = intensidad;
}

/**
  * @brief Función para modificar la intensidad del LED verde 
	* @param intensidad: Nueva intensidad quese desea establecer en el Led verde
	*				 El parámetro tiene que ser inferior a 65536.
  * @retval None
  */
void intensidad_LED_verde (int intensidad){
		htim4.Instance->CCR4 = intensidad;
}



