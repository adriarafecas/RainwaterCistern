#include <Wire.h>  // Include the Wire library for I2C communication
#include <RTClib.h>  // Include the RTClib library for RTC clock handling
#include <LiquidCrystal_I2C.h>  // Include the LiquidCrystal_I2C library for I2C LCD handling
#include <UltraDistSensor.h>  // Include the UltraDistSensor library for ultrasonic sensor handling
#include <SD.h>  // Include the SD library for SD card handling

RTC_DS1307 rtc;                      // Instance of the RTC clock
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Instance of the LCD with I2C address and dimensions 16x2
UltraDistSensor distSensor;          // Instance of the ultrasonic sensor
File logfile;                         // File for logging data

const int led10 = 2;   // Pin of the LED that lights up when the level is below 10%
const int led90 = 3;   // Pin of the LED that lights up when the level is above 90%
const int chipSelect = 10; // SD card CS pin

const float rad_deposito = 1.8;  // Radius of the tank
const float hight = 3;          // Height of the tank

void setup() {
  pinMode(led10, OUTPUT);  // Configure the LED 10% pin as output
  pinMode(led90, OUTPUT);  // Configure the LED 90% pin as output

  Serial.begin(9600);  // Start serial communication for debugging

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);  // Infinite loop if RTC not found
  }

  if (!rtc.isrunning()) {
    Serial.println("RTC is NOT running, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));  // Set the time based on compilation
  }

  Serial.print("Initializing SD card...");
  if (SD.begin(chipSelect)) {
    Serial.println("card initialized.");
    openLogFile();
  } else {
    Serial.println("Card failed, or not present");
    while (1);  // Infinite loop if SD card initialization fails
  }

  lcd.begin(16, 2);  // Initialize the LCD with 16 columns and 2 rows
  lcd.setBacklight(HIGH);  // Turn on the LCD backlight
  lcd.print("Water Level:");  // Display a message on the LCD
  distSensor.attach(5, 6);  // Connect the ultrasonic sensor to Trigger and Echo pins
  delay(1000);  // Wait for 1 second
}

void loop() {
  DateTime now = rtc.now();  // Get the current date and time from the RTC
  float distance = distSensor.distanceInCm();  // Get the distance measured by the ultrasonic sensor
  float liters = calculateLiters(distance);  // Calculate the volume of water based on the distance

  lcd.clear();  // Clear the current content of the LCD
  lcd.setCursor(0, 0);  // Set the cursor to the first row, first column
  lcd.print("Distan_cm:");
  lcd.print(distance);  // Display the distance on the LCD
  lcd.setCursor(0, 1);  // Set the cursor to the second row, first column
  lcd.print("Liters:");
  lcd.print(liters);  // Display the volume on the LCD

  logData(now, distance, liters);  // Log data every 10s

  float percent = distance / hight;
  if (percent <= 10) {
    digitalWrite(led90, HIGH);  // Turn on the 90% LED
    digitalWrite(led10, LOW);  
  } else if (percent >= 90) {
    digitalWrite(led90, LOW);  
    digitalWrite(led10, HIGH);  // Turn on the 10% LED
  } else {
    digitalWrite(led90, LOW);  // Turn off
    digitalWrite(led10, LOW);  // Turn off
  }

  delay(10000);  // Wait for 10 seconds before executing the loop again
}

float calculateLiters(float distance) {
  // Adjust the formula based on the relationship between distance and volume
  // You can use an empirical formula based on the tank's characteristics
  float volume = (PI * rad_deposito * rad_deposito * distance / 100);  // Volume occupied by water
  float total_volume = (PI * rad_deposito * rad_deposito * hight);       // Total volume of the tank
  return (total_volume - volume) * 1000;  // Difference between total volume and occupied volume
}

void logData(DateTime now, float distance, float liters) {
  logfile = SD.open("datalog.txt", FILE_WRITE);
  if (logfile) { 
    logfile.print(now.year(), DEC);
    logfile.print('/');
    logfile.print(now.month(), DEC);
    logfile.print('/');
    logfile.print(now.day(), DEC);
    logfile.print(' ');
    logfile.print(now.hour(), DEC);
    logfile.print(':');
    logfile.print(now.minute(), DEC);
    logfile.print(':');
    logfile.print(now.second(), DEC);
    logfile.print(" - Distance: ");
    logfile.print(distance);
    logfile.print(" cm, Liters: ");
    logfile.print(liters);
    logfile.println(" L");
    logfile.close();
    Serial.println("Data logged!");
  } else {
    Serial.println("Error opening datalog.txt");
  }
}

void openLogFile() {
  logfile = SD.open("datalog.txt", FILE_WRITE);
  if (logfile) {
    Serial.println("Log file opened!");
    logfile.close();
  } else {
    Serial.println("Error opening datalog.txt");
  }
}
