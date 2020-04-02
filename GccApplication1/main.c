/*
 * Sim900.c
 *
 * Created: 24.08.2017 22:26:28
 * Author : Drumir
 */ 

#include "vodyanoy2.0.h"

volatile uint16_t TimeoutTackts = 0;


int main(void)
{
	ACSR |= 0x80;			// Выключим не нужный, но включеный по умолчанию аналоговый компаратор
  DDRC  = 0b11110100;    // 7-н, 6-DQ DS18B20, 5-LCD_RESET, 4-насос, 3-н, 2-ТЭН, 1-н, 0-н
  PORTC = 0b01100000;
  DDRD =  0b10000010;    // 7-En4V, 6-3 клава, 2-геркон, 1-TX, 0-RX
  PORTD = 0b01111000;
  DDRB  = 0b11111010;    // 7-LCD_CLK, 6-LCD_DC, 5-LCD_MOSI, 4-LCD_SS, 3 - подсветка дисплея, 2-power_fail, 1 - SIM900_pwrkey, 0 - SIM900_status
//  DDRB  = 0b00001010;    // 7-4 - н, 3 - подсветка дисплея, 2-power_fail, 1 - SIM900_pwrkey, 0 - SIM900_status
  PORTB = 0b00000010;
	DDRA =  0b00000000;			// 7-1-н, 0 - ADC Vbat
	PORTA = 0b00000000;


  sbi(GICR, 7);          // Разрешить INT1  Для клавы
  sbi(MCUCR, 3); cbi(MCUCR, 2);  // Прерывание по переходу 1->0  на линии CLK клавы 

      /* Инициализация 16-битного таймера1 ( 10Гц) */
  TCCR1A =  0b00000000;	// CTC. Считать до OCR1A
  TCCR1B =  0b00001100;	// Делитель для таймера1 - 100 - 256
  OCR1A = 3125;			// Считать до 3125	Прерывание будет генерироваться с частотой 10 Гц
  TIMSK = 0b00010000;		// По совпадению счектчика1 и OCR1A

          	/* Инициализация АЦП */
            // АЦП En,  not now,  single mode, reset iflag, INTs Enable,       предделитель частоты
  ADCSRA = 1 << ADEN | 0 << ADSC | 0 << ADATE | 0 << ADIF | 1 << ADIE | 1 << ADPS2 | 1 << ADPS1 | 1 << ADPS0; 
//	ADCSRA = 0b10001111;		// 1 - вкл, 0 - еще не старт, 0 - однократно, 0 - прерывания генерировать, 0 - прерывания АЦП разрешить, 111 - предделитель частоты 128
	ADMUX  = 0b11000000;		// 11 - Опорное напряжение = 2,56В, 0 - выравнивание вправо, 0 - резерв, 0 - резерв, 000 - выбор канала ADC0
	ADCSRA |= 1<<ADSC;		// Старт пробного мусорного преобразования  

	for(uint16_t i = 0; i < RXBUFSTRCOUNT; i ++) rx.ptrs[i] = -1;   // Почистим массив указателей на начала принятых сообщений
	rx.wptr = 0;                                           // Установим указатель записи на начало буфера
	rx.startptr = 0;                                       // Начало записываемого сообщения
	rx.buf_overflow_count = 0;
	rx.ptrs_overflow_count = 0;
	rx.buf[RXBUFMAXSIZE] = 0;                               // Запишем сразу после буфера '\0' чтобы строковые функции не сошли с ума
  
  SIM900Status = SIM900_NOTHING;
	MenuMode = MD_STAT;
	LightLeft = LIGHT_TIME; LIGHT_ON;	// Сразу включим подсветку
	PumpPause = 0;
	Seconds = 0;
	CheckUPause = 0;
	SilentLeft = FIRST_CONNECT_DELAY;
  
  Now.yy = 0;         // По нулю в количестве лет определим, что Now еще не актуализировалось
  remoteSettingsTimestamp.yy = 0;

  eeprom_read_block(&options, (const void*)0x00, sizeof(struct TSettings));


//	if(options.fFreezeNotifications == 0xFF || options.ConnectPeriod == 0xFFFF) 	// Если прочитался мусор (после перепрошивки) сбросим значения в поумолчанию.
	if(0) 	// Если прочитался мусор (после перепрошивки) сбросим значения в поумолчанию.
	{
  	options.FrostFlag = 0;
  	options.PumpWorkFlag = 0;
    options.fFreezeNotifications = 0;           // Флаги оповещения о заморозке. 1 в 3 бите - смс оператору. Во 2 бите - смс админу. в 1 - звонок оператору. в 0 - звонок админу.
    options.fWarmNotifications = 0;             // Флаги оповещения о перегреве.
    options.fDoorNotifications = 0;             // Флаги оповещения об открытии двери.
    options.fFloodNotifications = 0;            // Флаги оповещения о затоплении.
    options.fPowerNotifications = 0;            // Флаги оповещения о перебое электроснабжения.
    options.fPowerRestNotifications = 0;        // Флаги оповещения о восстановлении электроснабжения.
    options.fOfflineNotifications = 0;          // Флаги оповещения о длительном отсутствии интернета.
    options.fBalanceNotifications = 0;          // Флаги оповещения о критическом балансе на счёте
    options.fDailyNotifications = 0;            // Флаги ежедневного оповещения о состоянии
    strcpy((char*)options.OperatorTel, "9027891301");  // Номер телефона оператора
    strcpy((char*)options.AdminTel, "9040448302");  // Номер телефона администратора
  	options.PumpWorkDuration = 120;             // Сколько времени должен работать насос (в минутах)
  	options.PumpRelaxDuration = 240;            // Сколько времени должен отдыхать насос (в минутах)
    options.ConnectPeriod = 5;                  // Период (в минутах) докладов на сервер. Если 0 - только при необходимости или по звонку
    options.DisconnectionTime = 120;            // Длительность отсутствия связи для оповещения о проблемах со связью
    options.MinBalance = 10;                    // Баланс, при достижении которого нужно оповестить о критическом балансе
    options.DailyReportTime = 960;              // Время ежедневного отчета о состоянии (в минутах с начала суток)
    options.PumpTimeLeft = 0;		                // Сколько осталось качать/отдыхать насосу (в минутах?)
    options.HeaterOnTemp = 2;				            // Температура отключения обогревателя
    options.HeaterOffTemp = 10;			            // Температура включения обогревателя
    options.FreezeTemp = 0;                    // Температура предупреждения о заморозке
    options.WarmTemp = 40;                      // Температура предупреждения о перегреве
    options.localSettingsTimestamp.yy = 0;      // Время последнего обновления локальных настроек
    options.localSettingsTimestamp.MM = 0;
    options.localSettingsTimestamp.dd = 0;
    options.localSettingsTimestamp.hh = 0;
    options.localSettingsTimestamp.mm = 0;
    options.localSettingsTimestamp.ss = 0;
	}
  
  //Save();

	if(options.PumpWorkFlag == 1 && options.PumpTimeLeft != 0)		// Если до длительного отключения насос был включен
	PumpPause = PUMP_RESTART_PAUSE/2;						// На всякий случай подождем ещ половину "паузы перед включением"
    
  uart_init();
  spi_init();
 	_delay_ms(100);      //на момент инициализации SLI_LCD, дисплей должен быть уже включен и готов
  LCD_init();
  sensor_write(0x44);   // старт измерения температуры
  _delay_ms(1000);
  State.Temp = sensor_write(0xBE); // чтение температурных данных c dc18_B_20 / dc18_S_20

  sei();
  
//  SIM900_SendReport();
  while (1) 
  {
    if(SilentLeft == 0)    // Счетчик секунд до сеанса связи
    {
      SilentLeft = options.ConnectPeriod*60;    // Следующий сеанс связи через 5 минут
      SIM900_PrepareConnection();
      if(SIM900Status >= SIM900_UP){
        if (Now.yy == 0){
          SIM900_GetTime();              // Перед сеансом связи актуализируем время.
          RecToHistory(EVENT_START);     // Если сеанс первый - впишем в историю событие старта
        }         
        else SIM900_GetTime();              
      }  
       
//	  uart_send("ATD+79040448302;"); _delay_ms(5000); uart_send("ATH0");
//		SIM900_GetBalance();
			State.balance = 179;

      if(SIM900Status >= SIM900_GPRS_OK)    // Если со связью всё в порядке замутим сеанс связи с сервером
      {
        SIM900_GetRemoteSettingsTimestamp();   // Получим время последнего изменения настроек на сервере
        if(timeCompare(&options.localSettingsTimestamp, &remoteSettingsTimestamp) > 0)   // Если локальные настройки новее
        {
          SIM900_SendSettings();                                                  // Отошлем их на сервер
        } 
        else if (timeCompare(&options.localSettingsTimestamp, &remoteSettingsTimestamp) < 0) // Если настройки на сервере новее
        {
          SIM900_GetSettings();                                                       // Скачаем и примем их
        }

        SIM900_SendStatus();                                                          // Отошлем на сервер текущее состояние
//        SIM900_SendHistory();                                                         // Отошлем на сервер историю событий
      }
    } 
  }
}

//------------------------------------------------------------------------------
void OneMoreMin(void)
{
  static uint8_t Min = 0;
  Min ++;
  if (Min == 60)
  {
    Min = 0;
    //SIM900_GetBalance();
  }

  if(Min%5 == 0){
    //SIM900_SendReport();
  }  

  if(State.Vbat < 660){         // 3.3V
    //uart_send("AT+CPOWD=1");   // Выключим модуль
  }
}
//---------------------------------------------------------------------
void OneMoreSec(void)
{
  uint16_t a;
  sei();
  Seconds ++;
  Now.ss ++; 
  if (Now.ss == 60)
  {
    Now.ss = 0; Now.mm ++;
    if(Now.mm == 60)
    {
      Now.mm = 0;Now.hh ++;
      if(Now.hh == 24){Now.hh = 0; Now.dd ++;}
    }
  }

  if(PumpPause > 0) PumpPause --;	// Отсчитываем паузу перед повторным включением насоса
  if(options.PumpWorkFlag == 1 && (PORTC & 0b00010000) == 0 ) // Если насос должен быть включен, но он вЫключен,
  {
    a = options.PumpTimeLeft;										// Сохраним оставшееся время
    PumpStart();										// Снова включим насос (Оставшееся время при этом станет равным заданному в расписании)
    options.PumpTimeLeft = a;										// Вернем оставшееся время
  }

  if(LightLeft > 0) LightLeft --; 
  if(LightLeft == 0) {LightLeft = -1; LIGHT_OFF; Save(); MenuMode = MD_STAT;}

      // ????????????
  if(PumpPause != 0 && options.PumpTimeLeft < 3)	// Если таймер на паузе перед включением и скоро включение - время не отсчитывается!
  {
    Seconds --;
    return;
  }
  
  if(Seconds == 33){         // Каждую 56 секунду
    sensor_write(0x44);   // старт измерения температуры
  }

  if(Seconds == 35){     // Каждую 58 секунду - чтение температуры
    State.Temp = sensor_write(0xBE); // чтение температурных данных c dc18_B_20 / dc18_S_20
    //  Temp >>= 4; // 4
    if(State.Temp <= options.HeaterOnTemp*16) HeaterStart();	//
    if(State.Temp >= options.HeaterOffTemp*16) HeaterStop();	// При необходимости включим или выключим обогреватель
    if(State.Temp < -3 && options.PumpWorkFlag == 0) options.FrostFlag = 1;
  }


  if(Seconds == 60)			// Прошла еще одна минута
  {
    Seconds = 0;
    
    if(options.PumpWorkDuration != 0 && options.PumpRelaxDuration != 0)	//Проверим что время работы и отдыха насоса не равно 0. Если равно - никакого автоматического включения/выключения!
    {
      options.PumpTimeLeft --;
      if(options.PumpTimeLeft <= 0 && options.PumpWorkFlag == 1)PumpStop();   // Закончилось время работы насоса
      if(options.PumpTimeLeft <= 0 && options.PumpWorkFlag == 0)              // Закончилось время отдыха насоса
        if(PumpStart() == 0)    // Если запуск насоса не удался
        {
          //LCD_gotoXY(0, 2); LCD_writeString(str);		//Если старт насоса вернул ошибку - отобразим её
          options.PumpTimeLeft ++; 									//И подождем еще одну минуту
        }
    }
    OneMoreMin();
  }
  if(SilentLeft > 0) SilentLeft --;     // Счетчик секунд до сеанса связи

  if(MenuMode == MD_STAT)ShowStat();
  
}

//------------------------------------------------------------------------------
ISR(ADC_vect)          // Завершение преобразования АЦП
{
  	State.Vbat = ADC;		// Преобразуем условные единицы в вольты
}
//------------------------------------------------------------------------------

ISR(INT1_vect)   //CLK от клавы                          КЛАВИАТУРА
{
  unsigned char bstat;  
	cli();
	LightLeft = LIGHT_TIME; 
  LIGHT_ON;

  bstat = PIND & 0b01111000;
  if (bstat == 0b01010000)       // Кнопка вниз 
	{
  	switch(MenuMode)
  	{
    	case MD_DIRPUMP:{ PumpStop(); LCD_gotoXY(0, 3);LCD_writeString("   Отключено  "); break;}
    	case MD_DIRHEATER:{ HeaterStop(); LCD_gotoXY(0, 3);LCD_writeString("   Отключено  "); break;}
    	case MD_PUMPWORKTIME:
    	{
      	if(options.PumpWorkDuration == 0) options.PumpWorkDuration = 420;
      	else options.PumpWorkDuration -= DELTA_TIME;
				settingsWasChanged = 1;
      	DrawMenu();
      	break;
    	}
    	case MD_PUMPRELAXTIME:
    	{
      	if(options.PumpRelaxDuration == 0) options.PumpRelaxDuration = 2160;
      	else options.PumpRelaxDuration -= DELTA_TIME;
				settingsWasChanged = 1;
      	DrawMenu();
      	break;
    	}

    	case MD_MIN_T:
    	{
      	if(options.HeaterOnTemp > 0) options.HeaterOnTemp --;
      	DrawMenu();
				settingsWasChanged = 1;
      	break;
    	}

    	case MD_MAX_T:
    	{
      	if(options.HeaterOffTemp-1 > options.HeaterOnTemp) options.HeaterOffTemp --;
      	DrawMenu();
				settingsWasChanged = 1;
      	break;
    	}
    	case MD_STAT:
    	{
				SilentLeft = 2;
				break;
    	}
    	case MD_CLEAR:			// Забыть статистику
    	{
      	options.FrostFlag = 0;
      	PumpPause = 0;
      	MenuMode = MD_STAT;
      	DrawMenu();
      	break;
    	}
  	}
	}
  if (bstat == 0b01100000)       // Кнопка вверх
	{
  	switch(MenuMode)
  	{
    	case MD_DIRPUMP:				// Включить насос вручную
    	{
      	LCD_gotoXY(0, 3);
      	if(PumpStart() == 0) LCD_writeStringInv(str);	//Если старт насоса вернул ошибку - отобразить её
      	else { LCD_writeString("  Включено    ");}		// Если все ОК. И сбросим счетчик секунд чтобы новая минута началась с нуля
      	break;
    	}
    	case MD_DIRHEATER:{ HeaterStart(); LCD_gotoXY(0, 3);LCD_writeString("  Включено    "); break;}
    	case MD_PUMPWORKTIME:
    	{
      	options.PumpWorkDuration += DELTA_TIME;
      	if(options.PumpWorkDuration > 420) options.PumpWorkDuration = 0;
				settingsWasChanged = 1;
      	DrawMenu();
      	break;
    	}
    	case MD_PUMPRELAXTIME:
    	{
      	options.PumpRelaxDuration += DELTA_TIME;
      	if(options.PumpRelaxDuration > 2160) options.PumpRelaxDuration = 0;
				settingsWasChanged = 1;
      	DrawMenu();
      	break;
    	}

    	case MD_MIN_T:
    	{
      	if(options.HeaterOnTemp+1 < options.HeaterOffTemp) options.HeaterOnTemp ++;
      	DrawMenu();
				settingsWasChanged = 1;
      	break;
    	}

    	case MD_MAX_T:
    	{
      	if(options.HeaterOffTemp < 20) options.HeaterOffTemp ++;			//Временно для тестов. Исправить на 10С-15С
      	DrawMenu();
				settingsWasChanged = 1;
      	break;
    	}
    	case MD_STAT:
    	{
      	break;
    	}
    	case MD_CLEAR:			// Забыть настройки
    	{
      	options.PumpRelaxDuration = 0;
      	options.PumpWorkDuration = 0;
      	options.HeaterOffTemp = 10;
      	options.HeaterOnTemp = 5;
				settingsWasChanged = 1;
      	Save();
      	options.PumpTimeLeft = 0;
      	PumpStop();
      	MenuMode = MD_STAT;
      	DrawMenu();
      	break;
    	}
    				
  	}
	}
  if (bstat == 0b00110000)       // Кнопка влево
  {
		Save();
		MenuMode --;
		if(MenuMode < 0) MenuMode = MD_LAST-1;
		DrawMenu();
  }
  
  if (bstat == 0b01110000)      // Кнопка вправо
  {  
		Save();
		MenuMode ++;
		if(MenuMode == MD_LAST) MenuMode = 0;
		DrawMenu();
  }

  sei();
}
//------------------------------------------------------------------------------
ISR(TIMER1_COMPA_vect) //обработка совпадения счетчика1. Частота 10Гц. (для 8МГц)
{
  static uint16_t Tacts = 0;
  Tacts ++;

  if(TimeoutTackts > 0) TimeoutTackts --; 

  if(Tacts == 10 )		// (8 000 000 / 256) / 3125 = 10
  {
    Tacts = 0;
    OneMoreSec();
  }
}
//----------------------------------------------------------------
ISR(USART_RXC_vect) //Обработчик прерывания по окончанию приёма байта
{
	int16_t nextwptr, prevwptr, i;
	if(rx.wptr == rx.startptr && rx.ptrs[0] == -1){   // Если это первый байт новой строки и нет необработаных строк
  	rx.wptr = 0;
  	rx.startptr = 0;    // Запишем эту строку в самое начало буфера
	}
	rx.buf[rx.wptr] = UDR;
	nextwptr = rx.wptr + 1;
	prevwptr = rx.wptr - 1;
	if(rx.buf[rx.wptr] == CHAR_LF && rx.wptr > 0 && rx.buf[prevwptr] == CHAR_CR){  // Если приняли подряд CHAR_CR и CHAR_LF
  	rx.buf[prevwptr] = '\0';                                      // Допишем нуль-терминатор (потеряв(отбросив) CHAR_CR и CHAR_LF
  	if(prevwptr != rx.startptr){                                  // Если это нормальная, полноразмерная строка(состоит не только из CR LF)
    	i = 0;
    	while(rx.ptrs[i] != -1 && i < RXBUFSTRCOUNT) i ++;    // Найдем в rx.ptrs первую незанятую ячейку
    	if(i == RXBUFSTRCOUNT) { // Ужас-ужас - кончилось место под смещения строк (rx.ptrs)
      	rx.ptrs_overflow_count ++;
      	i = RXBUFSTRCOUNT-1;      // Перезапишем последнее сообщение
      	//return; 								// Или просто потеряем его (последнее сообщение)
    	}
    	rx.ptrs[i] = rx.startptr;   // Впишем в rx.ptrs смещение свеже принятой строки
    	rx.startptr = rx.wptr;      // Запомним где начнется след сообщение
  	}
  	else {    // Пустая строка CR+LF
    	rx.wptr = prevwptr;
  	}
	}
	else {      // Просто еще один символ запишем в буфер
  	if(nextwptr == RXBUFMAXSIZE){ // Ужас-ужас - буфер закончился
    	rx.buf_overflow_count ++;   // Никакого ужаса. Мы просто не инкрементируем rx.wptr, и перезаписи не произойдет.
    	rx.buf[prevwptr] = CHAR_CR; // Когда придет CHAR_LF, функция сможет закончить эту строку
  	}
  	else
  	rx.wptr = nextwptr;
	}
}
//----------------------------------------------------------------
void uart_init( void )
{
  //настройка скорости обмена
  UBRRH = 0;
  UBRRL = 12;      // для 8МГц 51 - 9600+0.2%ошибки,  25 - 19200+0,2%ошибки,  12 - 38400
  //8 бит данных, 1 стоп бит, без контроля четности
  UCSRC = ( 1 << URSEL ) | ( 1 << UCSZ1 ) | ( 1 << UCSZ0 );
  //разрешить передачу, прием данных и прерывание по приёму байта
  UCSRB =  ( 1 << TXEN ) | ( 1 << RXEN ) | (1 << RXCIE );
}
//----------------------------------------------------------------
ISR(USART_UDRE_vect)     // Прерывание по опустошению регистра данных
{
  cli();
  if( FIFO_IS_EMPTY( uart_tx_fifo ) ) {
    UCSRB &= ~( 1 << UDRIE ); //если данных в fifo больше нет то запрещаем это прерывание
  }
  else {
    //иначе передаем следующий байт
    char txbyte = FIFO_FRONT( uart_tx_fifo );
    FIFO_POP( uart_tx_fifo );
    UDR = txbyte;
  }
  sei();
}
//----------------------------------------------------------------
int uart_send(char *str)
{
  for(int i = 0; str[i] != 0; i ++)    //Помещаем строку в буфер передатчика
  FIFO_PUSH( uart_tx_fifo, str[i] );

  FIFO_PUSH( uart_tx_fifo, CHAR_CR );  //CR
  FIFO_PUSH( uart_tx_fifo, CHAR_LF );  //LF

  UCSRB |= ( 1 << UDRIE);  // Разрешаем прерывание по освобождению передатчика
  return 0;
}
//----------------------------------------------------------------
void incomingMessage(char* s)
{
  //  char strnum[15];
  //  static uint16_t num = 0;
  /*
  free(message);
  message = (char*) calloc(strlen(s)+3, sizeof(char));
  if(message == NULL){
    LCDPuts("___calloc fail");
    return;
  }
  strcpy(message, s);
  */
  //  strnum[0] = '_';
  //  itoa(num, strnum+1, 10);
  //  strncat(strnum, message, 11);
  //  num ++;
  //  LCD_Puts(strnum);
  
}
//----------------------------------------------------------------
void renewLCD(void)
{
}
//----------------------------------------------------------------
uint8_t waitAnswer(char *answer, uint16_t timeout)  // Ожидает ответа от sim900, сравнивает с заданым. Если равны, возвращает 1. По таймауту (в сек/10) возвращает 0
{
  TimeoutTackts = timeout;			// Запустим отсчёт таймаута
  while(1){
    while(rx.ptrs[0] == -1 && TimeoutTackts != 0);
    if(TimeoutTackts == 0)                                          // Если вышли из цикла по таймауту, возвращаем 0 
      return 0;
    if(strncmp((char*)rx.buf+rx.ptrs[0], answer, strlen(answer)) == 0){     // Если получен нужный ответ, возвращаем 1
      dropMessage();
      return 1;                                                
    }
    dropMessage();		// "неожиданные" сообщения теряются
  } 
  return 0;
}
//----------------------------------------------------------------
void waitMessage(void)
{
  while(rx.ptrs[0] == -1);
}
//----------------------------------------------------------------
void dropMessage(void)
{
	uint8_t i;
	for (i = 0; i < RXBUFSTRCOUNT-1 && rx.ptrs[i] != -1; i ++)
	rx.ptrs[i] = rx.ptrs[i+1];
	rx.ptrs[RXBUFSTRCOUNT-1] = -1;
}
//----------------------------------------------------------------
int16_t str2int(char* str)
{
  uint8_t minus = 1;
  uint16_t result = 0;
  while(*str != '\0' && (*str > '9' || *str < '0')) str ++;   // Ищем первую цифру или конец строки
  if(*str == '\0') return 0;                                  // Если нашли конец строки, возвращаем 0
  if(*(str-1) == '-') minus = -1;                             // Если цифру, смотрим нет ли перед ней минуса
  result = *str - '0';
  str ++;
  while(*str <= '9' && *str >= '0'){
    result  *= 10;
    result += *str - '0';
    str ++;
  }
  return result * minus;
}
//---------------------------------------------------------------------
void Save(void)
{
  if(options.PumpWorkDuration == 0 || options.PumpRelaxDuration == 0) PumpStop();  // Это выключает насос если в меню задали нулевую длительность работы или отдыха насоса
//  if((uint16_t)eeprom_read_word((uint16_t *)0x0000) == 0 && PumpRelaxDuration != 0) PumpTimeLeft = PumpRelaxDuration; //PumpStop();		Что-то тут непонятное........

	if(settingsWasChanged){
		options.localSettingsTimestamp.yy = Now.yy; options.localSettingsTimestamp.MM = Now.MM; options.localSettingsTimestamp.dd = Now.dd;
		options.localSettingsTimestamp.hh = Now.hh; options.localSettingsTimestamp.mm = Now.mm; options.localSettingsTimestamp.ss = Now.ss;
		eeprom_write_block(&options, (void*)0x00, sizeof(struct TSettings));
		settingsWasChanged = 0;
	}
}
//---------------------------------------------------------------------
void PumpStop(void)
{
  options.PumpTimeLeft = options.PumpRelaxDuration;
  options.PumpWorkFlag = 0;
  PORTC &= 0b11101111;			// вЫключим насос (он на PC4)
  if(PumpPause < 1) PumpPause = PUMP_RESTART_PAUSE;
}
//---------------------------------------------------------------------
uint8_t PumpStart(void)
{
  if(options.PumpWorkDuration == 0  || options.PumpRelaxDuration == 0){strcpy(str, "Нет расписания"); return 0;}
  if(options.FrostFlag == 1){strcpy(str, "Возм.заморозка"); return 0;}
  if(PumpPause > 0){ strcpy(str, "Пауза "); itoa(PumpPause/10, buf, 10); strcat(str, buf); strcat(str, " сек   ");return 0;}
  options.PumpTimeLeft = options.PumpWorkDuration;
  CheckUPause = 20;		// 2 секунды не проверять питающее напряжение!
  options.PumpWorkFlag = 1;
  PORTC |= 0b00010000;	// Включим насос (он на PC4)
  return 1;
}
//---------------------------------------------------------------------
void HeaterStart(void)
{
  PORTC |= 0b00000100;	// Включим обогреватель (он на PC2)
}
//---------------------------------------------------------------------
void HeaterStop(void)
{
  PORTC &= 0b11111011;	// вЫключим обогреватель (он на PC2)
}
//---------------------------------------------------------------------
int8_t timeCompare(struct TTime *time1, struct TTime *time2)    // Возвращает 1 если time1 больше (позже) time2. -1 если наоборот. 0 если даты равны
{
  if(time1->yy > time2->yy) return 1;
  if(time1->yy < time2->yy) return -1;
  if(time1->MM > time2->MM) return 1;
  if(time1->MM < time2->MM) return -1;
  if(time1->dd > time2->dd) return 1;
  if(time1->dd < time2->dd) return -1;
  if(time1->hh > time2->hh) return 1;
  if(time1->hh < time2->hh) return -1;
  if(time1->mm > time2->mm) return 1;
  if(time1->mm < time2->mm) return -1;
  if(time1->ss > time2->ss) return 1;
  if(time1->ss < time2->ss) return -1;
  return 0;
}
//---------------------------------------------------------------------
void DrawMenu(void)
{
  int x;
  LCD_clear();
  switch(MenuMode)
  {
    case MD_DIRPUMP:
    case MD_DIRHEATER:
    {
      strcpy(buf, "    Насос     "); buf[11] = 0x80; buf[12] = 0x81;
      if(MenuMode == MD_DIRHEATER){strcpy(buf, "   Обогрев    "); buf[11] = 0xA8; buf[12] = 0xA8;}
      LCD_gotoXY(0, 0);LCD_writeStringInv(buf);
      LCD_gotoXY(0, 1);	LCD_writeString(" Прямо сейчас ");
      LCD_gotoXY(0, 2);		strcpy(buf, "     ВКЛ      "); buf[4] = 0xAB; buf[9] = 0xAB; LCD_writeString(buf);
      LCD_gotoXY(0, 3);	LCD_writeString("              ");
      LCD_gotoXY(0, 4);		strcpy(buf, "     ВыКЛ     "); buf[4] = 0xAC; buf[9] = 0xAC; LCD_writeString(buf);
      break;
    }
    case MD_PUMPWORKTIME:
    case MD_PUMPRELAXTIME:
    {
      x = 3; if(MenuMode == MD_PUMPRELAXTIME) x = 10;
      LCD_gotoXY(0, 0);LCD_writeStringInv(" Насос.Распис ");
      LCD_gotoXY(0, 1);	LCD_writeString(" Работ  Стоит ");
      LCD_gotoXY(0, 2);		strcpy(buf, "              "); buf[x] = 0xAB; LCD_writeString(buf);
      LCD_gotoXY(0, 3);MinToStr(options.PumpWorkDuration, buf+1); strcat(buf, "  ");MinToStr(options.PumpRelaxDuration, str);strcat(buf, str);LCD_writeString(buf);
      LCD_gotoXY(0, 4);		strcpy(buf, "              "); buf[x] = 0xAC; LCD_writeString(buf);
      break;
    }
    case MD_MIN_T:
    case MD_MAX_T:
    {
      x = 3; if(MenuMode == MD_MAX_T) x = 10;
      LCD_gotoXY(0, 0);LCD_writeStringInv("Обогрев.Распис");
      LCD_gotoXY(0, 1);	LCD_writeString("  ВКЛ   ВыКЛ  ");
      LCD_gotoXY(0, 2);		strcpy(buf, "              "); buf[x] = 0xAB; LCD_writeString(buf);
      itoa(options.HeaterOnTemp, buf+2, 10); strcpy(str, "*C    "); str[0] = 0xBF; strcat(buf, str); itoa(options.HeaterOffTemp, str, 10); strcat(buf, str); strcpy(str, "*C   ");str[0] = 0xBF; strcat(buf, str);
      LCD_gotoXY(0, 3);	LCD_writeString(buf);
      LCD_gotoXY(0, 4);		strcpy(buf, "              "); buf[x] = 0xAC; LCD_writeString(buf);
      break;
    }
    case MD_STAT: { ShowStat(); break;}
    case MD_CLEAR:
    {
      LCD_gotoXY(0, 0);LCD_writeStringInv("    Сброс     ");
      LCD_gotoXY(0, 1);LCD_writeStringInv("              ");
      LCD_gotoXY(0, 2);		strcpy(buf, "  настройки   "); buf[1] = 0xAB; buf[12] = 0xAB; LCD_writeString(buf);
      LCD_gotoXY(0, 3);	LCD_writeString("   Сбросить   ");
      LCD_gotoXY(0, 4);		strcpy(buf, "  статистику  "); buf[1] = 0xAC; buf[12] = 0xAC; LCD_writeString(buf);
      break;
    }
		case MD_DEBUG:
		{
			LCD_gotoXY(0, 0);LCD_writeStringInv("    DEBUG     ");
			itoa(State.balance, buf, 10); strcat(buf, " R, "); itoa(State.Vbat, str, 10); strcat(buf, str); strcat(buf, "V");
			LCD_gotoXY(0, 1);LCD_writeString(buf);

			strcpy(buf, "N");
			itoa(Now.yy, str, 10); strcat(buf, str); itoa(Now.MM, str, 10); strcat(buf, str);itoa(Now.dd, str, 10); strcat(buf, str); strcat(buf, " ");
			itoa(Now.hh, str, 10); strcat(buf, str); strcat(buf, ":"); itoa(Now.mm, str, 10); strcat(buf, str); strcat(buf, ":"); itoa(Now.ss, str, 10); strcat(buf, str);
			LCD_gotoXY(0, 2);LCD_writeString(buf);
			strcpy(buf, "R");
			itoa(remoteSettingsTimestamp.yy, str, 10); strcat(buf, str); itoa(remoteSettingsTimestamp.MM, str, 10); strcat(buf, str); itoa(remoteSettingsTimestamp.dd, str, 10); strcat(buf, str); strcat(buf, " ");
			itoa(remoteSettingsTimestamp.hh, str, 10); strcat(buf, str); strcat(buf, ":"); itoa(remoteSettingsTimestamp.mm, str, 10); strcat(buf, str); strcat(buf, ":"); itoa(remoteSettingsTimestamp.ss, str, 10); strcat(buf, str);
			LCD_gotoXY(0, 3);LCD_writeString(buf);
			strcpy(buf, "L");
			itoa(options.localSettingsTimestamp.yy, str, 10); strcat(buf, str); itoa(options.localSettingsTimestamp.MM, str, 10); strcat(buf, str); itoa(options.localSettingsTimestamp.dd, str, 10); strcat(buf, str); strcat(buf, " ");
			itoa(options.localSettingsTimestamp.hh, str, 10); strcat(buf, str); strcat(buf, ":"); itoa(options.localSettingsTimestamp.mm, str, 10); strcat(buf, str); strcat(buf, ":"); itoa(options.localSettingsTimestamp.ss, str, 10); strcat(buf, str);
			LCD_gotoXY(0, 4);LCD_writeString(buf);

		}
  }
}
//---------------------------------------------------------------------
void ShowStat(void)
{
  LCD_clear();
  LCD_gotoXY(0, 0); LCD_writeStringInv("  Статистика  ");		// Отобразим заголовок

  itoa(State.Temp/16, buf, 10);
  strcpy(str, "*C           "); str[0] = 0xBF;
//  if( PORTD & 0b01000000 ){str[4] = 0xA8;str[5] = 0xA8;}
//  if( PORTD & 0b10000000 ){str[7] = 0x80;str[8] = 0x81;}
  if( 1 ){str[4] = 0xA8;str[5] = 0xA8;}
  if( 1 ){str[7] = 0x80;str[8] = 0x81;}
  strcat(buf, str);
  LCD_gotoXY(0, 1);	 LCD_writeString(buf);					// Отобразим температуру и состояние обогревателя и насоса

  if(options.PumpWorkDuration == 0 || options.PumpRelaxDuration == 0)
  strcpy(buf, "Таймер ОТКЛ   ");
  else
  {
    MinToStr(options.PumpTimeLeft, buf);
    strcat(buf, " до ");
    options.PumpWorkFlag == 1 ? strcpy(str, "вЫк       ") : strcpy(str, "вкл      ");
    str[3] = 0x80; str[4] = 0x81;
    strcat(buf, str);
  }
  if(PumpPause != 0 && options.PumpWorkFlag == 1)
  {
    strcpy(buf, "Пауза ");
    itoa(PumpPause/10, str, 10);
    strcat(buf, str);
    strcat(buf, " сек.   ");
  }
  if(options.FrostFlag == 1){
		strcpy(buf, "Возм.заморозка");
		LCD_gotoXY(0, 2);	 LCD_writeString(buf);					// Отобразим "время до" или предупреждение если есть
	}

	
	itoa(SilentLeft/60, buf, 10);
	strcat(buf, "min");
	LCD_gotoXY(0, 4); LCD_writeString(buf);
	strcpy(buf, "SIM900Stat ");
	itoa(SIM900Status, str, 10);
	strcat(buf, str);
	LCD_gotoXY(0, 5); LCD_writeString(buf);
	itoa(State.balance, buf, 10);
	LCD_gotoXY(0, 3); LCD_writeString(buf);

	itoa(State.balance, buf, 10); strcat(buf, " R, "); itoa(State.Vbat, str, 10); strcat(buf, str); strcat(buf, "V");
	LCD_gotoXY(0, 2);LCD_writeString(buf);
	itoa(Now.yy, buf, 10); itoa(Now.MM, str, 10); strcat(buf, str); itoa(Now.dd, str, 10); strcat(buf, str); strcat(buf, " ");
	itoa(Now.hh, str, 10); strcat(buf, str); strcat(buf, ":"); itoa(Now.mm, str, 10); strcat(buf, str); strcat(buf, ":"); itoa(Now.ss, str, 10); strcat(buf, str);
	LCD_gotoXY(0, 3);LCD_writeString(buf);

	

  /*
  strcpy(buf, "Сб.пит:");
  itoa(LongBreak, str, 10); strcat(buf, str); strcat(buf, "дл.");
  itoa(ShortBreak, str, 10); strcat(buf, str); strcat(buf, "кр.   ");
  LCD_gotoXY(0, 3); LCD_writeString(buf);						// Отобразим статистику по питанию
  */
  /*
  strcpy(buf, "  вкл "); buf[0] = 0xA8; buf[1] = 0xA8;
  if(HeaterTotal != 0) itoa((unsigned int)((HeaterWork*100)/HeaterTotal), str, 10);
  else strcpy(str, "--");
  strcat(buf, str); strcat(buf, "% врем  ");
  if(HeaterWork >= 0x28F3980) strcpy(buf, "НуженСбросСтат");	// Обогреватель работает уже 497 суток. Счетчик вот-вот переполнится
  LCD_gotoXY(0, 4); LCD_writeString(buf);						// Отобразим статистику по обогреву
  */
}
//---------------------------------------------------------------------
void MinToStr(unsigned int Min, char *str)
{
  int m, h;
  h = Min/60;
  m = Min - h*60;
  str[0] = '0';
  h < 10 ? itoa(h, str+1, 10) : itoa(h, str, 10);
  str[2] = ':';
  str[3] = '0';
  m < 10 ? itoa(m, str+4, 10) : itoa(m, str+3, 10);
  return;
}
//---------------------------------------------------------------------
