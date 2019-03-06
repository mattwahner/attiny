
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

void initADC() {
    ADMUX = 
        (1 << ADLAR) |
        (0 << REFS1) |
        (0 << REFS0) |
        (0 << MUX3)  |
        (0 << MUX2)  |
        (1 << MUX1)  |
        (1 << MUX0);

    ADCSRA =
        (1 << ADEN)     |
        (1 << ADIE)     |
        (1 << ADPS2)    |
        (1 << ADPS1)    |
        (0 << ADPS0);
}

ISR (ADC_vect) {
    if (ADCH > 128) {
        PORTB |= (1 << PB4);
    } else {
        PORTB &= ~(1 << PB4);
    }
    ADCSRA |= (1 << ADSC);
}

int main(void) {
    initADC();
    sei();
    DDRB |= (1 << 4);

    ADCSRA |= (1 << ADSC);
    while (1);
}

