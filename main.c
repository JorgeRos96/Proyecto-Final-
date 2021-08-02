/**
  ******************************************************************************
  * @file    Templates/Src/main.c 
  * @author  MCD Application Team
  * @brief   Servidor web a traves de la conexión Ethernet del microcontrolador.
	*					 Se utiliza plantilla del ejemplo para microcontroladores STM32F4.
	*					 Dentro de la página web se encuentran cinco ventanas, ademas de las 
	*					 que se encuentran por defecto (Network, System, Language y Statics),
	*					 donde se puedenrealizar las siguientes funciones:
	*	
	*					 - LED: Manejo de los LEDs del microcontrolador donde se puede seleccionar
	*						 encender los LEDs de manera cíclica o manejarlos de manera individual.
	*					 - LCD: Donde se puede establecer el texto que se desea mostrar por la 
	*						 pantalla LCD de la tarjeta de aplicaciones.
	*					 - AD: Se puede visualizar el valor del potenciometro 1 de la tarjeta de
	*					 	 aplicaciones. Además se puede configurar para actualizar el valor de manera 
	*					 	 periódica
	*					 - Temperature: En esta página se puede visualizar la temperatura que capta
	*						 el sensor de temperatura. También se puede configurar para actualizar
	*						 el valor de manera periodica.
	*					 - Time: Se visualiza la hora y fecha, además se puede configurar la hora 
	*						 y fecha que se desea mostrar.
	*					
	*					 Se configura la conexión a traves del CMSIS Driver utilizando el 
	*					 driver para LAN8742A. Se configura servidor Http donde se programan 
	*					 las páginas a traves de CGI.
	*					 En el fichero RTE_Device.h se activan los pines que se van a utilizar
	*					 del Ethernet, configurando el RMII y el managment data interface. Se 
	*					 dejan los pines por defecto a excepción del ETH_RMII_TXD0 en el que
	*					 hay que seleccionar el pin PG13 y el ETH_RMII_TX_EN  donde se selecciona
	*					 el pin PG11.
	*					 Despues en el fichero Net_Config_ETH_0.h se configura la IPV4 y se 
	*					 deshabilita el DHCP ya que la IP la vamos a fijar nosotros. Se 
	*					 configura la IP: 192.168.1.100 y se puede ver que la MAC configurada es
	*					 1E-30-6C-A2-45-5E.
	*
	*					 En este proyecto también se utiliza el LED RGB para poder visualizar 
	*					 cuando la temperatura es excesiva y supera los 30ºC encendiendose el
	*					 color rojo del RGB. En caso de que la temperatura sea inferior a 30ºC
	*					 se encuentra encendido el color azul del RGB.
	*
	*					 A traves de la USART 3 se envían mensajes al terminal cuando se realizan
	*					 las consultas de la temperatura y del ADC con el valor obtenido, y se 
	*					 envía mensaje avisando cuando se reliza una escritura en la pantalla 
	*					 desde la página web. También envía mensaje al introducir una nueva hora
	*					 y fecha desde la página web, y cada minuto envía la hora y fecha al 
	*					 terminal.
	*
	*  				 Para el uso del Ethernet es necesario que trabaje a una frecuencia
	*					 mayor de 25 MHz. En este proyecto se ha configurado el reloj del 
  *					 sistema a 180 MHz utilizando el PLL y el HSI como fuente de reloj.
	*
  * @note    modified by ARM
  *          The modifications allow to use this file as User Code Template
  *          within the Device Family Pack.
  ******************************************************************************
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "Delay.h"


#ifdef RTE_CMSIS_RTOS2_RTX5
/**
  * Override default HAL_GetTick function
  */
uint32_t HAL_GetTick (void) {
  static uint32_t ticks = 0U;
         uint32_t i;

  if (osKernelGetState () == osKernelRunning) {
    return ((uint32_t)osKernelGetTickCount ());
  }

  /* If Kernel is not running wait approximately 1 ms then increment 
     and return auxiliary tick counter value */
  for (i = (SystemCoreClock >> 14U); i > 0U; i--) {
    __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
    __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
  }
  return ++ticks;
}

#endif

/** @addtogroup stm32f4xx_hal_Examples
  * @{
  */

/** @addtogroup Templates
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void Error_Handler(void);

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{

  /* stm32f7xx HAL library initialization:
       - Configure the Flash prefetch, Flash preread and Buffer caches
       - Systick timer is configured by default as source of time base, but user 
             can eventually implement his proper time base source (a general purpose 
             timer for example or other time source), keeping in mind that Time base 
             duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and 
             handled in milliseconds basis.
       - Low Level Initialization
     */
  HAL_Init();

  /* Configure the system clock to 168 MHz */
  SystemClock_Config();
  SystemCoreClockUpdate();
	
	Init_Delay(180,4);

  /* Add your application code here
     */

#ifdef RTE_CMSIS_RTOS2
  /* Initialize CMSIS-RTOS2 */
  osKernelInitialize ();

  /* Create application main thread */
  osThreadNew(app_main, NULL, &app_main_attr);

  /* Start thread execution */
  osKernelStart();
#endif

  /* Infinite loop */
  while (1)
  {
  }
}

/**
  * @brief  Función de configuración del reloj del sistema en el que se utiliza
	*					como fuente de reloj del sistema el PLL con el HSI como fuente de 
	*					reloj. Se configura una frecuencia del sistema de 180 MHz.
  *         Se configuran los paramteros de la siguiente manera: 
  *            System Clock source            = PLL (HSI)
  *            SYSCLK(Hz)                     = 180000000
  *            HCLK(Hz)                       = 180000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSI Frequency(Hz)              = 16000000
  *            PLL_M                          = 8
  *            PLL_N                          = 180
  *            PLL_P                          = 2
  *            PLL_Q                          = 4
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 5
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{ 
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
	
  /** Se configura el HSI como fuente de reloj del PLL y se configuran
	* 	los parametros del PLL para ajusta la frecuencia a 180 MHz con una
	* 	frecuencia del HSI de 16 MHZ (por defecto).
	* 	SYSCLK =[(16MHz(frecuencia HSI)/8(PLLM))*180 (PLLN)]/2 (PLLP) = 180 MHz
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 180;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Se activa el modo de Over Drive para poder alcanzar los 180 MHz
	* 	como frecuencia del sistema
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }
  /** Se selecciona el PLL como fuente de reloj del sistema y se configuran los parametros
	*		para configurar el HCLK, PCLK1 y PCLK2. La frecuencia máxima del HCLK es 180 MHZ, la 
	*		frecuencia máxima del PCLK1 es de 45 MHZ y la frecuencia máxima del PCLK2 es de 90 MHz
	*		HCLK = SYSCK/AHB = 180 MHz / 1 = 180 MHz
	*		PCLK1 = HCLK/APB1 = 180 MHz / 4 = 45 MHZ
	*		PCLK2 = HCLK/APB2 = 180 MHz / 2 = 90 MHZ
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
  /* User may add here some code to deal with this error */
  while(1)
  {
  }
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}

#endif

/**
  * @}
  */ 

/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
