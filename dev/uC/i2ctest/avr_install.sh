avr-g++ main.c -DF_CPU=8000000UL -O2 -Lavr -mmcu=attiny461a -o test.o
avr-objcopy -R .eeprom -O ihex test.o test.hex
avrdude -P usb -b 19200 -c avrispmkII -p t461 -U lfuse:w:0xe2:m flash:w:test.hex
