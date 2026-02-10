# Stromsensor + TinyML Geräteerkennung + LoRaWAN Cloud-Upload (NUCLEO-F303RE)

Dieses Repository enthält ein Embedded-System zur **Geräteerkennung anhand von Stromsignalen**:
Ein Stromsensor liefert ein analoges Signal (mit Offset), das vom **NUCLEO-F303RE** per ADC abgetastet wird. Aus den Messdaten werden Merkmale berechnet und ein **TinyML-Modell (TensorFlow Lite Micro)** klassifiziert das angeschlossene Gerät. Das Erkennungsergebnis wird anschließend per **LoRaWAN (LoRa-E5 via UART)** als kompakte Nachricht an die Cloud übertragen.

## System-Übersicht

**Pipeline (Echtbetrieb):**
1. **Strommessung (ADC):** Analogsignal mit Offset → Nucleo ADC (A0)
2. **Sampling:** ~3000 Hz (Delay ~333 µs)
3. **Feature-Extraktion:** Fenster = 20 ms → 7 Features:
   - `I_rms`, `I_max`
   - Spektralamplituden bei `50/100/150/200/250 Hz` (Goertzel)
4. **Normalisierung:** z-Score mit `NORM_MEAN` & `NORM_STD`
5. **Inference:** TensorFlow Lite Micro über `EloquentTinyML`
6. **Übertragung:** erkannte Klasse → UART → LoRa-E5 → LoRaWAN Gateway/Cloud

**Klassen (aktuell):**
- `noDevice`
- `Lötstation`
- `Ventilator`
- `Laptopnetzteil`

## Repository-Inhalt

- `main.cpp` – Setup, Model-Loading, Live-Inferenz, LoRaWAN-Senden  
- `CurrentSensor.hh/.cpp` – Abtastung (3 kHz), Fensterlogik, Voting, Ausgabe  
- `feature.hh/.cpp` – FeatureExtractor (20 ms) inkl. Goertzel + Hamming  
- `norm_params.hh/.cpp` – Mittelwert/Std für Feature-Normalisierung  
- `device_model_int8_floatio_data.h` – exportiertes TFLM Modell (C-Array)  
- `LoRaE5.hh/.cpp` – LoRa-E5 AT-Kommandos (Join + Send)  
- `current_train.ipynb` – Training/Export Pipeline (Google Colab/Jupyter)  
- `platformio.ini` – PlatformIO-Konfiguration (NUCLEO-F303RE + Libs)

## Hardware

- STM32 **NUCLEO-F303RE**
- Stromsensor (analog, mit Offset – z. B. 1.65 V Mittelwert)
- Seeed Studio **LoRa-E5** (LoRaWAN) oder Wio-E5 / LoRa-E5 Modul
- Optional: Lasten/Verbraucher (Lötstation, Ventilator, Laptopnetzteil)

### Voraussetzungen
- VS Code + PlatformIO Extension
- USB-Treiber/STM32 ST-Link (über Nucleo integriert)

