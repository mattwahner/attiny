
src=uart
avrType=attiny85
programmerType=usbtiny
F_CPU=1000000

all: flash

compile:
	avr-gcc -DF_CPU=$(F_CPU) -Wall -Werror -Wextra -O -mmcu=$(avrType) -o $(src).o $(src).c

hex: compile
	avr-objcopy -j .text -j .data -O ihex $(src).o $(src).hex

flash: hex
	avrdude -p$(avrType) -c$(programmerType) -v -U flash:w:$(src).hex

