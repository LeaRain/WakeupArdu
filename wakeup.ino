#include <FastLED.h>
#include <ThreeWire.h>  
#include <RtcDS1302.h>
#include <LiquidCrystal.h>

// Define two modes parameter change during the setting of the alarm
#define SET_HOUR 0
#define SET_MINUTE 1
// Define two modes for the current operation for the alarm time
#define DECREASE_ALARM 0
#define INCREASE_ALARM 1
// Define values for the different buttons on the LCD
#define BUTTON_NONE -1
#define BUTTON_LEFT 0
#define BUTTON_UP 1
#define BUTTON_DOWN 2
#define BUTTON_RIGHT 3
#define BUTTON_SELECT 4

// Define number of LEDs on stripe
#define NUM_LEDS 120
// Data pin on Arduino Mega for LED stripes
#define DATA_PIN 36
// Mode for lights (on or off)
#define LIGHT_OFF 0
#define LIGHT_ON 1

// Define an array for single access of LED on stripe
CRGB leds[NUM_LEDS];

// LCD parameters for display
LiquidCrystal myLCD(8, 9, 4, 5, 6, 7);
// IO, SCLK, CE mapping on Arduino Mega
ThreeWire myWire(20,21,19);
RtcDS1302<ThreeWire> myRTC(myWire);
// Value for alarm later
RtcDateTime alarmTime;
// Value for current mode for setting alarm -> minute or hour
int currentSetMode;
// Counter for void loop()
int loopRound = 0;
// Value determining if the wakeup has started
int wakeUpStarted = 0;
// Value determining if the evening has started
int eveningStarted = 0;


/* 
 * Setup for different components of the alarm: LCD, RTC, alarm itself, LED stripe
 */
void setup() {
  Serial.begin(9600);
  setupLCD();
  setupRTC();
  setupAlarm();
  setupLED();
}

/*
 * LCD Setup: Define display and current mode
 */
void setupLCD() {
  myLCD.begin(16,2);
  // Setting mode of the alarm in LCD
  changeAlarmSettingMode(SET_HOUR);
}

/*
 * RTC Setup: Define current time, if not already done
 */
void setupRTC() {
  myRTC.Begin();
  // Get current time
  RtcDateTime compileTime = RtcDateTime(__DATE__, __TIME__);

  // Ask RTC if time is valid, if not, set
  if (!myRTC.IsDateTimeValid()) {
    myRTC.SetDateTime(compileTime);
  }

  if (myRTC.GetIsWriteProtected()) {
    myRTC.SetIsWriteProtected(false);
    }

    if (!myRTC.GetIsRunning()) {
        myRTC.SetIsRunning(true);
    }

  // Check for current RTC time, overwrite if time is incorrect
  RtcDateTime now = myRTC.GetDateTime();
  if (now < compileTime) {
        myRTC.SetDateTime(compileTime);
    }
}

/*
 * Setup alarm with an initial time
 */
void setupAlarm() {
  // Initial alarm time: One hour after boot
  RtcDateTime now = myRTC.GetDateTime();
  alarmTime = now + 3600;
}

/*
 * Setup LEDs with their type, data and number of LEDs
 */
void setupLED() {
  FastLED.addLeds<WS2812, DATA_PIN, RGB>(leds, NUM_LEDS);
  // Disable light if it was on
  turnLEDOffOn(LIGHT_OFF);
}

void loop() {
  // Get current time
  RtcDateTime now = myRTC.GetDateTime();
  // Set current time and current alarm time on LCD
  printCurrentTime(now);
  printAlarmTime(alarmTime);
  // Get the current pressed button to apply changes
  int button = getPressedButton();
  switch (button) {
    case BUTTON_LEFT:
      changeAlarmSettingMode(SET_HOUR);
      break;
    case BUTTON_RIGHT:
      changeAlarmSettingMode(SET_MINUTE);
      break;
    case BUTTON_DOWN:
      changeAlarmTime(DECREASE_ALARM);
      break;
    case BUTTON_UP:
      changeAlarmTime(INCREASE_ALARM);
      break;
     case BUTTON_SELECT:
      turnLEDOffOn(LIGHT_OFF);
     default:
      break;
    }

  // Increase loop count for checkups
  loopRound += 1;

  // Check every 30 seconds (since delay is 500 ms) if it's time to start a LED procedure
  // Exact second matching is not necessary in this case, we are open to some inaccuracies
  if (loopRound % 60 == 0) {
    // If it's time to wakeup, start the wakeup routine
    if (checkWakeUp(now) == 1) startWakeUpRoutine();  
    // Start evening routine if it's evening
    if (checkEveningRoutine(now) == 1) startEveningRoutine();
  }

  // Check for currently running wakeup routine and proceed if necessary
  if (wakeUpStarted == 1) {
    continueWakeUpRoutine(now);
  }

  if (eveningStarted == 1) {
    continueEveningRoutine(now);  
  }
  delay(500);
}


// Print current time, source: https://starthardware.org/arduino-uhrzeit-mit-der-real-time-clock-rtc/
#define countof(a) (sizeof(a) / sizeof(a[0]))

void printCurrentTime(const RtcDateTime& dt)
{
  myLCD.setCursor(0, 0);
    char datestring[20];

    snprintf_P(datestring, 
            countof(datestring),
            PSTR("Now: %02u:%02u(%02u)"),
            dt.Hour(),
            dt.Minute(),
            dt.Day());
    myLCD.print(datestring);
}

void printAlarmTime(const RtcDateTime& dt) {
  myLCD.setCursor(0, 1);
    char datestring[20];

    snprintf_P(datestring, 
            countof(datestring),
            PSTR("Alarm: %02u:%02u(%02u)"),
            dt.Hour(),
            dt.Minute(),
            dt.Day());
    myLCD.print(datestring);
}

/*
 * Get the current pressed button for proceeding with further input
 * Values for key_in measured with Serial.println(key_in)
 */
int getPressedButton() {
  int key_in = analogRead(0);
  if (key_in < 50) return BUTTON_RIGHT;
  if (key_in < 150) return BUTTON_UP;
  if (key_in < 350) return BUTTON_DOWN;
  if (key_in < 500) return BUTTON_LEFT;
  if (key_in < 700) return BUTTON_SELECT;
  return BUTTON_NONE;
}

/*
 * Change the mode for alarm setting to hour or minute
 */
void changeAlarmSettingMode(int mode) {
  currentSetMode = mode;  
}

/*
 * Change the alarm time based on the current mode (hour/minute and increase/decrease)
 */
void changeAlarmTime(int mode) {
  int changeBy;
  if (currentSetMode == SET_HOUR) {
    changeBy = 3600;
  }
  else {
    changeBy = 60;
  }

  if (mode == DECREASE_ALARM) {
    alarmTime -= changeBy;  
  }

  else {
    alarmTime += changeBy;  
  }
}

/*
 * Check if it's time for wakeup based on the current time
 */
int checkWakeUp(const RtcDateTime& currentTime) {
  // Check for hour and minute, seconds irrelevant, inaccuracy allowed
  if (currentTime.Hour () == alarmTime.Hour() && currentTime.Minute() == alarmTime.Minute()) return 1;
  return 0;
}

/*
 * Check if it's time for evening based on the current time
 */
int checkEveningRoutine(const RtcDateTime& currentTime) {
  // Current hour for further checks
  int currentHour = currentTime.Hour();
  // Yes my evening lasts from 16:00 to 20:59 
  if (currentHour >= 16 || currentHour <= 20) return 1;
  return 0;
}

/*
 * Check for the seconds after the alarm time based on the current time
 */
int checkSecondsAfterAlarm(const RtcDateTime& currentTime) {
  int difference = currentTime - alarmTime;
  return difference;
}

/*
 * Start the wake up: Turn the lights on, activate the start mode
 */
void startWakeUpRoutine() {
   turnLEDOffOn(LIGHT_ON);
   FastLED.setBrightness(4); 
   FastLED.show();
   wakeUpStarted = 1;
}

/*
 * Start the evening routine: just turn the lights on
 */
void startEveningRoutine() {
  for (int i = 0; i < NUM_LEDS; i++) {
      // today's choice: orange
      leds[i] = CRGB::Orange;
  }
  FastLED.show();
  eveningStarted = 1;
}

/*
 * Increase brightness (or end lighting) based on the time since the alarm started
 */
void continueWakeUpRoutine(const RtcDateTime& currentTime) {
  int secondsAfterAlarm = checkSecondsAfterAlarm(currentTime);
  
  if (secondsAfterAlarm > 1200) {
    Serial.println("Ending wakeup...");
    endWakeUpRoutine();
   }

  if (secondsAfterAlarm > 900) {
    FastLED.setBrightness(128);
    return;
  }

  if (secondsAfterAlarm > 600) {
    FastLED.setBrightness(64);
    return;
  }
   
  if (secondsAfterAlarm > 300) {
    FastLED.setBrightness(32);
    return;
   }  
}

/*
 * Check if the evening routine is still up and running
 */
void continueEveningRoutine(const RtcDateTime& currentTime) {
  int currentHour = currentTime.Hour();
  Serial.println(currentHour);
  // Evening's over
  if (currentHour < 16 && currentHour > 20) endEveningRoutine();
}


/*
 * End the wakeup routine with turning light off and changing the parameter
 */
void endWakeUpRoutine() {
    // Set the alarm time to the same time on the next day
  uint8_t newAlarmDay = alarmTime.Day() + 1;
  RtcDateTime newAlarmTime = RtcDateTime(
    alarmTime.Year(),
    alarmTime.Month(),
    newAlarmDay,
    alarmTime.Hour(),
    alarmTime.Minute(),
    alarmTime.Second());
   alarmTime = newAlarmTime;

  wakeUpStarted = 0;
  turnLEDOffOn(LIGHT_OFF);
}

/*
 * Turn lights off, go to bed
 */
void endEveningRoutine() {
  eveningStarted = 0;  
  turnLEDOffOn(LIGHT_OFF);
}

/*
 * Turn the light off or on
 */
void turnLEDOffOn(int colorValue) {
  if (colorValue == LIGHT_ON) {  
  
  for (int i = 0; i < NUM_LEDS; i++) {
      // BlueViolet is beautiful
      leds[i] = CRGB::BlueViolet;
    }
    return;
  }
  
  if (colorValue == LIGHT_OFF) {
    // Lights off
    FastLED.clear(); 
    FastLED.show();
   }
}
