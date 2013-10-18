#include <avr/io.h>

#define OUTPUT PB1 //Pin 6
#define INPUT PB4 //Pin 3



void initIO(void) {
	//Set input pin with pull up
	PORTB |= (1 << INPUT);
	//Set output pin
	DDRB |= (1 << OUTPUT);
}

void initTimer(void) {
	//Set Compare mode to clear on match, set on $00
	TCCR1 |= (1 << COM1A1) | (1 << CTC1) | (1 << PWM1A);
	//Set to PCK/16384 Prescaling
	TCCR1 |= (1 << CS13) | (1 << CS12) | (1 << CS11) | (1 << CS10);
	//PLL Control and status
	PLLCSR |= (1 << PCKE) | (1 << PLLE);
	//When to "overflow" Sets to 50 Hz
	OCR1C = 77;
}

void init(void) {
	initIO();
	initTimer();
}

int main (void){
	
	init();
	//Whatever duty cycle you want
	//THIS IS HOW TO SET DUTY CYCLE FOR SERVO
	//OCR1A = (uint8_t)(77/2) /50%;

	//Run
	while(1) {

		uint8_t i = PINB;
		i = i & (1 << INPUT);
		if (i == 0) {
			OCR1A = (uint8_t)(77 / 2);
		} else {
			OCR1A = 0;
		}
	}
}
