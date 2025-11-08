
#include "DistanceSensor.hh"
 void Distance_Sensor:: sensorInit(uint32_t echo_Pin, uint32_t triger_Pin){
    this->echoPin=echo_Pin;
    this->trigPin=triger_Pin;
    pinMode(echoPin, INPUT);
    pinMode(trigPin,OUTPUT);
 }
float Distance_Sensor:: getDistance(uint32_t time_pulse){
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = (duration*.0343)/2;
  delay(time_pulse);
  return distance;
}