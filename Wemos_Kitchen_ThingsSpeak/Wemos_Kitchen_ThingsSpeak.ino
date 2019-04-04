/* This sketch read data and sends then to thingspeak.com
 *
 * This project is intended for mounting inside the house.
 * 
 * TODO: make settings configurable in eeprom.
 */

#include <ThingSpeak.h>

#include <OneWire.h>
#include <DallasTemperature.h>

#include "IoTBase.h"


//*** ThingSpeak setup (https://thingspeak.com/channels/148872/private_show)
// static unsigned long theChannelNumber = 148872;
// static const char *theWriteAPIKey = "6Q1ETMKASAL6U6DP";



//*** DS 18B20 ***
// Data wire is plugged into port 12 on the Wemos
#define ONE_WIRE_BUS 12
#define TEMPERATURE_PRECISION 12

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// arrays to hold device addresses
DeviceAddress thermometer1;
DeviceAddress thermometer2;
DeviceAddress thermometer3;
DeviceAddress thermometer4;

long startMillis;

void setup() 
{
    startMillis = millis();
    
    Serial.begin(9600);
    delay(10);
    Serial.println();
    Serial.println( "Kitchen v1.0" );
    
    Serial.println();
    
    IoTBase.ConnectToAP( "Kitchen_v1_0");
    
    // Start up the DS-temp sensor library:
    sensors.begin();
    
    if (!sensors.getAddress(thermometer1, 0)) Serial.println("Unable to find address for Device 1"); else printAddress( thermometer1 );
    if (!sensors.getAddress(thermometer2, 1)) Serial.println("Unable to find address for Device 2"); else printAddress( thermometer2 );
    if (!sensors.getAddress(thermometer3, 2)) Serial.println("Unable to find address for Device 3"); else printAddress( thermometer3 );
    if (!sensors.getAddress(thermometer4, 3)) Serial.println("Unable to find address for Device 4"); else printAddress( thermometer4 );
    
    // set the resolution:
    sensors.setResolution(thermometer1, TEMPERATURE_PRECISION);
    sensors.setResolution(thermometer2, TEMPERATURE_PRECISION);
    sensors.setResolution(thermometer3, TEMPERATURE_PRECISION);
    sensors.setResolution(thermometer4, TEMPERATURE_PRECISION);
    
    IoTBase.begin();
    delay(15000);
}




// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
    for (uint8_t i = 0; i < 8; i++)  {
        // zero pad the address if necessary
        if (deviceAddress[i] < 16) Serial.print("0");
        Serial.print(deviceAddress[i], HEX);
    }
    Serial.println();
}



unsigned long nextSend = 0;

  
void loop() 
{
    if (nextSend < millis()) {
        nextSend += 120000; // check for send once every 2 minutes
        Send();
    }

    delay( 5000 );
}


float lastTemp = 0;
int ticsSinceLastSend = 0;

void Send()
{
    long startTime = millis();
  
    // *** Send temperatur:
    // call sensors.requestTemperatures() to issue a global temperature 
    // request to all devices on the bus
    Serial.print("Requesting temperatures...");
    sensors.requestTemperatures();
    Serial.println("DONE");
  
    float tempC1 = sensors.getTempC(thermometer1);
    float tempC2 = sensors.getTempC(thermometer2);
    float tempC3 = sensors.getTempC(thermometer3);
    float tempC4 = sensors.getTempC(thermometer4);
   
        long endMillis = millis();
        float duration = (endMillis-startTime)/1000.0;
      
        // Send data to backend:
        IoTBase.initData();
        IoTBase.addValue( tempC1 );
        IoTBase.addValue( tempC2 );
        IoTBase.addValue( tempC3 );
        IoTBase.addValue( tempC4 );
        IoTBase.addValue( duration );

        int ret = IoTBase.sendData( "KITC" );
        Serial.println( String("Status from backend: " )+ ret );
        
        Serial.println( "Send " + String(tempC1) + ", " + String(tempC2) + ", " + String(tempC3) + ", " + String(tempC4) 
          + ", " + String(duration) + ". Response from thingspeak: " + String(ret) );
}





