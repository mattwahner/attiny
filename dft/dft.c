
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

typedef uint8_t bool;
#define true 1
#define false 0

volatile bool compute = false;
volatile size_t i = 0;

#define N 30

uint16_t samples[N];

#define PI2 M_PI * 2

volatile uint8_t tx;
volatile uint8_t tx_i;

// Sets timer0 to frequency ~30Hz
// Sets timer1 to 9600 baudrate
void setup_timer() {
    // Setup timer hw
    OCR0A = 31;
    OCR1A = 104;
    OCR1C = 104;

    TCCR0A =
        (1 << WGM01);   // Set CTC mode

    TCCR0B =
        (1 << CS02) |   // Set 1024 prescale
        (1 << CS00);

    TCCR1 = 
        (1 << CTC1) |
        (1 << CS10);

    // Setup timer interrupts
    TIFR |= (1 << OCF0A);
    TIMSK |= (1 << OCIE0A);
}

EMPTY_INTERRUPT(TIMER0_COMPA_vect);

ISR (TIMER1_COMPA_vect) {
    if (tx_i == 0) {
        PORTB &= ~(1 << PB4);
        tx_i++;
    } else if (tx_i >= 1 && tx_i <= 8) {
        if (tx & (1 << (8 - tx_i))) {
            PORTB |= (1 << PB4);
        } else {
            PORTB &= ~(1 << PB4);
        }
        tx_i++;
    } else if (tx_i == 9) {
        PORTB |= (1 << PB4);
        tx_i++;
    } else {
        tx_i = 0;
        TIFR |= (1 << OCF1A);
        TIMSK &= ~(1 << OCIE1A);
    }
}

void send_byte(uint8_t byte) {
    tx = byte;
    tx_i = 0;
    TIFR |= (1 << OCF1A);
    TIMSK |= (1 << OCIE1A);
}

// Sets up ADC3 to sample on timer0 ref to VCC
void setup_adc() {
    ADMUX = 
        (0 << ADLAR) |  // Set 10 bit precision
        (0 << REFS1) |  // Set VCC as reference
        (0 << REFS0) |
        (0 << MUX3)  |  // Set ADC3 as active
        (0 << MUX2)  |
        (1 << MUX1)  |
        (1 << MUX0);

    ADCSRA =
        (1 << ADEN)  |  // Enable ADC
        (1 << ADATE) |  // Enable ADC auto trigger
        (1 << ADIE)  |  // Enable conv complete interrupt
        (0 << ADPS2) |  // Set prescale to 8
        (1 << ADPS1) |
        (1 << ADPS0);

    ADCSRB =
        (0 << ADTS2) | // Set trigger source to Timer0
        (1 << ADTS1) |
        (1 << ADTS0);
}

ISR (ADC_vect) {
    TIFR |= (1 << OCF0A);

    uint16_t value = ADCL | (ADCH << 8);
    if (++i >= N) i = 0;
    samples[i] = value;
    compute = true;
}

void setup() {
    DDRB |= (1 << PB4);
    PORTB |= (1 << PB4);
    setup_timer();
    setup_adc();
    sei();
}

void blink() {
    PORTB |= (1 << 4);
    _delay_ms(500);
    PORTB &= ~(1 << 4);
    _delay_ms(500);
}

int main(void) {
    setup();
    _delay_ms(1000);
    send_byte(0xaa);
    _delay_ms(1000);
    while (1);
    uint16_t x[N];
    float Xre[N];
    float Xim[N];
    float abs[N];

    size_t max_i = 0;

    while (1) {
        if (compute) {
            size_t curr_index = i;
            for (int j = 0; j < N; j++) {
                x[j] = samples[curr_index++];
                if (curr_index >= N)
                    curr_index = 0;
            }

            max_i = 0;

            for (int k = 0; k < N; k++) {
                Xre[k] = 0;
                for (int n = 0; n < N; n++)
                    Xre[k] += x[n] * cos(n * k * PI2 / N);

                Xim[k] = 0;
                for (int n = 0; n < N; n++)
                    Xim[k] -= x[n] * sin(n * k * PI2 / N);

                abs[k] = sqrt((Xre[k] * Xre[k]) + (Xim[k] * Xim[k]));
                if (abs[k] > abs[max_i])
                    max_i = k;
            }

            if (max_i >= 4 && max_i <= 6)
                PORTB |= (1 << PB4);
            else
                PORTB &= ~(1 << PB4);

            compute = false;
        }
    }
}

