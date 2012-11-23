/*
 * main.h
 *
 *  Created on: Sep 21, 2012
 *      Author: cberg
 */

#ifndef MAIN_H_
#define MAIN_H_

#define SCL_PIN BIT5 //CLK pin for Max6921
#define SDA_PIN BIT7  //DIN pin for MAX6921
#define VFD_CS_PIN BIT0 //CS line for the MAX6921
#define VFD_BLANK_PIN BIT1 //Blank pin for Max6921 - active high
#define VFD_BLANK_ON P2OUT |= VFD_BLANK_PIN
#define VFD_BLANK_OFF P2OUT &= ~VFD_BLANK_PIN
#define VFD_SELECT P2OUT &= ~VFD_CS_PIN
#define VFD_DESELECT P2OUT |= VFD_CS_PIN
//button defines
#define S1_PIN BIT5
#define S2_PIN BIT4
#define S3_PIN BIT3


//feature pins - both on Port 1
#define BUZ_PIN BIT3
#define ONEWIRE_PIN BIT4

#define BUZ_OFF P1OUT &= ~BUZ_PIN
#define BUZ_ON P1OUT |= BUZ_PIN
#define BUZ_TOGGLE P1OUT |= BUZ_PIN

struct btm time;

//alarm defines and variables
#define NUM_ALARMS 2
volatile struct alarm_bcd alarms[NUM_ALARMS];
volatile char alarms_enabled = 0;  //use bits to enable different alarms
#define ALARM_TIME 60; //how long to sound an alarm in seconds
const int alarm_on_time = 50;
const int alarm_off_time = 100;
volatile char alarm_duration = 0; //how much longer to sound the alarm
char alarm_snooze = 0;

//temperature defines and variables
volatile char take_temp = 0; //indicates that we should read the temperature
int temp_c, temp_c_1, temp_c_2, temp_f, temp_c_last;

//Function Prototypes
void initSPI();
void write(char address, char data);
void initDisplayTimer();
void initUART();
void initClockTimer();
void displayTime(struct btm *t);
void displayDate(struct btm *t, char override);
void displayWeekday(unsigned int wday);
void switchMode(char newMode);
void displayString(char * c, char len);
void displayORString(char * c, char len, char time);
char translateChar(char c);
void initPeripherals();
void clearDisplay(char buffer);
void Time_ChangeSetting();
void Time_ChangeValue(char add);
void Alarm_InitSettings();
void Alarm_SettingTick();
void Alarm_DisplayAlarms();
void displayAlarm(char alarm_num, char display_type);
void Alarm_ChangeSetting();
void Alarm_TickSetting(); //executes every second
void Alarm_ChangeValue(char add);
void setScreen(char index, char value, char override);
void alarm_off();
int read_block(unsigned char *d, unsigned len);
void display_temp(int n, char override,char type);
int GetTemp();

//assembly functions
void utoa(unsigned, char *);

/*
 * Mode Values
 * New mode values should be defined here - add on anything you want!
 * Change the ModeMax value to the highest mode to be used
 */
#define ModeTime  	0
#define ModeAlarm  	1
#define ModeTemp	2
#define ModeText  	3
#define ModeMax		2 //highest mode - drop back to 0 after

//Global settings
volatile char DisplayMode = 0; //See defines above for values
volatile char bufferPlace = 0;
char hourMode = 1; //0 = 24h, 1 = 12h
char tempMode = 1; //0 = celsius, 1 = fahrenheit
char settings_mode = 0;
volatile char setting_place = 0;  //with multiple settings, keeps track of which we're on
volatile char allow_repeat = 0; //if 1, holding down the button will increment values quickly
volatile char alarm_index = 0; //allows iterating through alarms in settings mode

volatile char screen[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
volatile char screenOR[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
//length of time to override the display
volatile char overrideTime = 8;

volatile char CustomMsg[20] = {'4','3','o','h','.','c','o','m',' ','r','u','l','e','z',0,0,0,0,0,0};
volatile char CustomMsgLen = 14;

//Serial debugging
	void RunSerialTX();

#endif /* MAIN_H_ */
