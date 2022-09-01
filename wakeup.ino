#include <ThreeWire.h>  
#include <RtcDS1302.h>
#include <LiquidCrystal.h>

#define SET_HOUR 0
#define SET_MINUTE 1
#define BUTTON_NONE -1
#define BUTTON_LEFT 0
#define BUTTON_UP 1
#define BUTTON_DOWN 2
#define BUTTON_RIGHT 3

LiquidCrystal myLCD(8, 9, 4, 5, 6, 7);
ThreeWire myWire(20,21,19); // IO, SCLK, CE
RtcDS1302<ThreeWire> myRTC(myWire);
RtcDateTime alarmTime;
int currentSetMode;
int loopRound = 0;

void setup() {
  Serial.begin(9600);
  setupLCD();
  setupRTC();
  setupAlarm();
}

void setupLCD() {
  myLCD.begin(16,2);
  // Setting mode of the alarm in LCD
  currentSetMode = SET_HOUR;
}

void setupRTC() {
  myRTC.Begin();
  RtcDateTime compileTime = RtcDateTime(__DATE__, __TIME__);

  if (!myRTC.IsDateTimeValid()) {
    myRTC.SetDateTime(compileTime);
  }

  if (myRTC.GetIsWriteProtected()) {
    myRTC.SetIsWriteProtected(false);
    }

    if (!myRTC.GetIsRunning()) {
        myRTC.SetIsRunning(true);
    }
  RtcDateTime now = myRTC.GetDateTime();
  if (now < compileTime) {
        myRTC.SetDateTime(compileTime);

    }
}

void setupAlarm() {
  // Initial alarm: 8 in the morning
  alarmTime = RtcDateTime(__DATE__, "08:00:00.000");
}

void loop() {
  RtcDateTime now = myRTC.GetDateTime();
  printCurrentTime(now);
  printAlarmTime(alarmTime);
  int button = getPressedButton();
  switch (button) {
    case BUTTON_LEFT:
      changeToHourMode();
      break;
    case BUTTON_RIGHT:
      changeToMinuteMode();
      break;
    case BUTTON_DOWN:
      decreaseAlarmTime();
      break;
    case BUTTON_UP:
      increaseAlarmTime();
      break;
     default:
      break;
    }
  loopRound += 1;

  if (loopRound % 60 == 0) {
    if (checkWakeUp(now) == 1) Serial.println(1337);  
  }
  
  delay(500);
  // put your main code here, to run repeatedly:

}
#define countof(a) (sizeof(a) / sizeof(a[0]))

void printCurrentTime(const RtcDateTime& dt)
{
  myLCD.setCursor(0, 0);
    char datestring[20];

    snprintf_P(datestring, 
            countof(datestring),
            PSTR("Now: %02u:%02u"),
            dt.Hour(),
            dt.Minute());
    myLCD.print(datestring);
}

void printAlarmTime(const RtcDateTime& dt) {
  myLCD.setCursor(0, 1);
    char datestring[20];

    snprintf_P(datestring, 
            countof(datestring),
            PSTR("Alarm: %02u:%02u"),
            dt.Hour(),
            dt.Minute());
    myLCD.print(datestring);
}

int getPressedButton() {
  int key_in = analogRead(0);
  if (key_in < 50) return BUTTON_RIGHT;
  if (key_in < 250) return BUTTON_UP;
  if (key_in < 450) return BUTTON_DOWN;
  // TODO: Debug something with button left
  if (key_in < 600) return BUTTON_LEFT;
  // Ignore BUTTON_SELECT case, (currently) not necessary
  return BUTTON_NONE;
}

void changeToHourMode() {
  currentSetMode = SET_HOUR;
  
}

void changeToMinuteMode() {
  currentSetMode = SET_MINUTE;
}

void decreaseAlarmTime() {
  int decreaseBy;
  if (currentSetMode == SET_HOUR) {
    decreaseBy = 3600;
  }
  else {
    decreaseBy = 60;
  }

  alarmTime -= decreaseBy;
}

void increaseAlarmTime() {
  int increaseBy;
  if (currentSetMode == SET_HOUR) {
    increaseBy = 3600;
  }
  else {
    increaseBy = 60;
  }
  alarmTime += increaseBy; 
}

int checkWakeUp(const RtcDateTime& currentTime) {
  if (currentTime.Hour () == alarmTime.Hour() && currentTime.Minute() == alarmTime.Minute()) return 1;
  return 0;
}
