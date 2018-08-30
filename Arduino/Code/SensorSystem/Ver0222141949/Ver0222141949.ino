#include <Adafruit_BMP180_U.h>

/* Version 0221141301 */
/* Renamed to BMP180 from BMP085 */
/* Restructured main loop for output to host device */

    /* Calculating altitude with reasonable accuracy requires pressure */
    /* sea level pressure for your position at the moment the data is */
    /* converted, as well as the ambient temperature in degress */
    /* celcius. If you don't have these values, a 'generic' value of */
    /* 1013.25 hPa can be used (defined as SENSORS_PRESSURE_SEALEVELHPA */
    /* in sensors.h), but this isn't ideal and will give variable */
    /* results from one day to the next. */
    /* */
    /* You can usually find the current SLP value by looking at weather */
    /* websites or from environmental information centers near any major */
    /* airport. */

    #include <Wire.h>
    #include <Adafruit_Sensor.h>
    #include <Adafruit_BMP180_U.h>

/* Create and instance of the temp, pressure, altitude sensor from BMP180 Library */
    Adafruit_BMP180_Unified bmp = Adafruit_BMP180_Unified(10085);

    void setup(void)
        /*  Setup Arduino  */
    {
    /* Setup Serial to 9600 */  
    Serial.begin(9600);
    
    Serial.println("Establishing Serial Connection to Sensor Board"); 
    Serial.println("");
    
    /* Initialise the temp, pressure, altitude sensor from BMP180 Library */
    if(!bmp.begin())
      {
      /* There was a problem detecting the BMP180 ... check your connections */
            Serial.print("BMP180 not detected ... Check your wiring or I2C ADDR!");
            while(1); /* Hold here */
      }
    /* End Arduino Setup  */
    }
     
    void loop(void)
    {
    /* Setup Variables */  

        float AtmosphericPressure;
        float Temperature;
        float seaLevelPressure = SENSORS_PRESSURE_SEALEVELHPA;
        float Altitude;
        float FAltitude;
   
    /* Get a new temp, pressure, and altitude sensor event */
    sensors_event_t event;
    bmp.getEvent(&event);
    
      /* Display the results (barometric pressure is measure in hPa) */
      if (event.pressure)
           {
             /* Get Atmospheric Pressure in hPa */
             AtmosphericPressure = event.pressure;
             
             /* Get Temperature in Celsius */
             bmp.getTemperature(&Temperature);
             
             /* Calculate Alititude */
             Altitude = bmp.pressureToAltitude(seaLevelPressure,AtmosphericPressure,Temperature);
             
             
                /* Display atmospheric pressue in hPa */
                Serial.print("Pressure: ");
                Serial.print(AtmosphericPressure);
                Serial.println(" hPa");
                
                Serial.print("Pressure: ");
                Serial.print(AtmosphericPressure/33.86);
                Serial.println(" inHg");
            
                Serial.print("Temperature: ");
                Serial.print(Temperature);
                Serial.println(" C");
                
                Serial.print("Temperature: ");
                Serial.print(((Temperature * 9) / 5) + 32);
                Serial.println(" F");
                
                Serial.print("Altitude: ");
                Serial.print(Altitude);
                Serial.println(" m");
                
                Serial.print("Altitude: ");
                Serial.print((Altitude * 39.370)/12);
                Serial.println(" f");
                Serial.println("");
    
            }
    else
            {
                Serial.println("Sensor error");
            }
            
   /* Time Delay before next read */        
    delay(1000);
    
    /* End Loop */
    }

