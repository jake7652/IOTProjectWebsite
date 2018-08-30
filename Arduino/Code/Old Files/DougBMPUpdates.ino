    #include <Wire.h>
    #include <Adafruit_Sensor.h>
    #include <Adafruit_BMP085_U.h>

    Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);
     
    void setup(void)
    {
    Serial.begin(9600);
    Serial.println("Pressure Sensor Test"); Serial.println("");
    /* Initialise the sensor */
    if(!bmp.begin())
    {
    /* There was a problem detecting the BMP085 ... check your connections */
    Serial.print("Ooops, no BMP085 detected ... Check your wiring or I2C ADDR!");
    while(1);
    }
    }
     
    void loop(void)
    {
    /* Get a new sensor event */
    sensors_event_t event;
    bmp.getEvent(&event);
    /* Display the results (barometric pressure is measure in hPa) */
    if (event.pressure)
    {
    /* Display atmospheric pressue in hPa */
    Serial.print("Pressure: ");
    Serial.print(event.pressure);
    Serial.println(" hPa");
    
    Serial.print("Pressure: ");
    Serial.print(event.pressure/33.86);
    Serial.println(" inHg");
    
    
    /* Calculating altitude with reasonable accuracy requires pressure *
    * sea level pressure for your position at the moment the data is *
    * converted, as well as the ambient temperature in degress *
    * celcius. If you don't have these values, a 'generic' value of *
    * 1013.25 hPa can be used (defined as SENSORS_PRESSURE_SEALEVELHPA *
    * in sensors.h), but this isn't ideal and will give variable *
    * results from one day to the next. *
    * *
    * You can usually find the current SLP value by looking at weather *
    * websites or from environmental information centers near any major *
    * airport. *
    * *
    * For example, for Paris, France you can check the current mean *
    * pressure and sea level at: http://bit.ly/16Au8ol */
    /* First we get the current temperature from the BMP085 */
    float temperature;
    float ftemperature;
    bmp.getTemperature(&temperature);

    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" C");
    
    ftemperature=((temperature * 9) / 5) + 32;
    Serial.print("Temperature: ");
    Serial.print(ftemperature);
    Serial.println(" F");
     
    /* Then convert the atmospheric pressure, SLP and temp to altitude */
    /* Update this next line with the current SLP for better results */
    float seaLevelPressure = SENSORS_PRESSURE_SEALEVELHPA;
    float MAltitude = bmp.pressureToAltitude(seaLevelPressure,event.pressure,temperature);
    float FAltitude = (MAltitude * 39.370)/12;
    
    Serial.print("Altitude: ");
    Serial.print(MAltitude);
    Serial.println(" m");
    
    Serial.print("Altitude: ");
    Serial.print(FAltitude);
    Serial.println(" f");
    Serial.println("");
    
    }
    else
    {
    Serial.println("Sensor error");
    }
    delay(500);
    }

