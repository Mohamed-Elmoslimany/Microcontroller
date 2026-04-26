#define F_CPU 11059200UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>

// =========================
// UART FUNCTIONS
// =========================
void UART_init(void)
{
    uint16_t ubrr = 71; // 9600 baud @ 11.0592MHz

    UBRRH = (ubrr >> 8);
    UBRRL = ubrr;

    UCSRB = (1 << TXEN); // Enable TX
    UCSRC = (1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0); // 8-bit
}

void UART_send_char(char c)
{
    while (!(UCSRA & (1 << UDRE)));
    UDR = c;
}

void UART_send_string(const char *str)
{
    while (*str)
    {
        UART_send_char(*str++);
    }
}

// =========================
// TIMER1 ? 500 ms toggle
// =========================
void Timer1_init(void)
{
    // CTC mode, toggle handled in ISR
    TCCR1A = 0x00;
    TCCR1B = (1 << WGM12) | (1 << CS12) | (1 << CS10); // prescaler 1024

    // 500 ms:
    // OCR1A = (F_CPU / (1024 * 2Hz)) - 1 = 5399
    OCR1A = 5399;

    TIMSK |= (1 << OCIE1A);
}

// =========================
// GLOBALS
// =========================
volatile uint8_t wdt_disabled = 0;

// =========================
// TIMER1 ISR ? LED toggle
// =========================
ISR(TIMER1_COMPA_vect)
{
    PORTB ^= (1 << PB0);
}

// =========================
// INT0 ISR ? Button press
// =========================
ISR(INT0_vect)
{
    if (!wdt_disabled)
    {
        wdt_disabled = 1;

        wdt_disable();
        UART_send_string("WDT disabled by user.\r\n");
    }
}

// =========================
// MAIN
// =========================
int main(void)
{
    // LED output
    DDRB |= (1 << PB0);

    // Button PD2 input + pull-up
    DDRD &= ~(1 << PD2);
    PORTD |= (1 << PD2);

    // Enable INT0 (falling edge)
    MCUCR |= (1 << ISC01);
    GICR |= (1 << INT0);

    UART_init();
    Timer1_init();

    sei();

    // Check reset source
    if (MCUCSR & (1 << WDRF))
    {
        // WDT caused reset
        MCUCSR &= ~(1 << WDRF);
    }

    UART_send_string("System started - WDT enabled.\r\n");

    // Enable WDT ~2.1 sec
    wdt_enable(WDTO_2S);

    while (1)
    {
        if (wdt_disabled)
        {
            // wait 5 seconds
            for (uint8_t i = 0; i < 5; i++)
                _delay_ms(1000);

            UART_send_string("WDT re-enabled.\r\n");

            wdt_enable(WDTO_2S);

            wdt_disabled = 0;
        }

        // ? DO NOT reset WDT here ? forces reset
    }
}
