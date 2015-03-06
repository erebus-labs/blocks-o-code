avr-g++ main.c -DF_CPU=8000000UL -O2 -Lavr -mmcu=attiny2313 -o test.o
avr-objcopy -R .eeprom -O ihex test.o test.hex
avrdude -P usb -b 19200 -c avrispmkII -p t2313 -U flash:w:test.hex
