//********************************************************
//**** Functions for Interfacing NOKIA 3310 Display *****
//********************************************************
//Controller:	ATmega32 (Clock: 1 Mhz-internal)
//Compiler:		ImageCraft ICCAVR
//Author:		CC Dharmani, Chennai (India)
//Date:			Sep 2008
//
// display_fft() added by Michael Spiceland (tinkerish.com)
//
//********************************************************

#include <avr/io.h>


#include "3310_routines.h"
#include <util/delay.h>


//global variable for remembering where to start writing the next text string on 3310 LCD
unsigned char char_start;



/*--------------------------------------------------------------------------------------------------
  Name         :  spi_init
  Description  :  Initialising atmega SPI for using with 3310 LCD
  Argument(s)  :  None.
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
//SPI initialize
//clock rate: CPUclk/2 
void spi_init(void)
{
//	SPCR = 0x58; //setup SPI		
  SPCR = 0 << SPIE | 1 << SPE | 0 << DORD | 1 << MSTR | 1 << CPOL | 0 << CPHA | 0 << SPR1 | 0 << SPR0;
  SPSR = 1 << SPI2X;
}

/*--------------------------------------------------------------------------------------------------
  Name         :  LCD_init
  Description  :  LCD controller initialization.
  Argument(s)  :  None.
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
void LCD_init ( void )
{
    
	_delay_ms(100);
	
	CLEAR_SCE_PIN;    //Enable LCD
    
	CLEAR_RST_PIN;	//reset LCD
    _delay_ms(100);
    SET_RST_PIN;
	
	SET_SCE_PIN;	//disable LCD

    LCD_writeCommand( 0x21 );  // LCD Extended Commands.
    LCD_writeCommand( 0xE0 );  // Set LCD Vop (Contrast).
    LCD_writeCommand( 0x04 );  // Set Temp coefficent.
    LCD_writeCommand( 0x13 );  // LCD bias mode 1:48.
    LCD_writeCommand( 0x20 );  // LCD Standard Commands, Horizontal addressing mode.
    LCD_writeCommand( 0x0c );  // LCD in normal mode.
}

/*--------------------------------------------------------------------------------------------------
  Name         :  LCD_writeCommand
  Description  :  Sends command to display controller.
  Argument(s)  :  command -> command to be sent
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
void LCD_writeCommand ( unsigned char command )
{
    CLEAR_SCE_PIN;	  //enable LCD
	
	CLEAR_DC_PIN;	  //set LCD in command mode
	
    //  Send data to display controller.
    SPDR = command;

    //  Wait until Tx register empty.
    while ( !(SPSR & 0x80) );

    SET_SCE_PIN;   	 //disable LCD
}

/*--------------------------------------------------------------------------------------------------
  Name         :  LCD_writeData
  Description  :  Sends Data to display controller.
  Argument(s)  :  Data -> Data to be sent
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
void LCD_writeData ( unsigned char Data )
{
    CLEAR_SCE_PIN;	  //enable LCD
	
	  SET_DC_PIN;	  //set LCD in Data mode
	
    //  Send data to display controller.
    SPDR = Data;

    //  Wait until Tx register empty.
    while ( !(SPSR & 0x80) );

    SET_SCE_PIN;   	 //disable LCD
}

/*--------------------------------------------------------------------------------------------------
  Name         :  LCD_clear
  Description  :  Clears the display
  Argument(s)  :  None.
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
void LCD_clear ( void )
{
    int i;
	
//	LCD_gotoXY (0,0);  	//start with (0,0) position

    for(i = 0; i < 524; i ++)
	     LCD_writeData( 0x00 );
   
//    LCD_gotoXY (0,0);	//bring the XY position back to (0,0)
      
}

/*--------------------------------------------------------------------------------------------------
  Name         :  LCD_gotoXY
  Description  :  Sets cursor location to xy location corresponding to basic font size.
  Argument(s)  :  x - range: 0 to 84
  			   	  y -> range: 0 to 5
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
void LCD_gotoXY ( unsigned char x, unsigned char y )
{
  LCD_writeCommand (0x80 | x);   //column
  LCD_writeCommand (0x40 | y);   //row
}

//--------------------------------------------------------------------------------------------------

void LCD_gotoX ( unsigned char x)
{
  LCD_writeCommand (0x80 | x);   //column
}
//--------------------------------------------------------------------------------------------------

void LCD_gotoY (unsigned char y )
{
  LCD_writeCommand (0x40 | y);   //row
}

/*--------------------------------------------------------------------------------------------------
  Name         :  LCD_writeChar
  Description  :  Displays a character at current cursor location and increment cursor location.
  Argument(s)  :  ch   -> Character to write.
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
void LCD_writeChar (unsigned char ch)
{
   unsigned char j;
  
	//LCD_writeData(0x00);
   
   for(j = 0; j < 6; j ++)
     LCD_writeData( pgm_read_byte(&(smallFont [(ch-32)*6 + j] )));
	 
//   LCD_writeData(0x00);
} 
/*--------------------------------------------------------------------------------------------------
  Name         :  LCD_writeCharInv
  Description  :  Displays a character at current cursor location and increment cursor location in invers mode.
  Argument(s)  :  ch   -> Character to write.
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
void LCD_writeCharInv (unsigned char ch)
{
   unsigned char j;
  
   //LCD_writeData(0xFF);
   
   for(j = 0; j < 6; j ++)
     LCD_writeData( ~(pgm_read_byte(&(smallFont [(ch-32)*6 + j] ))));
	 
//   LCD_writeData( 0xFF );
} 



/*--------------------------------------------------------------------------------------------------
  Name         :  LCD_writeString
  Description  :  Displays a string stored in RAM, in small fonts (refer to 3310_routines.h)
  Argument(s)  :  string -> Pointer to ASCII string (stored in RAM)
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
void LCD_writeString ( const char *string )
{
    while ( *string )
        LCD_writeChar( *string++ );
}

/*--------------------------------------------------------------------------------------------------
  Name         :  LCD_writeString
  Description  :  Displays a string stored in FLASH, in small fonts (refer to 3310_routines.h)
  Argument(s)  :  string -> Pointer to ASCII string (stored in FLASH)
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
void LCD_writePMstring ( const char *string )
{
	char lastChar;
  do{
		lastChar = pgm_read_byte(string ++);
		LCD_writeChar(lastChar);
	}while(lastChar != '\0');
}
/*--------------------------------------------------------------------------------------------------
  Name         :  LCD_writeStringInv
  Description  :  Displays a string stored in FLASH, in small inverse fonts (refer to 3310_routines.h)
  Argument(s)  :  string -> Pointer to ASCII string (stored in FLASH)
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
void LCD_writeStringInv ( const char *string )
{
    while ( *string )
        LCD_writeCharInv( *string++ );
}
/*--------------------------------------------------------------------------------------------------
  Name         :  LCD_writeString
  Description  :  Displays a string stored in FLASH, in small fonts (refer to 3310_routines.h)
  Argument(s)  :  string -> Pointer to ASCII string (stored in FLASH)
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
void LCD_writePMstringInv ( const char *string )
{
	char lastChar;
  do{
		lastChar = pgm_read_byte(string ++);
		LCD_writeCharInv(lastChar);
	}while(lastChar != '\0');
}

/*--------------------------------------------------------------------------------------------------
                                         End of file.
--------------------------------------------------------------------------------------------------*/

