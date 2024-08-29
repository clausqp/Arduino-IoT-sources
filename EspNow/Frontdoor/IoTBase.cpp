
#include <ESP8266WiFi.h>
#include "IoTBase.h"


IoTBaseClass IoTBase;




    bool IoTBaseClass::begin()
    {

        return true;
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
    
