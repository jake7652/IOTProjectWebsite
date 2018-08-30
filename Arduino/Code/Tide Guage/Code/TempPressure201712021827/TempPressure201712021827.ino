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
// One Wire Declarations
#include <Wire.h>
#include <OneWire.h>
// Data wire is plugged into pin 10 on the Arduino
#define ONE_WIRE_BUS 10
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Dallas Temperature Declarations
#include <DallasTemperature.h>
// Variables for Dallas Temperature Sensors
float DB18B20A=0; // Fahrenheit
float DB18B20B=0; // Fahrenheit
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// SPI Libary Declarations
#include <SPI.h>

// Adafruit Master and BMP280 Sensor Library Declarations
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
// Variables for BMP280 with conversations
float BMP280Temp=0; // Fahrenheit
float BMP280Pressure=0; // InHg
float BMP280Altitude=0; // Feet
// BMP280 Sensor object
Adafruit_BMP280 bmp; // I2C

// DHTXX Declarations
#include "DHT.h"
// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
// Change number to pin read
#define DHTPIN 2
float DHT22Humidity=0; // Percentage Saturation
float DHT22Temperature=0; // Fahrenheit
float DHT22HeatIndex=0; // Fahrenheit
// DHT Sensor Object
DHT dht(DHTPIN, DHTTYPE);

/* DS3231 RTC Library */
#include <DS3231.h> 
/* Real Time Clock Parameters */
DS3231 RTClock;
#define RTCReadModeTime 0
#define RTCReadModeTemperature 1
String RTCStringDateTime = "";
String RTCSYear, RTCSMonth, RTCSDay, RTCSDOW, RTCSHour, RTCSMinute, RTCSSecond, RTCSTemperature;
// Fix Later Should not need after fixing Library
bool h12;
bool PM;

//Transducer
String STranducer;
boolean stringComplete = false;



void setup() {
  Serial.begin(9600);
  Serial.setTimeout(500);
  Serial.flush();
  Serial1.begin(9600);
  Serial1.setTimeout(500);
  Serial1.flush();
  Serial.println(F("Tide Gauge Test"));
;


  //Setup BMP280 Sensor
  if (!bmp.begin()) {  
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
    while (1);
  }

  //Setup DB20 Temperature Sensor
    sensors.begin(); // IC Default 9 bit. If you have troubles consider upping it 12. Ups the delay giving the IC more time to process the temperature measurement 

  //Setup DHT Temperature and Humidity Sensor
    dht.begin();

  //Serial1 Initialize for Transducer
  Serial1.begin(9600);
  Serial1.setTimeout(500);
  Serial1.flush();
   
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

    //RTC 
    RTCStringDateTime=(RTCRead(RTCReadModeTime));
    RTCSTemperature=(RTCRead(RTCReadModeTemperature));

    //Transducer
    int range = EZread();
    if(stringComplete)
        {stringComplete = false;
        Serial.print("Range ");
        Serial.println(range);}
   
    
    //Print to Serial
    Serial.println("");

    Serial.print("RTC - ");
    Serial.print("Time : ");
    Serial.print(RTCStringDateTime);
    Serial.print(" Temperature : ");
    Serial.print(RTCSTemperature);
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

    Serial.print("Transducer - ");
    //Serial.print(STranducer);

    Serial.println(""); 

}



String RTCRead(int Mode)
{
  byte RTCYear, RTCMonth, RTCDay, RTCDOW, RTCHour, RTCMinute, RTCSecond, RTCTemperature;
  //String RTCSYear, RTCSMonth, RTCSDay, RTCSDOW, RTCSHour, RTCSMinute, RTCSSecond, RTCSTemperature;
  
  Wire.begin();
  switch (Mode)
  {
      case RTCReadModeTime:
        RTClock.getTime(RTCYear, RTCMonth, RTCDay, RTCDOW, RTCHour, RTCMinute, RTCSecond);
        //***************************************************************************************
        // Temporary Fix Need to Fix Library
        // **************************************************************************************
        RTCHour=RTClock.getHour(h12, PM);
        
          if (RTCYear <= 9) 
            {
              RTCSYear="200"+String(RTCYear);
            }
          else 
            {
              RTCSYear="20"+String(RTCYear);
            }  
          if (RTCMonth <= 9) 
            {
              RTCSMonth="0"+String(RTCMonth);
            }
          else 
            {
              RTCSMonth=String(RTCMonth);
            }
          if (RTCDay <= 9) 
            {
              RTCSDay="0"+String(RTCDay);
            }
          else 
            {
              RTCSDay=String(RTCDay);
            }  
          if (RTCHour <= 9) 
            {
              RTCSHour="0"+String(RTCHour);              
            }
          else 
            {
              RTCSHour=String(RTCHour);
            }  
          if (RTCMinute <= 9) 
            {
              RTCSMinute="0"+String(RTCMinute);
            }
          else 
            {
              RTCSMinute=String(RTCMinute);
            } 
          if (RTCSecond <= 9) 
            {
              RTCSSecond="0"+String(RTCSecond);
            }
          else 
            {
              RTCSSecond=String(RTCSecond);
            }    
         return (RTCSYear + RTCSMonth + RTCSDay + RTCSHour + RTCSMinute + RTCSSecond);  
         
       case RTCReadModeTemperature :
         RTCSTemperature=String(((RTClock.getTemperature()*1.8)+32));
         return (RTCSTemperature);  
  }  
}

int EZread()
{
int result;
char inData[5];
int index = 0;

//sonarSerial.flush();
Serial1.flush();

while (stringComplete == false)
{// Serial.print ("reading ");
  //if (sonarSerial.available())
  if (Serial1.available())
  {
    //char rByte = sonarSerial.read();
    char rByte = Serial1.read();

    if(rByte == 'R')
    {
      while (index < 4)
      {
        if (Serial1.available())
        //if(sonarSerial.available())
        {
          //inData[index] = sonarSerial.read();
          inData[index] = Serial1.read();
          //Serial.println(inData[index]);
          index++;
        }
      }
      inData[index] = 0x00;
    }
    rByte = 0;
    index = 0;
    stringComplete = true;
    result = atoi(inData);
  }
}
return result;
}




