#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <inttypes.h>
#include "my_lcd.h"

#ifdef DEBUG
    // Debug mode skip delays
    #define MS_DELAY(x) ((void)0)
    #define US_DELAY(x) ((void)0)
#else
    // Production mode real delays
    #define MS_DELAY(x) _delay_ms(x)
    #define US_DELAY(x) _delay_us(x)
#endif

volatile uint8_t button_pressed = 0;

ISR(INT0_vect)
{
    button_pressed = 1;
}

int main() {
    // Configure LED
    DDRB |= (1<<DDB0);
    PORTB |= (1<<PORTB0);   // LED OFF (assuming active-low LED)
    // Enable internal pull-up on FD2 (INT0 pin)
    DDRD &= ~(1<<DDD2);       // Set PD2 as input (explicit)
    PORTD |= (1<<PORTD2);   // Enable pull-up resistor
    // Configure INT0
    MCUCR |= (1<<ISC01);    //Falling Edge detection
    MCUCR &= ~(1<<ISC00);    // (ISC01-1, ISC00-0)
    GIFR |= (1<<INTF0);     //Clear any pending interrupt
    GICR |= (1<<INT0);      // Enable INT0 interrupt
    sei();      // Enable global interrupts
    // Configure LCD
    LCD_Init();
    LCD_Gotoxy(0,0);
    LCD_String("Well Hello To");
    LCD_String_xy(1,3, "Embedded Sys.");
    while(1)
    {
        if(button_pressed){
            MS_DELAY(20);
            if(!(PIND & (1<<PIND2)))
            {
                PORTB ^= (1<<PORTB0);
                LCD_String_xy(0,0, "Button Pressed! ");
                LCD_String_xy(1,0, "                ");
                
                while (!(PIND & (1<<PIND2)))
                {
                    MS_DELAY(20);
                }
                MS_DELAY(20000);
                LCD_Gotoxy(0,0);
                LCD_String("Well Hello To   ");
                LCD_String_xy(1,3, "Embedded Sys.");
                button_pressed = 0;
            }   
        }
    }
    return (0);
}