/*
 * Sim900.c
 *
 * Created: 24.08.2017 22:26:28
 * Author : Drumir
 */ 

#include "vodyanoy2.0.h"


int main(void)
{
	ACSR |= 0x80;			// �������� �� ������, �� ��������� �� ��������� ���������� ����������
  DDRC  = 0b11111100;    // 7-�, 6-DQ DS18B20, 5-LCD_RESET, 4-�����, 3-�������, 2-���, 1-�, 0-�
  PORTC = 0b01100000;
  DDRD =  0b10000010;    // 7-En4V, 6-3 �����, 2-������, 1-TX, 0-RX
  PORTD = 0b01111000;
  DDRB  = 0b11111010;    // 7-LCD_CLK, 6-LCD_DC, 5-LCD_MOSI, 4-LCD_SS, 3 - ��������� �������, 2-power_fail, 1 - SIM900_pwrkey, 0 - SIM900_status
  PORTB = 0b00000010;
	DDRA =  0b00000000;			// 7-1-�, 0 - ADC Vbat
	PORTA = 0b00000000;


  sbi(GICR, INT1);          // ��������� INT1 ��� �����
  sbi(GICR, INT2);          // ��������� INT2 ��� �������� �������
	
  sbi(MCUCR, ISC11); cbi(MCUCR, ISC10);		// ���������� INT1 �� �������� 1->0  �� ����� CLK ����� 
	cbi(MCUCSR, ISC2);											// ���������� INT2 �� �������� 1->0 �� �������� �������

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
	rx.buf[RXBUFMAXSIZE-1] = 0;                               // ������� ����� ����� ������ '\0' ����� ��������� ������� �� ����� � ���
  
  SIM900Status = SIM900_NOTHING;
	MenuMode = MD_STAT;
	LightLeft = LIGHT_TIME; LIGHT_ON;	// ����� ������� ���������
	PumpPause = 0;
	Seconds = 0;
	CheckUPause = 0;
	SilentLeft = FIRST_CONNECT_DELAY;
	TimeoutTackts = 0;
	TimeoutsCount = 0;
	State.PowerFailFlag = 0;
	State.OpenDoorFlag = 0;
	State.FloodingFlag = 0;

  
  Now.yy = 0;         // �� ���� � ���������� ��� ���������, ��� Now ��� �� �����������������
  remoteSettingsTimestamp.yy = 0;

  eeprom_read_block(&options, (const void*)0x00, sizeof(struct TSettings));


	if(options.fFreezeNotifications == 0xFF || options.ConnectPeriod == 0xFFFF) 	// ���� ���������� ����� (����� ������������) ������� �������� � �����������.
//	if(1) 	// ���� ���������� ����� (����� ������������) ������� �������� � �����������.
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
    options.ConnectPeriod = 10;                  // ������ (� �������) �������� �� ������. ���� 0 - ������ ��� ������������� ��� �� ������
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
		strcpyPM((char*)options.Link, PM_link);
	}
  
  //Save();

	if(options.PumpWorkFlag == 1 && options.PumpTimeLeft != 0)		// ���� �� ����������� ���������� ����� ��� �������
	PumpPause = PUMP_RESTART_PAUSE/2;						// �� ������ ������ �������� �� �������� "����� ����� ����������"
    
  uart_init();
  spi_init();
 	_delay_ms(100);      //�� ������ ������������� SLI_LCD, ������� ������ ���� ��� ������� � �����
  LCD_init();
  sensor_write(0x44);   // ����� ��������� �����������
  _delay_ms(1000);
  State.Temp = sensor_write(0xBE); // ������ ������������� ������ c dc18_B_20 / dc18_S_20
	measureBattery();
  sei();
	ShowStat();

	LCD_gotoXY(0, 4); LCD_writePMstring(MSG_Loading);
  SIM900_PrepareConnection();
	LCD_gotoXY(0, 4); 
	switch(SIM900Status){
		case SIM900_BAT_FAIL:		{LCD_writePMstring(MSG_BatFail); break;}
		case SIM900_FAIL:				{LCD_writePMstring(MSG_SimFail); break;}
		case SIM900_GSM_FAIL:		{LCD_writePMstring(MSG_GSMFail); break;}
		case SIM900_GPRS_FAIL:	{LCD_writePMstring(MSG_GPRSFail); break;}
		default:								{LCD_writePMstring(MSG_Successful);}	
	}

    if(SIM900Status >= SIM900_UP){
	    if (Now.yy == 0){
		    SIM900_GetTime();              // ����� ������� ����� ������������� �����.
		    RecToHistory(EVENT_START);     // ���� ����� ������ - ������ � ������� ������� ������
	    }
	    else SIM900_GetTime();
    }

	if(SIM900Status >= SIM900_GSM_OK){
		LCD_gotoXY(0, 4); LCD_writeString(" Get balance  ");
		SIM900_GetBalance();
	}

  while (1) 
  {
	if(OneMoreSecCount > 0) OneMoreSec();
	if(bstat) OnKeyPress();

	CheckIncomingMessages();

    if(SilentLeft == 0)    // ������� ������ �� ������ �����
    {
      SilentLeft = options.ConnectPeriod*60;    // ��������� ����� ����� ����� options.ConnectPeriod �����
       
      if(SIM900Status >= SIM900_GPRS_OK)    // ���� �� ������ �� � ������� ������� ����� ����� � ��������
      {
				//SIM900_SetTimeFromServer();
        //SIM900_GetTime();              // ����� ������� ����� ������������� �����.
				LCD_gotoXY(0, 4); LCD_writeString("CheckServTimeS");
        SIM900_GetRemoteSettingsTimestamp();   // ������� ����� ���������� ��������� �������� �� �������
        if(timeCompare(&options.localSettingsTimestamp, &remoteSettingsTimestamp) > 0)   // ���� ��������� ��������� �����
        {
					LCD_gotoXY(0, 4); LCD_writeString("RenewServSetts");
          SIM900_SendSettings();                                                  // ������� �� �� ������
        } 
        else if (timeCompare(&options.localSettingsTimestamp, &remoteSettingsTimestamp) < 0) // ���� ��������� �� ������� �����
        {
          SIM900_GetSettings(); 
					LCD_gotoXY(0, 4); LCD_writeString("RevewLoclSetts");
			    SilentLeft = options.ConnectPeriod * 60;		// ������� ������ ������� �����
        }

				LCD_gotoXY(0, 4); LCD_writeString("Sending  Stats ");
        SIM900_SendStatus();                                                          // ������� �� ������ ������� ���������
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
    SIM900_GetBalance();
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
	OneMoreSecCount --; 
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
  if(LightLeft == 0) {LightLeft = -1; LIGHT_OFF; Save(); MenuMode = MD_STAT;}

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

	measureBattery();  

	if(MenuMode == MD_STAT)ShowStat();
  
}

//------------------------------------------------------------------------------
ISR(ADC_vect)          // ���������� �������������� ���
{
  	State.Vbat = ADC;		// ����������� �������� ������� � ������
}
//------------------------------------------------------------------------------

ISR(INT1_vect)   //CLK �� �����                          ����������
{
	LightLeft = LIGHT_TIME;			// ������� ��������� ����� ����� ��������, ��� �� �����
	LIGHT_ON;
	bstat = PIND & 0b01111000;	// ������ ������������ ������� ����� OnKeyPress()
}
//------------------------------------------------------------------------------
ISR(INT2_vect)   //CLK �� �����                          ���������� �������
{
	LightLeft = LIGHT_TIME;			// ������� ��������� ����� ����� ��������, ��� �� �����
	LIGHT_ON;
	State.PowerFailFlag = 1;
	PumpStop();
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
		OneMoreSecCount ++;		//� ������� ����� ������, ��� OneMoreSecCount != 0 � ������� OneMoreSec();
		if(OneMoreSecCount > 240) SoftReset();  //���� ������� ���� �� ���������� ��� 240 ������, �������� ������������ ����������
  }
}
//----------------------------------------------------------------
ISR(USART_RXC_vect) //���������� ���������� �� ��������� ����� �����
{
	volatile int16_t nextwptr, prevwptr, i;
	static uint8_t SmsFromAdminFlag = 0;
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
										/*������� ��������� ������������ �� ��� �� ������ � ������� "reset"  */
			if(SmsFromAdminFlag > 0)												//���� ���������� ������ ������������ ��� ��������� ��� �� ������,
			{
				SmsFromAdminFlag --;
				if(strncmp("reset", (char*)rx.buf+rx.startptr, 5) == 0)	// ���������� ����� ��� �� ������ � "reset"
					SoftReset();
			}
			if(strncmp("+CMT: \"+79040448302\"", (char*)rx.buf+rx.startptr, 20) == 0)		//��������� �� ��� �� ��� �� ������
				SmsFromAdminFlag = 2;
										/*����� ��������� ������� ������������*/
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
/*
ISR(USART_RXC_vect) //���������� ���������� �� ��������� ����� �����
{
	volatile int16_t nextwptr, prevwptr, i;
	if(rx.wptr == rx.startptr && rx.ptrs[0] == -1){   // ���� ��� ������ ���� ����� ������ � ��� ������������� �����
  		rx.wptr = 0;
  		rx.startptr = 0;    // ������� ��� ������ � ����� ������ ������
	}
	rx.buf[rx.wptr] = UDR;
	nextwptr = rx.wptr + 1;
	prevwptr = rx.wptr - 1;
	if(rx.buf[rx.wptr] == CHAR_LF && rx.wptr > 0 && rx.buf[prevwptr] == CHAR_CR)  // ���� ������� ������ CHAR_CR � CHAR_LF, ������ ��������� ���������. 
	{
  		rx.buf[prevwptr] = '\0';                                      // ������� ����-���������� (�������(��������) CHAR_CR � CHAR_LF
  		if(prevwptr != rx.startptr)                                   // ���� ��� ����������, �������������� ������(������� �� ������ �� CR LF)
			{
    		i = 0;
    		while(rx.ptrs[i] != -1 && i < RXBUFSTRCOUNT-1) i ++;    // ������ � rx.ptrs ������ ��������� ������
    		rx.ptrs[i] = rx.startptr;   // ������ � rx.ptrs �������� ����� �������� ������
    		rx.startptr = rx.wptr;      // �������� ��� �������� ���� ���������
  		}
  		else {    // ������ ������ CR+LF
    		rx.wptr = prevwptr;
  		}

		i = 0;
		while(rx.ptrs[i] != -1 && i < RXBUFSTRCOUNT-1) i ++;    // �������� �� ���������� �� � ��� ������ ��� ��������� �� �������� ������  (rx.ptrs)
		if(i == RXBUFSTRCOUNT-1) { // ����-���� - ��������� ����� ��� �������� ����� (rx.ptrs)
 			rx.ptrs_overflow_count ++;
			dropMessage();					// �������� ����� ������(������) ���������
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
}*/
//----------------------------------------------------------------
void OnKeyPress(void)
{
	if (bstat == 0b01010000)       // ������ ����
	{
		switch(MenuMode)
		{
			case MD_DIRPUMP:{ PumpStop(); LCD_gotoXY(0, 3);LCD_writeString("   ���������  "); break;}
			case MD_DIRHEATER:{ HeaterStop(); LCD_gotoXY(0, 3);LCD_writeString("   ���������  "); break;}
			case MD_PUMPWORKTIME:
			{
				if(options.PumpWorkDuration == 0) options.PumpWorkDuration = 420;
				else options.PumpWorkDuration -= DELTA_TIME;
				settingsWasChanged = 1;
				DrawMenu();
				break;
			}
			case MD_PUMPRELAXTIME:
			{
				if(options.PumpRelaxDuration == 0) options.PumpRelaxDuration = 2160;
				else options.PumpRelaxDuration -= DELTA_TIME;
				settingsWasChanged = 1;
				DrawMenu();
				break;
			}

			case MD_MIN_T:
			{
				if(options.HeaterOnTemp > 0) options.HeaterOnTemp --;
				DrawMenu();
				settingsWasChanged = 1;
				break;
			}

			case MD_MAX_T:
			{
				if(options.HeaterOffTemp-1 > options.HeaterOnTemp) options.HeaterOffTemp --;
				DrawMenu();
				settingsWasChanged = 1;
				break;
			}
			case MD_STAT:
			{
				SilentLeft = 2;
				break;
			}
			case MD_CLEAR:			// ������ ����������
			{
				options.FrostFlag = 0;
				PumpPause = 0;
				MenuMode = MD_STAT;
				DrawMenu();
				break;
			}
		}
	}
	if (bstat == 0b01100000)       // ������ �����
	{
		switch(MenuMode)
		{
			case MD_DIRPUMP:				// �������� ����� �������
			{
				LCD_gotoXY(0, 3);
				if(PumpStart() == 0) LCD_writeStringInv(strD);	//���� ����� ������ ������ ������ - ���������� �
				else { LCD_writeString("  ��������    ");}		// ���� ��� ��. � ������� ������� ������ ����� ����� ������ �������� � ����
				break;
			}
			case MD_DIRHEATER:{ HeaterStart(); LCD_gotoXY(0, 3);LCD_writeString("  ��������    "); break;}
			case MD_PUMPWORKTIME:
			{
				options.PumpWorkDuration += DELTA_TIME;
				if(options.PumpWorkDuration > 420) options.PumpWorkDuration = 0;
				settingsWasChanged = 1;
				DrawMenu();
				break;
			}
			case MD_PUMPRELAXTIME:
			{
				options.PumpRelaxDuration += DELTA_TIME;
				if(options.PumpRelaxDuration > 2160) options.PumpRelaxDuration = 0;
				settingsWasChanged = 1;
				DrawMenu();
				break;
			}

			case MD_MIN_T:
			{
				if(options.HeaterOnTemp+1 < options.HeaterOffTemp) options.HeaterOnTemp ++;
				DrawMenu();
				settingsWasChanged = 1;
				break;
			}

			case MD_MAX_T:
			{
				if(options.HeaterOffTemp < 20) options.HeaterOffTemp ++;			//�������� ��� ������. ��������� �� 10�-15�
				DrawMenu();
				settingsWasChanged = 1;
				break;
			}
			case MD_STAT:
			{
				break;
			}
			case MD_CLEAR:			// ������ ���������
			{
				options.PumpRelaxDuration = 0;
				options.PumpWorkDuration = 0;
				options.HeaterOffTemp = 10;
				options.HeaterOnTemp = 5;
				settingsWasChanged = 1;
				Save();
				options.PumpTimeLeft = 0;
				PumpStop();
				MenuMode = MD_STAT;
				DrawMenu();
				break;
			}
			
		}
	}
	if (bstat == 0b00110000)       // ������ �����
	{
		Save();
		MenuMode --;
		if(MenuMode < 0) MenuMode = MD_LAST-1;
		DrawMenu();
	}
	
	if (bstat == 0b01110000)      // ������ ������
	{
		Save();
		MenuMode ++;
		if(MenuMode == MD_LAST) MenuMode = 0;
		DrawMenu();
	}
	bstat = 0;
}
//----------------------------------------------------------------
void SoftReset(void)
{
	uart_send("AT+CPOWD=1");		// ���������� ���������� ������ SIM900
	LCD_gotoXY(0, 4); LCD_writeStringInv("POWER DOWN NOW");
	_delay_ms(4000);
	wdt_enable(WDTO_15MS);		// ����� ����� 15�� ���������� ��������
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
void renewLCD(void)
{
}
//----------------------------------------------------------------
uint8_t waitAnswer(char *answer, uint16_t timeout)  // ������� ������ �� sim900, ���������� � �������. ���� �����, ���������� 1. �� �������� (� ���/10) ���������� 0
{
  TimeoutTackts = timeout;			// �������� ������ ��������
  while(1){
    while(rx.ptrs[0] == -1 && TimeoutTackts != 0);
    if(TimeoutTackts == 0){                                          // ���� ����� �� ����� �� ��������, ���������� 0 
			TimeoutsCount ++;
      return 0;
		}
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
	TimeoutTackts = 100;		// ��������������� 10 ���.
  while(rx.ptrs[0] == -1 && TimeoutTackts != 0);
	if(TimeoutTackts == 0) TimeoutsCount ++;
}
//----------------------------------------------------------------
void CheckIncomingMessages(void)
{
																	   // +CLIP: "+78312330158",145,"",,"",0
	if(strncmp((char*)rx.buf+rx.ptrs[0], "+CLIP: \"+7", 10) == 0){	// �������� ������. ������ � ������������ ������� ������������
		if(strncmp((char*)rx.buf+rx.ptrs[0]+10, options.OperatorTel, 10) == 0 || strncmp((char*)rx.buf+rx.ptrs[0]+10, options.AdminTel, 10) == 0)
		{
			if(SilentLeft > 5) SilentLeft = 0;	// ������ ������������ ����� ����� � ��������
		}
		dropMessage();				// �������� "+CLIP: "+78312330158",145,"",,"",0"
		uart_send("ATH0");		// ������� �����
		waitMessage();dropMessage();        // ����������� ���
		waitMessage();dropMessage();				// ����������� ��
		return;
	}
	
	if(strncmp((char*)rx.buf+rx.ptrs[0], "+CMTI:", 6) == 0){	// �������� ���
		uart_send("AT+CMGD=4");			// ������� ������ ��� ���������
		waitMessage();dropMessage();        // ����������� ���
		waitMessage();dropMessage();				// ����������� ��
	}
	
	dropMessage();		// ��� ��������� ��������� ������ �����������
}
//----------------------------------------------------------------
void dropMessage(void)
{
	volatile uint8_t i;
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

	if(settingsWasChanged){
		options.localSettingsTimestamp.yy = Now.yy; options.localSettingsTimestamp.MM = Now.MM; options.localSettingsTimestamp.dd = Now.dd;
		options.localSettingsTimestamp.hh = Now.hh; options.localSettingsTimestamp.mm = Now.mm; options.localSettingsTimestamp.ss = Now.ss;
		eeprom_write_block(&options, (void*)0x00, sizeof(struct TSettings));
		settingsWasChanged = 0;
	}
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
  if(options.PumpWorkDuration == 0  || options.PumpRelaxDuration == 0){strcpy(strD, "��� ����������"); return 0;}
  if(options.FrostFlag == 1){strcpy(strD, "����.���������"); return 0;}
  if(PumpPause > 0){ strcpy(strD, "����� "); itoa(PumpPause, buf, 10); strcat(strD, buf); strcat(strD, " ���   ");return 0;}
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
//---------------------------------------------------------------------
void DrawMenu(void)
{
  int x;
  LCD_clear();
  switch(MenuMode)
  {
    case MD_DIRPUMP:
    case MD_DIRHEATER:
    {
      strcpyPM(buf, MSG_Pump); buf[11] = 0x80; buf[12] = 0x81;
      if(MenuMode == MD_DIRHEATER){strcpyPM(buf, MSG_Heater); buf[11] = 0xA8; buf[12] = 0xA8;}
      LCD_gotoXY(0, 0);LCD_writeStringInv(buf);
      LCD_gotoXY(0, 1);	LCD_writePMstring(MSG_RightNow);
      LCD_gotoXY(0, 2);	strcpyPM(buf, MSG_On); buf[4] = 0xAB; buf[9] = 0xAB; LCD_writeString(buf);
      LCD_gotoXY(0, 3);	LCD_writeString("              ");
      LCD_gotoXY(0, 4);	strcpyPM(buf, MSG_Off); buf[4] = 0xAC; buf[9] = 0xAC; LCD_writeString(buf);
      break;
    }
    case MD_PUMPWORKTIME:
    case MD_PUMPRELAXTIME:
    {
      x = 3; if(MenuMode == MD_PUMPRELAXTIME) x = 10;
      LCD_gotoXY(0, 0); LCD_writePMstringInv(MSG_PumpSchedule); // ���������� ������
      LCD_gotoXY(0, 1); LCD_writePMstring(MSG_WorkRelax);		// ��������/�����
      LCD_gotoXY(0, 2); strcpyPM(buf, MSG_Blank); buf[x] = 0xAB; LCD_writeString(buf);
      LCD_gotoXY(0, 3); MinToStr(options.PumpWorkDuration, buf+1); strcat(buf, "  ");MinToStr(options.PumpRelaxDuration, strD);strcat(buf, strD);LCD_writeString(buf);
      LCD_gotoXY(0, 4);	strcpyPM(buf, MSG_Blank); buf[x] = 0xAC; LCD_writeString(buf);
      break;
    }
    case MD_MIN_T:
    case MD_MAX_T:
    {
      x = 3; if(MenuMode == MD_MAX_T) x = 10;
      LCD_gotoXY(0, 0); LCD_writePMstringInv(MSG_HeaterSchedul);
      LCD_gotoXY(0, 1);	LCD_writePMstring(MSG_OnOff);
      LCD_gotoXY(0, 2);	strcpyPM(buf, MSG_Blank); buf[x] = 0xAB; LCD_writeString(buf);
      itoa(options.HeaterOnTemp, buf+2, 10); strcpy(strD, "*C    "); strD[0] = 0xBF; strcat(buf, strD); itoa(options.HeaterOffTemp, strD, 10); strcat(buf, strD); strcpy(strD, "*C   ");strD[0] = 0xBF; strcat(buf, strD);
      LCD_gotoXY(0, 3);	LCD_writeString(buf);
      LCD_gotoXY(0, 4);	strcpyPM(buf, MSG_Blank); buf[x] = 0xAC; LCD_writeString(buf);
      break;
    }
    case MD_STAT: { ShowStat(); break;}
    case MD_CLEAR:
    {
      LCD_gotoXY(0, 0); LCD_writePMstringInv(MSG_Reset);
      LCD_gotoXY(0, 1); LCD_writePMstringInv(MSG_Blank);
      LCD_gotoXY(0, 2);	strcpyPM(buf, MSG_ToSettings); buf[1] = 0xAB; buf[12] = 0xAB; LCD_writeString(buf);
      LCD_gotoXY(0, 3);	LCD_writePMstring(MSG_ToReset);
      LCD_gotoXY(0, 4);	strcpyPM(buf, MSG_ToStat); buf[1] = 0xAC; buf[12] = 0xAC; LCD_writeString(buf);
      break;
    }
		case MD_DEBUG:
		{
			LCD_gotoXY(0, 0);LCD_writePMstringInv(MSG_Info);
			itoa(State.balance, buf, 10); strcat(buf, " R, "); itoa(State.Vbat, strD, 10); strcat(buf, strD); strcat(buf, "V");
			LCD_gotoXY(0, 1);LCD_writeString(buf);

			strcpy(buf, "N");
			itoa(Now.MM, strD, 10); strcat(buf, strD);itoa(Now.dd, strD, 10); strcat(buf, strD); strcat(buf, " ");
			itoa(Now.hh, strD, 10); strcat(buf, strD); strcat(buf, ":"); itoa(Now.mm, strD, 10); strcat(buf, strD); strcat(buf, ":"); itoa(Now.ss, strD, 10); strcat(buf, strD);
			LCD_gotoXY(0, 2);LCD_writeString(buf);
			strcpy(buf, "R");
			itoa(remoteSettingsTimestamp.MM, strD, 10); strcat(buf, strD); itoa(remoteSettingsTimestamp.dd, strD, 10); strcat(buf, strD); strcat(buf, " ");
			itoa(remoteSettingsTimestamp.hh, strD, 10); strcat(buf, strD); strcat(buf, ":"); itoa(remoteSettingsTimestamp.mm, strD, 10); strcat(buf, strD); strcat(buf, ":"); itoa(remoteSettingsTimestamp.ss, strD, 10); strcat(buf, strD);
			LCD_gotoXY(0, 3);LCD_writeString(buf);
			strcpy(buf, "L");
			itoa(options.localSettingsTimestamp.MM, strD, 10); strcat(buf, strD); itoa(options.localSettingsTimestamp.dd, strD, 10); strcat(buf, strD); strcat(buf, " ");
			itoa(options.localSettingsTimestamp.hh, strD, 10); strcat(buf, strD); strcat(buf, ":"); itoa(options.localSettingsTimestamp.mm, strD, 10); strcat(buf, strD); strcat(buf, ":"); itoa(options.localSettingsTimestamp.ss, strD, 10); strcat(buf, strD);
			LCD_gotoXY(0, 4);LCD_writeString(buf);
			
		}
  }
}
//---------------------------------------------------------------------
void ShowStat(void)
{
  LCD_clear();
  LCD_gotoXY(0, 0); LCD_writePMstringInv(MSG_Stat);		// ��������� ���������

  itoa(State.Temp/16, buf, 10);
  strcpyPM(strD, MSG_Celsium); strD[0] = 0xBF;
  if( PORTC & 0b00000100 ){strD[4] = 0xA8;strD[5] = 0xA8;}
  if( PORTC & 0b00010000 ){strD[7] = 0x80;strD[8] = 0x81;}
  strcat(buf, strD);
  LCD_gotoXY(0, 1);	 LCD_writeString(buf);					// ��������� ����������� � ��������� ������������ � ������

  if(options.PumpWorkDuration == 0 || options.PumpRelaxDuration == 0)
  strcpyPM(buf, MSG_TimerOff);
  else
  {
    MinToStr(options.PumpTimeLeft, buf);
    strcat(buf, " �� ");
    options.PumpWorkFlag == 1 ? strcpy(strD, "���       ") : strcpy(strD, "���      ");
    strD[3] = 0x80; strD[4] = 0x81;
    strcat(buf, strD);
  }
  if(PumpPause != 0 && options.PumpWorkFlag == 1)
  {
    strcpy(buf, "����� ");
    itoa(PumpPause/10, strD, 10);
    strcat(buf, strD);
    strcat(buf, " ���.   ");
  }
  if(options.FrostFlag == 1){
		strcpyPM(buf, MSG_Freezing);
	}
	LCD_gotoXY(0, 2);	
	if(State.PowerFailFlag == 0) LCD_writeString(buf);					// ��������� "����� ��" ��� �������������� ������ ���� ����
	else LCD_writeString("���� ������-��");

	itoa(State.balance, buf, 10); strcat(buf, "p "); itoa(State.Vbat/2, strD, 10); strcat(buf, strD); strcat(buf, "v ");
	if(SilentLeft >180){
		itoa(SilentLeft/60, strD, 10); strcat(buf, strD); strcat(buf, "m");
	}
	else {
		itoa(SilentLeft, strD, 10); strcat(buf, strD); strcat(buf, "s");
	}
	LCD_gotoXY(0, 3);LCD_writeString(buf);					// ��������� ������, ���������� �������, ����� �� ���� ������ �����

	//			volatile uint16_t i = 0;
	//			for(i = 0; rx.ptrs[i] != -1 && i < RXBUFSTRCOUNT; i ++);
	//			strcpy(buf, "p"); itoa(i, strD, 10);strcat(buf, strD);
	//			strcat(buf, " w"); itoa(rx.wptr, strD, 10);strcat(buf, strD);
	//			strcat(buf, " b");itoa(rx.buf_overflow_count, strD, 10);strcat(buf, strD);
	strcpy(buf, "bo");itoa(rx.buf_overflow_count, strD, 10);strcat(buf, strD);
	strcat(buf, " po");itoa(rx.ptrs_overflow_count, strD, 10);strcat(buf, strD);
	strcat(buf, " tc");itoa(TimeoutsCount, strD, 10);strcat(buf, strD);
	LCD_gotoXY(0, 4); LCD_writeString(buf);

/*	strcpy(buf, "SIM900Stat ");
	itoa(SIM900Status, strD, 10);
	strcat(buf, strD);
	LCD_gotoXY(0, 4); LCD_writeString(buf);
*/
	uint8_t hh, dd = Now.dd;
	hh = Now.hh + 3;
	if(hh > 23) {hh -= 24; dd ++;}
	itoa(Now.yy, buf, 10); itoa(Now.MM, strD, 10); strcat(buf, strD); itoa(dd, strD, 10); strcat(buf, strD); strcat(buf, " ");
	itoa(hh, strD, 10); strcat(buf, strD); strcat(buf, ":"); itoa(Now.mm, strD, 10); strcat(buf, strD); strcat(buf, ":"); itoa(Now.ss, strD, 10); strcat(buf, strD);
	LCD_gotoXY(0, 5);LCD_writeString(buf);				// ��������� ������� �����

	

  /*
  strcpy(buf, "��.���:");
  itoa(LongBreak, str, 10); strcat(buf, str); strcat(buf, "��.");
  itoa(ShortBreak, str, 10); strcat(buf, str); strcat(buf, "��.   ");
  LCD_gotoXY(0, 3); LCD_writeString(buf);						// ��������� ���������� �� �������
  */
  /*
  strcpy(buf, "  ��� "); buf[0] = 0xA8; buf[1] = 0xA8;
  if(HeaterTotal != 0) itoa((unsigned int)((HeaterWork*100)/HeaterTotal), str, 10);
  else strcpy(str, "--");
  strcat(buf, str); strcat(buf, "% ����  ");
  if(HeaterWork >= 0x28F3980) strcpy(buf, "��������������");	// ������������ �������� ��� 497 �����. ������� ���-��� ������������
  LCD_gotoXY(0, 4); LCD_writeString(buf);						// ��������� ���������� �� ��������
  */
}
//---------------------------------------------------------------------
void MinToStr(unsigned int Min, char *str)		// ��������� ����� ����� � ������ ���� HH:MM
{
  int m, h;
  h = Min/60;
  m = Min - h*60;
  str[0] = '0';
  h < 10 ? itoa(h, str+1, 10) : itoa(h, str, 10);
  str[2] = ':';
  str[3] = '0';
  m < 10 ? itoa(m, str+4, 10) : itoa(m, str+3, 10);
  return;
}
//---------------------------------------------------------------------
void measureBattery(void)
{
	ADMUX  = 0b11000000;		  // 11 - ������� ���������� = 2,56�, 0 - ������������ ������, 0 - ������, 0 - ������, 000 - ����� ������ ADC0
	ADCSRA |= 1<<ADSC;		    // ����� ��������������
	while (ADCSRA & 0x40);		// ���� ����������(������ ����� ADSC � 0)
	uint32_t vLongBat = ADC;
	vLongBat *= 1024;					// ��������� ���������
	State.Vbat = vLongBat / 1000;         // ���������� = ADC * 200
}
//---------------------------------------------------------------------
void strcpyPM(char *dest, const char *PMsrc)		// �������� ������ �� PROGMEM � dest;
{
	char lastChar;
	uint16_t index = 0;
	do{
		lastChar = pgm_read_byte(PMsrc + index);
		*(dest + index) = lastChar;
		index ++;
	}while(lastChar != '\0');
	
}
//---------------------------------------------------------------------
