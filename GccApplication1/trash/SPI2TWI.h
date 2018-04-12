/*
 * HD44780SPI.h
 *
 * Created: 11.02.2018 21:27:59
 *  Author: Drumir
 */ 


#ifndef HD44780SPI_H_
#define HD44780SPI_H_

#include <avr/io.h>
#include <util/delay.h>

//#define WRT_CMD     0
#define GOTO_XY     1
#define PRN_STR     2
#define PRN_STR_XY  3
#define LCD_CLR     4
#define PRN_CHAR    5
#define PRN_CHAR_XY 6
//#define PRN_SCRN    7
#define LCD_TEST    8
#define LCD_PUTS    9

#define US_WAIT     12

#define SET_SS        PORTB |= 0b00010000
#define CLEAR_SS      PORTB &= 0b11101111

#define SET_RESET     PORTC |= 0b00100000
#define CLEAR_RESET   PORTC &= 0b11011111



void LCD_init(void);
void LCD_gotoXY(uint8_t x, uint8_t y);
void LCD_writeString(const char *string);
void LCD_writeStrXY(const char *string, uint8_t x, uint8_t y);
//void LCD_writeCommand(uint8_t command);
void LCD_clear(void);
void LCD_writeChar(uint8_t character);
void LCD_writeCharXY(uint8_t character, uint8_t x, uint8_t y);
//void LCD_writeScreen(const char *screen);
void LCD_test(void);
void LCD_Puts(char *string);








#endif /* HD44780SPI_H_ */