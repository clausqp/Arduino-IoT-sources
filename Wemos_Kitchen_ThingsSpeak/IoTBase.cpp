
#include <ESP8266WiFi.h>
#include "IoTBase.h"
#include <ESP8266HTTPClient.h>


//*** Network ***
const char* ssid     = "egeCL";
const char* password = "egeCL150";
static WiFiClient client;


IoTBaseClass IoTBase;



void IoTBaseClass::ConnectToAP( char *iotName )
{
    Serial.println("Attempting to Connect");

    Serial.print("Connecting to ");
    Serial.println(ssid);

    int x = 0;
    int dlyTime = 200;
    WiFi.hostname( iotName );
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(dlyTime); Serial.print(".");
        x += dlyTime;
    }
    int secs = x/1000;
    int ms = x - secs*1000;
    Serial.print("WiFi connected in "); Serial.print( secs ); Serial.print(" secs. "); Serial.print(ms); Serial.println(" ms");
    Serial.print("IP address: "); Serial.println(WiFi.localIP());
}


    bool IoTBaseClass::begin()
    {
#ifdef PRINT_DEBUG_MESSAGES
        Serial.print("ts::tsBegin");
#endif
        this->setClient( client);
        this->setServer();
        return true;
    };
    

    void IoTBaseClass::setServer(const char * customHostName, unsigned int port)
    {
#ifdef PRINT_DEBUG_MESSAGES
        Serial.print("ts::setServer  (URL: \""); Serial.print(customHostName); Serial.println("\")");
#endif
        this->customIP = INADDR_NONE;
        this->customHostName = customHostName;
        this->port = port;
    };

    void IoTBaseClass::setServer(IPAddress customIP, unsigned int port)
    {
#ifdef PRINT_DEBUG_MESSAGES
        Serial.print("ts::setServer  (IP: \""); Serial.print(customIP); Serial.println("\")");
#endif
        this->customIP = customIP;
        this->customHostName = NULL;
        this->port = port;
    };

    void IoTBaseClass::setServer()
    {
#ifdef PRINT_DEBUG_MESSAGES
        Serial.print("ts::setServer  (default)");
#endif
        this->customIP = INADDR_NONE;
        this->customHostName = NULL;
        this->port = THINGSPEAK_PORT_NUMBER;
    };


    int IoTBaseClass::convertFloatToChar(float value, char *valueString)
    {
        // Supported range is -999999000000 to 999999000000
        if (0 == isinf(value) && (value > 999999000000 || value < -999999000000))
        {
          // Out of range
          return ERR_OUT_OF_RANGE;
        }
        // assume that 3 places right of decimal should be sufficient for most applications
    
            #if defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_SAM)
          sprintf(valueString, "%.3f", value);
        #else
          dtostrf(value,1,3, valueString);
            #endif
        return OK_SUCCESS;
    };


    int IoTBaseClass::sendData( const char* shortForm )
    {
        long startMs = millis();

        Serial.print( "Sending " );
        Serial.print( buffer );
        Serial.println( String(" to ") + shortForm );
        
        HTTPClient http;
        http.begin("http://192.168.1.7/DataCollector/Api/AllNumbers?shortForm=" + String(shortForm) + "&data=" + buffer );
    
        //http.begin( "http://192.168.1.7/DataCollector/Api/FrontDoor?doorState=1&vBatt=12" ); 
        int httpCode = http.GET();
        Serial.println( "Http code returned: " + String(httpCode) );
        http.end();   //Close connection
        Serial.println( "Data sent at "+ String(millis()) );
    
        long duration = millis() - startMs;
        Serial.println( "total duration " + String(duration) );
    
        return httpCode;

    }


    int IoTBaseClass::addValue(String value)
    {
        // Max # bytes for ThingSpeak field is 255 (UTF-8)
        if (value.length() > FIELDLENGTH_MAX) return ERR_OUT_OF_RANGE;

        if (buffer.length() > 0) {
            buffer = buffer + ";";
        }
        buffer = buffer + value;
        
        return OK_SUCCESS;
    };  
    
