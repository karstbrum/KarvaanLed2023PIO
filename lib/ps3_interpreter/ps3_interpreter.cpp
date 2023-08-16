#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "ps3_interpreter.h"


controller_handler::controller_handler(){
};

void controller_handler::set_objects(Pixels* led_object_, Bulbgroups* bulb_object_){
    led_object = led_object_;
    bulb_object = bulb_object_;
};

// void controller_handler::setup_leds(uint8_t numSidesLED_, uint8_t pixelsPerSide_[], uint8_t numPins_, uint8_t sidesPerPin_[], 
// uint8_t LEDPin_[], uint16_t Ts_){
    
// };

// void controller_handler::setup_bulbs( uint8_t Bulbpins_[], uint8_t numSidesBulb_, uint8_t bulbsPerSide_[],uint16_t Ts_){
    
// };

void controller_handler::default_settings(uint8_t BPM, uint8_t mode, uint8_t color, float brightness, 
uint8_t min_BPM_, uint8_t max_BPM_, uint8_t max_mode_, uint8_t max_color_){
    controller_handler::states.BPM = BPM;
    controller_handler::states.mode = mode;
    controller_handler::states.color = color;
    controller_handler::states.brightness = brightness;

    min_BPM = min_BPM_;
    max_BPM = max_BPM_;
    max_mode = max_mode_;
    max_color = max_color_;

}

// parse the logic of the controller states
bool controller_handler::combination_parser(int input){

    // first check if any of the states are set to true
    // loop to all the states that are true and see if it is possible to set it
    bool set_state = true;

    // loop through number of inputs
    for(int k=0; k<INPUTS; k++){
        // if one of the state is toggled, check if combination is possible
        if(controller_handler::controller_toggle[k]){
            // both need to be true, otherwise input is not possible
            set_state = set_state && controller_handler::combination_array[k][input];
        };
    };

    return set_state;
    
};

// make the rumble 
void controller_handler::rumble(uint8_t times){
    for(int k=0; k<times; k++){
        Ps3.setRumble(100,0);
        delay(500);
        Ps3.setRumble(0,0);
    }
};

// function to check if controller should be activated
void controller_handler::activate_controller(){
    if (!controller_handler::controller_toggle[SELECT] && !controller_handler::controller_toggle[START]){
        controller_handler::use_controller = true;
    }
}

// loop through the function pointer matrix
// 
void controller_handler::function_mode_selector(){

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

// toggle through settings on callback
void controller_handler::function_setting_selector(){
    if (controller_handler::controller_toggle[START]){
        controller_handler::start();
    }
    if (controller_handler::controller_toggle[SELECT]){
        controller_handler::select();
    }
};

// make setting functions modes
void controller_handler::start(){
    // only toggle between true and false on use_controller
    use_controller = !use_controller;
};

void controller_handler::select(){
    // cycle through modes
    if (controller_handler::controller_toggle[CROSS]){
        controller_handler::states.mode = (controller_handler::states.mode>0) ? controller_handler::states.mode-1 : controller_handler::states.mode;
    }
    if (controller_handler::controller_toggle[TRIANGLE]){
        controller_handler::states.mode = (controller_handler::states.mode<max_mode) ? controller_handler::states.mode+1 : controller_handler::states.mode;
    }

    // set color, continuous loop
    if (controller_handler::controller_toggle[SQUARE]){
        controller_handler::states.color = (controller_handler::states.color>0) ? controller_handler::states.color-1 : max_color;
    }
    if (controller_handler::controller_toggle[CIRCLE]){
        controller_handler::states.color = (controller_handler::states.color<max_color) ? controller_handler::states.color+1 : 0;
    }

    // set brightness
    if (controller_handler::controller_toggle[D_LEFT]){
        controller_handler::states.brightness = (controller_handler::states.brightness>0) ? controller_handler::states.brightness-0.02 : controller_handler::states.brightness;
    }
    if (controller_handler::controller_toggle[D_RIGHT]){
        controller_handler::states.brightness = (controller_handler::states.brightness<1) ? controller_handler::states.brightness+0.02 : controller_handler::states.brightness;
    }

    // set BPM
    if (controller_handler::controller_toggle[D_DOWN]){
        controller_handler::states.BPM = (controller_handler::states.BPM>min_BPM) ? controller_handler::states.BPM-1 : controller_handler::states.BPM;
        // rumble when multiple of 10
        if (controller_handler::states.BPM%10 == 0){
            controller_handler::rumble(1);
        } 
    }
    if (controller_handler::controller_toggle[D_UP]){
        controller_handler::states.BPM = (controller_handler::states.BPM<max_BPM) ? controller_handler::states.BPM+1 : controller_handler::states.BPM;
        // rumble when multiple of 10
        if (controller_handler::states.BPM%10 == 0){
            controller_handler::rumble(1);
        } 
    }

};

// make all modes
void controller_handler::cross(){
    float freqdiv_led;
    float controller_value = static_cast<float>(controller_handler::controller_states[L_STICK_Y]);
    if(controller_handler::controller_states[L_STICK_Y]<0){
        freqdiv_led = 1 - abs(controller_value/150);
    } else if(controller_handler::controller_states[L_STICK_Y]>0) {
        freqdiv_led = 1 + abs(controller_value/25);
    } else {
        freqdiv_led = 1;
    }
    led_object->freqdiv = freqdiv_led;
    Serial.println(led_object->freqdiv);
    uint8_t clusters[] = {4, 5, 4, 6};
    uint8_t numClusters = sizeof(clusters);
    led_object->travelSides(states.color, 1, 0, 1, numClusters, clusters, 0, 1);
    bulb_object->flashingBulbs(1, 1, 1);
};
void controller_handler::square(){
    
}; 
void controller_handler::triangle(){
    
};
void controller_handler::circle(){
    
};
void controller_handler::l_trigger(){
    
};
void controller_handler::r_trigger(){
    
};
void controller_handler::l_bumper(){
    
};
void controller_handler::r_bumper(){
    
};
void controller_handler::sticks(){
    
};
void controller_handler::l_stick_press(){
    
};
void controller_handler::r_stick_press(){
    
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
        ctrl.function_setting_selector();
        ctrl.activate_controller();
        ctrl.controller_use_time = millis();
    };
    if( Ps3.event.button_up.cross ){
        ctrl.controller_toggle[CROSS] = false;
        ctrl.controller_states[CROSS] = 0;
        ctrl.controller_use_time = millis();
    };
    if( Ps3.event.button_down.square ){
        if(ctrl.combination_parser(SQUARE)){
            ctrl.controller_toggle[SQUARE] = true;
            ctrl.controller_states[SQUARE] = 1;
        };
        ctrl.function_setting_selector();
        ctrl.activate_controller();
        ctrl.controller_use_time = millis();
    };
    if( Ps3.event.button_up.square ){    
        ctrl.controller_toggle[SQUARE] = false;
        ctrl.controller_states[SQUARE] = 0;
        ctrl.controller_use_time = millis();
    };
    if( Ps3.event.button_down.triangle ){
        if(ctrl.combination_parser(TRIANGLE)){
            ctrl.controller_toggle[TRIANGLE] = true;
            ctrl.controller_states[TRIANGLE] = 1;
        };
        ctrl.function_setting_selector();
        ctrl.activate_controller();
        ctrl.controller_use_time = millis();
    };  
    if( Ps3.event.button_up.triangle ){
        ctrl.controller_toggle[TRIANGLE] = false;
        ctrl.controller_states[TRIANGLE] = 0;
        ctrl.controller_use_time = millis();
    };
    if( Ps3.event.button_down.circle ){
        if(ctrl.combination_parser(CIRCLE)){
            ctrl.controller_toggle[CIRCLE] = true;
            ctrl.controller_states[CIRCLE] = 1;
        };
        ctrl.function_setting_selector();
        ctrl.activate_controller();
        ctrl.controller_use_time = millis();
    };
    if( Ps3.event.button_up.circle ){
        ctrl.controller_toggle[CIRCLE] = false;
        ctrl.controller_states[CIRCLE] = 0;
        ctrl.controller_use_time = millis();
    };

    // d pad
    if( Ps3.event.button_down.up){
        if(ctrl.combination_parser(D_UP)){
            ctrl.controller_toggle[D_UP] = true;
            ctrl.controller_states[D_UP] = 1;
        };
        ctrl.function_setting_selector();
        ctrl.controller_use_time = millis();
    };
    if( Ps3.event.button_up.up){
        ctrl.controller_toggle[D_UP] = false;
        ctrl.controller_states[D_UP] = 0;
        ctrl.controller_use_time = millis();
    };
    if( Ps3.event.button_down.right){
        if(ctrl.combination_parser(D_RIGHT)){
            ctrl.controller_toggle[D_RIGHT] = true;
            ctrl.controller_states[D_RIGHT] = 1;
        };
        ctrl.function_setting_selector();
        ctrl.controller_use_time = millis();
    };
    if( Ps3.event.button_up.right){
        ctrl.controller_toggle[D_RIGHT] = false;
        ctrl.controller_states[D_RIGHT] = 0;
        ctrl.controller_use_time = millis();
    };
    if( Ps3.event.button_down.down){
        if(ctrl.combination_parser(D_DOWN)){
            ctrl.controller_toggle[D_DOWN] = true;
            ctrl.controller_states[D_DOWN] = 1;
        };
        ctrl.function_setting_selector();
        ctrl.controller_use_time = millis();
    };
    if( Ps3.event.button_up.down){
        ctrl.controller_toggle[D_DOWN] = false;
        ctrl.controller_states[D_DOWN] = 0;
        ctrl.controller_use_time = millis();
    };
    if( Ps3.event.button_down.left){
        if(ctrl.combination_parser(D_LEFT)){
            ctrl.controller_toggle[D_LEFT] = true;
            ctrl.controller_states[D_LEFT] = 1;
        };
        ctrl.function_setting_selector();
        ctrl.controller_use_time = millis();
    };
    if( Ps3.event.button_up.left){
        ctrl.controller_toggle[D_LEFT] = false;
        ctrl.controller_states[D_LEFT] = 0;
        ctrl.controller_use_time = millis();
    };
    
    // start and select
    if (Ps3.event.button_down.start){
        if(ctrl.combination_parser(START)){
            ctrl.controller_toggle[START] = true;
            ctrl.controller_states[START] = 1;
        };
        ctrl.function_setting_selector();
        ctrl.controller_use_time = millis();
    };
    if (Ps3.event.button_up.start){
        ctrl.controller_toggle[START] = false;
        ctrl.controller_states[START] = 0;
        ctrl.controller_use_time = millis();
    };
    if (Ps3.event.button_down.select){
        if(ctrl.combination_parser(SELECT)){
            ctrl.controller_toggle[SELECT] = true;
            ctrl.controller_states[SELECT] = 1;
        };
        ctrl.function_setting_selector();
        ctrl.controller_use_time = millis();
    };
    if (Ps3.event.button_up.select){
        ctrl.controller_toggle[SELECT] = false;
        ctrl.controller_states[SELECT] = 0;
        ctrl.controller_use_time = millis();
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
        ctrl.activate_controller();
        ctrl.controller_use_time = millis();
    };
    if( abs(Ps3.event.analog_changed.button.r1) ){
        if(Ps3.data.analog.button.r1 >0 && ctrl.combination_parser(R_BUMPER)){
            ctrl.controller_toggle[R_BUMPER] = true;
            ctrl.controller_states[R_BUMPER] = 1;
        } else {
            ctrl.controller_toggle[R_BUMPER] = false;
            ctrl.controller_states[R_BUMPER] = 0;
        };
        ctrl.activate_controller();
        ctrl.controller_use_time = millis();
    };
    if( abs(Ps3.event.analog_changed.button.l2) ){
        if(Ps3.data.analog.button.l2 >0 && ctrl.combination_parser(L_TRIGGER)){
            ctrl.controller_toggle[L_TRIGGER] = true;
            ctrl.controller_states[L_TRIGGER] = Ps3.data.analog.button.l2;
        } else {
            ctrl.controller_toggle[L_TRIGGER] = false;
            ctrl.controller_states[L_TRIGGER] = 0;
        };
        ctrl.activate_controller();
        ctrl.controller_use_time = millis();
    };
    if( abs(Ps3.event.analog_changed.button.r2) ){
        if(Ps3.data.analog.button.r2 >0 && ctrl.combination_parser(R_TRIGGER)){
            ctrl.controller_toggle[R_TRIGGER] = true;
            ctrl.controller_states[R_TRIGGER] = Ps3.data.analog.button.r2;
        } else {
            ctrl.controller_toggle[R_TRIGGER] = false;
            ctrl.controller_states[R_TRIGGER] = 0;
        };
        ctrl.activate_controller();
        ctrl.controller_use_time = millis();
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
        ctrl.activate_controller();
        ctrl.controller_use_time = millis();
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
        ctrl.activate_controller();
        ctrl.controller_use_time = millis();
    };

    // stick press l3 and r3
    if (Ps3.event.button_down.l3){
        if(ctrl.combination_parser(L_STICK_PRESS)){
            ctrl.controller_toggle[L_STICK_PRESS] = true;
            ctrl.controller_states[L_STICK_PRESS] = 1;
        };
        ctrl.activate_controller();
        ctrl.controller_use_time = millis();
    };
    if (Ps3.event.button_up.l3){
        ctrl.controller_toggle[L_STICK_PRESS] = false;
        ctrl.controller_states[L_STICK_PRESS] = 0;
        ctrl.controller_use_time = millis();
    };
    if (Ps3.event.button_down.r3){
        if(ctrl.combination_parser(R_STICK_PRESS)){
            ctrl.controller_toggle[R_STICK_PRESS] = true;
            ctrl.controller_states[R_STICK_PRESS] = 1;
        };
        ctrl.activate_controller();
        ctrl.controller_use_time = millis();
    };
    if (Ps3.event.button_up.r3){
        ctrl.controller_toggle[R_STICK_PRESS] = false;
        ctrl.controller_states[R_STICK_PRESS] = 0;
        ctrl.controller_use_time = millis();
    };

};