#include <ThreeWire.h>  
#include <RtcDS1302.h>

#include <LiquidCrystal.h>

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
ThreeWire myWire(20,21,19); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);

void setup() {
  Serial.begin(9600);
  lcd.begin(16,1);
  Rtc.Begin();
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  Rtc.SetDateTime(compiled);
  

  
}

void loop() {

  RtcDateTime now = Rtc.GetDateTime();
  lcd.setCursor(0, 1);
  printDateTime(now);
  delay(1000);
  // put your main code here, to run repeatedly:

}
#define countof(a) (sizeof(a) / sizeof(a[0]))

void printDateTime(const RtcDateTime& dt)
{
    char datestring[20];

    snprintf_P(datestring, 
            countof(datestring),
            PSTR("%02u:%02u:%02u"),
            dt.Hour(),
            dt.Minute(),
            dt.Second() );
    lcd.print(datestring);
}
