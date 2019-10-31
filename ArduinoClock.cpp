// Example code for the MD_DS3231 library
//
// Implements a clock display on a 16x2 LCD shield.
// Display updates are triggered by a 1 second alarm from the RTC alarm 1.
//
// The example shows using the library in polled mode, polled mode with callback,
// or interrupt mode. Different modes of operation are enabled through setting
// one of the USE_* defines at the top of the file.
//
#include <Arduino.h>
#include <LiquidCrystal.h>
#include <MD_DS3231.h>
#include <Wire.h>
#include <Keypad.h>

// Select the mode of operation for the library
#define USE_POLLED      0 // polled only mode - use the return status from checkAlarm()
#define USE_POLLED_CB   1 // polled mode with callback - checkAlarm() will invoke the callback
#define USE_INTERRUPT   0 // external interrupt operation, set up for Arduino Uno pin PIN_INTERRUPT

#define PIN_INTERRUPT   2 // interrupt pin
static uint8_t POS[] = { 0, 4, 5, 6, 7, 9, 10, 12, 13, 16, 17, 19, 20, 22, 23 };
static uint8_t POSES_NO = 15;
static uint8_t cursor = 0;
static uint8_t x = 0;
static uint8_t y = 0;

const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
//define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = { { '1', '2', '3', 'A' }, { '4', '5', '6', 'B' }, {
		'7', '8', '9', 'C' }, { '*', '0', '#', 'D' } };
byte rowPins[ROWS] = { 5, 4, 6, 2 }; //connect to the row pinouts of the keypad
byte colPins[COLS] = { A0, A1, A2, A3 }; //connect to the column pinouts of the keypad

//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS,
		COLS);

LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

volatile boolean bShowTime = false;
volatile boolean bIsEditMode = false;

void p2dig(uint8_t v)
// print 2 digits with leading zero
		{
	if (v < 10)
		lcd.print("0");
	lcd.print(v);
}

const char* dow2String(uint8_t code)
// Day of week to string. DOW 1=Sunday, 0 is undefined
		{
	static const char *str[] = { "---", "Sun", "Mon", "Tue", "Wed", "Thu",
			"Fri", "Sat" };

	if (code > 7)
		code = 0;
	return (str[code]);
}

void printTime(void)
// Print the current time and date to the LCD display
		{
	lcd.setCursor(0, 0);
	lcd.print(dow2String(RTC.dow));
	lcd.print(" ");
	lcd.print(RTC.yyyy);
	lcd.print("-");
	p2dig(RTC.mm);
	lcd.print("-");
	p2dig(RTC.dd);

	lcd.setCursor(0, 1);
	p2dig(RTC.h);
	lcd.print(":");
	p2dig(RTC.m);
	lcd.print(":");
	p2dig(RTC.s);
	if (RTC.status(DS3231_12H) == DS3231_ON)
		lcd.print(RTC.pm ? " pm" : " am");
}

void displayUpdate(void)
// update the display
		{
	RTC.readTime();
	printTime();
}

#if USE_POLLED_CB || USE_INTERRUPT
void alarmICB(void)
// callback and interrupt function (same functionality)
		{
	bShowTime = true;    // set the flag to update
}
#endif

void setup() {
	// initialise 16x2 LCD display shield
	lcd.begin(16, 2);
	lcd.clear();
	//lcd.noCursor();
	//lcd.setCursor(0, 0);
	lcd.noBlink();
	Serial.begin(9600);

	/// set up the alarm environment
#if USE_POLLED
  // nothing more to do here
#endif
#if USE_POLLED_CB
	RTC.setAlarm1Callback(alarmICB);
#endif
#if USE_INTERRUPT
  // set up hardware at Arduino end
  pinMode(PIN_INTERRUPT, INPUT_PULLUP);
  attachInterrupt(0, alarmICB, FALLING);
  // set up the clock interrupt registers
  RTC.control(DS3231_A2_INT_ENABLE, DS3231_ON);
  RTC.control(DS3231_A1_INT_ENABLE, DS3231_ON);
  RTC.control(DS3231_INT_ENABLE, DS3231_ON);
  // set the Alarm flag off in case it is on
  RTC.control(DS3231_A1_FLAG, DS3231_OFF);
#endif

	// now initialise the 1 second alarm for screen updates
	RTC.setAlarm1Type(DS3231_ALM_SEC);
}

void loop() {
	char customKey = customKeypad.getKey();
	if (customKey) {
		Serial.println(customKey);
		switch (customKey) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			break;
		case 'A':
			bIsEditMode = !bIsEditMode;
			cursor = 0;
			lcd.setCursor(0, 0);
			if (bIsEditMode) {
				lcd.blink();
			} else {
				lcd.noBlink();
			}
			break;
		case 'B':
			break;
		case 'C':
			break;
		case 'D':
			break;
		case '*':
			Serial.print("Move cursor down");

			if (cursor > 1)
				cursor = (cursor - 1) % POSES_NO;
			else
				cursor = 0;
			x = POS[cursor] % 16;
			y = POS[cursor] / 16;
			lcd.setCursor(x, y);

			break;
		case '#':
			Serial.print("Move cursor up");
			cursor = (cursor + 1) % POSES_NO;
			x = POS[cursor] % 16;
			y = POS[cursor] / 16;
			lcd.setCursor(x, y);

			break;
		}
		Serial.print("Cursor:");
		Serial.println(cursor);


	}

#if USE_POLLED
  bShowTime = RTC.checkAlarm1();
#endif
#if USE_POLLED_CB
	RTC.checkAlarm1();
#endif

	// if the flag has been set, update the display then reset the show flag
	if (bShowTime) {
#if USE_INTERRUPT
    RTC.control(DS3231_A1_FLAG, DS3231_OFF);  // clear the alarm flag
#endif
		displayUpdate();
	}
	bShowTime = false;
}
