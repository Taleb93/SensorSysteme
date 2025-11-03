#ifndef LEDCONTROL_H
#define LEDCONTROL_H

#include <Arduino.h>

class LedControl {
private:

    uint8_t pin;              
    bool state;                
    unsigned long lastToggleTime; 
    uint16_t blinkInterval; 

public:
    LedControl(uint8_t pinNumber);
    void on();
    void off();
    void blink1(uint8_t pause);
    void blink(uint16_t interval);
    void update();

    void toggle();
};

#endif
