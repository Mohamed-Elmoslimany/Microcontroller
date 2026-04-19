#define F_CPU 11059200UL

#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <inttypes.h>

#ifdef DEBUG
    // Debug mode skip delays
    #define MS_DELAY(x) ((void)0)
    #define US_DELAY(x) ((void)0)
#else
    // Production mode real delays
    #define MS_DELAY(x) _delay_ms(x)
    #define US_DELAY(x) _delay_us(x)
#endif

// Global flag to signal button press
volatile uint8_t button_pressed_counter = 0;
volatile uint8_t button_pressed = 0;
volatile uint8_t reset_1 = 0;
// INTO Interrupt Service Routine
ISR(INT0_vect)
{
    // Just set the flag keep ISR fast!
    button_pressed = 1;
}

ISR(INT2_vect)
{
    // Just set the flag keep ISR fast!
    reset_1 = 1;
}

int main(void)
{
    // Enable internal pull-up on FD2 (INT0 pin)
    DDRD &= ~(1<<DDD2);       // Set PD2 as input (explicit)
    PORTD |= (1<<PORTD2);   // Enable pull-up resistor
    // Enable internal pull-up on FB2 (INT2 pin)
    DDRB &= ~(1<<DDB2);       // Set PD2 as input (explicit)
    // Setup PBG as output
    DDRB |= (1<<DDB0);
    PORTB |= (1<<PORTB0);   // LED OFF (assuming active-low LED)
    // Setup PBG as output
    DDRB |= (1<<DDB1);
    PORTB |= (1<<PORTB1);   // LED OFF (assuming active-low LED)
    // Configure INT0
    MCUCR |= (1<<ISC01);    //Falling Edge detection
    MCUCR &= ~(1<<ISC00);    // (ISC01-1, ISC00-0)
    GIFR |= (1<<INTF0);     //Clear any pending interrupt
    GICR |= (1<<INT0);      // Enable INT0 interrupt
    // Configure INT2
    MCUCSR |= (1<<ISC2);    // (ISC01-1, ISC00-0)
    GIFR |= (1<<INTF2);     //Clear any pending interrupt
    GICR |= (1<<INT2);      // Enable INT2 interrupt
    sei();      // Enable global interrupts
//    int button_pressed_counter = 0;

    while (1)
    {
        if (reset_1)
        {
            MS_DELAY(20);
            if(!(PINB & (1<<PINB2)))
            {
                
                while (!(PINB & (1<<PINB2)))
                {
                    MS_DELAY(20);
                }
                MS_DELAY(50);
            }   
            PORTB |= (1<<PORTB0);
            PORTB |= (1<<PORTB1);   
            button_pressed_counter = 0;
            button_pressed = 0;
            reset_1 = 0; // Clear the flag
        }
        if (button_pressed)
        {
            MS_DELAY(20);
            if(!(PIND & (1<<PIND2)))
            {
                PORTB ^= (1<<PORTB0);
                button_pressed_counter++;
                if(button_pressed_counter == 10)
                {
                    PORTB ^= (1<<PORTB1);
                    button_pressed_counter = 0;
                }
                while (!(PIND & (1<<PIND2)))
                {
                    MS_DELAY(20);
                }
                MS_DELAY(50);
            }   
            button_pressed = 0; // Clear the flag
        }
    }
    return 0;
}