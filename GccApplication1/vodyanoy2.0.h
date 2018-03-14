
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



#define MD_PUMPRELAXTIME	0x00	//  ��������� ������������ ������� ������
#define MD_PUMPWORKTIME		0x01	//  ��������� ������������ ������ ������
#define MD_DIRPUMP			  0x02	//  ������ ���������� �������
#define MD_STAT				    0x03	//  ����������
#define MD_DIRHEATER		  0x04	//  ������ ���������� �������������
#define MD_MAX_T			    0x05	//  ��������� ������������ �����������
#define MD_MIN_T			    0x06	//  ��������� ����������� �����������
#define MD_CLEAR			    0x07	//  ����� �������� ��� ����������
//................
#define MD_LAST		0x08	//��������� ���������

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
#define EVENT_NEW_LOCAL_SETTINGS    5   // ������� ������ ����� ��������� ��������
#define EVENT_NEW_REMOTE_SETTINGS   6   // �������� (����� ��������) ������ ����� ��������� �������� 
#define EVENT_PUMP_START_AUTO       7   // ����� ������ �� ����������
#define EVENT_PUMP_START_MANUAL     8   // ����� ������ �������
#define EVENT_PUMP_START_REMOTE     9   // ����� ������ ��������
#define EVENT_PUMP_STOP_AUTO        10  // ���������� ������ �� ����������
#define EVENT_PUMP_STOP_MANUAL      11  // ���������� ������ �������
#define EVENT_PUMP_STOP_REMOTE      12  // ���������� ������ ��������
#define EVENT_PUMP_STOP_EMERGENCY   13  // ��������� ���������� ������
#define EVENT_PUMP_START_DENIED     14  // ������ ������ �� ��������
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
#define EVENT_OTHER									31  // ��� ��������� �������

#define LIGHT_TIME						10		// ����� ������ ���������. � ���
#define PUMP_RESTART_PAUSE		30	  // ������������ ����� ����� ��������� ���������� ������. � ���
#define DELTA_TIME						15		// ����� ������� � ������� ����������. � �������
#define FIRST_CONNECT_DELAY		11		// ����� � �������� �� ��������� ���������� �� ������ ������� ����� � ��������

#define LIGHT_ON	PORTB &= 0b11110111
#define LIGHT_OFF PORTB |= 0b00001000

#define RXBUFMAXSIZE 128
#define RXBUFSTRCOUNT 6

FIFO( 128 ) uart_tx_fifo;

struct TTime {
  uint8_t yy, MM, dd, hh, mm, ss;
};

struct TState {
  int16_t balance;    // ���������� ������� �� �������� � ������. ������� �������������
  int16_t Temp;				// ������� ����������� � ��������� ���������� �� 16
  uint16_t Vbat;      // ���������� �� ������ ���������� �� 200
}State;

struct TRXB {
  volatile char    buf[RXBUFMAXSIZE+1];     // ����� ��� ����� �������� USART ��������� �� ������ ������ ������� ����� ���� 0
  volatile int16_t ptrs[RXBUFSTRCOUNT];     // ������ �������� �� ������ �������� ���������. �������� -1 ��������, ��� �������� ������
  volatile int16_t wptr;                    // �������� ������ � buf
  volatile int16_t startptr;                // �������� ������ �������� ������������� ���������
  volatile uint16_t buf_overflow_count;     // ������� ������������ ������ ������
  volatile uint16_t ptrs_overflow_count;    // ������� ������������ ptrs
}rx;

ISR(USART__RXC_vect);  //����� ���������� ���������� � c:\Program Files\Atmel\AVR Studio 5.1\extensions\Atmel\AVRGCC\3.3.1.27\AVRToolchain\avr\include\avr\iom32a.h (��� mega32a) ������!!!
ISR(USART__UDRE_vect);
ISR(INT1_vect);         //CLK �� �����
ISR(TIMER1__COMPA_vect); //��������� ���������� ��������1. ������� 10��.  ����� ���������� ���������� � c:\Program Files\Atmel\AVR Studio 5.1\extensions\Atmel\AVRGCC\3.3.1.27\AVRToolchain\avr\include\avr\iom8a.h (��� mega8a)
ISR(ADC_vect);          // ���������� �������������� ���

void ShowStat(void);
void MinToStr(uint16_t Min, char *str);		// ��������� ���������� ����� � ������ hh:mm
void DrawMenu(void);
void Save(void);								// ���������� � ������ ������� ���������
void PumpStop(void);
uint8_t PumpStart(void);							// ���������� 1 ���� �������. ����� 0;
void HeaterStart(void);
void HeaterStop(void);
void OneMoreMin(void);							// ��� ���� ������� ������.
int AdcToVolts(int A);							// ����������� ���������� ��� ���������� � ������
void RecToHistory(uint8_t eventCode);
uint8_t ConnectToServer(void);      // ������������ � ������� ��� �������� ����������, ��������� ��������. ��������� 1 ���� ��� ��. ����� 0

void incomingMessage(char* s);
void renewLCD(void);
uint8_t waitAnswer(char *answer, uint16_t timeout);
void dropMessage(void);
void smsToText(char *sms, char *text);
void readSMS(void);
int16_t str2int(char* str);
void waitMessage(void);
int8_t timeCompare(struct TTime *timeOne, struct TTime *timeTwo);

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
void SIM900_GetRemoteSettingsTimestamp(void);      // �������� ����� ���������� ��������� �������� �� �������
void SIM900_SendSettings(void);                    // �������� ��������� �� ������ ��� ����������
void SIM900_GetSettings(void);                     // ����� ��������� � ������� � ��������� ��
void SIM900_SendStatus(void);                      // ������� �� ������ ������� ���������
void SIM900_SendHistory(void);                     // ������� �� ������ ������� �������





char buf[23], str[100];
uint8_t					LightLeft, 		  // ������� �������� �������� ��������� � ���.
                CheckUPause, 	  // �������� �������� ��������� ���������� ��� ������ ������ � ���/10
                FrostFlag,		  // ���� ����, ��� ����������� ������ �� -3*� �, ��������, ����� ������.
                BtStat,				  // ��������� ������
                Seconds,			  // ������� ������ �� ������� OneMoreSec()
                PumpWorkFlag;   // ����, ��� � ������ ������ ����� ������ ��������

uint16_t				PumpWorkDuration,  	// ������� ������� ������ �������� �����
                PumpRelaxDuration, 	// ������� ������� ������ �������� �����
                PumpPause,			    // �������� ����� ��������� ���������� ������ � ���
                Volts;

volatile uint32_t	SilentLeft;		      // ������� ������ �������� �� ������� ����� � ��������

int16_t					HeaterOnTemp,				// ����������� ���������� ������������
                PumpTimeLeft,		    // ������� �������� ������/�������� ������ (� �������?)
                HeaterOffTemp;			// ����������� ��������� ������������

int8_t					MenuMode,			  // ����� �������� ������ ����
                SIM900Status;   // C�������� �����

struct TTime Now, localSettingsTimestamp, remoteSettingsTimestamp;

char istr[23];		// ����� ��� ������������� � �����������
char query[100];

#endif  // VODYANOY20_H