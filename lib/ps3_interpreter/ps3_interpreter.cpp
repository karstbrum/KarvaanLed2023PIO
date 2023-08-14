#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "ps3_interpreter.h"


controller_handler::controller_handler(){

};

void controller_handler::setup_leds(uint8_t numSidesLED_, uint8_t pixelsPerSide_[], uint8_t numPins_, uint8_t sidesPerPin_[], 
uint8_t LEDPin_[], uint16_t Ts_){


    
};

void controller_handler::setup_bulbs( uint8_t Bulbpins_[], uint8_t numSidesBulb_, uint8_t bulbsPerSide_[],uint16_t Ts_){
    
};

// parse the logic of the controller states
bool controller_handler::combination_parser(int input){

    // first check if any of the states are set to true
    // loop to all the states that are true and see if it is possible to set it
    static bool set_state = true;

    // loop through number of inputs
    for(int k=0; k<INPUTS; k++){
        // if one of the state is toggled, check if combination is possible
        if(controller_toggle[k]){
            // both need to be true, otherwise input is not possible
            set_state = set_state && combination_array[k][input];
        };
    };

    return set_state;
    
};

// loop through the function pointer matrix
// 
void controller_handler::function_selector(){

    // just use if statements 
    if (controller_handler::controller_toggle[CROSS] && !controller_handler::controller_toggle[SELECT]){
        controller_handler::cross();
    }
    if (controller_handler::controller_toggle[SQUARE] && !controller_handler::controller_toggle[SELECT]){
        controller_handler::square();
    }
    if (controller_handler::controller_toggle[TRIANGLE] && !controller_handler::controller_toggle[SELECT]){
        controller_handler::triangle();
    }
    if (controller_handler::controller_toggle[CIRCLE] && !controller_handler::controller_toggle[SELECT]){
        controller_handler::circle();
    }
    if (controller_handler::controller_toggle[START]){
        controller_handler::start();
    }
    if (controller_handler::controller_toggle[SELECT]){
        controller_handler::select();
    }
    if (controller_handler::controller_toggle[L_TRIGGER]){
        controller_handler::l_trigger();
    }
    if (controller_handler::controller_toggle[R_TRIGGER]){
        controller_handler::r_trigger();
    }
    if (controller_handler::controller_toggle[L_BUMPER]){
        controller_handler::l_bumper();
    }
    if (controller_handler::controller_toggle[R_BUMPER]){
        controller_handler::r_bumper();
    }
    if ((controller_handler::controller_toggle[L_STICK_X] || controller_handler::controller_toggle[L_STICK_Y] || 
    controller_handler::controller_toggle[R_STICK_X] || controller_handler::controller_toggle[R_STICK_Y]) && 
    !controller_handler::controller_toggle[CROSS] && !controller_handler::controller_toggle[SQUARE] && 
    !controller_handler::controller_toggle[TRIANGLE] && !controller_handler::controller_toggle[CIRCLE]){
        controller_handler::sticks();
    }
    if (controller_handler::controller_toggle[L_STICK_PRESS]){
        controller_handler::l_stick_press();
    }
    if (controller_handler::controller_toggle[R_STICK_PRESS]){
        controller_handler::r_stick_press();
    }

};

// make all modes
void controller_handler::cross(){
    Serial.println("cross");
};
void controller_handler::square(){
    Serial.println("square");
}; 
void controller_handler::triangle(){
    Serial.println("triangle");
};
void controller_handler::circle(){
    Serial.println("circle");
};
void controller_handler::start(){
    Serial.println("start");
};
void controller_handler::select(){
    Serial.println("select");
};
void controller_handler::l_trigger(){
    Serial.println("l trigger");
};
void controller_handler::r_trigger(){
    Serial.println("r trigger");
};
void controller_handler::l_bumper(){
    Serial.println("l bumper");
};
void controller_handler::r_bumper(){
    Serial.println("r bumper");
};
void controller_handler::sticks(){
    Serial.println("sticks");
};
void controller_handler::l_stick_press(){
    Serial.println("l stick");
};
void controller_handler::r_stick_press(){
    Serial.println("r stick");
};


// declare object
controller_handler ctrl;

// setup a new controller
void setup_controller(char* address){

    Ps3.attach(controller_callbacks);
    Ps3.attachOnConnect(on_connect);
    Ps3.begin(address);

};

// define what to do on connect
void on_connect(){
    // define function what to do on connect (flash??)
    //Ps3.setRumble(100,1);
};

void controller_callbacks(){

    // update states of controller

    // right thumb buttons
    if( Ps3.event.button_down.cross ){
        if(ctrl.combination_parser(CROSS)){
            ctrl.controller_toggle[CROSS] = true;
            ctrl.controller_states[CROSS] = 1;
        };
    };
    if( Ps3.event.button_up.cross ){
        ctrl.controller_toggle[CROSS] = false;
        ctrl.controller_states[CROSS] = 0;
    };
    if( Ps3.event.button_down.square ){
        if(ctrl.combination_parser(SQUARE)){
            ctrl.controller_toggle[SQUARE] = true;
            ctrl.controller_states[SQUARE] = 1;
        };
    };
    if( Ps3.event.button_up.square ){    
        ctrl.controller_toggle[SQUARE] = false;
        ctrl.controller_states[SQUARE] = 0;
    };
    if( Ps3.event.button_down.triangle ){
        if(ctrl.combination_parser(TRIANGLE)){
            ctrl.controller_toggle[TRIANGLE] = true;
            ctrl.controller_states[TRIANGLE] = 1;
        };
    };  
    if( Ps3.event.button_up.triangle ){
        ctrl.controller_toggle[TRIANGLE] = false;
        ctrl.controller_states[TRIANGLE] = 0;
    };
    if( Ps3.event.button_down.circle ){
        if(ctrl.combination_parser(CIRCLE)){
            ctrl.controller_toggle[CIRCLE] = true;
            ctrl.controller_states[CIRCLE] = 1;
        };
    };
    if( Ps3.event.button_up.circle ){
        ctrl.controller_toggle[CIRCLE] = false;
        ctrl.controller_states[CIRCLE] = 0;
    };
    
    // start and select
    if (Ps3.event.button_down.start){
        if(ctrl.combination_parser(START)){
            ctrl.controller_toggle[START] = true;
            ctrl.controller_states[START] = 1;
        };
    };
    if (Ps3.event.button_up.start){
        ctrl.controller_toggle[START] = false;
        ctrl.controller_states[START] = 0;
    };
    if (Ps3.event.button_down.select){
        if(ctrl.combination_parser(SELECT)){
            ctrl.controller_toggle[SELECT] = true;
            ctrl.controller_states[SELECT] = 1;
        };
    };
    if (Ps3.event.button_up.select){
        ctrl.controller_toggle[SELECT] = false;
        ctrl.controller_states[SELECT] = 0;
    };

    // trigger and bumper 1 is bumpler, 2 is trigger
    if( abs(Ps3.event.analog_changed.button.l1) ){
        if(Ps3.data.analog.button.l1 >0 && ctrl.combination_parser(L_BUMPER)){
            ctrl.controller_toggle[L_BUMPER] = true;
            ctrl.controller_states[L_BUMPER] = 1;
        } else {
            ctrl.controller_toggle[L_BUMPER] = false;
            ctrl.controller_states[L_BUMPER] = 0;
        };
    };
    if( abs(Ps3.event.analog_changed.button.r1) ){
        if(Ps3.data.analog.button.r1 >0 && ctrl.combination_parser(R_BUMPER)){
            ctrl.controller_toggle[R_BUMPER] = true;
            ctrl.controller_states[R_BUMPER] = 1;
        } else {
            ctrl.controller_toggle[R_BUMPER] = false;
            ctrl.controller_states[R_BUMPER] = 0;
        };
    };
    if( abs(Ps3.event.analog_changed.button.l2) ){
        if(Ps3.data.analog.button.l2 >0 && ctrl.combination_parser(L_TRIGGER)){
            ctrl.controller_toggle[L_TRIGGER] = true;
            ctrl.controller_states[L_TRIGGER] = Ps3.data.analog.button.l2;
        } else {
            ctrl.controller_toggle[L_TRIGGER] = false;
            ctrl.controller_states[L_TRIGGER] = 0;
        };
    };
    if( abs(Ps3.event.analog_changed.button.r2) ){
        if(Ps3.data.analog.button.r2 >0 && ctrl.combination_parser(R_TRIGGER)){
            ctrl.controller_toggle[R_TRIGGER] = true;
            ctrl.controller_states[R_TRIGGER] = Ps3.data.analog.button.r2;
        } else {
            ctrl.controller_toggle[R_TRIGGER] = false;
            ctrl.controller_states[R_TRIGGER] = 0;
        };
    };

    // sticks logic for X and Y is same, so just use X for both X and Y
    if( abs(Ps3.event.analog_changed.stick.lx) + abs(Ps3.event.analog_changed.stick.ly) > 2 ){
        if((abs(Ps3.data.analog.stick.lx)+abs(Ps3.data.analog.stick.ly))>2 && ctrl.combination_parser(L_STICK_X)){
            ctrl.controller_toggle[L_STICK_X] = true;
            ctrl.controller_states[L_STICK_X] = Ps3.data.analog.stick.lx+1;
            ctrl.controller_toggle[L_STICK_Y] = true;
            ctrl.controller_states[L_STICK_Y] = Ps3.data.analog.stick.ly+1;
        } else {
            ctrl.controller_toggle[L_STICK_X] = false;
            ctrl.controller_states[L_STICK_X] = 0;
            ctrl.controller_toggle[L_STICK_Y] = false;
            ctrl.controller_states[L_STICK_Y] = 0;
        };
    };
    if( abs(Ps3.event.analog_changed.stick.rx) + abs(Ps3.event.analog_changed.stick.ry) > 2 ){
        if((abs(Ps3.data.analog.stick.rx)+abs(Ps3.data.analog.stick.ry))>2 && ctrl.combination_parser(R_STICK_X)){
            ctrl.controller_toggle[R_STICK_X] = true;
            ctrl.controller_states[R_STICK_X] = Ps3.data.analog.stick.rx+1;
            ctrl.controller_toggle[R_STICK_Y] = true;
            ctrl.controller_states[R_STICK_Y] = Ps3.data.analog.stick.ry+1;
        } else {
            ctrl.controller_toggle[R_STICK_X] = false;
            ctrl.controller_states[R_STICK_X] = 0;
            ctrl.controller_toggle[R_STICK_Y] = false;
            ctrl.controller_states[R_STICK_Y] = 0;
        };
    };

    // stick press l3 and r3
    if (Ps3.event.button_down.l3){
        if(ctrl.combination_parser(L_STICK_PRESS)){
            ctrl.controller_toggle[L_STICK_PRESS] = true;
            ctrl.controller_states[L_STICK_PRESS] = 1;
        };
    };
    if (Ps3.event.button_up.l3){
        ctrl.controller_toggle[L_STICK_PRESS] = false;
        ctrl.controller_states[L_STICK_PRESS] = 0;
    };
    if (Ps3.event.button_down.r3){
        if(ctrl.combination_parser(R_STICK_PRESS)){
            ctrl.controller_toggle[R_STICK_PRESS] = true;
            ctrl.controller_states[R_STICK_PRESS] = 1;
        };
    };
    if (Ps3.event.button_up.r3){
        ctrl.controller_toggle[R_STICK_PRESS] = false;
        ctrl.controller_states[R_STICK_PRESS] = 0;
    };

};