#include <Arduino.h>

// own libraries
#include "bulb_auto_mode.h"
#include "led_auto_mode.h"
#include "ps3_interpreter.h"

// Sampling time (Ts)
#define Ts 40

// max numbers for settings
#define MAXCOLORS 10
#define MAXMODES 16 // to be defined
#define MINBPM 101
#define MAXBPM 199

// set up the different cores
TaskHandle_t ControllerTask;
TaskHandle_t LEDTask;

uint16_t LEDsPerSide[] = {18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
                         12, 13, 17, 17, 12, 12, 13, 13, 17,
                         12, 13, 13, 12, 12, 13, 13, 13, 13, 12};
uint8_t numSides = sizeof(LEDsPerSide)/sizeof(uint16_t);
uint8_t sidesPerPin[] = {22, 9, 10};
uint8_t LEDPins[] = {26, 13, 14};
uint8_t numPins = sizeof(LEDPins);
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
int auto_switch_time = 120000;

// random timer
int random_timer = 0; 
int random_time_switch = 30000;


// check if switched to controller
bool controller_switch = true;

// define states 
struct {
  uint8_t BPM = 120;
  uint8_t mode = 0;
  uint8_t color = 0;
  float brightness = 0.7;
  } states;

// set the initial states in the objects
void set_initial_states(){
  LED.setBPM(states.BPM);
  Bulb.setBPM(states.BPM);
  LED.setColor(states.color);
  LED.setDimmer(states.brightness);
  Bulb.setDimmer(states.brightness/4);
  // set default states, sync with controller
  ctrl.default_settings(states.BPM, states.mode, states.color, states.brightness, 
  MINBPM, MAXBPM, MAXMODES, MAXCOLORS);
  
}

void start_controller(){ 

  // search for controller with this address
  char address[] = "aa:bb:cc:dd:ee:ff";
  setup_controller(address);

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
    Bulb.setDimmer(states.brightness/4);
  }
}

// sync index of bulb & led by setting to 0
void sync_index(){
  LED.pulseIndex = 0;
  Bulb.pulseIndex = 0;
}

// define all auto functions
void auto_functions(){

  // define mode used, to switch also in random mode
  static uint8_t mode_used;
  static uint8_t prev_mode_used;
  
  // define mode that is used
  bool select_random = (states.mode == MAXMODES) ? true : false;
  if (select_random){
    if (millis() - random_timer > random_time_switch){
      mode_used = random(MAXMODES-1);
      random_timer = millis();
    }
  } else{
    mode_used = states.mode;
  }
  
  // sync if mode is different
  if (mode_used != prev_mode_used){
    sync_index();
  }

  // switch between modes
  switch (mode_used){
    case 0: {// set all to static
      LED.setColor(states.color);
      Bulb.staticValue();
      break; }
    case 1: {// pulse with same color with fade
      LED.freqdiv = 2;
      Bulb.freqdiv = 2;
      LED.pulseSameColor(states.color,1);
      Bulb.travelSides(1);
      break;}
    case 2: {// pulse to other coler with fade
      LED.freqdiv = 2;
      Bulb.freqdiv = 2;
      LED.pulseToOtherColor(1,1);
      Bulb.travelSides(1);
      break;}
    case 3: {// travel up and down wiht band of 20% pixels, 3 bulbs with fading brightness
      uint8_t clusters[] = {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2};
      int Direction[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, 1, -1, -1, 1, 1, -1, 1};
      uint8_t numClusters = sizeof(clusters);
      LED.freqdiv = 2;
      Bulb.freqdiv = 2;
      LED.upDown(0.2, states.color, 0, 0, numClusters, clusters, 1, Direction);
      Bulb.upDown(1, 1);
      break;}   
    case 4: {// travel up and down wiht band of 20% pixels, 3 bulbs with fading brightness
      uint8_t clusters[] = {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2};
      int Direction[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, 1, -1, -1, 1, 1, -1, 1};
      uint8_t numClusters = sizeof(clusters);
      int bulbDirection[] = {-1, -1};
      LED.freqdiv = 2;
      Bulb.freqdiv = 2;
      LED.upDown(0.2, states.color, 0, 0, numClusters, clusters, 1, Direction);
      Bulb.upDown(1, 1, 0, 0, true, bulbDirection);
      break;}   
    case 5 : { // single random letter or 2 or 3 poles flashing in random color, single random flashing bulb per pole
      uint8_t clusters[] = {4, 4, 6, 4, 4, 4, 5, 4, 6};
      uint8_t numClusters = sizeof(clusters);
      LED.freqdiv = 2;
      Bulb.freqdiv = 2;
      LED.travelSides(states.color, 1, 0, 1, numClusters, clusters, 0, 1);
      Bulb.flashingBulbs(1, 1, 1);
      break;}  
    case 6 : {// leds fill up in wave, poles with phase updown
      LED.freqdiv = 2;
      Bulb.freqdiv = 2;
      uint8_t clusters[] = {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2};
      int Direction[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, 1, -1, -1, 1, 1, -1, 1};
      uint8_t numClusters = sizeof(clusters);
      float phase_led[numClusters];
      for (uint8_t k=0; k<numClusters; k++){
        phase_led[k] = static_cast<float>(k)/static_cast<float>(numClusters);
      }
      float phase_bulb[] = {0, 0.5};
      LED.fillUp(states.color, 0, 0, numClusters, clusters, 1, Direction, 1, phase_led);
      Bulb.upDown(2, 1, 0, 0, 0, Direction, 1, phase_bulb);
      break;} 
    case 7 : {// leds fill up in wave, poles with phase updown
      LED.freqdiv = 2;
      Bulb.freqdiv = 2;
      uint8_t clusters[] = {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2};
      int Direction[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, 1, -1, -1, 1, 1, -1, 1};
      uint8_t numClusters = sizeof(clusters);
      float phase_led[numClusters];
      for (uint8_t k=0; k<numClusters; k++){
        phase_led[k] = -static_cast<float>(k)/static_cast<float>(numClusters);
      }
      float phase_bulb[] = {0, 0.5};
      LED.fillUp(states.color, 0, 0, numClusters, clusters, 1, Direction, 1, phase_led);
      Bulb.upDown(2, 1, 0, 0, 0, Direction, 1, phase_bulb);
      break;} 
    case 8: {// travel up and down wiht band of 20% pixels inverse, 3 bulbs with fading brightness
      uint8_t clusters[] = {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2};
      int Direction[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, 1, -1, -1, 1, 1, -1, 1};
      uint8_t numClusters = sizeof(clusters);
      int bulbDirection[] = {-1, -1};
      LED.freqdiv = 2;
      Bulb.freqdiv = 2;
      LED.upDown(0.2, states.color, 0, 0, numClusters, clusters, 1, Direction, true);
      Bulb.upDown(1, 1, 0, 0, true, bulbDirection);
      break;} 
    case 9 : { // 50% chance flashing pixels, 30% flashing chance for all bulbs
      uint8_t color2 = (states.color == 0) ? color2 = 1 : states.color;
      LED.flashingPixels(states.color, color2, 30);
      Bulb.flashingBulbs(1, 0, 0, 50);
      break;}  
    case 10 : { // wave of 1 letter with positive direction, single random flashing bulb per pole
      LED.freqdiv = 2;
      Bulb.freqdiv = 2;
      LED.travelingWave(states.color, 1, 1, 0.5);
      Bulb.flashingBulbs(1, 1, 1);
      break;}
    case 11 : { // wave of 1 letter with negative direction, single random flashing bulb per pole
      LED.freqdiv = 2;
      Bulb.freqdiv = 2;
      LED.travelingWave(states.color, 1, -1, 0.5);
      Bulb.flashingBulbs(1, 1, 1);
      break;}
    case 12: {// random 50% pole fillboth, single flashing bulb
      uint8_t clusters[] = {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2};
      uint8_t numClusters = sizeof(clusters);
      LED.freqdiv = 2;
      Bulb.freqdiv = 2;
      LED.fillBoth(states.color, 0.5, 0.8, 70, false, numClusters, clusters, false);
      Bulb.flashingBulbs(1, 1, 1);
      break;}   
    case 13: {// random 50% pole fillboth, random position, single flashing bulb
      uint8_t clusters[] = {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2};
      uint8_t numClusters = sizeof(clusters);
      LED.freqdiv = 2;
      Bulb.freqdiv = 2;
      LED.fillBoth(states.color, 0.5, 0.8, 50, true, numClusters, clusters, false);
      Bulb.flashingBulbs(1, 1, 1);
      break;}   
    // negative and positive fillup
    case 14: {// random 50% pole fillboth, single flashing bulb
      uint8_t clusters[] = {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2};
      uint8_t numClusters = sizeof(clusters);
      int Direction[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, 1, -1, -1, 1, 1, -1, 1};
      LED.freqdiv = 2;
      Bulb.freqdiv = 2;
      LED.fillUp(states.color, 0, 0, numClusters, clusters, true, Direction, false, {}, false);
      Bulb.flashingBulbs(1, 1, 1);
      break;}   
    case 15: {// random 50% pole fillboth, random position, single flashing bulb
      uint8_t clusters[] = {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2};
      uint8_t numClusters = sizeof(clusters);
      int Direction[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, 1, -1, -1, 1, 1, -1, 1};
      LED.freqdiv = 2;
      Bulb.freqdiv = 2;
      LED.fillUp(states.color, 0, 0, numClusters, clusters, true, Direction, false, {}, true);
      Bulb.flashingBulbs(1, 1, 1);
      break;}   
  }

  prev_mode_used = mode_used;

}

// Task for handling the LEDs on core 1
void LightsTaskcode( void * pvParameters ){

  // set initial states
  set_initial_states();

  static bool use_controller;
  static bool prev_use_controller;

  // another option to have a timed loop is to use vTaskDelayUntil(), have to look into it first
  for(;;){
    if(millis()-loopTime >= Ts){

      loopTime = millis();

      // sync index if switched from controller 
      if (use_controller != prev_use_controller){
        sync_index();
      }

      prev_use_controller = use_controller;

      // set the lights, do first to make sampling as equidistant as possible
      if(!ctrl.use_controller){
        // call auto mode
        auto_functions();
        use_controller = true;
      } else {
        // call control mode
        ctrl.function_mode_selector();
        use_controller = false;
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

      // allow some delay for idle task
      vTaskDelay(5);

    }

  }
}

// Task for handling the controller on core 0
void ControllerTaskcode( void * pvParameters ){ 
  start_controller();
  // start task loop to keep open
  for(;;){
    // make space for idle task by 10ms delay, make sure esp does not crash
    vTaskDelay(10);
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
                    0);          /* pin task to core 0 */                  
  delay(500);

  //create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
                    LightsTaskcode, /* Task function. */
                    "LightsTask",   /* name of task. */
                    20000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &LEDTask,     /* Task handle to keep track of created task */
                    1);          /* pin task to core 1 */
  delay(500); 

  
}

void loop() {
}