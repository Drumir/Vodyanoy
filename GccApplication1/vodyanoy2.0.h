
#ifndef VODYANOY20_H
#define VODYANOY20_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

#include <stdlib.h>
#include <string.h>
#include "global.h"
#include <util/delay.h>
#include "uart.h"
#include "DS18B20.h"
#include "SPI2TWI.h"



#define MD_PUMPRELAXTIME	0x00	//Коррекция длительности простоя насоса
#define MD_PUMPWORKTIME		0x01	//Коррекция длительности работы насоса
#define MD_DIRPUMP			0x02	//Прямое управление насосом
#define MD_STAT				0x03	//Статистика
#define MD_DIRHEATER		0x04	//Прямое управление обогревателем
#define MD_MAX_T			0x05	//Коррекция максимальной температуры
#define MD_MIN_T			0x06	//Коррекция минимальной температуры
#define MD_CLEAR			0x07	//Сброс настроек или статистики
//................
#define MD_LAST		0x08	//Последнне состояние

#define SIM900_NOTHING        0
#define SIM900_BAT_FAIL       1
#define SIM900_BAT_OK         2
#define SIM900_DOWN           3
#define SIM900_TURN_ON        4
#define SIM900_FAIL           5
#define SIM900_UP             6
#define SIM900_REG_GSM        7
#define SIM900_GSM_FAIL       8
#define SIM900_GSM_OK         9
#define SIM900_REG_GPRS       10
#define SIM900_GPRS_FAIL      11
#define SIM900_GPRS_OK        12
#define SIM900_REG_HTTP       13
#define SIM900_HTTP_FAIL      14
#define SIM900_HTTP_OK        15

#define EVENT_START						0			// Включение водяного
#define EVENT_CONNECTION_FAIL 1     // Неудачная попытка связи
#define EVENT_OTHER						2			// Все остальные события

#define LIGHT_TIME						10		// Время работы подсветки. В сек
#define PUMP_RESTART_PAUSE		18	  // Длительность паузы перед повторным включением насоса. В сек
#define DELTA_TIME						15		// Квант времени в задании расписания. В минутах
#define FIRST_CONNECT_DELAY		11		// Время в секундах от включения устройства до первой попытки связи с сервером

#define LIGHT_ON	PORTB &= 0b11110111
#define LIGHT_OFF PORTB |= 0b00001000

#define RXBUFMAXSIZE 128
#define RXBUFSTRCOUNT 6

FIFO( 128 ) uart_tx_fifo;

struct TTime {
  uint8_t yy, MM, dd, hh, mm, ss;
};

struct TState {
  int16_t balance;    // Количество средств на симкарте в рублях. Копейки отбрасываются
  int16_t Temp;				// Текущая температура в помещении умноженная на 16
  uint16_t Vbat;      // Напряжение на аккуме
}State;

struct TRXB {
  volatile char    buf[RXBUFMAXSIZE+1];     // Буфер для приёма входящих USART сообщений на всякий случай запишем после него 0
  volatile int16_t ptrs[RXBUFSTRCOUNT];     // Массив смещений на начала принятых сообщений. смещение -1 означает, что смещение пустое
  volatile int16_t wptr;                    // Смещение записи в buf
  volatile int16_t startptr;                // Смещение начала текущего записываемого сообщения
  volatile uint16_t buf_overflow_count;     // Счетчик переполнения буфера приема
  volatile uint16_t ptrs_overflow_count;    // Счетчик переполнения ptrs
}rx;

ISR(USART__RXC_vect);  //Имена прерываний определены в c:\Program Files\Atmel\AVR Studio 5.1\extensions\Atmel\AVRGCC\3.3.1.27\AVRToolchain\avr\include\avr\iom32a.h (для mega32a) БЛЕАТЬ!!!
ISR(USART__UDRE_vect);
ISR(INT1_vect);         //CLK от клавы
ISR(TIMER1__COMPA_vect); //обработка совпадения счетчика1. Частота 10Гц.  Имена прерываний определены в c:\Program Files\Atmel\AVR Studio 5.1\extensions\Atmel\AVRGCC\3.3.1.27\AVRToolchain\avr\include\avr\iom8a.h (для mega8a)
ISR(ADC_vect);          // Завершение преобразования АЦП

void ShowStat(void);
void MinToStr(uint16_t Min, char *str);		// Переводит количество минут в строку hh:mm
void DrawMenu(void);
void Save(void);								// Записывает в память текущие настройки
void PumpStop(void);
uint8_t PumpStart(void);							// Возвращает 1 если успешно. Иначе 0;
void HeaterStart(void);
void HeaterStop(void);
void OneMoreMin(void);							// Еще одна секунда прошла.
int AdcToVolts(int A);							// Преобразует измеренное АЦП напряжение в Вольты
void RecordToHistory(uint8_t eventCode);
uint8_t ConnectToServer(void);      // Подключается к серверу для передачи статистики, получения настроек. возвращет 1 если все ок. Иначе 0

void incomingMessage(char* s);
void renewLCD(void);
uint8_t waitAnswer(char *answer, uint16_t timeout);
void dropMessage(void);
void smsToText(char *sms, char *text);
void readSMS(void);
int16_t str2int(char* str);
void waitMessage(void);

void SIM900_SendReport(void);
void SIM900_GetBalance(void);
void SIM900_WaitRegistration(void);
void SIM900_PowerOn(void);
void SIM900_PowerOff(void);
void SIM900_WaitRegistration(void);
void SIM900_GetTime(void);
void SIM900_EnableGPRS(void);
void SIM900_CheckHTTP(void);
void SIM900_PrepareConnection(void);


char buf[23], str[100];
uint8_t					LightLeft, 		  // Сколько осталось работать подсветке в сек.
                CheckUPause, 	  // Задержка проверки питающего напряжения при старте насоса в сек/10
                FrostFlag,		  // Флаг того, что температура падала до -3*С и, возможно, насос замерз.
                BtStat,				  // Состояние кнопок
                Seconds,			  // Счетчик секунд из функции OneMoreSec()
                PumpWorkFlag;   // Флаг, что в данный момент насос должен работать

uint16_t				PumpWorkDuration,  	// Сколько времени должен работать насос
                PumpRelaxDuration, 	// Сколько времени должен отдыхать насос
                PumpPause,			    // Задержка перед повторным включением насоса в сек
                Volts;

volatile uint32_t	SilentLeft;		      // Сколько секунд осталось до попытки связи с сервером

int16_t					HeaterOnTemp,				// Температура отключения обогревателя
                PumpTimeLeft,		    // Сколько осталось качать/отдыхать насосу (в минутах?)
                HeaterOffTemp;			// Температура включения обогревателя

int8_t					MenuMode,			  // Номер текущего режима меню
                SIM900Status;   // Cостояние связи

struct TTime Now, StartTime;

char istr[23];		// Буфер для использования в прерываниях
char query[100];

#endif  // VODYANOY20_H