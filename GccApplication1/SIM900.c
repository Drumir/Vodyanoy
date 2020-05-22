/*
 * SIM900.c
 *
 * Created: 12.03.2018 22:11:42
 *  Author: Drumir
 */ 

#include "vodyanoy2.0.h"

void SIM900_RepairConnection(void)   // ��������� ��������� �����, ��� ������������� ������������� ������ SIM900
{
  LCD_gotoXY(0, 4); LCD_writeString("RepairConnect");
  if(SIM900Status == SIM900_HTTP_FAIL || SIM900Status == SIM900_GPRS_FAIL){  // 
    uart_send("AT+SAPBR=0,1");      //�������� GPRS
    waitMessage();dropMessage();	  // �������� ���
    waitDropOK();                   // �������� ��
    _delay_ms(3000);
    SIM900_EnableGPRS();            // ������� GPRS
    if(SIM900Status >= SIM900_GPRS_OK){   // ���� ��������� ���������
      RecToHistory(EVENT_REPAIRCONN_OK);  // ������� � ������� ������� ��������������
      return;
    }    
    else
      SIM900Status = SIM900_GSM_FAIL;     // ����� �������, ��� ��������� � GSM (��� ��� ����)
  }

  if(SIM900Status == SIM900_GSM_FAIL){  //
    uart_send("AT+CREG=0");         // ����������������� � ������� ����
    waitMessage();dropMessage();	  // �������� ���
    waitDropOK();                   // �������� ��
    waitMessage();dropMessage();	  // �������� ����� (���� �� ������ ������ ����)
    _delay_ms(3000);
    uart_send("AT+CREG=1");         // �������� ����������� � ������� ����
    waitMessage();dropMessage();	  // �������� ���
    waitDropOK();                   // �������� ��
    waitMessage();dropMessage();	  // �������� ����� (���� �� ������ ������ ����)
    SIM900_EnableGSM();             // ���� �����������
    if(SIM900Status >= SIM900_GSM_OK){   // ���� ����������� ������ �������
      SIM900_PrepareConnection();         // ��������� ������� �����������
      if(SIM900Status >= SIM900_GPRS_OK){   // ���� ��������� ����������� �������
        RecToHistory(EVENT_REPAIRCONN_OK);  // ������� � ������� ������� ��������������
        return;
      }      
      else 
        SIM900Status = SIM900_GSM_FAIL;     // ����� �������, ��� ��������� �� ����� ������ ������
    }
    else
    SIM900Status = SIM900_GSM_FAIL;     // ����� �������, ��� ��������� �� ����� ������ ������
  }

  if(SIM900Status == SIM900_GSM_FAIL){
    uart_send("AT+CPOWD=1");   // �������� ������
    waitMessage(); dropMessage();     // �������� ���
    waitMessage(); dropMessage();     // �������� "NORMAL POWER DOWN"
    cbi(PORTD, 7);      // ��������� ������� SIM900
    RecToHistory(EVENT_SIM900_RESTART);
    SIM900Status = SIM900_NOTHING;
    _delay_ms(5000);   // ���� 5 ���
    SIM900_PrepareConnection();		// �������� � ������� ������������
  }  
}
//----------------------------------------------------------------
void SIM900_CheckConnection(void)   // ��������� ��������� �����, ��� ������������� ������������� ������ SIM900
{
  LCD_gotoXY(0, 4); LCD_writeString("�������� HTTP ");
  if(SIM900_OpenHttpGetSession("act=check") == 200){  // ���� ������ ������ ���������� ������ �������
    SIM900_CloseHttpGetSession();
    SIM900Status = SIM900_HTTP_OK;									// �� ������ ��� ��, ������ ��������
    RecToHistory(EVENT_CHECK_CONN_OK);
    return;
  }
  SIM900_CloseHttpGetSession();
  SIM900Status = SIM900_HTTP_FAIL;  // �������� ������� GPRS

  LCD_gotoXY(0, 4); LCD_writeString("�������� GPRS ");
  SIM900_EnableGPRS();
  if(SIM900Status == SIM900_GPRS_OK){
    RecToHistory(EVENT_HTTP_FAIL);
    return;
  }
  SIM900Status = SIM900_GPRS_FAIL;  // �������� GPRS ���������
  LCD_gotoXY(0, 4); LCD_writeString("�������� GSM  ");
  SIM900_EnableGSM();
  if(SIM900Status == SIM900_GSM_OK){
    RecToHistory(EVENT_GPRS_FAIL);
    return;
  }
  SIM900Status = SIM900_GSM_FAIL;  // �������� GSM ���������
  RecToHistory(EVENT_GSM_FAIL);
  LCD_gotoXY(0, 4); LCD_writeString("����������SIM9");
  uart_send("AT+CPOWD=1");   // �������� ������
  waitMessage(); dropMessage();     // �������� ���
  waitMessage(); dropMessage();     // �������� "NORMAL POWER DOWN"
  cbi(PORTD, 7);      // ��������� ������� SIM900
  RecToHistory(EVENT_SIM900_RESTART);
  _delay_ms(5000);   // ���� 5 ���
  SIM900Status = SIM900_NOTHING;
  SIM900_PrepareConnection();		// �������� � ������� ������������

}
//----------------------------------------------------------------

void SIM900_PrepareConnection(void)   // ��������� ���������� �������, �������� ��������, ���������������� � ���� ������ � ������� GPRS ������
{                                     // ���������� ������������ � SIM900Status
  switch(SIM900Status){
    case SIM900_NOTHING:  // ���900 ��� ���� �� ��������
    {                           // ������� ���������� �� �������
			measureBattery();
      if(State.Vbat < 35*20)
      {
        SIM900Status = SIM900_BAT_FAIL;
        return;  
      }
    SIM900Status = SIM900_BAT_OK;
    }  
    case SIM900_BAT_OK:
    {
      SIM900_PowerOn();
    }
    case SIM900_UP:
    {
      SIM900_EnableGSM();
      if(SIM900Status < SIM900_GSM_OK){
        return;
			}
    }    
    case SIM900_GSM_OK:
    {
      SIM900_EnableGPRS();
			if(SIM900Status == SIM900_GPRS_FAIL){
				return;
			}
    }  
    case SIM900_GPRS_OK:
    {
      return;
    }  
  }  
}
//----------------------------------------------------------------
void SIM900_SendStatus(void)
{
  if(SIM900Status < SIM900_GPRS_OK) return;
  itoa(State.Temp, strS, 10);
  strcpy(query,"act=sS&t=");
  strcat(query, strS);
  strcat(query, "&vb=");
  itoa(State.Vbat, strS, 10);
  strcat(query, strS);
  strcat(query, "&b=");
  itoa(State.balance, strS, 10);
  strcat(query, strS);
	uint8_t f = 0;
  if(PORTC & 0b00010000)				{f |= 0b00000001;}		// ����� �������
  if(PORTC & 0b00000100)				{f |= 0b00000010;}		// ������� �������
  if(State.PowerFailFlag == 0)	{f |= 0b00000100;}		// ������� ������������
  if(1)													{f |= 0b00001000;}		// ����� �������
  if(1)													{f |= 0b00010000;}		// ����� �����������
  strcat(query, "&f=");
  itoa(f, strS, 10);
  strcat(query, strS);
  SIM900_OpenHttpGetSession(query);
  SIM900_CloseHttpGetSession();
}
//----------------------------------------------------------------
void SIM900_PowerOn(void)
{
  if((PINB & 0b00000001) != 0){   // ������ ��� �������
    SIM900Status = SIM900_UP;
    return;
  }
	measureBattery();
	if(State.Vbat < 35*20)
	{
  	SIM900Status = SIM900_BAT_FAIL;
  	return;
	}

  sbi(PORTD, 7);      // Enable 4.0v
  _delay_ms(200);   // ���� ��?
  cbi(PORTB, 1);      // ����� PWRKEY ����
  _delay_ms(1500);
  sbi(PORTB, 1);      // ��������� PWRKEY
  while((PINB & 0b00000001) == 0);    // ���� 1 �� STATUS
  uart_send("AT");
  waitMessage();dropMessage();
  waitMessage();dropMessage();
  SIM900Status = SIM900_UP;
}
//----------------------------------------------------------------
void SIM900_EnableGSM(void)
{
  if(SIM900Status < SIM900_UP) return;
  SIM900Status = SIM900_REG_GSM;
  uint8_t iterations = 0;
  do{
    uart_send("AT+CREG?");
    waitMessage();dropMessage();        // ����������� ���
    waitMessage();
    if(strncmp((char*)rx.buf+rx.ptrs[0], "+CREG: 0,1", 10) == 0){ // ����������� �����
			dropMessage();
			waitDropOK();      // ����������� "��"
			break;
		}
    dropMessage();
    waitDropOK();      // ����������� "��"
    iterations ++;
    _delay_ms(1500);
  }while(iterations < 10);

  SIM900Status = SIM900_GSM_OK;
  if(iterations == 10) SIM900Status = SIM900_GSM_FAIL;
}
//----------------------------------------------------------------
void SIM900_GetTime(void)
{
	if(SIM900Status < SIM900_UP) return;
	uart_send("AT+CCLK?");  //		uart_send("AT+CCLK=\"20/04/02,14:43:00+03\"");
	waitMessage(); dropMessage();     // �������� ���
	waitMessage();                    // +CCLK: "20/05/15,19:02:09+03"
  if(strlen((char*)rx.buf+rx.ptrs[0]) == 29)
  {
	  Now.yy = (uint8_t) str2int((char*)rx.buf+rx.ptrs[0]+8);
	  Now.MM = (uint8_t) str2int((char*)rx.buf+rx.ptrs[0]+11);
	  Now.dd = (uint8_t) str2int((char*)rx.buf+rx.ptrs[0]+14);
	  Now.hh = (uint8_t) str2int((char*)rx.buf+rx.ptrs[0]+17);
	  Now.mm = (uint8_t) str2int((char*)rx.buf+rx.ptrs[0]+20);
	  Now.ss = (uint8_t) str2int((char*)rx.buf+rx.ptrs[0]+23);
  }  
	dropMessage();     // �������� �����
	waitDropOK();     // �������� ��
}
//----------------------------------------------------------------
void SIM900_SetTimeFromServer(void)
{
  if(SIM900Status < SIM900_GPRS_OK) return;
  if(SIM900_OpenHttpGetSession("act=getSrvTime") == 200){  // ���� ������ ������ ���������� ������ �������
    uart_sendPM(MSG_HTTPREAD);
	  waitMessage(); dropMessage();     // �������� ���
	  waitMessage(); dropMessage();     // �������� "+HTTPREAD:22"
	  waitMessage();																// {"status":"success","result":"20 04 04,10:05:33"}
		strcpy(strS, (char*)rx.buf+rx.ptrs[0]+30);		// �������� � strS ���������� �����
		strS[17] = '\0';
	  dropMessage();     // �������� �����������
	  waitDropOK();     // �������� "��"
  }
  SIM900_CloseHttpGetSession();
  
	strcpy(query, "AT+CCLK=\"");
	strcat(query, strS);
	strcat(query, "+03\"");
	query[11] = '/'; query[14] = '/';
	uart_send(query);
	waitMessage(); dropMessage();     // �������� ���
	waitMessage(); dropMessage();     // �������� ����� 
}
//----------------------------------------------------------------
void SIM900_EnableGPRS(void)
{
  uint8_t iterations = 0;
  if(SIM900Status < SIM900_GSM_OK) return;
  SIM900Status = SIM900_REG_GPRS;
  
  //    ��� ��������� ��� ��������� � ����������������� ������ ������
  //uart_send("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"");waitDropOK();uart_send("AT+SAPBR=3,1,\"APN\",\"internet.tele2.ru\"");waitDropOK();
  //uart_send("AT+SAPBR=3,1,\"USER\",\"\"");waitDropOK();uart_send("AT+SAPBR=3,1,\"PWD\",\"\"");waitDropOK();
	//uart_send("AT+CNMI=2,2");waitMessage();dropMessage();waitMessage();dropMessage(); // �������� ��� ����� ���������� � �� ��� ����� ���� ��������� � �����������

  do{
    iterations ++;
    uart_send("AT+SAPBR=1,1");
		waitMessage();dropMessage();	// �������� ���
		waitMessage();
		if(strncmp((char*)rx.buf+rx.ptrs[0], "OK", 2) == 0){dropMessage(); break;}
		dropMessage();
    _delay_ms(1500);
  } while(iterations < 10);
  SIM900Status = SIM900_GPRS_OK;
  if(iterations == 10) SIM900Status = SIM900_GPRS_FAIL;
  
}
//----------------------------------------------------------------
void SIM900_GetBalance(void)
{
  if(SIM900Status < SIM900_REG_GSM) return;
  //  uart_send("AT+CUSD=1,\"*120#\"");         // ������ �� ����������   AT+CUSD=1,"*120$23"$0d
  uart_sendPM(MSG_Balance);         // ������ �������  AT+CUSD=1,"*105*5$23"$0d
  waitMessage(); dropMessage();     // �������� ���
  waitDropOK();     // �������� ��
  waitMessage();
	strcpy(DebugStr, (char*)rx.buf+rx.ptrs[0]);
  State.balance = str2int((char*)rx.buf+rx.ptrs[0]+10);
  dropMessage();     // ��������
}
//----------------------------------------------------------------
void SIM900_GetRemoteSettingsTimestamp(void)      // �������� ����� ���������� ��������� �������� �� �������
{
  if(SIM900Status < SIM900_GPRS_OK) return;

  if(SIM900_OpenHttpGetSession("act=gts") == 200){  // ���� ������ ������ ���������� ������ �������
    uart_sendPM(MSG_HTTPREAD);
    waitMessage(); dropMessage();     // �������� ���
    waitMessage(); dropMessage();     // �������� "+HTTPREAD:22"
    waitMessage(); 
									// {"status":"success","result":[{"timestamp":"2020-04-11 14:16:01"}]}
									// {"status":"success","result":[{"timestamp":"2020-04-11 14:16:01"}]}
									// {"status":"success","result":[{"timestamp":"2020-04-11 14:16:01"}]}

		uint16_t commaPosition = 0;
		while(*(rx.buf+rx.ptrs[0]+commaPosition) != '-' && *(rx.buf+rx.ptrs[0]+commaPosition) != '\0') commaPosition ++;  // ������ ������ ���� � ������ - ����� ��� ���������� ���!
		if(*(rx.buf+rx.ptrs[0]+commaPosition) != '\0'){
			*(rx.buf+rx.ptrs[0]+commaPosition) = ' ';			// ��������� �� ������ ����, ������� str2int() ��������� �� ������!!!
			*(rx.buf+rx.ptrs[0]+commaPosition + 3) = ' ';
		}
    remoteSettingsTimestamp.yy = str2int((char*)rx.buf+rx.ptrs[0]+commaPosition-2);
    remoteSettingsTimestamp.MM = str2int((char*)rx.buf+rx.ptrs[0]+commaPosition+1);
    remoteSettingsTimestamp.dd = str2int((char*)rx.buf+rx.ptrs[0]+commaPosition+4);
    remoteSettingsTimestamp.hh = str2int((char*)rx.buf+rx.ptrs[0]+commaPosition+7);  // �� ������� ����������� ������� �����
    remoteSettingsTimestamp.mm = str2int((char*)rx.buf+rx.ptrs[0]+commaPosition+10);
    remoteSettingsTimestamp.ss = str2int((char*)rx.buf+rx.ptrs[0]+commaPosition+13);

    dropMessage();     // �������� �����������
    waitDropOK();     // �������� "��"
  }
  SIM900_CloseHttpGetSession();
}
//----------------------------------------------------------------
void SIM900_SendSettings(void)                    // �������� ��������� �� ������ ��� ����������
{
  if(SIM900Status < SIM900_GPRS_OK) return;

  strcpy(query,"act=wS&pump1="); // Write Settings
  itoa(options.PumpWorkDuration, strS, 10); strcat(query, strS); strcat(query, "&pump0=");
  itoa(options.PumpRelaxDuration, strS, 10); strcat(query, strS); strcat(query, "&mint=");
  itoa(options.HeaterOnTemp, strS, 10); strcat(query, strS); strcat(query, "&maxt=");
  itoa(options.HeaterOffTemp, strS, 10); strcat(query, strS);

  SIM900_OpenHttpGetSession(query);
  SIM900_CloseHttpGetSession();
}
//----------------------------------------------------------------
void SIM900_GetSettings(void)                     // ����� ��������� � ������� � ��������� ��
{
  uint16_t commaPosition = 0;
  if(SIM900Status < SIM900_GPRS_OK) return;
  if(SIM900_OpenHttpGetSession("act=GetSettings") == 200){  // ���� ������ ������ ���������� ������ �������
    uart_sendPM(MSG_HTTPREAD);
    waitMessage(); dropMessage();     // �������� ���
    waitMessage(); dropMessage();     // �������� "+HTTPREAD:22"
    waitMessage();                    // {"status":"success","result":"180315230102,120,240,3,10,5,8,-1,8,40,8,8,8,8,8,60,8,20,8,960,9027891301,9040448302"}
																			// {"status":"success","result":"2020-04-11 14:16:01,45,45,22,25,5,11,-1,11,50,0,2,2,0,9,1440,1,20,0,840,9027891301,9040448302"}

		while(*(rx.buf+rx.ptrs[0]+commaPosition) != '-' && *(rx.buf+rx.ptrs[0]+commaPosition) != '\0') commaPosition ++;  // ������ ������ ���� � ������ - ����� ��� ���������� ���!
		if(*(rx.buf+rx.ptrs[0]+commaPosition) != '\0'){
			*(rx.buf+rx.ptrs[0]+commaPosition) = ' ';			// ��������� �� ������ ����, ������� str2int() ��������� �� ������!!!
			*(rx.buf+rx.ptrs[0]+commaPosition + 3) = ' ';
		}
    strncpy(strS, (char*)rx.buf+rx.ptrs[0]+commaPosition-2, 2); strS[2] = '\0';options.localSettingsTimestamp.yy = str2int(strS);
    strncpy(strS, (char*)rx.buf+rx.ptrs[0]+commaPosition+1, 2); strS[2] = '\0';options.localSettingsTimestamp.MM = str2int(strS);
    strncpy(strS, (char*)rx.buf+rx.ptrs[0]+commaPosition+4, 2); strS[2] = '\0';options.localSettingsTimestamp.dd = str2int(strS);
    strncpy(strS, (char*)rx.buf+rx.ptrs[0]+commaPosition+7, 2); strS[2] = '\0';options.localSettingsTimestamp.hh = str2int(strS); // ��������� ����� � ���������
    strncpy(strS, (char*)rx.buf+rx.ptrs[0]+commaPosition+10, 2); strS[2] = '\0';options.localSettingsTimestamp.mm = str2int(strS);
    strncpy(strS, (char*)rx.buf+rx.ptrs[0]+commaPosition+13, 2); strS[2] = '\0';options.localSettingsTimestamp.ss = str2int(strS);  
    //while(*(rx.buf+rx.ptrs[0]+commaPosition) != ',' && *(rx.buf+rx.ptrs[0]+commaPosition) != '\0') commaPosition ++;  // ���� � ������ �������
    //commaPosition ++;
    while(*(rx.buf+rx.ptrs[0]+commaPosition) != ',' && *(rx.buf+rx.ptrs[0]+commaPosition) != '\0') commaPosition ++;  // ���� � ������ ������ �������
    //commaPosition ++;
    options.PumpWorkDuration = str2int((char*)rx.buf+rx.ptrs[0] + ++commaPosition);
    while(*(rx.buf+rx.ptrs[0]+commaPosition) != ',' && *(rx.buf+rx.ptrs[0]+commaPosition) != '\0') commaPosition ++;  // ���� � ������ �������
    options.PumpRelaxDuration = str2int((char*)rx.buf+rx.ptrs[0] + ++commaPosition );  
    while(*(rx.buf+rx.ptrs[0]+commaPosition) != ',' && *(rx.buf+rx.ptrs[0]+commaPosition) != '\0') commaPosition ++;  // ���� � ������ �������
    options.HeaterOnTemp = str2int((char*)rx.buf+rx.ptrs[0] + ++commaPosition );  
    while(*(rx.buf+rx.ptrs[0]+commaPosition) != ',' && *(rx.buf+rx.ptrs[0]+commaPosition) != '\0') commaPosition ++;  // ���� � ������ �������
    options.HeaterOffTemp = str2int((char*)rx.buf+rx.ptrs[0] + ++commaPosition );  
    while(*(rx.buf+rx.ptrs[0]+commaPosition) != ',' && *(rx.buf+rx.ptrs[0]+commaPosition) != '\0') commaPosition ++;  // ���� � ������ �������
    options.ConnectPeriod = str2int((char*)rx.buf+rx.ptrs[0] + ++commaPosition );  
    while(*(rx.buf+rx.ptrs[0]+commaPosition) != ',' && *(rx.buf+rx.ptrs[0]+commaPosition) != '\0') commaPosition ++;  // ���� � ������ �������
    options.fFrostNotifications = str2int((char*)rx.buf+rx.ptrs[0] + ++commaPosition );
    while(*(rx.buf+rx.ptrs[0]+commaPosition) != ',' && *(rx.buf+rx.ptrs[0]+commaPosition) != '\0') commaPosition ++;  // ���� � ������ �������
    options.FrostTemp = str2int((char*)rx.buf+rx.ptrs[0] + ++commaPosition );
    while(*(rx.buf+rx.ptrs[0]+commaPosition) != ',' && *(rx.buf+rx.ptrs[0]+commaPosition) != '\0') commaPosition ++;  // ���� � ������ �������
    options.fWarmNotifications = str2int((char*)rx.buf+rx.ptrs[0] + ++commaPosition );  
    while(*(rx.buf+rx.ptrs[0]+commaPosition) != ',' && *(rx.buf+rx.ptrs[0]+commaPosition) != '\0') commaPosition ++;  // ���� � ������ �������
    options.WarmTemp = str2int((char*)rx.buf+rx.ptrs[0] + ++commaPosition );  
    while(*(rx.buf+rx.ptrs[0]+commaPosition) != ',' && *(rx.buf+rx.ptrs[0]+commaPosition) != '\0') commaPosition ++;  // ���� � ������ �������
    options.fDoorNotifications = str2int((char*)rx.buf+rx.ptrs[0] + ++commaPosition );  
    while(*(rx.buf+rx.ptrs[0]+commaPosition) != ',' && *(rx.buf+rx.ptrs[0]+commaPosition) != '\0') commaPosition ++;  // ���� � ������ �������
    options.fFloodNotifications = str2int((char*)rx.buf+rx.ptrs[0] + ++commaPosition );  
    while(*(rx.buf+rx.ptrs[0]+commaPosition) != ',' && *(rx.buf+rx.ptrs[0]+commaPosition) != '\0') commaPosition ++;  // ���� � ������ �������
    options.fPowerNotifications = str2int((char*)rx.buf+rx.ptrs[0] + ++commaPosition );  
    while(*(rx.buf+rx.ptrs[0]+commaPosition) != ',' && *(rx.buf+rx.ptrs[0]+commaPosition) != '\0') commaPosition ++;  // ���� � ������ �������
    options.fPowerRestNotifications = str2int((char*)rx.buf+rx.ptrs[0] + ++commaPosition );  
    while(*(rx.buf+rx.ptrs[0]+commaPosition) != ',' && *(rx.buf+rx.ptrs[0]+commaPosition) != '\0') commaPosition ++;  // ���� � ������ �������
    options.fOfflineNotifications = str2int((char*)rx.buf+rx.ptrs[0] + ++commaPosition );  
    while(*(rx.buf+rx.ptrs[0]+commaPosition) != ',' && *(rx.buf+rx.ptrs[0]+commaPosition) != '\0') commaPosition ++;  // ���� � ������ �������
    options.DisconnectionTime = str2int((char*)rx.buf+rx.ptrs[0] + ++commaPosition );  
    while(*(rx.buf+rx.ptrs[0]+commaPosition) != ',' && *(rx.buf+rx.ptrs[0]+commaPosition) != '\0') commaPosition ++;  // ���� � ������ �������
    options.fBalanceNotifications = str2int((char*)rx.buf+rx.ptrs[0] + ++commaPosition );  
    while(*(rx.buf+rx.ptrs[0]+commaPosition) != ',' && *(rx.buf+rx.ptrs[0]+commaPosition) != '\0') commaPosition ++;  // ���� � ������ �������
    options.MinBalance = str2int((char*)rx.buf+rx.ptrs[0] + ++commaPosition );  
    while(*(rx.buf+rx.ptrs[0]+commaPosition) != ',' && *(rx.buf+rx.ptrs[0]+commaPosition) != '\0') commaPosition ++;  // ���� � ������ �������
    options.fDailyNotifications = str2int((char*)rx.buf+rx.ptrs[0] + ++commaPosition );  
    while(*(rx.buf+rx.ptrs[0]+commaPosition) != ',' && *(rx.buf+rx.ptrs[0]+commaPosition) != '\0') commaPosition ++;  // ���� � ������ �������
    options.DailyReportTime = str2int((char*)rx.buf+rx.ptrs[0] + ++commaPosition );  
    while(*(rx.buf+rx.ptrs[0]+commaPosition) != ',' && *(rx.buf+rx.ptrs[0]+commaPosition) != '\0') commaPosition ++;  // ���� � ������ �������
    options.DirectControlFlags = str2int((char*)rx.buf+rx.ptrs[0] + ++commaPosition );
    while(*(rx.buf+rx.ptrs[0]+commaPosition) != ',' && *(rx.buf+rx.ptrs[0]+commaPosition) != '\0') commaPosition ++;  // ���� � ������ �������
    strncpy((char*)options.OperatorTel, (char*)rx.buf+rx.ptrs[0] + ++commaPosition, 10);  
    while(*(rx.buf+rx.ptrs[0]+commaPosition) != ',' && *(rx.buf+rx.ptrs[0]+commaPosition) != '\0') commaPosition ++;  // ���� � ������ �������
    strncpy((char*)options.AdminTel, (char*)rx.buf+rx.ptrs[0] + ++commaPosition, 10);

		if(options.PumpWorkDuration == 0 || options.PumpRelaxDuration == 0) PumpStop();  // ��� ��������� ����� ���� � ���� ������ ������� ������������ ������ ��� ������ ������
		eeprom_write_block(&options, (void*)0x00, sizeof(struct TSettings));		// �������� ���������� ��������� � EEPROM
		RecToHistory(EVENT_NEW_REMOTE_SETTINGS);

		dropMessage();     // �������� �����������
    waitDropOK();     // �������� "��"
  }
  SIM900_CloseHttpGetSession();
}
//----------------------------------------------------------------
void SIM900_SendHistory(void)                     // ������� �� ������ ������� �������
{
	uint16_t ptr;
  if(SIM900Status < SIM900_GPRS_OK) return;
	for(ptr = 0; History[ptr].EventCode == EVENT_NONE && ptr < HISTORYLENGTH; ptr ++);  // ���� ������ �������� �������
	if(ptr == HISTORYLENGTH) return; // � ������� ��� ������������ �������
	for(; History[ptr].EventCode != EVENT_NONE && ptr < HISTORYLENGTH; ptr ++)
	{
		strcpy(query, "act=sh&ts=");
		itoa(History[ptr].EventTime.yy, strS, 10); strcat(query, strS);
		if(History[ptr].EventTime.MM < 10)strcat(query, "0"); itoa(History[ptr].EventTime.MM, strS, 10); strcat(query, strS);
		if(History[ptr].EventTime.dd < 10)strcat(query, "0"); itoa(History[ptr].EventTime.dd, strS, 10); strcat(query, strS);
		if(History[ptr].EventTime.hh < 10)strcat(query, "0"); itoa(History[ptr].EventTime.hh, strS, 10); strcat(query, strS);
		if(History[ptr].EventTime.mm < 10)strcat(query, "0"); itoa(History[ptr].EventTime.mm, strS, 10); strcat(query, strS);
		if(History[ptr].EventTime.ss < 10)strcat(query, "0"); itoa(History[ptr].EventTime.ss, strS, 10); strcat(query, strS);
		strcat(query, "&ec="); 
		itoa(History[ptr].EventCode, strS, 10); strcat(query, strS);
    if(SIM900_OpenHttpGetSession(query) == 200)       // ���� �������� ������� �� ������ �������, ������ ��� �� �������
  		History[ptr].EventCode = EVENT_NONE;
    SIM900_CloseHttpGetSession();
	}
}
//----------------------------------------------------------------
void SIM900_SendSMS(char *number, char *text)	   // �������� ��� � ������� text �� ����� number
{
  if(SIM900Status < SIM900_GSM_OK) return;
	strcpyPM(strS, MSG_AT_CMGS);
	strcat(strS, number);
	strcat(strS, "\"");
	uart_send(strS);
	waitMessage(); dropMessage();     // �������� ���
	_delay_ms(500);
	text[strlen(text)] = 26;
	uart_send(text);
	waitMessage(); dropMessage();     // �������� ���
	waitMessage(); dropMessage();     // �������� +CMGS: 2  ����� 2 - ����� ���������, ������� ����������� �� ��������
	waitDropOK();
	uart_send("AT+CMGD=4");			// ������� ������ ��� ���������
	waitMessage();dropMessage();        // ����������� ���
	waitDropOK();				// ����������� ��

	
}
//----------------------------------------------------------------
void SIM900_Call(char *number)										 // ������ �� ����� number. 
{
  if(SIM900Status < SIM900_GSM_OK) return;
	strcpy(strS, "ATD+7");
	strcat(strS, number);
	strcat(strS, ";");
	uart_send(strS);
	waitMessage(); dropMessage();     // �������� ���
	waitDropOK();     // �������� ��
}
//----------------------------------------------------------------
uint16_t SIM900_OpenHttpGetSession(char *params)                     // ��������� HTTP GET ������ � ����������� query, ������������ ��� ���������
{
  uint16_t http_code = 0;
  uart_sendPM(MSG_HTTPINIT);
  waitDropOK();
  uart_sendPM(MSG_HTTPPARACID);
  waitDropOK();
  uart_sendPM_wo_CRLF(MSG_HTTPPARAURL); uart_send_wo_CRLF(options.Link); uart_send_wo_CRLF("?"); uart_send_wo_CRLF(params); uart_send("\"");
  waitDropOK();
  uart_sendPM(MSG_HTTPACTION0);   // ������� �����: ��� / ��, / +HTTPACTION:1,200,20
  waitMessage(); dropMessage();     // �������� ���
  waitDropOK();     // �������� "��"
  waitMessage();  // +HTTPACTION:0,200,32
  http_code = str2int((char*)(rx.buf+rx.ptrs[0] + 14));
  if(http_code != 200)
    TimeoutsCount ++;
  if(http_code >= 600){
    RecToHistory(EVENT_ERROR_60X);
    TimeoutsCount = 10;
  }
  dropMessage();	
  return http_code;		
}
//----------------------------------------------------------------
void SIM900_CloseHttpGetSession(void)                     // ��������� HTTP GET ������
{
	uart_sendPM(MSG_HTTPTERM);   // ������� �����: ��� / ��,
	waitMessage(); dropMessage();     // �������� ���
	waitDropOK();     // �������� "��"
}
/*			static uint8_t smsNun = 0;
			if (smsNun == 0)
			{
				uart_send("AT+CMGF=1"); waitMessage(); dropMessage(); waitDropOK();				// �������� ��������� ����� ���
				uart_send("AT+CSCS= \"GSM\""); waitMessage(); dropMessage(); waitDropOK();// ������������ ��������� "GSM"
				uart_send("AT+CSCB=1"); waitMessage(); dropMessage(); waitDropOK();				// ��������� ����� ����������� ���������
			}
			else{
				strcpy(strS, "AT+CMGR="); itoa(smsNun, strD, 10); strcat(strS, strD); uart_send(strS); // ��������� ��� ����� smsNun
				dropMessage(); dropMessage();dropMessage();
			}
			smsNun ++;
*/