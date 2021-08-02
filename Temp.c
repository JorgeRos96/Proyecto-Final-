/**
  ******************************************************************************
  * @file    Templates/Src/sensor_temp.c
  * @author  MCD Application Team
  * @brief   Fichero de inicialización del sensor de temperatura de la tarjeta 
	*					 de aplicaciones. 
	
	*					 Según aparece en el datahseet la dirección del sensor de temperatura
	*					 esta definida como: 1 0 0 1 A2 A1 A0. Donde los 3 ultimos bits son
	*					 para diferencial el sensor frente a otros perifericos que se empleen 
	*					 con el I2C. Dado que se va a utilizar el acelerometro que tiene los
	*					 bits: 1 0 0, se definen a 0 para diferenciarlo. Por lo que la dirección
	*					 del sensor de temperaura es:
	*					 Dirección ->0x48	
	*					
	*					 Para configurar el I2C se utiliza el CMSIS Driver, a traves de sus 
	*					 sus funciones se inicializa y configura para trabajar en modo FAST.
	*				 	 Los pines del I2C se configuran a traves del fichero RTE_Device.h:
	*					 PIN SDL-> 	 PB8	
	*					 PIN SDA->	 PB9
  *
  * @note    modified by ARM
  *          The modifications allow to use this file as User Code Template
  *          within the Device Family Pack.
  ******************************************************************************
  * 
  ******************************************************************************
  */
	
#include "Temp.h"
#include "Driver_I2C.h"

#define LM75B_I2C_ADDR 0x48

extern ARM_DRIVER_I2C Driver_I2C1;
static ARM_DRIVER_I2C *I2Cdev = &Driver_I2C1;


void initI2c();
float leerTemperatura();

/**
  * @brief Función de inicialización del I2C en modo FAST
	* @param None
  * @retval None
  */
void initI2c(){
	
	I2Cdev -> Initialize (NULL);
	I2Cdev -> PowerControl (ARM_POWER_FULL);
	I2Cdev -> Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_FAST);
	I2Cdev -> Control (ARM_I2C_BUS_CLEAR,0);	
	
}

/**
  * @brief Función que permite obtener el valor de la temperatura del sensor de temperatura
	* @param None
  * @retval None
  */
float leerTemperatura(){
	float temp;
	uint16_t aux;
	uint8_t buf [2];

	/*Se habilita el sensor para que comience a medir*/
	I2Cdev -> MasterTransmit(LM75B_I2C_ADDR, 0x00, 1, true);
	while(I2Cdev -> GetStatus().busy);
	
	/*Se recibe el valor del registro que almacena la temperatura*/
	I2Cdev -> MasterReceive(LM75B_I2C_ADDR, buf, 2, true);
	while(I2Cdev -> GetStatus().busy);

	/*Se seleccionan los bits 15:5 donde se encuentra el valor de la temperatura*/
	aux = ((buf[0]<<8)|buf[1])>>5;
	/*Si el MSB esta a 1 la temperatura es negativa*/
	if ( aux & (1 << 10) )
		/*Se pasa el valor en complemento a 2 a su valor correspondiente en negativo*/
		temp = -((~aux)+1)*0.125;
	else
		/*Valor de la temperatura con una resolución de 0.125 ºC */
		temp = aux *0.125;
	return temp;
}
