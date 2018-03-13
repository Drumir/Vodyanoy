/*
 * SIM900.c
 *
 * Created: 12.03.2018 22:11:42
 *  Author: Drumir
 */ 

#include "vodyanoy2.0.h"

void SIM900_PrepareConnection(void)
{
  switch(SIM900Status){
    case SIM900_NOTHING:  // СИМ900 еще даже не запитана
    {                           // Измерим напряжение на батарее
      ADMUX  = 0b11000000;		  // 11 - Опорное напряжение = 2,56В, 0 - выравнивание вправо, 0 - резерв, 0 - резерв, 000 - выбор канала ADC0
      ADCSRA |= 1<<ADSC;		    // Старт преобразования
  	  while (ADCSRA & 0x40);		// Ждем завершения(сброса флага ADSC в 0)
      State.Vbat = ADC;         // Напряжение = ADC * 200
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


void SIM900_SendReport(void)
{
  if(SIM900Status < SIM900_GPRS_OK) return;
  uart_send("AT+HTTPINIT");
  waitAnswer("OK", 20);
  uart_send("AT+HTTPPARA=\"CID\",1");
  waitAnswer("OK", 20);

  itoa(State.Temp, str, 10);
  strcpy(query, "AT+HTTPPARA=\"URL\",\"http://drumir.16mb.com/k/r.php?act=wT&t=");
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
  uart_send("AT+HTTPACTION=0");   // Ответом будет: эхо / ок, / +HTTPACTION:1,200,20
  waitMessage(); dropMessage();     // Отбросим эхо
  waitMessage(); dropMessage();     // Отбросим "ОК"
  waitMessage();
  if(str2int((char*)rx.buf+rx.ptrs[0]+18) > 32){
    dropMessage();     // Отбросим ответ сервера
    uart_send("AT+HTTPREAD=0,128");
    waitMessage(); dropMessage();     // Отбросим эхо
    waitMessage(); dropMessage();     // Отбросим "+HTTPREAD:22"
    waitMessage(); dropMessage();     // Отбросим прочитанное
    waitMessage(); dropMessage();     // Отбросим "ОК"
  }
  dropMessage();
  uart_send("AT+HTTPTERM");   // Ответом будет: эхо / ок,
  waitMessage(); dropMessage();     // Отбросим эхо
  waitMessage(); dropMessage();     // Отбросим "ОК"

}
//----------------------------------------------------------------
void SIM900_PowerOn(void)
{
  if(SIM900Status < SIM900_BAT_OK) return;
  sbi(PORTD, 7);      // Enable 4.0v
  _delay_ms(200);   // Надо ли?
  
  if((PINB & 0b00000001) != 0){   // Модуль уже включен
    SIM900Status = SIM900_UP;
    return;
  }
  
  cbi(PORTB, 1);      // Тянем PWRKEY вниз
  _delay_ms(1500);
  sbi(PORTB, 1);      // Отпускаем PWRKEY
  while((PINB & 0b00000001) == 0);    // Ждем 1 на STATUS
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
    waitMessage();dropMessage();        // Выбрасываем эхо
    waitMessage();
    if(strncmp((char*)rx.buf+rx.ptrs[0], "+CREG: 0,1", 10) == 0) sucess_flag = 1; // Анализируем ответ
    dropMessage();
    waitMessage();dropMessage();      // Выбрасываем "ОК"
    iterations ++;
  }while(sucess_flag == 0 && iterations < 30);

  SIM900Status = SIM900_GSM_OK;
  if(sucess_flag == 0) SIM900Status = SIM900_GSM_FAIL;
}
//----------------------------------------------------------------
void SIM900_GetTime(void)
{
  if(SIM900Status < SIM900_UP) return;
  uart_send("AT+CCLK?");  //   uart_send("AT+CCLK=\"18/03/04,13:6:00+12\"");
  waitMessage(); dropMessage();     // Отбросим эхо
  waitMessage(); dropMessage();     // Отбросим ответ
  Now.yy = (uint8_t) str2int((char*)rx.buf+rx.ptrs[0]);
  Now.MM = (uint8_t) str2int((char*)rx.buf+rx.ptrs[0]+3);
  Now.dd = (uint8_t) str2int((char*)rx.buf+rx.ptrs[0]+6);
  Now.hh = (uint8_t) str2int((char*)rx.buf+rx.ptrs[0]+9);
  Now.mm = (uint8_t) str2int((char*)rx.buf+rx.ptrs[0]+12);
  Now.ss = (uint8_t) str2int((char*)rx.buf+rx.ptrs[0]+15);
  waitMessage(); dropMessage();     // Отбросим ОК
}
//----------------------------------------------------------------
void SIM900_EnableGPRS(void)
{
  uint8_t iterations = 0;
  if(SIM900Status < SIM900_GSM_OK) return;
  SIM900Status = SIM900_REG_GPRS;
  
  //    Эти настройки УЖЕ сохранены в энергонезависимой памяти модуля
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
  //  uart_send("AT+CUSD=1,\"*120#\"");         // Баланс на английском   AT+CUSD=1,"*120$23"$0d
  uart_send("AT+CUSD=1,\"*105*5#\"");         // Запрос баланса  AT+CUSD=1,"*105*5$23"$0d
  waitMessage(); dropMessage();     // Отбросим эхо
  waitMessage(); dropMessage();     // Отбросим ОК
  waitMessage();
  State.balance = str2int((char*)rx.buf+rx.ptrs[0]+10);
  dropMessage();     // Отбросим
}