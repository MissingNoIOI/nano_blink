BOARD_TAG = nano
BOARD_SUB = atmega328
ARDUINO_LIBS = SPI Wire SSD1306Ascii RTCLib_by_NeiroN OneWire DallasTemperature EEPROM EEPROMAnything Bounce2 
MONITOR_PORT = /dev/ttyUSB0
include /usr/share/arduino/Arduino.mk
AVR_TOOLS_DIR=/usr
ARDUINO_DIR   = /usr/share/arduino
ARDMK_DIR     = /usr/share/arduino
