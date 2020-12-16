#include <SendOnlySoftwareSerial.h>
#include "RTClib.h"
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/power.h>
RTC_DS3231 rtc;

int LED_PIN = 1;
int SECONDS_DELAY = 61;

SendOnlySoftwareSerial mySerial(3);  // rx, tx


void setup ()
{
   mySerial.begin(9600);
   mySerial.println("Starting up...");

   
   pinMode(LED_PIN, OUTPUT);

   if (! rtc.begin())
   {
      mySerial.println("Couldn't find RTC");
      mySerial.flush();
      abort();
   }
   if (rtc.lostPower())
   {
      mySerial.println("RTC lost power, let's set the time!");
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
   }
   //we don't need the 32K Pin, so disable it
    rtc.disable32K();

     
     pinMode(4, INPUT_PULLUP);



  
  
        // stop oscillating signals at SQW Pin
    // otherwise setAlarm1 will fail
     rtc.writeSqwPinMode(DS3231_OFF);

     // turn off alarm 2 (in case it isn't off already)
    // again, this isn't done at reboot, so a previously set alarm could easily go overlooked
      rtc.clearAlarm(1);
     rtc.clearAlarm(2);
     rtc.disableAlarm(2);


    
   
    
}


void setAlarm(const DateTime& dt){
    
    
    if(!rtc.setAlarm1(
            dt,
            DS3231_A1_Date // this mode triggers the alarm when the seconds match. See Doxygen for other options
    )) {
        mySerial.println("Error, alarm wasn't set!");
    }else {
        mySerial.println("Alarm set");  
    }
    
}



void printDateTime(DateTime d){
mySerial.println(d.year());
mySerial.println(d.month());
mySerial.println(d.day());
mySerial.println(d.hour());
mySerial.println(d.minute());
mySerial.println(d.second());
mySerial.println("---------------------------");
}


void enterSleep (void)
{
  mySerial.println("Going to Sleep");
   GIMSK |= _BV(PCIE);    // turns on pin change interrupts
  PCMSK |= _BV(PCINT4);    // turn on interrupts on pins B4
  ADCSRA &= ~_BV(ADEN);                              // ADC off
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable(); // Sets the Sleep Enable bit in the MCUCR Register (SE BIT)
   sei(); // Enable interrupts
  sleep_cpu();
   cli();                                  // Disable interrupts                             
  sleep_disable();   
  sei();
   mySerial.println("Resuming process");
}

void loop ()
{
    
   DateTime currentTime = rtc.now();
  DateTime futureOnTime = DateTime(currentTime.year(),currentTime.month(),currentTime.day(),17,0,0);


  
  DateTime futureOffTime = DateTime(currentTime.year(),currentTime.month(),currentTime.day(),21,0,0);

  if (currentTime < futureOnTime){
     digitalWrite(LED_PIN, LOW);
    setAlarm(futureOnTime + TimeSpan(SECONDS_DELAY));
    mySerial.println("Not ready to turn on. Turning on at:");
    printDateTime(futureOnTime + TimeSpan(SECONDS_DELAY));
    enterSleep();
  }

  else if (currentTime >= futureOnTime && currentTime < futureOffTime){
       digitalWrite(LED_PIN, HIGH);
       setAlarm(futureOffTime + TimeSpan(SECONDS_DELAY));
       mySerial.println("Lights on, turrning off at:");
       printDateTime(futureOffTime + TimeSpan(SECONDS_DELAY));
       enterSleep();
  } else{
    digitalWrite(LED_PIN, LOW);
    setAlarm(futureOnTime + TimeSpan(1,0,0,SECONDS_DELAY));
    mySerial.println("Out of time range. Setting for Tomorrow");
    printDateTime(futureOnTime + TimeSpan(1,0,0,SECONDS_DELAY));
    enterSleep();
  }

   if(rtc.alarmFired(1)) {
    mySerial.println("alarm 1 has been triggered");
     rtc.clearAlarm(1);
  }

   if(rtc.alarmFired(2)) {
    mySerial.println("alarm 1 has been triggered");
     rtc.clearAlarm(1);
  }
  
}

  


ISR(PCINT0_vect){
   
}
