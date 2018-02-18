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
#include "HD44780SPI.h"
#include <util/delay.h>
#include "uart.h"
#include "vodyanoy2.0.h"

#define RXBUFMAXSIZE 64
#define RXBUFSTRCOUNT 4
uint16_t char_count = 0, str_count = 0,  mode = 0;
volatile uint16_t cur_str = 0, TimeoutTackts = 0;
char buf[23], str[23], rxb[RXBUFSTRCOUNT][RXBUFMAXSIZE];
FIFO( 128 ) uart_tx_fifo;

ISR(USART__RXC_vect);  //����� ���������� ���������� � c:\Program Files\Atmel\AVR Studio 5.1\extensions\Atmel\AVRGCC\3.3.1.27\AVRToolchain\avr\include\avr\iom32a.h (��� mega32a) ������!!!
ISR(USART__UDRE_vect);
ISR(INT1_vect);         //CLK �� �����
ISR(TIMER1__COMPA_vect); //��������� ���������� ��������1. ������� 10��.  ����� ���������� ���������� � c:\Program Files\Atmel\AVR Studio 5.1\extensions\Atmel\AVRGCC\3.3.1.27\AVRToolchain\avr\include\avr\iom8a.h (��� mega8a)


void incomingMessage(char* s);
void renewLCD(void);
void SIM900_PowerOn(void);
void SIM900_WaitRegistration(void);
uint8_t waitAnswer(char *answer, uint16_t timeout); 
void dropMessage(void);



int main(void)
{
  DDRC  = 0b00100000;    // 7-�, 6-18b20, 5-LCD_RESET, 4-0 - �
  PORTC = 0b00100000;
  DDRD =  0b10000010;    // 7-En4V, 6-3 �����, 2-������, 1-TX, 0-RX
  PORTD = 0b01111100;
  DDRB  = 0b10111010;    // 7-LCD_CLK, 6-LCD_BUSY, 5-LCD_MOSI, 4-LCD_SS, 3 - ��������� �������, 2-power_fail, 1 - SIM900_pwrkey, 0 - SIM900_status
  PORTB = 0b00000010;

  sbi(GICR, 7);          // ��������� INT1  ��� �����
  sbi(MCUCR, 3); cbi(MCUCR, 2);  // ���������� �� �������� 1->0  �� ����� CLK ����� 

      /* ������������� 16-������� �������1 ( 10��) */
  TCCR1A =  0b00000000;	// CTC. ������� �� OCR1A
  TCCR1B =  0b00001100;	// �������� ��� �������1 - 100 - 256
  OCR1A = 3125;			// ������� �� 3125	���������� ����� �������������� � �������� 10 ��
  TIMSK = 0b00010000;		// �� ���������� ���������1 � OCR1A
  
  uart_init();
  LCD_init();
  sei();
  
  for(uint8_t i = 0; i < RXBUFSTRCOUNT; i ++)
    for(uint16_t j = 0; j < RXBUFMAXSIZE; j ++)
      rxb[i][j] = 0;
      
  LCD_Puts("�����");
    
  SIM900_PowerOn();   
  LCD_Puts("�����������");
  SIM900_WaitRegistration();
  LCD_Puts("��");

  LCD_Puts("GPRS ������");
  uart_send("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"");
  waitAnswer("OK", 20);
  uart_send("AT+SAPBR=3,1,\"APN\",\"internet.tele2.ru\"");
  waitAnswer("OK", 20);
  uart_send("AT+SAPBR=3,1,\"USER\",\"\"");
  waitAnswer("OK", 20);
  uart_send("AT+SAPBR=3,1,\"PWD\",\"\"");
  waitAnswer("OK", 20);
  do{
    uart_send("AT+SAPBR=1,1");
  } while(waitAnswer("OK", 20) != 1);
  LCD_Puts("��");

  LCD_Puts("IP:");
  uart_send("AT+SAPBR=2,1");
  while(rxb[cur_str][0] == 0); dropMessage();  // ������� ���
  while(rxb[cur_str][0] == 0); 
  strncpy(str, &rxb[cur_str][13], 14); dropMessage();
  LCD_Puts(str);
  
  LCD_Puts("drumir.16mb.com");
  uart_send("AT+HTTPINIT");
  waitAnswer("OK", 20);
  uart_send("AT+HTTPPARA=\"CID\",1");
  waitAnswer("OK", 20);
  uart_send("AT+HTTPPARA=\"URL\",\"http://drumir.16mb.com/merman.php\"");
  waitAnswer("OK", 20);
  uart_send("AT+HTTPACTION=0");   // ������� �����: ��� / ��, / +HTTPACTION:0,200,20
  while(rxb[cur_str][0] == 0); dropMessage();     // �������� ���
  while(rxb[cur_str][0] == 0); dropMessage();     // �������� "��"
  while(rxb[cur_str][0] == 0);
  if(strncmp(&rxb[cur_str][0], "+HTTPACTION:0,200,", 18) == 0) 
    LCD_Puts("��");
  else {
    strncpy(str, &rxb[cur_str][14], 3);
    str[3] = '\0';
    LCD_Puts(str);
  }
  dropMessage();

  uart_send("AT+HTTPREAD=0,20");
  while(rxb[cur_str][0] == 0); dropMessage();     // �������� ���
  while(rxb[cur_str][0] == 0); dropMessage();     // �������� "+HTTPREAD:22"
  while(rxb[cur_str][0] == 0);
  LCD_Puts(&rxb[cur_str][1]);dropMessage();
  while(rxb[cur_str][0] == 0); dropMessage();     // �������� "��"

  while (1) 
    {
    while(rxb[cur_str][0] == 0);
    renewLCD();
    _delay_ms(200);
    }
}

//------------------------------------------------------------------------------
ISR(INT1_vect)   //CLK �� �����                          ����������
{
  sei();
  //char str[20];
  unsigned char bstat;  
  bstat = PIND & 0b01111000;
  if (bstat == 0b01010000)       // ������ ���� 
  {
    cbi(PORTB, 1);
    _delay_ms(1500);
    sbi(PORTB, 1);
    LCD_Puts("*TurnOFF*");
    while((PINB & 0b00000001) == 1)_delay_ms(10);      // ���� 0 �� STATUS
    LCD_Puts("*PowerDown*");
    cbi(PORTD, 7);      // Disable 4.0v
  }
  if (bstat == 0b01100000)       // ������ �����
  {
  }
  if (bstat == 0b00110000)       // ������ �����
  {
    //LCDPuts(&rxb[cur_str][0]);
    LCD_clear();
    uart_send("AT+HTTPREAD=0,20");
	  while(rxb[cur_str][0] == 0);renewLCD();
	  while(rxb[cur_str][0] == 0);renewLCD();
	  while(rxb[cur_str][0] == 0);renewLCD();
	  while(rxb[cur_str][0] == 0);renewLCD();
	  while(rxb[cur_str][0] == 0);renewLCD();
	  while(rxb[cur_str][0] == 0);renewLCD();


  }
  
  if (bstat == 0b01110000)      // ������ ������
  {   
    uart_send("AT+SAPBR=1,1");
    _delay_ms(100);renewLCD();_delay_ms(100);renewLCD();_delay_ms(2000);
    uart_send("AT+SAPBR=2,1");
    _delay_ms(100);renewLCD();_delay_ms(100);renewLCD();_delay_ms(2000);

	  uart_send("AT+HTTPINIT");
	  waitAnswer("OK", 20);
	  uart_send("AT+HTTPPARA=\"CID\",1");
	  waitAnswer("OK", 20);
	  uart_send("AT+HTTPPARA=\"URL\",\"http://drumir.16mb.com/merman.php\"");
	  waitAnswer("OK", 20);
	  uart_send("AT+HTTPACTION=0");
	  while(rxb[cur_str][0] == 0);renewLCD();
	  while(rxb[cur_str][0] == 0);renewLCD();
	  _delay_ms(2000);
	  uart_send("AT+HTTPREAD=0,14");
  }
}
//------------------------------------------------------------------------------
ISR(TIMER1_COMPA_vect) //��������� ���������� ��������1. ������� 10��. (��� 8���)
{
if(TimeoutTackts > 0) TimeoutTackts --; 
}
//----------------------------------------------------------------
 
ISR(USART_RXC_vect) //���������� ���������� �� ��������� ����� �����
{
  static uint16_t length = 0, strnum = 0;
  static char rxbuf[RXBUFMAXSIZE];
  uint8_t rxbyte;
  rxbyte = UDR;
  rxbuf[length] = rxbyte; 
//  sei();
  length ++;
  if(length == RXBUFMAXSIZE - 2){      // ���� ������ ��������� ����� �������� ������
    rxbuf[length] = 0;          // ������� � ����� '/0' 
    strcpy(&rxb[strnum][0], rxbuf);          // ��������� ��� � �������� ���������
    strnum ++; if(strnum == RXBUFSTRCOUNT) strnum = 0;
    length = 0;
    str_count ++;
  }
  if(rxbyte == CHAR_LF && length > 1 && rxbuf[length-2] == CHAR_CR){    // ���� ������� ������ CHAR_CR � CHAR_LF
    if(length == 2)          // � ����� ��� ������ ������ ���
      length = 0;          // ��������� �� ���
    else{                  // ����� ��� ������ ���������
      rxbuf[length] = 0;        // ������� � ����� '/0'
      strcpy(&rxb[strnum][0], rxbuf);          // ��������� ��� � �������� ���������
      strnum ++; if(strnum == RXBUFSTRCOUNT) strnum = 0;
      length = 0;
      str_count ++;
    }
  }
}
//----------------------------------------------------------------
void uart_init( void )
{
  //��������� �������� ������
  UBRRH = 0;
  UBRRL = 12;      // ��� 8��� 51 - 9600+0.2%������,  25 - 19200+0,2%������,  12 - 38400
  //8 ��� ������, 1 ���� ���, ��� �������� ��������
  UCSRC = ( 1 << URSEL ) | ( 1 << UCSZ1 ) | ( 1 << UCSZ0 );
  //��������� ��������, ����� ������ � ���������� �� ����� �����
  UCSRB =  ( 1 << TXEN ) | ( 1 << RXEN ) | (1 << RXCIE );
}


ISR(USART_UDRE_vect)     // ���������� �� ����������� �������� ������
{
  cli();
  if( FIFO_IS_EMPTY( uart_tx_fifo ) ) {
    UCSRB &= ~( 1 << UDRIE ); //���� ������ � fifo ������ ��� �� ��������� ��� ����������
  }
  else {
    //����� �������� ��������� ����
    char txbyte = FIFO_FRONT( uart_tx_fifo );
    FIFO_POP( uart_tx_fifo );
    UDR = txbyte;
  }
  sei();
}

int uart_send(char *str)
{
  for(int i = 0; str[i] != 0; i ++)    //�������� ������ � ����� �����������
  FIFO_PUSH( uart_tx_fifo, str[i] );

  FIFO_PUSH( uart_tx_fifo, CHAR_CR );  //CR
  FIFO_PUSH( uart_tx_fifo, CHAR_LF );  //LF

  UCSRB |= ( 1 << UDRIE);  // ��������� ���������� �� ������������ �����������
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

//----------------------------------------------------------------
void renewLCD(void)
{
  if(rxb[cur_str][0] != 0) {
    LCD_Puts(&rxb[cur_str][0]);
    rxb[cur_str][0] = 0;
    cur_str ++; if(cur_str == RXBUFSTRCOUNT) cur_str = 0;
  }
  LCD_gotoXY(8, 0);
  itoa(str_count, buf, 10);
  LCD_writeString(buf);
}

//----------------------------------------------------------------
void SIM900_PowerOn(void)
{
  sbi(PORTD, 7);      // Enable 4.0v 
  _delay_ms(200);
  
  if((PINB & 0b00000001) != 0)   // ������ ��� �������
    return;
     
  cbi(PORTB, 1);      // ����� PWRKEY ����
  _delay_ms(1500);
  sbi(PORTB, 1);      // ��������� PWRKEY
  while((PINB & 0b00000001) == 0)_delay_ms(50);    // ���� 1 �� STATUS
  uart_send("AT");
  while(rxb[cur_str][0] == 0);dropMessage();
  while(rxb[cur_str][0] == 0);dropMessage();
}

//----------------------------------------------------------------
void SIM900_WaitRegistration(void)
{
uint8_t sucess_flag = 0;
  do{
    uart_send("AT+CREG?");
    while(rxb[cur_str][0] == 0);dropMessage();        // ����������� ���
    while(rxb[cur_str][0] == 0);
    if(strncmp(&rxb[cur_str][0], "+CREG: 0,1", 10) == 0) sucess_flag = 1; // ����������� �����
    dropMessage();  
    while(rxb[cur_str][0] == 0);dropMessage();      // ����������� "��"
  }while(sucess_flag == 0);
  
}

uint8_t waitAnswer(char *answer, uint16_t timeout)  // ������� ������ �� sim900, ���������� � �������. ���� �����, ���������� 1. �� �������� (� ���/10) ���������� 0
{
  TimeoutTackts = timeout;			// �������� ������ ��������
  while(1){
    while(rxb[cur_str][0] == 0 && TimeoutTackts != 0);
    if(TimeoutTackts == 0)                                          // ���� ����� �� ����� �� ��������, ���������� 0 
      return 0;
    if(strncmp(&rxb[cur_str][0], answer, strlen(answer)) == 0){     // ���� ������� ������ �����, ���������� 1
      dropMessage();
      return 1;                                                
    }
    dropMessage();		// "�����������" ��������� ��������
  } 
  return 0;
}

void dropMessage(void)
{
  rxb[cur_str][0] = 0; cur_str ++; if(cur_str == RXBUFSTRCOUNT) cur_str = 0;
}