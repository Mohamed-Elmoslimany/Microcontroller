#define F_CPU 11059200UL
#include <avr/io.h>
#include <avr/interrupt.h>



ISR(TIMER0_OVF_vect)
{
    TCNT0 = 0x94;
    PORTB ^= (1<<PORTB0);
}


int main()
{
    DDRB |= (1<<DDB0);
    PORTB &= ~(1<<PORTB0);

    //setup Timer 0 to overflow every 10ms
    TCCR0 = 0x05;
    TCNT0 = 0x94;
    OCR0 = 0;
    TIFR = (1<<TOV0);
    TIMSK = (1<<TOIE0);

    //Enable Interrupts
    sei();

    while(1);
    {

    }
    return (0);
}
