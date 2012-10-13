/*
 * main.c
 * V2.1.1
 * Christopher Berg
 * Compatible with V1.1 & V1.0 of the IV-18 Booster Pack
 *
 * 9/16/12 - adding alarm functionality
 */
#include "msp430.h"
#include "font.h"
#include "time.h"
#include "main.h"  //look at this file for pin definitions, etc
#include "one_wire.h"




void main(void) {
	WDTCTL = WDTPW + WDTHOLD;                 // Stop watchdog timer

	//initial values for time - incrementing the year would take a long time :)
	time.year = 0x2012;
	time.mday = 0x01;

	//setting up cloks - 16mhz, using external crystal
	BCSCTL2 = SELM_0 + DIVM_0 + DIVS_2;
	BCSCTL1 = CALBC1_16MHZ; // 16MHz clock
	DCOCTL = CALDCO_16MHZ;
	BCSCTL1 |= XT2OFF + DIVA_3;
	BCSCTL3 = XT2S_0 + LFXT1S_0 + XCAP_3;

	P1DIR |= BIT0; //enable use of LED

	VFD_BLANK_ON; //blank display for startup
	initSPI();
	initDisplayTimer();
	initClockTimer();
	initUART();
	initPeripherals();
	write(0x00,0x00);
	_delay_cycles(10000); //wait for boost to bring up voltage
	VFD_BLANK_OFF;//starting writes, turn display on

	//serial_setup(2, 1000000 / 9600);
	one_wire_setup(&P1DIR, &P1IN, ONEWIRE_PIN, 16);
	owex(0, 0); // Reset device
	unsigned char b[16];


	//Testing alarms - alarm on startup
	alarms_enabled = 1;
	alarms[0].daysOfWeek = 0xFF;
	alarms[0].hour = 0;
	alarms[0].min = 1;

	switchMode(0);//set up time display

	displayORString("startup",7,8);

	__bis_SR_register(LPM0_bits | GIE);       //enable interrupts

	while(1)
		{
		//check to see if alarm is on
			while (alarm_duration > 0 && alarm_snooze == 0)
			{
				BUZ_ON; //toggle pin
				_delay_cycles(2500);
				BUZ_OFF;
				_delay_cycles(2500);
			}
			BUZ_OFF;
			if(take_temp)
			{
				owex(0, 0); owex(0x44CC, 16);						// Convert
				//I tried to drop into low poer for a bit, but it didn't work - not sure why
				__delay_cycles(800000);								// Wait for conversion to complete
				owex(0, 0); owex(0xBECC, 16);						// Read temperature
				read_block(b, 9);
				temp_c = (b[1] << 8)| b[0];
				temp_f = temp_c * 9 / 5 + (512);
				take_temp = 0;
			}
			__bis_SR_register(LPM0_bits | GIE);       //enable interrupts, low power
		}
}

//Sets refresh rate for display
void initDisplayTimer()
{
	TA0CCTL0 = CM_0 + CCIS_0 + OUTMOD_0 + CCIE;
	TA0CTL = TASSEL_2 + ID_3 + MC_1;
	TA0CCR0 = 1000; //you can change the refresh rate by changing this value
}

/*
 * Initializes the interrupt for the RTC functionality and button debouncing
 */
void initClockTimer()
{
	//we are using the external crystal for accuracy

	//Init timer
	TA1CCTL0 = CM_0 + CCIS_0 + OUTMOD_0 + CCIE;
	//TA1CCR0 = 1023;
	TA1CCR0 = 255;
	TA1CTL = TASSEL_1 + ID_2 + MC_1;
}

//Sets up 9600 baud serial connection
//Can be used for the LP's USB/Serial connection, bluetooth serial or GPS module
void initUART()
{
	P1SEL |= BIT1 + BIT2 ;                     // P1.1 = RXD, P1.2=TXD
	P1SEL2 |= BIT1 + BIT2 ;                     // P1.1 = RXD, P1.2=TXD
	UCA0CTL1 |= UCSWRST;
	UCA0CTL1 = UCSSEL_2 + UCSWRST;
	UCA0MCTL = UCBRF_0 + UCBRS_6;
	UCA0BR0 = 160;
	UCA0BR1 = 1;
	UCA0CTL1 &= ~UCSWRST;
	IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt
}

/*
 * Initializes SPI
 * This is used to communicate with the MAX6921 chip
 */
void initSPI()
{

	P2DIR |= VFD_BLANK_PIN | VFD_CS_PIN;
	VFD_BLANK_OFF;
	VFD_DESELECT;

	P1SEL |= SCL_PIN + SDA_PIN;
	P1SEL2 |= SCL_PIN + SDA_PIN;

	UCB0CTL0 |= UCCKPH + UCMSB + UCMST + UCSYNC; // 3-pin, 8-bit SPI master
	UCB0CTL1 |= UCSSEL_2; // SMCLK
	UCB0BR0 |= 0x01; // div/1
	UCB0BR1 = 0;
	UCB0CTL1 &= ~UCSWRST; // Initialize


	//IE2 |= UCB0RXIE;                          // Enable RX interrupt
	_delay_cycles(5000);
}

//Configure interrupts on buttons
void initPeripherals()
{
	P2DIR &= ~(S1_PIN|S2_PIN|S3_PIN);
	P2REN |= (S1_PIN|S2_PIN|S3_PIN);
	P1OUT &= ~BUZ_PIN;
	P1DIR |= BUZ_PIN;
}

/*
 * Outputs a character to the VFD - one digit at a time
 * Used by Interrupt
 * The device accepts 20 bits, send 24 - first 4 bits are discarded
 */
void write(char display, char digit) {
	//VFD_BLANK_ON;
	VFD_SELECT;

	char send1, send2;
	if (digit == 8)
		{
			send1 = 0x00;
			send2 = 1<<3;
		}
	else
	{
		send1 = 1<<(7 -digit);
		send2 = 0x00;
	}
	UCB0TXBUF = display>>4; //send first 4 bits
	while (!(IFG2 & UCB0TXIFG)); //wait for send to complete
	UCB0TXBUF = (display<<4) | (send1>>4); //next 8 bits
	while (!(IFG2 & UCB0TXIFG)); //wait for send to complete
	UCB0TXBUF = (send1<<4) | send2; //last 8 bits
	while (!(IFG2 & UCB0TXIFG)); //wait for send to complete
	VFD_DESELECT;
	//VFD_BLANK_OFF;
}

/*
 * Writes the day of week out to the display
 */
void displayWeekday(unsigned int wday)
{
	switch(wday)
		{
			case 0x0:
				displayString("Sun",3);
				break;
			case 0x1:
				displayString("Mon",3);
				break;
			case 0x2:
				displayString("Tue",3);
				break;
			case 0x3:
				displayString("Wed",3);
				break;
			case 0x4:
				displayString("Thu",3);
				break;
			case 0x5:
				displayString("Fri",3);
				break;
			case 0x6:
				displayString("Sat",3);
				break;
			default:
				displayString("err",3);
				break;
		}
}

/*
 * Displays the current time for display
 */
void displayTime(struct btm *t)
{
	if (hourMode == 1) //subtract 12 before displaying hour
	{
		if (t->hour == 0x12)
		{
			screen[1] = numbertable[(t->hour & 0xF0)>>4];
			screen[2] = numbertable[(t->hour & 0x0F)];
			//screen[3] = alphatable[15]; //PM
			screen[0] |= BIT0;//turn on dot
		}
		else if (t->hour == 0)
		{
			screen[1] = numbertable[1];
			screen[2] = numbertable[2];
			//screen[3] = alphatable[0]; //AM
			screen[0] &= ~BIT0;//turn off dot
		}
		else if (t->hour > 0x12)
		{
			unsigned int localHour = 0;
			localHour = _bcd_add_short(t->hour, 0x9988);
			screen[1] = numbertable[(localHour & 0xF0)>>4];
			screen[2] = numbertable[(localHour & 0x0F)];
			//screen[3] = alphatable[15]; //PM
			screen[0] |= BIT0;//turn on dot
		}
		else
		{
			screen[1] = numbertable[(t->hour & 0xF0)>>4];
			screen[2] = numbertable[(t->hour & 0x0F)];
			//screen[3] = alphatable[0]; //AM
			screen[0] &= ~BIT0;//turn off dot
		}
	}
	else
	{
		screen[1] = numbertable[(t->hour & 0xF0)>>4];
		screen[2] = numbertable[(t->hour & 0x0F)];
		screen[0] &= ~BIT0;//turn off dot
	}

	screen[4] = numbertable[(t->min & 0xF0)>>4];
	screen[5] = numbertable[(t->min & 0x0F)];
	screen[7] = numbertable[(t->sec & 0xF0)>>4];
	screen[8] = numbertable[(t->sec & 0x0F)];
}

/*
 * If anything special needs to happen between modes, it should be added here
 * Not really being used yet, but leaving in case it's needed
 */
void switchMode(char newMode)
{
	bufferPlace = 0;
	DisplayMode = newMode;
	if (DisplayMode == ModeAlarm)
		Alarm_DisplayAlarms();
}

/*
 * Blanks the display array
 * Clears out display specified by parameter
 */
void clearDisplay(char buffer)
{
	if(buffer==0)
	{
		screen[0] = 0;
		screen[1] = 0;
		screen[2] = 0;
		screen[3] = 0;
		screen[4] = 0;
		screen[5] = 0;
		screen[6] = 0;
		screen[7] = 0;
		screen[8] = 0;
	}
	else if (buffer ==1)
	{
		screenOR[0] = 0;
		screenOR[1] = 0;
		screenOR[2] = 0;
		screenOR[3] = 0;
		screenOR[4] = 0;
		screenOR[5] = 0;
		screenOR[6] = 0;
		screenOR[7] = 0;
		screenOR[8] = 0;
	}
}

/*
 *  Screen refresh timer interrupt
 */
#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void)
{
	static char digit = 0;
	char toDisplay;
	if(overrideTime > 0)
		toDisplay = screenOR[digit];
	else
		toDisplay = screen[digit];

	write(toDisplay, digit);

	digit++;
	if (digit > 8)
	{
		digit = 0;
	}
}

/*
 * Serial RX interrupt
 *
 */
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
	static char place = 1;
	static char serialBuffer[] ={0,0,0,0,0,0,0};
	char waitMode = 1; //waiting for mode input

	char rx = UCA0RXBUF;
	if (waitMode == 1)
	{
		if (rx == 'T')
			switchMode(0);
		if (rx == 'M')
			switchMode(1);
		waitMode = 0;
	}
	else if (DisplayMode == 0)
	{
		serialBuffer[bufferPlace] = rx;
		if (bufferPlace == 5 || rx == 13)
		{
			//buffer filled up - process command
			unsigned int hour, min;
			hour = (serialBuffer[0]-48) << 4;
			hour |= (serialBuffer[1] - 48);
			min = (serialBuffer[2]-48) << 4;
			min |= (serialBuffer[3] - 48);
			time.hour = hour;
			time.min = min;

			  while (!(IFG2&UCA0TXIFG));
				  UCA0TXBUF = 'S';                    // respond as saved


			serialBuffer[0] = 0;
			serialBuffer[1] = 0;
			serialBuffer[2] = 0;
			serialBuffer[3] = 0;
			serialBuffer[4] = 0;
			bufferPlace = 0;
			waitMode = 1;
		}
		else
			bufferPlace++;
	}
	else if (DisplayMode == 1)
	{
		if (rx >= 97 & rx <= 122) //lowercase letters
			screen[place] = alphatable[rx - 97];
		else if (rx >= 65 & rx <= 90) //uppercase letters - use same table
			screen[place] = alphatable[rx - 65];
		else if (rx >= 48 & rx <= 57) //numbers
			screen[place] = numbertable[rx - 48];
		else if (rx == 32) //space
			screen[place] = 0x00;
		else if (rx == 46) //period
		{
			if (place > 1) //back up if we haven't wrapped to add a period to the previous character
				place--;
			screen[place] |= 1;
		}

		place++;
		if (place > 8)
			place = 1;
	}
}

/*
 * one quarter second timer interrupt (250ms)
 * used for RTC functions, button reading and debouncing and alarm checks
 */
#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_ISR(void)
{
	static unsigned int last_min = 99;
	if (overrideTime > 0)
		overrideTime--;
	static char count = 0; //how many intervals - 4 = 1s


	count++;
	if (count == 4)
	{
		rtc_tick_bcd(&time);
		if (DisplayMode == ModeTime & (settings_mode == 0 | (settings_mode == 1 & (setting_place >= 1 & setting_place <= 3))))
		{
			displayTime(&time);


			if (settings_mode == 1)
			{
				//set dots based on current setting location
				switch(setting_place)
				{
					case 1:
						screen[1] |= 1;
						screen[2] |= 1;
						break;
					case 2:
						screen[4] |= 1;
						screen[5] |= 1;
						break;
					case 3:
						screen[7] |= 1;
						screen[8] |= 1;
						break;
				}
			}
			else //toggle dots only when not in settings mode
			{
				screen[3] ^= 1;//toggle dots off and on every second
				screen[6] ^= 1;//toggle dots off and on every second
			}
		}
		else if (DisplayMode == ModeAlarm & settings_mode == 1)
		{
			if (setting_place == 0)
				screen[alarm_index + 1] ^= numbertable[alarm_index+1];
		}
		if(alarm_duration == 0) //don't need to check if alarm is already on
		{
			alarm_snooze = 0;
			int alm_num;
			for (alm_num=0;alm_num < NUM_ALARMS; alm_num++)
			{
				if (1<<alm_num & alarms_enabled) //check to see if alarm is on
					if(check_alarm(&time,&alarms[alm_num]))
					{
						alarm_duration = ALARM_TIME; //turn alarm on
						displayString("alarm",2);
						LPM0_EXIT;
					}
			}
		}
		else
		{
			alarm_duration--;

			LPM0_EXIT;
		}
		count = 0;
	}

	static char S1_Time, S2_Time, S3_Time = 0;

	if ((P2IN & S1_PIN) == 0) //Mode
	{
		S1_Time++;
		alarm_off();
	}
	else
	{
		if(S1_Time > 1)
		{
			if (settings_mode == 0) //change mode
			{
				char NextMode = DisplayMode + 1;
				if(NextMode > ModeMax)
					NextMode = 0;
				clearDisplay(0);
				switchMode(NextMode);
				settings_mode = 0;
			}
			else //in settings mode, this becomes decrement
			{
				if(DisplayMode == ModeTime)
					Time_ChangeValue(0);
				else if (DisplayMode == ModeAlarm)
					Alarm_ChangeValue(0);
			}
		}
		S1_Time = 0;
	}
	if ((P2IN & S2_PIN) == 0) //Set
	{
		S2_Time++;
		alarm_off();
	}
	else
	{
		if (S2_Time > 6) //a nice long press will drop us in or out of settings mode
		{
			if (settings_mode == 0)
			{
				settings_mode = 1;
				setting_place = 0;//move to the first setting
				alarm_index = 99;
				if (DisplayMode == ModeAlarm)
					Alarm_InitSettings();
			}
			else if (settings_mode == 1) //exit settings mode
			{
				clearDisplay(0);
				settings_mode = 0;
				if(DisplayMode == ModeAlarm)
					Alarm_DisplayAlarms();
			}
		}
		else if (settings_mode == 1 & S2_Time > 1) //short press toggles through settings
		{

		}
		else if (settings_mode == 0 & S2_Time > 1) //short press, normal mode
		{
			if (DisplayMode == ModeTime)
			{
				//2sec - display date
				displayDate(&time, 1);
				overrideTime = 8;
			}
		}
		if (settings_mode == 1 & S2_Time > 1) //in settings mode and a button was pressed
		{
			//go through settings place by mode
			if (DisplayMode == ModeTime)
			{
				setting_place++;
				Time_ChangeSetting();
			}
			else if (DisplayMode == ModeAlarm)
			{
				Alarm_ChangeSetting();
			}
		}
		S2_Time = 0;
	}
	if ((P2IN & S3_PIN) == 0) //Value
	{
		S3_Time++;
		alarm_off();
	}
	if ((P2IN & S3_PIN) != 0 | (S3_Time > 0 & allow_repeat == 1)) //using an if here to allow holding down the button
	{
		if (settings_mode == 1 & S3_Time > 0)
		{
			if(DisplayMode == ModeTime)
			{
				Time_ChangeValue(1);
			}
			else if (DisplayMode == ModeAlarm)
			{
				Alarm_ChangeValue(1);
			}
		}
		else if (settings_mode == 0 & S3_Time > 0)
		{
			if(DisplayMode == ModeTime)
				display_temp(temp_f,1);
		}


		S3_Time = 0;
	}

	if (time.min != last_min)
	{
		last_min = time.min;
		take_temp = 1;
		LPM0_EXIT; //exit to take temperature
	}
}
/*
	 * Different setting places by mode
	 * Alarm Mode
	 * 		0 = Alarms enable/disable
	 * 		1 = Hour
	 * 		2 = Minute
	 * 		3 = Sunday
	 * 		4 = Monday
	 * 		5 = Tuesday
	 * 		6 = Wednesday
	 * 		7 = Thursday
	 * 		8 = Friday
	 * 		9 = Saturday
	 */
void Alarm_ChangeSetting()
{
	if(setting_place == 0)
	{
		if (alarm_index >= NUM_ALARMS)
		{
			alarm_index = 0;
			setting_place++;//move on to next setting now that we've covered the alarm enable
		}
		else
			alarm_index++;

	}
	if(setting_place == 9)
	{
		if(alarm_index >= NUM_ALARMS)
		{
			setting_place = 0;
			alarm_index = 0;
		}
		else
		{
			setting_place == 1;
			alarm_index++;
		}
	}
	allow_repeat = 0;
	switch(setting_place)
	{
		case 0:
			Alarm_DisplayAlarms();
			break;
	}
}
/*
	 * Different setting places by mode
	 * Time Mode
	 * 		0 = AM/PM
	 * 		1 = Hour
	 * 		2 = Minute
	 * 		3 = Second
	 * 		4 = Month
	 * 		5 = Day
	 * 		6 = Year
	 * 		7 = Weekday
	 */
void Time_ChangeSetting()
{
	allow_repeat = 0;
	switch(setting_place)
	{
		case 1: //hour
			clearDisplay(0);
			displayORString("Set Time",8,8);
			allow_repeat = (char)0x1;
			displayTime(&time);
			screen[1] |= 1;
			screen[2] |= 1;
			break;
		case 2: //minute
			clearDisplay(0);
			allow_repeat = 1;
			displayTime(&time);
			screen[4] |= 1;
			screen[5] |= 1;
			break;
		case 3: //seconds
			clearDisplay(0);
			allow_repeat = 1;
			displayTime(&time);
			screen[7] |= 1;
			screen[8] |= 1;
			break;
		case 4: //Month
			clearDisplay(0);
			displayORString("Set Date",8,8);
			displayDate(&time,0);
			screen[1] |= 1;
			screen[2] |= 1;
			allow_repeat = 1;
			break;
		case 5: //day
			clearDisplay(0);
			displayDate(&time,0);
			screen[4] |= 1;
			screen[5] |= 1;
			allow_repeat = 1;
			break;
		case 6: //Year
			clearDisplay(0);
			displayDate(&time,0);
			screen[7] |= 1;
			screen[8] |= 1;
			//allow_repeat = 1;
			break;
		case 7: //Weekday
			clearDisplay(0);
			displayORString("Set DOW",7,8);
			displayWeekday(time.wday);
			//allow_repeat = 1;
			break;
		default: //24/12h - 0 or over last setting
			setting_place = 0; //reset in case setting # went past highest
			clearDisplay(0);
			displayORString("Hr Mode",7,8);
			if (hourMode == 0) //24h
			{
				displayString("   24h", 6);
			}
			else
			{
				displayString("   12h", 6);
			}
			break;
	}
}
void Alarm_ChangeValue(char add)
{
	//TODO: Finish
}
void Time_ChangeValue(char add)
{
	short daysInMonth = dim[time.mon][is_leap_year_bcd(time.year)];
	switch(setting_place)
	{
		case 1://hour
			clearDisplay(0);
			if(add)
			{
				time.hour = _bcd_add_short(time.hour, 0x1);
				if(time.hour > 0x23)
					time.hour = 0;
			}
			else
			{
				time.hour = _bcd_add_short(time.hour, 0x9999);
				if(time.hour > 0x23)
					time.hour = 0x23;
			}
			displayTime(&time);
			screen[1] |= 1;
			screen[2] |= 1;
			break;
		case 2://minute
			clearDisplay(0);
			if(add)
			{
				time.min = _bcd_add_short(time.min, 0x1);
				if (time.min > 0x59)
					time.min = 0;
				time.sec = 0x00;
			}
			else
			{
				time.min = _bcd_add_short(time.min, 0x9999);
				if (time.min > 0x59)
					time.min = 0x59;
				time.sec = 0x00;
			}
			displayTime(&time);
			screen[4] |= 1;
			screen[5] |= 1;
			break;
		case 3://second
			clearDisplay(0);
			if(add)
			{
				time.sec = _bcd_add_short(time.sec, 1);
				if (time.sec > 0x59)
					time.sec = 0;
			}
			else
			{
				time.sec = _bcd_add_short(time.sec, 0x9999);
				if (time.sec > 0x59)
					time.sec = 0x59;
			}
			displayTime(&time);
			screen[7] |= 1;
			screen[8] |= 1;
			break;
		case 4: //Month
			clearDisplay(0);
			if(add)
			{
				time.mon = _bcd_add_short(time.mon, 0x01);
				if (time.mon > 0x11)
					time.mon = 0x0;
			}
			else
			{
				time.mon = _bcd_add_short(time.mon, 0x9999);
				if (time.mon > 0x11)
					time.mon = 0x11;
			}
			displayDate(&time,0);
			screen[1] |= 1;
			screen[2] |= 1;
			break;
		case 5: //day
			clearDisplay(0);
			if(add)
			{
				time.mday = _bcd_add_short(time.mday, 1);   // Increment day of month, check for overflow
				if (time.mday > daysInMonth) //if over last day
					time.mday = 0x01;
			}
			else
			{
				time.mday = _bcd_add_short(time.mday, 0x9999);   // Increment day of month, check for overflow
				if (time.mday == 0) //detect wrap
					time.mday = daysInMonth;
			}
			displayDate(&time,0);
			screen[4] |= 1;
			screen[5] |= 1;
			break;
		case 6: //Year
			clearDisplay(0);
			if(add)
			{
				time.year = _bcd_add_short(time.year,1);
				if (time.year > 0x2099)
					time.year = 0x2000;
			}
			else
			{
				time.year = _bcd_add_short(time.year,0x9999);
				if (time.year < 0x2000)
					time.year = 0x2099;
			}
			displayDate(&time,0);
			screen[7] |= 1;
			screen[8] |= 1;
			break;
		case 7: //Weekday
			clearDisplay(0);
			if(add)
			{
				time.wday = _bcd_add_short(time.wday,1);
				if (time.wday > 0x06)
					time.wday = 0x00;
			}
			else
			{
				time.wday = _bcd_add_short(time.wday,0x9999);
				if (time.wday > 0x06)
					time.wday = 0x06;
			}
			displayWeekday(time.wday);
			break;
		default://12 or 24h
			clearDisplay(0);
			if (hourMode == 1) //switch to 24h
			{
				hourMode = 0;
				displayString("   24h", 6);
			}
			else //switch to 12h
			{
				hourMode = 1;
				displayString("   12h", 6);
			}
			break;
	}
}

//TODO - having trouble finding length of array - clean up later
void displayString(char * c, char len)
{
	char index = 1;
	while(index - 1 < len) //as long as there are more characters, and we aren't past the edge of our display
	{
		//char toDisp = ;
		screen[index] = translateChar(*c++);
		index++;
	}
}
//set an override string and display time
void displayORString(char * c, char len, char time)
{
	char index = 1;
	while(index - 1 < len & index -1 < 9) //as long as there are more characters, and we aren't past the edge of our display
	{
		//char toDisp = ;
		screenOR[index] = translateChar(*c++);
		index++;
	}
	overrideTime = time;
}
char translateChar(char c)
{
	if (c >= 97 & c <= 122) //lowercase letters
		return alphatable[c - 97];
	else if (c >= 65 & c <= 90) //uppercase letters - use same table
		return alphatable[c - 65];
	else if (c >= 48 & c <= 57) //numbers
		return numbertable[c - 48];
	else if (c == 32) //space
		return 0x00;
	else if (c == 46) //period
		return 1;
	else if (c == '°')
		return specialchars[1];
	else
		return 0x80; //Error character
}

//Handles display formatting for a date
void displayDate(struct btm *t, char override)
{
	unsigned int month = _bcd_add_short(t->mon,1);//month is 0-11
	setScreen(1, numbertable[(month & 0x00F0)>>4],override);
	setScreen(2, numbertable[(month & 0x000F)],override);
	setScreen(3, specialchars[0],override);
	setScreen(4, numbertable[(t->mday & 0x00F0)>>4],override);
	setScreen(5, numbertable[(t->mday & 0x000F)],override);
	setScreen(6, specialchars[0],override);
	setScreen(7, numbertable[(t->year & 0x00F0)>>4],override);
	setScreen(8, numbertable[(t->year & 0x000F)],override);
}

void setScreen(char index, char value, char override)
{
	if (override)
		screenOR[index] = value;
	else
		screen[index] = value;
}

void alarm_off()
{
	if (alarm_duration > 0)
		alarm_snooze = 1;
}

//Sets up the alarm settings screen when settings entered
void Alarm_InitSettings()
{

	displayORString("alarms",6,4);
	Alarm_DisplayAlarms();
}
//Alarm settings - setting 0 screen
void Alarm_DisplayAlarms()
{
	clearDisplay(0);
	char alarm_idx = 0;
	for (alarm_idx = 0; alarm_idx < NUM_ALARMS; alarm_idx++)
	{
		screen[alarm_idx+1] = numbertable[alarm_idx+1];
		if(alarms_enabled & 1<<alarm_idx)
			screen[alarm_idx+1] |= 1; //turn on decimal place if enabled
	}
}

int read_block(unsigned char *d, unsigned len)
{
	while(len--) {
		*d++ = owex(-1, 8);
	}
	return 0;
}

void display_temp(int n, char override)
{
	char index = 0, temp = 0;
	char toDisplay_tmp[8];
	for (temp = 0; temp < 8; temp++)
		toDisplay_tmp[temp] = ' ';
	char s[16];
	if(n < 0)
	{
		n = -n;
		toDisplay_tmp[index] = '-';
		index++;
	}
	utoa(n >> 4, s);

	//puts(s);
	for (temp = 0; temp < 4; temp++)
	{
		if(s[temp])
		{
			toDisplay_tmp[index] = s[temp];
			index++;
		}
		else
			temp = 99; //stop going through string
	}
	//add decimal place
	toDisplay_tmp[index] = '.';
	index++;

	//grab just the value after the decimal place
	n = n & 0x000F;
	if(n)
	{
		utoa(n * 625, s);
		if(n < 2)
		{
			index++;
			toDisplay_tmp[index] = '0';
		}
		//puts(s);
		for (temp = 0; temp < 2; temp++) //we only have room to print two decimal places
		{
			if(s[temp])
			{

				toDisplay_tmp[index] = s[temp];
				index++;
			}
		}
	}
	else
	{
		toDisplay_tmp[index] = '0';
		toDisplay_tmp[index+1] = '0';
		index += 2;

		//puts("0000");
	}
	toDisplay_tmp[index] = '°';
	toDisplay_tmp[index + 1] = 'F';//TODO - pass what type of temperature
	if (override)
	{
		displayORString(toDisplay_tmp,8,12);
	}
	else
	{
		displayString(toDisplay_tmp,8);
	}
}
