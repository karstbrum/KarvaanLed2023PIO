#include <Arduino.h>

// own libraries
#include "bulb_auto_mode.h"
#include "led_auto_mode.h"
#include "ps3_interpreter.h"

// Sampling time (Ts)
#define Ts 25
// number of ms before next loop iteration which allows 1ms task delay
#define delay_ms 5

// max numbers for settings
#define MAXCOLORS 13
#define MAXMODES 25 // to be defined
#define MINBPM 101
#define MAXBPM 199

// set up the different cores
TaskHandle_t ControllerTask;
TaskHandle_t LEDTask;

// SETUP PART FOR LED
// Pixels(numSides, pixelsPerSide, pixelsRing, LEDpin, Ts)
uint8_t LEDsPerSide[] = {12, 13, 17, 17, 12, 12, 13, 13, 17, 
                         12, 13, 13, 12, 12, 13, 13, 13, 13, 12};
uint8_t numSides = sizeof(LEDsPerSide);
uint8_t sidesPerPin[] = {9, 10};
uint8_t LEDPins[] = {13, 14};
uint8_t numPins = sizeof(LEDPins);

// uint8_t LEDsPerSide[] = {36, 36};
// uint8_t numSides = sizeof(LEDsPerSide);
// uint8_t sidesPerPin[] = {2};
// uint8_t LEDPins[] = {13};
// uint8_t numPins = sizeof(LEDPins);
Pixels LED(numSides, LEDsPerSide, numPins, sidesPerPin, LEDPins, Ts);

// SETUP PART FOR BULB
//bulbs(bulbpins, numCombinations, BulbsPerCombination, Ts);
uint8_t bulbpins[] = {21, 22, 33, 25, 12, 
                      15, 2, 18, 19, 32};
uint8_t numCombinations = 2;
uint8_t BulbsPerCombination[] = {5, 5};
Bulbgroups Bulb(bulbpins, numCombinations, BulbsPerCombination, Ts);

// pointers to objects
Pixels* LED_pointer = &LED;
Bulbgroups* Bulb_pointer = &Bulb;

// TIME VARIABLES
// Time spent in the main loop
int loopTime = 0;

// time to automatically switch to auto mode in milliseconds
int auto_switch_time = 60000;

// set statespace checker
int check_statespace = -1;

// check if switched to controller
bool controller_switch = true;

// define states 
struct {
  uint8_t BPM = 120;
  uint8_t mode = 0;
  uint8_t travelMode = 0;
  uint8_t color = 0;
  float brightness = 1;
  } states;

// set the initial states in the objects
void set_initial_states(){
  LED.setBPM(states.BPM);
  Bulb.setBPM(states.BPM);
  LED.setColor(states.color);
  LED.setDimmer(states.brightness);
  Bulb.setDimmer(states.brightness);
  LED.setTravelMode(states.BPM);
}

void start_controller(){ 

  // search for controller with this address
  char address[] = "aa:bb:cc:dd:ee:ff";
  setup_controller(address);

  // set default states, sync with controller
  ctrl.default_settings(states.BPM, states.mode, states.color, states.brightness, 
  MINBPM, MAXBPM, MAXMODES, MAXCOLORS);

  // set the controller objects for controlling lights
  ctrl.set_objects(LED_pointer, Bulb_pointer);

}

void sync_states(){
  if (states.BPM != ctrl.states.BPM){
    states.BPM = ctrl.states.BPM;
    LED.setBPM(states.BPM);
    Bulb.setBPM(states.BPM);
  }
  if (states.mode != ctrl.states.mode){
    states.mode = ctrl.states.mode;
    // no need to switch mode, check in loop constantly
  }
  if (states.color != ctrl.states.color){
    states.color = ctrl.states.color;
    LED.setColor(states.color);
  }
  if (states.brightness != ctrl.states.brightness){
    states.brightness = ctrl.states.brightness;
    LED.setDimmer(states.brightness);
    Bulb.setDimmer(states.brightness);
  }
}

// check if state space needs to be changed
void statespace_checker(int mode_used, float fall_time, float rise_time){
  if(check_statespace != mode_used){
    check_statespace = mode_used;
    ctrl.check_statespace = -1;
    LED.set_statespace(fall_time, rise_time);
  }
}

// check if switched to controller
void to_controller(){
  if(controller_switch){
    // color 14 is off
    LED.setColor(14);
    // staticvalue 0 is off
    Bulb.staticValue(0);
  }
}

// define all auto functions
void auto_functions(){

  // define mode used, to switch also in random mode
  static uint8_t mode_used;
  
  // define mode that is used
  mode_used = states.mode;

  // switch between modes
  switch (mode_used){
    case 0: {// set all to static
      statespace_checker(0, 0, 0);
      LED.setColor(states.color);
      Bulb.staticValue();
      break; }
    case 1: {// pulse with same color with fade
      statespace_checker(0, 0, 0);
      LED.freqdiv = 2;
      Bulb.freqdiv = 2;
      LED.pulseSameColor(states.color,1);
      Bulb.pulse(1);
      break;}
    case 2: {// pulse to other coler with fade
      statespace_checker(0, 0, 0);
      LED.freqdiv = 2;
      Bulb.freqdiv = 2;
      LED.pulseToOtherColor(1,1);
      Bulb.pulse(1);
      break;}
    case 3: {// travel up and down wiht band of 100% pixels, 3 bulbs with fading brightness
      statespace_checker(0, 0, 0);
      uint8_t clusters[] = {2, 2, 2, 3, 2, 2, 2, 2, 2};
      int Direction[] = {-1, 1, -1, 1, 1, -1, -1, 1, -1};
      uint8_t numClusters = sizeof(clusters);
      LED.upDown(0.3, states.color, 1, 0, numClusters, clusters, 1, Direction);
      Bulb.upDown(1, 1, 1);
      break;}   
  }

}

// Task for handling the LEDs on core 1
void LightsTaskcode( void * pvParameters ){

  // set initial states
  set_initial_states();

  // another option to have a timed loop is to use vTaskDelayUntil(), have to look into it first
  for(;;){
    if(millis()-loopTime >= Ts){

      loopTime = millis();

      // set the lights, do first to make sampling as equidistant as possible
      if(!ctrl.use_controller){
        // call auto mode
        auto_functions();
      } else {
        // reset statespace for auto mode
        statespace_checker(-1, 0, 0);
        // call control mode
        ctrl.function_mode_selector();
      };

      // states of leds are determined, now write to leds
      LED.activateColor();
      // states of bulbs are determined, now write to bulbs
      Bulb.setLevels();

      // sync modes BMP etc between controller and main and update if any changed
      sync_states();

      // check if switch to auto mode after idle time controller
      if (((millis()-ctrl.controller_use_time) > auto_switch_time) && ctrl.use_controller){
        ctrl.use_controller = false;
      }
   
    }

    // make space for idle task by 1ms delay, make sure esp does not crash
    // don't allow delay at the start of next iteration, delay_ms ms max
    if(millis()-loopTime < Ts-delay_ms){
      vTaskDelay(1);
    }
  }
}

// Task for handling the controller on core 0
void ControllerTaskcode( void * pvParameters ){ 
  start_controller();
  // start task loop to keep open
  for(;;){
    // make space for idle task by 1ms delay, make sure esp does not crash
    vTaskDelay(1);
  }
}


void setup() {

  Serial.begin(115200);

  //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
                    ControllerTaskcode, /* Task function. */
                    "ControllerTask",   /* name of task. */
                    20000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &ControllerTask,    /* Task handle to keep track of created task */
                    1);          /* pin task to core 0 */                  
  delay(500);

  //create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
                    LightsTaskcode, /* Task function. */
                    "LightsTask",   /* name of task. */
                    20000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &LEDTask,     /* Task handle to keep track of created task */
                    0);          /* pin task to core 1 */
  delay(500); 

  
}

void loop() {
}