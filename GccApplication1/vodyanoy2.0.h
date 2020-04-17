
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



#define MD_PUMPRELAXTIME	0x00	//  ����� ������������ ������� ������
#define MD_PUMPWORKTIME		0x01	//  ����� ������������ ������ ������
#define MD_DIRPUMP			  0x02	//  ������ ���������� �������
#define MD_STAT				    0x03	//  ����������
#define MD_DIRHEATER		  0x04	//  ������ ���������� �������������
#define MD_MAX_T			    0x05	//  ��������� ������������ �����������
#define MD_MIN_T			    0x06	//  ��������� ����������� �����������
#define MD_CLEAR			    0x07	//  ����� �������� ��� ����������
#define MD_DEBUG					0x08	//	���������� ����������
#define MD_LAST						0x09	//��������� ���������
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

#define EVENT_NONE                  0   // ������ �������
#define EVENT_BAT_FAIL              1   // ��������� �������� ������������ (������� ������ ����������)
#define EVENT_GSM_FAIL              2   // ������ �� ����� ������������������ � GSM ���� (��� �������?) 
#define EVENT_GPRS_FAIL             3   // ������ �� ����� �������� GPRS (�������� ��������?)
#define EVENT_HTTP_FAIL             4   // ������ �� ����� ������������ � ������� (��������  � ��������?)
#define EVENT_NEW_LOCAL_SETTINGS    5   // ������� (��������) ������ ����� ��������� ��������
#define EVENT_NEW_REMOTE_SETTINGS   6   // �������� (����� ��������) ������ ����� ��������� �������� 
#define EVENT_PUMP_START_AUTO       7   // ����� ������ �� ����������
#define EVENT_PUMP_START_MANUAL     8   // ����� ������ �������
#define EVENT_PUMP_START_REMOTE     9   // ����� ������ ��������
#define EVENT_PUMP_STOP_AUTO        10  // ���������� ������ �� ����������
#define EVENT_PUMP_STOP_MANUAL      11  // ���������� ������ �������
#define EVENT_PUMP_STOP_REMOTE      12  // ���������� ������ ��������
#define EVENT_PUMP_STOP_EMERGENCY   13  // ��������� ���������� ������
#define EVENT_PUMP_NO_SHEDULE       14  // ������ ������ �� �������� ��-�� ���������� ����������
#define EVENT_HEATER_START_AUTO     15  // �������������� ��������� ������������ 
#define EVENT_HEATER_START_MANUAL   16  // ������ ��������� ������������
#define EVENT_HEATER_START_REMOTE   17  // ��������� ��������� ������������
#define EVENT_HEATER_STOP_AUTO      18  // �������������� ���������� ������������
#define EVENT_HEATER_STOP_MANUAL    19  // ������ ���������� ������������
#define EVENT_HEATER_STOP_EMERGENCY 20  // ��������� ���������� ������������
#define EVENT_DOOR_OPEN             21  // �������� ������� �����
#define EVENT_DOOR_CLOSE            22  // �������� ������� �����
#define EVENT_AC_FAIL               23  // ���������� �������������� (380�)
#define EVENT_AC_RESTORE            24  // ������������� �������������� (380�)
#define EVENT_FREEZE                25  // �������� ���������
#define EVENT_TOO_HOOT              26  // ��������
#define EVENT_RXB_OVERLOAD          27  // ������������ �������� ������ RX
#define EVENT_HISTORY_OVERLOAD      28  // ������������ �������
#define EVENT_START						      29	// ��������� ��������
#define EVENT_BALANCE_LOW			      30	// ���� ����� �� ����� ��� �����
#define EVENT_PUMP_CAN_FREEZE				31  // ������ ������ �� �������� ��-�� ��������� ���������
#define EVENT_OTHER									32  // ��� ��������� �������

#define LIGHT_TIME						30		// ����� ������ ���������. � ���
#define PUMP_RESTART_PAUSE		30	  // ������������ ����� ����� ��������� ���������� ������. � ���
#define DELTA_TIME						15		// ����� ������� � ������� ����������. � �������
#define FIRST_CONNECT_DELAY		25 		// ����� � �������� �� ��������� ���������� �� ������ ������� ����� � ��������

#define LIGHT_ON	PORTB &= ~(1 << 3)
#define LIGHT_OFF PORTB |= (1 << 3)

#define RXBUFMAXSIZE 300
#define RXBUFSTRCOUNT 10

FIFO( 128 ) uart_tx_fifo;

struct TTime {
  uint8_t yy, MM, dd, hh, mm, ss;
};

struct TState {
  int16_t balance;				// ���������� ������� �� �������� � ������. ������� �������������
  int16_t Temp;						// ������� ����������� � ��������� ���������� �� 16
  uint16_t Vbat;					// ���������� �� ������ ���������� �� 200
	uint8_t PowerFailFlag;	//���� ������ �������
	uint8_t OpenDoorFlag;		//���� �������� �����
	uint8_t FloodingFlag;		//���� ����������
}State;

struct TSettings {          // ��������� ��� �������� ���� ����������� � eeprom �������� � ����������.
  uint8_t       FrostFlag,		            // ���� ����, ��� ����������� ������ �� -3*� �, ��������, ����� ������.
  PumpWorkFlag,             // ����, ��� � ������ ������ ����� ������ ��������
  fFreezeNotifications,     // ����� ���������� � ���������. 1 � 3 ���� - ��� ���������. �� 2 ���� - ��� ������. � 1 - ������ ���������. � 0 - ������ ������.
  fWarmNotifications,       // ����� ���������� � ���������.
  fDoorNotifications,       // ����� ���������� �� �������� �����.
  fFloodNotifications,      // ����� ���������� � ����������.
  fPowerNotifications,      // ����� ���������� � ������� ����������������.
  fPowerRestNotifications,  // ����� ���������� � �������������� ����������������.
  fOfflineNotifications,    // ����� ���������� � ���������� ���������� ���������.
  fBalanceNotifications,    // ����� ���������� � ����������� ������� �� �����
  fDailyNotifications;      // ����� ����������� ���������� � ���������
  
  char          OperatorTel[11],          // ����� �������� ���������
  AdminTel[11],             // ����� �������� ��������������
	Link[40];									// ����� ������� ������� "vodyanoy.000webhostapp.com/r.php"
  
  
  uint16_t			PumpWorkDuration,  	      // ������� ������� ������ �������� ����� (� �������)
  PumpRelaxDuration, 	      // ������� ������� ������ �������� ����� (� �������)
  ConnectPeriod,            // ������ (� �������) �������� �� ������. ���� 0 - ������ ��� ������������� ��� �� ������
  DisconnectionTime,        // ������������ ���������� ����� ��� ���������� �� ���������� �����
  MinBalance,               // ������, ��� ���������� �������� ����� ���������� � ����������� �������
  DailyReportTime;          // ����� ����������� ������ � ��������� (� ������� � ������ �����)

  int16_t				PumpTimeLeft,		          // ������� �������� ������/�������� ������ (� �������)
  HeaterOnTemp,				      // ����������� ���������� ������������
  HeaterOffTemp,			      // ����������� ��������� ������������
  FreezeTemp,               // ����������� �������������� � ���������
  WarmTemp;                 // ����������� �������������� � ���������

  
  struct TTime  localSettingsTimestamp;
};

struct TRXB {
  char    buf[RXBUFMAXSIZE];     // ����� ��� ����� �������� USART ��������� �� ������ ������ ������� ����� ���� 0
  int16_t ptrs[RXBUFSTRCOUNT];     // ������ �������� �� ������ �������� ���������. �������� -1 ��������, ��� �������� ������
  uint16_t buf_overflow_count;     // ������� ������������ ������ ������
  uint16_t ptrs_overflow_count;    // ������� ������������ ptrs
  int16_t wptr;                    // �������� ������ � buf
  int16_t startptr;                // �������� ������ �������� ������������� ���������
};

ISR(USART__RXC_vect);  //����� ���������� ���������� � c:\Program Files\Atmel\AVR Studio 5.1\extensions\Atmel\AVRGCC\3.3.1.27\AVRToolchain\avr\include\avr\iom32a.h (��� mega32a) ������!!!
ISR(USART__UDRE_vect);
ISR(INT1_vect);         //CLK �� �����
ISR(INT2_vect);         //INT �� �������� �������
ISR(TIMER1__COMPA_vect); //��������� ���������� ��������1. ������� 10��.  ����� ���������� ���������� � c:\Program Files\Atmel\AVR Studio 5.1\extensions\Atmel\AVRGCC\3.3.1.27\AVRToolchain\avr\include\avr\iom8a.h (��� mega8a)
ISR(ADC_vect);          // ���������� �������������� ���

void ShowStat(void);
void MinToStr(uint16_t Min, char *str);		// ��������� ���������� ����� � ������ hh:mm
void OnKeyPress(void);					// ���������� �� �������� ����� ���� ���������� ���������� ����� �������� ���� �������.
void DrawMenu(void);
void Save(void);								// ���������� � ������ ������� ���������
void PumpStop(void);
uint8_t PumpStart(void);							// ���������� 1 ���� �������. ����� 0;
void HeaterStart(void);
void HeaterStop(void);
void OneMoreMin(void);							// ��� ���� ������ ������.
void OneMoreSec(void);							// ��� ���� ������� ������.
int AdcToVolts(int A);							// ����������� ���������� ��� ���������� � ������
void RecToHistory(uint8_t eventCode);
uint8_t ConnectToServer(void);      // ������������ � ������� ��� �������� ����������, ��������� ��������. ��������� 1 ���� ��� ��. ����� 0
void SoftReset(void);										// ��������� �� ���������� ������� ��� ���������� �������� ��������� �� 60 ���
void measureBattery(void);					// �������� ���������� ������������, ���������� ��� � state.vBat
void CheckIncomingMessages(void);		// ��������� ������� �������� ������������� ���������. ������������ ��.


void renewLCD(void);
uint8_t waitAnswer(char *answer, uint16_t timeout);
void dropMessage(void);
void smsToText(char *sms, char *text);
void readSMS(void);
int16_t str2int(char* str);
void waitMessage(void);
int8_t timeCompare(struct TTime *timeOne, struct TTime *timeTwo);
void strcpyPM(char *dest, const char *PMsrc);		// �������� �� PROGMEM ������ � dest;


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
void SIM900_GetRemoteSettingsTimestamp(void);      // �������� ����� ���������� ��������� �������� �� �������
void SIM900_SendSettings(void);                    // �������� ��������� �� ������ ��� ����������
void SIM900_GetSettings(void);                     // ����� ��������� � ������� � ��������� ��
void SIM900_SendStatus(void);                      // ������� �� ������ ������� ���������
void SIM900_SendHistory(void);                     // ������� �� ������ ������� �������


uint8_t								LightLeft, 					// ������� �������� �������� ��������� � ���.
											CheckUPause, 				// �������� �������� ��������� ���������� ��� ������ ������ � ���/10
											BtStat,							// ��������� ������
											Seconds;						// ������� ������ �� ������� OneMoreSec()
uint16_t							PumpPause,			    // �������� ����� ��������� ���������� ������ � ���
											Volts;
volatile uint32_t			SilentLeft;		      // ������� ������ �������� �� ������� ����� � ��������
int8_t								MenuMode,						// ����� �������� ������ ����
											SIM900Status,				// C�������� �����
											settingsWasChanged, // ���� ������������� ��������
											OneMoreSecCount;		// ���������� ������ ��� �� ����������� by OneMoreSec(). ����� ������ � ���������.
volatile uint8_t	bstat;							// ���� �� ����, �� ������������� ��� ������� �������   

struct TTime Now, remoteSettingsTimestamp;
volatile struct TRXB rx;
struct TSettings options;     // � �� ������� �� �� volatile ?!??!!?
volatile uint16_t TimeoutTackts;		// ������������� ������������� �� ���� ����������� ������� 10 ��� � �������
volatile uint16_t TimeoutsCount;		// ������� ������������ ��������� � �������� waitMessage(), waitAnswer()

char strI[23];		// ����� ��� ������������� � �����������
char strD[23];		// ����� ��� ������������� � ������ �� �������
char strS[100];		// ����� ��� ������������� � �������� SIM900
char query[100];  // ��������� ����� ��� ������������ Http ��������.
char buf[23];			// ��� ���� ����� ��� �������
//char DebugStr[35];

static const char PM_link[]						PROGMEM	= "vodyanoy.000webhostapp.com/r.php";
static const char MSG_BatFail[]				PROGMEM = "Battery_FAIL  ";
static const char MSG_SimFail[]				PROGMEM = "SIM900_FAIL   ";
static const char MSG_GSMFail[]				PROGMEM = "SIM90_GSM_FAIL";
static const char MSG_GPRSFail[]			PROGMEM = "SIM9_GPRS_FAIL";
static const char MSG_HTTPFail[]			PROGMEM = "SIM9_HTTP_FAIL";
static const char MSG_Loading[]				PROGMEM = "�������� �����";
static const char MSG_Stat[]					PROGMEM = "  ����������  ";
static const char MSG_PumpSchedule[]	PROGMEM = " �����.������ ";
static const char MSG_Reset[]					PROGMEM = "    �����     ";
static const char MSG_HeaterSchedul[] PROGMEM = "�������.������";
static const char MSG_Successful[]		PROGMEM = "   �������    ";
static const char MSG_Blank[]					PROGMEM = "              ";
static const char MSG_ToReset[]				PROGMEM = "   ��������   ";
static const char MSG_Freezing[]			PROGMEM = "����.���������";
static const char MSG_RightNow[]			PROGMEM = " ����� ������ ";
static const char MSG_WorkRelax[]			PROGMEM = " �����  ����� ";
static const char MSG_OnOff[]					PROGMEM = "  ���   ����  ";
static const char MSG_Pump[]					PROGMEM = "    �����     ";
static const char MSG_Heater[]				PROGMEM = "   �������    ";
static const char MSG_On[]						PROGMEM = "     ���      ";
static const char MSG_Off[]						PROGMEM = "     ����     ";
static const char MSG_ToSettings[]		PROGMEM = "  ���������   ";
static const char MSG_ToStat[]				PROGMEM = "  ����������  ";
static const char MSG_Info[]					PROGMEM = "�����-� ����-�";
static const char MSG_TimerOff[]			PROGMEM = "������ ����   ";
static const char MSG_Celsium[]				PROGMEM = "*C           ";

																					
#endif  // VODYANOY20_H