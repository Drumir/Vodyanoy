
#ifndef VODYANOY20_H
#define VODYANOY20_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
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

#define EVENT_PUMP_START_AUTO       0   // ����� ������ ������������� (�� ���������� ��� ����� ���� AC)
#define EVENT_PUMP_START_MANUAL     1   // ����� ������ �������
#define EVENT_PUMP_START_REMOTE     2   // ����� ������ ��������
#define EVENT_PUMP_STOP_AUTO        3		// ���������� ������ �� ����������
#define EVENT_PUMP_STOP_MANUAL      4		// ���������� ������ �������
#define EVENT_PUMP_STOP_REMOTE      5		// ���������� ������ ��������
#define EVENT_PUMP_STOP_EMERGENCY   6		// ��������� ���������� ������
#define EVENT_PUMP_FAIL_NO_SCHEDULE	7		// ��������� ������ �� ������� ��-�� �������������� ����������
#define EVENT_PUMP_FAIL_FREEZE			8		// ��������� ������ �� ������� ��-�� ���������
#define EVENT_PUMP_FAIL_NO_AC				9		// ��������� ������ �� ������� ��-�� �������������� �������

#define EVENT_HEATER_START_AUTO     10  // �������������� ��������� ������������
#define EVENT_HEATER_STOP_AUTO      11  // �������������� ���������� ������������
#define EVENT_HEATER_START_MANUAL   12  // ������ ��������� ������������
#define EVENT_HEATER_STOP_MANUAL    13  // ������ ���������� ������������
#define EVENT_HEATER_START_REMOTE   14  // ��������� ��������� ������������
#define EVENT_HEATER_STOP_REMOTE		15  // ��������� ���������� ������������
#define EVENT_HEATER_STOP_EMERGENCY 16  // ��������� ���������� ������������

#define EVENT_AC_FAIL               20  // ���������� �������������� (380�)
#define EVENT_AC_RESTORE            21  // ������������� �������������� (380�)

#define EVENT_FLOOD_START           30  // ����������
#define EVENT_FLOOD_STOP						31  // ��������

#define EVENT_DOOR_OPEN             40  // �������� ������� �����
#define EVENT_DOOR_CLOSE            41  // �������� ������� �����

#define EVENT_LOW_BALANCE						50  // ������ ���� �����������
#define EVENT_GSM_FAIL              51  // ������ �� ����� ������������������ � GSM ���� (��� �������?)
#define EVENT_GPRS_FAIL             52  // ������ �� ����� �������� GPRS (�������� ��������?)
#define EVENT_HTTP_FAIL             53  // ������ �� ����� ������������ � ������� (��������  � ��������?)
#define EVENT_NEW_REMOTE_SETTINGS   54  // �������� (����� ��������) ������ ����� ��������� ��������
#define EVENT_CHECK_CONN_OK         55  // �������� ����� �������� SIM900_CheckConnection ������ �������
#define EVENT_REPAIRCONN_OK         56  // �������������� ����� �������� SIM900_RepairConnection ������ �������
#define EVENT_SIM900_RESTART				57	// �������������� ������� ������ �������� SIM900_RepairConnection
#define EVENT_RESTART_BY_SMS				58	// ������ ������������ �������� �� ���
#define EVENT_ERROR_601				      59	// ������ HTTP 601	

#define EVENT_BAT_FAIL              60  // ��������� �������� ������������ (������� ������ ����������)
#define EVENT_NEW_LOCAL_SETTINGS    61  // ������� (��������) ������ ����� ��������� ��������
#define EVENT_FREEZE                62  // �������� ���������
#define EVENT_WARM									63  // ��������
#define EVENT_RXB_OVERLOAD          64  // ������������ �������� ������ RX
#define EVENT_HISTORY_OVERLOAD      65  // ������������ �������
#define EVENT_START						      66	// ��������� ��������
#define EVENT_FROST									67  // ��������������

#define EVENT_NONE                  0xFF   // ������ �������

#define LIGHT_TIME						60		// ����� ������ ���������. � ���
#define PUMP_RESTART_PAUSE		30	  // ������������ ����� ����� ��������� ���������� ������. � ���
#define DELTA_TIME						15		// ����� ������� � ������� ����������. � �������
#define FIRST_CONNECT_DELAY		25 		// ����� � �������� �� ��������� ���������� �� ������ ������� ����� � ��������
#define UNSTABLE_POWER_DELAY	10		// ����� � �������� ������� ������ ������ ����� ��������� �������, ����� ����� ���� ������� ������� ����������.

#define LIGHT_ON	PORTB &= ~(1 << 3)
#define LIGHT_OFF PORTB |= (1 << 3)

#define RXBUFMAXSIZE 300						// ������ ������ �������� UART
#define RXBUFSTRCOUNT 10						// ������ ������� ��� �������� �������� �� ������ �������� ���������

#define HISTORYLENGTH	40						// ������ ������ �������. ������ ������ � 4 ����
#define SOFT_RESET_FLAG 0xAAAA

FIFO( 128 ) uart_tx_fifo;						// ����� ����������� UART 

struct TTime {
  uint8_t yy, MM, dd, hh, mm, ss;
};

struct TState {
  int16_t balance;						// ���������� ������� �� �������� � ������. ������� �������������
  int16_t Temp;								// ������� ����������� � ��������� ���������� �� 16
  uint16_t Vbat;							// ���������� �� ������ ���������� �� 200
	uint8_t PowerFailFlag;			//���� ������ �������
	uint8_t OpenDoorFlag;				//���� �������� �����
	uint8_t FloodingFlag;				//���� ����������
	uint16_t PowerFailSecCount; // ������� ������ �� �������, ����� ������� ����� ������� ����������
	uint16_t PumpPause;			    // �������� ����� ��������� ���������� ������ � ���
	
}State;

struct TNotifications {		// ����� � ����������� ������� � ������� ����� ��������� ���������/������
// 0 - ������� �� ���������, 1 - ������� ���������, ���������� ��� �� ����, -1 - ������� ���������, ���������� ��� �����������
	int8_t	Frost,				// ���� � ������ ����������� 
					Warm,					// ���� � ������� �����������
					Door,					// ���� �������� �����
					Flood,				// ���� ���������� ���������
					PowerFail,		// ���� ������ �������� ����������
					PowerRestore,	// ���� �������������� �������� ����������
					Offline,			// ���� ������� ����������� ���������� �����
					Balance,			// ���� ������� �������
					Daily;				// ���� ����������� ���������� � ��������
	}Notifications;

struct TSettings {          // ��������� ��� �������� ���� ����������� � eeprom �������� � ����������.
  uint8_t	FreezeFlag,				// ���� ����, ��� ����������� ������ �� -3*� �, ��������, ����� ������.
  PumpWorkFlag,             // ����, ��� � ������ ������ ����� ������ ��������
	DirectControlFlags,				// ����� ������� ���������� ���������� 0b000,�����������������, ��� �������, ��� �������, ��� �����, ��� �����
  fFrostNotifications,			// ����� ���������� �� ����������. 1 � 3 ���� - ��� ���������. �� 2 ���� - ��� ������. � 1 - ������ ���������. � 0 - ������ ������.
  fWarmNotifications,       // ����� ���������� � ���������.
  fDoorNotifications,       // ����� ���������� �� �������� �����.
  fFloodNotifications,      // ����� ���������� � ����������.
  fPowerNotifications,      // ����� ���������� � ������� ����������������.
  fPowerRestNotifications,  // ����� ���������� � �������������� ����������������.
  fOfflineNotifications,    // ����� ���������� � ���������� ���������� ���������.
  fBalanceNotifications,    // ����� ���������� � ����������� ������� �� �����
  fDailyNotifications;      // ����� ����������� ���������� � ���������
  
  char	OperatorTel[11],		// ����� �������� ���������
  AdminTel[11],             // ����� �������� ��������������
	Link[40];									// ����� ������� ������� "vodyanoy.000webhostapp.com/r.php"
  
  
  uint16_t PumpWorkDuration,// ������� ������� ������ �������� ����� (� �������)
  PumpRelaxDuration, 	      // ������� ������� ������ �������� ����� (� �������)
  ConnectPeriod,            // ������ (� �������) �������� �� ������. ���� 0 - ������ ��� ������������� ��� �� ������
  DisconnectionTime,        // ������������ ���������� ����� ��� ���������� �� ���������� �����
  MinBalance,               // ������, ��� ���������� �������� ����� ���������� � ����������� �������
  DailyReportTime;          // ����� ����������� ������ � ��������� (� ������� � ������ �����)

  int16_t	PumpTimeLeft,			// ������� �������� ������/�������� ������ (� �������)
  HeaterOnTemp,				      // ����������� ���������� ������������
  HeaterOffTemp,			      // ����������� ��������� ������������
  FrostTemp,								// ����������� �������������� �� ����������
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

struct HistoryEvent {
	uint8_t					EventCode;
	struct TTime		EventTime;
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
void OnPowerFail(void);
void OnPowerRestore(void);
void DrawMenu(void);
void Save(void);								// ���������� � ������ ������� ���������
void PumpStop(void);
uint8_t PumpStart(void);							// ���������� 1 ���� �������. ����� 0;
void HeaterStart(void);
void HeaterStop(void);
void OneMoreMin(void);									// ��� ���� ������ ������.
void OneMoreSec(void);									// ��� ���� ������� ������.
int AdcToVolts(int A);									// ����������� ���������� ��� ���������� � ������
void RecToHistory(uint8_t eventCode);   // ���������� ������� � ����� eventCode � ������� �������� � �������
void SaveHistoryToEEPROM(void);         // ��������� ���� ������ ������� � EEPROM
uint8_t ConnectToServer(void);					// ������������ � ������� ��� �������� ����������, ��������� ��������. ��������� 1 ���� ��� ��. ����� 0
void SoftReset(void);										// ��������� �� ���������� ������� ��� ���������� �������� ��������� �� 60 ���
void measureBattery(void);							// �������� ���������� ������������, ���������� ��� � state.vBat
void CheckIncomingMessages(void);				// ��������� ������� �������� ������������� ���������. ������������ ��.
void CheckNotifications(void);					// ��������� ������� ���������� ����������
void ApplyDirectControl(void);					// ��������� �������� ������� ���������� ����������


uint8_t waitAnswer(char *answer, uint16_t timeout); // ������� ������ �� sim900, ���������� � �������. ���� �����, ���������� 1. �� �������� (� ���/10) ���������� 0
void waitDropOK(void);
void dropMessage(void);
int16_t str2int(char* str);
void waitMessage(void);
int8_t timeCompare(struct TTime *timeOne, struct TTime *timeTwo);
void strcpyPM(char *dest, const char *PMsrc);		// �������� �� PROGMEM ������ � dest;
void ShowHistory(void);


void SIM900_GetBalance(void);
void SIM900_PowerOn(void);
void SIM900_PowerOff(void);
void SIM900_EnableGSM(void);
void SIM900_GetTime(void);
void SIM900_SetTimeFromServer(void);
void SIM900_EnableGPRS(void);
void SIM900_CheckHTTP(void);
void SIM900_PrepareConnection(void);                // �������������� ������ SIM900 � �������������
void SIM900_CheckConnection(void);								  // ��������� ��������� �����. ��������� � SIM900Status
void SIM900_RepairConnection(void);                 // ��������������� ������������ �����
void SIM900_GetRemoteSettingsTimestamp(void);      // �������� ����� ���������� ��������� �������� �� �������
void SIM900_SendSettings(void);                    // �������� ��������� �� ������ ��� ����������
void SIM900_GetSettings(void);                     // ����� ��������� � ������� � ��������� ��
void SIM900_SendStatus(void);                      // ������� �� ������ ������� ���������
void SIM900_SendHistory(void);                     // ������� �� ������ ������� �������
void SIM900_SendSMS(char *number, char *text);	   // �������� ��� � ������� text �� ����� number
void SIM900_Call(char *number);										 // ������ �� ����� number. ��� ������ ����������
uint16_t SIM900_OpenHttpGetSession(char *params);  // ��������� HTTP GET ������ � ����������� query, ��������� ��� ���������
void SIM900_CloseHttpGetSession(void);             // ��������� HTTP GET ������



uint8_t								LightLeft, 					// ������� �������� �������� ��������� � ���.
											CheckUPause, 				// �������� �������� ��������� ���������� ��� ������ ������ � ���/10
											BtStat,							// ��������� ������
											Seconds;						// ������� ������ �� ������� OneMoreSec()
uint16_t							Volts;
volatile int32_t			SilentLeft;		      // ������� ������ �������� �� ������� ����� � ��������. ���� -1, ����� ������ �� ������
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
struct HistoryEvent History[HISTORYLENGTH];
volatile uint16_t historyPtr;

char strI[23];		// ����� ��� ������������� � �����������
char strD[23];		// ����� ��� ������������� � ������ �� �������
char strS[100];		// ����� ��� ������������� � �������� SIM900
char query[100];  // ��������� ����� ��� ������������ Http ��������.
char buf[23];			// ��� ���� ����� ��� �������
//char DebugStr[35];

static const char PM_link[]						PROGMEM	= "vodyanoy.000webhostapp.com/r.php";
static const char MSG_BatFail[]				PROGMEM = "Battery_FAIL  ";
static const char MSG_SIM900Start[]		PROGMEM = "���������SIM90";
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
static const char MSG_Warming[]				PROGMEM = "  Peregrev!   ";
static const char MSG_PowerLost[]			PROGMEM = "Elektropitanie otklu4eno!";
static const char MSG_PowerRestore[]	PROGMEM = "Elektropitanie vostanovleno";
static const char MSG_BalanceLow[]		PROGMEM = "Nizkiy balans";
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