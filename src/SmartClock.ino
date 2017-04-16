// This #include statement was automatically added by the Particle IDE.
#include <Grove_LCD_RGB_Backlight.h>

// This #include statement was automatically added by the Particle IDE.
#include <SparkTime.h>


int speakerPin = A4;
int buttonPin = D2;

bool ButtonNow = FALSE;//Create two states to compare so that the button doesn't continously count
bool ButtonLast = FALSE;


enum led_mode_t {
    OFF,
    ON,
};
led_mode_t mode = ON;

//Timer variables
UDP UDPClient;
SparkTime rtc;

unsigned long currentTime;
unsigned long lastTime = 0UL;
String timeStr;

//LCD variables
rgb_lcd lcd;

const int colorR = 122;
const int colorG = 122;
const int colorB = 122;


void setup() {

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
    Serial.begin(9600);
}

void loop() {

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

        delay(1000);
        timeStr = "";

    }
    //Update the time
    lastTime = currentTime;

    //play alarm if the mode is ON
    if (mode == ON)
    {
        //the pin, the frequency of tone in hertz, the duration of tone in milliseconds
        tone(speakerPin, 880, 1000);
        delay(1100);
        tone(speakerPin, 440, 1000);
        delay(1100);
    }
    else
    {
    noTone(speakerPin);
    }

    ButtonNow = digitalRead(buttonPin);

    if(ButtonNow == HIGH && ButtonLast == LOW)
    {
        mode = ON;

    }
    else if (ButtonNow == LOW)
    {
        ButtonLast = OFF;
    }
}
