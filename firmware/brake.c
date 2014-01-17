#include "brake.h"

//PWM defines
#define PWM_OVERFLOW 159
#define CLOSE_DUTY 30
#define OPEN_DUTY 30

//Number of cycels to remain on specific speed profile value
#define INCREMENT_CYCLES 50
//Analog value of voltage that determines when to stop motor

#define CURRENT_THRESHOLD 500 //1 AMP with 1OOhm resistor

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
const uint8_t speedProfile[15] = {
    150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150
};




//Blocks until ADC is completed
uint16_t getMCurrent(void){
    start_adc();
    while (adc_is_running()) {
        
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
    set_out1_low();
    set_out2_high();
	set_duty(speed); //start the output again
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
            	if (1){//in->mI < CURRENT_THRESHOLD) {
                	state->count++;
                    state->closeCount++;
                	if (state->count > INCREMENT_CYCLES){
                        stopBrake();
                        cli();
                        break;
                    	state->count = 0;
                    	if (state->profIndex < sizeof(speedProfile)/sizeof(*speedProfile)) {
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
                state->changeInState = 1;
                state->count = 0;
                state->profIndex = sizeof(speedProfile)/sizeof(*speedProfile) - 1;
                break;
    		case OPENING:
                if (state->closeCount > 0) {
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
ISR(TIM1_OVF_vect) {
    Input i = getInput();
    getNextState(&i, &state);
    if (state.changeInState) {
        state.changeInState = 1;
        doAction(&state);
    }
    //reset_timer();
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
    state.changeInState = 0;

	//enable interrupts
	sei();
}


//TODO Button stabalization, Threshold voltage, Unbrake timing, speed profile
int main (void){
    init();
 //    initIO();
 //    initADC();

 //    set_enable_high();
 //    set_out1_high();
 //    _delay_ms(2000);
	// while(1) {
 //        if (getMCurrent() > CURRENT_THRESHOLD) {
 //            //ADMUX &= ~(1 << ADEN);
 //            set_enable_low();
 //            _delay_ms(2000);
 //            set_enable_high();
 //            _delay_ms(1000);
 //            //ADMUX |= (1 << ADEN);
 //        }
	// } 

    while(1) {

    }

    return 1;
}
