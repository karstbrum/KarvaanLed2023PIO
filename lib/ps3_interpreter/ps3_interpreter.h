#ifndef PS3_INTERPRETER_H
#define PS3_INTERPRETER_H
 
#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <Ps3Controller.h>
#include "led_auto_mode.h"
#include "bulb_auto_mode.h"

#define MAXCONTROLLERS 4

// make definition of every button
// put into one big array
#define CROSS 0
#define SQUARE 1
#define TRIANGLE 2
#define CIRCLE 3
#define D_UP 4
#define D_RIGHT 5
#define D_DOWN 6
#define D_LEFT 7
#define START 8
#define SELECT 9
#define L_TRIGGER 10
#define R_TRIGGER 11
#define L_BUMPER 12
#define R_BUMPER 13
#define L_STICK_X 14
#define L_STICK_Y 15
#define R_STICK_X 16
#define R_STICK_Y 17
#define L_STICK_PRESS 18
#define R_STICK_PRESS 19

// num of inputs
#define INPUTS 20

// setup controller
void setup_controller(char* address);

// handle controller callbacks
void controller_callbacks();

// define what to do on connect
void on_connect();

// from here on put all in a class
class controller_handler {

    public:

        controller_handler();

        // check for possible combinations
        bool combination_parser(int input);

        // logic to select the correct function to determine mode, call in main
        void function_mode_selector();

        // logic to select the correct function to determine settings, on callback
        void function_setting_selector();

        // function to check if switcht is needed from manual to controller
        void activate_controller();

        // setup the lights led and bulbs
        void setup_leds(uint8_t numSidesLED_, uint8_t pixelsPerSide_[], uint8_t numPins_, uint8_t sidesPerPin_[], 
        uint8_t LEDPin_[], uint16_t Ts_);
        void setup_bulbs(uint8_t Bulbpins_[], uint8_t numSidesBulb_, uint8_t bulbsPerSide_[],uint16_t Ts_);

        // default settings
        void default_settings(uint8_t BPM = 120, uint8_t mode = 1, uint8_t color = 1, float brightness = 0.3, 
        uint8_t min_BPM_ = 10, uint8_t max_BPM_ = 200, uint8_t max_mode_ = 10, uint8_t max_color_ = 14);

        // set object of bulbs and leds
        void set_objects(Pixels* led_object_, Bulbgroups* bulb_object_);

        // statespace checker
        void statespace_checker(int mode_used, float fall_time, float rise_time);

        // define function per combination
        void cross(); // cross mode
        void square(); // square mode   
        void triangle(); // triangle mode
        void circle(); // circle mode
        void start(); // toggle between controller and auto
        void select(); // select with d-pad or buttons to configure 
        void l_trigger(); // left post
        void r_trigger(); // right post
        void l_bumper(); // left side of leds
        void r_bumper(); // right side of leds
        void sticks(); // control focus point horizontal or vertical or both
        void l_stick_press(); //pulse all leds at high frequency?
        void r_stick_press(); //all leds go from top to bottem or vice versa, random pattern
        // set controller rumble
        void rumble(uint8_t times);

        // states struct
        struct {
            uint8_t BPM;
            uint8_t mode;
            uint8_t color;
            float brightness;
            } states;
        
        // set min and max of settings
        uint8_t min_BPM;
        uint8_t max_BPM;
        uint8_t max_mode;
        uint8_t max_color;

        // declare pointer for led mode class
        Pixels* led_object;
        Bulbgroups* bulb_object;

        // checks for combinations and values
        int controller_states[INPUTS] = {};
        bool controller_toggle[INPUTS] = {};

        // make variable to determine mode switch between manual and auto mode
        bool use_controller = false;
        
        // last time controller is used
        int controller_use_time;

        // check if statespace is set for mode
        int check_statespace = -1;

        // define the possible combinations
        // input is row and checks if possible wiht column
        // include itself, the diagonal
        bool combination_array[INPUTS][INPUTS] = 
        {{1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0}, // cross
         {0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0}, // square
         {0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0}, // triangle
         {0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0}, // circle
         {0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // d up
         {0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // d right
         {0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // d down
         {0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // d left
         {1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // start
         {1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // select
         {1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0}, // l trigger
         {1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0}, // r trigger 
         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0}, // l bumper
         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0}, // r bumper 
         {1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0}, // l stick x
         {1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0}, // l stick y
         {1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0}, // r stick x
         {1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0}, // r stick y
         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0}, // l stick press
         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}};// r stick press

};

// declare object to use in main
extern controller_handler ctrl;


#endif