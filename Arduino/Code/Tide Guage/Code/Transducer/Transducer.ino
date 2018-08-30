
   String CommandIn2 = "";         // a string to hold incoming data
   boolean CommandInReady2 = false;  // whether the string is complete


void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);
  Serial.setTimeout(500);
  Serial.flush();

  Serial2.begin(9600);
  Serial2.setTimeout(500);
  Serial2.flush();



}

void loop() {
  // put your main code here, to run repeatedly:


    Serial2.flush();
    Serial2.end();  
    Serial2.begin(9600);
    Serial2.setTimeout(500);
    Serial2.flush();

    if (Serial2.available()) 
      {
        CommandIn2=Serial2.readStringUntil('\n');
        CommandInReady2 = true;
      }


}
