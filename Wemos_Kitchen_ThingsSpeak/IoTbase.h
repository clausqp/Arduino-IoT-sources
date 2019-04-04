// base IoT class 

#ifndef IoTBase_h
#define IoTBase_h



#if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_ESP32)
  #include "Arduino.h"
  #include <Client.h>
#else
  #error Only Arduino MKR1000, Yun, Uno/Mega/Due with either WiFi101 or Ethernet shield. ESP8266 and ESP32 are also supported.
#endif



#define THINGSPEAK_URL "api.thingspeak.com"
#define THINGSPEAK_IPADDRESS IPAddress(184,106,153,149)
#define THINGSPEAK_PORT_NUMBER 80



#ifdef ARDUINO_ARCH_AVR
    #ifdef ARDUINO_AVR_YUN
        #define TS_USER_AGENT "tslib-arduino/1.3 (arduino yun)"
    #else
        #define TS_USER_AGENT "tslib-arduino/1.3 (arduino uno or mega)"
    #endif
#elif defined(ARDUINO_ARCH_ESP8266)
    #define TS_USER_AGENT "tslib-arduino/1.3 (ESP8266)"
#elif defined(ARDUINO_SAMD_MKR1000)
  #define TS_USER_AGENT "tslib-arduino/1.3 (arduino mkr1000)"
#elif defined(ARDUINO_SAM_DUE)
  #define TS_USER_AGENT "tslib-arduino/1.3 (arduino due)"
#elif defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_SAM)
  #define TS_USER_AGENT "tslib-arduino/1.3 (arduino unknown sam or samd)"
#elif defined(ARDUINO_ARCH_ESP32)
  #define TS_USER_AGENT "tslib-arduino/1.3 (ESP32)"
#else
  #error "Platform not supported"
#endif

#define FIELDLENGTH_MAX 255  // Max length for a field in ThingSpeak is 255 bytes (UTF-8)

#define OK_SUCCESS              200     // OK / Success
#define ERR_BADAPIKEY           400     // Incorrect API key (or invalid ThingSpeak server address)
#define ERR_BADURL              404     // Incorrect API key (or invalid ThingSpeak server address)
#define ERR_OUT_OF_RANGE        -101    // Value is out of range or string is too long (> 255 bytes)
#define ERR_INVALID_FIELD_NUM   -201    // Invalid field number specified
#define ERR_SETFIELD_NOT_CALLED -210    // setField() was not called before writeFields()
#define ERR_CONNECT_FAILED      -301    // Failed to connect to ThingSpeak
#define ERR_UNEXPECTED_FAIL     -302    // Unexpected failure during write to ThingSpeak
#define ERR_BAD_RESPONSE        -303    // Unable to parse response
#define ERR_TIMEOUT             -304    // Timeout waiting for server to respond
#define ERR_NOT_INSERTED        -401    // Point was not inserted (most probable cause is the rate limit of once every 15 seconds)


class IoTBaseClass
{
  public:
    IoTBaseClass()
    {
        buffer = String("");
    };

    void ConnectToAP( char *iotName );

    bool begin(Client & client, const char * customHostName, unsigned int port)
    {
#ifdef PRINT_DEBUG_MESSAGES
        Serial.print("ts::tsBegin    (client: Client URL: "); Serial.print(customHostName); Serial.println(")");
#endif
        this->setClient(&client);
        this->setServer(customHostName, port);
        return true;
    };

    bool begin(Client & client, IPAddress customIP, unsigned int port)
    {
#ifdef PRINT_DEBUG_MESSAGES
        Serial.print("ts::tsBegin    (client: Client IP: "); Serial.print(customIP); Serial.println(")");
#endif
        this->setClient(&client);
        this->setServer(customIP, port);
        return true;
    };

    bool begin();

    void setServer(const char * customHostName, unsigned int port);
    void setServer(IPAddress customIP, unsigned int port);
    void setServer();

    void initData()
    {
        buffer = String("");
    }
    
  
    int addValue(int value)
    {
        char valueString[10];  // int range is -32768 to 32768, so 7 bytes including terminator
        itoa(value, valueString, 10);
        return addValue( valueString);
    };
  
    int addValue(long value)
    {
        char valueString[15];  // long range is -2147483648 to 2147483647, so 12 bytes including terminator
        ltoa(value, valueString, 10);
        return addValue( valueString);
    };
  
    int addValue(float value)
    {
        char valueString[20]; // range is -999999000000.00000 to 999999000000.00000, so 19 + 1 for the terminator
        int status = convertFloatToChar(value, valueString);
        if(status != OK_SUCCESS) return status;
        return addValue( valueString);
    };
  
    int addValue(const char * value)
    {
        return addValue( String(value));
    };
  
    int addValue(String value);


    int sendData( const char* shortForm );

private:
    String buffer;

    int convertFloatToChar(float value, char *valueString);

    void setClient(Client * client) { this->client = client; };

    Client * client = NULL;
    const char * customHostName = NULL;
    IPAddress customIP = INADDR_NONE;
    unsigned int port = THINGSPEAK_PORT_NUMBER;

  
};

extern IoTBaseClass IoTBase;

#endif // IoTBase_h


