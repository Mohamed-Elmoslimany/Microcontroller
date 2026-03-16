#define F_CPU 11059200UL
#define BAUD_PRESCALE          (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)

#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <inttypes.h>
#include "my_lcd.h"

// Global flag to signal button press
volatile uint8_t button_pressed = 0;
volatile uint8_t interupt_2 = 0;

// INT0 Interrupt Service Routine
ISR(INT0_vect)
{
    // Just set the flag - keep ISR fast!
    button_pressed = 1;
}

ISR(INT2_vect)
{
    // Just set the flag keep ISR fast!
    interupt_2 = 1;
}

float analog = 0.0;
unsigned int adc_val = 0;
int threshold_counter = 0;
unsigned int threshold_0 = 256;
unsigned int threshold_1 = 512;
unsigned int threshold_2 = 768;
int current_threshold = 256;
char message[17] = "";
char state[5] = "";

void ReadVoltage()
{
    // Start the AD conversion
    ADCSRA|=(1<<ADSC);
    // Wait for the AD conversion to complete
    while ((ADCSRA & (1<<ADIF))==0);
    ADCSRA|=(1<<ADIF);
    adc_val = ADCW;
//    analog = (float) adc_val * 5.0 / 1024.0;
//    sprintf(message,"ADC: %4dV", adc_val);
    if (adc_val >= current_threshold){
        sprintf(state,"ABOVE");
        sprintf(message,"%5s  THR: %3d", state, current_threshold);
        PORTB |= (1<<PORTB1);
        PORTB &= ~(1<<PORTB0);
    }
    else if (adc_val < current_threshold){
        sprintf(state,"BELOW");
        sprintf(message,"%5s  THR: %3d", state, current_threshold);
        PORTB |= (1<<PORTB0);
        PORTB &= ~(1<<PORTB1);
    }
    LCD_String_xy(0,0, message);
    sprintf(message,"ADC: %4d", adc_val);
    LCD_String_xy(1,0, message);
}

void UpdateLCD()
{
    ReadVoltage();
//    sprintf(message,"threshold:  %4d", current_threshold);
//    LCD_String_xy(1,0, message);
    printf("ADC:%4d STATE:%5s\r\n", adc_val, state);
    printf("Current Threshold: %4d\r\n", current_threshold);
}

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

static FILE uart_str = FDEV_SETUP_STREAM(UART_putChar, UART_getChar, _FDEV_SETUP_RW);

int main()
{
    PORTD |= (1<<PORTD2);
    PORTB &= ~(1<<PORTB2);
    GICR|=(0<<INT1) | (1<<INT0) | (1<<INT2);
    MCUCR|=(0<<ISC11) | (0<<ISC10) | (1<<ISC01) | (0<<ISC00);
    MCUCSR|=(1<<ISC2);
    GIFR|=(0<<INTF1) | (1<<INTF0) | (1<<INTF2);

    ADMUX = 0x03;
    ADCSRA|=(1<<ADEN) | (0<<ADSC) | (0<<ADATE) | (0<<ADIF) | (0<<ADIE) | (1<<ADPS2) | (1<<ADPS1) | (0<<ADPS0);
    SFIOR |=(0<<ADTS2) | (0<<ADTS1) | (0<<ADTS0);

    // Setup PBG as output
    DDRB |= (1<<DDB0);
    PORTB |= (1<<PORTB0);   // LED OFF (assuming active-low LED)
    // Setup PBG as output
    DDRB |= (1<<DDB1);
    PORTB |= (1<<PORTB1);   // LED OFF (assuming active-low LED)
    
    UART_init(9600);
    stdin = stdout = &uart_str;
    
    uint32_t x = F_CPU;
    float y = 789.4567;

    unsigned char cmd = 0;

    printf("Testing UART!\n");
    printf("Printing long and float numbers\r\n");
    printf("Long value: %lu, Float value: %f \r\n", x, y);
    
    LCD_Init();
    UpdateLCD();
    
    sei();
    while(1){
        
        cmd = UART_getChar_NonBlocking();
        if(cmd != -1)  // If valid data received
        {
            if(cmd == 'L' || cmd == 'l'){
                current_threshold = threshold_0;
                UpdateLCD();
            } else if(cmd == 'M' || cmd == 'm'){
                current_threshold = threshold_1;
                UpdateLCD();
            } else if(cmd == 'H' || cmd == 'h'){
                current_threshold = threshold_2;
                UpdateLCD();
            }
//            ReadVoltage();
        }
        
        if(button_pressed)  // Check if button was pressed
        {
            _delay_ms(50);
            if(!(PIND & (1<<PIND2)))
            {
                UpdateLCD();
            }
            while(!(PIND & (1<<PIND2))) // Wait for button release
            {
                _delay_ms(10);
            }
            _delay_ms(50);
            button_pressed = 0; // Clear the flag
        }
        
        if (interupt_2)
        {
            _delay_ms(20);
            if((PINB & (1<<PINB2)))
            {
                threshold_counter++;
                switch(threshold_counter){
                        case 0: 
                            current_threshold = threshold_0; 
                            break;
                        case 1: 
                            current_threshold = threshold_1; 
                            break;
                        case 2: 
                            current_threshold = threshold_2; 
                            break;
                        case 3: 
                            threshold_counter = 0; 
                            current_threshold = threshold_0;
                }
//                PORTB ^= (1<<PORTB0);
                UpdateLCD();
                while ((PINB & (1<<PINB2)))
                {
                    _delay_ms(20);
                }
                _delay_ms(50);
            } 
            interupt_2 = 0; // Clear the flag
        }
    }
    return (0);
}