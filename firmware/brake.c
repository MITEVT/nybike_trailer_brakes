#include "brake.h"

//PWM defines
#define PWM_OVERFLOW 159
#define CLOSE_DUTY 30
#define OPEN_DUTY 30

//Number of cycels to remain on specific speed profile value
#define INCREMENT_CYCLES 255
//Analog value of voltage that determines when to stop motor
#define CURRENT_THRESHOLD 100

uint8_t duty = 150;
uint8_t inc = -1;

typedef enum {
    OPEN,
    OPENING,
    CLOSED,
    CLOSING
 } moveState;

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
const uint8_t speedProfile[1] = {159};


const uint8_t PROFILE_SIZE = 1;

typedef struct Input_s{
	uint8_t bI; //button Input on or off stored in the first bit
	uint16_t mI; //motor Input 8 bit value corelated with motor current
} Input;


//Blocks until ADC is completed
uint16_t getMCurrent(void){
    start_adc();
    while (get_adc_running()) {
        
    }
	return get_value();
}

void setSpeed(uint8_t speed) {
    set_duty(speed);
}

void closeBrake() {
    set_out1_high();
    set_out2_low();
}

void openBrake() {
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
    		case OPEN: //Follow through to OPENING
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
    		case CLOSED: //Follow through to closed
            case CLOSING:
                state->state = OPENING;
                state->count = 0;
                state->profIndex = PROFILE_SIZE - 1;
                state->changeInState = 1;
                break;
    		case OPENING:
                if (state->closeCount > 0) {
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
//ISR(TIM1_COMPB_vect) {
ISR(TIM0_OVF_vect) {
    Input i = getInput();
    getNextState(&i, &state);
    if (state.changeInState) {
        state.changeInState = 0;
        doAction(&state);
    }
}



void initTimers(void) {
	set_up_timer(PWM_OVERFLOW);
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

int main (void){
    init();

    uint8_t t = 0;
    uint8_t duty = 150;
    uint8_t count = 0;
	while(1) {
        // Input i = getInput();
        // if (!i.bI && !t) {
        //     duty = 150;
        //     closeBrake(duty);
        //     t = 1;
        // } else if(i.bI && t) {
        //     stopBrake();
        //     t = 0;
        // } else if(!i.bI && t && duty > 10) {
        //     count++;
        //     if (count > 100) {
        //         count = 0;
        //         duty--;
        //         closeBrake(duty);
        //     }
        // }
	} 
    return 1;
}
