//Libraries that are needed. If one is missing you can add them via Arduino Studio.
#include <RTClib.h>
RTC_DS1307 rtc;
#include <Arduino.h>
#include "HX711.h"
#include "DHT.h"
#include <SPI.h>
#include <SD.h>

// Change the following parameters according to your wiring:
// Scale HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 2;
const int LOADCELL_SCK_PIN = 7;
HX711 scale;

// PIR Sensors wiring
const int PIR_INSIDE_PIN = 5;
const int PIR_ENTRANCE_PIN = 3;

// DHT Sensor wiring
DHT dht(8, DHT22);

//LED wiring
int LEDblau = 10;
int LEDrot = 6;
int LEDgruen = 9;

//SD card attached to Arduino as follows:
//MOSI - pin 11
//ISO - pin 12
//CLK - pin 13
//CS - pin 4 (for MKRZero SD: SDCARD_SS_PIN)

//LED settings
int p = 1000;
int brightness1a = 150;
int brightness1b = 150;
int brightness1c = 150;
int dunkel = 0;

//Initalization of variables
float temperature;
float humidity = 60.0;
long time;
bool motion_inside = false;
bool motion_outside = false;
float weight;
int led_status = 0;
File myFile;
int counter = 0;

void setup() {
  //Initalize RealTimeClock
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1)
      ;
  }
  //Starting a seriell connection
  Serial.begin(9600);
  while (!Serial) {
    ;  //Waiting for connection
  }
  //SD-initallization
  Serial.print("Initializing SD card...");
  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    while (1)
      ;
  }

  //Scale initalization
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  Serial.print("read: \t\t");
  Serial.println(scale.read());
  Serial.print("read average: \t\t");
  Serial.println(scale.read_average(20));
  Serial.println(scale.get_value(5));
  scale.set_scale(450);  // this value is set manually to calibrate the scale
  scale.tare();          // reset the scale to 0

  //PIR Sensors initalization
  pinMode(PIR_INSIDE_PIN, INPUT);
  pinMode(PIR_ENTRANCE_PIN, INPUT);

  //DHT Sensor initalization
  dht.begin();

  //LED initalization
  pinMode(LEDblau, OUTPUT);
  pinMode(LEDgruen, OUTPUT);
  pinMode(LEDrot, OUTPUT);
}

// This function uses the sensor data variables as input and then saves thos to the SD card every time this function is called.
void myPrintSave(long time, float temperature, float humidity, bool motion_inside, bool motion_outside, float weight, int led_status) {
  myFile = SD.open("Igelhaus.txt", FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to test.txt...");
    //save all variabels
    myFile.print(time);
    Serial.println(time);
    myFile.print(",");
    myFile.print(temperature);
    Serial.println(temperature);
    myFile.print(",");
    myFile.print(humidity);
    Serial.println(humidity);
    myFile.print(",");
    myFile.print(motion_inside);
    Serial.println(motion_inside);
    myFile.print(",");
    myFile.print(motion_outside);
    Serial.println(motion_outside);
    myFile.print(",");
    myFile.print(weight);
    Serial.println(weight);
    myFile.print(",");
    myFile.print(led_status);
    Serial.println(led_status);
    myFile.print("\n");
    // close the file:
    myFile.close();
    Serial.println("done.");
    set_led(led_status);
    Serial.println(led_status);
    delay(100);

  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
}
//This function reads all sensor data once it is called and updates the sensors variables.
void readAllSensors() {
  //RTC
  time = rtc.now().unixtime();
  delay(50);
  // Scale
  weight = scale.get_units(10), 5;
  delay(50);
  //PIR
  motion_inside = digitalRead(PIR_INSIDE_PIN);
  delay(50);
  motion_outside = digitalRead(PIR_ENTRANCE_PIN);
  delay(50);
  //DHT
  temperature = dht.readHumidity();
  humidity = dht.readTemperature();
}
//This function is used to change the color of the status RGB led.
void set_led(int led_status) {
  switch (led_status) {
    //Set color to blue
    case 0:
      analogWrite(LEDblau, brightness1a);
      analogWrite(LEDrot, dunkel);
      analogWrite(LEDgruen, dunkel);
      break;
    //Set color to green
    case 1:
      analogWrite(LEDblau, dunkel);
      analogWrite(LEDrot, dunkel);
      analogWrite(LEDgruen, brightness1c);
      break;
    //Set color to red
    case 2:
      analogWrite(LEDblau, dunkel);
      analogWrite(LEDrot, brightness1b);
      analogWrite(LEDgruen, dunkel);
      break;
  }
}

void loop() {
  set_led(led_status);
  //Hedgehog moves into shelter
  if (digitalRead(PIR_ENTRANCE_PIN) == true && led_status == 0) {
    //Hedgehog is in entrance.
    Serial.print("Movement in entrance");
    delay(250);
    Serial.print(scale.get_units(10));
  }
  if (scale.get_units(10) >= 55 && digitalRead(PIR_INSIDE_PIN)) {
    //Hedgehog is in the shelter
    Serial.print("Hedgehog is in the home");
    led_status = 1;
    set_led(led_status);
    readAllSensors();
    myPrintSave(time, temperature, humidity, motion_inside, motion_outside, weight, led_status);
  }

  if (led_status == 1 && digitalRead(PIR_INSIDE_PIN)) {
    //Hedgehog moves in the house
    Serial.print("Hedgehog moves in the house");
    readAllSensors();
    myPrintSave(time, temperature, humidity, motion_inside, motion_outside, weight, led_status);
  }

  if (digitalRead(PIR_ENTRANCE_PIN) == true && scale.get_units(10) <= 5) {
    //Hedgehog leaves the house
    led_status = 0;
    Serial.print("Hedgehog moves in the house");
    readAllSensors();
    myPrintSave(time, temperature, humidity, motion_inside, motion_outside, weight, led_status);
    scale.tare();
  }
  if (scale.get_units(10) > 10 && scale.get_units(10) < 45 && led_status == 1) {
    //Hedgehog might have a problem
    led_status = 2;
    set_led(led_status);
    Serial.print("Hedgehog might have a problem");
    readAllSensors();
    myPrintSave(time, temperature, humidity, motion_inside, motion_outside, weight, led_status);
  }
  //Loop to gather continous data
  Serial.print("Saving Loop");
  if (counter == 100) {
    counter = 0;
    readAllSensors();
    myPrintSave(time, temperature, humidity, motion_inside, motion_outside, weight, led_status);
  } else {
    counter += 1;
    delay(10);
  }
}

// End of program
