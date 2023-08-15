#include <Arduino.h>

// own libraries
#include "bulb_auto_mode.h"
#include "led_auto_mode.h"
#include "ps3_interpreter.h"

// Sampling time (Ts)
#define Ts 100
// number of ms before next loop iteration which allows 1ms task delay
#define delay_ms 5

// max numbers for settings
#define MAXCOLORS 15
#define MAXBRIGHT 100
#define MAXMODES 25
#define MAXBPM 300

// set up the different cores
TaskHandle_t ControllerTask;
TaskHandle_t LEDTask;

// SETUP PART FOR LED
// Pixels(numSides, pixelsPerSide, pixelsRing, LEDpin, Ts)
uint8_t LEDsPerSide[] = {36};
uint8_t numSides = sizeof(LEDsPerSide);
uint8_t sidesPerPin[] = {1};
uint8_t LEDPins[] = {13};
uint8_t numPins = sizeof(LEDPins);
Pixels LED(numSides, LEDsPerSide, numPins, sidesPerPin, LEDPins, Ts);

// SETUP PART FOR BULB
//bulbs(bulbpins, numCombinations, BulbsPerCombination, Ts);
uint8_t bulbpins[] = {21, 22, 33, 25, 12, 
                      15, 2, 18, 19, 32};
uint8_t numCombinations = 2;
uint8_t BulbsPerCombination[] = {5, 5};
Bulbgroups Bulb(bulbpins, numCombinations, BulbsPerCombination, Ts);

// TIME VARIABLES
// Time spent in the main loop
int loopTime = 0;

// select auto mode or controller
bool auto_mode = false;

// time to automatically switch to auto mode in milliseconds
int auto_switch_time = 60000;

// define states 
struct {
  uint8_t BPM = 120;
  uint8_t mode = 0;
  uint8_t color = 0;
  float brightness = 0.5;
  } states;

void start_controller(){ 

  char address[] = "aa:bb:cc:dd:ee:ff";
  setup_controller(address);

  ctrl.default_settings(states.BPM, states.mode, states.color, states.brightness);

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
    LED.setDimmer(states.BPM);
    Bulb.setDimmer(states.BPM);
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
    case 1: {// set all to static
      LED.setColor(states.color);
      Bulb.staticValue();
      break; }
  }

}


// Task for handling the LEDs on core 1
void LightsTaskcode( void * pvParameters ){
  // another option to have a timed loop is to use vTaskDelayUntil(), have to look into it first
  for(;;){
    if(millis()-loopTime >= Ts){

      loopTime = millis();

      // set the lights, do first to make sampling as equidistant as possible
      if(!ctrl.use_controller){
        auto_functions();
      } else {
        ctrl.function_mode_selector();
      };

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

  // setup the ps3 controller
  
  
}

void loop() {
}