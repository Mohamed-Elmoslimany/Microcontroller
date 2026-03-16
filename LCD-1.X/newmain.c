#define F_CPU 11059200UL
#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "my_lcd.h"

// Global flag to signal button press
volatile uint8_t button_pressed = 0;
// INT0 Interrupt Service Routine
ISR(INT0_vect)
{
    // Just set the flag - keep ISR fast!
    button_pressed = 1;
}

float analog = 0.0;
unsigned int adc_val = 0;
char message[17] = "";

void ReadVoltage()
{
    // Start the AD conversion
    ADCSRA|=(1<<ADSC);
    // Wait for the AD conversion to complete
    while ((ADCSRA & (1<<ADIF))==0);
    ADCSRA|=(1<<ADIF);
    adc_val = ADCW;
    analog = (float) adc_val * 5.0 / 1024.0;
    sprintf(message,"ADC: %1.4f    V", analog);
    LCD_String_xy(1,0, message);
}

int main()
{
    PORTD |= (1<<PORTD2);
    GICR|=(0<<INT1) | (1<<INT0) | (0<<INT2);
    MCUCR|=(0<<ISC11) | (0<<ISC10) | (1<<ISC01) | (0<<ISC00);
    MCUCSR|=(0<<ISC2);
    GIFR|=(0<<INTF1) | (1<<INTF0) | (0<<INTF2);
    ADMUX = 0x03;
    ADCSRA|=(1<<ADEN) | (0<<ADSC) | (0<<ADATE) | (0<<ADIF) | (0<<ADIE) | (1<<ADPS2) | (1<<ADPS1) | (0<<ADPS0);
    SFIOR |=(0<<ADTS2) | (0<<ADTS1) | (0<<ADTS0);
    LCD_Init();
    LCD_Gotoxy(0,0);
    LCD_String("Press the button");
    ReadVoltage();
    sei();
    while(1){
        if(button_pressed)  // Check if button was pressed
        {
            _delay_ms(50);
            if(!(PIND & (1<<PIND2)))
            {
                ReadVoltage();
            }
            while(!(PIND & (1<<PIND2))) // Wait for button release
            {
                _delay_ms(10);
            }
            _delay_ms(50);
            button_pressed = 0; // Clear the flag
        }
    }
    return (0);
}