#include <avr/io.h>
#include <avr/interrupt.h>

//outputs
#define OUT1 PB1 //Pin 6
#define OUT2 PB2 //Pin 7
#define ENA  PB4 //Pin 3

//inputs
#define INPUT PB3 //Pin 2
#define ISense 

//PWM defines
#define PWM_OVERFLOW 159;

typedef struct Input {
	int bI; //button Input on or off stored in the first bit
	int mI; //motor Input 8 bit value corelated with motor current

} Input;

void setEnableDuty(uint8_t dutyCycle){
	if (dutyCycle > PWM_OVERFLOW){
		dutyCycle = PWM_OVERFLOW
	}
	OCR1B = dutyCycle
}

void getSwitchInput(void){
	uint8_t switchIn = PINB & (1 << INPUT);
	return switchIn;
}

void getMCurrent(void){

}


void setMotorDir(motorCommand cmd){
}

void closeBrake(void) {
}

void openBrake(void) {
}

void stopBrake(void) {
}

Input getInput(void){

}

void getNextState(brakeState state, int input, smResult * result){
	//passes struct of new state and function pointer for action
}

void initTimers(void) {
	//Clear timer on compare match with OCR1C
	TCCR1 |= (1 << CTC1);
	//Set to PCK/4 Prescaling
	TCCR1 |= (1 << CS11) | (1 << CS10);
	//Turn on PWM based on Comparator B, Compare mode to clear on match
	GTCCR |= (1 << PWM1B) | (1 << COM1B1);
	//PLL Control and status
	PLLCSR |= (1 << PCKE) | (1 << PLLE);
	//When to "overflow"
	OCR1C = overflow;
	//When to clear
	OCR1B = pwm;
}

void adcInit(void) {
	ADCMUX |= 
}

void initIO(void) {
	//Set input pin with pull up
	PORTB |= (1 << INPUT);
	//Set output pin
	DDRB |= (1 << OUT1);
	DDRB |= (1 << OUT2);
	DDRB |= (1 << ENA);
}

void init(void) {
	initIO();
	initTimers();
	//setup state


	//enable interrupts last
	sei();
}

int main (void){
	
	init();

	while(1) {
		
		
	}
}
