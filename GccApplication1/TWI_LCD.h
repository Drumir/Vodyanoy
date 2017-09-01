/*! \file TWI_LCD.h \*/
//*****************************************************************************
// Содержит заголовки функций используемых для управления I2C LCD дисплеем
//  TIC32 на контролере PCF8531
//*****************************************************************************

#ifndef TWI_LCD_H
#define TWI_LCD_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>

#include "global.h"


// include project-specific configuration

#define LOCAL_ADDR 0xA0   //адрес мастера ATMEGA8
#define TARGET_ADDR 0x78  //7-битный адрес + бит R/W slave-а TWI_LCD. (0x3C сдвинутый на 1 разряд влево)

#define I2C_SEND_DATA_BUFFER_SIZE		0x20


// TWSR values (not bits)
// (taken from avr-libc twi.h - thank you Marek Michalkiewicz)
// Master
#define TW_START					0x08
#define TW_REP_START				0x10
// Master Transmitter
#define TW_MT_SLA_ACK				0x18
#define TW_MT_SLA_NACK				0x20
#define TW_MT_DATA_ACK				0x28
#define TW_MT_DATA_NACK				0x30
#define TW_MT_ARB_LOST				0x38
// Master Receiver
#define TW_MR_ARB_LOST				0x38
#define TW_MR_SLA_ACK				0x40
#define TW_MR_SLA_NACK				0x48
#define TW_MR_DATA_ACK				0x50
#define TW_MR_DATA_NACK				0x58
// Slave Transmitter
#define TW_ST_SLA_ACK				0xA8
#define TW_ST_ARB_LOST_SLA_ACK		0xB0
#define TW_ST_DATA_ACK				0xB8
#define TW_ST_DATA_NACK				0xC0
#define TW_ST_LAST_DATA				0xC8
// Slave Receiver
#define TW_SR_SLA_ACK				0x60
#define TW_SR_ARB_LOST_SLA_ACK		0x68
#define TW_SR_GCALL_ACK				0x70
#define TW_SR_ARB_LOST_GCALL_ACK	0x78
#define TW_SR_DATA_ACK				0x80
#define TW_SR_DATA_NACK				0x88
#define TW_SR_GCALL_DATA_ACK		0x90
#define TW_SR_GCALL_DATA_NACK		0x98
#define TW_SR_STOP					0xA0
// Misc
#define TW_NO_INFO					0xF8
#define TW_BUS_ERROR				0x00

// defines and constants
#define TWCR_CMD_MASK		0x0F
#define TWSR_STATUS_MASK	0xF8

// return values
#define I2C_OK				0x00
#define I2C_ERROR_NODEV		0x01

#define LCD_CMD		0b10000000  // означает, что следующий байт для TWI_LCD будет командой
#define LCD_DATA	0b01000000	// означает что следующий байт будет данными


// types
typedef enum
{
	I2C_IDLE = 0, I2C_BUSY = 1,
	I2C_MASTER_TX = 2, I2C_MASTER_RX = 3,
	I2C_SLAVE_TX = 4, I2C_SLAVE_RX = 5
} eI2cStateType;

// I2C state and address variables
static volatile eI2cStateType I2cState;
static unsigned char I2cDeviceAddrRW;
// send/transmit buffer (outgoing data)
static unsigned char I2cSendData[I2C_SEND_DATA_BUFFER_SIZE];
static unsigned char I2cSendDataIndex;
static unsigned char I2cSendDataLength;


// functions

void LCDInit(uint8_t vlcd);
void LCDClear(void);
void LCDGotoXY(unsigned char x, unsigned char y);
void LCDWriteChar (unsigned char ch);
void LCDWriteString (char *string);
void LCDPuts (char *string );


//! Initialize I2C (TWI) interface
void i2cInit(void);

//! Set the I2C transaction bitrate (in KHz)
void i2cSetBitrate(unsigned int bitrateKHz);

// I2C setup and configurations commands
//! Set the local (AVR processor's) I2C device address
void i2cSetLocalDeviceAddr(unsigned char deviceAddr, unsigned char genCallEn);

// Low-level I2C transaction commands 
//! Send an I2C start condition in Master mode
void i2cSendStart(void);
//! Send an I2C stop condition in Master mode
void i2cSendStop(void);
//! Wait for current I2C operation to complete
void i2cWaitForComplete(void);
//! Send an (address|R/W) combination or a data byte over I2C
void i2cSendByte(unsigned char data);
//! Receive a data byte over I2C  
// ackFlag = TRUE if recevied data should be ACK'ed
// ackFlag = FALSE if recevied data should be NACK'ed
unsigned char i2cGetStatus(void);

// high-level I2C transaction commands

//! send I2C data to a device on the bus
void i2cMasterSend(unsigned char deviceAddr, unsigned char length, unsigned char *data);

//! Get the current high-level state of the I2C interface
eI2cStateType i2cGetState(void);

#endif
