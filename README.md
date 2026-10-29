# ESP32 Digital Overcurrent Relay

An educational DC overcurrent relay built with ESP32, an SSD1306 OLED, a low-side shunt and an active-LOW relay module.

## How it works

Press the reset/start button to enable the fan. After a **2-second startup interval**, the relay trips when measured current exceeds **60mA for three consecutive averaged readings**. A fault stays latched until the button is pressed again. The OLED shows supply voltage, current and operating state; after a trip it shows the recorded trip current and limit.

## Hardware

| Item | Connection / value |
|---|---|
| Main load supply | Approximately 6V DC |
| ESP32 | Powered by USB |
| Relay module | Regulated 5V, active LOW, IN → GPIO26 |
| OLED SSD1306 | 3.3V, SDA → GPIO21, SCL → GPIO22; 128×64, address 0x3C |
| Voltage divider | 10kΩ upper / 2.15kΩ lower; midpoint → GPIO35 |
| Current shunt | Two 10Ω resistors in parallel = 5Ω; top node → GPIO34 |
| Reset/start button | GPIO27 to GND, INPUT_PULLUP |
| DC fan | Supply positive → relay COM; relay NO → fan positive; fan negative → shunt top |

All grounds share the source-negative reference. Fan return current flows through the shunt. The motor symbol in the drawing is a visual substitute for the DC fan.

## Circuit diagram

![Fritzing circuit diagram](images/Circuit-Diagram.png)

[Download the editable Fritzing project](hardware/ESP32-Relay.fzz).

## Actual hardware and results

Original project photographs; displayed readings have not been independently verified here.

### Hardware setup

![Actual hardware setup](images/Hardware-Setup.jpeg)

### Normal operation

The OLED shows **6.21V**, **57.4mA** and **NORMAL**.

![Normal operation](images/Normal-Operation.jpeg)

### Overcurrent trip

The OLED shows **PROTECTION TRIPPED**, **FAULT: OVERCURRENT**, a trip reading of **66.3mA**, and a **60mA** limit.

![Overcurrent trip](images/Overcurrent-Trip.jpeg)

## Arduino code

The author's original code is included unchanged in [finalCode.ino](firmware/finalCode/finalCode.ino).

1. Install the Espressif ESP32 board package in Arduino IDE.
2. Install **Adafruit SSD1306**, **Adafruit GFX Library**, and their dependencies using Library Manager.
3. Open `firmware/finalCode/finalCode.ino`, select the actual ESP32 board and port, then compile and upload.
4. Open Serial Monitor at **115200 baud**. Verify the wiring and safe supply limits before enabling the load.

The sketch uses 100 ADC samples per channel, with a 2ms delay per sample, plus a 100ms loop delay. Three consecutive readings therefore do not mean three rapid ADC conversions. The current calibration factor is **0.853**, specific to the original setup; calibrate your own measurements against a meter. No new hardware test or toolchain compilation was performed while preparing this repository.

## Limitations and safety

This is an **educational, unprotected prototype**, not a certified protective device. GPIO34 is connected directly to the shunt top; excessive shunt voltage or transients can damage the ESP32 before software opens the relay. The startup interval intentionally blanks overcurrent detection, and software/relay delays are not guaranteed protection times.

Use only a suitably limited low-voltage DC setup. Do not use mains or high-power loads. Verify resistor power ratings, relay DC/inrush ratings, 3.3V relay-input compatibility, reset behavior and fan transient suppression. Independent current limiting/fusing and properly engineered ADC input protection are needed before fault testing or adapting the circuit to larger loads. Do not bypass the shunt with another ground return.

