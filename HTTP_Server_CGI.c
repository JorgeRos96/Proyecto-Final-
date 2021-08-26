/**
  ******************************************************************************
  * @file    Templates/Src/HTTP_Server_CGI.c 
  * @author  MCD Application Team
  * @brief   Fichero donde se realizan las funciones que tratan los datos que se
	*					 envían desde la página web a traves de solicitudes GET y POST.
	*					 Además se realiza la gestión de los comandos codificados en los ficheros
	*					 CGI cuando se realizan las peticiones desde la página web.
	*
  * @note    modified by ARM
  *          The modifications allow to use this file as User Code Template
  *          within the Device Family Pack.
  ******************************************************************************
  *
  ******************************************************************************
  */
	
#include <stdio.h>
#include <string.h>
#include "cmsis_os2.h"                  // ::CMSIS:RTOS2
#include "rl_net.h"                     // Keil.MDK-Pro::Network:CORE
#include "LCD.h"
#include "Board_LED.h"                  // ::Board Support:LED
#include "USART.h"
#include "RTC.h"


#if      defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
#pragma  clang diagnostic push
#pragma  clang diagnostic ignored "-Wformat-nonliteral"
#endif

/*External variables*/
extern char time_text[2][20+1];
extern osThreadId_t TID_Rtc_setTime;
extern osThreadId_t TID_Rtc_setDate;

// http_server.c
extern uint16_t AD_in (uint32_t ch);
extern float temperatura(void);
extern bool LEDrun;
extern char lcd_text[2][20+1];
extern osThreadId_t TID_Display;

// Local variables.
static uint8_t P2;
static uint8_t ip_addr[NET_ADDR_IP6_LEN];
static char    ip_string[40];
char ubuf[100];
int s= 0;

// My structure of CGI status variable.
typedef struct {
  uint8_t idx;
  uint8_t unused[3];
} MY_BUF;
#define MYBUF(p)        ((MY_BUF *)p)

/**
  * @brief Función que procesa la consulta recibida por la petición GET. 
	* @param arg
  * @retval None
  */
void netCGI_ProcessQuery (const char *qstr) {
	
  netIF_Option opt = netIF_OptionMAC_Address;
  int16_t      typ = 0;
  char var[40];

  do {
    // Loop through all the parameters
    qstr = netCGI_GetEnvVar (qstr, var, sizeof (var));
    
		// Check return string, 'qstr' now points to the next parameter
    switch (var[0]) {
      case 'i': // Local IP address
        if (var[1] == '4') { opt = netIF_OptionIP4_Address;       }
        else               { opt = netIF_OptionIP6_StaticAddress; }
        break;

      case 'm': // Local network mask
        if (var[1] == '4') { opt = netIF_OptionIP4_SubnetMask; }
        break;

      case 'g': // Default gateway IP address
        if (var[1] == '4') { opt = netIF_OptionIP6_DefaultGateway; }
        else               { opt = netIF_OptionIP6_DefaultGateway; }
        break;

      case 'p': // Primary DNS server IP address
        if (var[1] == '4') { opt = netIF_OptionIP4_PrimaryDNS; }
        else               { opt = netIF_OptionIP6_PrimaryDNS; }
        break;

      case 's': // Secondary DNS server IP address
        if (var[1] == '4') { opt = netIF_OptionIP4_SecondaryDNS; }
        else               { opt = netIF_OptionIP6_SecondaryDNS; }
        break;
      
      default: var[0] = '\0'; break;
    }

    switch (var[1]) {
      case '4': typ = NET_ADDR_IP4; break;
      case '6': typ = NET_ADDR_IP6; break;

      default: var[0] = '\0'; break;
    }

    if ((var[0] != '\0') && (var[2] == '=')) {
      netIP_aton (&var[3], typ, ip_addr);
      // Set required option
      netIF_SetOption (NET_IF_CLASS_ETH, opt, ip_addr, sizeof(ip_addr));
    }
  } while (qstr);
}

/**
  * @brief Función que procesa la consulta recibida por la solicitud POST.
	*	 			 Type code: - 0 = www-url-encoded form data.
	*										- 1 = filename for file upload (null-terminated string).
	*										- 2 = file upload raw data.
	*										- 3 = end of file upload (file close requested).
	*										- 4 = any XML encoded POST data (single or last stream).
	*										- 5 = the same as 4, but with more XML data to follow.
	* @param arg
  * @retval None
  */
void netCGI_ProcessData (uint8_t code, const char *data, uint32_t len) {
  char var[40],passw[12];

  if (code != 0) {
    // Ignore all other codes
    return;
  }
  P2 = 0;
  LEDrun = true;
  if (len == 0) {
    // No data or all items (radio, checkbox) are off
    LED_SetOut (P2);
    return;
  }
  passw[0] = 1;
  do {
    // Parse all parameters
    data = netCGI_GetEnvVar (data, var, sizeof (var));
		// First character is non-null, string exists
		if (var[0] != 0) {
      /*Si se recibe el comando de encender el LED 0*/
      if (strcmp (var, "led0=on") == 0) {
        P2 |= 0x01;
      }
			/*Si se recibe el comando de encender el LED 1*/
      else if (strcmp (var, "led1=on") == 0) {
        P2 |= 0x02;
      }
			/*Si se recibe el comando de encender el LED 2*/
      else if (strcmp (var, "led2=on") == 0) {
        P2 |= 0x04;
      }
			/*Si se recibe el comando para entrar en el modo Browser se para el running lights*/
      else if (strcmp (var, "ctrl=Browser") == 0) {
        LEDrun = false;
      }
			// Change password, retyped password
      else if ((strncmp (var, "pw0=", 4) == 0) ||(strncmp (var, "pw2=", 4) == 0)) {
        /*Si se encuentra activo el login para acceder*/
        if (netHTTPs_LoginActive()) {
          if (passw[0] == 1) {
            strcpy (passw, var+4);
          }
					// Both strings are equal, change the password
          else if (strcmp (passw, var+4) == 0) {
            netHTTPs_SetPassword (passw);
          }
        }
      }
			/* Si se reibe para representar la primera linea de la pantalla LCD*/
      else if (strncmp (var, "lcd1=", 5) == 0) {
       /*Se copia en el buffer que se escribe por pantalla de la línea 1*/
       strcpy (lcd_text[0], var+5);
      }
			// Si se reibe para representar la segunda linea de la pantalla LCD*/
      else if (strncmp (var, "lcd2=", 5) == 0) {
				/*Se copia en el buffer que se escribe por pantalla de la línea 2*/
        strcpy (lcd_text[1], var+5);
				/*Se envía una señal al hilo Display para representar por pantalla*/
        osThreadFlagsSet (TID_Display, 0x01);
      }
			/*Se recibe el valor de la hora estableido en la página web y manda señal para establecerlo en RTC*/				 
			else if (strncmp (var, "tset=", 5) == 0) {
        strcpy (time_text[0], var+5);
        osThreadFlagsSet (TID_Rtc_setTime, 0x20);
      }
			/*Se recibe el valor de la fecha estableido en la página web y manda señal para establecerlo en RTC*/				 
      else if (strncmp (var, "dset=", 5) == 0) {
        strcpy (time_text[1], var+5);
				osThreadFlagsSet (TID_Rtc_setDate, 0x20);
      }
    }
  } while (data);
	/*Se actualiza el LED que se ha seleccionado encender*/
  LED_SetOut (P2);
}

/**
  * @brief Función que procesa la información del script CGI cuyas lineas comienzan por el comando c. Dentro de la variable
	*				 env que se recibe por parámetro se encuentra la misma cadena que se define en el script y por el que se identifica
	*				 la acción a realizar. 
	* @param arg
	* @param *buf: El argumento buff es el puntero al buffer de salida donde la función escribe la respuesta http
	*	@param buflen: Indica el tamaño del buffer de salida en bytes
	*	@param *pcgi: Puntero a la variable que no se modifica y se puede utilizar para almacenar parametros durante varias llamadas
	*				 a la función.
  * @retval Devuelve el número de bytes escritos en el buffer de salida
  */
uint32_t netCGI_Script (const char *env, char *buf, uint32_t buflen, uint32_t *pcgi) {
  
	int32_t socket;
  netTCP_State state;
  NET_ADDR r_client;
  const char *lang;
  uint32_t len = 0U;
  uint8_t id;
  static uint32_t adv;
	float temp;
  netIF_Option opt = netIF_OptionMAC_Address;
  int16_t      typ = 0;


  switch (env[0]) {
    // Analyze a 'c' script line starting position 2
    case 'a' :
      // Network parameters from 'network.cgi'
      switch (env[3]) {
        case '4': typ = NET_ADDR_IP4; break;
        case '6': typ = NET_ADDR_IP6; break;

        default: return (0);
      }
      
      switch (env[2]) {
        case 'l':
          // Link-local address
          if (env[3] == '4') { return (0);                             }
          else               { opt = netIF_OptionIP6_LinkLocalAddress; }
          break;

        case 'i':
          // Write local IP address (IPv4 or IPv6)
          if (env[3] == '4') { opt = netIF_OptionIP4_Address;       }
          else               { opt = netIF_OptionIP6_StaticAddress; }
          break;

        case 'm':
          // Write local network mask
          if (env[3] == '4') { opt = netIF_OptionIP4_SubnetMask; }
          else               { return (0);                       }
          break;

        case 'g':
          // Write default gateway IP address
          if (env[3] == '4') { opt = netIF_OptionIP4_DefaultGateway; }
          else               { opt = netIF_OptionIP6_DefaultGateway; }
          break;

        case 'p':
          // Write primary DNS server IP address
          if (env[3] == '4') { opt = netIF_OptionIP4_PrimaryDNS; }
          else               { opt = netIF_OptionIP6_PrimaryDNS; }
          break;

        case 's':
          // Write secondary DNS server IP address
          if (env[3] == '4') { opt = netIF_OptionIP4_SecondaryDNS; }
          else               { opt = netIF_OptionIP6_SecondaryDNS; }
          break;
					
      }

      netIF_GetOption (NET_IF_CLASS_ETH, opt, ip_addr, sizeof(ip_addr));
      netIP_ntoa (typ, ip_addr, ip_string, sizeof(ip_string));
      len = (uint32_t)sprintf (buf, &env[5], ip_string);
      break;

    case 'b':
      /* Control del  LED que se manda desde el 'led.cgi'*/
      if (env[2] == 'c') {
        // Select Control
        len = (uint32_t)sprintf (buf, &env[4], LEDrun ?     ""     : "selected",
                                               LEDrun ? "selected" :    ""     );
        break;
      }
      // LED CheckBoxes
      id = env[2] - '0';
      if (id > 7) {
        id = 0;
      }
      id = (uint8_t)(1U << id);
      len = (uint32_t)sprintf (buf, &env[4], (P2 & id) ? "checked" : "");
      break;

    case 'c':
      // TCP status from 'tcp.cgi'
      while ((uint32_t)(len + 150) < buflen) {
        socket = ++MYBUF(pcgi)->idx;
        state  = netTCP_GetState (socket);

        if (state == netTCP_StateINVALID) {
          /* Invalid socket, we are done */
          return ((uint32_t)len);
        }

        // 'sprintf' format string is defined here
        len += (uint32_t)sprintf (buf+len,   "<tr align=\"center\">");
        if (state <= netTCP_StateCLOSED) {
          len += (uint32_t)sprintf (buf+len, "<td>%d</td><td>%d</td><td>-</td><td>-</td>"
                                             "<td>-</td><td>-</td></tr>\r\n",
                                             socket,
                                             netTCP_StateCLOSED);
        }
        else if (state == netTCP_StateLISTEN) {
          len += (uint32_t)sprintf (buf+len, "<td>%d</td><td>%d</td><td>%d</td><td>-</td>"
                                             "<td>-</td><td>-</td></tr>\r\n",
                                             socket,
                                             netTCP_StateLISTEN,
                                             netTCP_GetLocalPort(socket));
        }
        else {
          netTCP_GetPeer (socket, &r_client, sizeof(r_client));

          netIP_ntoa (r_client.addr_type, r_client.addr, ip_string, sizeof (ip_string));
          
          len += (uint32_t)sprintf (buf+len, "<td>%d</td><td>%d</td><td>%d</td>"
                                             "<td>%d</td><td>%s</td><td>%d</td></tr>\r\n",
                                             socket, netTCP_StateLISTEN, netTCP_GetLocalPort(socket),
                                             netTCP_GetTimer(socket), ip_string, r_client.port);
        }
      }
      /* More sockets to go, set a repeat flag */
      len |= (1u << 31);
      break;

    case 'd':
      // System password from 'system.cgi'
      switch (env[2]) {
        case '1':
          len = (uint32_t)sprintf (buf, &env[4], netHTTPs_LoginActive() ? "Enabled" : "Disabled");
          break;
        case '2':
          len = (uint32_t)sprintf (buf, &env[4], netHTTPs_GetPassword());
          break;
      }
      break;

    case 'e':
      // Browser Language from 'language.cgi'
      lang = netHTTPs_GetLanguage();
      if      (strncmp (lang, "en", 2) == 0) {
        lang = "English";
      }
      else if (strncmp (lang, "de", 2) == 0) {
        lang = "German";
      }
      else if (strncmp (lang, "fr", 2) == 0) {
        lang = "French";
      }
      else if (strncmp (lang, "sl", 2) == 0) {
        lang = "Slovene";
      }
      else {
        lang = "Unknown";
      }
      len = (uint32_t)sprintf (buf, &env[2], lang, netHTTPs_GetLanguage());
      break;

    case 'f':
      // Control de la pantalla LCD desde 'lcd.cgi' */
      switch (env[2]) {
        case '1':
          len = (uint32_t)sprintf (buf, &env[4], lcd_text[0]);
          break;
        case '2':
          len = (uint32_t)sprintf (buf, &env[4], lcd_text[1]);
          break;
      }
      break;

    case 'g':
      // Control del potenciometro mandado desdez 'ad.cgi'*/
      switch (env[2]) {
				/*Se manda actualizar el valor del potenciometro en hexadecimal*/
        case '1':
          adv = AD_in (0);
          len = (uint32_t)sprintf (buf, &env[4], adv);
				break;
				
				/*Se manda actualizar el valor del potenciometro en voltios*/
        case '2':				
          len = (uint32_t)sprintf (buf, &env[4], (double)((float)adv*3.3f)/4096);
					/*Se envía el valor actualizado del potenciometro a traves de la USART*/
					s = sprintf(ubuf,"\rEl valor del potenciometro es %f\n", (double)((float)adv*3.3f)/4096);
					tx_USART(ubuf, s);
        break;
				
				/*Se manda actualizar el valor del potenciometro del gráfico*/
        case '3':
          adv = (adv * 100) / 4096;
          len = (uint32_t)sprintf (buf, &env[4], adv);
				break;
      }
    break;
			
		/* Actualización del valor del potenciometro de manera periodica desde "AD.cgx"*/
    case 'x':
      adv = AD_in (0);
      len = (uint32_t)sprintf (buf, &env[1], adv);
			/*Se envía el valor actualizado del potenciometro a traves de la USART*/
			s = sprintf(ubuf,"\rEl valor del potenciometro es %f\n", (double)((float)adv*3.3f)/4096);
			tx_USART(ubuf, s);
    break;
		
		/*Actualización del valot de la temperatura desde Temp.cgi*/
		case 'z':
      temp = temperatura();
		  len = (uint32_t)sprintf (buf, &env[4], temp);
			/*Se envía el valor actualizado de ña temperatura a traves de la USART*/
			s = sprintf(ubuf,"\rLa temperatura es de %f\n", temp);
			tx_USART(ubuf, s);
		break;
		
		/*Actualización del valor de la temperatura de manera periodica desde Temp.cgx*/
		case 'm':
			temp = temperatura();
		  len = (uint32_t)sprintf (buf, &env[1], temp);
			/*Se envía el valor actualizado de ña temperatura a traves de la USART*/
			s = sprintf(ubuf,"\rLa temperatura es de %f\n", temp);
			tx_USART(ubuf, s);
    break;
		
		case 'r':
			switch (env[2]) {
				/*Se escribe en el buffer de salida el número total de segundos*/
				case '1':
					adv=getTotalSeconds(); 
					len = (uint64_t)sprintf (buf, &env[4], adv);
				break;
			}			
		break;
			
		case 's':
      switch (env[2]) {
				/*Se escribe en el buffer de salida el número total de segundos para actualizar el EPOCH Time*/
        case '1':
         adv=getTotalSeconds();
				 len = (uint64_t)sprintf (buf, &env[4], adv);
				break;
      }		
		break;
			
		case 't':
			switch (env[2]) {
				/*Escribe en el buffer de salida la hora*/
				case '1':
					len = (uint32_t)sprintf (buf, &env[4], time_text[0]);
				break;
					/*Escribe en el buffer de salida la fecha*/
				case '2':
					len = (uint32_t)sprintf (buf, &env[4], time_text[1]);
				break;
			}
		break;
  }
  return (len);
}

#if      defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
#pragma  clang diagnostic pop
#endif
