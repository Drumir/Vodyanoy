

#include "TWI_LCD.h"
//#include "3310_routines.h"

// Standard I2C bit rates are:
// 100KHz for slow speed
// 400KHz for high speed



// functions

void LCDInit(uint8_t vlcd)
{
	
i2cInit();				// Инициализируем TWI
i2cSetBitrate(400);		// Поменяем битрейд по умолчания - 100Кбит/с на 400

i2cSetLocalDeviceAddr(LOCAL_ADDR, TRUE); 

while(I2cState);
I2cState = I2C_MASTER_TX;
I2cDeviceAddrRW = (TARGET_ADDR & 0xFE);	// RW cleared: write operation

I2cSendData[0] = LCD_CMD;  I2cSendData[1] =  0b00000001;		// на основн страницу команд
I2cSendData[2] = LCD_CMD;  I2cSendData[3] =  0b00100000;		// Enable chip, горизонтальная
I2cSendData[4] = LCD_CMD;  I2cSendData[5] =  0b00001001;		// На страницу настройки дисплея
//buf[6] = LCD_CMD;  buf[7] =  0b00001001;		// Display Mode: NORMAL, Image Mode: NORMAL
I2cSendData[6] = LCD_CMD;  I2cSendData[7] =  0b00001100;		// Display Mode: NORMAL
I2cSendData[8] = LCD_CMD;  I2cSendData[9] =  0b00000101;		// MUX-rate: 1/34
I2cSendData[10] = LCD_CMD; I2cSendData[11] = 0b00010100;		// Bias system: 3
I2cSendData[12] = LCD_CMD; I2cSendData[13] = 0b00000001;			// на основн стр
I2cSendData[14] = LCD_CMD; I2cSendData[15] =  0b00001010;			// page select - Настройка умножителя.
I2cSendData[16] = LCD_CMD; I2cSendData[17] =  0b00000101;			// Vlcd - LOW; Voltage multiplier ON
I2cSendData[18] = LCD_CMD; I2cSendData[19] =  0b10000000 | vlcd;			// Vlcd set
I2cSendDataIndex = 0;
I2cSendDataLength = 20;
i2cSendStart();

}

void LCDWriteChar (unsigned char ch)
{
unsigned char i;
	// wait for interface to be ready
while(I2cState);
	// set state
I2cState = I2C_MASTER_TX;
	// save data
I2cDeviceAddrRW = (TARGET_ADDR & 0xFE);	// RW cleared: write operation
I2cSendData[0] = LCD_DATA;	

if(ch == CHAR_CR) ch = 0xAC;
if(ch == CHAR_LF) ch = 0xAE;

for( i = 0; i < 6; i ++)
	I2cSendData[i+1] =  pgm_read_byte(&(smallFont [(ch-32)*6 + i] ));
I2cSendDataIndex = 0;
I2cSendDataLength = 7;
i2cSendStart();				// отправляем блок команд
} 

void LCDWriteString ( char *string )
{
    while ( *string )
        LCDWriteChar ( *string++ );
}

void LCDPuts (char *string )
{
static char ScreenBuff[4][22];
static unsigned char x = 0;
unsigned char i;

if(x < 3) x ++; 
else for( i = 0; i < 3; i ++) strcpy(ScreenBuff[i], ScreenBuff[i+1]);

strncpy(ScreenBuff[x], string, 21);
ScreenBuff[x][21] = 0;

LCDClear();
for(i = 0; i <= x; i ++)
	{
	LCDGotoXY(0, i);
	LCDWriteString(ScreenBuff[i]);
	}
}

void LCDGotoXY(unsigned char x, unsigned char y)
{
while(I2cState);
I2cState = I2C_MASTER_TX;
I2cDeviceAddrRW = (TARGET_ADDR & 0xFE);	// RW cleared: write operation
I2cSendData[0] = LCD_CMD; I2cSendData[1] = 0b00000001;			// на основн стр
I2cSendData[2] = LCD_CMD; I2cSendData[3] = 0b01000000 | y;			// Установка Y = 0;
I2cSendData[4] = LCD_CMD; I2cSendData[5] = 0b10000000 | x;			// Установка Y = 0;
I2cSendDataIndex = 0;
I2cSendDataLength = 6;
i2cSendStart();				// отправляем блок команд
}

void LCDClear(void)
{
unsigned char i;

LCDGotoXY(0, 0);
while(I2cState);
I2cSendData[0] = LCD_DATA;	
for(i = 1; i < I2C_SEND_DATA_BUFFER_SIZE; i ++) I2cSendData[i] = 0x00;		// Готовим блок данных

for(i = 0; i < 18; i ++)			// отсылаем блоки данных
	{
	while(I2cState);
	I2cState = I2C_MASTER_TX;
	I2cDeviceAddrRW = (TARGET_ADDR & 0xFE);	
	I2cSendDataIndex = 0;
	I2cSendDataLength = I2C_SEND_DATA_BUFFER_SIZE;
	i2cSendStart();		
	}
}

void i2cInit(void)
{
	// set pull-up resistors on I2C bus pins
	// TODO: should #ifdef these
	sbi(PORTC, 0);	// i2c SCL on ATmega8
	sbi(PORTC, 1);	// i2c SDA on ATmega8
	
	// set i2c bit rate to 100KHz
	i2cSetBitrate(100);
	// enable TWI (two-wire interface)
	sbi(TWCR, TWEN);
	// set state
	I2cState = I2C_IDLE;
	// enable TWI interrupt and slave address ACK
	sbi(TWCR, TWIE);
	sbi(TWCR, TWEA);
	//outb(TWCR, (inb(TWCR)&TWCR_CMD_MASK)|BV(TWINT)|BV(TWEA));
	// enable interrupts
	sei();
}

void i2cSetBitrate(unsigned int bitrateKHz)
{
	unsigned char bitrate_div;
	// set i2c bitrate
	// SCL freq = F_CPU/(16+2*TWBR))
	#ifdef TWPS0
		// for processors with additional bitrate division (mega128)
		// SCL freq = F_CPU/(16+2*TWBR*4^TWPS)
		// set TWPS to zero
		cbi(TWSR, TWPS0);
		cbi(TWSR, TWPS1);
	#endif
	// calculate bitrate division	
	bitrate_div = ((F_CPU/1000l)/bitrateKHz);
	if(bitrate_div >= 16)
		bitrate_div = (bitrate_div-16)/2;
	outb(TWBR, bitrate_div);
}

void i2cSetLocalDeviceAddr(unsigned char deviceAddr, unsigned char genCallEn)
{
	// set local device address (used in slave mode only)
	outb(TWAR, ((deviceAddr&0xFE) | (genCallEn?1:0)) );
}

inline void i2cSendStart(void)
{
	// send start condition
	outb(TWCR, (inb(TWCR)&TWCR_CMD_MASK)|BV(TWINT)|BV(TWSTA));
}

inline void i2cSendStop(void)
{
	// transmit stop condition
	// leave with TWEA on for slave receiving
	outb(TWCR, (inb(TWCR)&TWCR_CMD_MASK)|BV(TWINT)|BV(TWEA)|BV(TWSTO));
}

inline void i2cWaitForComplete(void)
{
	// wait for i2c interface to complete operation
	while( !(inb(TWCR) & BV(TWINT)) );
}

inline void i2cSendByte(unsigned char data)
{
	// save data to the TWDR
	outb(TWDR, data);
	// begin send
	outb(TWCR, (inb(TWCR)&TWCR_CMD_MASK)|BV(TWINT));
}

inline unsigned char i2cGetStatus(void)
{
	// retieve current i2c status from i2c TWSR
	return( inb(TWSR) );
}

void i2cMasterSend(unsigned char deviceAddr, unsigned char length, unsigned char* data)
{
	unsigned char i;
	// wait for interface to be ready
	while(I2cState);
	// set state
	I2cState = I2C_MASTER_TX;
	// save data
	I2cDeviceAddrRW = (deviceAddr & 0xFE);	// RW cleared: write operation
	for(i=0; i<length; i++)
		I2cSendData[i] = *data++;
	I2cSendDataIndex = 0;
	I2cSendDataLength = length;
	// send start condition
	i2cSendStart();
}

ISR(TWI_vect)
{
	// read status bits
	unsigned char status = inb(TWSR) & TWSR_STATUS_MASK;

	switch(status)
	{
	// Master General
	case TW_START:						// 0x08: Sent start condition
	case TW_REP_START:					// 0x10: Sent repeated start condition
		// send device address
		i2cSendByte(I2cDeviceAddrRW);
		break;
	
	// Only Master Transmitter status codes
	case TW_MT_SLA_ACK:					// 0x18: Slave address acknowledged
	case TW_MT_DATA_ACK:				// 0x28: Data acknowledged
		if(I2cSendDataIndex < I2cSendDataLength)
		{
			// send data
			i2cSendByte( I2cSendData[I2cSendDataIndex++] );
		}
		else
		{
			// transmit stop condition, enable SLA ACK
			i2cSendStop();
			// set state
			I2cState = I2C_IDLE;
		}
		break;
	case TW_MT_SLA_NACK:				// 0x20: Slave address not acknowledged
	case TW_MT_DATA_NACK:				// 0x30: Data not acknowledged
		// transmit stop condition, enable SLA ACK
		i2cSendStop();
		// set state
		I2cState = I2C_IDLE;
		break;
	case TW_MT_ARB_LOST:				// 0x38: Bus arbitration lost
	//case TW_MR_ARB_LOST:				// 0x38: Bus arbitration lost
		// release bus
		outb(TWCR, (inb(TWCR)&TWCR_CMD_MASK)|BV(TWINT));
		// set state
		I2cState = I2C_IDLE;
		// release bus and transmit start when bus is free
		//outb(TWCR, (inb(TWCR)&TWCR_CMD_MASK)|BV(TWINT)|BV(TWSTA));
		break;
	// Misc
	case TW_NO_INFO:					// 0xF8: No relevant state information
		// do nothing
		break;
	case TW_BUS_ERROR:					// 0x00: Bus error due to illegal start or stop condition
		// reset internal hardware and release bus
		outb(TWCR, (inb(TWCR)&TWCR_CMD_MASK)|BV(TWINT)|BV(TWSTO)|BV(TWEA));
		// set state
		I2cState = I2C_IDLE;
		break;
	}
}

eI2cStateType i2cGetState(void)
{
	return I2cState;
}
