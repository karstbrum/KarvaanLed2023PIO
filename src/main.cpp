#include <Arduino.h>

// own libraries
#include "bulb_auto_mode.h"
#include "led_auto_mode.h"
#include "ps3_interpreter.h"


// ps3 controller library
#include <Ps3Controller.h>

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

// controller battery
int battery = 0;

// setup controller callbacks
void controller_callbacks()
{
    //--- Digital cross/square/triangle/circle button events ---
    if( Ps3.event.button_down.cross )
        Serial.println("Started pressing the cross button");
    if( Ps3.event.button_up.cross )
        Serial.println("Released the cross button");

    if( Ps3.event.button_down.square )
        Serial.println("Started pressing the square button");
    if( Ps3.event.button_up.square )
        Serial.println("Released the square button");

    if( Ps3.event.button_down.triangle )
        Serial.println("Started pressing the triangle button");
    if( Ps3.event.button_up.triangle )
        Serial.println("Released the triangle button");

    if( Ps3.event.button_down.circle )
        Serial.println("Started pressing the circle button");
    if( Ps3.event.button_up.circle )
        Serial.println("Released the circle button");


    // sticks
    if( abs(Ps3.event.analog_changed.stick.lx) + abs(Ps3.event.analog_changed.stick.ly) > 2 ){
       Serial.print("Moved the left stick:");
       Serial.print(" x="); Serial.print(Ps3.data.analog.stick.lx, DEC);
       Serial.print(" y="); Serial.print(Ps3.data.analog.stick.ly, DEC);
       Serial.println();
    }

    if( abs(Ps3.event.analog_changed.stick.rx) + abs(Ps3.event.analog_changed.stick.ry) > 2 ){
       Serial.print("Moved the right stick:");
       Serial.print(" x="); Serial.print(Ps3.data.analog.stick.rx, DEC);
       Serial.print(" y="); Serial.print(Ps3.data.analog.stick.ry, DEC);
       Serial.println();
    }

    // triggers
    if( abs(Ps3.event.analog_changed.button.l1)){
       Serial.print("Pressing the left shoulder button: ");
       Serial.println(Ps3.data.analog.button.l1, DEC);
    }

    if( abs(Ps3.event.analog_changed.button.r1) ){
        Serial.print("Pressing the right shoulder button: ");
        Serial.println(Ps3.data.analog.button.r1, DEC);
    }

    if( abs(Ps3.event.analog_changed.button.l2) ){
        Serial.print("Pressing the left trigger button: ");
        Serial.println(Ps3.data.analog.button.l2, DEC);
    }

    if( abs(Ps3.event.analog_changed.button.r2) ){
        Serial.print("Pressing the right trigger button: ");
        Serial.println(Ps3.data.analog.button.r2, DEC);
    }



}

void on_connect(){
    Serial.println("Connected.");
}

void setup_controller(){
    Ps3.attach(controller_callbacks);
    Ps3.attachOnConnect(on_connect);
    Ps3.begin("aa:bb:cc:dd:ee:ff");
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
  setup_controller();
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