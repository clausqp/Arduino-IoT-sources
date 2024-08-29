/* Arduino ESP8266 D1 R2 & mini
 * 
 * note: 
 *  A0 receives battery voltage throug a 120K resistor => 4,2V gives 3.0V at A0
 *  Dx receive reed switch input, uses internal pullup. When door is closed the switch is closed to ground
 *  Dy receives contact switch input, uses internal pullup. When door is locked the locking pall closes the switch to ground
 */

#include <ESP8266WiFi.h>
#include <espnow.h>
#include "IoTBase.h"

const int vInPin = A0;  // Input voltage from battery
const int ledPin = D3;  // build in LED

const int doorClosedPin = D5; // N.O.
const int doorLockedPin = D6; // N.C.
   

#define WIFI_CHANNEL      1
#define SLEEP_SECS        2 * 60 // 15 minutes (max time is approx. 23 minutes)
#define SEND_TIMEOUT      245  // 245 millis seconds timeout 


uint8_t  gatewayAddress[]      =  {0xc8, 0xc9, 0xa3, 0x30, 0x40, 0x21};


bool      callbackCalled;
uint8_t   sendStatusReceived;

typedef struct struct_message {
  uint8_t Length;
  uint8_t SensorMac[6];
  char    ShortName[5];
  char    Data[100];
} struct_message;

struct_message msg;
#define HEADER_SIZE (sizeof(msg.Length) + sizeof(msg.SensorMac) + sizeof(msg.ShortName))



long startMillis;
long lastSend = 0;
int transmissions = 0;
bool debug = false;


void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) 
{
    Serial.printf("OnDataSent. Send done, status = %i\n", sendStatus);
    callbackCalled = true;
    sendStatusReceived = sendStatus;
}


void setup()
{
    startMillis = millis();
    WiFi.persistent( false );
    pinMode( ledPin, OUTPUT );
    digitalWrite( ledPin, HIGH );
    pinMode( doorClosedPin, INPUT );
    pinMode( doorLockedPin, INPUT );

    Serial.begin(115200);
    delay(200);
    Serial.println( "Fordør_v1_02");

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    
    // Init ESP-NOW
    if (esp_now_init() != 0) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }
  
    // Once ESPNow is successfully Init, we will register for Send CB to
    // get the status of Trasnmitted packet
    esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
    esp_now_register_send_cb(OnDataSent);

    // Register peer
    esp_now_add_peer(gatewayAddress, ESP_NOW_ROLE_SLAVE, WIFI_CHANNEL, NULL, 0);
    
    Serial.println("\n\nStarting fordør v1.02");
    digitalWrite(ledPin, LOW); // Signal init end

    Serial.println( "before send data" );
    TransmitData();
    //digitalWrite( powerToPeriferalsPin, HIGH ); // power down periferals

    Serial.println("\nSetup ended\n");
    digitalWrite( ledPin, LOW );
}


void loop()
{
   if (callbackCalled || (millis() > SEND_TIMEOUT)) {
        GoToSleep();
    }
    delay( 25 );
}



// Send data to gateway using ESPnow
uint16_t EspSender( String readings ) 
{
    msg.Length = 42;
    WiFi.macAddress( msg.SensorMac );
    sprintf( msg.ShortName, "FRDR" );
    readings.toCharArray( msg.Data, 100 );
    Serial.println( String("Data length " ) + strlen(msg.Data) );
  
    uint16_t result = esp_now_send( gatewayAddress, (uint8_t *) &msg, HEADER_SIZE+strlen(msg.Data)+1 );
    // uint16_t result=esp_now_send(NULL, bs, sizeof(sensorData)); // NULL means send to all peers
    return result;
}


void TransmitData()
{
    long startTime = millis();
    
    int ain = analogRead( vInPin );
    float batteryVoltage = (ain/1024.0)*4.3;

    int doorClosed = digitalRead( doorClosedPin ) == 1; // N.O. => when door is open, the reed switch is closed and input is low
    int doorLocked = digitalRead( doorLockedPin ) == 0; // N.C. => when door is locked, the the switch is closed and input is low
    Serial.print( "Door closed = " ); Serial.print( doorClosed ); 
    Serial.print( ", locked = "); Serial.print( doorLocked ); 
    Serial.print( ", batery " ); Serial.print( batteryVoltage ); Serial.println( " V");
    
    // Send data to backend:
    IoTBase.initData();
    IoTBase.addValue( doorLocked );
    IoTBase.addValue( doorClosed );
    IoTBase.addValue( batteryVoltage );
    IoTBase.addValue( transmissions++ );
    
    int ret = EspSender( IoTBase.buffer );
    Serial.println( String("Status from backend: " )+ ret );
    
    long endMillis = millis();
    float duration = (endMillis-startTime)/1000.0;

    Serial.println( String("Send " ) + String(duration) );
}


void GoToSleep() 
{
  int sleepSecs = SLEEP_SECS;
  // Serial.printf("Up for %i ms, going to sleep for %i secs...\n", millis(), sleepSecs); 
  Serial.print("will sleep");
  ESP.deepSleep(sleepSecs * 1000000, RF_NO_CAL);
}
