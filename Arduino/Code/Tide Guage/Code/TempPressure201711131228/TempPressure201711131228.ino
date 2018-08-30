/***************************************************************************
  This is a library for the BMP280 humidity, temperature & pressure sensor

  Designed specifically to work with the Adafruit BMEP280 Breakout 
  ----> http://www.adafruit.com/products/2651

  These sensors use I2C or SPI to communicate, 2 or 4 pins are required 
  to interface.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit andopen-source hardware by purchasing products
  from Adafruit!

  Written by Limor Fried & Kevin Townsend for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ***************************************************************************/

#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include <SPI.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

#include "DHT.h"

#include "RTClib.h"

// Data wire is plugged into pin 10 on the Arduino
#define ONE_WIRE_BUS 10

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
// Change number to pin read
#define DHTPIN 2

// Variables for Sensors
float BMP280Temp=0; // vFahrenheit
float BMP280Pressure=0; // Pa
float BMP280Altitude=0; // Feet
float DB18B20A=0; // Fahrenheit
float DB18B20B=0; // Fahrenheit
float DHT22Humidity=0; // Percentage Saturation
float DHT22Temperature=0; // Fahrenheit
float DHT22HeatIndex=0; // Fahrenheit

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// BMP280 Sensor object
Adafruit_BMP280 bmp; // I2C

// Initialize DHT sensor.
// Note that older versions of this library took an optional third parameter to
// tweak the timings for faster processors.  This parameter is no longer needed
// as the current DHT reading algorithm adjusts itself to work on faster procs.
DHT dht(DHTPIN, DHTTYPE);

RTC_DS3231 rtc;

void setup() {
  Serial.begin(9600);
  Serial.println(F("Tide Gauge Test"));

  //Setup BMP280 Sensor
  if (!bmp.begin()) {  
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
    while (1);
  }

  //Setup DB20 Temperature Sensor
    sensors.begin(); // IC Default 9 bit. If you have troubles consider upping it 12. Ups the delay giving the IC more time to process the temperature measurement 

  //Setup DHT Temperature and Humidity Sensor
    dht.begin();

  //Setup DS3231 Realtime Clock
    if (! rtc.begin()) {
      Serial.println("Couldn't find RTC");
      while (1);

  //Need to update DS3231 from computer clock
  // following line sets the RTC to the date & time this sketch was compiled
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // This line sets the RTC with an explicit date & time, for example to set
  // January 21, 2014 at 3am you would call:
  rtc.adjust(DateTime(2017, 11, 13, 19, 10, 0));
      
  }    
}

void loop() {

    //BMP280 Pressure / Temperature Sensor
    BMP280Temp=((bmp.readTemperature()*1.8)+32); //F
    BMP280Pressure=(bmp.readPressure()*.000295);
    BMP280Altitude=(bmp.readAltitude(1025.7)*3.2084); //f

    //DB18B20 Temperature Sensor (2 on Bus)
    sensors.requestTemperatures(); // Send the command to get temperatures
    DB18B20A=((sensors.getTempCByIndex(0)*1.8)+32); // Why "byIndex"? You can have more than one IC on the same bus. 0 refers to the first IC on the wire
    DB18B20B=((sensors.getTempCByIndex(1)*1.8)+32); // Why "byIndex"? You can have more than one IC on the same bus. 0 refers to the first IC on the wire

    //DHT22 Humidity / Temperature Sensor
    DHT22Humidity=dht.readHumidity();
    DHT22Temperature=((dht.readTemperature()*1.8)+32);
    DHT22HeatIndex=dht.computeHeatIndex(DHT22Temperature, DHT22Humidity);

    //RTC SD3231
    // Need to add
    
    //Print to Serial

    Serial.print("RTC - ");
    Serial.println("");

    
    Serial.print("BMP280 - ");
    Serial.print("Temperature : ");
    Serial.print(BMP280Temp);
    Serial.print(" Pressure : ");
    Serial.print(BMP280Pressure);
    Serial.print(" Altitude : ");
    Serial.print(BMP280Altitude);
    Serial.println("");

    Serial.print("DB18B20 - ");
    Serial.print("Temperature 1 : ");
    Serial.print(DB18B20A);
    Serial.print(" Temperature 2 : ");
    Serial.print(DB18B20B);
    
    Serial.println("");

    Serial.print("DHT22 - ");
    Serial.print(" Temperature : ");
    Serial.print(DHT22Temperature);
    Serial.print(" Humidity : ");
    Serial.print(DHT22Humidity);
    Serial.print(" Heat Index : ");
    Serial.print(DHT22HeatIndex);

    Serial.println("");


    
}
