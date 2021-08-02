
#include <stdio.h>
#include <string.h>
#include "stm32f4xx_hal.h"

int LCD_reset(void);
void lcd_clean(void);
void copy_to_lcd(void);
void wr_data(unsigned char data);
void wr_cmd(unsigned char cmd);
void lcd_clean(void);
int EscribeLetra_L1 (uint8_t letra);
int EscribeLetra_L2 (uint16_t letra);
void actualizar(char lcd_text[2][20+1]);
void pixel (int x, int y, int color);
void GPIO_INIT();
void escribe();
void pant_neg (void);

