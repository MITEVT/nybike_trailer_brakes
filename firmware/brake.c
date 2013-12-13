#include "brake.h"

//PWM defines
#define PWM_OVERFLOW 159
#define CLOSE_DUTY 30
#define OPEN_DUTY 30

//Number of cycels to remain on specific speed profile value
#define INCREMENT_CYCLES 255
//Analog value of voltage that determines when to stop motor
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
    uint16_t closeCount;
    moveState state;
} State; 

State state;

//Allows for gradient opening and closing
const uint8_t speedProfile[32] = {
    30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30
};

typedef struct Input{
	uint8_t bI; //button Input on or off stored in the first bit
	uint8_t mI; //motor Input 8 bit value corelated with motor current
} Input;


//Blocks until ADC is completed
uint8_t getMCurrent(void){
    start_adc();
    while (get_adc_running()) {
        
    }
	return get_value();
}

void closeBrake(uint8_t speed) {
	set_duty(0); //kill the output in case switching is dangerous
    set_out1_high();
    set_out2_low();
	set_duty(speed); //start the output again
}

void openBrake(uint8_t speed) {
	set_duty(0); //kill the output in case switching is dangerous
    set_out1_low()
    set_out2_high()
	set_duty(speed); //start the output again
}

void stopBrake(void) {
	set_duty(0); //kill the output
    set_out1_high();
    set_out2_high();
    set_duty(PWM_OVERFLOW); //start the output again
}

Input getInput(void){
	Input in;
	in.bI = get_input();
	in.mI = getMCurrent();
	return in;
}

//Takes input and curent state and sets next state
void getNextState(Input* in, State* state){
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
                    state->closeCount++;
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
                if (state->closeCount > 0) {
        			state->state = OPENING;
                	state->count++;
                    state->closeCount--;
                	if (state->count > INCREMENT_CYCLES) {
                    	state->count = 0;
                    	if (state->profIndex > -1) {
                        	state->profIndex--;
                   		}
                	}
                } else {
                    state->state = OPEN;
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

//Takes a state and completes the required action
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

//Interupt function for when Timer 1 gets a compare match, or is cleared
//Gets input, finds next state, and does required action
ISR(TIM1_COMPB_vect) {
    Input i = getInput();
    getNextState(&i, &state);
    doAction(&state);
}



void initTimers(void) {
	set_up_timer(PWM_OVERFLOW);
}

void initADC(void) {
    set_up_adc();
}


void initIO(void) {
	set_up_input();
    set_up_output();
}

void init(void) {
	initIO();
	initTimers();
    initADC();
	
    //Initial State
    state.state = OPEN;
    state.closeCount = 0;

	//enable interrupts
	sei();
}

int main (void){
	
	init();

	while(1) {
	} 
    return 1;
}
