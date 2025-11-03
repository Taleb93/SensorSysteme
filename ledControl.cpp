#include "LedControl.hh"

LedControl::LedControl(uint8_t pinNumber) {
     pin = pinNumber;            
    state = false;               
    lastToggleTime = 0;        
    blinkInterval = 0;          
    pinMode(pin, OUTPUT);        
    digitalWrite(pin, LOW);      
}

void LedControl::on() {
    digitalWrite(pin, HIGH);
    state = true;
}

void LedControl::off() {
    digitalWrite(pin, LOW);
    state = false;
}

void LedControl::toggle() {
    state = !state;
    digitalWrite(pin, state ? HIGH : LOW);
}
void LedControl::blink1(uint8_t time){
   
   while(1)
   {
    on();
    delay(time);
    off();
   }
    

}
unsigned long lastToggleTime = 0;
uint16_t blinkInterval = 0;

void LedControl::blink(uint16_t interval) {
    blinkInterval = interval;
}

void LedControl::update() {
    if (blinkInterval > 0 && millis() - lastToggleTime >= blinkInterval) {
        toggle();
        lastToggleTime = millis();
    }
}

