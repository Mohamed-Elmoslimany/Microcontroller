#define F_CPU 11059200UL
#include <avr/io.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <avr/interrupt.h>

void blink_led(uint8_t pin, uint8_t times) {
    for (uint8_t i = 0; i < times; i++) {
        PORTB &= ~(1 << pin);
        _delay_ms(200);
        PORTB |= (1 << pin);
        _delay_ms(200);
    }
}

int main(void) {
    uint8_t was_watchdog_reset;
    // Check reset source BEFORE clearing the flag
    was_watchdog_reset = MCUSR & (1 << WDRF);

    // Clear watchdog reset flag and disable watchdog
    MCUSR &= ~(1 << WDRF);
    wdt_disable();

    // Configure PB0 and PB1 as outputs (for LEDs)
    DDRB |= (1 << DDB0) | (1 << DDB1);
    PORTB |= (1 << PORTB0) | (1 << PORTB1); // Turn off all LEDs initially

    // Configure PD2 as input with pull-up (for button)
    DDRD &= ~(1 << DDD2);
    PORTD |= (1 << PORTD2);

    if (was_watchdog_reset) {
        // Watchdog caused the reset - system recovered from crash!
        // Blink RED LED 5 times to indicate watchdog recovery
        blink_led(PORTB1, 5);
    } else {
        // Normal power-on reset or external reset
        // Blink GREEN LED 3 times to indicate normal start
        blink_led(PORTB0, 3);
    }
    _delay_ms(500);
    wdt_enable(WDTO_2S);
    // Turn on GREEN LED to show normal operation
    PORTB &= ~(1 << PORTB0);
    while (1) {
        wdt_reset();
        // Toggle GREEN LED to show system is running
        PORTB ^= (1 << PORTB0);
        _delay_ms(500); // 500ms delay (well under 2 second timeout)
        if (!(PIND & (1 << PIND2))) { // Button pressed (active low)
            _delay_ms(50); // Debounce delay
            if (!(PIND & (1 << PIND2))) {
                PORTB |= (1 << PORTB0); // Turn off GREEN LED
                PORTB &= ~(1 << PORTB1); // Turn on RED LED
                while (1) {
                    // toggle GREEN LED
                    PORTB ^= (1 << PORTB0);
                    _delay_ms(100);
                }
            }
        }
    }
    return 0;
}

