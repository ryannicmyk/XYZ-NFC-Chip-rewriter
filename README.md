# XYZ-NFC-Chip-rewriter
Interfaces and Rewrites NFC Tag data on the XYZ 3D Printer Filament
## Requirements 
```
Arduino
PN532 NFC Module
16x2 Display
Joystick
Note: Can be run with only the Arduino and PN532 If needed, Just reformat code to only run function + not use 16x2
```
## Instructions/Connections
```
PN532_SCK  (D2)
PN532_MOSI (D3)
PN532_SS   (D4)
PN532_MISO (D5)
PN532_IRQ   (D2)
PN532_RESET (D3)
LCD 16x2 on D7, D8, D9, D10, D11 , and D12
Joystick Analog on A0
```
Flash Arduino and have fun


Uses a modified ADAFRUIT PN532 Library under a BSD license
