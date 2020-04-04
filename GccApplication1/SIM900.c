/*
 * SIM900.c
 *
 * Created: 12.03.2018 22:11:42
 *  Author: Drumir
 */ 

#include "vodyanoy2.0.h"

void SIM900_PrepareConnection(void)   // ��������� ���������� �������, �������� ��������, ���������������� � ���� ������ � ������� GPRS ������
{                                     // ���������� ������������ � SIM900Status
  switch(SIM900Status){
    case SIM900_NOTHING:  // ���900 ��� ���� �� ��������
    {                           // ������� ���������� �� �������
      ADMUX  = 0b11000000;		  // 11 - ������� ���������� = 2,56�, 0 - ������������ ������, 0 - ������, 0 - ������, 000 - ����� ������ ADC0
      ADCSRA |= 1<<ADSC;		    // ����� ��������������
  	  while (ADCSRA & 0x40);		// ���� ����������(������ ����� ADSC � 0)
			uint32_t vLongBat = ADC;
			vLongBat *= 1024;					// ��������� ���������
      State.Vbat = vLongBat / 1000;         // ���������� = ADC * 200
      if(State.Vbat < 32*20)
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
      SIM900_WaitRegistration();
      if(SIM900Status < SIM900_GSM_OK)
        return;
    }    
    case SIM900_GSM_OK:
    {
      SIM900_EnableGPRS();
    }  
    case SIM900_GPRS_OK:
    {
      return;
    }  
  }  
}


void SIM900_SendStatus(void)
{
  if(SIM900Status < SIM900_GPRS_OK) return;
  uart_send("AT+HTTPINIT");
  waitAnswer("OK", 20);
//  uart_send("AT+HTTPSSL=1");
//  waitAnswer("OK", 20);
  uart_send("AT+HTTPPARA=\"CID\",1");
  waitAnswer("OK", 20);

  itoa(State.Temp, str, 10);
  strcpy(query, "AT+HTTPPARA=\"URL\",\"");
  strcat(query, link);
  strcat(query, "?act=wT&t=");
  strcat(query, str);
  strcat(query, "&vb=");
  itoa(State.Vbat, str, 10);
  strcat(query, str);
  strcat(query, "&b=");
  itoa(State.balance, str, 10);
  strcat(query, str);
  strcat(query, "\"");
  uart_send(query);
  waitAnswer("OK", 20);
  uart_send("AT+HTTPACTION=0");   // ������� �����: ��� / ��, / +HTTPACTION:1,200,20
  waitMessage(); dropMessage();     // �������� ���
  waitMessage(); dropMessage();     // �������� "��"
  waitMessage();
  if(str2int((char*)rx.buf+rx.ptrs[0]+18) > 32){
    dropMessage();     // �������� ����� �������
    uart_send("AT+HTTPREAD=0,128");
    waitMessage(); dropMessage();     // �������� ���
    waitMessage(); dropMessage();     // �������� "+HTTPREAD:22"
    waitMessage(); dropMessage();     // �������� �����������
    waitMessage(); dropMessage();     // �������� "��"
  }
  dropMessage();
  uart_send("AT+HTTPTERM");   // ������� �����: ��� / ��,
  waitMessage(); dropMessage();     // �������� ���
  waitMessage(); dropMessage();     // �������� "��"

}
//----------------------------------------------------------------
void SIM900_PowerOn(void)
{
  if(SIM900Status < SIM900_BAT_OK) return;
  sbi(PORTD, 7);      // Enable 4.0v
  _delay_ms(200);   // ���� ��?
  
  if((PINB & 0b00000001) != 0){   // ������ ��� �������
    SIM900Status = SIM900_UP;
    return;
  }
  
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
void SIM900_WaitRegistration(void)
{
  if(SIM900Status < SIM900_UP) return;
  SIM900Status = SIM900_REG_GSM;
  uint8_t iterations = 0, sucess_flag = 0;
  do{
    _delay_ms(500);
    uart_send("AT+CREG?");
    waitMessage();dropMessage();        // ����������� ���
    waitMessage();
    if(strncmp((char*)rx.buf+rx.ptrs[0], "+CREG: 0,1", 10) == 0) sucess_flag = 1; // ����������� �����
    dropMessage();
    waitMessage();dropMessage();      // ����������� "��"
    iterations ++;
  }while(sucess_flag == 0 && iterations < 30);

  SIM900Status = SIM900_GSM_OK;
  if(sucess_flag == 0) SIM900Status = SIM900_GSM_FAIL;
}
//----------------------------------------------------------------
void SIM900_GetTime(void)
{
	if(SIM900Status < SIM900_UP) return;
	uart_send("AT+CCLK?");  //		uart_send("AT+CCLK=\"20/04/02,14:43:00+03\"");
	waitMessage(); dropMessage();     // �������� ���
	waitMessage(); /*dropMessage();*/     // �������� �����
	Now.yy = (uint8_t) str2int((char*)rx.buf+rx.ptrs[0]+8);
	Now.MM = (uint8_t) str2int((char*)rx.buf+rx.ptrs[0]+11);
	Now.dd = (uint8_t) str2int((char*)rx.buf+rx.ptrs[0]+14);
	Now.hh = (uint8_t) str2int((char*)rx.buf+rx.ptrs[0]+17);
	Now.mm = (uint8_t) str2int((char*)rx.buf+rx.ptrs[0]+20);
	Now.ss = (uint8_t) str2int((char*)rx.buf+rx.ptrs[0]+23);
	waitMessage(); dropMessage();     // �������� ��
}
//----------------------------------------------------------------
void SIM900_SetTimeFromServer(void)
{
  if(SIM900Status < SIM900_GPRS_OK) return;
  uart_send("AT+HTTPINIT");
  waitAnswer("OK", 20);
  uart_send("AT+HTTPPARA=\"CID\",1");
  waitAnswer("OK", 20);

  strcpy(query, "AT+HTTPPARA=\"URL\",\""); strcat(query, link); strcat(query, "?act=getSrvTime\"");
  uart_send(query);
  waitAnswer("OK", 20);
  uart_send("AT+HTTPACTION=0");   // ������� �����: ��� / ��, / +HTTPACTION:1,200,20
  waitMessage(); dropMessage();     // �������� ���
  waitMessage(); dropMessage();     // �������� "��"
  waitMessage();
  if(str2int((char*)rx.buf+rx.ptrs[0]+14) == 200){  // ���� ������ ������ ���������� ������ �������
	  dropMessage();     // �������� ����� �������
	  uart_send("AT+HTTPREAD=0,128");
	  waitMessage(); dropMessage();     // �������� ���
	  waitMessage(); dropMessage();     // �������� "+HTTPREAD:22"
	  waitMessage();																// {"status":"success","result":"20 04 04,10:05:33"}
		strcpy(str, "AT+CCLK=\"");
	  strncpy(str+9, (char*)rx.buf+rx.ptrs[0]+30, 17);
		str[11] = '/'; str[14] = '/';
		strcat(str, "+03\"");

	  dropMessage();     // �������� �����������
	  waitMessage(); dropMessage();     // �������� "��"
  }
  dropMessage();
  uart_send("AT+HTTPTERM");   // ������� �����: ��� / ��,
  waitMessage(); dropMessage();     // �������� ���
  waitMessage(); dropMessage();     // �������� "��"
	uart_send(str);
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
  //uart_send("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"");waitAnswer("OK", 20);uart_send("AT+SAPBR=3,1,\"APN\",\"internet.tele2.ru\"");waitAnswer("OK", 20);
  //uart_send("AT+SAPBR=3,1,\"USER\",\"\"");waitAnswer("OK", 20);uart_send("AT+SAPBR=3,1,\"PWD\",\"\"");waitAnswer("OK", 20);
  do{
    iterations ++;
    _delay_ms(500);
    uart_send("AT+SAPBR=1,1");
  } while(waitAnswer("OK", 20) != 1 && iterations < 30);
  SIM900Status = SIM900_GPRS_OK;
  if(iterations == 30) SIM900Status = SIM900_GPRS_FAIL;
  
}
//----------------------------------------------------------------
void SIM900_GetBalance(void)
{
  if(SIM900Status < SIM900_REG_GSM) return;
  //  uart_send("AT+CUSD=1,\"*120#\"");         // ������ �� ����������   AT+CUSD=1,"*120$23"$0d
  uart_send("AT+CUSD=1,\"*105*5#\"");         // ������ �������  AT+CUSD=1,"*105*5$23"$0d
  waitMessage(); dropMessage();     // �������� ���
  waitMessage(); dropMessage();     // �������� ��
  waitMessage(); dropMessage();     // �������� ��
  waitMessage();
	strcpy(DebugStr, (char*)rx.buf+rx.ptrs[0]);
  State.balance = str2int((char*)rx.buf+rx.ptrs[0]+10);
  dropMessage();     // ��������
}
//----------------------------------------------------------------
void SIM900_GetRemoteSettingsTimestamp(void)      // �������� ����� ���������� ��������� �������� �� �������
{
  if(SIM900Status < SIM900_GPRS_OK) return;
  uart_send("AT+HTTPINIT");
  waitAnswer("OK", 20);
  uart_send("AT+HTTPPARA=\"CID\",1");
  waitAnswer("OK", 20);

  strcpy(query, "AT+HTTPPARA=\"URL\",\""); strcat(query, link); strcat(query, "?act=gts\"");
  uart_send(query);
  waitAnswer("OK", 20);
  uart_send("AT+HTTPACTION=0");   // ������� �����: ��� / ��, / +HTTPACTION:1,200,20
  waitMessage(); dropMessage();     // �������� ���
  waitMessage(); dropMessage();     // �������� "��"
  waitMessage();
  if(str2int((char*)rx.buf+rx.ptrs[0]+14) == 200){  // ���� ������ ������ ���������� ������ �������
    dropMessage();     // �������� ����� �������
    uart_send("AT+HTTPREAD=0,128");
    waitMessage(); dropMessage();     // �������� ���
    waitMessage(); dropMessage();     // �������� "+HTTPREAD:22"
    waitMessage(); 
    strncpy(str, (char*)rx.buf+rx.ptrs[0]+46, 2); remoteSettingsTimestamp.yy = str2int(str);
    strncpy(str, (char*)rx.buf+rx.ptrs[0]+49, 2); remoteSettingsTimestamp.MM = str2int(str);
    strncpy(str, (char*)rx.buf+rx.ptrs[0]+52, 2); remoteSettingsTimestamp.dd = str2int(str);
    strncpy(str, (char*)rx.buf+rx.ptrs[0]+55, 2); remoteSettingsTimestamp.hh = str2int(str);  // �� ������� ����������� ������� �����
    strncpy(str, (char*)rx.buf+rx.ptrs[0]+58, 2); remoteSettingsTimestamp.mm = str2int(str);
    strncpy(str, (char*)rx.buf+rx.ptrs[0]+61, 2); remoteSettingsTimestamp.ss = str2int(str);
		//strcpy(DebugStr, (char*)rx.buf+rx.ptrs[0]+44);
    dropMessage();     // �������� �����������
    waitMessage(); dropMessage();     // �������� "��"
  }
  dropMessage();
  uart_send("AT+HTTPTERM");   // ������� �����: ��� / ��,
  waitMessage(); dropMessage();     // �������� ���
  waitMessage(); dropMessage();     // �������� "��"
}
//----------------------------------------------------------------
void SIM900_SendSettings(void)                    // �������� ��������� �� ������ ��� ����������
{
  if(SIM900Status < SIM900_GPRS_OK) return;
  uart_send("AT+HTTPINIT");
  waitAnswer("OK", 20);
  uart_send("AT+HTTPPARA=\"CID\",1");
  waitAnswer("OK", 20);

  strcpy(query, "AT+HTTPPARA=\"URL\",\""); strcat(query, link); strcat(query, "?act=wS&pump1="); // Write Settings
  itoa(options.PumpWorkDuration, str, 10); strcat(query, str); strcat(query, "&pump0=");
  itoa(options.PumpRelaxDuration, str, 10); strcat(query, str); strcat(query, "&mint=");
  itoa(options.HeaterOnTemp, str, 10); strcat(query, str); strcat(query, "&maxt=");
  itoa(options.HeaterOffTemp, str, 10); strcat(query, str); strcat(query, "\"");

  uart_send(query);

  waitAnswer("OK", 20);
  uart_send("AT+HTTPACTION=0");   // ������� �����: ��� / ��, / +HTTPACTION:1,200,20
  waitMessage(); dropMessage();     // �������� ���
  waitMessage(); dropMessage();     // �������� "��"

  waitMessage(); dropMessage();     // �������� �����

  uart_send("AT+HTTPTERM");   // ������� �����: ��� / ��,
  waitMessage(); dropMessage();     // �������� ���
  waitMessage(); dropMessage();     // �������� "��"

}
//----------------------------------------------------------------
void SIM900_GetSettings(void)                     // ����� ��������� � ������� � ��������� ��
{
  uint16_t commaPosition = 0;
  if(SIM900Status < SIM900_GPRS_OK) return;
  uart_send("AT+HTTPINIT");
  waitAnswer("OK", 20);
  uart_send("AT+HTTPPARA=\"CID\",1");
  waitAnswer("OK", 20);
  strcpy(query, "AT+HTTPPARA=\"URL\",\""); strcat(query, link); strcat(query, "?act=GetSettings\"");
  uart_send(query);
  waitAnswer("OK", 20);
  uart_send("AT+HTTPACTION=0");   // ������� �����: ��� / ��, / +HTTPACTION:1,200,20
  waitMessage(); dropMessage();     // �������� ���
  waitMessage(); dropMessage();     // �������� "��"
  waitMessage();
  if(str2int((char*)rx.buf+rx.ptrs[0]+14) == 200){  // ���� ������ ������ ���������� ������ �������
    dropMessage();     // �������� ����� �������
    uart_send("AT+HTTPREAD=0,128");
    waitMessage(); dropMessage();     // �������� ���
    waitMessage(); dropMessage();     // �������� "+HTTPREAD:22"
    waitMessage();                    // {"status":"success","result":"180315230102,120,240,3,10,5,8,-1,8,40,8,8,8,8,8,60,8,20,8,960,9027891301,9040448302"}
    strncpy(str, (char*)rx.buf+rx.ptrs[0]+32, 2); options.localSettingsTimestamp.yy = str2int(str);
    strncpy(str, (char*)rx.buf+rx.ptrs[0]+35, 2); options.localSettingsTimestamp.MM = str2int(str);
    strncpy(str, (char*)rx.buf+rx.ptrs[0]+38, 2); options.localSettingsTimestamp.dd = str2int(str);
    strncpy(str, (char*)rx.buf+rx.ptrs[0]+41, 2); options.localSettingsTimestamp.hh = str2int(str); // ��������� ����� � ���������
    strncpy(str, (char*)rx.buf+rx.ptrs[0]+44, 2); options.localSettingsTimestamp.mm = str2int(str);
    strncpy(str, (char*)rx.buf+rx.ptrs[0]+47, 2); options.localSettingsTimestamp.ss = str2int(str);  
    while(*(rx.buf+rx.ptrs[0]+commaPosition) != ',' && *(rx.buf+rx.ptrs[0]+commaPosition) != '\0') commaPosition ++;  // ���� � ������ �������
    commaPosition ++;
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
    options.fFreezeNotifications = str2int((char*)rx.buf+rx.ptrs[0] + ++commaPosition );  
    while(*(rx.buf+rx.ptrs[0]+commaPosition) != ',' && *(rx.buf+rx.ptrs[0]+commaPosition) != '\0') commaPosition ++;  // ���� � ������ �������
    options.FreezeTemp = str2int((char*)rx.buf+rx.ptrs[0] + ++commaPosition );  
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
    strncpy((char*)options.AdminTel, (char*)rx.buf+rx.ptrs[0] + ++commaPosition, 10);  
    while(*(rx.buf+rx.ptrs[0]+commaPosition) != ',' && *(rx.buf+rx.ptrs[0]+commaPosition) != '\0') commaPosition ++;  // ���� � ������ �������
    strncpy((char*)options.OperatorTel, (char*)rx.buf+rx.ptrs[0] + ++commaPosition, 10);

		if(options.PumpWorkDuration == 0 || options.PumpRelaxDuration == 0) PumpStop();  // ��� ��������� ����� ���� � ���� ������ ������� ������������ ������ ��� ������ ������
		eeprom_write_block(&options, (void*)0x00, sizeof(struct TSettings));		// �������� ���������� ��������� � EEPROM

    dropMessage();     // �������� �����������
    waitMessage(); dropMessage();     // �������� "��"
  }
  dropMessage();
  uart_send("AT+HTTPTERM");   // ������� �����: ��� / ��,
  waitMessage(); dropMessage();     // �������� ���
  waitMessage(); dropMessage();     // �������� "��"
}
//----------------------------------------------------------------
void SIM900_SendHistory(void)                     // ������� �� ������ ������� �������
{
  if(SIM900Status < SIM900_GPRS_OK) return;
  uart_send("AT+HTTPINIT");
  waitAnswer("OK", 20);
  uart_send("AT+HTTPPARA=\"CID\",1");
  waitAnswer("OK", 20);

  strcpy(query, "AT+HTTPPARA=\"URL\",\""); strcat(query, link); strcat(query, "?act=sendHistory\"");

  uart_send(query);
  waitAnswer("OK", 20);
  uart_send("AT+HTTPACTION=0");   // ������� �����: ��� / ��, / +HTTPACTION:1,200,20
  waitMessage(); dropMessage();     // �������� ���
  waitMessage(); dropMessage();     // �������� "��"
}
//----------------------------------------------------------------
void RecToHistory(uint8_t eventCode)              // ������� � ������� �������
{
  
}
//----------------------------------------------------------------
