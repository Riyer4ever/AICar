// include some head files
#include <Servo.h>            // Third-party servo library
#include "Wire.h"                   // I2C library for tracking, color recognition, and ultrasonic detection
#include "MH_TCS34725.h"            // Color sensor library

// Low power mode definition file
#ifdef __AVR__
#include "avr/power.h"
#endif


// Color sensor channel configuration
MH_TCS34725 tcs = MH_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_16X); 

// define servo pin
Servo myservo;


// function of initilization
void setup()
{


}

// function of loop program
void loop()
{

}
