/**
  ******************************************************************************
  * @file    Templates/Src/LCD.c
  * @author  MCD Application Team
  * @brief   Fichero de inicialización del la pantalla LCD de la tarjeta de 
	*					 aplicaciones y funciones para representar texto.Para representar el 
	*			 		 texto por pantalla se utiliza la fuente de texto Arial a traves de 
	*					 la librería Arial12x12. Se emplea el SPI1 con la configuración:
	*					
	*					 - SPI Mode Master
	*					 - CPOL1 y CPHA1
	*					 - MSB a LSB
	*					 - Envío de 8 bits de datos
	*					 - Velocidad del SPI 1 MHz
	*
	*					 Los pines MOSI y SCK se configuran a traves del fichero RTE_Device.h.
	*					 El resto de los pines se configuran en la función GPIO_INIT de la  
	*					 librería LCD. Los pines que se utilizan son los correspondientes al 
	*					 conexionado de la mbed application shield y que vienen definidos en  
	*					 el fichero mbedApplicationBoard_PINOUT.
	*					 Debido al conflicto entre el SPI y el ETHERNET se ha realizado el 
	*					 el cambio del Solder Bridge SB121 para colocarlo en el SB122. Con esto
	*					 al conectar el MOSI en el pin PA7 en realidad se conecta al pin PB5,
	*					 por lo que la configuración de los pines es la siguiente:
	*					
	*					 PIN MOSI-> PB5	
	*					 PIN SCK->	PA5	
	*					 PIN Reset->PA6	
	*					 PIN A0->	 	PF13
	*					 PIN nCs-> 	PD14	
  *
  * @note    modified by ARM
  *          The modifications allow to use this file as User Code Template
  *          within the Device Family Pack.
  ******************************************************************************
  * 
  ******************************************************************************
  */


#include "LCD.h"
#include "Arial12x12.h"
#include "Driver_SPI.h"
#include "mbedAppBoard_PINOUT.h"
#include "Delay.h"

unsigned char buffer[512];
uint8_t posicionL1=0;
uint16_t posicionL2=256;
uint16_t comienzo = 0;
int j;
char L1[128];	//El máximo de caracteres pueden ser 128 por linea, en concreto 128 exclamaciones (!)
char L2[128];
uint8_t recoger1;
uint16_t recoger2;

extern ARM_DRIVER_SPI Driver_SPI1;
ARM_DRIVER_SPI* SPIdrv = &Driver_SPI1;

/**
  * @brief Función que realiza el envío de datos a traves del SPI a la pantalla. Para ello se activa a nivel
	*				 bajo el CS para sleccionar el periférico y con A0 a 1 se indica que se envían datos en lugar de
	*				 comandos. Una vez enviados los datos se vuelve a poner el CS a 1.
	* @param data: datos a enviar mediante el SPI a la pantalla
  * @retval None
  */
void wr_data(unsigned char data){	
  
	ARM_SPI_STATUS stat;

	/*Se activa el CS a nivel bajo*/
	HAL_GPIO_WritePin(LCD_CS_N.Port, LCD_CS_N.IO, GPIO_PIN_RESET);
	/*Se selecciona A0 a 1 para indicar envío de datos*/
	HAL_GPIO_WritePin(LCD_A0.Port, LCD_A0.IO, GPIO_PIN_SET);	
	/*Envío de datos a traves de función Send de CMSIS Driver*/
	SPIdrv -> Send(&data,sizeof(data));	
	/*Se espera hasta que se haya realizado el envío correctamente*/
	stat = SPIdrv->GetStatus ();
	while(stat.busy){
	stat = SPIdrv->GetStatus ();
	}
	/*Se vuelve a poner el CS a 1*/
	HAL_GPIO_WritePin(LCD_CS_N.Port, LCD_CS_N.IO, GPIO_PIN_SET);

}
/**
  * @brief Función que realiza el envío de comandos a traves del SPI a la pantalla. Para ello se activa a nivel
	*				 bajo el CS para sleccionar el periférico y con A0 a 0 se indica que se envían comandos en lugar de
	*				 datos. Una vez enviados los datos se vuelve a poner el CS a 1.
	* @param cmd: comando a enviar mediante el SPI a la pantalla
  * @retval None
  */
void wr_cmd(unsigned char cmd){
	
	ARM_SPI_STATUS stat;
		
	/*Se activa el CS a nivel bajo*/
	HAL_GPIO_WritePin(LCD_CS_N.Port, LCD_CS_N.IO, GPIO_PIN_RESET);	
	/*Se selecciona A0 a 0 para indicar envío de comandos*/
  HAL_GPIO_WritePin(LCD_A0.Port, LCD_A0.IO, GPIO_PIN_RESET);
	/*Envío del comando a traves de función Send de CMSIS Driver*/
	SPIdrv -> Send(&cmd,sizeof(cmd));		 
	/*Se espera hasta que se haya realizado el envío correctamente*/
	stat = SPIdrv->GetStatus ();
	while(stat.busy){
	stat = SPIdrv->GetStatus ();
	}
	/*Se vuelve a poner el CS a 1*/
	HAL_GPIO_WritePin(LCD_CS_N.Port, LCD_CS_N.IO, GPIO_PIN_SET);
}

/**
  * @brief Función de inicialización y configuraciónde la pantalla LCD a traves de las funciones del 
	*				 CMSIS Driver y se resetea la pantalla a traves del envío de comandos.
	*				 Se configura con la siguiente configuración:
	*				
	*				 	 - SPI Mode Master
	*					 - CPOL1 y CPHA1
	*					 - MSB a LSB
	*					 - Envío de 8 bits de datos
	*					 - Velocidad del SPI 1 MHz
	*
	* @param None
  * @retval None
  */
int LCD_reset(void){
	int i;
	
	uint32_t	status = 0;
	
	/*Inicialización del SPI*/
	status = SPIdrv->Initialize(NULL);
	status = SPIdrv->PowerControl(ARM_POWER_FULL);
	status = SPIdrv->Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL1_CPHA1 | ARM_SPI_MSB_LSB | ARM_SPI_DATA_BITS(8), 1000000);



	/*Activación de la señal de reset de la pantalla a nivel bajo*/
	HAL_GPIO_WritePin(LCD_RESET.Port, LCD_RESET.IO, GPIO_PIN_RESET); //RESET = 0
	Delay_us(5);
	HAL_GPIO_WritePin(LCD_RESET.Port, LCD_RESET.IO, GPIO_PIN_SET);		//RESET = 1
	Delay_us(10);
	
	/*Comandos para la incialización de la pantalla*/
	
	/*LCD Display OFF*/
	wr_cmd(0xAE);
	/*Fija tension de polarizacion del LCD a 1/9*/
	wr_cmd(0xA2);
	/*Fija RAM address SEG output corrspondence a normal*/
	wr_cmd(0xA0);
	/*El scan en las salidas del COM es el normal*/
	wr_cmd(0xC8);
	/*Selecciona la relacion de resistencias internas (Rb/Ra) a dos*/
	wr_cmd(0x22);
	/*POwer ON*/
	wr_cmd(0x2F);
	/*Pone la linea de comienzo del display en 0*/
	wr_cmd(0x40);
	/*LCD Display a ON*/
	wr_cmd(0xAF);
	/*Se establece el contraste*/
	wr_cmd(0x81);
	/*Contraste Bajo*/
	wr_cmd(0x17);
	/*Todos los puntos del display a ON*/
	wr_cmd(0xA6);
	
	/*Función que limpia la pnatalla poniendo todos los pixeles en blanco*/
	lcd_clean();
	
	return status;
}

/**
  * @brief Función que realiza la escritura de los datos almacenados en el buffer en las cuatro páginas
	*				 de la panatalla. 
	* @param None
  * @retval None
  */
void copy_to_lcd(void){
    int i;
    wr_cmd(0x00);      // 4 bits de la parte baja de la dirección a 0
    wr_cmd(0x10);      // 4 bits de la parte alta de la dirección a 0
    wr_cmd(0xB0);      // Página 0
    
		/*Escritura de los 128 primeros datos del buffer en la pantalla*/
    for(i=0;i<128;i++){
        wr_data(buffer[i]);
        }
  
     
    wr_cmd(0x00);      // 4 bits de la parte baja de la dirección a 0
    wr_cmd(0x10);      // 4 bits de la parte alta de la dirección a 0
    wr_cmd(0xB1);      // Página 1
    
		/*Escritura de los 128 segundos datos del buffer en la pantalla*/
    for(i=128;i<256;i++){
        wr_data(buffer[i]);
        }
    
    wr_cmd(0x00);      // 4 bits de la parte baja de la dirección a 0
    wr_cmd(0x10);      // 4 bits de la parte alta de la dirección a 0
    wr_cmd(0xB2);      //Página 2
				
		/*Escritura de los 128 terceros datos del buffer en la pantalla*/
    for(i=256;i<384;i++){
        wr_data(buffer[i]);
        }
    
				
    wr_cmd(0x00);      // 4 bits de la parte baja de la dirección a 0
    wr_cmd(0x10);      // 4 bits de la parte alta de la dirección a 0
    wr_cmd(0xB3);      // Pagina 3
     				
		/*Escritura de los 128 ultimos datos del buffer en la pantalla*/	
    for(i=384;i<512;i++){
        wr_data(buffer[i]);
        }
}

/**
  * @brief Función que resetea todos los pixeles de la pantalla dejando la pantalla en blanco
	* @param None
  * @retval None
  */
void lcd_clean(){
	
	/*Inicialización del buffer a 0*/
	for(int i=0; i<512;i++){
		buffer[i]=0;
	}
	/*Escritura en pantalla*/
	copy_to_lcd();
}
/**
  * @brief Función que realiza la escritura del caracter de la primera linea que se pasa por parametro en el 
	*				 buffer para escribir por pantalla con la fuente Arial.
	* @param letra: caracter que se desea escribir en la pimera línea de la pantalla
  * @retval None
  */
int EscribeLetra_L1 (uint8_t letra){
		uint8_t i, valor1 , valor2;

	if(posicionL1<=115){	//Si la cadena se va a salir de la linea dejas de escribir, hemos tomado como referencia el caracter mas grande (@) que son 12 lineas 128 -12 -1=115
	comienzo = 25 *(letra - ' ');	//Coge el primer byte de la tabla (que es el ancho del caracter)
	for (i=0; i<12; i++){

		valor1 = Arial12x12[comienzo+i*2+1];	//guardas el byte de la parte superior del caracter
		valor2 = Arial12x12[comienzo+i*2+2];	//guardas el byte de la parte inferior del caracter
		
		buffer[i+posicionL1] = valor1;				//copias en el buffer el byte guardado y lo pone
		buffer[i+128+posicionL1] = valor2;
	}

	
	posicionL1 = posicionL1 + Arial12x12[comienzo];	// actualiza el valor de la dirección para que escriba el siguiente caracter (le suma el ancho del caracter)
}
	return 0;	
}
/**
  * @brief Función que realiza la escritura del caracter de la segunad linea que se pasa por parametro en el 
	*				 buffer para escribir por pantalla con la fuente Arial.
	* @param letra: caracter que se desea escribir en la segunda línea de la pantalla
  * @retval None
  */
int EscribeLetra_L2 (uint16_t letra){
	uint8_t i, valor1 , valor2;
	
	if(posicionL2<=371){	//Si la cadena se va a salir de la linea dejas de escribir, hemos tomado como referencia el caracter mas grande (@) que son 12 lineas 384 -12 -1=371
	
	comienzo = 25 *(letra - ' ');	//Coge el primer byte de la tabla (que es el ancho del caracter)
	for (i=0; i<12; i++){

		valor1 = Arial12x12[comienzo+i*2+1];	//guardas el byte de la parte superior del caracter
		valor2 = Arial12x12[comienzo+i*2+2];	//guardas el byte de la parte inferior del caracter
		
		buffer[i+posicionL2] = valor1;				//copias en el buffer el byte guardado y lo pone
		buffer[i+128+posicionL2] = valor2;
	}

	
	posicionL2 = posicionL2 + Arial12x12[comienzo];	// actualiza el valor de la dirección para que escriba el siguiente caracter (le suma el ancho del caracter)
}
	return 0;	
}
/**
  * @brief Función que realiza la escritura por pantalla de las líneas de texto que se pasan por parámetro
	* @param lcd_text: Array con las lineas de texto que se desean escribir en la pantalla
  * @retval None
  */
void actualizar(char lcd_text[2][20+1]){
	posicionL1=0;
	posicionL2=256;
	/*Se escriben en el buffer los caracteres de la primera linea de texto*/
	for (j=0; j<strlen(lcd_text[0]);j++){
		recoger1= lcd_text[0][j];
		EscribeLetra_L1(recoger1);
		}
	/*Se escriben en el buffer los caracteres de la segunda linea de texto*/
	for (j=0; j<strlen(lcd_text[1]);j++){
		recoger2= lcd_text[1][j];
		EscribeLetra_L2(recoger2);
		}
	/*Se imprime en la pantalla las líneas de texto que se han escrito en el buffer*/
	copy_to_lcd();
}

/**
  * @brief Función que escribe en todos los pixeles de la pantalla, poniendola en negro, para comprobar
	*				 el correcto funcionamiento de todos los pixeles. 
	* @param None
  * @retval None
  */
void pant_neg (void){
	for(int i=0; i<512;i++){
		buffer[i]=0xff;
	}
	copy_to_lcd();
}

/**
  * @brief Función que realiza la inicialización de tres de los pines A0, nCs y Reset.
	*	  		 Se configuran en los pines:
	*					 PIN Reset->PA6	
	*					 PIN A0->	 	PF13
	*					 PIN nCs-> 	PD14	
	* @param letra: caracter que se desea escribir en la pimera línea de la pantalla
  * @retval None
  */
void GPIO_INIT(void){
	GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LCD_RESET.Port, LCD_RESET.IO, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LCD_A0.Port, LCD_A0.IO, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LCD_CS_N.Port, LCD_CS_N.IO, GPIO_PIN_SET);

  /*Configure GPIO pin : Reset */
  GPIO_InitStruct.Pin = LCD_RESET.IO;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LCD_RESET.Port, &GPIO_InitStruct);

  /*Configure GPIO pin : A0 */
  GPIO_InitStruct.Pin = LCD_A0.IO;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LCD_A0.Port, &GPIO_InitStruct);

  /*Configure GPIO pin : CS */
  GPIO_InitStruct.Pin = LCD_CS_N.IO;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LCD_CS_N.Port, &GPIO_InitStruct);

}