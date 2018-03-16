/*
 * Sim900.c
 *
 * Created: 24.08.2017 22:26:28
 * Author : Drumir
 */ 

#include "vodyanoy2.0.h"

volatile uint16_t TimeoutTackts = 0;


int main(void)
{
	ACSR |= 0x80;			// �������� �� ������, �� ��������� �� ��������� ���������� ����������
  DDRC  = 0b01110100;    // 7-�, 6-DQ DS18B20, 5-LCD_RESET, 4-�����, 3-�, 2-���, 1-�, 0-�
  PORTC = 0b01100000;
  DDRD =  0b10000010;    // 7-En4V, 6-3 �����, 2-������, 1-TX, 0-RX
  PORTD = 0b01111000;
//  DDRB  = 0b10111010;    // 7-LCD_CLK, 6-LCD_BUSY, 5-LCD_MOSI, 4-LCD_SS, 3 - ��������� �������, 2-power_fail, 1 - SIM900_pwrkey, 0 - SIM900_status
  DDRB  = 0b00001010;    // 7-4 - �, 3 - ��������� �������, 2-power_fail, 1 - SIM900_pwrkey, 0 - SIM900_status
  PORTB = 0b00000010;
	DDRA =  0b00000000;			// 7-1-�, 0 - ADC Vbat
	PORTA = 0b00000000;


  sbi(GICR, 7);          // ��������� INT1  ��� �����
  sbi(MCUCR, 3); cbi(MCUCR, 2);  // ���������� �� �������� 1->0  �� ����� CLK ����� 

      /* ������������� 16-������� �������1 ( 10��) */
  TCCR1A =  0b00000000;	// CTC. ������� �� OCR1A
  TCCR1B =  0b00001100;	// �������� ��� �������1 - 100 - 256
  OCR1A = 3125;			// ������� �� 3125	���������� ����� �������������� � �������� 10 ��
  TIMSK = 0b00010000;		// �� ���������� ���������1 � OCR1A

          	/* ������������� ��� */
            // ��� En,  not now,  single mode, reset iflag, INTs Enable,       ������������ �������
  ADCSRA = 1 << ADEN | 0 << ADSC | 0 << ADATE | 0 << ADIF | 1 << ADIE | 1 << ADPS2 | 1 << ADPS1 | 1 << ADPS0; 
//	ADCSRA = 0b10001111;		// 1 - ���, 0 - ��� �� �����, 0 - ����������, 0 - ���������� ������������, 0 - ���������� ��� ���������, 111 - ������������ ������� 128
	ADMUX  = 0b11000000;		// 11 - ������� ���������� = 2,56�, 0 - ������������ ������, 0 - ������, 0 - ������, 000 - ����� ������ ADC0
	ADCSRA |= 1<<ADSC;		// ����� �������� ��������� ��������������  

	for(uint16_t i = 0; i < RXBUFSTRCOUNT; i ++) rx.ptrs[i] = -1;   // �������� ������ ���������� �� ������ �������� ���������
	rx.wptr = 0;                                           // ��������� ��������� ������ �� ������ ������
	rx.startptr = 0;                                       // ������ ������������� ���������
	rx.buf_overflow_count = 0;
	rx.ptrs_overflow_count = 0;
	rx.buf[RXBUFMAXSIZE] = 0;                               // ������� ����� ����� ������ '\0' ����� ��������� ������� �� ����� � ���
  
  SIM900Status = SIM900_NOTHING;
	MenuMode = MD_STAT;
	LightLeft = LIGHT_TIME; LIGHT_ON;	// ����� ������� ���������
	PumpPause = 0;
	Seconds = 0;
	CheckUPause = 0;
	SilentLeft = FIRST_CONNECT_DELAY;
  
  Now.yy = 0;         // �� ���� � ���������� ��� ���������, ��� Now ��� �� �����������������
  remoteSettingsTimestamp.yy = 0;

  eeprom_read_block(&options, (const void*)0x00, sizeof(struct TSettings));


//	if(options.fFreezeNotifications == 0xFF || options.ConnectPeriod == 0xFFFF) 	// ���� ���������� ����� (����� ������������) ������� �������� � �����������.
	if(0) 	// ���� ���������� ����� (����� ������������) ������� �������� � �����������.
	{
  	options.FrostFlag = 0;
  	options.PumpWorkFlag = 0;
    options.fFreezeNotifications = 0;           // ����� ���������� � ���������. 1 � 3 ���� - ��� ���������. �� 2 ���� - ��� ������. � 1 - ������ ���������. � 0 - ������ ������.
    options.fWarmNotifications = 0;             // ����� ���������� � ���������.
    options.fDoorNotifications = 0;             // ����� ���������� �� �������� �����.
    options.fFloodNotifications = 0;            // ����� ���������� � ����������.
    options.fPowerNotifications = 0;            // ����� ���������� � ������� ����������������.
    options.fPowerRestNotifications = 0;        // ����� ���������� � �������������� ����������������.
    options.fOfflineNotifications = 0;          // ����� ���������� � ���������� ���������� ���������.
    options.fBalanceNotifications = 0;          // ����� ���������� � ����������� ������� �� �����
    options.fDailyNotifications = 0;            // ����� ����������� ���������� � ���������
    strcpy((char*)options.OperatorTel, "9027891301");  // ����� �������� ���������
    strcpy((char*)options.AdminTel, "9040448302");  // ����� �������� ��������������
  	options.PumpWorkDuration = 120;             // ������� ������� ������ �������� ����� (� �������)
  	options.PumpRelaxDuration = 240;            // ������� ������� ������ �������� ����� (� �������)
    options.ConnectPeriod = 5;                  // ������ (� �������) �������� �� ������. ���� 0 - ������ ��� ������������� ��� �� ������
    options.DisconnectionTime = 120;            // ������������ ���������� ����� ��� ���������� � ��������� �� ������
    options.MinBalance = 10;                    // ������, ��� ���������� �������� ����� ���������� � ����������� �������
    options.DailyReportTime = 960;              // ����� ����������� ������ � ��������� (� ������� � ������ �����)
    options.PumpTimeLeft = 0;		                // ������� �������� ������/�������� ������ (� �������?)
    options.HeaterOnTemp = 2;				            // ����������� ���������� ������������
    options.HeaterOffTemp = 10;			            // ����������� ��������� ������������
    options.FreezeTemp = 0;                    // ����������� �������������� � ���������
    options.WarmTemp = 40;                      // ����������� �������������� � ���������
    options.localSettingsTimestamp.yy = 0;      // ����� ���������� ���������� ��������� ��������
    options.localSettingsTimestamp.MM = 0;
    options.localSettingsTimestamp.dd = 0;
    options.localSettingsTimestamp.hh = 0;
    options.localSettingsTimestamp.mm = 0;
    options.localSettingsTimestamp.ss = 0;
	}
  
//  Save();

	if(options.PumpWorkFlag == 1 && options.PumpTimeLeft != 0)		// ���� �� ����������� ���������� ����� ��� �������
	PumpPause = PUMP_RESTART_PAUSE/2;						// �� ������ ������ �������� �� �������� "����� ����� ����������"
    
  uart_init();
  sei();
  
//  SIM900_SendReport();


  while (1) 
  {
    if(SilentLeft == 0)    // ������� ������ �� ������ �����
    {
      SilentLeft = 300;    // ��������� ����� ����� ����� 5 �����
      SIM900_PrepareConnection();
      if(SIM900Status >= SIM900_UP){
        if (Now.yy == 0){
          SIM900_GetTime();              // ����� ������� ����� ������������� �����.
          RecToHistory(EVENT_START);     // ���� ����� ������ - ������ � ������� ������� ������
        }         
        else SIM900_GetTime();              
      }  
       
      if(SIM900Status >= SIM900_GPRS_OK)    // ���� �� ������ �� � ������� ������� ����� ����� � ��������
      {
        SIM900_GetRemoteSettingsTimestamp();   // ������� ����� ���������� ��������� �������� �� �������
        if(timeCompare(&options.localSettingsTimestamp, &remoteSettingsTimestamp) > 0)   // ���� ��������� ��������� �����
        {
          SIM900_SendSettings();                                                  // ������� �� �� ������
        } 
        else if (timeCompare(&options.localSettingsTimestamp, &remoteSettingsTimestamp) < 0) // ���� ��������� �� ������� �����
        {
          SIM900_GetSettings();                                                       // ������� � ������ ��
        }
        strcpy(str, "");
        itoa(options.PumpWorkDuration ,buf, 10); strcat(str, buf); strcat(str, " ");
        itoa(options.PumpRelaxDuration ,buf, 10); strcat(str, buf); strcat(str, " ");
        itoa(options.HeaterOnTemp ,buf, 10); strcat(str, buf); strcat(str, " ");
        itoa(options.HeaterOffTemp ,buf, 10); strcat(str, buf); strcat(str, " ");
        itoa(options.ConnectPeriod ,buf, 10); strcat(str, buf); strcat(str, " ");
        itoa(options.FreezeTemp ,buf, 10); strcat(str, buf); strcat(str, " ");
        itoa(options.fFreezeNotifications ,buf, 10); strcat(str, buf); strcat(str, " ");
        itoa(options.WarmTemp ,buf, 10); strcat(str, buf); strcat(str, " ");
        itoa(options.fWarmNotifications ,buf, 10); strcat(str, buf); strcat(str, " ");
        itoa(options.fPowerNotifications ,buf, 10); strcat(str, buf); strcat(str, " ");
        itoa(options.DailyReportTime ,buf, 10); strcat(str, buf); strcat(str, " ");
        strcat(str, options.OperatorTel); strcat(str, " ");
        strcat(str, options.AdminTel);
        uart_send(str);
        _delay_ms(1000);
//        SIM900_SendStatus();                                                          // ������� �� ������ ������� ���������
//        SIM900_SendHistory();                                                         // ������� �� ������ ������� �������
      }
    }    
  }
}

//------------------------------------------------------------------------------
void OneMoreMin(void)
{
  static uint8_t Min = 0;
  Min ++;
  if (Min == 60)
  {
    Min = 0;
    //SIM900_GetBalance();
  }

  if(Min%5 == 0){
    //SIM900_SendReport();
  }  

  if(State.Vbat < 660){         // 3.3V
    //uart_send("AT+CPOWD=1");   // �������� ������
  }
}
//---------------------------------------------------------------------
void OneMoreSec(void)
{
  uint16_t a;
  sei();
  Seconds ++;
  Now.ss ++; 
  if (Now.ss == 60)
  {
    Now.ss = 0; Now.mm ++;
    if(Now.mm == 60)
    {
      Now.mm = 0;Now.hh ++;
      if(Now.hh == 24){Now.hh = 0; Now.dd ++;}
    }
  }

  if(PumpPause > 0) PumpPause --;	// ����������� ����� ����� ��������� ���������� ������
  if(options.PumpWorkFlag == 1 && (PORTC & 0b00010000) == 0 ) // ���� ����� ������ ���� �������, �� �� ��������,
  {
    a = options.PumpTimeLeft;										// �������� ���������� �����
    PumpStart();										// ����� ������� ����� (���������� ����� ��� ���� ������ ������ ��������� � ����������)
    options.PumpTimeLeft = a;										// ������ ���������� �����
  }

  if(LightLeft > 0) LightLeft --; 
  if(LightLeft == 0) {LightLeft = -1; LIGHT_OFF; /*Save();*/ MenuMode = MD_STAT;}

      // ????????????
  if(PumpPause != 0 && options.PumpTimeLeft < 3)	// ���� ������ �� ����� ����� ���������� � ����� ��������� - ����� �� �������������!
  {
    Seconds --;
    return;
  }
  
  if(Seconds == 33){         // ������ 56 �������
    sensor_write(0x44);   // ����� ��������� �����������
  }

  if(Seconds == 35){     // ������ 58 ������� - ������ �����������
    State.Temp = sensor_write(0xBE); // ������ ������������� ������ c dc18_B_20 / dc18_S_20
    //  Temp >>= 4; // 4
    if(State.Temp <= options.HeaterOnTemp*16) HeaterStart();	//
    if(State.Temp >= options.HeaterOffTemp*16) HeaterStop();	// ��� ������������� ������� ��� �������� ������������
    if(State.Temp < -3 && options.PumpWorkFlag == 0) options.FrostFlag = 1;
  }


  if(Seconds == 60)			// ������ ��� ���� ������
  {
    Seconds = 0;
    
    if(options.PumpWorkDuration != 0 && options.PumpRelaxDuration != 0)	//�������� ��� ����� ������ � ������ ������ �� ����� 0. ���� ����� - �������� ��������������� ���������/����������!
    {
      options.PumpTimeLeft --;
      if(options.PumpTimeLeft <= 0 && options.PumpWorkFlag == 1)PumpStop();   // ����������� ����� ������ ������
      if(options.PumpTimeLeft <= 0 && options.PumpWorkFlag == 0)              // ����������� ����� ������ ������
        if(PumpStart() == 0)    // ���� ������ ������ �� ������
        {
          //LCD_gotoXY(0, 2); LCD_writeString(str);		//���� ����� ������ ������ ������ - ��������� �
          options.PumpTimeLeft ++; 									//� �������� ��� ���� ������
        }
    }
    OneMoreMin();
  }
  if(SilentLeft > 0) SilentLeft --;     // ������� ������ �� ������ �����

//  if(MenuMode == MD_STAT)ShowStat();
  
}

//------------------------------------------------------------------------------
ISR(ADC_vect)          // ���������� �������������� ���
{
  	State.Vbat = ADC;		// ����������� �������� ������� � ������
}
//------------------------------------------------------------------------------

ISR(INT1_vect)   //CLK �� �����                          ����������
{
  sei();
  unsigned char bstat;  
  bstat = PIND & 0b01111000;
  if (bstat == 0b01010000)       // ������ ���� 
  {
    PORTC &= 0b11101111;  //��������� �����
    /*
    cbi(PORTB, 1);
    _delay_ms(1500);
    sbi(PORTB, 1);
    LCD_Puts("*TurnOFF*");
    while((PINB & 0b00000001) == 1)_delay_ms(10);      // ���� 0 �� STATUS
    LCD_Puts("*PowerDown*");
    cbi(PORTD, 7);      // Disable 4.0v
    */
  }
  if (bstat == 0b01100000)       // ������ �����
  {
    PORTC |= 0b00010000;  //��������� �����
  }
  if (bstat == 0b00110000)       // ������ �����
  {
  }
  
  if (bstat == 0b01110000)      // ������ ������
  {  
  }
}
//------------------------------------------------------------------------------
ISR(TIMER1_COMPA_vect) //��������� ���������� ��������1. ������� 10��. (��� 8���)
{
  static uint16_t Tacts = 0;
  Tacts ++;

  if(TimeoutTackts > 0) TimeoutTackts --; 

  if(Tacts == 10 )		// (8 000 000 / 256) / 3125 = 10
  {
    Tacts = 0;
    OneMoreSec();
  }
}
//----------------------------------------------------------------
ISR(USART_RXC_vect) //���������� ���������� �� ��������� ����� �����
{
	int16_t nextwptr, prevwptr, i;
	if(rx.wptr == rx.startptr && rx.ptrs[0] == -1){   // ���� ��� ������ ���� ����� ������ � ��� ������������� �����
  	rx.wptr = 0;
  	rx.startptr = 0;    // ������� ��� ������ � ����� ������ ������
	}
	rx.buf[rx.wptr] = UDR;
	nextwptr = rx.wptr + 1;
	prevwptr = rx.wptr - 1;
	if(rx.buf[rx.wptr] == CHAR_LF && rx.wptr > 0 && rx.buf[prevwptr] == CHAR_CR){  // ���� ������� ������ CHAR_CR � CHAR_LF
  	rx.buf[prevwptr] = '\0';                                      // ������� ����-���������� (�������(��������) CHAR_CR � CHAR_LF
  	if(prevwptr != rx.startptr){                                  // ���� ��� ����������, �������������� ������(������� �� ������ �� CR LF)
    	i = 0;
    	while(rx.ptrs[i] != -1 && i < RXBUFSTRCOUNT) i ++;    // ������ � rx.ptrs ������ ��������� ������
    	if(i == RXBUFSTRCOUNT) { // ����-���� - ��������� ����� ��� �������� ����� (rx.ptrs)
      	rx.ptrs_overflow_count ++;
      	i = RXBUFSTRCOUNT-1;      // ����������� ��������� ���������
      	//return; 								// ��� ������ �������� ��� (��������� ���������)
    	}
    	rx.ptrs[i] = rx.startptr;   // ������ � rx.ptrs �������� ����� �������� ������
    	rx.startptr = rx.wptr;      // �������� ��� �������� ���� ���������
  	}
  	else {    // ������ ������ CR+LF
    	rx.wptr = prevwptr;
  	}
	}
	else {      // ������ ��� ���� ������ ������� � �����
  	if(nextwptr == RXBUFMAXSIZE){ // ����-���� - ����� ����������
    	rx.buf_overflow_count ++;   // �������� �����. �� ������ �� �������������� rx.wptr, � ���������� �� ����������.
    	rx.buf[prevwptr] = CHAR_CR; // ����� ������ CHAR_LF, ������� ������ ��������� ��� ������
  	}
  	else
  	rx.wptr = nextwptr;
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
//----------------------------------------------------------------
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
//----------------------------------------------------------------
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
}
//----------------------------------------------------------------
uint8_t waitAnswer(char *answer, uint16_t timeout)  // ������� ������ �� sim900, ���������� � �������. ���� �����, ���������� 1. �� �������� (� ���/10) ���������� 0
{
  TimeoutTackts = timeout;			// �������� ������ ��������
  while(1){
    while(rx.ptrs[0] == -1 && TimeoutTackts != 0);
    if(TimeoutTackts == 0)                                          // ���� ����� �� ����� �� ��������, ���������� 0 
      return 0;
    if(strncmp((char*)rx.buf+rx.ptrs[0], answer, strlen(answer)) == 0){     // ���� ������� ������ �����, ���������� 1
      dropMessage();
      return 1;                                                
    }
    dropMessage();		// "�����������" ��������� ��������
  } 
  return 0;
}
//----------------------------------------------------------------
void waitMessage(void)
{
  while(rx.ptrs[0] == -1);
}
//----------------------------------------------------------------
void dropMessage(void)
{
	uint8_t i;
	for (i = 0; i < RXBUFSTRCOUNT-1 && rx.ptrs[i] != -1; i ++)
	rx.ptrs[i] = rx.ptrs[i+1];
	rx.ptrs[RXBUFSTRCOUNT-1] = -1;
}
//----------------------------------------------------------------
int16_t str2int(char* str)
{
  uint8_t minus = 1;
  uint16_t result = 0;
  while(*str != '\0' && (*str > '9' || *str < '0')) str ++;   // ���� ������ ����� ��� ����� ������
  if(*str == '\0') return 0;                                  // ���� ����� ����� ������, ���������� 0
  if(*(str-1) == '-') minus = -1;                             // ���� �����, ������� ��� �� ����� ��� ������
  result = *str - '0';
  str ++;
  while(*str <= '9' && *str >= '0'){
    result  *= 10;
    result += *str - '0';
    str ++;
  }
  return result * minus;
}
//---------------------------------------------------------------------
void Save(void)
{
  if(options.PumpWorkDuration == 0 || options.PumpRelaxDuration == 0) PumpStop();  // ��� ��������� ����� ���� � ���� ������ ������� ������������ ������ ��� ������ ������

//  if((uint16_t)eeprom_read_word((uint16_t *)0x0000) == 0 && PumpRelaxDuration != 0) PumpTimeLeft = PumpRelaxDuration; //PumpStop();		���-�� ��� ����������........
  eeprom_write_block(&options, (void*)0x00, sizeof(struct TSettings));
}
//---------------------------------------------------------------------
void PumpStop(void)
{
  options.PumpTimeLeft = options.PumpRelaxDuration;
  options.PumpWorkFlag = 0;
  PORTC &= 0b11101111;			// �������� ����� (�� �� PC4)
  if(PumpPause < 1) PumpPause = PUMP_RESTART_PAUSE;
}
//---------------------------------------------------------------------
uint8_t PumpStart(void)
{
  if(options.PumpWorkDuration == 0  || options.PumpRelaxDuration == 0){strcpy(str, "��� ����������"); return 0;}
  if(options.FrostFlag == 1){strcpy(str, "����.���������"); return 0;}
  if(PumpPause > 0){ strcpy(str, "����� "); itoa(PumpPause/10, buf, 10); strcat(str, buf); strcat(str, " ���   ");return 0;}
  options.PumpTimeLeft = options.PumpWorkDuration;
  CheckUPause = 20;		// 2 ������� �� ��������� �������� ����������!
  options.PumpWorkFlag = 1;
  PORTC |= 0b00010000;	// ������� ����� (�� �� PC4)
  return 1;
}
//---------------------------------------------------------------------
void HeaterStart(void)
{
  PORTC |= 0b00000100;	// ������� ������������ (�� �� PC2)
}
//---------------------------------------------------------------------
void HeaterStop(void)
{
  PORTC &= 0b11111011;	// �������� ������������ (�� �� PC2)
}
//---------------------------------------------------------------------
int8_t timeCompare(struct TTime *time1, struct TTime *time2)    // ���������� 1 ���� time1 ������ (�����) time2. -1 ���� ��������. 0 ���� ���� �����
{
  if(time1->yy > time2->yy) return 1;
  if(time1->yy < time2->yy) return -1;
  if(time1->MM > time2->MM) return 1;
  if(time1->MM < time2->MM) return -1;
  if(time1->dd > time2->dd) return 1;
  if(time1->dd < time2->dd) return -1;
  if(time1->hh > time2->hh) return 1;
  if(time1->hh < time2->hh) return -1;
  if(time1->mm > time2->mm) return 1;
  if(time1->mm < time2->mm) return -1;
  if(time1->ss > time2->ss) return 1;
  if(time1->ss < time2->ss) return -1;
  return 0;
}
