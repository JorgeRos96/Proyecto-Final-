#include "stm32f4xx_hal.h"

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);
int initRGB (void);
void encender_LED_rojo ( int intensidad);
void encender_LED_azul (int intensidad);
void encender_LED_verde (int intensidad);
void apagar_LED_rojo (void);
void apagar_LED_azul (void);
void apagar_LED_verde (void);
void intensidad_LED_rojo (int intensidad);
void intensidad_LED_azul (int intensidad);
void intensidad_LED_verde (int intensidad);
