#include <avr/io.h>
#include <avr/interrupt.h>

//outputs
#define OUT1 PB1 //Pin 6
#define OUT2 PB0 //Pin 7
#define ENA  PB4 //Pin 3

//inputs
#define INPUT PB3 //Pin 2
#define ISense PB2

//PWM defines
#define PWM_OVERFLOW 159
#define CLOSE_DUTY 30
#define OPEN_DUTY 30

#define INCREMENT_CYCLES 256
#define CURRENT_THRESHOLD 255

typedef enum {
    OPEN,
    OPENING,
    CLOSED,
    CLOSING
 } moveState;

typedef struct State {
    uint8_t count;
    uint8_t profIndex;
    moveState state;
} State; 

State state;

const uint8_t speedProfile[32] = {
    30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30
};

typedef struct Input{
	uint8_t bI; //button Input on or off stored in the first bit
	uint8_t mI; //motor Input 8 bit value corelated with motor current
} Input;

/*typedef struct Action {
	moveState newState;
	void (*actPointer)(State *s);
} Action;*/

void setEnableDuty(uint8_t dutyCycle){
	if (dutyCycle > PWM_OVERFLOW){
		dutyCycle = PWM_OVERFLOW;
	}
	OCR1B = dutyCycle;
}

uint8_t getSwitchInput(void){
	uint8_t switchIn = (PINB & (1 << INPUT)) >> INPUT;
	return switchIn;
}


//blocking until conversion finish
uint8_t getMCurrent(void){
    ADCSRA |= (1 << ADSC);
    while ((ADCSRA & (1 << ADSC)) >> ADSC == 1) {
        
    }
	return ADCH;
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
	setEnableDuty(0); //kill the output
    PORTB |= (1 << OUT1) | (1 << OUT2);
    setEnableDuty(PWM_OVERFLOW);
	//TODO: is sohould we brake the motor or can we leave this be
}

Input getInput(void){
	Input in;
	in.bI = getSwitchInput();
	in.mI = getMCurrent();
	return in;
}

//Takes input and curent state and sets next action to be completed
void getNextState(Input * in, State * state){
	//passes struct of new state and function pointer for action
    if (in->bI == 1) {
    	switch (state->state) {
    		case OPEN:
    			state->state = CLOSING;
    			state->count = 0;
    			state->profIndex = 0;
    			break;
    		case OPENING:
    			state->state = CLOSING;
    			state->count = 0;
    			state->profIndex = 0;
    			break;
    		case CLOSING:
            	if (in->mI < CURRENT_THRESHOLD) {
                	state->state = CLOSING;
                	state->count++;
                	if (state->count > INCREMENT_CYCLES){
                    	state->count = 0;
                    	if (state->profIndex < sizeof(speedProfile)) {
                        	state->profIndex ++;
                    	}
                	}
            	} else {
                	state->state = CLOSED;
            	}
            	break;
    		case CLOSED:
    			state->state = CLOSED;
    			break;
    	}
    } else {
    	switch (state->state) {
    		case OPEN:
    			state->state = OPEN;
    			break;
    		case OPENING:
    			state->state = OPENING;
            	state->count ++;
            	if (state->count > INCREMENT_CYCLES) {
                	state->count = 0;
                	if (state->profIndex > -1) {
                    	state->profIndex--;
               		}
                //Need to discuss time for opening
            	}
    			break;
    		case CLOSING:
            	state->state = OPENING;
            	state->count = 0;
            	state->profIndex = sizeof(speedProfile) - 1;
            	break;
    		case CLOSED:
            	state->state = OPENING;
            	state->count = 0;
            	state->profIndex = sizeof(speedProfile) - 1;
    			break;
    	}
    }
}

void doAction(State* state) {
    switch (state->state) {
        case OPEN:
            stopBrake();
            break;
        case OPENING:
            openBrake(speedProfile[state->profIndex]);
            break;
        case CLOSING:
            closeBrake(speedProfile[state->profIndex]);
            break;
        case CLOSED:
            stopBrake();
            break;
    }
}

ISR(TIM1_COMPB_vect) {
    Input i = getInput();
    getNextState(&i, &state);
    doAction(&state);
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
    //Set to interrupt on overflow
    TIMSK |= (1 << TOIE1);
	//When to "overflow"
	OCR1C = PWM_OVERFLOW;
	//When to clear
	//don't output anything till we are ready
	OCR1B = 0;
}

void initADC(void) {
    //Set ADC to use VCC as voltage reference, Single Ended Input on PB2, Enable ADC
    //Not left adjusted FOR SAFETY
    ADMUX |= (1 << MUX0) | (1 << ADLAR);
	ADCSRA |= (1 << ADEN);
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
    initADC();
	
    state.state = OPEN;

	//enable interrupts last
	sei();
}

int main (void){
	
	init();

	while(1) {
	} 
}
