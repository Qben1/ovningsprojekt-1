/*
* Name: clock and temp project
* Author: Victor Huke
* Date: 2024-10-10
* Description: This project uses a ds3231 to measure time and displays the time to an 1306 oled display,
* Further, it measures temprature with a analog temprature module and displays a mapped value to a 9g-servo-motor
*/

// Include Libraries
#include <RTClib.h>   // Library for RTC (Real-Time Clock) module
#include <Wire.h>     // Library for I2C communication
#include "U8glib.h"   // Library for OLED display
#include <Servo.h>    // Library for controlling the servo motor

// Init constants
const int tempPin = A1;  // Define the analog pin connected to the temperature sensor

// Init global variables
int pos = 0;  // Initialize servo position variable

// construct objects
Servo myservo;                               // Create a Servo object for controlling the servo
RTC_DS3231 rtc;                              // Create an RTC object for interacting with the DS3231 module
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NO_ACK); // Create an OLED display object (SSD1306, 128x64 resolution)

void setup() {
  // Init communication
  Serial.begin(9600);   // Start serial communication at 9600 baud rate for debugging or data output
  Wire.begin();         // Initialize I2C communication

  // Init hardware
  pinMode(tempPin, INPUT);  // Set temperature sensor pin as input
  rtc.begin();              // Initialize the RTC module
  myservo.attach(7);        // Attach the servo to pin 7

  // Set pins 9, 10, and 11 as outputs using a for loop
  for (int pin = 9; pin <= 11; pin++) {
    pinMode(pin, OUTPUT);   // Set pins 9, 10, and 11 (RGB LED control pins) as outputs
  }

  // Setting
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));  // Set the RTC to the current time (compile time)
  u8g.setFont(u8g_font_helvB14);                   // Set the font for the OLED display
}

void loop() {

  servoWrite(getTemp());  // Move servo based on temperature

  // Write time and temperature to OLED
  oledWrite("TIME " + String(getTime()), "TEMP " + String(getTemp()) + " " + String(char(176)) + "C"); // Display time and temperature on OLED

  delay(1000);  // Delay 1 second between updates
  
  // Control RGB LED based on temperature
  rgbControl(getTemp());  // Adjust the RGB LED color based on the temperature
}

/*
* This function reads time from an ds3231 module and package the time as a String
* Parameters: Void
* Returns: time in hh:mm:ss as String
*/
String getTime() {

  DateTime now = rtc.now();  // Get the current time from the RTC module
  return String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second());  // Format the time as hh:mm:ss
}

/*
* This function reads an analog pin connected to an analog temprature sensor and calculates the corresponding temp
* Parameters: Void
* Returns: temprature as float
*/
float getTemp() {
  float temp = 0;
  float R1 = 10000;  // value of R1 on board (10k ohms)
  float logR2, R2, T; // Variables for calculations
  float c1 = 0.001129148, c2 = 0.000234125, c3 = 0.0000000876741;  // Steinhart-Hart coefficients for the thermistor
  int Vo = analogRead(tempPin);  // Read analog value from the temperature sensor
  R2 = R1 * (1023.0 / (float)Vo - 1.0);  // Calculate resistance on the thermistor
  logR2 = log(R2);  // Take the logarithm of the resistance
  temp = (1.0 / (c1 + c2 * logR2 + c3 * logR2 * logR2 * logR2));  // Calculate temperature in Kelvin using Steinhart-Hart equation
  temp = temp - 273.15;  // Convert Kelvin to Celsius
  return temp;  // Return temperature in Celsius
}

/*
* This function takes a string and draws it to an oled display
* Parameters: - text1: First string to write to display
*             - text2: Second string to write to display
* Returns: void
*/
void oledWrite(String text1, String text2) {
  u8g.firstPage();  // Begin the first page of the OLED display (for handling display buffer)
  do {
    u8g.drawStr(0, 20, text1.c_str());  // Draw first string at (0, 20) position on OLED
    u8g.drawStr(0, 50, text2.c_str());  // Draw second string at (0, 50) position on OLED
  } while (u8g.nextPage());  // Continue to the next page of the OLED until finished
}

/*
* Takes a temperature value and maps it to a corresponding degree on a servo
* Parameters: - value: temperature to map
* Returns: void
*/
void servoWrite(float value) {
  float pos = map(value, 22, 30, 0, 360);  // Map temperature range (22 to 30°C) to servo position range (0 to 360°)
  myservo.write(pos);  // Set servo to the calculated position
}

/*
* Controls the RGB diode based on the current temperature
* Parameters: - temp: current temperature value
* Returns: void
*/
void rgbControl(float temp) {
  // Control RGB LEDs based on temperature thresholds
  if (temp < 25) {
    digitalWrite(9, LOW);    // Red LED off
    digitalWrite(10, LOW);   // Green LED off
    digitalWrite(11, HIGH);  // Blue LED on (Cold)
  }
  else if (temp > 25 && temp < 28) {
    digitalWrite(9, HIGH);   // Red LED on
    digitalWrite(10, HIGH);  // Green LED on (Yellow for moderate temperature)
    digitalWrite(11, LOW);   // Blue LED off
  }
  else if (temp > 28 && temp < 30) {
    digitalWrite(9, HIGH);   // Red LED on
    digitalWrite(10, HIGH);  // Green LED on
    digitalWrite(11, HIGH);  // Blue LED on (White for high temperature)
  }
  else {
    digitalWrite(9, HIGH);   // Red LED on
    digitalWrite(10, LOW);   // Green LED off
    digitalWrite(11, LOW);   // Blue LED off (Red for very high temperature)
  }
}