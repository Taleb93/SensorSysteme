#pragma once
#include <Arduino.h>

class LoRaE5 {
private:
    HardwareSerial &serial;  // Referenz auf den HardwareSerial-Port
    bool joined;
  

    void clearBuffer();
    String sendCommand(const String &cmd, unsigned long timeout = 5000);

public:
    bool Done;
    bool busy;
    bool faild;
    LoRaE5(HardwareSerial &serialPort);
    void begin(unsigned long baud = 9600);
    bool joinNetwork();
    void sendTestMessage();
    void readAndSendTemperature();
    void readAndSendTemperatureWithDistance(float distance);
};
