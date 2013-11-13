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
#define PWM_OVERFLOW 159
#define CLOSE_DUTY 30
#define OPEN_DUTY 30

typedef struct Input {
	int bI; //button Input on or off stored in the first bit
	int mI; //motor Input 8 bit value corelated with motor current

} Input;

void setEnableDuty(uint8_t dutyCycle){
	if (dutyCycle > PWM_OVERFLOW){
		dutyCycle = PWM_OVERFLOW;
	}
	OCR1B = dutyCycle;
}

uint8_t getSwitchInput(void){
	uint8_t switchIn = PINB & (1 << INPUT);
	return switchIn;
}

uint8_t getMCurrent(void){
	return 0;
}

void closeBrake(uint8_t speed) {
	setEnableDuty(0); //kill the output in case switching is dangerous
	PORTB |= (1 << OUT1);
	PORTB &= ~(1 << OUT2);
	setEnableDuty(speed); //start the output again
}

void openBrake(uint8_t speed) {
	setEnableDuty(0); //kill the output in case switching is dangerous
	PORTB &= ~(1 << OUT1);
	PORTB |= (1 << OUT2);
	setEnableDuty(speed); //start the motor controller output again
}

void stopBrake(void) {
	setEnableDuty(0); //kil the output
	//TODO: is sohould we brake the motor or can we leave this be
}

Input getInput(void){
	Input in;
	in.bI = getSwitchInput();
	in.mI = getMCurrent();
	return in;
}

void getNextState(void){
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
	OCR1C = PWM_OVERFLOW;
	//When to clear
	//don't output anything till we are ready
	OCR1B = 0;
}

void adcInit(void) {
	//ADCMUX |= 
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
