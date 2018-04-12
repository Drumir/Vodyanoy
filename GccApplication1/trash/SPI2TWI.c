/*
 * HD44780SPI.c
 *
 * Created: 11.02.2018 21:27:28
 *  Author: Drumir
 */ 

#include <avr/io.h>
#include "SPI2TWI.h"

void LCD_init(void)
{
  SPCR = (1<<SPE) | (1<<MSTR) | (1<<SPR0);  // �������� SPI, � ������ �������, �� ��������� Fclk/16
  SPSR = (1<<SPI2X);  // �������� �������� ������ SPI �� 2 (�� Fclk/8)

  CLEAR_RESET;
  _delay_ms(50);
  SET_RESET;
  _delay_ms(50);
}

void LCD_gotoXY(uint8_t x, uint8_t y)
{
  while(PINB & 0b01000000);   // ���� ���������� ��������
  CLEAR_SS;                   // ����������� SS � ����, �������� ��������, ��� ������ �������� ��������
  SPDR = GOTO_XY;             // ������ ������ ���������� ������� GOTO_XY
  while ( !(SPSR & 0x80) );   // ���� ���������� ��������
  SPDR = x;                   // �������� X
  while ( !(SPSR & 0x80) );
  SPDR = y;                   // �������� ����� ������ �� ������ (Y)
  while ( !(SPSR & 0x80) );   // ���� ���������� ��������
  SET_SS;                     // ��������� SS (�������� ���������)
  _delay_us(US_WAIT);               // ���� ����� ��������� ������, ��� �������� �����������
}

void LCD_writeString(const char *string)
{
  while(PINB & 0b01000000);   // ���� ���������� ��������
  CLEAR_SS;
  SPDR = PRN_STR;
  while ( !(SPSR & 0x80) );
  while(*string != 0)
  {
    SPDR = *string;
    string++;
    while ( !(SPSR & 0x80) );
  }
  SET_SS;
  _delay_us(US_WAIT);
}

void LCD_writeStrXY(const char *string, uint8_t x, uint8_t y)
{
  while(PINB & 0b01000000);   // ���� ���������� ��������
  CLEAR_SS;
  SPDR = PRN_STR_XY;
  while ( !(SPSR & 0x80) );
  SPDR = x;
  while ( !(SPSR & 0x80) );
  SPDR = y;
  while ( !(SPSR & 0x80) );
  while(*string != 0)
  {
    SPDR = *string;
    string++;
    while ( !(SPSR & 0x80) );
  }
  SET_SS;
  _delay_us(US_WAIT);
}
/*
void LCD_writeCommand(uint8_t command)
{
  while(PINB & 0b01000000);   // ���� ���������� ��������
  CLEAR_SS;
  SPDR = WRT_CMD;
  while ( !(SPSR & 0x80) );
  SPDR = command;
  while ( !(SPSR & 0x80) );
  SET_SS;
  _delay_us(5);
}
*/
void LCD_clear(void)
{
  while(PINB & 0b01000000);   // ���� ���������� ��������
  CLEAR_SS;
  SPDR = LCD_CLR;
  while ( !(SPSR & 0x80) );
  SET_SS;
  _delay_us(US_WAIT);
}

void LCD_writeChar(uint8_t character)
{
  while(PINB & 0b01000000);   // ���� ���������� ��������
  CLEAR_SS;
  SPDR = PRN_CHAR;
  while ( !(SPSR & 0x80) );
  SPDR = character;
  while ( !(SPSR & 0x80) );
  SET_SS;
  _delay_us(US_WAIT);
}

void LCD_writeCharXY(uint8_t character, uint8_t x, uint8_t y)
{
  while(PINB & 0b01000000);   // ���� ���������� ��������
  CLEAR_SS;
  SPDR = PRN_CHAR_XY;
  while ( !(SPSR & 0x80) );
  SPDR = character;
  while ( !(SPSR & 0x80) );
  SPDR = x;
  while ( !(SPSR & 0x80) );
  SPDR = y;
  while ( !(SPSR & 0x80) );
  SET_SS;
  _delay_us(US_WAIT);
}
/*
void LCD_writeScreen(const char *screen)
{
  while(PINB & 0b01000000);   // ���� ���������� ��������
  CLEAR_SS;
  SPDR = PRN_SCRN;
  while ( !(SPSR & 0x80) );
  while(*screen != 0)
  {
    SPDR = *screen;
    screen++;
    while ( !(SPSR & 0x80) );
  }
  SET_SS;
  _delay_us(US_WAIT);
}
*/
void LCD_test(void)
{
  while(PINB & 0b01000000);   // ���� ���������� ��������
  CLEAR_SS;
  SPDR = LCD_TEST;
  while ( !(SPSR & 0x80) );
  SET_SS;
  _delay_us(US_WAIT);
}

void LCD_Puts(char *string)
{
  while(PINB & 0b01000000);   // ���� ���������� ��������
  CLEAR_SS;
  SPDR = LCD_PUTS;
  while ( !(SPSR & 0x80) );
  while(*string != 0)
  {
    SPDR = *string;
    string++;
    while ( !(SPSR & 0x80) );
  }
  SET_SS;
  _delay_us(US_WAIT);
}