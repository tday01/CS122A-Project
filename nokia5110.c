#include "nokia5110.h"
#define F_CPU 1000000UL
#include <avr/pgmspace.h>
#include <avr/io.h>
#include <util/delay.h>
#include "nokia5110_map.h"

static struct
{
    uint8_t screen[504];
    uint8_t cursor_x;
    uint8_t cursor_y;
    
} nokia_lcd = {
    .cursor_x = 0,
    .cursor_y = 0};

static void write(uint8_t bytes, uint8_t is_data)
{
    register uint8_t i;
    PORT_LCD &= ~(1 << LCD_SCE);
	
    if (is_data)
        PORT_LCD |= (1 << LCD_DC);
    else
        PORT_LCD &= ~(1 << LCD_DC);

    for (i = 0; i < 8; i++)
    {
        if ((bytes >> (7 - i)) & 0x01)
            PORT_LCD |= (1 << LCD_DIN);
        else
            PORT_LCD &= ~(1 << LCD_DIN);

        PORT_LCD |= (1 << LCD_CLK);
        PORT_LCD &= ~(1 << LCD_CLK);
    }
    PORT_LCD |= (1 << LCD_SCE);
}

static void write_cmd(uint8_t cmd)
{
    write(cmd, 0);
}

static void write_data(uint8_t data)
{
    write(data, 1);
}

void LCD_5110_init(void)
{
    register unsigned i;
    DDR_LCD |= (1 << LCD_SCE);
    DDR_LCD |= (1 << LCD_RST);
    DDR_LCD |= (1 << LCD_DC);
    DDR_LCD |= (1 << LCD_DIN);
    DDR_LCD |= (1 << LCD_CLK);
    PORT_LCD |= (1 << LCD_RST);
    PORT_LCD |= (1 << LCD_SCE);
    _delay_ms(10);
    PORT_LCD &= ~(1 << LCD_RST);
    _delay_ms(70);
    PORT_LCD |= (1 << LCD_RST);
    PORT_LCD &= ~(1 << LCD_SCE);
 
    write_cmd(0x21);
    write_cmd(0x13);
    write_cmd(0x06);
    write_cmd(0xC2);
    write_cmd(0x20);
    write_cmd(0x09);
    write_cmd(0x80);
    write_cmd(LCD_CONTRAST);
    for (i = 0; i < 504; i++)
        write_data(0x00);
    write_cmd(0x08);
    write_cmd(0x0C);
}

void LCD_5110_clear(void)
{
    register unsigned i;
    write_cmd(0x80);
    write_cmd(0x40);
    nokia_lcd.cursor_x = 0;
    nokia_lcd.cursor_y = 0;
    for (i = 0; i < 504; i++)
        nokia_lcd.screen[i] = 0x00;
}

void LCD_5110_power(uint8_t on)
{
    write_cmd(on ? 0x20 : 0x24);
}


void LCD_5110_print_num(uint16_t value)
{
	signed char count = 0;
	uint16_t digit;
	uint8_t buff[16];
	
	while (value > 0) {
		digit = value % 10;
		buff[count] = (uint8_t)(digit + 48);
		++count;
		value /= 10;
	}
		--count;
		while(count >= 0){
			LCD_5110_write(buff[count], 1);
			--count;
		}
}

void LCD_5110_set_point(uint8_t x, uint8_t y, uint8_t value)
{
    uint8_t *byte = &nokia_lcd.screen[y / 8 * 84 + x];
    if (value)
        *byte |= (1 << (y % 8));
    else
        *byte &= ~(1 << (y % 8));
}

void LCD_5110_write(char code, uint8_t scale)
{
    register uint8_t x, y;
    
    for (x = 0; x < 5 * scale; x++)
        for (y = 0; y < 7 * scale; y++)
            if (pgm_read_byte(&MAP[code - 32][x / scale]) & (1 << y / scale))
                LCD_5110_set_point(nokia_lcd.cursor_x + x, nokia_lcd.cursor_y + y, 1);
            else
                LCD_5110_set_point(nokia_lcd.cursor_x + x, nokia_lcd.cursor_y + y, 0);
    
    nokia_lcd.cursor_x += 5 * scale + 1;
    if (nokia_lcd.cursor_x >= 84)
    {
        nokia_lcd.cursor_x = 0;
        nokia_lcd.cursor_y += 7 * scale + 1;
    }
    if (nokia_lcd.cursor_y >= 48)
    {
        nokia_lcd.cursor_x = 0;
        nokia_lcd.cursor_y = 0;
    }
}

void LCD_5110_write_string(const char *str, uint8_t scale)
{
    while (*str)
        LCD_5110_write(*str++, scale);
}

void LCD_5110_set_position(uint8_t x, uint8_t y)
{
    nokia_lcd.cursor_x = x;
    nokia_lcd.cursor_y = y;
}

void LCD_5110_print(void)
{
    register unsigned i;
    write_cmd(0x80);
    write_cmd(0x40);
    
    for (i = 0; i < 504; i++)
        write_data(nokia_lcd.screen[i]);
}

void LCD_5110_image(const uint8_t *img)
{
    for (register unsigned i = 0; i < 504; i++)
    {
        nokia_lcd.screen[i] = img[i];
    }
}
