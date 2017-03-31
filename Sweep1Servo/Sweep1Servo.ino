// Add the Servo library. This library is standard library
#include <Servo.h>

// Servo 腳位
int ServoPins[] = {3};

// Define our servos
Servo servo1;

// Servo position in degrees
int servoPos;

// Servo 腳位
int delayTime = 30;

void setup()
{

	// Define servos signal inputs (digital PWM 3-5-6-9)
	servo1.attach(ServoPins[0]);
	
	servo1.write(0);
	delay(1000);
	 
}

void loop()
{
// scan from 0 to 180 degrees
	for(int servoPos = 0; servoPos < 180; servoPos++)
	{
		servo1.write(servoPos);
		delay(delayTime);
	}
	
	// now scan back from 180 to 0 degrees
	for(int servoPos = 180; servoPos > 0; servoPos--)
	{
		servo1.write(servoPos);
		delay(delayTime);
	}	
}
