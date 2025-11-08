#include <Arduino.h>
class Distance_Sensor{
private:
uint32_t trigPin;
uint32_t echoPin;
float duration, distance;
public:
void sensorInit(uint32_t echoPin, uint32_t trigerPin);
float getDistance(uint32_t time_pulse);

};