#define F_CPU 11059200UL
#include <avr/io.h>
#include <avr/interrupt.h>


volatile uint8_t t0_count = 0;

ISR(TIMER0_OVF_vect)
{
    TCNT0 = 0x94; 

    t0_count++;
    if (t0_count >= 100) 
    {
        t0_count = 0;
        PORTB ^= (1 << PB0); 
    }
}


int main(void)
{
    DDRB |= (1<<DDB0);
    PORTB &= ~(1<<PORTB0);

    //setup Timer 0 to overflow every 10ms
    TCCR0 = 0x05;
    TCNT0 = 0x94;
    OCR0 = 0;
    TIFR = (1<<TOV0);
    TIMSK = (1<<TOIE0);

    DDRD |= (1 << PD5) | (1 << PD7);

    TCCR1A = (1 << COM1A0); 
    TCCR1B = (1 << WGM12)   
           | (1 << CS12) | (1 << CS10); 

    OCR1A = 10799; // 0.5 Hz

    TCCR2 = (1 << WGM20) | (1 << WGM21)  
          | (1 << COM21)                 
          | (1 << CS22);                 

    OCR2 = 64; 
  
    sei();

    while (1);
    {
        
    }
}
