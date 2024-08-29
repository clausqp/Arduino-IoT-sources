/* Water meter - reads the turning wheel/missor on the water meter and detects when a turn is made
 * One turn = 1 liter of spent water
 */


#include "IoTBase.h"


long startMillis;
const int ledPin = LED_BUILTIN;


void setup()
{
    startMillis = millis();
  
    Serial.begin(9600);
    delay(10);
    Serial.println();
    Serial.println();
    Serial.println( "Wemos water meter v0.30" );

    pinMode(ledPin, OUTPUT);
    digitalWrite( ledPin, LOW ); //HIGH );

    IoTBase.ConnectToAP( "Water_v0_30");

    long duration = millis() - startMillis;
    Serial.println( "Connection duration " + String(duration) );

    IoTBase.begin();
    digitalWrite( ledPin, HIGH ); // LOW );
}


int lastVal = 0;

int minThr = 400;
int maxThr = 800;

int state = 0; // 0 = low, 1 = middle, 2 = high;


unsigned long nextSend = 0;

  
void loop() 
{
    int val = analogRead( 0 );

    if (abs (lastVal- val) > 3) {
        if (val < minThr) {
            Serial.print("<Min ");
            state = 0;
        }
        else if (val > maxThr) {
            Serial.print( ">Max " );
            if (state != 2) {
                GoingHigh();
            }
            state = 2;
        }
        else {
            Serial.print( "-NC- " );
            state = 1;
        }
        Serial.println( val );
        lastVal = val;
    }

    if (nextSend < millis()) {
        nextSend += 60000; // check for send once every 1 minute
        Send();
    }

    delay(50);
}


int highFound = 0;

void GoingHigh()
{
    Serial.print( "***" );
    highFound++;
}


long sum = 0;

void Send()
{
    if (highFound == 0 && sum == 0) return; // Nothing to send!

    long startTime = millis();
    Serial.print( highFound );
    sum += highFound;
    
    // Send data to backend:
    IoTBase.initData();
    IoTBase.addValue( highFound );
    if (highFound == 0) {
        // Send the sum only when no change!
        IoTBase.addValue( sum );
        sum = 0;
    } else {
        IoTBase.addValue( 0 );
    }
    highFound = 0;
    
    int ret = IoTBase.sendData( "WATR" );
    Serial.println( String("Status from backend: " )+ ret );
    
    long endMillis = millis();
    float duration = (endMillis-startTime)/1000.0;

    Serial.println( String("Send " )
      + ", " + String(duration) + ". Response from thingspeak: " + String(ret) );
}
