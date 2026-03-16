#define F_CPU 11059200UL
#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "my_lcd.h"

float analog = 0.0;
volatile unsigned int adc_val = 0;
char message[17] = "";
volatile char flag = 0;

ISR(ADC_vect)
{
    //adc_val = ADCL;          // Read low byte first
    //adc_val |= (ADCH << 8);  // Then high byte
    adc_val = ADCW;
    PORTB ^= (1<<PORTB0);
    flag = 1;
}

ISR(TIMER1_OVF_vect)
{
    TCNT1 = 0xD5D0;
}

int main()
{
    DDRB |= (1<<DDB0);
    PORTB |= (1<<PORTB0);

    TCCR1A=(0<<COM1A1) | (0<<COM1A0) | (0<<COM1B1) | (0<<COM1B0) | (0<<WGM11) | (0<<WGM10);
    TCCR1B=(0<<ICNC1) | (0<<ICES1) | (0<<WGM13) | (0<<WGM12) | (1<<CS12) | (0<<CS11) | (1<<CS10);
    TCNT1H=0xD5;
    TCNT1L=0xD0;
    TIFR |= (1<<TOV1);
    TIMSK |= (1<<TOIE1);

    ADMUX = 0x03;
    ADCSRA|=(1<<ADEN) | (1<<ADSC) | (1<<ADATE) | (1<<ADIF) | (1<<ADIE) | (1<<ADPS2) | (1<<ADPS1) | (0<<ADPS0);
    SFIOR |=(1<<ADTS2) | (1<<ADTS1) | (0<<ADTS0);

    LCD_Init();
    LCD_Gotoxy(0,0);
    sei();

    while(1)
    {
        if(flag) // ADC finished conversion
        {
            analog = (float) adc_val * 5.0 / 1024.0;
            sprintf(message, "ADC: %1.4f    V", analog);
            LCD_String_xy(1,0, message);
            flag = 0; // Clear the flag
        }
    }
    return (0);
}