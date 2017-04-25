#include "Adafruit_TSL2561_U.h"
#include "Adafruit_BMP085.h"

/*
    Wiring
    ------
    BMP085 Vcc to 3.3V
    BMP085 GND to GND
    BMP085 SCL to D1
    BMP085 SDA to D0

    TSL Vcc to 3.3V
    TSL GND to GND
    TSL SCL to D1
    TSL SDA to D0
*/

Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);
Adafruit_BMP085 bmp;

float read_p0(float myAltitude, float ABSpressure)
{
    float p0 = ABSpressure / pow((1.0 - ( myAltitude / 44330.0 )), 5.255);  
    return p0;
}

// Publish Pressure, Altitude
void PublishBMP085Info()
{
    float temperature = bmp.readTemperature();
    Serial.print("Temperature = ");
    Serial.print(temperature);
    Serial.println(" *C");
    
    float pressure = bmp.readPressure() / 100.0f;
    Serial.print("Pressure = ");
    Serial.print(pressure);
    Serial.println(" Pa");
    
    float p0 = read_p0(951, pressure);
    Serial.print("P0 = ");
    Serial.print(p0);
    Serial.println(" Pa");
    
    // Calculate altitude assuming 'standard' barometric
    // pressure of 1013.25 millibar = 101325 Pascal
    Serial.print("Altitude = ");
    Serial.print(bmp.readAltitude());
    Serial.println(" meters");

  // you can get a more precise measurement of altitude
  // if you know the current sea level pressure which will
  // vary with weather and such. If it is 1015 millibars
  // that is equal to 101500 Pascals.
    Serial.print("Real altitude = ");
    Serial.print(bmp.readAltitude(p0 * 100));
    Serial.println(" meters");
    
    char szEventInfo[64];
    sprintf(szEventInfo, "Temperature=%s °C, Pressure=%s hPa, Altitude=%s m", String(temperature, 2).c_str(), String(pressure, 2).c_str(), String(bmp.readAltitude(), 2).c_str());
    
    Particle.publish("bmpo85info", szEventInfo);
}

/**************************************************************************/
/*
    Displays some basic information on this sensor from the unified
    sensor API sensor_t type (see Adafruit_Sensor for more information)
*/
/**************************************************************************/
void displaySensorDetails(void)
{
  sensor_t sensor;
  tsl.getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" lux");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" lux");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" lux");  
  Serial.println("------------------------------------");
  Serial.println("");
  delay(500);
}

/**************************************************************************/
/*
    Configures the gain and integration time for the TSL2561
*/
/**************************************************************************/
void configureSensor(void)
{
    /* You can also manually set the gain or enable auto-gain support */
    // tsl.setGain(TSL2561_GAIN_1X);      /* No gain ... use in bright light to avoid sensor saturation */
    // tsl.setGain(TSL2561_GAIN_16X);     /* 16x gain ... use in low light to boost sensitivity */
    tsl.enableAutoRange(true);            /* Auto-gain ... switches automatically between 1x and 16x */
    
    /* Changing the integration time gives you better sensor resolution (402ms = 16-bit data) */
    // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);      /* fast but low resolution */
    tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_101MS);  /* medium resolution and speed   */
    // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);  /* 16-bit data but slowest conversions */
    
    /* Update these values depending on what you've set above! */  
    Serial.println("------------------------------------");
    Serial.print  ("Gain:         "); Serial.println("Auto");
    Serial.print  ("Timing:       "); Serial.println("101 ms");
    Serial.println("------------------------------------");
}

// Blink LED and wait for some time
void BlinkLED()
{
    static int state = HIGH;
    
    digitalWrite(D5, state);
    state = !state;
}

void PublishTSL2561Info()
{
    /* Get a new sensor event */ 
    sensors_event_t event;
    tsl.getEvent(&event);
 
    /* Display the results (light is measured in lux) */
    if (event.light)
    {
        Serial.print(event.light);
        Serial.println(" lux");
        char szEventInfo[16];
        sprintf(szEventInfo, "Lux=%s lux", String(event.light, 2).c_str());
    
        Particle.publish("tsl2561", szEventInfo);
    }
    else
    {
        /* If event.light = 0 lux the sensor is probably saturated
        and no reliable data could be generated! */
        Serial.println("Sensor overload");
    }
}

void setup()
{
    Serial.begin(19200);
    pinMode(D7, OUTPUT);
    pinMode(D5, OUTPUT);

    if (!bmp.begin()) {
        Serial.println("Could not find a valid BMP085 sensor, check wiring!");
        while (1) {}
    }

    /* Initialise the sensor */
    if(!tsl.begin())
    {
        /* There was a problem detecting the ADXL345 ... check your connections */
        Serial.print("Ooops, no TSL2561 detected ... Check your wiring or I2C ADDR!");
        while(1) {}
    }
    
    /* Display some basic information on this sensor */
    displaySensorDetails();
    
    /* Setup the sensor gain and integration time */
    configureSensor();
}

void loop()
{
    // Publish events. Wait for 2 second between publishes
    PublishBMP085Info(); 
    
    BlinkLED();
    
    PublishTSL2561Info();
    
    delay(30000);
}
