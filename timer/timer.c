
#include <avr/io.h>
#include <avr/interrupt.h>

int main(void) {
    DDRB |= (1 << PB4);

    OCR0A = 0x70;
    TCCR0A = 0x02;
    TIFR |= 0x10;
    TIMSK = 0x10;
    TCCR0B = 0x05;
    sei();

    while (1);
}

ISR (TIMER0_COMPA_vect) {
    static char toggle = 0;

    if (toggle) {
        toggle = 0;
        PORTB &= ~(1 << PB4);
    } else {
        toggle = 1;
        PORTB |= (1 << PB4);
    }
}

