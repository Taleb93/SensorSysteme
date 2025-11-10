//#include <Arduino.h>
#include "LoRaE5.hh"
#include "DistanceSensor.hh"
Distance_Sensor Abstand_Sensor;
HardwareSerial LoRaSerial(USART3);
LoRaE5 sensor(LoRaSerial);
const int trigPin = 9;
const int echoPin = 10;
float  distance;
void control_ledLink(bool done,bool busy){
 if(done)
    {
        digitalWrite(D8,LOW);
        digitalWrite(D6,LOW);
        digitalWrite(D7,HIGH);
        delay(200);
        digitalWrite(D7,LOW);
    }
    else if(busy){
        digitalWrite(D7,LOW);
        digitalWrite(D6,LOW);
        digitalWrite(D8,HIGH);
        delay(200);
        digitalWrite(D8,LOW);
    }
    else{
         digitalWrite(D7,LOW);
         digitalWrite(D8,LOW);
         digitalWrite(D6,HIGH);
         delay(200);
         digitalWrite(D6,LOW);
         }
}
void setup() {
     Abstand_Sensor.sensorInit(10,9);
     Serial.begin(9600);
     sensor.begin(9600);
     sensor.joinNetwork();
     pinMode(D6,OUTPUT);
     pinMode(D7,OUTPUT);
     pinMode(D8,OUTPUT);   
}

void loop() {
 
    distance = Abstand_Sensor.getDistance(100);
    static unsigned long lastSend = 0;
    unsigned long now = millis();
    if (now - lastSend > 10000) {  // alle 10 Sekunden
    sensor.readAndSendTemperatureWithDistance(distance);
   control_ledLink(sensor.Done,sensor.busy);
    lastSend = now;
}
}
