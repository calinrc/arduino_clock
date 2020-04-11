# arduino_clock
Arduino clock build from Arduino Pro Mini, LCD, RT DS3231 and Matrix Keyboard


## Pinout

### LiquidCrystalDisplay (LCD)

- VSS - GND
- VDD - VCC
- VO - Potentiometer PIN 2
- RS - D7
- RW - GND
- E - D8
- D0 - --
- D1 - --
- D2 - --
- D3 - --
- D4 - D9
- D5 - D10
- D6 - D11
- D7 - D12
- A - Resistor 220 Ohm -> VCC
- K - GND

### Matrix Keyboard (MK)

- W1 - D5
- W2 - D4
- W3 - D6
- W4 - D2
- W5 - A0
- W6 - A1
- W7 - A2
- W8 - A3

### RT DS3231 (RTC)

- GND - GND 
- VCC - VCC
- SDA - A4
- SCL - A5
- SQW - --
- 32K - --

### Potentiometer (POT)

 - Pin 1 - VCC
 - Pin 2 - LCD V0 PIN 3
 - Pin 3 - GND
 
### Buzzer
 - Pin 1 - GND
 - Pin 2 - D3
 
 
## Behavior 

Press A to chnage between Edit mode and Normal (display) mode
Press B to chnage between alarm and Edit Alarm mode
Press * to move cursor in edit mode to left
Press # to move cursor in edit mode to right
Use numeric pad in edit mode to chnage current cursor value