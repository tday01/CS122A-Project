#include <avr/pgmspace.h>
#include <stdint.h>

#ifndef NOKIA5110_H_
#define NOKIA5110_H_

#define PORT_LCD PORTB
#define DDR_LCD DDRB
#define LCD_SCE PB1
#define LCD_RST PB2
#define LCD_DC PB3
#define LCD_DIN PB4
#define LCD_CLK PB5
#define LCD_CONTRAST 0X40

void LCD_5110_print_num(uint16_t value);
void LCD_5110_init(void);
void LCD_5110_clear(void);
void LCD_5110_power(uint8_t on);
void LCD_5110_set_point(uint8_t x, uint8_t y, uint8_t value);
void LCD_5110_write(char code, uint8_t scale);
void LCD_5110_write_string(const char *str, uint8_t scale);
void LCD_5110_set_position(uint8_t x, uint8_t y);
void LCD_5110_print(void);
void LCD_5110_image(const uint8_t *img);

#endif /* NOKIA5110_H_ */
