#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "ps3_interpreter.h"


controller_handler::controller_handler(){

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
    if (Ps3.event.button_up.start){
        if(ctrl.combination_parser(START)){
            ctrl.controller_toggle[START] = true;
            ctrl.controller_states[START] = 1;
        };
    };
    if (Ps3.event.button_down.start){
        ctrl.controller_toggle[START] = false;
        ctrl.controller_states[START] = 0;
    };
    if (Ps3.event.button_up.start){
        if(ctrl.combination_parser(SELECT)){
            ctrl.controller_toggle[SELECT] = true;
            ctrl.controller_states[SELECT] = 1;
        };
    };
    if (Ps3.event.button_down.start){
        ctrl.controller_toggle[SELECT] = false;
        ctrl.controller_states[SELECT] = 0;
    };

    // triggers 1 is bumpler, 2 is trigger
    if( abs(Ps3.event.analog_changed.button.l1) ){
        if(Ps3.data.analog.button.l1 >0){
            Serial.println("l1");
        };
    };
    if( abs(Ps3.event.analog_changed.button.r1) ){
        Serial.print("Pressing the right trigger button: ");
        Serial.println(Ps3.data.analog.button.r2, DEC);
    };

    if( abs(Ps3.event.analog_changed.button.l2) ){
        Serial.print("Pressing the left trigger button: ");
        Serial.println(Ps3.data.analog.button.l2, DEC);
    };

    if( abs(Ps3.event.analog_changed.button.r2) ){
        Serial.print("Pressing the right trigger button: ");
        Serial.println(Ps3.data.analog.button.r2, DEC);
    };

    // bumpers


    // sticks
    if( abs(Ps3.event.analog_changed.stick.lx) + abs(Ps3.event.analog_changed.stick.ly) > 2 ){
       if(ctrl.combination_parser(SELECT)){
            ctrl.controller_toggle[SELECT] = true;
            ctrl.controller_states[SELECT] = 1;
        };
    };
    if( abs(Ps3.event.analog_changed.stick.rx) + abs(Ps3.event.analog_changed.stick.ry) > 2 ){
       Serial.println(Ps3.data.analog.stick.rx);
       Serial.println(Ps3.data.analog.stick.rx);
    };

    

};