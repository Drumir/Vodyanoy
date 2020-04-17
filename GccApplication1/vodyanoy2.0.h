
#ifndef VODYANOY20_H
#define VODYANOY20_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>

#include <stdlib.h>
#include <string.h>
#include "global.h"
#include <util/delay.h>
#include "uart.h"
#include "DS18B20.h"
#include "3310_routines.h"



#define MD_PUMPRELAXTIME	0x00	//  Выбор длительности простоя насоса
#define MD_PUMPWORKTIME		0x01	//  Выбор длительности работы насоса
#define MD_DIRPUMP			  0x02	//  Прямое управление насосом
#define MD_STAT				    0x03	//  Статистика
#define MD_DIRHEATER		  0x04	//  Прямое управление обогревателем
#define MD_MAX_T			    0x05	//  Коррекция максимальной температуры
#define MD_MIN_T			    0x06	//  Коррекция минимальной температуры
#define MD_CLEAR			    0x07	//  Сброс настроек или статистики
#define MD_DEBUG					0x08	//	Отладочная информация
#define MD_LAST						0x09	//Последнне состояние
//................

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

#define EVENT_NONE                  0   // Пустое событие
#define EVENT_BAT_FAIL              1   // Провалена проверка аккумулятора (слишком низкое напряжение)
#define EVENT_GSM_FAIL              2   // Модуль не может зарегистрироваться в GSM сети (нет сигнала?) 
#define EVENT_GPRS_FAIL             3   // Модуль не может включить GPRS (отключен интернет?)
#define EVENT_HTTP_FAIL             4   // Модуль не может подключиться к серверу (проблема  с сервером?)
#define EVENT_NEW_LOCAL_SETTINGS    5   // Вручную (локально) заданы новые настройки водяного
#define EVENT_NEW_REMOTE_SETTINGS   6   // Удаленно (через интернет) заданы новые настройки водяного 
#define EVENT_PUMP_START_AUTO       7   // Старт насоса по расписанию
#define EVENT_PUMP_START_MANUAL     8   // Старт насоса вручную
#define EVENT_PUMP_START_REMOTE     9   // Старт насоса удаленно
#define EVENT_PUMP_STOP_AUTO        10  // Отключение насоса по расписанию
#define EVENT_PUMP_STOP_MANUAL      11  // Отключение насоса вручную
#define EVENT_PUMP_STOP_REMOTE      12  // Отключение насоса удаленно
#define EVENT_PUMP_STOP_EMERGENCY   13  // Аварийное отключение насоса
#define EVENT_PUMP_NO_SHEDULE       14  // Запуск насоса не разрешен из-за отсутствия расписания
#define EVENT_HEATER_START_AUTO     15  // Автоматическое включение обогревателя 
#define EVENT_HEATER_START_MANUAL   16  // Ручное включение обогревателя
#define EVENT_HEATER_START_REMOTE   17  // Удаленное включение обогревателя
#define EVENT_HEATER_STOP_AUTO      18  // Автоматическое отключение обогревателя
#define EVENT_HEATER_STOP_MANUAL    19  // Ручное отключение обогревателя
#define EVENT_HEATER_STOP_EMERGENCY 20  // Аварийное отключение обогревателя
#define EVENT_DOOR_OPEN             21  // Открытие входной двери
#define EVENT_DOOR_CLOSE            22  // Закрытие входной двери
#define EVENT_AC_FAIL               23  // Отключение электропитания (380В)
#define EVENT_AC_RESTORE            24  // Возобновление электропитания (380В)
#define EVENT_FREEZE                25  // Возможно заморозка
#define EVENT_TOO_HOOT              26  // Перегрев
#define EVENT_RXB_OVERLOAD          27  // Переполнение приёмного буфера RX
#define EVENT_HISTORY_OVERLOAD      28  // Переполнение истории
#define EVENT_START						      29	// Включение водяного
#define EVENT_BALANCE_LOW			      30	// Мало денег на счету сим карты
#define EVENT_PUMP_CAN_FREEZE				31  // Запуск насоса не разрешен из-за возможной заморозки
#define EVENT_OTHER									32  // Все остальные события

#define LIGHT_TIME						30		// Время работы подсветки. В сек
#define PUMP_RESTART_PAUSE		30	  // Длительность паузы перед повторным включением насоса. В сек
#define DELTA_TIME						15		// Квант времени в задании расписания. В минутах
#define FIRST_CONNECT_DELAY		25 		// Время в секундах от включения устройства до первой попытки связи с сервером

#define LIGHT_ON	PORTB &= ~(1 << 3)
#define LIGHT_OFF PORTB |= (1 << 3)

#define RXBUFMAXSIZE 300
#define RXBUFSTRCOUNT 10

FIFO( 128 ) uart_tx_fifo;

struct TTime {
  uint8_t yy, MM, dd, hh, mm, ss;
};

struct TState {
  int16_t balance;				// Количество средств на симкарте в рублях. Копейки отбрасываются
  int16_t Temp;						// Текущая температура в помещении умноженная на 16
  uint16_t Vbat;					// Напряжение на аккуме умноженное на 200
	uint8_t PowerFailFlag;	//Флаг отказа питания
	uint8_t OpenDoorFlag;		//Флаг открытой двери
	uint8_t FloodingFlag;		//Флаг затопления
}State;

struct TSettings {          // Структура для хранения всех сохраняемых в eeprom настроек и переменных.
  uint8_t       FrostFlag,		            // Флаг того, что температура падала до -3*С и, возможно, насос замерз.
  PumpWorkFlag,             // Флаг, что в данный момент насос должен работать
  fFreezeNotifications,     // Флаги оповещения о заморозке. 1 в 3 бите - смс оператору. Во 2 бите - смс админу. в 1 - звонок оператору. в 0 - звонок админу.
  fWarmNotifications,       // Флаги оповещения о перегреве.
  fDoorNotifications,       // Флаги оповещения об открытии двери.
  fFloodNotifications,      // Флаги оповещения о затоплении.
  fPowerNotifications,      // Флаги оповещения о перебое электроснабжения.
  fPowerRestNotifications,  // Флаги оповещения о восстановлении электроснабжения.
  fOfflineNotifications,    // Флаги оповещения о длительном отсутствии интернета.
  fBalanceNotifications,    // Флаги оповещения о критическом балансе на счёте
  fDailyNotifications;      // Флаги ежедневного оповещения о состоянии
  
  char          OperatorTel[11],          // Номер телефона оператора
  AdminTel[11],             // Номер телефона администратора
	Link[40];									// Адрес сервера формата "vodyanoy.000webhostapp.com/r.php"
  
  
  uint16_t			PumpWorkDuration,  	      // Сколько времени должен работать насос (в минутах)
  PumpRelaxDuration, 	      // Сколько времени должен отдыхать насос (в минутах)
  ConnectPeriod,            // Период (в минутах) докладов на сервер. Если 0 - только при необходимости или по звонку
  DisconnectionTime,        // Длительность отсутствия связи для оповещения об отсутствии связи
  MinBalance,               // Баланс, при достижении которого нужно оповестить о критическом балансе
  DailyReportTime;          // Время ежедневного отчета о состоянии (в минутах с начала суток)

  int16_t				PumpTimeLeft,		          // Сколько осталось качать/отдыхать насосу (в минутах)
  HeaterOnTemp,				      // Температура отключения обогревателя
  HeaterOffTemp,			      // Температура включения обогревателя
  FreezeTemp,               // Температура предупреждения о заморозке
  WarmTemp;                 // Температура предупреждения о перегреве

  
  struct TTime  localSettingsTimestamp;
};

struct TRXB {
  char    buf[RXBUFMAXSIZE];     // Буфер для приёма входящих USART сообщений на всякий случай запишем после него 0
  int16_t ptrs[RXBUFSTRCOUNT];     // Массив смещений на начала принятых сообщений. смещение -1 означает, что смещение пустое
  uint16_t buf_overflow_count;     // Счетчик переполнения буфера приема
  uint16_t ptrs_overflow_count;    // Счетчик переполнения ptrs
  int16_t wptr;                    // Смещение записи в buf
  int16_t startptr;                // Смещение начала текущего записываемого сообщения
};

ISR(USART__RXC_vect);  //Имена прерываний определены в c:\Program Files\Atmel\AVR Studio 5.1\extensions\Atmel\AVRGCC\3.3.1.27\AVRToolchain\avr\include\avr\iom32a.h (для mega32a) БЛЕАТЬ!!!
ISR(USART__UDRE_vect);
ISR(INT1_vect);         //CLK от клавы
ISR(INT2_vect);         //INT от монитора питания
ISR(TIMER1__COMPA_vect); //обработка совпадения счетчика1. Частота 10Гц.  Имена прерываний определены в c:\Program Files\Atmel\AVR Studio 5.1\extensions\Atmel\AVRGCC\3.3.1.27\AVRToolchain\avr\include\avr\iom8a.h (для mega8a)
ISR(ADC_vect);          // Завершение преобразования АЦП

void ShowStat(void);
void MinToStr(uint16_t Min, char *str);		// Переводит количество минут в строку hh:mm
void OnKeyPress(void);					// Вызывается из главного цикла если обработчик прерываний клавы выставил флаг нажатия.
void DrawMenu(void);
void Save(void);								// Записывает в память текущие настройки
void PumpStop(void);
uint8_t PumpStart(void);							// Возвращает 1 если успешно. Иначе 0;
void HeaterStart(void);
void HeaterStop(void);
void OneMoreMin(void);							// Еще одна минута прошла.
void OneMoreSec(void);							// Еще одна секунда прошла.
int AdcToVolts(int A);							// Преобразует измеренное АЦП напряжение в Вольты
void RecToHistory(uint8_t eventCode);
uint8_t ConnectToServer(void);      // Подключается к серверу для передачи статистики, получения настроек. возвращет 1 если все ок. Иначе 0
void SoftReset(void);										// Вызыватся из прерывания таймера при подвисании основной программы на 60 сек
void measureBattery(void);					// Измеряет напряжение аккумулятора, записывает его в state.vBat
void CheckIncomingMessages(void);		// Проверяет наличие принятых необработаных сообщений. Обрабатывает их.


void renewLCD(void);
uint8_t waitAnswer(char *answer, uint16_t timeout);
void dropMessage(void);
void smsToText(char *sms, char *text);
void readSMS(void);
int16_t str2int(char* str);
void waitMessage(void);
int8_t timeCompare(struct TTime *timeOne, struct TTime *timeTwo);
void strcpyPM(char *dest, const char *PMsrc);		// Копирует из PROGMEM строку в dest;


void SIM900_GetBalance(void);
void SIM900_WaitRegistration(void);
void SIM900_PowerOn(void);
void SIM900_PowerOff(void);
void SIM900_WaitRegistration(void);
void SIM900_GetTime(void);
void SIM900_SetTimeFromServer(void);
void SIM900_EnableGPRS(void);
void SIM900_CheckHTTP(void);
void SIM900_PrepareConnection(void);
void SIM900_GetRemoteSettingsTimestamp(void);      // Получает время последнего изменения настроек на сервере
void SIM900_SendSettings(void);                    // Отсылает настройки на сервер для сохранения
void SIM900_GetSettings(void);                     // Берет настройки с сервера м применяет их
void SIM900_SendStatus(void);                      // Отошлем на сервер текущее состояние
void SIM900_SendHistory(void);                     // Отошлем на сервер историю событий


uint8_t								LightLeft, 					// Сколько осталось работать подсветке в сек.
											CheckUPause, 				// Задержка проверки питающего напряжения при старте насоса в сек/10
											BtStat,							// Состояние кнопок
											Seconds;						// Счетчик секунд из функции OneMoreSec()
uint16_t							PumpPause,			    // Задержка перед повторным включением насоса в сек
											Volts;
volatile uint32_t			SilentLeft;		      // Сколько секунд осталось до попытки связи с сервером
int8_t								MenuMode,						// Номер текущего режима меню
											SIM900Status,				// Cостояние связи
											settingsWasChanged, // Флаг несохраненных настроек
											OneMoreSecCount;		// Количество секунд еще не обработаных by OneMoreSec(). можно судить о зависании.
volatile uint8_t	bstat;							// Если не ноль, то необработаный код нажатой клавиши   

struct TTime Now, remoteSettingsTimestamp;
volatile struct TRXB rx;
struct TSettings options;     // А не сделать ли их volatile ?!??!!?
volatile uint16_t TimeoutTackts;		// Автоматически декремируется до нуля прерыванием таймера 10 раз в секунду
volatile uint16_t TimeoutsCount;		// Счетчик срабатывания таймаутов в функциях waitMessage(), waitAnswer()

char strI[23];		// Буфер для использования в прерываниях
char strD[23];		// Буфер для использования в выводе на дисплей
char strS[100];		// Буфер для использования в функциях SIM900
char query[100];  // Текстовый буфер для формирования Http запросов.
char buf[23];			// Еще один буфер для дисплея
//char DebugStr[35];

static const char PM_link[]						PROGMEM	= "vodyanoy.000webhostapp.com/r.php";
static const char MSG_BatFail[]				PROGMEM = "Battery_FAIL  ";
static const char MSG_SimFail[]				PROGMEM = "SIM900_FAIL   ";
static const char MSG_GSMFail[]				PROGMEM = "SIM90_GSM_FAIL";
static const char MSG_GPRSFail[]			PROGMEM = "SIM9_GPRS_FAIL";
static const char MSG_HTTPFail[]			PROGMEM = "SIM9_HTTP_FAIL";
static const char MSG_Loading[]				PROGMEM = "Устновка связи";
static const char MSG_Stat[]					PROGMEM = "  Статистика  ";
static const char MSG_PumpSchedule[]	PROGMEM = " Насос.Распис ";
static const char MSG_Reset[]					PROGMEM = "    Сброс     ";
static const char MSG_HeaterSchedul[] PROGMEM = "Обогрев.Распис";
static const char MSG_Successful[]		PROGMEM = "   Успешно    ";
static const char MSG_Blank[]					PROGMEM = "              ";
static const char MSG_ToReset[]				PROGMEM = "   Сбросить   ";
static const char MSG_Freezing[]			PROGMEM = "Возм.заморозка";
static const char MSG_RightNow[]			PROGMEM = " Прямо сейчас ";
static const char MSG_WorkRelax[]			PROGMEM = " Работ  Стоит ";
static const char MSG_OnOff[]					PROGMEM = "  ВКЛ   ВыКЛ  ";
static const char MSG_Pump[]					PROGMEM = "    Насос     ";
static const char MSG_Heater[]				PROGMEM = "   Обогрев    ";
static const char MSG_On[]						PROGMEM = "     ВКЛ      ";
static const char MSG_Off[]						PROGMEM = "     ВыКЛ     ";
static const char MSG_ToSettings[]		PROGMEM = "  настройки   ";
static const char MSG_ToStat[]				PROGMEM = "  статистику  ";
static const char MSG_Info[]					PROGMEM = "Отлад-я инфо-я";
static const char MSG_TimerOff[]			PROGMEM = "Таймер ОТКЛ   ";
static const char MSG_Celsium[]				PROGMEM = "*C           ";

																					
#endif  // VODYANOY20_H