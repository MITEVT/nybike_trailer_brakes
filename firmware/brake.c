#include "brake.h"

/* **************************************
 * Instruction to change strength:
 * 
 * Change CURRENT_THRESHOLD
 * Higher number means higher cut off current
 * Be careful, too high and it won't ever cut off, causing many amps to be continuosly drawn, small fire may ensue
 * CHANGE NOTHING ELSE
 * **************************************/


//PWM defines
#define PWM_OVERFLOW 159
#define CLOSE_DUTY 30
#define OPEN_DUTY 30

//Number of cycels to remain on specific speed profile value
#define INCREMENT_CYCLES 50
//Analog value of voltage that determines when to stop motor

#define CURRENT_THRESHOLD 370 //At running it creates 1.6V drop
#define REVERSE_CURRENT_THRESHOLD 350

#define SENSE_HOLD_OFF 50
#define REVERSE_SENSE_HOLD_OFF 50

#define REVERSE_TAP

uint8_t duty = 150;
uint8_t inc = -1;

typedef enum{
    OPEN,
    OPENING,
    CLOSED,
    CLOSING
 } moveState;

typedef struct Input_s{
    uint8_t bI; //button Input on or off stored in the first bit
    uint16_t mI; //motor Input 8 bit value corelated with motor current
} Input;

typedef struct State {
    uint8_t changeInState;
    uint8_t count;
    uint8_t profIndex;
    uint16_t closeCount;
    moveState state;
} State; 

State state;

//Allows for gradient opening and closing
// const uint8_t speedProfile[32] = {
//     150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150
// };
const uint8_t speedProfile[2] = {159, 159};
const uint8_t PROFILE_SIZE = 2;

//Blocks until ADC is completed
uint16_t getMCurrent(void){
    start_adc();
    while (adc_is_running()) {
        
    }
	return get_value();
}

void setSpeed(uint8_t speed) {
    set_duty(speed);
}

void closeBrake(void) {
    set_out1_high();
    set_out2_low();
}

void openBrake(void) {
    set_out1_low();
    set_out2_high();
}

void stopBrake(void) {
	set_duty(0); //kill the output
    set_out1_high();
    set_out2_high();
}

Input getInput(void){
	Input in;
	in.bI = get_input();
	in.mI = getMCurrent();
	return in;
}

//Takes input and curent state and sets next state
void getNextState(Input* in, State* state){
    if (in->bI == 0) {
    	switch (state->state) {
    		case OPEN:
    		case OPENING:
    			state->state = CLOSING;
    			state->count = 0;
    			state->profIndex = 0;
                state->changeInState = 1;
    			break;
    		case CLOSING:
            	if (in->mI < CURRENT_THRESHOLD || (state->count < SENSE_HOLD_OFF && state->profIndex == 0)) {
                	state->count++;
                    state->closeCount++;
                	if (state->count > INCREMENT_CYCLES){
                    	state->count = 0;
                    	if (state->profIndex < PROFILE_SIZE) {
                        	state->profIndex ++;
                    	}
                	}
            	} else {
                	state->state = CLOSED;
                    state->changeInState = 1;
            	}
            	break;
    		case CLOSED:
                break;
    	}
    } else {
    	switch (state->state) {
    		case CLOSED:
            case CLOSING:
                state->state = OPENING;
                state->count = 0;
                state->profIndex = PROFILE_SIZE - 1;
                state->changeInState = 1;
                break;
    		case OPENING:
                #ifndef REVERSE_TAP
                if (state->closeCount > 0) {
                #else
                if ((in->mI < REVERSE_CURRENT_THRESHOLD && state->closeCount > 0) || (state->count < REVERSE_SENSE_HOLD_OFF && state->profIndex == PROFILE_SIZE - 1)) {
                #endif
                	state->count++;
                    state->closeCount--;
                	if (state->count > INCREMENT_CYCLES) {
                    	state->count = 0;
                    	if (state->profIndex > 0) {
                        	state->profIndex--;
                   		}
                	}
                } else {
                    state->state = OPEN;
                    state->closeCount = 0;
                    state->changeInState = 1;
                }
    			break;
    		case OPEN:
                break;
    	}
    }
}

//Takes a state and completes the required action
void doAction(State* state) {
    setSpeed(speedProfile[state->profIndex]);
    switch (state->state) {
        case OPEN:
            stopBrake();
            break;
        case OPENING:
            openBrake();
            break;
        case CLOSING:
            closeBrake();
            break;
        case CLOSED:
            stopBrake();
            break;
    }
}

//Interupt function for when Timer 1 gets a compare match, or is cleared
//Gets input, finds next state, and does required action
ISR(TIM0_OVF_vect) {
    Input i = getInput();
    getNextState(&i, &state);
    if (state.changeInState) {
        state.changeInState = 0;
        doAction(&state);
    }
}



void initTimers(void) {
	set_up_timer1(PWM_OVERFLOW);
    set_up_timer0();
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
    state.changeInState = 0;
    state.profIndex = 0;

	//enable interrupts
	sei();
}


//TODO Button stabalization, Threshold voltage, Unbrake timing, speed profile
int main (void){
    init();
    while(1) {

    }
    return 1;
}

