// base IoT class 

#ifndef IoTBase_h
#define IoTBase_h

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

    bool begin();

 
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

    

    String buffer;
private:
    

    int convertFloatToChar(float value, char *valueString);
};

extern IoTBaseClass IoTBase;

#endif // IoTBase_h
