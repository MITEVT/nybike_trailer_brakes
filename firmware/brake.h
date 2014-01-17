#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

//outputs
#define OUT1 PB1 //Pin 6
#define OUT2 PB0 //Pin 7
#define ENA  PB4 //Pin 3

//inputs
#define INPUT PB3 //Pin 2
#define ISense PB2 


#define set_up_input() (PORTB |= (1 << INPUT))
#define set_up_output() (DDRB |= (1 << OUT1) | (1 << OUT2) | (1 << ENA))

//Set single ended input on PB2, Prescale by 8 to get 125kHz Sample frequency, and enable
#define set_up_adc() {ADMUX |= (1 << MUX0) | (0 << ADLAR); ADCSRA |= (1 << ADPS1) | (1 << ADPS0); ADCSRA |= (1 << ADEN);}

#define set_up_timer(pwm) { OCR1B = 1; \
							OCR1C = pwm; \
							TCCR1 |= (1 << CS11) | (1 << CS10); \
							GTCCR |= (1 << PWM1B) | (1 << COM1B1); \
							PLLCSR |= (1 << PLLE); \
							_delay_us(100); \
							while(!(PLLCSR & (1 << PLOCK))) {} \
							PLLCSR |= (1 << PCKE); \
							TIMSK |= (1 << TOIE1); }

#define disable_interrupt() (TIMSK &= ~(1 << TOIE1))
#define reset_timer() (TCNT1 = 0)
#define enable_interrupt() (TIMSK |= (1 << TOIE1))

#define set_duty(duty) (OCR1B = duty)
#define get_input() ((PINB & (1 << INPUT)))

#define set_out1_high() (PORTB |= (1 << OUT1))
#define set_out1_low() (PORTB &= ~(1 << OUT1))
#define set_out2_high() (PORTB |= (1 << OUT2))
#define set_out2_low() (PORTB &= ~(1 << OUT2))
#define set_enable_high() (PORTB |= (1 << ENA))
#define set_enable_low() (PORTB &= ~(1 << ENA))

#define start_adc() (ADCSRA |= (1 << ADSC))
#define adc_is_running() ((ADCSRA & (1 << ADSC)) >> ADSC)
#define get_value() (ADCL + (ADCH << 8))