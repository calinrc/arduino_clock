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

#define DAYOFWEEK  0
#define YEARM  4
#define YEARC  5
#define YEARD  6
#define YEARU  7
#define MONTHD  9
#define MONTHU  10
#define DAYD 12
#define DAYU 13
#define HOURD 16
#define HOURU 17
#define MIND 19
#define MINU 20
#define SECD 22
#define SECU 23

static uint8_t CLOCK_POS[] ={ DAYOFWEEK, YEARM, YEARC, YEARD, YEARU, MONTHD, MONTHU, DAYD, DAYU, HOURD, HOURU, MIND, MINU, SECD, SECU };
static uint8_t ALARM_POS[] ={  HOURD, HOURU, MIND, MINU, SECD, SECU };
static uint8_t CLOCK_POSES_NO = 15;
static uint8_t ALARM_POSES_NO = 6;
static uint8_t cursor = 0;
static uint8_t alarmCursor = 0;
static uint8_t x = 0;
static uint8_t y = 0;
static MD_DS3231 EDIT_RTC;
static MD_DS3231 EDIT_ALARM_RTC;

const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
//define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] =
{
{ '1', '2', '3', 'A' },
{ '4', '5', '6', 'B' },
{ '7', '8', '9', 'C' },
{ '*', '0', '#', 'D' } };
byte rowPins[ROWS] =
{ 5, 4, 6, 2 }; //connect to the row pinouts of the keypad
byte colPins[COLS] =
{ A0, A1, A2, A3 }; //connect to the column pinouts of the keypad

//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

enum CLOCK_STATE
{
	NORMAL,
	EDIT_TIME,
	ALARM,
	EDIT_ALARM,
	CLOCK_STATES_NO
};

volatile boolean bShowTime = false;
volatile boolean bShowAlarm = false;
volatile CLOCK_STATE eClockMode = NORMAL;

void p2dig(uint8_t v)
// print 2 digits with leading zero
{
	if (v < 10)
		lcd.print("0");
	lcd.print(v);
}

const char* dow2String(uint8_t code)
// Day of week to string. DAYOFWEEK 1=Sunday, 0 is undefined
{
	static const char *str[] =
	{ "---", "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

	if (code > 7)
		code = 0;
	return (str[code]);
}

void printTime(MD_DS3231 *CLOCK)
{
// Print the current time and date to the LCD display

	lcd.setCursor(0, 0);
	lcd.print(dow2String(CLOCK->dow));
	lcd.print(" ");
	lcd.print(CLOCK->yyyy);
	lcd.print("-");
	p2dig(CLOCK->mm);
	lcd.print("-");
	p2dig(CLOCK->dd);

	lcd.setCursor(0, 1);
	p2dig(CLOCK->h);
	lcd.print(":");
	p2dig(CLOCK->m);
	lcd.print(":");
	p2dig(CLOCK->s);
	if (CLOCK->status(DS3231_12H) == DS3231_ON)
		lcd.print(CLOCK->pm ? " pm" : " am");

	lcd.setCursor(x, y);
}

void displayUpdate(MD_DS3231 *CLOCK)
// update the display
{
	CLOCK->readTime();
	printTime(CLOCK);
}

#if USE_POLLED_CB || USE_INTERRUPT
void alarmICB(void)
// callback and interrupt function (same functionality)
{
	bShowTime = true;    // set the flag to update
}

void alarm2ICB(void)
{
	bShowAlarm = true;
}
#endif

void setup()
{
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
	RTC.setAlarm2Callback(alarm2ICB);
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
	RTC.setAlarm2Type(DS3231_ALM_HM);
}

void changeItem(MD_DS3231 *EDITED, uint8_t pressedNumericKey, uint8_t *usedCursor)
{
	x = CLOCK_POS[*usedCursor] % 16;
	y = CLOCK_POS[*usedCursor] / 16;
	lcd.setCursor(x, y);

	switch (CLOCK_POS[*usedCursor])
	{
	case DAYOFWEEK:
		if (pressedNumericKey >= 1 && pressedNumericKey <= 7)
		{
			Serial.print("Change DayOfWeek ");
			Serial.println(pressedNumericKey);
			EDITED->dow = pressedNumericKey;
			lcd.print(dow2String(EDITED->dow));
		}
		break;
	case YEARM:
		if (pressedNumericKey == 1 || pressedNumericKey == 2)
		{
			Serial.print("Change YEARM ");
			Serial.println(pressedNumericKey * 1000 + EDITED->yyyy % 1000);
			EDITED->yyyy = pressedNumericKey * 1000 + EDITED->yyyy % 1000;
			lcd.print(pressedNumericKey);
		}
		break;
	case YEARC:
		Serial.print("Change YEARC ");
		Serial.println(pressedNumericKey * 100 + (EDITED->yyyy / 1000) * 1000);
		EDITED->yyyy = pressedNumericKey * 100 + (EDITED->yyyy / 1000) * 1000 + EDITED->yyyy % 100;
		lcd.print(pressedNumericKey);
		break;
	case YEARD:
		Serial.print("Change YEARD ");
		Serial.println(pressedNumericKey * 10 + (EDITED->yyyy / 100) * 100 + EDITED->yyyy % 10);

		EDITED->yyyy = pressedNumericKey * 10 + (EDITED->yyyy / 100) * 100 + EDITED->yyyy % 10;
		lcd.print(pressedNumericKey);
		break;
	case YEARU:
		Serial.print("Change YEARU ");
		Serial.println(pressedNumericKey + (EDITED->yyyy / 10) * 10);
		EDITED->yyyy = pressedNumericKey + (EDITED->yyyy / 10) * 10;
		lcd.print(pressedNumericKey);
		break;
	case MONTHD:

		if (pressedNumericKey == 0 || pressedNumericKey == 1)
		{
			Serial.print("Change MONTHD ");
			Serial.println(pressedNumericKey * 10 + EDITED->mm % 10);
			EDITED->mm = pressedNumericKey * 10 + EDITED->mm % 10;
			lcd.print(pressedNumericKey);
		}
		break;
	case MONTHU:
		if (pressedNumericKey + (EDITED->mm / 10) * 10 <= 12)
		{
			Serial.print("Change MONTHU ");
			Serial.println(pressedNumericKey + (EDITED->mm / 10) * 10);

			EDITED->mm = pressedNumericKey + (EDITED->mm / 10) * 10;
			lcd.print(pressedNumericKey);
		}
		break;
	case DAYD:
		if (pressedNumericKey >= 0 && pressedNumericKey <= 3)
		{
			Serial.print("Change DAYD ");
			Serial.println(pressedNumericKey * 10 + EDITED->dd % 10);

			EDITED->dd = pressedNumericKey * 10 + EDITED->dd % 10;
			lcd.print(pressedNumericKey);
		}
		break;
	case DAYU:
		if (pressedNumericKey + (EDITED->dd / 10) * 10 <= 31)
		{
			Serial.print("Change DAYU ");
			Serial.println(pressedNumericKey + (EDITED->dd / 10) * 10);

			EDITED->dd = pressedNumericKey + (EDITED->dd / 10) * 10;
			lcd.print(pressedNumericKey);
		}
		break;
	case HOURD:
		if (pressedNumericKey >= 0 && pressedNumericKey <= 2)
		{
			Serial.print("Change HOURD ");
			Serial.println(pressedNumericKey * 10 + EDITED->h % 10);

			EDITED->h = pressedNumericKey * 10 + EDITED->h % 10;
			lcd.print(pressedNumericKey);
		}
		break;
	case HOURU:
		if (pressedNumericKey + (EDITED->h / 10) * 10 <= 23)
		{
			Serial.print("Change HOURU ");
			Serial.println(pressedNumericKey + (EDITED->h / 10) * 10);

			EDITED->h = pressedNumericKey + (EDITED->h / 10) * 10;
			lcd.print(pressedNumericKey);
		}
		break;
	case MIND:
		if (pressedNumericKey >= 0 && pressedNumericKey <= 5)
		{
			Serial.print("Change MIND ");
			Serial.println(pressedNumericKey * 10 + EDITED->m % 10);

			EDITED->m = pressedNumericKey * 10 + EDITED->m % 10;
			lcd.print(pressedNumericKey);
		}
		break;
	case MINU:
		if (pressedNumericKey + (EDITED->m / 10) * 10 <= 59)
		{
			Serial.print("Change MINU ");
			Serial.println(pressedNumericKey + (EDITED->m / 10) * 10);

			EDITED->m = pressedNumericKey + (EDITED->m / 10) * 10;
			lcd.print(pressedNumericKey);
		}
		break;
	case SECD:
		if (pressedNumericKey >= 0 && pressedNumericKey <= 5)
		{
			Serial.print("Change SECD ");
			Serial.println(pressedNumericKey * 10 + EDITED->s % 10);

			EDITED->s = pressedNumericKey * 10 + EDITED->s % 10;
			lcd.print(pressedNumericKey);
		}
		break;
	case SECU:
		if (pressedNumericKey + (EDITED->m / 10) * 10 <= 59)
		{
			Serial.print("Change SECU ");
			Serial.println(pressedNumericKey + (EDITED->s / 10) * 10);

			EDITED->s = pressedNumericKey + (EDITED->s / 10) * 10;
			lcd.print(pressedNumericKey);
		}
		break;
	}
	*usedCursor = (*usedCursor + 1) % CLOCK_POSES_NO;
	x = CLOCK_POS[*usedCursor] % 16;
	y = CLOCK_POS[*usedCursor] / 16;
	lcd.setCursor(x, y);

}

void loop()
{
	char customKey = customKeypad.getKey();
	if (customKey)
	{
		Serial.println(customKey);
		switch (customKey)
		{
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
			if (eClockMode == EDIT_TIME)
			{
				changeItem(&EDIT_RTC, (customKey - '0'), &cursor);
			}
			else if (eClockMode == EDIT_ALARM)
			{
				changeItem(&EDIT_ALARM_RTC, (customKey - '0'), &alarmCursor);
			}
			break;
		case 'A':
			//Stopped here. Use CLOCK_STATES_NO
			eClockMode = (eClockMode == NORMAL) ? EDIT_TIME : NORMAL;
			cursor = 0;
			alarmCursor = 0;
			lcd.setCursor(0, 0);
			if (eClockMode == EDIT_TIME)
			{
				EDIT_RTC = RTC;
				lcd.blink();
			}
			else
			{
				EDIT_RTC.writeTime();
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

			if (eClockMode == EDIT_TIME)
			{
				if (cursor > 1)
					cursor = (cursor - 1) % CLOCK_POSES_NO;
				else
					cursor = 0;
				x = CLOCK_POS[cursor] % 16;
				y = CLOCK_POS[cursor] / 16;

			}
			else if (eClockMode == EDIT_ALARM)
			{
				if (alarmCursor > 1)
					alarmCursor = (alarmCursor - 1) % ALARM_POSES_NO;
				else
					alarmCursor = 0;
				x = ALARM_POS[alarmCursor] % 16;
				y = ALARM_POS[alarmCursor] / 16;

			}

			lcd.setCursor(x, y);

			break;
		case '#':
			Serial.print("Move cursor up");
			if (eClockMode == EDIT_TIME)
			{
				cursor = (cursor + 1) % CLOCK_POSES_NO;
				x = CLOCK_POS[cursor] % 16;
				y = CLOCK_POS[cursor] / 16;

			}
			else if (eClockMode == EDIT_ALARM)
			{
				alarmCursor = (alarmCursor + 1) % ALARM_POSES_NO;
				x = ALARM_POS[alarmCursor] % 16;
				y = ALARM_POS[alarmCursor] / 16;

			}
			lcd.setCursor(x, y);

			break;
		}

	}
	if (eClockMode == NORMAL)
	{

#if USE_POLLED
		bShowTime = RTC.checkAlarm1();
		bShowAlarm = RTC.checkAlarm2();
#endif
#if USE_POLLED_CB
		RTC.checkAlarm1();
		RTC.checkAlarm2();
#endif

		// if the flag has been set, update the display then reset the show flag
		if (bShowTime)
		{
#if USE_INTERRUPT
			RTC.control(DS3231_A1_FLAG, DS3231_OFF);  // clear the alarm flag
#endif
			displayUpdate(&RTC);
		}
	}
	bShowTime = false;
}
