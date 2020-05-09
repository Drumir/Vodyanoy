/*
 * Sim900.c
 *
 * Created: 24.08.2017 22:26:28
 * Author : Drumir
 */ 

#include "vodyanoy2.0.h"


int main(void)
{
	ACSR |= 0x80;			// Выключим не нужный, но включеный по умолчанию аналоговый компаратор
  DDRC  = 0b01111100;    // 7-Датчик потопа, 6-DQ DS18B20, 5-LCD_RESET, 4-насос, 3-LCD_DC, 2-ТЭН, 1-н, 0-н
  PORTC = 0b11100000;
  DDRD =  0b10000010;    // 7-En4V, 6-3 клава, 2-геркон, 1-TX, 0-RX
  PORTD = 0b01111000;
  DDRB  = 0b11111010;    // 7-LCD_CLK, 6-LCD_DC, 5-LCD_MOSI, 4-LCD_SS, 3 - подсветка дисплея, 2-power_fail, 1 - SIM900_pwrkey, 0 - SIM900_status
  PORTB = 0b00000010;
	DDRA =  0b00000000;			// 7-1-н, 0 - ADC Vbat
	PORTA = 0b00000000;


  sbi(GICR, INT1);          // Разрешить INT1 для клавы
  sbi(GICR, INT2);          // Разрешить INT2 для монитора питания
	
  sbi(MCUCR, ISC11); cbi(MCUCR, ISC10);		// Прерывание INT1 по переходу 1->0  на линии CLK клавы 
	cbi(MCUCSR, ISC2);											// Прерывание INT2 по переходу 1->0 от монитора питания

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
	rx.buf[RXBUFMAXSIZE-1] = 0;                               // Запишем сразу после буфера '\0' чтобы строковые функции не сошли с ума
  
  SIM900Status = SIM900_NOTHING;
	MenuMode = MD_STAT;
	LightLeft = LIGHT_TIME; LIGHT_ON;	// Сразу включим подсветку
	State.PumpPause = 0;
	Seconds = 0;
	CheckUPause = 0;
	SilentLeft = FIRST_CONNECT_DELAY;
	TimeoutTackts = 0;
	TimeoutsCount = 0;
	State.PowerFailFlag = 0;
	State.OpenDoorFlag = 0;
	State.FloodingFlag = 0;
	historyPtr = 0;
  if(eeprom_read_word((void*)sizeof(struct TSettings)) == SOFT_RESET_FLAG){   // Если в EEPROM сохранена история, загрузим ее
  	eeprom_read_block(&History, (void*)sizeof(struct TSettings)+2, sizeof(History)); // Загрузим сохраненную историю из EEPROM
    eeprom_write_word((void*)sizeof(struct TSettings), 0);  //Запишем сразу после настроек число 0, означающий, что записанная история уже не актуальна
    for(historyPtr = HISTORYLENGTH-1; History[historyPtr].EventCode == EVENT_NONE && historyPtr >= 0; historyPtr --);  // Ищем первое непустое событие
	historyPtr ++; if(historyPtr == HISTORYLENGTH) historyPtr = 0;
  }
  else  // Иначе просто очистим историю
    for(uint16_t i = 0; i < HISTORYLENGTH; i ++)		// Очистим буфер истории
      History[i].EventCode = EVENT_NONE;

  
  Now.yy = 0;         // По нулю в количестве лет определим, что Now еще не актуализировалось
  remoteSettingsTimestamp.yy = 0;

  eeprom_read_block(&options, (const void*)0x00, sizeof(struct TSettings));


	if(options.fFrostNotifications == 0xFF || options.ConnectPeriod == 0xFFFF) 	// Если прочитался мусор (после перепрошивки) сбросим значения в поумолчанию.
//	if(1) 	// Если прочитался мусор (после перепрошивки) сбросим значения в поумолчанию.
	{
  	options.FreezeFlag = 0;											// Флаг о возможной заморозке насоса
  	options.PumpWorkFlag = 0;										// Флаг, что в данный момент насос включен
		options.DirectControlFlags = 0;
    options.fFrostNotifications = 0;            // Флаги оповещения об охлаждении. 1 в 3 бите - смс оператору. Во 2 бите - смс админу. в 1 - звонок оператору. в 0 - звонок админу.
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
    options.ConnectPeriod = 10;                  // Период (в минутах) докладов на сервер. Если 0 - только при необходимости или по звонку
    options.DisconnectionTime = 120;            // Длительность отсутствия связи для оповещения о проблемах со связью
    options.MinBalance = 10;                    // Баланс, при достижении которого нужно оповестить о критическом балансе
    options.DailyReportTime = 960;              // Время ежедневного отчета о состоянии (в минутах с начала суток)
    options.PumpTimeLeft = 0;		                // Сколько осталось качать/отдыхать насосу (в минутах?)
    options.HeaterOnTemp = 2;				            // Температура отключения обогревателя
    options.HeaterOffTemp = 10;			            // Температура включения обогревателя
    options.FrostTemp = 0;                    // Температура предупреждения о заморозке
    options.WarmTemp = 40;                      // Температура предупреждения о перегреве
    options.localSettingsTimestamp.yy = 0;      // Время последнего обновления локальных настроек
    options.localSettingsTimestamp.MM = 0;
    options.localSettingsTimestamp.dd = 0;
    options.localSettingsTimestamp.hh = 0;
    options.localSettingsTimestamp.mm = 0;
    options.localSettingsTimestamp.ss = 0;
		strcpyPM((char*)options.Link, PM_link);
	}
  
  //Save();

	if(options.PumpWorkFlag == 1 && options.PumpTimeLeft != 0)		// Если до длительного отключения насос был включен
	State.PumpPause = PUMP_RESTART_PAUSE/2;						// На всякий случай подождем ещ половину "паузы перед включением"
    
  uart_init();
  spi_init();
 	_delay_ms(100);      //на момент инициализации SLI_LCD, дисплей должен быть уже включен и готов
  LCD_init();
  sensor_write(0x44);   // старт измерения температуры
  _delay_ms(1000);
  State.Temp = sensor_write(0xBE); // чтение температурных данных c dc18_B_20 / dc18_S_20
  sei();
	measureBattery();
	ShowStat();

  LCD_gotoXY(0, 4); LCD_writePMstring(MSG_SIM900Start);
  SIM900_PowerOn();	
  if(SIM900Status >= SIM900_UP){
    SIM900_GetTime();              // Актуализируем время.
    RecToHistory(EVENT_START);     // Впишем в историю событие старта
  }

  LCD_gotoXY(0, 4); LCD_writePMstring(MSG_Loading);
  SIM900_PrepareConnection();
	LCD_gotoXY(0, 4); 
	switch(SIM900Status){
		case SIM900_BAT_FAIL:		{LCD_writePMstring(MSG_BatFail); break;}
		case SIM900_FAIL:				{LCD_writePMstring(MSG_SimFail); break;}
		case SIM900_GSM_FAIL:		{LCD_writePMstring(MSG_GSMFail); break;}
		case SIM900_GPRS_FAIL:	{LCD_writePMstring(MSG_GPRSFail); break;}
		default:								{LCD_writePMstring(MSG_Successful);}	
	}

	if(SIM900Status >= SIM900_GSM_OK){
		LCD_gotoXY(0, 4); LCD_writeString(" Get balance  ");
		SIM900_GetBalance();
	}

  while (1)				/*				ГЛАВНЫЙ ЦИКЛ				ГЛАВНЫЙ ЦИКЛ				ГЛАВНЫЙ ЦИКЛ				ГЛАВНЫЙ ЦИКЛ				ГЛАВНЫЙ ЦИКЛ */
  {
	if(OneMoreSecCount > 0) OneMoreSec();
	if(bstat) OnKeyPress();
	CheckIncomingMessages();            // Проверим буфер принятых от SIM900 сообщений на предмет необработанных
	CheckNotifications();               // Проверим не произошло ли какое-то событие о котором нужно уведомить админа/оператора 
    if(SilentLeft == 0)    // Проверим не пора ли организовать сеанс связи
    {
      SIM900_GetTime();              // Перед сеансом связи актуализируем время.
      SilentLeft = options.ConnectPeriod*60 - 1L;    // Следующий сеанс связи через options.ConnectPeriod минут
       
      if(SIM900Status >= SIM900_GPRS_OK)    // Если со связью всё в порядке замутим сеанс связи с сервером
      {
				//SIM900_SetTimeFromServer();
        SIM900_GetTime();              // Перед сеансом связи актуализируем время.
				LCD_gotoXY(0, 4); LCD_writeString("CheckServTimeS");
        SIM900_GetRemoteSettingsTimestamp();   // Получим время последнего изменения настроек на сервере
        if(timeCompare(&options.localSettingsTimestamp, &remoteSettingsTimestamp) > 0)   // Если локальные настройки новее
        {
					LCD_gotoXY(0, 4); LCD_writeString("RenewServSetts");
          SIM900_SendSettings();                                                  // Отошлем их на сервер
        } 
        else if (timeCompare(&options.localSettingsTimestamp, &remoteSettingsTimestamp) < 0) // Если настройки на сервере новее
        {
          SIM900_GetSettings(); 
					LCD_gotoXY(0, 4); LCD_writeString("RevewLoclSetts");
			    SilentLeft = options.ConnectPeriod * 60;		// Обновим период сеансов связи
					if(options.DirectControlFlags != 0)					// Проверим состояние флагов прямого удаленного управления
						ApplyDirectControl();
        }

				LCD_gotoXY(0, 4); LCD_writeString("Sending Stats ");
        SIM900_SendStatus();                                                          // Отошлем на сервер текущее состояние
				LCD_gotoXY(0, 4); LCD_writeString("SendingHistory");
        SIM900_SendHistory();                                                         // Отошлем на сервер историю событий
      }
			if(TimeoutsCount > 5 || SIM900Status < SIM900_GPRS_OK)				// Если за один сеанс связи произошло больше 5 таймаутов ожидания ответа от SIM900
			{
				SIM900_CheckConnection();				// Проверим наличие связи
				LCD_gotoXY(0, 4);
				switch(SIM900Status){
					case SIM900_BAT_FAIL:		{LCD_writePMstring(MSG_BatFail); break;}
					case SIM900_FAIL:				{LCD_writePMstring(MSG_SimFail); break;}
					case SIM900_GSM_FAIL:		{LCD_writePMstring(MSG_GSMFail); break;}
					case SIM900_GPRS_FAIL:	{LCD_writePMstring(MSG_GPRSFail); break;}
					default:								{LCD_writePMstring(MSG_Successful);}
				}

			}
			TimeoutsCount = 0;
    } 
  }
}
//------------------------------------------------------------------------------
void ApplyDirectControl(void)
{
	if(options.DirectControlFlags & 0b00010000)		// Сбросить состояние заморозки
		options.FreezeFlag = 0;
	if(options.DirectControlFlags & 0b00001000)		// вЫключить обогреватель
	{
		HeaterStop(); RecToHistory(EVENT_HEATER_STOP_REMOTE);
	}
	else if(options.DirectControlFlags & 0b00000100)		// включить обогреватель
	{
		HeaterStart(); RecToHistory(EVENT_HEATER_START_REMOTE);
	}
	if(options.DirectControlFlags & 0b00000010)		// вЫключить насос
	{
		PumpStop(); RecToHistory(EVENT_PUMP_STOP_REMOTE);
	}
	else if(options.DirectControlFlags & 0b00000001)		// включить насос
	{
		uint8_t eventCode = PumpStart();
		if(eventCode == EVENT_NONE) RecToHistory(EVENT_PUMP_START_REMOTE);
		else RecToHistory(eventCode);
	}
	options.DirectControlFlags = 0;
}

//------------------------------------------------------------------------------
void OneMoreMin(void)
{
  static uint8_t Min = 0;
    
  if(options.PumpWorkDuration != 0 && options.PumpRelaxDuration != 0)	//Проверим что время работы и отдыха насоса не равно 0. Если равно - никакого автоматического включения/выключения!
  {
	  if(State.PowerFailFlag == 0 || options.PumpWorkFlag == 0)						// Если питание ОК или насос отдыхает
	  options.PumpTimeLeft --;
	  if(options.PumpTimeLeft <= 0 && options.PumpWorkFlag == 1){					// Закончилось время работы насоса
		  PumpStop();RecToHistory(EVENT_PUMP_STOP_AUTO);
	  }
	  if(options.PumpTimeLeft <= 0 && options.PumpWorkFlag == 0){             // Закончилось время отдыха насоса
		  if(PumpStart() != EVENT_NONE)    // Если запуск насоса не удался
		  {
			  LCD_gotoXY(0, 2); LCD_writeString(strD);		//Если старт насоса вернул ошибку - отобразим её
			  options.PumpTimeLeft ++; 									//И подождем еще одну минуту
		  }
		  else
		  RecToHistory(EVENT_PUMP_START_AUTO);
	  }
  }

  if(State.Temp <= options.FrostTemp && Notifications.Frost == 0){
	  Notifications.Frost = 1;
	  RecToHistory(EVENT_FROST);
  }
  if(State.Temp > options.FrostTemp){
	  Notifications.Frost = 0;
  }

  if(State.Temp >= options.WarmTemp && Notifications.Warm == 0){
	  Notifications.Warm = 1;
	  RecToHistory(EVENT_WARM);
  }
  if(State.Temp < options.WarmTemp){
	  Notifications.Warm = 0;
  }

	Min ++;
  if (Min == 29)
  {
    Min = 0;
    SIM900_GetBalance();
		if(State.balance < options.MinBalance && Notifications.Balance == 0){ //Если баланс ниже критического и оповещения о низком балансе еще не было
			Notifications.Balance = 1;
			RecToHistory(EVENT_LOW_BALANCE);
		}
		if(State.balance > options.MinBalance)	// Если баланс выше критического, сбросим флаг о том, что оповещение о низком балансе уже было произведено
			Notifications.Balance = 0;
  }

  if(Min%5 == 0){     // Актуализируем время каждые 5 минут
    SIM900_GetTime(); 
  }  

  if(State.Vbat < 660){         // 3.3V
    uart_send("AT+CPOWD=1");   // Выключим модуль
  }
}
//---------------------------------------------------------------------
void OneMoreSec(void)
{
  uint16_t a;
	OneMoreSecCount --; 
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

  if(State.PumpPause > 0)State.PumpPause --;	// Отсчитываем паузу перед повторным включением насоса
  if(options.PumpWorkFlag == 1 && (PORTC & 0b00010000) == 0 ) // Если насос должен быть включен, но он вЫключен,
  {
    a = options.PumpTimeLeft;										// Сохраним оставшееся время
    if(PumpStart() == EVENT_NONE) RecToHistory(EVENT_PUMP_START_AUTO);	// Снова включим насос (Оставшееся время при этом станет равным заданному в расписании)
    options.PumpTimeLeft = a;										// Вернем оставшееся время
  }

  if(LightLeft > 0) LightLeft --; 
  if(LightLeft == 0) {LightLeft = -1; LIGHT_OFF; Save(); MenuMode = MD_STAT;}

      // ????????????
  if(State.PumpPause != 0 && options.PumpTimeLeft < 3)	// Если таймер на паузе перед включением и скоро включение - время не отсчитывается!
  {
    Seconds --;
    return;
  }

  if(Seconds == 33){         // Каждую 33 секунду
    sensor_write(0x44);   // старт измерения температуры
  }

  if(Seconds == 35){     // Каждую 35 секунду - чтение температуры
    State.Temp = sensor_write(0xBE); // чтение температурных данных c dc18_B_20 / dc18_S_20
    //  Temp >>= 4; // 4
    if((PORTC & 0b00000100) == 0 && State.Temp <= options.HeaterOnTemp*16){ HeaterStart(); RecToHistory(EVENT_HEATER_START_AUTO);}	//
    if((PORTC & 0b00000100) != 0 && State.Temp >= options.HeaterOffTemp*16){ HeaterStop(); RecToHistory(EVENT_HEATER_STOP_AUTO);}	// При необходимости включим или выключим обогреватель
    if(State.Temp < -3 && options.PumpWorkFlag == 0 && options.FreezeFlag != 1){ options.FreezeFlag = 1; RecToHistory(EVENT_FREEZE);}
  }


  if(Seconds == 60)			// Прошла еще одна минута
  {
    Seconds = 0;
    OneMoreMin();
  }
  if(SilentLeft > -1) SilentLeft --;     // Счетчик секунд до сеанса связи

	measureBattery();  

	if(MenuMode == MD_STAT)ShowStat();
	
	if(State.PowerFailFlag == 1){			// Если питание отключили
		if(PINB & 0b00000100){						// Потом снова включили
			if(State.PowerFailSecCount > 0)  State.PowerFailSecCount--; // держим паузу перед признанием питания полностью восстановленным
			else OnPowerRestore();  
		}
	}
}

//------------------------------------------------------------------------------
ISR(ADC_vect)          // Завершение преобразования АЦП
{
  	State.Vbat = ADC;		// Преобразуем условные единицы в вольты
}
//------------------------------------------------------------------------------

ISR(INT1_vect)   //CLK от клавы                          КЛАВИАТУРА
{
	LightLeft = LIGHT_TIME;			// Включим подсветку сразу чтобы показать, что не висит
	LIGHT_ON;
	bstat = PIND & 0b01111000;	// Дальше обрабатывать нажатие будет OnKeyPress()
}
//------------------------------------------------------------------------------
ISR(INT2_vect)   //                           Отключение питания
{
//	OnPowerFail();
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
		OneMoreSecCount ++;		//В главном цикле увидят, что OneMoreSecCount != 0 и вызовут OneMoreSec();
//		if(OneMoreSecCount > 240) SoftReset();  //Если главный цикл не выполнялся уже 240 секунд, выполним перезагрузку устройства
  }
}
//----------------------------------------------------------------
ISR(USART_RXC_vect) //Обработчик прерывания по окончанию приёма байта
{
	volatile int16_t nextwptr, prevwptr, i;
	static uint8_t SmsFromAdminFlag = 0;
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
										/*Система удаленной перезагрузки по СМС от админа с текстом "reset"  */
			if(SmsFromAdminFlag > 0)												//Если предыдущая строка распозналась как заголовок смс от админа,
			{
				SmsFromAdminFlag --;
				if(strncmp("reset", (char*)rx.buf+rx.startptr, 5) == 0)	// Сравниваем текст смс от админа с "reset"
        {
          RecToHistory(EVENT_RESTART_BY_SMS);
          SaveHistoryToEEPROM();
					SoftReset();
        }
			}
			if(strncmp("+CMT: \"+79040448302\"", (char*)rx.buf+rx.startptr, 20) == 0)		//Проверяем не смс ли это от админа
				SmsFromAdminFlag = 2;
										/*Конец удаленной системы перезагрузки*/
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
/*
ISR(USART_RXC_vect) //Обработчик прерывания по окончанию приёма байта
{
	volatile int16_t nextwptr, prevwptr, i;
	if(rx.wptr == rx.startptr && rx.ptrs[0] == -1){   // Если это первый байт новой строки и нет необработаных строк
  		rx.wptr = 0;
  		rx.startptr = 0;    // Запишем эту строку в самое начало буфера
	}
	rx.buf[rx.wptr] = UDR;
	nextwptr = rx.wptr + 1;
	prevwptr = rx.wptr - 1;
	if(rx.buf[rx.wptr] == CHAR_LF && rx.wptr > 0 && rx.buf[prevwptr] == CHAR_CR)  // Если приняли подряд CHAR_CR и CHAR_LF, значит сообщение завершено. 
	{
  		rx.buf[prevwptr] = '\0';                                      // Допишем нуль-терминатор (потеряв(отбросив) CHAR_CR и CHAR_LF
  		if(prevwptr != rx.startptr)                                   // Если это нормальная, полноразмерная строка(состоит не только из CR LF)
			{
    		i = 0;
    		while(rx.ptrs[i] != -1 && i < RXBUFSTRCOUNT-1) i ++;    // Найдем в rx.ptrs первую незанятую ячейку
    		rx.ptrs[i] = rx.startptr;   // Впишем в rx.ptrs смещение свеже принятой строки
    		rx.startptr = rx.wptr;      // Запомним где начнется след сообщение
  		}
  		else {    // Пустая строка CR+LF
    		rx.wptr = prevwptr;
  		}

		i = 0;
		while(rx.ptrs[i] != -1 && i < RXBUFSTRCOUNT-1) i ++;    // Проверим не заполнился ли у нас массив под указатели на принятые строки  (rx.ptrs)
		if(i == RXBUFSTRCOUNT-1) { // Ужас-ужас - кончилось место под смещения строк (rx.ptrs)
 			rx.ptrs_overflow_count ++;
			dropMessage();					// Выбросим самое первое(старое) сообщение
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
}*/
//----------------------------------------------------------------
void OnKeyPress(void)
{
	if (bstat == 0b01010000)       // Кнопка вниз
	{
		switch(MenuMode)
		{
			case MD_DIRPUMP:{ PumpStop(); RecToHistory(EVENT_PUMP_STOP_MANUAL); LCD_gotoXY(0, 3);LCD_writeString("   Отключено  "); break;}
			case MD_DIRHEATER:{ HeaterStop(); RecToHistory(EVENT_HEATER_STOP_MANUAL); LCD_gotoXY(0, 3);LCD_writeString("   Отключено  "); break;}
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
				options.FreezeFlag = 0;
				State.PumpPause = 0;
				MenuMode = MD_STAT;
				settingsWasChanged = 1;
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
				if(PumpStart() != EVENT_NONE) LCD_writeStringInv(strD);	//Если старт насоса вернул ошибку - отобразить её
				else { LCD_writeString("  Включено    "); RecToHistory(EVENT_PUMP_START_MANUAL);}		// Если все ОК. И сбросим счетчик секунд чтобы новая минута началась с нуля
				break;
			}
			case MD_DIRHEATER:{ HeaterStart(); RecToHistory(EVENT_HEATER_START_MANUAL); LCD_gotoXY(0, 3);LCD_writeString("  Включено    "); break;}
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
	bstat = 0;
}
//----------------------------------------------------------------
void SoftReset(void)
{
	uart_send("AT+CPOWD=1");		// Нормальное завершение работы SIM900
	LCD_gotoXY(0, 4); LCD_writeStringInv("POWER DOWN NOW");
	_delay_ms(4000);
	wdt_enable(WDTO_15MS);		// Сброс через 15МС сторожевым таймером
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
int uart_send_wo_CRLF(char *str)
{
  for(int i = 0; str[i] != 0; i ++)    //Помещаем строку в буфер передатчика
  FIFO_PUSH( uart_tx_fifo, str[i] );
  UCSRB |= ( 1 << UDRIE);  // Разрешаем прерывание по освобождению передатчика
  return 0;
}
//----------------------------------------------------------------
uint8_t waitAnswer(char *answer, uint16_t timeout)  // Ожидает ответа от sim900, сравнивает с заданым. Если равны, возвращает 1. По таймауту (в сек/10) возвращает 0
{
  TimeoutTackts = timeout;			// Запустим отсчёт таймаута
  while(1){
    while(rx.ptrs[0] == -1 && TimeoutTackts != 0);
    if(TimeoutTackts == 0){                                          // Если вышли из цикла по таймауту, возвращаем 0 
			TimeoutsCount ++;
      return 0;
		}
    if(strncmp((char*)rx.buf+rx.ptrs[0], answer, strlen(answer)) == 0){     // Если получен нужный ответ, возвращаем 1
      dropMessage();
      return 1;                                                
    }
    dropMessage();		// "неожиданные" сообщения теряются
  } 
  return 0;
}
//----------------------------------------------------------------
void waitDropOK(void)
{
	TimeoutTackts = 100;			// Запустим отсчёт таймаута (10сек)
	while(1){
		while(rx.ptrs[0] == -1 && TimeoutTackts != 0);
		if(TimeoutTackts == 0){                                          // Если вышли из цикла по таймауту, возвращаем 0
			TimeoutsCount ++;
			return;
		}
		if(strncmp((char*)rx.buf+rx.ptrs[0], "OK", 3) == 0){     // Если получен ответ ОК
			dropMessage();
			return;
		}
		dropMessage();		// "неожиданные" сообщения теряются
	}
}
//----------------------------------------------------------------
void waitMessage(void)
{
	TimeoutTackts = 100;		// Таймаутожидания 10 сек.
  while(rx.ptrs[0] == -1 && TimeoutTackts != 0);
	if(TimeoutTackts == 0) TimeoutsCount ++;
}
//----------------------------------------------------------------
void CheckIncomingMessages(void)
{
																	   // +CLIP: "+78312330158",145,"",,"",0
	if(strncmp((char*)rx.buf+rx.ptrs[0], "+CLIP: \"+7", 10) == 0){	// Входящий звонок. Звонки с нероссийских номеров игнорируются
		if(strncmp((char*)rx.buf+rx.ptrs[0]+10, options.OperatorTel, 10) == 0 || strncmp((char*)rx.buf+rx.ptrs[0]+10, options.AdminTel, 10) == 0)
		{
			SilentLeft = 1;	// Срочно организовать сеанс связи с сервером
		}
		dropMessage();				// Отбросим "+CLIP: "+78312330158",145,"",,"",0"
		uart_send("ATH0");		// Сбросим вызов
		waitMessage();dropMessage();        // Выбрасываем эхо
		waitDropOK();												// Выбрасываем ОК
		return;
	}
	
	if(strncmp((char*)rx.buf+rx.ptrs[0], "+CMTI:", 6) == 0){	// Входящее СМС
		uart_send("AT+CMGD=4");			// Удаляем вообще все сообщения
		waitMessage();dropMessage();        // Выбрасываем эхо
		waitDropOK();												// Выбрасываем ОК
	}
	
	dropMessage();		// Все остальные сообщения просто отбрасываем
}
//----------------------------------------------------------------
void dropMessage(void)
{
	volatile uint8_t i;
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
		RecToHistory(EVENT_NEW_LOCAL_SETTINGS);
		settingsWasChanged = 0;
	}
}
//---------------------------------------------------------------------
void SaveHistoryToEEPROM(void)         // Сохраняет весь массив истории в EEPROM
{
  eeprom_write_word((void*)sizeof(struct TSettings), SOFT_RESET_FLAG);  //Запишем сразу после настроек число 0xAAAA, означающее, что дальше идет действительнаяистория
	eeprom_write_block(&History, (void*)sizeof(struct TSettings)+2, sizeof(History)); // После флага запишем всю историю
}
//---------------------------------------------------------------------
void PumpStop(void)
{
  options.PumpTimeLeft = options.PumpRelaxDuration;
  options.PumpWorkFlag = 0;
  PORTC &= 0b11101111;			// вЫключим насос (он на PC4)
  if(State.PumpPause < 1) State.PumpPause = PUMP_RESTART_PAUSE;
}
//---------------------------------------------------------------------
uint8_t PumpStart(void)
{
  if(options.PumpWorkDuration == 0  || options.PumpRelaxDuration == 0){strcpy(strD, "Нет расписания"); return EVENT_PUMP_FAIL_NO_SCHEDULE;}
  if(options.FreezeFlag == 1){strcpy(strD, "Возм.заморозка"); return EVENT_PUMP_FAIL_FREEZE;}
  if(State.PumpPause > 0){ strcpy(strD, "Пауза "); itoa(State.PumpPause, buf, 10); strcat(strD, buf); strcat(strD, " сек   ");return EVENT_PUMP_FAIL_NO_AC;}
	if(State.PowerFailFlag == 1){ strcpy(strD, "ОтсутстЭлектич"); return EVENT_PUMP_FAIL_NO_AC;}
  options.PumpTimeLeft = options.PumpWorkDuration;
  CheckUPause = 20;		// 2 секунды не проверять питающее напряжение!
  options.PumpWorkFlag = 1;
  PORTC |= 0b00010000;	// Включим насос (он на PC4)
  return EVENT_NONE;
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
      strcpyPM(buf, MSG_Pump); buf[11] = 0x80; buf[12] = 0x81;
      if(MenuMode == MD_DIRHEATER){strcpyPM(buf, MSG_Heater); buf[11] = 0xA8; buf[12] = 0xA8;}
      LCD_gotoXY(0, 0);LCD_writeStringInv(buf);
      LCD_gotoXY(0, 1);	LCD_writePMstring(MSG_RightNow);
      LCD_gotoXY(0, 2);	strcpyPM(buf, MSG_On); buf[4] = 0xAB; buf[9] = 0xAB; LCD_writeString(buf);
      LCD_gotoXY(0, 3);	LCD_writeString("              ");
      LCD_gotoXY(0, 4);	strcpyPM(buf, MSG_Off); buf[4] = 0xAC; buf[9] = 0xAC; LCD_writeString(buf);
      break;
    }
    case MD_PUMPWORKTIME:
    case MD_PUMPRELAXTIME:
    {
      x = 3; if(MenuMode == MD_PUMPRELAXTIME) x = 10;
      LCD_gotoXY(0, 0); LCD_writePMstringInv(MSG_PumpSchedule); // Расписание насоса
      LCD_gotoXY(0, 1); LCD_writePMstring(MSG_WorkRelax);		// Работает/стоит
      LCD_gotoXY(0, 2); strcpyPM(buf, MSG_Blank); buf[x] = 0xAB; LCD_writeString(buf);
      LCD_gotoXY(0, 3); MinToStr(options.PumpWorkDuration, buf+1); strcat(buf, "  ");MinToStr(options.PumpRelaxDuration, strD);strcat(buf, strD);LCD_writeString(buf);
      LCD_gotoXY(0, 4);	strcpyPM(buf, MSG_Blank); buf[x] = 0xAC; LCD_writeString(buf);
      break;
    }
    case MD_MIN_T:
    case MD_MAX_T:
    {
      x = 3; if(MenuMode == MD_MAX_T) x = 10;
      LCD_gotoXY(0, 0); LCD_writePMstringInv(MSG_HeaterSchedul);
      LCD_gotoXY(0, 1);	LCD_writePMstring(MSG_OnOff);
      LCD_gotoXY(0, 2);	strcpyPM(buf, MSG_Blank); buf[x] = 0xAB; LCD_writeString(buf);
      itoa(options.HeaterOnTemp, buf+2, 10); strcpy(strD, "*C    "); strD[0] = 0xBF; strcat(buf, strD); itoa(options.HeaterOffTemp, strD, 10); strcat(buf, strD); strcpy(strD, "*C   ");strD[0] = 0xBF; strcat(buf, strD);
      LCD_gotoXY(0, 3);	LCD_writeString(buf);
      LCD_gotoXY(0, 4);	strcpyPM(buf, MSG_Blank); buf[x] = 0xAC; LCD_writeString(buf);
      break;
    }
    case MD_STAT: { ShowStat(); break;}
    case MD_CLEAR:
    {
      LCD_gotoXY(0, 0); LCD_writePMstringInv(MSG_Reset);
      LCD_gotoXY(0, 1); LCD_writePMstringInv(MSG_Blank);
      LCD_gotoXY(0, 2);	strcpyPM(buf, MSG_ToSettings); buf[1] = 0xAB; buf[12] = 0xAB; LCD_writeString(buf);
      LCD_gotoXY(0, 3);	LCD_writePMstring(MSG_ToReset);
      LCD_gotoXY(0, 4);	strcpyPM(buf, MSG_ToStat); buf[1] = 0xAC; buf[12] = 0xAC; LCD_writeString(buf);
      break;
    }
		case MD_DEBUG:
		{
      ShowHistory();
      /*
			LCD_gotoXY(0, 0);LCD_writePMstringInv(MSG_Info);
			itoa(State.balance, buf, 10); strcat(buf, " R, "); itoa(State.Vbat, strD, 10); strcat(buf, strD); strcat(buf, "V");
			LCD_gotoXY(0, 1);LCD_writeString(buf);
*/
			strcpy(buf, "N");
			itoa(Now.MM, strD, 10); strcat(buf, strD);itoa(Now.dd, strD, 10); strcat(buf, strD); strcat(buf, " ");
			itoa(Now.hh, strD, 10); strcat(buf, strD); strcat(buf, ":"); itoa(Now.mm, strD, 10); strcat(buf, strD); strcat(buf, ":"); itoa(Now.ss, strD, 10); strcat(buf, strD);
			LCD_gotoXY(0, 3);LCD_writeString(buf);
			strcpy(buf, "R");
			itoa(remoteSettingsTimestamp.MM, strD, 10); strcat(buf, strD); itoa(remoteSettingsTimestamp.dd, strD, 10); strcat(buf, strD); strcat(buf, " ");
			itoa(remoteSettingsTimestamp.hh, strD, 10); strcat(buf, strD); strcat(buf, ":"); itoa(remoteSettingsTimestamp.mm, strD, 10); strcat(buf, strD); strcat(buf, ":"); itoa(remoteSettingsTimestamp.ss, strD, 10); strcat(buf, strD);
			LCD_gotoXY(0, 4);LCD_writeString(buf);
			strcpy(buf, "L");
			itoa(options.localSettingsTimestamp.MM, strD, 10); strcat(buf, strD); itoa(options.localSettingsTimestamp.dd, strD, 10); strcat(buf, strD); strcat(buf, " ");
			itoa(options.localSettingsTimestamp.hh, strD, 10); strcat(buf, strD); strcat(buf, ":"); itoa(options.localSettingsTimestamp.mm, strD, 10); strcat(buf, strD); strcat(buf, ":"); itoa(options.localSettingsTimestamp.ss, strD, 10); strcat(buf, strD);
			LCD_gotoXY(0, 5);LCD_writeString(buf);
			
		}
  }
}
//---------------------------------------------------------------------
void ShowStat(void)
{
  LCD_clear();
  LCD_gotoXY(0, 0); LCD_writePMstringInv(MSG_Stat);		// Отобразим заголовок

  itoa(State.Temp/16, buf, 10);
  strcpyPM(strD, MSG_Celsium); strD[0] = 0xBF;
  if( PORTC & 0b00000100 ){strD[4] = 0xA8;strD[5] = 0xA8;}
  if( PORTC & 0b00010000 ){strD[7] = 0x80;strD[8] = 0x81;}
  strcat(buf, strD);
  LCD_gotoXY(0, 1);	 LCD_writeString(buf);					// Отобразим температуру и состояние обогревателя и насоса

  if(options.PumpWorkDuration == 0 || options.PumpRelaxDuration == 0)
  strcpyPM(buf, MSG_TimerOff);
  else
  {
    MinToStr(options.PumpTimeLeft, buf);
    strcat(buf, " до ");
    options.PumpWorkFlag == 1 ? strcpy(strD, "вЫк       ") : strcpy(strD, "вкл      ");
    strD[3] = 0x80; strD[4] = 0x81;
    strcat(buf, strD);
  }
  if(State.PumpPause != 0 && options.PumpWorkFlag == 1)
  {
    strcpy(buf, "Пауза ");
    itoa(State.PumpPause, strD, 10);
    strcat(buf, strD);
    strcat(buf, " сек.   ");
  }
  if(options.FreezeFlag == 1){
		strcpyPM(buf, MSG_Freezing);
	}
	LCD_gotoXY(0, 2);	
	if(State.PowerFailFlag == 0) LCD_writeString(buf);					// Отобразим "время до" или предупреждение насоса если есть
	else LCD_writeString("Сбой элект-ния");

	itoa(State.balance, buf, 10); strcat(buf, "p "); itoa(State.Vbat/2, strD, 10); strcat(buf, strD); strcat(buf, "v ");
	if(SilentLeft > 180){
		itoa(SilentLeft/60, strD, 10); strcat(buf, strD); strcat(buf, "m");
	}
	else if(SilentLeft >= 0) {
		itoa(SilentLeft, strD, 10); strcat(buf, strD); strcat(buf, "s");
	}
	else strcat(buf, "звон");
	LCD_gotoXY(0, 3);LCD_writeString(buf);					// Отобразим баланс, напряжение батареи, время до след сеанса связи

	//			volatile uint16_t i = 0;
	//			for(i = 0; rx.ptrs[i] != -1 && i < RXBUFSTRCOUNT; i ++);
	//			strcpy(buf, "p"); itoa(i, strD, 10);strcat(buf, strD);
	//			strcat(buf, " w"); itoa(rx.wptr, strD, 10);strcat(buf, strD);
	//			strcat(buf, " b");itoa(rx.buf_overflow_count, strD, 10);strcat(buf, strD);
	strcpy(buf, "bo");itoa(rx.buf_overflow_count, strD, 10);strcat(buf, strD);
	strcat(buf, " po");itoa(rx.ptrs_overflow_count, strD, 10);strcat(buf, strD);
	strcat(buf, " tc");itoa(TimeoutsCount, strD, 10);strcat(buf, strD);
	LCD_gotoXY(0, 4); LCD_writeString(buf);

/*	strcpy(buf, "SIM900Stat ");
	itoa(SIM900Status, strD, 10);
	strcat(buf, strD);
	LCD_gotoXY(0, 4); LCD_writeString(buf);
*/
	uint8_t hh, dd = Now.dd;
	hh = Now.hh + 3;
	if(hh > 23) {hh -= 24; dd ++;}
	itoa(Now.yy, buf, 10); itoa(Now.MM, strD, 10); strcat(buf, strD); itoa(dd, strD, 10); strcat(buf, strD); strcat(buf, " ");
	itoa(hh, strD, 10); strcat(buf, strD); strcat(buf, ":"); itoa(Now.mm, strD, 10); strcat(buf, strD); strcat(buf, ":"); itoa(Now.ss, strD, 10); strcat(buf, strD);
	LCD_gotoXY(0, 5);LCD_writeString(buf);				// Отобразим текущее время

	

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
void MinToStr(unsigned int Min, char *str)		// Переводит число минут в строку типа HH:MM
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
void measureBattery(void)
{
	ADMUX  = 0b11000000;		  // 11 - Опорное напряжение = 2,56В, 0 - выравнивание вправо, 0 - резерв, 0 - резерв, 000 - выбор канала ADC0
	ADCSRA |= 1<<ADSC;		    // Старт преобразования
	while (ADCSRA & 0x40);		// Ждем завершения(сброса флага ADSC в 0)
	uint32_t vLongBat = ADC;
	vLongBat *= 1026;					// Коррекция измерения
	State.Vbat = vLongBat / 1000;         // Напряжение = ADC * 200
}
//---------------------------------------------------------------------
void strcpyPM(char *dest, const char *PMsrc)		// Копирует строку из PROGMEM в dest;
{
	char lastChar;
	uint16_t index = 0;
	do{
		lastChar = pgm_read_byte(PMsrc + index);
		*(dest + index) = lastChar;
		index ++;
	}while(lastChar != '\0');
	
}
//---------------------------------------------------------------------
void OnPowerFail(void)
{
	LightLeft = LIGHT_TIME;			// Включим подсветку сразу показать, что не висит
	LIGHT_ON;
	State.PowerFailFlag = 1;
	if(options.PumpWorkFlag == 1){
		//options.PumpWorkFlag = 0;  // Выключим насос, но флаг работы оставим, чтобы знать, что после восст питания его нужно включить
		PORTC &= 0b11101111;			// вЫключим насос (он на PC4)
		if(State.PumpPause < 1) State.PumpPause = PUMP_RESTART_PAUSE;
	}
	if(State.PowerFailSecCount == 0)
	{
		RecToHistory(EVENT_AC_FAIL);	// Чтобы не было повторных записей при частых отключениях
		Notifications.PowerFail = 1;
	}
	State.PowerFailSecCount = UNSTABLE_POWER_DELAY;
}
//---------------------------------------------------------------------
void OnPowerRestore(void)
{
	State.PowerFailFlag = 0;	
	RecToHistory(EVENT_AC_RESTORE);
	Notifications.PowerRestore = 1;
}
//---------------------------------------------------------------------
void RecToHistory(uint8_t eventCode)
{
	History[historyPtr].EventTime.yy = Now.yy;
	History[historyPtr].EventTime.MM = Now.MM;
	History[historyPtr].EventTime.dd = Now.dd;
	History[historyPtr].EventTime.hh = Now.hh;
	History[historyPtr].EventTime.mm = Now.mm;
	History[historyPtr].EventTime.ss = Now.ss;
	History[historyPtr].EventCode = eventCode;
	historyPtr ++;
	if(historyPtr == HISTORYLENGTH) historyPtr = 0; // При заполнении буфера истории просто начинаем заполнять его с начала
}

//  options.PumpTimeLeft = options.PumpRelaxDuration;
//---------------------------------------------------------------------
void CheckNotifications(void)
{
	if(Notifications.Frost == 1)
	{
		if(options.fFrostNotifications & 0b00001100) // Смс Оператору или админу
		{
			strcpyPM(buf, MSG_Freezing);
			if(options.fFrostNotifications & 0b00001000) // Смс Оператору
			SIM900_SendSMS(options.OperatorTel, buf);
			if(options.fFrostNotifications & 0b00000100) // Смс Админу
			SIM900_SendSMS(options.AdminTel, buf);
		}
		if(options.fFrostNotifications & 0b00000010) // Звонок Оператору
		SIM900_Call(options.OperatorTel);
		if(options.fFrostNotifications & 0b00000001) // Звонок Админу
		SIM900_Call(options.AdminTel);
		
		Notifications.Frost = -1;
	}
	
	if(Notifications.Warm == 1)
	{
		if(options.fWarmNotifications & 0b00001100) // Смс Оператору или админу
		{
			strcpyPM(buf, MSG_Warming);
			if(options.fWarmNotifications & 0b00001000) // Смс Оператору
			SIM900_SendSMS(options.OperatorTel, buf);
			if(options.fWarmNotifications & 0b00000100) // Смс Админу
			SIM900_SendSMS(options.AdminTel, buf);
		}
		if(options.fWarmNotifications & 0b00000010) // Звонок Оператору
		SIM900_Call(options.OperatorTel);
		if(options.fWarmNotifications & 0b00000001) // Звонок Админу
		SIM900_Call(options.AdminTel);
		
		Notifications.Warm = -1;
	}
	
	if(Notifications.PowerFail == 1)
	{
		if(options.fPowerNotifications & 0b00001100) // Смс Оператору или админу
		{
			strcpyPM(buf, MSG_PowerLost);
			if(options.fPowerNotifications & 0b00001000) // Смс Оператору
			SIM900_SendSMS(options.OperatorTel, buf);
			if(options.fPowerNotifications & 0b00000100) // Смс Админу
			SIM900_SendSMS(options.AdminTel, buf);
		}
		if(options.fPowerNotifications & 0b00000010) // Звонок Оператору
		SIM900_Call(options.OperatorTel);
		if(options.fPowerNotifications & 0b00000001) // Звонок Админу
		SIM900_Call(options.AdminTel);
		
		Notifications.PowerFail = -1;
	}

	if(Notifications.PowerRestore == 1)
	{
		if(options.fPowerRestNotifications & 0b00001100) // Смс Оператору или админу
		{
			strcpyPM(buf, MSG_PowerRestore);
			if(options.fPowerRestNotifications & 0b00001000) // Смс Оператору
			SIM900_SendSMS(options.OperatorTel, buf);
			if(options.fPowerRestNotifications & 0b00000100) // Смс Админу
			SIM900_SendSMS(options.AdminTel, buf);
		}
		if(options.fPowerRestNotifications & 0b00000010) // Звонок Оператору
		SIM900_Call(options.OperatorTel);
		if(options.fPowerRestNotifications & 0b00000001) // Звонок Админу
		SIM900_Call(options.AdminTel);
		
		Notifications.PowerRestore = -1;
	}

	if(Notifications.Balance == 1)
	{
		if(options.fBalanceNotifications & 0b00001100) // Смс Оператору или админу
		{
			strcpyPM(buf, MSG_BalanceLow);
			if(options.fBalanceNotifications & 0b00001000) // Смс Оператору
			SIM900_SendSMS(options.OperatorTel, buf);
			if(options.fBalanceNotifications & 0b00000100) // Смс Админу
			SIM900_SendSMS(options.AdminTel, buf);
		}
		if(options.fBalanceNotifications & 0b00000010) // Звонок Оператору
		SIM900_Call(options.OperatorTel);
		if(options.fBalanceNotifications & 0b00000001) // Звонок Админу
		SIM900_Call(options.AdminTel);
		
		Notifications.Balance = -1;
	}
}

//----------------------------------------------------------------
void ShowHistory(void)                     // Отобразим на дисплее историю
{
  LCD_clear();
  uint16_t ptr, count = 0, y = 0;
  strcpy(strD, "");
  for(ptr = 0; History[ptr].EventCode == EVENT_NONE && ptr < HISTORYLENGTH; ptr ++);  // Ищем первое непустое событие
  if(ptr == HISTORYLENGTH){
    LCD_gotoXY(0, y);
    LCD_writeString("История пуста");
    return; // В истории нет ни одного события
  }  
    
  for(; History[ptr].EventCode != EVENT_NONE && ptr < HISTORYLENGTH; ptr ++){
    itoa(History[ptr].EventCode, buf, 10);
    strcat(strD, buf);
    strcat(strD, " ");
    if(count > 1 && count%4 == 0){
      LCD_gotoXY(0, y);
      LCD_writeString(strD);
      y ++;
      count = 0;
      strcpy(strD, "");
    }
    else count ++;
  }
	LCD_gotoXY(0, y);
	LCD_writeString(strD);
}