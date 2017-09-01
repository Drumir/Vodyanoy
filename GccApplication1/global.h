/*! \file global.h \brief AVRlib project global include. */
//*****************************************************************************
//
// File Name	: 'global.h'
// Title		: AVRlib project global include 
// Author		: Pascal Stang - Copyright (C) 2001-2002
// Created		: 7/12/2001
// Revised		: 9/30/2002
// Version		: 1.1
// Target MCU	: Atmel AVR series
// Editor Tabs	: 4
//
//	Description : This include file is designed to contain items useful to all
//					code files and projects.
//
// This code is distributed under the GNU Public License
//		which can be found at http://www.gnu.org/licenses/gpl.txt
//
//*****************************************************************************

#ifndef GLOBAL_H
#define GLOBAL_H

#include <avr/io.h>
#include <avr/pgmspace.h>
// global AVRLIB defines
#include "avrlibdefs.h"

// project/system dependent defines

// CPU clock speed
//#define F_CPU        16000000               		// 16MHz processor
//#define F_CPU        14745000               		// 14.745MHz processor
#define F_CPU        8000000               		// 8MHz processor
//#define F_CPU        7372800               		// 7.37MHz processor
//#define F_CPU        4000000               		// 4MHz processor
//#define F_CPU        3686400               		// 3.69MHz processor
#define CYCLES_PER_US ((F_CPU+500000)/1000000) 	// cpu cycles per microsecond

#define CHAR_CR       0x0D
#define CHAR_LF       0x0A


static const unsigned char smallFont[] PROGMEM =
{
		0x00,0x00,0x00,0x00,0x00,0x00,      // 0x20  ������                         
		0x00,0x00,0x4F,0x00,0x00,0x00,      // 0x21  ! 
		0x00,0x07,0x00,0x07,0x00,0x00,      // 0x22  " 
		0x14,0x7F,0x14,0x7F,0x14,0x00,      // 0x23  # 
		0x24,0x2A,0x7F,0x2A,0x12,0x00,      // 0x24  $ 
		0x63,0x13,0x08,0x64,0x63,0x00,      // 0x25  %
		0x36,0x49,0x55,0x22,0x50,0x00,      // 0x26  &
		0x00,0x05,0x03,0x00,0x00,0x00,      // 0x27  '                                   
		0x1C,0x22,0x41,0x00,0x00,0x00,      // 0x28  (                                   
		0x00,0x00,0x41,0x22,0x1C,0x00,      // 0x29  )                                   
		0x14,0x08,0x3E,0x08,0x14,0x00,      // 0x2A  *                                  
		0x08,0x08,0x3E,0x08,0x08,0x00,      // 0x2B  +                                  
		0x00,0x00,0x50,0x30,0x00,0x00,      // 0x2C  ,           
		0x08,0x08,0x08,0x08,0x08,0x00,      // 0x2D  - 
		0x00,0x00,0x60,0x60,0x00,0x00,      // 0x2E  . 
		0x20,0x10,0x08,0x04,0x02,0x00,      // 0x2F  /
		0x3E,0x51,0x49,0x45,0x3E,0x00,      // 0x30  0 
		0x00,0x42,0x7F,0x40,0x00,0x00,      // 0x31  1 
		0x42,0x61,0x51,0x49,0x46,0x00,      // 0x32  2
		0x21,0x41,0x45,0x4B,0x31,0x00,      // 0x33  3
		0x18,0x14,0x12,0x7F,0x10,0x00,      // 0x34  4
		0x27,0x45,0x45,0x45,0x39,0x00,      // 0x35  5
		0x3C,0x4A,0x49,0x49,0x30,0x00,      // 0x36  6
		0x01,0x71,0x09,0x05,0x03,0x00,      // 0x37  7
		0x36,0x49,0x49,0x49,0x36,0x00,      // 0x38  8
		0x06,0x49,0x49,0x29,0x1E,0x00,      // 0x39  9
		0x00,0x00,0x36,0x36,0x00,0x00,      // 0x3A  :
		0x00,0x00,0x56,0x36,0x00,0x00,      // 0x3B  ;
		0x08,0x14,0x22,0x41,0x00,0x00,      // 0x3C  <
		0x14,0x14,0x14,0x14,0x14,0x00,      // 0x3D  =
		0x00,0x41,0x22,0x14,0x08,0x00,      // 0x3E  >
		0x02,0x01,0x51,0x09,0x06,0x00,      // 0x3F  ?
		0x32,0x49,0x39,0x42,0x3C,0x00,      // 0x40  @
		0x7E,0x11,0x11,0x11,0x7E,0x00,      // 0x41  � 
		0x7F,0x49,0x49,0x49,0x36,0x00,      // 0x42  �
		0x3E,0x41,0x41,0x41,0x22,0x00,      // 0x43  �
		0x7F,0x41,0x41,0x22,0x1C,0x00,      // 0x44  D
		0x7F,0x49,0x49,0x49,0x41,0x00,      // 0x45  �
		0x7F,0x09,0x09,0x09,0x01,0x00,      // 0x46  F
		0x3E,0x41,0x49,0x49,0x7A,0x00,      // 0x47  G
		0x7F,0x08,0x08,0x08,0x7F,0x00,      // 0x48  �                                 
		0x00,0x41,0x7F,0x41,0x00,0x00,      // 0x49  I
		0x20,0x40,0x41,0x3F,0x01,0x00,      // 0x4A  J
		0x7F,0x08,0x14,0x22,0x41,0x00,      // 0x4B  �
		0x7F,0x40,0x40,0x40,0x40,0x00,      // 0x4C  L
		0x7F,0x02,0x04,0x02,0x7F,0x00,      // 0x4D  �
		0x7F,0x04,0x08,0x10,0x7F,0x00,      // 0x4E  N
		0x3E,0x41,0x41,0x41,0x3E,0x00,      // 0x4F  �                               
		0x7F,0x09,0x09,0x09,0x06,0x00,      // 0x50  �
		0x3E,0x41,0x51,0x21,0x5E,0x00,      // 0x51  Q
		0x7F,0x09,0x19,0x29,0x46,0x00,      // 0x52  R
		0x46,0x49,0x49,0x49,0x31,0x00,      // 0x53  S
		0x01,0x01,0x7F,0x01,0x01,0x00,      // 0x54  �          
		0x3F,0x40,0x40,0x40,0x3F,0x00,      // 0x55  U
		0x1F,0x20,0x40,0x20,0x1F,0x00,      // 0x56  V
		0x3F,0x40,0x38,0x40,0x3F,0x00,      // 0x57  W
		0x63,0x14,0x08,0x14,0x63,0x00,      // 0x58  �          
		0x07,0x08,0x78,0x08,0x07,0x00,      // 0x59  Y
		0x61,0x51,0x49,0x45,0x43,0x00,      // 0x5A  Z
		0x00,0x7F,0x41,0x41,0x00,0x00,      // 0x5B  [
		0x18,0x24,0x7E,0x24,0x18,0x00,      // 0x5C  �
		0x00,0x41,0x41,0x7F,0x00,0x00,      // 0x5D  ]
		0x04,0x02,0x01,0x02,0x04,0x00,      // 0x5E  ^
		0x40,0x40,0x40,0x40,0x40,0x00,      // 0x5F  _
		0x00,0x00,0x01,0x02,0x00,0x00,      // 0x60  '
		0x20,0x54,0x54,0x54,0x78,0x00,      // 0x61  �          
		0x7F,0x48,0x44,0x44,0x38,0x00,      // 0x62  b
		0x38,0x44,0x44,0x44,0x20,0x00,      // 0x63  �
		0x38,0x44,0x44,0x48,0x7F,0x00,      // 0x64  d
		0x38,0x54,0x54,0x54,0x18,0x00,      // 0x65  �
		0x08,0x7E,0x09,0x01,0x02,0x00,      // 0x66  f
		0x08,0x54,0x54,0x54,0x3C,0x00,      // 0x67  g
		0x7F,0x08,0x04,0x04,0x78,0x00,      // 0x68  h
		0x00,0x44,0x7D,0x40,0x00,0x00,      // 0x69  i
		0x20,0x40,0x44,0x3D,0x00,0x00,      // 0x6A  j
		0x00,0x7F,0x10,0x28,0x44,0x00,      // 0x6B  k
		0x00,0x41,0x7F,0x40,0x00,0x00,      // 0x6C  l
		0x7C,0x04,0x18,0x04,0x78,0x00,      // 0x6D  m
		0x7C,0x08,0x04,0x04,0x78,0x00,      // 0x6E  n
		0x38,0x44,0x44,0x44,0x38,0x00,      // 0x6F  o
		0x7C,0x14,0x14,0x14,0x08,0x00,      // 0x70  �
		0x08,0x14,0x14,0x14,0x7C,0x00,      // 0x71  q
		0x7C,0x08,0x04,0x04,0x08,0x00,      // 0x72  r
		0x48,0x54,0x54,0x54,0x24,0x00,      // 0x73  s
		0x04,0x3F,0x44,0x40,0x20,0x00,      // 0x74  t
		0x3C,0x40,0x40,0x20,0x7C,0x00,      // 0x75  u
		0x1C,0x20,0x40,0x20,0x1C,0x00,      // 0x76  v
		0x3C,0x40,0x30,0x40,0x3C,0x00,      // 0x77  w
		0x44,0x28,0x10,0x28,0x44,0x00,      // 0x78  �
		0x0C,0x50,0x50,0x50,0x3C,0x00,      // 0x79  �
		0x44,0x64,0x54,0x4C,0x44,0x00,      // 0x7A  z
		0x00,0x08,0x36,0x41,0x00,0x00,      // 0x7B  {
		0x00,0x00,0x7F,0x00,0x00,0x00,      // 0x7C  |
		0x00,0x41,0x36,0x08,0x00,0x00,      // 0x7D  }
		0x10,0x08,0x08,0x10,0x10,0x00,      // 0x7E  ~ 
		 
//============================================================================================
//	� ����� ����� ����� ����������� ����� �������. ����� ������� �� ������� ������. � ��� �� ���������� ������ ���� ���������
                                  
		0xFF,0x01,0x01,0x01,0x01,0x01,      // 0x7F ������� �������������
		0x01,0x01,0x01,0x01,0x01,0x01,      // 0x80
		0x01,0x01,0xFF,0x01,0x01,0x01,      // 0x81                          
		0x01,0x01,0x01,0x01,0x01,0xFF,      // 0x82
		0xFF,0x00,0x00,0x00,0x00,0x00,      // 0x83                          
		0x00,0x00,0xFF,0x00,0x00,0x00,      // 0x84                          
		0x00,0x00,0x00,0x00,0x00,0xFF,      // 0x85
		0xFF,0x08,0x08,0x08,0x08,0x08,      // 0x86
		0x08,0x08,0x08,0x08,0x08,0x08,      // 0x87
		0x80,0x80,0xFF,0x80,0x80,0x80,      // 0x88  
		0x80,0x80,0x80,0x80,0x80,0xFF,      // 0x89
		0xFF,0x80,0x80,0x80,0x80,0x80,      // 0x8A
		0x80,0x80,0x80,0x80,0x80,0x80,      // 0x8B
		0x08,0x08,0xFF,0x08,0x08,0x08,      // 0x8C
		0x08,0x08,0x08,0x08,0x08,0xFF,      // 0x8D
		0xFF,0x01,0xFD,0x05,0x05,0x05,      // 0x8E
		0x05,0x05,0x05,0x05,0x05,0x05,      // 0x8F
		0x05,0xFD,0x01,0xFD,0x05,0x05,      // 0x90
		0x05,0x05,0x05,0xFD,0x01,0xFF,      // 0x91
		0xFF,0x00,0xFF,0x00,0x00,0x00,      // 0x92
		0x00,0xFF,0x00,0xFF,0x00,0x00,      // 0x93      
		0x00,0x00,0x00,0xFF,0x00,0xFF,      // 0x94
		0xFF,0x00,0xF7,0x14,0x14,0x14,      // 0x95
		0x14,0x14,0x14,0x14,0x14,0x14,      // 0x96
		0x14,0xF7,0x00,0xF7,0x14,0x14,      // 0x97
		0x14,0x14,0x14,0xF7,0x00,0xFF,      // 0x98
		0xFF,0x80,0xBF,0xA0,0xA0,0xA0,      // 0x99
		0xA0,0xA0,0xA0,0xA0,0xA0,0xA0,      // 0x9A                                     
		0xA0,0xBF,0x80,0xBF,0xA0,0xA0,      // 0x9B              
		0xA0,0xA0,0xA0,0xBF,0x80,0xFF,      // 0x9C
		0x00,0x00,0x7C,0x44,0x7C,0x00,      // 0x9D 0 ��������� ����� 
		0x00,0x00,0x00,0x00,0x7C,0x00,      // 0x9E 1
		0x00,0x00,0x74,0x54,0x5C,0x00,      // 0x9F 2
		0x00,0x00,0x54,0x54,0x7C,0x00,      // 0xA0 3
		0x00,0x00,0x1C,0x10,0x7C,0x00,      // 0xA1 4
		0x00,0x00,0x5C,0x54,0x74,0x00,      // 0xA2 5
		0x00,0x00,0x7C,0x54,0x74,0x00,      // 0xA3 6
		0x00,0x00,0x04,0x04,0x7C,0x00,      // 0xA4 7
		0x00,0x00,0x7C,0x54,0x7C,0x00,      // 0xA5 8
		0x41,0x5D,0x55,0x49,0x41,0x5D,      // 0xA6 dli
		0x51,0x51,0x41,0x5D,0x41,0x00,      // 0xA7 
		0x11,0x2A,0x44,0x11,0x2A,0x44,      // 0xA8 ������
		0x55,0x55,0x55,0x55,0x55,0x00,      // 0xA9 �������������
		0x08,0x04,0x08,0x08,0x04,0x00,      // 0xAA ~; spe *
		0x04,0x02,0x7F,0x02,0x04,0x00,      // 0xAB su * ������� ���� 
		0x10,0x20,0x7F,0x20,0x10,0x00,      // 0xAC sd * // ����
		0x08,0x08,0x2A,0x1C,0x08,0x00,      // 0xAD sr * ������� � ����� ->
		0x08,0x1C,0x2A,0x08,0x08,0x00,      // 0xAE sl * <- ������� � �����
		0x00,0xE0,0x10,0xC8,0x24,0x14,      // 0xAF ������� ����� ����
		0x14,0x14,0x14,0x14,0x14,0x14,      // 0xB0 �������������� ����� ������
		0x14,0x24,0xC8,0x10,0xE0,0x00,      // 0xB1 ������ ������ ����
		0x00,0xFF,0x00,0xFF,0x00,0x00,      // 0xB2 ������������� ����� �����
		0x00,0x00,0xFF,0x00,0xFF,0x00,      // 0xB3 ������������ ������ �����
		0x00,0x07,0x08,0x13,0x24,0x28,      // 0xB4 ������ ����� ����
		0x28,0x24,0x13,0x08,0x07,0x00,      // 0xB5 ������ ������ ����
		0x28,0x28,0x28,0x28,0x28,0x28,      // 0xB6 �������������� ����� ������
		0x08,0x1C,0x3E,0x7F,0x00,0x00,      // 0xB7 ����������� �����
		0x00,0x7F,0x3E,0x1C,0x08,0x00,      // 0xB8 ����������� ������
		0x10,0x1E,0x3F,0x1E,0x10,0x00,      // 0xB9 kol * �����������
		0xC6,0xA4,0x18,0x18,0x25,0x63,      // 0xBA ����������
		0x7E,0x42,0x42,0x42,0x7E,0x00,      // 0xBB ������� ������
		0x7E,0x7E,0x7E,0x7E,0x7E,0x00,      // 0xBC ������� ������
		0x0C,0x30,0xC0,0x30,0x0C,0x03,      // 0xBD ������
		0x01,0xFF,0x80,0x80,0xFF,0x01,      // 0xBE �������
		0x06,0x09,0x09,0x06,0x00,0x00,      // 0xBF ������ *
//============================================================================================
//� ����� ����� ���������� ��������� � ���� ������ ������
 
		0x7E,0x11,0x11,0x11,0x7E,0x00,      // 0xC0  � 
		0x7F,0x49,0x49,0x49,0x30,0x00,      // �
		0x7F,0x49,0x49,0x49,0x36,0x00,      // �
		0x7F,0x01,0x01,0x01,0x03,0x00,      // �
		0xE0,0x51,0x4F,0x41,0xFF,0x00,      // �
		0x7F,0x49,0x49,0x49,0x41,0x00,      // �
		0x77,0x08,0x7F,0x08,0x77,0x00,      // �
		0x49,0x49,0x49,0x49,0x36,0x00,      // �
		0x7F,0x10,0x08,0x04,0x7F,0x00,      // �
		0x7C,0x21,0x12,0x09,0x7C,0x00,      // �
		0x7F,0x08,0x14,0x22,0x41,0x00,      // �
		0x20,0x41,0x3F,0x01,0x7F,0x00,      // �
		0x7F,0x02,0x04,0x02,0x7F,0x00,      // �
		0x7F,0x08,0x08,0x08,0x7F,0x00,      // �
		0x3E,0x41,0x41,0x41,0x3E,0x00,      // �
		0x7F,0x01,0x01,0x01,0x7F,0x00,      // �
		0x7F,0x09,0x09,0x09,0x06,0x00,      // 0xD0  �                                
		0x3E,0x41,0x41,0x41,0x22,0x00,      // �                               
		0x01,0x01,0x7F,0x01,0x01,0x00,      // �                                
		0x47,0x28,0x10,0x08,0x07,0x00,      // �                               
		0x18,0x24,0x7F,0x24,0x18,0x00,      // �       
		0x63,0x14,0x08,0x14,0x63,0x00,      // �                                
		0x7F,0x40,0x40,0x40,0xFF,0x00,      // �
		0x07,0x08,0x08,0x08,0x7F,0x00,      // �                               
		0x7F,0x40,0x7F,0x40,0x7F,0x00,      // �   
		0x7F,0x40,0x7F,0x40,0xFF,0x00,      // �                              
		0x01,0x7F,0x48,0x48,0x30,0x00,      // �
		0x7E,0x48,0x30,0x00,0x7E,0x00,      // �
		0x7E,0x48,0x48,0x48,0x30,0x00,      // �                               
		0x22,0x41,0x49,0x49,0x3E,0x00,      // �                               
		0x7F,0x08,0x3E,0x41,0x3E,0x00,      // �                              
		0x46,0x29,0x19,0x09,0x7F,0x00,      // �                               
		0x20,0x54,0x54,0x54,0x78,0x00,      // 0xE0  �        
		0x3C,0x4A,0x4A,0x49,0x31,0x00,      // �
		0x7C,0x54,0x54,0x28,0x00,0x00,      // �
		0x7C,0x04,0x04,0x04,0x0C,0x00,      // �
		0xE0,0x54,0x4C,0x44,0xFC,0x00,      // �
		0x38,0x54,0x54,0x54,0x18,0x00,      // �
		0x6C,0x10,0x7C,0x10,0x6C,0x00,      // �
		0x44,0x44,0x54,0x54,0x28,0x00,      // �
		0x7C,0x20,0x10,0x08,0x7C,0x00,      // �
		0x78,0x42,0x24,0x12,0x78,0x00,      // �
		0x7C,0x10,0x28,0x44,0x00,0x00,      // �
		0x20,0x44,0x3C,0x04,0x7C,0x00,      // �
		0x7C,0x08,0x10,0x08,0x7C,0x00,      // �
		0x7C,0x10,0x10,0x10,0x7C,0x00,      // �
		0x38,0x44,0x44,0x44,0x38,0x00,      // o
		0x7C,0x04,0x04,0x04,0x7C,0x00,      // �
		0x7C,0x14,0x14,0x14,0x08,0x00,      // 0xF0  �
		0x38,0x44,0x44,0x44,0x20,0x00,      // �
		0x04,0x04,0x7C,0x04,0x04,0x00,      // �
		0x0C,0x50,0x50,0x50,0x3C,0x00,      // �
		0x1C,0x22,0x7F,0x22,0x1C,0x00,      // �
		0x44,0x28,0x10,0x28,0x44,0x00,      // �
		0x7C,0x40,0x40,0x40,0xFC,0x00,      // �
		0x0C,0x10,0x10,0x10,0x7C,0x00,      // �
		0x7C,0x40,0x7C,0x40,0x7C,0x00,      // �
		0x7C,0x40,0x7C,0x40,0xFC,0x00,      // �
		0x04,0x7C,0x50,0x50,0x20,0x00,      // �
		0x7C,0x50,0x20,0x00,0x7C,0x00,      // �
		0x7C,0x50,0x50,0x20,0x00,0x00,      // �
		0x28,0x44,0x54,0x54,0x38,0x00,      // �
		0x7C,0x10,0x38,0x44,0x38,0x00,      // �
		0x08,0x54,0x34,0x14,0x7C,0x00,      // 0xFE  �
	};


#endif