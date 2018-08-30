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

//Transducer
String inByte;



void setup() {
  Serial.begin(9600);
  Serial.setTimeout(500);
  Serial.flush();
  Serial.println("Serial 0 Setup");

  //Serial1 Initialize for Transducer
  Serial1.begin(9600);
  Serial1.setTimeout(500);
  Serial1.flush();
  Serial.println("Serial 1 Setup");
   
}

void loop() {

    Serial.println("A");
    //Transducer
    //Serial1.flush();
    //Serial1.end();  
    //Serial1.begin(9600);
    //Serial1.setTimeout(500);
    //Serial1.flush();

    if (Serial1.available()) {inByte = Serial1.readStringUntil("\n");}
    Serial.println(inByte);
    Serial.println("B");
    
 
}

