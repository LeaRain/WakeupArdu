#include <ThreeWire.h>  
#include <RtcDS1302.h>
#include <LiquidCrystal.h>

LiquidCrystal myLCD(8, 9, 4, 5, 6, 7);
ThreeWire myWire(20,21,19); // IO, SCLK, CE
RtcDS1302<ThreeWire> myRTC(myWire);
RtcDateTime alarmTime;

void setup() {
  Serial.begin(9600);
  setupLCD();
  setupRTC();
  setupAlarm();
}

void setupLCD() {
  myLCD.begin(16,2);
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
  alarmTime = RtcDateTime(__DATE__, "08:00:00.000");
}

void loop() {
  RtcDateTime now = myRTC.GetDateTime();
  printCurrentTime(now);
  printAlarmTime(alarmTime);
  delay(1000);
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
