#include <Arduino.h>

// own libraries
#include "bulb_auto_mode.h"
#include "led_auto_mode.h"
#include "ps3_interpreter.h"

// Sampling time (Ts)
#define Ts 25
// number of ms before next loop iteration which allows 1ms task delay
#define delay_ms 5

// set up the different cores
TaskHandle_t ControllerTask;
TaskHandle_t LEDTask;

// TIME VARIABLES
// Time spent in the main loop
int loopTime = 0;

void start_controller(){ 

  char address[] = "aa:bb:cc:dd:ee:ff";
  setup_controller(address);

}


// Task for handling the LEDs on core 1
void LEDTaskcode( void * pvParameters ){
  // another option to have a timed loop is to use vTaskDelayUntil(), have to look into it first
  for(;;){
    if(millis()-loopTime >= Ts){

      loopTime = millis();
      // get states from controller if active

      // else get states from auto mode

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
                    LEDTaskcode, /* Task function. */
                    "LEDTask",   /* name of task. */
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