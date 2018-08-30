
boolean stringComplete = false;

void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);
Serial1.begin(9600);
delay(500);

pinMode(5, OUTPUT);          // sets the digital pin 13 as output
}

void loop() {
  int range = EZread();
  if(stringComplete)
  {stringComplete = false;
   Serial.print("Range ");
   Serial.println(range);}
}

int EZread()
{
int result;
char inData[5];
int index = 0;
Serial1.flush();

while (stringComplete == false)
{// Serial.print ("reading ");
  //if (sonarSerial.available())
  if (Serial1.available())
  {
    char rByte = Serial1.read();

    if(rByte == 'R')
    {
      while (index < 4)
      {
        if (Serial1.available())
        {
          inData[index] = Serial1.read();
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



