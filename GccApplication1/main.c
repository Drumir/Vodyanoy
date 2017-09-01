/*
 * Sim900_twi_lsd.c
 *
 * Created: 24.08.2017 22:26:28
 * Author : Drumir
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "TWI_LCD.h"
#include <util/delay.h>
#include "uart.h"
#include "vodyanoy2.0.h"

#define RXBUFMAXSIZE 64
#define RXBUFSTRCOUNT 4
uint16_t char_count = 0, str_count = 0,  mode = 0;
volatile uint16_t cur_str = 0;
char buf[23], str[23], rxb[RXBUFSTRCOUNT][RXBUFMAXSIZE];
FIFO( 128 ) uart_tx_fifo;

ISR(USART__RXC_vect);  //Имена прерываний определены в c:\Program Files\Atmel\AVR Studio 5.1\extensions\Atmel\AVRGCC\3.3.1.27\AVRToolchain\avr\include\avr\iom32a.h (для mega32a) БЛЕАТЬ!!!
ISR(USART__UDRE_vect);
ISR(INT1_vect);	       //CLK от клавы

void incomingMessage(char* s);
void renewLCD(void);
void SIM900_PowerOn(void);


int main(void)
{
	DDRC  = 0b01000011;		// 7-н, 6-/reset_lcd, 5-2 - н, 1-SDA_LCD, 0-SCL_LCD
	PORTC = 0b11000011;
	DDRD =  0b10000010;		// 7-En4V, 6-3 клава, 2-геркон, 1-TX, 0-RX
	PORTD = 0b01111100;
	DDRB  = 0b11111010;		// 7-4 отсутствующий SPI_LCD, 3 - подсветка дисплея, 2-power_fail, 1 - SIM900_pwrkey, 0 - SIM900_status
	PORTB = 0b00000010;

	sbi(GICR, 7);					// Разрешить INT1  Для клавы
	sbi(MCUCR, 3); cbi(MCUCR, 2);	// Прерывание по переходу 1->0  на линии CLK клавы
	
    cbi(PORTC, 6);			// reset TWILCD;
    _delay_ms(100);
    sbi(PORTC, 6);			// Выведем TWILCD из ресета;
    _delay_ms(100);
    LCDInit(0x60);
	LCDClear();
    LCDGotoXY(0, 0); 
	uart_init();
	sei();
//	_delay_ms(2000);
	
	for(uint8_t i = 0; i < RXBUFSTRCOUNT; i ++)
		for(uint16_t j = 0; j < RXBUFMAXSIZE; j ++)
			rxb[i][j] = 0;
			
	SIM900_PowerOn();
	
	LCDPuts("Sim900_Up");
	do{
		_delay_ms(1000);
		uart_send("AT+CREG?");
		while(rxb[cur_str][0] == 0);renewLCD();
		while(rxb[cur_str][0] == 0);
		if(strncmp(&rxb[cur_str][0], "+CREG: 0,1", 10) == 0) mode = 1; 
		renewLCD();		
		while(rxb[cur_str][0] == 0);renewLCD();		
	} while(mode != 1);
	LCDPuts("Registration 0,1");

	uart_send("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"");
	_delay_ms(100);renewLCD();_delay_ms(100);renewLCD();_delay_ms(2000);
	uart_send("AT+SAPBR=3,1,\"APN\",\"internet.tele2.ru\"");
	_delay_ms(100);renewLCD();_delay_ms(100);renewLCD();_delay_ms(2000);
	uart_send("AT+SAPBR=3,1,\"USER\",\"\"");
	_delay_ms(100);renewLCD();_delay_ms(100);renewLCD();_delay_ms(2000);
	uart_send("AT+SAPBR=3,1,\"PWD\",\"\"");
	_delay_ms(100);renewLCD();_delay_ms(100);renewLCD();_delay_ms(2000);
	uart_send("AT+SAPBR=1,1");
	_delay_ms(100);renewLCD();_delay_ms(100);renewLCD();_delay_ms(2000);
	uart_send("AT+SAPBR=2,1");
	_delay_ms(100);renewLCD();_delay_ms(100);renewLCD();_delay_ms(2000);
	LCDPuts("GPRS Connected");

	uart_send("AT+HTTPINIT");
	_delay_ms(100);renewLCD();_delay_ms(100);renewLCD();_delay_ms(2000);
	uart_send("AT+HTTPPARA=\"CID\",1");
	_delay_ms(100);renewLCD();_delay_ms(100);renewLCD();_delay_ms(2000);
	uart_send("AT+HTTPPARA=\"URL\",\"http://ya.ru\"");
	_delay_ms(100);renewLCD();_delay_ms(100);renewLCD();_delay_ms(2000);
	uart_send("AT+HTTPACTION=0");
	while(rxb[cur_str][0] == 0);renewLCD();
	while(rxb[cur_str][0] == 0);renewLCD();
	_delay_ms(5000);
	uart_send("AT+HTTPREAD=0,250");

    while (1) 
    {
		while(rxb[cur_str][0] == 0);
		renewLCD();
    }
}

//------------------------------------------------------------------------------
ISR(INT1_vect) 	//CLK от клавы                          КЛАВИАТУРА
{
	sei();
	//char str[20];
	unsigned char bstat;  
	bstat = PIND & 0b01111000;
	if (bstat == 0b01010000) 			// Кнопка вниз 
	{
		cbi(PORTB, 1);
		_delay_ms(1500);
		sbi(PORTB, 1);
		LCDPuts("*TurnOFF*");
		while((PINB & 0b00000001) == 1)_delay_ms(10);			// Ждем 0 на STATUS
		LCDPuts("*PowerDown*");
		cbi(PORTD, 7);			// Disable 4.0v
	}
	if (bstat == 0b01100000) 			// Кнопка вверх
	{
	}
	if (bstat == 0b00110000) 			// Кнопка влево
	{
		LCDClear();	
	}
	
	if (bstat == 0b01110000)      // Кнопка вправо
	{	
		switch(mode)
		{
			case 0: {uart_send("AT+CGATT?");break;}
			case 1: {uart_send("AT+CGATT=1");break;}
			case 2: {uart_send("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\""); break; }
			case 3: {uart_send("AT+SAPBR=3,1,\"APN\",\"internet.tele2.ru\""); break; }
			case 4: {uart_send("AT+SAPBR=3,1,\"USER\",\"\""); break; }
			case 5: {uart_send("AT+SAPBR=3,1,\"PWD\",\"\""); break; }
			case 6: {uart_send("AT+SAPBR=1,1"); break; }
			case 7: {uart_send("AT+HTTPINIT"); break; }
			case 8: {uart_send("AT+HTTPPARA=\"CID\",1"); break; } 
		
		
			/*                uart_send("AT+SAPBR=2,1"); break; 
			uart_send("AT+SAPBR=1,1"); _delay_ms(300); 
			uart_send("AT+SAPBR=2,1"); break; 
			break;} 
			case 1: {uart_send("AT+SAPBR=4,1"); break; } 
			case 2: {uart_send("AT+HTTPINIT"); break; } 
			case 3: {uart_send("AT+HTTPPARA=\"CID\",1"); break; } 
			case 4: {uart_send("AT+HTTPPARA=\"URL\",\"https://ya.ru\""); break; } 
			case 5: {uart_send("AT+HTTPACTION=0"); break; } 
			case 6: {uart_send("AT+HTTPREAD"); break; } 
			case 7: {uart_send("AT+HTTPTERM"); break; } 
			case 8: {uart_send("AT+SAPBR=0,1"); break; } */
		}	
		mode ++;
		if(mode == 9) mode = 0;	
	}
}
//------------------------------------------------------------------------------

ISR(USART_RXC_vect) //Обработчик прерывания по окончанию приёма байта
{
	static uint16_t length = 0, strnum = 0;
	static char rxbuf[RXBUFMAXSIZE];
	uint8_t rxbyte;
	rxbyte = UDR;
	rxbuf[length] = rxbyte; 
	sei();
	length ++;
	if(length == RXBUFMAXSIZE - 2){			// Если длинна сообщения почти достигла лимита
		rxbuf[length] = 0;					// Допишем в конец '/0' 
		strcpy(&rxb[strnum][0], rxbuf);					// Скопируем его в основную программу
		strnum ++; if(strnum == RXBUFSTRCOUNT) strnum = 0;
		length = 0;
		str_count ++;
	}
	if(rxbyte == CHAR_LF && length > 1 && rxbuf[length-2] == CHAR_CR){		// Если приняли подряд CHAR_CR и CHAR_LF
		if(length == 2)					// И кроме них ничего больше нет
			length = 0;					// избавимся от них
		else{									// Иначе это ценное сообщение
			rxbuf[length] = 0;				// Допишем в конец '/0'
			strcpy(&rxb[strnum][0], rxbuf);					// Скопируем его в основную программу
			strnum ++; if(strnum == RXBUFSTRCOUNT) strnum = 0;
			length = 0;
			str_count ++;
		}
	}
}
//-------------
void uart_init( void )
{
	//настройка скорости обмена
	UBRRH = 0;
	UBRRL = 12;      // для 8МГц 51 - 9600+0.2%ошибки,  25 - 19200+0,2%ошибки,  12 - 
	//8 бит данных, 1 стоп бит, без контроля четности
	UCSRC = ( 1 << URSEL ) | ( 1 << UCSZ1 ) | ( 1 << UCSZ0 );
	//разрешить передачу, прием данных и прерывание по приёму байта
	UCSRB =  ( 1 << TXEN ) | ( 1 << RXEN ) | (1 << RXCIE );
}


ISR(USART_UDRE_vect)     // Прерывание по опустошению регистра данных
{
	cli();
	if( FIFO_IS_EMPTY( uart_tx_fifo ) ) {
		UCSRB &= ~( 1 << UDRIE ); //если данных в fifo больше нет то запрещаем это прерывание
	}
	else {
		//иначе передаем следующий байт
		char txbyte = FIFO_FRONT( uart_tx_fifo );
		FIFO_POP( uart_tx_fifo );
		UDR = txbyte;
	}
	sei();
}

int uart_send(char *str)
{
	for(int i = 0; str[i] != 0; i ++)    //Помещаем строку в буфер передатчика
	FIFO_PUSH( uart_tx_fifo, str[i] );

	FIFO_PUSH( uart_tx_fifo, CHAR_CR );  //CR
	FIFO_PUSH( uart_tx_fifo, CHAR_LF );  //LF

	UCSRB |= ( 1 << UDRIE);  // Разрешаем прерывание по освобождению передатчика
	return 0;
}
//----------------------------------------------------------------
void incomingMessage(char* s)
{
	//  char strnum[15];
	//  static uint16_t num = 0;
	/*
	free(message);
	message = (char*) calloc(strlen(s)+3, sizeof(char));
	if(message == NULL){
		LCDPuts("___calloc fail");
		return;
	}
	strcpy(message, s);
	*/
	//  strnum[0] = '_';
	//  itoa(num, strnum+1, 10);
	//  strncat(strnum, message, 11);
	//  num ++;
	//  LCD_Puts(strnum);
	
}

void renewLCD(void)
{
	if(rxb[cur_str][0] != 0) {
		LCDPuts(&rxb[cur_str][0]);
		rxb[cur_str][0] = 0;
		cur_str ++; if(cur_str == RXBUFSTRCOUNT) cur_str = 0;
	}
	LCDGotoXY(110, 0);
	itoa(str_count, buf, 10);
	LCDWriteString(buf);
}
void SIM900_PowerOn(void)
{
  sbi(PORTD, 7);			// Enable 4.0v
  cbi(PORTB, 1);			// Тянем PWRKEY вниз
  _delay_ms(1500);
  sbi(PORTB, 1);			// Отпускаем PWRKEY
  while((PINB & 0b00000001) == 0)_delay_ms(10);		// Ждем 1 на STATUS
  LCDPuts("S900TurnOn");
  uart_send("AT");
  while(rxb[cur_str][0] == 0);
  renewLCD();
  _delay_ms(300);
  renewLCD();
  	
}
