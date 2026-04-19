#define F_CPU 11059200UL       /* Define frequency here its 8MHz */
#define BAUD_PRESCALE          (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)

#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <inttypes.h>

void UART_init(long USART_BAUDRATE)
{
    UBRRL = BAUD_PRESCALE;                         /* Load lower 8-bits of the baud rate */
    UBRRH = (BAUD_PRESCALE >> 8);                  /* Load upper 8-bits */
    UCSRB |= (1 << RXEN) | (1 << TXEN);            /* Turn on transmission and reception */
    UCSRC |= (1 << URSEL) | (1 << UCSZ0) | (1 << UCSZ1); /* Use 8-bit char size */
}

// Non-blocking function to check if data is available
uint8_t UART_dataAvailable(void)
{
    return (UCSRA & (1 << RXC));
}

// Non-blocking read - returns -1 if no data available
int UART_getChar_NonBlocking(void)
{
    if(UCSRA & (1 << RXC))
        return UDR;
    else
        return -1;
}

int UART_getChar(FILE *stream)
{
  while ((UCSRA & (1 << RXC)) == 0); /* Wait till data is received */
    return(UDR);                   /* Return the byte, Reading UDR clears RXC flag automatically */
}

// Send one character over UART
int UART_putChar(char c, FILE *stream) {
    while (!(UCSRA & (1 << UDRE))); // Wait until buffer is empty
    UDR = c;                        // Writing to UDR clears UDRE flag automatically
    return 0;
}

// Global flag to signal button press
volatile uint8_t button_pressed = 0;
// INTO Interrupt Service Routine
ISR(INT0_vect)
{
    // Just set the flag - keep ISR fast!
    button_pressed = 1;
}

static FILE uart_str = FDEV_SETUP_STREAM(UART_putChar, UART_getChar, _FDEV_SETUP_RW);

int main(void)
{
    uint32_t x = F_CPU;
    float y = 789.4567;

    unsigned char cmd = 0;
    
    // Enable internal pull-up on PD2 (INT0 pin)
    DDRD &= ~(1 << DDD2);       // Set PD2 as input (explicit)
    PORTD |= (1 << PORTD2);     // Enable pull-up resistor

    // Setup PB0 as output
    DDRB |= (1 << DDB0);
    PORTB |= (1 << PORTB0);     // LED OFF (assuming active-low LED)
    
    // Enable internal pull-up on FD2 (INT0 pin)
    DDRD &= ~(1<<DDD2);       // Set PD2 as input (explicit)
    PORTD |= (1<<PORTD2);   // Enable pull-up resistor
    
    MCUCR |= (1<<ISC01);    //Falling Edge detection
    MCUCR &= ~(1<<ISC00);    // (ISC01-1, ISC00-0)
    GIFR |= (1<<INTF0);     //Clear any pending interrupt
    GICR |= (1<<INT0);      // Enable INT0 interrupt
    sei();

    // initializing the UART with baud rate 9600 b/s
    UART_init(9600);
    stdin = stdout = &uart_str;

    printf("Testing UART!\n");
    printf("Printing long and float numbers\r\n");
    printf("Long value: %lu, Float value: %f \r\n", x, y);

    while(1)
    {
        cmd = UART_getChar_NonBlocking();
        if(cmd != -1)  // If valid data received
        {
            if(cmd == 'O' || cmd == 'o')
                PORTB &= ~(1<<PORTB0);  // LED ON
            else if(cmd == 'X' || cmd == 'x')
                PORTB |= (1<<PORTB0);   // LED OFF
        }
        
        if(button_pressed)
        {
            _delay_ms(50);
            if(!(PIND & (1<<PIND2)))
            {
                printf("Switch is pressed\r\n");
            }
            
            // Wait for button release
            while(!(PIND & (1<<PIND2)))
            {
                _delay_ms(10);
            }
            
            _delay_ms(50);
            // Clear the flag
            button_pressed = 0;
        }
    }
}