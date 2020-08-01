clean:
	rm *.elf *.o *.hex
build:
	avr-gcc -g -Os -mmcu=attiny85 -c main.c
	avr-gcc -g -mmcu=attiny85 -o main.elf main.o
	avr-objcopy -j .text -j .data -O ihex main.elf main.hex
bootloader:
	avrdude -c arduino -p t85 -U flash:w:t85_default.hex:i -B 20 -P /dev/ttyACM0
	avrdude -c arduino -p t85 -U lfuse:w:0xe1:m -U hfuse:w:0xdd:m -U efuse:w:0xfe:m -B 20 -P /dev/ttyACM0
flash-via-bootloader:
	..//micronucleus/commandline/micronucleus --run main.hex
flash:
	avrdude -c arduino -p t85 -U flash:w:main.hex:i -B 20 -P /dev/ttyACM0
	avrdude -c arduino -p t85 -U lfuse:w:0xe2:m -U hfuse:w:0xdd:m -U efuse:w:0xff:m -B 20 -P /dev/ttyACM0
