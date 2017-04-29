#include <neopixel.h>
#include <Grove_LCD_RGB_Backlight.h>
#include <SparkTime.h>
#include <string>
#include <sstream>

//LED Stuff
#define PIXEL_PIN D4
#define PIXEL_COUNT 2
#define PIXEL_TYPE WS2811
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);
int PixelColorOff = strip.Color( 0, 0, 0);
int PixelColorGreen = strip.Color( 0, 128, 0);
int PixelColorRed = strip.Color( 80, 0, 4);

rgb_lcd lcd;
const int colorR = 122;
const int colorG = 122;
const int colorB = 122;

//Functions
int playSpeaker(String nothing);
int Calculations(String wakeUpTime, String bedTime);
void alarmHandler(const char *event, const char *data);

//Pins
int speakerPin = A4;
int buttonPin = D2;
int button2Pin = D5;

//Button Stuff
bool ButtonNow = FALSE;//Create two states to compare so that the button doesn't continously count
bool ButtonLast = FALSE;
bool Button2Now = FALSE;
bool Button2Last = FALSE;
bool didSpeakerPlay = FALSE;//Did speaker play in last 24 hours?

//Time stuff
String bedTime;
String wakeUpTime;//stored time when alarm button is pressed
UDP UDPClient;
SparkTime rtc;
unsigned long currentTime;
unsigned long lastTime = 0UL;
String timeStr;


enum led_mode_t {
    OFF,
    ON,
};
led_mode_t mode = ON;

void setup() {
    //LED off
    strip.begin();
    strip.show();

    //Speaker function for IFTTT
    Particle.function("Speaker", playSpeaker);
    Particle.subscribe("Alarm", alarmHandler);

    // set up the LCD's number of columns and rows:
    lcd.begin(16, 2);

    lcd.setRGB(colorR, colorG, colorB);

    // Print a message to the LCD.
    lcd.print("The time is :)");

    delay(1000);

    //Begin time stuff
    rtc.begin(&UDPClient, "north-america.pool.ntp.org");
    rtc.setTimeZone(-6); // gmt offset

    //Button stuff
    pinMode(buttonPin, INPUT_PULLDOWN);
    pinMode(button2Pin, INPUT_PULLDOWN);
    Serial.begin(9600);
}

void loop() {
  //LED set off
  strip.setPixelColor(0, PixelColorOff);
  strip.setPixelColor(1,PixelColorOff);
  strip.show();

    // set the cursor to column 0, line 1
    // (note: line 1 is the second row, since counting begins with 0):
    lcd.setCursor(0, 1);

    //Display the time
    currentTime = rtc.now();
    if (currentTime != lastTime)
    {
    	// Just the time in 12 hour format
    	timeStr = "";
    	timeStr += rtc.hour12String(currentTime);
    	timeStr += ":";
    	timeStr += rtc.minuteString(currentTime);
    	timeStr += " ";
    	//timeStr += rtc.secondString(currentTime);
    	//timeStr += " ";
    	timeStr += rtc.AMPMString(currentTime);

	    // print the time string
        lcd.print(timeStr);

    }

    //Is it midnight? If so...24 hour reset!
    if(timeStr == "12:00 AM")
    {
        didSpeakerPlay = FALSE;//Speaker did not play today
    }

    //Update the time
    lastTime = currentTime;

    //Bedtime button
    if(Button2Now == HIGH && Button2Last == LOW)
	   {
    	bedTime=currentTime;
	   }
    //Did IFTTT Send Event?
    //When there is 15 minutes before event, a trigger to playSpeaker(); is called
    timeStr = "";
}

int playSpeaker(String nothing)
{

    if(didSpeakerPlay == FALSE)//Plays speaker for first time in the day
    {   //play alarm if the mode is ON
        while(mode == ON)
        {
            //the pin, the frequency of tone in hertz, the duration of tone in milliseconds
            tone(speakerPin, 880, 1000);
            delay(1100);
            tone(speakerPin, 440, 1000);
            delay(1100);

            ButtonNow = digitalRead(buttonPin);

            if(ButtonNow == HIGH && ButtonLast == LOW)//button is pressed
            {
                mode = OFF;
                noTone(speakerPin);
                wakeUpTime = timeStr;
                //Call the calculations function to send sleep time and get hours
                int hrsSlept = calculations(wakeUpTime, bedTime);
                //Call the LED function - pass hours
               leds(hrsSlept);
            }
            else if (ButtonNow == LOW)
            {
                ButtonLast = LOW;
            }
        }

    }
    //SORRY the alarm already played today! No more alarms
  return 0;
}

int calculations(String wakeUpTime, String bedTime)
{
    //Parse wake up time string into hours and minutes
    int whrs = atoi(wakeUpTime.substring(0,wakeUpTime.indexOf(":")));
    int wmins = atoi(wakeUpTime.substring(wakeUpTime.indexOf(":")+1, 2));
    //Parse bed time string into hours and minutes
    int bhrs = atoi(bedTime.substring(0,bedTime.indexOf(":")));
    int bmins = atoi(bedTime.substring(bedTime.indexOf(":")+1, 2));
    int hrsSlept;
    //Subtract the hours and store as hoursSlept
    if(bhrs > 8)
    {
        hrsSlept = (12 - bhrs) + whrs;//ex (12 -9) + 6
    }
    else
    {
        hrsSlept = whrs - bhrs;//ex 6 - 1
    }
    //Subtract the minutes and store as minutes slept
    int minsSlept = bmins + wmins;
    //If minutes are greater then 59, do mod and add an hourSlept
    if(minsSlept >= 60)
    {
        minsSlept = minsSlept % 60;
        hrsSlept++;
    }
    //Store hoursSlept and minutesSlept as 1 string and sent to excel

    wakeUpTime = "Hours slept: " + String(hrsSlept) + " Minutes Slept: " + String(minsSlept);
    Spark.publish("timeSlept", wakeUpTime);

    return hrsSlept;

}

void leds(int x)
{

    if ( x >=8 )
    {
        	strip.setPixelColor(0, PixelColorGreen);
        	strip.setPixelColor(1, PixelColorGreen);
        	strip.show();

    }
    else
    {
        	strip.setPixelColor(0, PixelColorRed);
        	strip.setPixelColor(1, PixelColorRed);
        	strip.show();

    }
    //strip.setPixelColor(0, PixelColorOff);
    //strip.setPixelColor(1, PixelColorOff);
    //strip.show();
    delay(3000);
}

void alarmHandler(const char *event, const char *data)
{
  playSpeaker(data);
}
