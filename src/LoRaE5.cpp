// LoRaE5.cpp
#include "LoRaE5.hh"                                   // LoRaE5 Klasse
LoRaE5::LoRaE5(HardwareSerial &serialPort)
  : serial(serialPort), joined(false) {}               // UART + Status
void LoRaE5::begin(unsigned long baud) {
    serial.begin(baud);                                // UART starten
    delay(1500);                                       // Modul booten
}
void LoRaE5::clearBuffer() {
    while (serial.available()) serial.read();          // UART leeren
}
String LoRaE5::sendCommand(const String &cmd, unsigned long timeout) {
    clearBuffer();                                     // alte Daten weg
    serial.println(cmd);                               // AT senden
    unsigned long start = millis();                    // Timeout-Start
    String response = "";                              // Antwort sammeln

    while (millis() - start < timeout) {               // bis Timeout
        while (serial.available()) {                   // Daten da?
            char c = serial.read();                    // Byte lesen
            response += c;                             // Antwort bauen

            if (response.indexOf("Done") != -1 || response.indexOf("busy") != -1) {
                response.trim();                       // Whitespace weg
                return response;                       // früh zurück
            }
        }
    }
    response.trim();                                   // Whitespace weg
    return response;                                   // Antwort zurück
}
bool LoRaE5::joinNetwork() {
    Serial.println(" Versuche, LoRaWAN-Netzwerk zu joinen..."); // Status
    while (!joined) {                                  // bis verbunden
        String resp = sendCommand("AT+JOIN", 10000);    // Join-Befehl
        Serial.print("LoRa → ");                        // Debug prefix
        Serial.println(resp);                           // Antwort anzeigen

        if (resp.indexOf("Network joined") != -1 || resp.indexOf("Joined already") != -1) {
            Serial.println(" Erfolgreich mit Netzwerk verbunden!"); // Erfolg
            joined = true;                              // Flag setzen
        } else {
            Serial.println("Join fehlgeschlagen, neuer Versuch in 15 Sekunden..."); // Retry
            delay(15000);                               // Join-Delay
        }
    }
    return joined;                                      // Status zurück
}
void LoRaE5::sendTestMessage() {
    String cmd = "AT+MSG=\"Test Network\",UNCNF";
    Serial.print("Sende: ");
    Serial.println(cmd);

    String resp = sendCommand(cmd, 8000);
    Serial.print("LoRa → ");
    Serial.println(resp);

    if (resp.indexOf("+MSG: Done") != -1) {
        Serial.println("Nachricht erfolgreich gesendet!\n");
    } else {
        Serial.println("Nachricht wurde nicht bestätigt!\n");
    }
}

void LoRaE5::readAndSendTemperature() {
    String resp = sendCommand("AT+TEMP", 2000);
    Serial.print("Antwort auf AT+TEMP → ");
    Serial.println(resp);

    int idx = resp.indexOf(":");
    if (idx > 0) {
        String tempStr = resp.substring(idx + 1);
        tempStr.trim();
        float tempC = tempStr.toFloat();

        Serial.print("🌡️ Gelesene Temperatur: ");
        Serial.println(tempC);

        String sendCmd = String("AT+MSG=\"TEMP:") + String(tempC, 1) +"\"";
        Serial.print("Sende an TTN: ");
        Serial.println(sendCmd);

        String sendResp = sendCommand(sendCmd, 10000);
        Serial.print("LoRa → ");
        Serial.println(sendResp);

        if (sendResp.indexOf("Done") != -1) {
            Serial.println(" Nachricht erfolgreich gesendet!");
            this->Done = true;
            this->busy = false;
        
        } else if (sendResp.indexOf("busy") != -1) {
            Serial.println("Modem war beschäftigt, später erneut senden!");
             this->Done = false;
              this->busy = true;
        } else {
            Serial.println("Nachricht wurde nicht bestätigt!");
             this->Done = false;
              this->busy = false;
        }

        delay(5000); // 5 Sekunden warten, damit das Modem sich erholt
    } else {
        Serial.println("⚠️ Keine gültige Temperaturantwort erhalten!");
    }
}
void LoRaE5::readAndSendTemperatureWithDistance(float distance) {
    // Temperatur aus dem Modul lesen
    String resp = sendCommand("AT+TEMP", 2000);
    Serial.print("Antwort auf AT+TEMP → ");
    Serial.println(resp);

    int idx = resp.indexOf(":");
    if (idx > 0) {
        String tempStr = resp.substring(idx + 1);
        tempStr.trim();
        float tempC = tempStr.toFloat();

        Serial.print("🌡️ Gelesene Temperatur: ");
        Serial.println(tempC, 1);
        Serial.print("📏 Aktueller Abstand: ");
        Serial.println(distance, 1);

        // Nachricht kombinieren → TEMP + DISTANCE
        String message = "TEMP:" + String(tempC, 1) + "C | DIST:" + String(distance, 1) + "cm";
        String sendCmd = "AT+MSG=\"" + message + +"\"";

        Serial.print("📤 Sende an TTN: ");
        Serial.println(sendCmd);

        // Befehl senden
        String sendResp = sendCommand(sendCmd, 10000);
        Serial.print("LoRa → ");
        Serial.println(sendResp);

        // Ergebnis auswerten
        if (sendResp.indexOf("Done") != -1) {
            Serial.println("✅ Nachricht erfolgreich gesendet!");
            this->Done = true;
            this->busy = false;
        } 
        else if (sendResp.indexOf("busy") != -1) {
            Serial.println("⚠️ Modem war beschäftigt, später erneut senden!");
            this->Done = false;
            this->busy = true;
        } 
        else {
            Serial.println("⚠️ Nachricht wurde nicht bestätigt!");
            this->Done = false;
            this->busy = false;
        }

        delay(5000);  // kleine Pause zwischen den Sendevorgängen
    } 
    else {
        Serial.println("⚠️ Keine gültige Temperaturantwort erhalten!");
    }
}
void LoRaE5::control_ledLink(uint32_t sendLed,uint32_t busyLed,uint32_t faildLed){
 if(this->Done)
    {
        digitalWrite(faildLed,LOW);
        digitalWrite(busyLed,LOW);
        digitalWrite(sendLed,HIGH);
        delay(200);
        digitalWrite(sendLed,LOW);
    }
    else if(this->busy){
        digitalWrite(faildLed,LOW);
        digitalWrite(sendLed,LOW);
        digitalWrite(busyLed,HIGH);
        delay(200);
        digitalWrite(busyLed,LOW);
    }
    else{
         digitalWrite(busyLed,LOW);
         digitalWrite(sendLed,LOW);
         digitalWrite(faildLed,HIGH);
         delay(200);
         digitalWrite(faildLed,LOW);
         }
}
void LoRaE5::SendDetectedDevice(const String &Device) {
    String payload;                                     // Payload String
    payload.reserve(4 + Device.length());               // Speicher sparen
    payload = "DEV:";                                   // Prefix
    payload += Device;                                  // Gerätename

    const size_t MAX_PAYLOAD = 51;                      // DR0 sicher
    if (payload.length() > MAX_PAYLOAD) {
        payload.remove(MAX_PAYLOAD);                    // hart kürzen
    }

    Serial.print(" Zum Server wird erkannt: ");         // Debug Text
    Serial.println(Device);                             // Gerät ausgeben
    Serial.print(" Payload (Bytes): ");                 // Länge zeigen
    Serial.println(payload.length());                   // Byte-Anzahl

    String sendCmd;                                     // AT Kommando
    sendCmd.reserve(8 + payload.length() + 1);          // Speicher sparen
    sendCmd = "AT+MSG=\"";                              // MSG Start
    sendCmd += payload;                                 // Payload rein
    sendCmd += "\"";                                    // MSG Ende

    Serial.print(" Sende an TTN: ");                    // Debug Text
    Serial.println(sendCmd);                            // Kommando zeigen

    String sendResp = sendCommand(sendCmd, 10000);       // senden + warten
    Serial.print("LoRa → ");                             // Debug prefix
    Serial.println(sendResp);                            // Antwort zeigen

    if (sendResp.indexOf("Done") != -1) {               // Erfolg?
        Serial.println(" Nachricht erfolgreich gesendet!"); // OK
    }
    else if (sendResp.indexOf("busy") != -1) {          // busy?
        Serial.println(" Modem war beschäftigt, später erneut senden!"); // Retry
    }
    else {
        Serial.println(" Nachricht wurde nicht bestätigt!"); // unklar
    }

    delay(5000);                                        // Pause nach Send
}