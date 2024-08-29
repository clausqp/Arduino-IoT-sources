/* 

THE GATEWAY - Wifi part

Receives messages on serial and sends the received data on to WiFi to central server API.

Board: LOLIN (Wemos) D1 R2 & mini
     
2024-04-18 - V1.01 - added restart if connection is lost for long time + code cleanup
2024-05-12 - v1.02 - better coding of data flowing between EspNow recivers and this node
*/

#include "IoTBase.h"


#ifdef DEBUG_FLAG
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else
#define debug(x)
#define debugln(x)
#endif


typedef struct struct_message {
  uint8_t Length;
  uint8_t SensorMac[6];
  char    ShortName[5];
  char    Data[100];
} struct_message;

struct_message  msg;

const int ledPin = D4;

byte receiverStatus = 1;    // tracks the ESPNow receiver status
long startMillis;
int packagesSent = 0;
long lastActivity = 0;
int nextNoActivityMinuttes = 1;

void setup() 
{
    startMillis = millis();
    Serial.begin(115200);
    delay(100);
    Serial.println();
    Serial.println();
    Serial.println( "#Wemos Gateway v1.02" );
    pinMode( ledPin, OUTPUT );
    digitalWrite( ledPin, LOW );

    IoTBase.ConnectToAP( "Gatw_v1_02");

    long duration = millis() - startMillis;
    Serial.println( "#Connection duration " + String(duration) );

    IoTBase.begin();
    IoTBase.setServer( "192.168.10.7", 80 );

    lastActivity = millis();
    digitalWrite( ledPin, HIGH );

  /* Test  
  msg.Length = 42;
  uint8_t  gatewayAddress[]      =  {0x48, 0x3F, 0xDA, 0x65, 0xB0, 0xAC}; // the MAC which the gateway is listening on
  memcpy(&msg.SensorMac, gatewayAddress, sizeof(gatewayAddress));
  sprintf( msg.ShortName, "DRIV" );
  sprintf( msg.Data, "42;43;44;45" );
  //Serial.println( msg );

  // Serial.print( (char)42 );
  char *sp = (char*)&msg;
  for (int i = 0; i < 22; i++ ) Serial.print( *sp++ );
  Serial.println();
  */
}


uint8_t incomingData[255];
long lostConnection = 0;

void loop() 
{
    if (!IoTBase.IsConnected()) {
        // Connection is lost
        if (lostConnection == 0) {
            lostConnection  = millis();
        }
    }
    
    if (Serial.available()) {
        digitalWrite( ledPin, LOW );
        int received_msg_length = Serial.readBytesUntil('\n', incomingData, sizeof(incomingData));
        if (received_msg_length > 0) {
            switch (incomingData[0]) {
                case '#': // comment/debug message, just ignore!
                    break;
                case 'D': // Data packet
                    memcpy(&msg, incomingData+1, received_msg_length-1);
                    if ( msg.Length == 42) {
                      msg.ShortName[4] = 0;
                      if (IoTBase.IsConnected()) {
                        TransmitData();
                        packagesSent++;
                        lostConnection = 0;
                        lastActivity = millis();
                      } else if (lostConnection+60000 <= millis()) {  
                        // if down for 1 minutte, then restart!
                        ESP.reset();
                      }
                    }
                    break;
                case 'S': // normal status
                    if (incomingData[1] == '0') receiverStatus = 0; // offline
                    if (incomingData[1] == '1') receiverStatus = 1; // online ready to receive ESP NOW packets
                    break;
                case 'A': // Alive status
                    TransmitStatus(incomingData[1]);
                    break;
            }
        }
        digitalWrite( ledPin, HIGH );
    }

    if (lastActivity+(nextNoActivityMinuttes*60000) < millis()) {
        // long time not heard from any ESP NOW sensors...
        TransmitStatus( nextNoActivityMinuttes );
        lastActivity = millis();
        nextNoActivityMinuttes++;
    }
}


void TransmitStatus( byte aliveStatus )
{
    long startTime = millis();
    float temperature = 25.1;

    // Send data to backend:
    IoTBase.initData();
    IoTBase.addValue( packagesSent );
    IoTBase.addValue( aliveStatus );
    IoTBase.addValue( temperature );

    int ret = IoTBase.sendData( "ESP1" );
    debugln( String("#Status from backend: " ) + ret );
    
    long endMillis = millis();
    float duration = (endMillis-startTime)/1000.0;

    debugln( String("#Send " ) + duration);
}




void TransmitData()
{
    long startTime = millis();

    char macAddr[18];
    sprintf( macAddr, "%02X%02X%02X%02X%02X%02X", msg.SensorMac[0], msg.SensorMac[1], msg.SensorMac[2], msg.SensorMac[3], msg.SensorMac[4], msg.SensorMac[5] );
    debugln( String("#Data from " ) + msg.ShortName + String(", at " ) + macAddr );
    
    // Send data to backend:
    IoTBase.initData();
    // Test:
    //float f = 42.42;
    //IoTBase.addValue( f );
    IoTBase.addValue( msg.Data );

    int ret = IoTBase.sendData( msg.ShortName );
    debugln( String("#Status from backend: " ) + ret );
    
    long endMillis = millis();
    float duration = (endMillis-startTime)/1000.0;

    debugln( String("#Send " ) + duration);
}
