
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdlib.h>

#define BAUDRATE 9600

typedef enum { AVAILABLE, FIRST, SECOND } UART_STATE;
volatile UART_STATE us;
volatile uint8_t tx;

uint8_t reverse_byte(uint8_t b) {
    b = ((b >> 1) & 0x55) | ((b << 1) & 0xaa);
    b = ((b >> 2) & 0x33) | ((b << 2) & 0xcc);
    b = ((b >> 4) & 0x0f) | ((b << 4) & 0xf0);
    return b;
}

void send_byte(uint8_t b) {
    while (us != AVAILABLE);
    us = FIRST;

    tx = b;

    TCCR0A = 2 << WGM00;
    TCCR0B = 1;
    GTCCR |= 1 << PSR0;
    OCR0A = 104;
    TCNT0 = 0;

    USIDR = (tx >> 1) | 0x00;
    USICR = (1 << USIOIE) |
        (0 << USIWM1) | (1 << USIWM0) |
        (0 << USICS1) | (1 << USICS0) |
        (0 << USICLK);
    DDRB |= (1 << PB1);
    USISR = (1 << USIOIF) | (16 - 8);
}

ISR (USI_OVF_vect) {
    
    if (us == FIRST) {
        us = SECOND;
        USIDR = (tx << 7 ) | 0x7f;
        USISR = 1 << USIOIF | 
            (16 - (1 + 1));
    } else {
        PORTB |= 1 << PB1;
        DDRB |= 1 << PB1;
        USICR = 0;
        USISR |= 1 << USIOIF;

        us = AVAILABLE;
    }
}

int main(void) {
    sei();
    DDRB |= (1 << PB1);
    PORTB |= (1 << PB1);
    char message[] = "deadass fuck thots on god";
    size_t len = sizeof(message) - 1;
    _delay_ms(5000);
    for (size_t i = 0; i < len; i++) {
        while (us != AVAILABLE);
        send_byte(message[i]);
    }
    while (1);
}

