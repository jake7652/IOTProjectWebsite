/*
  SerialPassthrough sketch

  Some boards, like the Arduino 101, the MKR1000, Zero, or the Micro, have one
  hardware serial port attached to Digital pins 0-1, and a separate USB serial
  port attached to the IDE Serial Monitor. This means that the "serial
  passthrough" which is possible with the Arduino UNO (commonly used to interact
  with devices/shields that require configuration via serial AT commands) will
  not work by default.

  This sketch allows you to emulate the serial passthrough behaviour. Any text
  you type in the IDE Serial monitor will be written out to the serial port on
  Digital pins 0 and 1, and vice-versa.

  On the 101, MKR1000, Zero, and Micro, "Serial" refers to the USB Serial port
  attached to the Serial Monitor, and "Serial1" refers to the hardware serial
  port attached to pins 0 and 1. This sketch will emulate Serial passthrough
  using those two Serial ports on the boards mentioned above, but you can change
  these names to connect any two serial ports on a board that has multiple ports.

  created 23 May 2016
  by Erik Nyquist
*/
int counter;

void setup() {
  // initialize serial:
  Serial.begin(9600);
  Serial.flush();
  Serial1.begin(9600);
  Serial1.flush();
  counter=0;
}

void loop() {
  /*
  if (Serial.available()) {      // If anything comes in Serial (USB),
    Serial1.write(Serial.read());   // read it and send it out Serial1 (pins 0 & 1)
  }

  if (Serial1.available()) {     // If anything comes in Serial1 (pins 0 & 1)
    Serial.write(Serial1.read());   // read it and send it out Serial (USB)
  }
*/
counter++;
Serial.print(counter);
Serial.println(serit());
Serial.println("---------");
}

String serit() {
  boolean stringComplete = false;
  String inputString="";
  //int seritcounter=0;
  while (stringComplete == false)
  {
    while (Serial1.available()and stringComplete == false)
    {
      char inChar = (char)Serial1.read();
      if (inChar == 13) 
        {stringComplete = true;
        //Serial.println("CR");
        } 
      if ((inChar >= 48 and inChar <= 57) or inChar == 82)
        {inputString += inChar;
         //seritcounter=seritcounter+1;
         //Serial.print(seritcounter);
         //Serial.println("-"+inputString);
        }    
    } 
  }
  //Strip Off First Character R
  inputString=inputString.substring(1,inputString.length());  
  return (inputString);
}  

