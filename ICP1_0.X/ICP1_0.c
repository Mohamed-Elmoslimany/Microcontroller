
#define F_CPU 11059200UL        /* Define frequency here its 8MHz */
#define BAUD_PRESCALE           (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)

#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>

#define F_CPU 11059200UL
#define Timer1_CLK (11059200UL / 8)

void UART_init(long USART_BAUDRATE)
{
    UBRRL = BAUD_PRESCALE;                      /* Load lower 8-bits of the baud rate */
    UBRRH = (BAUD_PRESCALE >> 8);               /* Load upper 8-bits */
    UCSRB |= (1 << RXEN) | (1 << TXEN);          /* Turn on transmission and reception */
    UCSRC |= (1 << URSEL) | (1 << UCSZ0) | (1 << UCSZ1); /* Use 8-bit char size */
}

int UART_getChar(FILE *stream)
{
    while ((UCSRA & (1 << RXC)) == 0); /* Wait till data is received */
    return(UDR);                       /* Return the byte, Reading UDR clears RXC flag automatically */
}

// Send one character over UART
int UART_putChar(char c, FILE *stream) {
    while (!(UCSRA & (1 << UDRE))); // Wait until buffer is empty
    UDR = c;                        // Writing to UDR clears UDRE flag automatically
    return 0;
}

static FILE uart_str = FDEV_SETUP_STREAM(UART_putChar, UART_getChar, _FDEV_SETUP_RW);

int main(void) {
    unsigned int a, b, c;
    float high, low, period;
    float freq;
    float duty_cycle;

    // Bit3=Out (OC0 for PWM)
    DDRB = (1 << DDB3);
    PORTB = 0x00;

    // Bit7=Out (OC2 for CTC), Bit6=In (ICP1 - Input Capture), rest as input
    DDRD = (1 << DDD7);

    // Generate PWM signal: 168.75Hz with 25% duty cycle on PB3
    TCCR0 = (1 << WGM00) | (1 << WGM01) |   // Fast PWM mode
            (1 << COM01) |                  // Non-inverted PWM
            (1 << CS02);                    // Prescaler = 256

    TCNT0 = 0x00;
    OCR0 = 0x40;  // 25% duty cycle (64/256)
    ASSR = 0x00;  // Use internal clock
    TCCR2 = (1 << COM20) |                  // Toggle OC2 on compare match
            (1 << WGM21) |                  // CTC mode
            (1 << CS22) | (1 << CS21) | (1 << CS20); // Prescaler = 1024

    TCNT2 = 0x00;
    OCR2 = 0xD7;  // 215 decimal

    TIMSK = 0x00; // All timer interrupts disabled

    UART_init(9600);
    stdin = stdout = &uart_str;

    printf("\r\n========================================\r\n");
    printf("Timer1 Input Capture - PWM Measurement\r\n");
    printf("----------------------------------------\r\n\r\n");

    TCCR1A = 0x00; // Normal mode, OC1A/OC1B disconnected
    TCNT1 = 0x0000;

    do {
        // Step 1: Capture first rising edge
        TCCR1B = (1 << ICES1) | (1 << CS11); // Rising edge, prescaler = 8
        TIFR = (1 << ICF1);                  // Clear input capture flag

        while (!(TIFR & (1 << ICF1)));       // Wait for rising edge
        a = ICR1;                            // Capture value at first rising edge

        // Step 2: Capture falling edge
        TCCR1B = (0 << ICES1) | (1 << CS11); // Falling edge, prescaler = 8
        TIFR = (1 << ICF1);                  // Clear input capture flag

        while (!(TIFR & (1 << ICF1)));       // Wait for falling edge
        b = ICR1;                            // Capture value at falling edge

        // Step 3: Capture second rising edge
        TCCR1B = (1 << ICES1) | (1 << CS11); // Rising edge, prescaler = 8
        TIFR = (1 << ICF1);                  // Clear input capture flag

        while (!(TIFR & (1 << ICF1)));       // Wait for rising edge
        c = ICR1;                            // Capture value at second rising edge

        // Stop timer
        TCCR1B = 0x00;
        TIFR = (1 << ICF1);

    } while (a > b || b > c); // Ensure valid measurement (no overflow)

    high = b - a;        // High pulse width (in timer ticks)
    low = c - b;         // Low pulse width (in timer ticks)
    period = high + low; // Total period (in timer ticks)

    printf("Raw Capture Values:\r\n");
    printf("a: %u   b: %u   c: %u\r\n\r\n", a, b, c);

    freq = (float)Timer1_CLK / period;  // Calculate frequency
    duty_cycle = (high / period) * 100.0; // Calculate duty cycle percentage

    printf("Measured Signal Parameters:\r\n");
    printf("High Time:  %f seconds\r\n", high / (float)Timer1_CLK);
    printf("Low Time:   %f seconds\r\n", low / (float)Timer1_CLK);
    printf("Period:     %f seconds\r\n", period / (float)Timer1_CLK);
    printf("Frequency:  %f Hz\r\n", freq);
    printf("Duty Cycle: %f %%\r\n", duty_cycle);

    printf("\r\n========================================\r\n");
    while (1);
    return 0;
}