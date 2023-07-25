#include <Arduino.h>

// include own libraries
#include <led_auto_mode.h>
#include <bulb_auto_mode.h>

// max numbers for settings
#define MAXCOLORS 15
#define MAXBRIGHT 100
#define MAXMODES 25
#define MAXBPM 300

// Sampling time (Ts)
#define Ts 25

// set up the different cores
TaskHandle_t SMSTask;
TaskHandle_t LEDTask;

// TIME VARIABLES
// Time spent in the main loop
int loopTime = 0;
// timer for random modes
int randomTimer = 0;
int modeTimer = 0;
int colorTimer = 0;

// SETUP PART FOR LED
// Pixels(numSides, pixelsPerSide, pixelsRing, LEDpin, Ts)
uint8_t LEDsPerSide[] = {12, 13, 17, 17, 12, 12, 13, 13, 17, 
                         12, 13, 13, 12, 12, 13, 13, 13, 13, 12};
uint8_t numSides = sizeof(LEDsPerSide);
uint8_t sidesPerPin[] = {9, 10};
uint8_t LEDPins[] = {13, 14};
uint8_t numPins = sizeof(LEDPins);
Pixels KRVN(numSides, LEDsPerSide, numPins, sidesPerPin, LEDPins, Ts);

// SETUP PART FOR BULB
//bulbs(bulbpins, numCombinations, BulbsPerCombination, Ts);
uint8_t bulbpins[] = {21, 22, 33, 25, 12, 
                      15, 2, 18, 19, 32};
uint8_t numCombinations = 2;
uint8_t BulbsPerCombination[] = {5, 5};
Bulbgroups Poles(bulbpins, numCombinations, BulbsPerCombination, Ts);

// structure of all states for the LEDs and set initial states
typedef struct stateData {
  int Mode = 1;
  int BPM = 100;
  int color = 3;
  int Bright1 = 70;
  int Bright2 = 30;
} stateData;
stateData stageData;

void setStates(){
//   // use some logic to set the different states

//   stageData.Mode = ....

//   float dimValue1 = stageData.Bright1;
//   KRVN.setDimmer(dimValue1/100);
//   KRVN.setBPM(stageData.BPM);

//   float dimValue2 = stageData.Bright2;
//   Poles.setDimmer(dimValue2/100);
//   Poles.setBPM(stageData.BPM);
}

void setMode(uint16_t cmode){
  static int color1;
  static int color2;
  static int modeUsed;
  // direction array, could be used for multiple cases
  static int randDirection[10] = {};
  // phase array, could be used for multiple cases
  static float randPhase[10] = {};
  // boolean array, could be used for multiple cases
  static bool randBool[10] = {};
  // set random inverse 
  static bool randInverse = 1;
  // both KRVN and pole random
  static int poleMode = 1;
  static int KRVNMode = 1;

  // switch wave direction
  static int switchDirection[] = {1, -1, 1, -1, -1, 1, 1, -1, 1};
  static float switchPhase[] = {0, 0.111, 0.222, 0.333, 0.444, 0.555, 0.666, 0.777, 0.888};

  if (cmode==25){
    if (modeTimer == 0){
      modeUsed = 2 + random(23);
    }
    modeTimer += Ts; 
    // switch every X seconds
    if (modeTimer >= 45000){
      modeTimer = 0;
    } 
  } else {
    modeUsed = cmode;
  }

  // set direction of 6 and 7 every 10 seconds
  if (modeUsed==6 || modeUsed==7 || modeUsed==16 || modeUsed==17 || modeUsed==18 || modeUsed==22){
    
    if (randomTimer == 0){
      for (int k=0; k<2; k++){
        randDirection[k] = random(2)*2-1;
      }
    }
    
    randomTimer += Ts; 
    
    // switch every X seconds
    if (randomTimer >= 10000){
      randomTimer = 0;
    }
  } 
  // switch bewteen normal and inverse every 15 seconds 
  else if (modeUsed==10 || modeUsed==11 ||modeUsed==12 || modeUsed==13 || modeUsed==14 || modeUsed==15){
    
    
    if (randomTimer == 0){

      if (random(2)){
        for (int k=0; k<10; k++){
          switchPhase[k] = -switchPhase[k];
          switchDirection[k] = -switchDirection[k];
        }
      }
      
      if (randInverse){
        randInverse = 0;
      } else {
        randInverse = 1;
      }
    }
  
    randomTimer += Ts; 

    // switch every X seconds
    if (randomTimer >= 15000){
      randomTimer = 0;
    }
  } else if (modeUsed==20 || modeUsed==21){
    if (stageData.color == 1 || stageData.color == 2){
      color2 == 2;
    } else{
      color2 == 0;
    }
  } else if (modeUsed == 23){
    if (randomTimer == 0){
      for (int k=0; k<9; k++){
        int randNum = random(101);
        randPhase[k] = (float)randNum/100;
        randDirection[k] = random(2)*2-1;
      }
    }
  
    randomTimer += Ts; 

    // switch every X seconds
    if (randomTimer >= 15000){
      randomTimer = 0;
    }
  }

  if (stageData.color == 15) {
    if (colorTimer == 0){
      color1 = random(15);
    }
    colorTimer += Ts; 
    // switch every X seconds
    if (colorTimer >= 15000){
      colorTimer = 0;
    }
  } else {
    color1 = stageData.color - 1;
  }
  

  switch (modeUsed){
    case 1: {// set all to static
      KRVN.setColor(color1);
      Poles.staticValue();
      break; }
    case 2: {// pulse with same color with fade
      KRVN.freqdiv = 2;
      Poles.freqdiv = 2;
      KRVN.pulseSameColor(color1,1);
      Poles.pulse(1);
      break;}
    case 3: {// pulse to same coler with face, poles half speed
      KRVN.freqdiv = 2;
      Poles.freqdiv = 4;
      KRVN.pulseSameColor(color1,1);
      Poles.pulse(1);
      break;}
    case 4: {// pulse to other coler with fade
      KRVN.freqdiv = 2;
      Poles.freqdiv = 2;
      KRVN.pulseToOtherColor(1,1);
      Poles.pulse(1);
      break;}
    case 5: {// pulse to other coler with fade, poles half speed
      KRVN.freqdiv = 2;
      Poles.freqdiv = 4;
      KRVN.pulseToOtherColor(1,1);
      Poles.pulse(1);
      break;}
    case 6:{// traveling waves in both KRVN and poles, one in total per KRVN and one per pole, random direction
      KRVN.freqdiv = 4;
      Poles.freqdiv = 2;
      KRVN.travelingWave(color1, 1, randDirection[0], 0.5);
      Poles.travelingWave(2, randDirection[1], 0.5);
      break;}
    case 7:{// traveling waves in both KRVN and poles, one per letter and 1 per pole, random direction
      KRVN.freqdiv = 2;
      Poles.freqdiv = 2;
      KRVN.travelingWave(color1, 4, randDirection[0], 0.5);
      Poles.travelingWave(2, randDirection[1], 0.5);
      break;}
    case 8: {// travel up and down wiht band of 20% pixels, 3 bulbs with fading brightness
      uint8_t clusters[] = {2, 2, 2, 3, 2, 2, 2, 2, 2};
      int Direction[] = {1, -1, 1, -1, -1, 1, 1, -1, 1};
      uint8_t numClusters = sizeof(clusters);
      KRVN.upDown(0.1, color1, 1, 0, numClusters, clusters, 1, Direction);
      Poles.upDown(1, 1, 1);
      break;}
    case 9: {// travel up and down wiht band of 100% pixels, 3 bulbs with fading brightness
      uint8_t clusters[] = {2, 2, 2, 3, 2, 2, 2, 2, 2};
      int Direction[] = {-1, 1, -1, 1, 1, -1, -1, 1, -1};
      uint8_t numClusters = sizeof(clusters);
      KRVN.upDown(0.3, color1, 1, 0, numClusters, clusters, 1, Direction);
      Poles.upDown(1, 1, 1);
      break;}   
    case 10: {// fill KRVN up from the bottom, traveling wave downwards per pole with offset at half speed
      Poles.freqdiv = 2;
      uint8_t clusters[] = {2, 2, 2, 3, 2, 2, 2, 2, 2};
      uint8_t numClusters = sizeof(clusters);
      KRVN.fillUp(color1, 0, 0, numClusters, clusters, 1, switchDirection);
      Poles.travelingWave(2, -1, 0.5);
      break;}  
    case 11: {// fill KRVN up from the bottom offset letters, single traveling wave trough boht poles downwards
      Poles.freqdiv = 2;
      uint8_t clusters[] = {2, 2, 2, 3, 2, 2, 2, 2, 2};
      uint8_t numClusters = sizeof(clusters);
      float phase[] = {0, 0, 0.5, 0.5, 0, 0, 0.5, 0.5, 0.5};
      KRVN.fillUp(color1, 0, 0, numClusters, clusters, 1, switchDirection, 1, phase);
      Poles.travelingWave(1, -1, 0.5);
      break;}
    case 12: {// sine wave trough the letters with small band by setting phase to 1/9, Poles fast down out of phase
      Poles.freqdiv = 2;
      uint8_t clusters[] = {2, 2, 2, 3, 2, 2, 2, 2, 2};
      int Direction[] = {1, -1, 1, -1, -1, 1, 1, -1, 1};
      uint8_t numClusters = sizeof(clusters);
      float phase[] = {0, 0.111, 0.222, 0.333, 0.444, 0.555, 0.666, 0.777, 0.888};
      KRVN.upDown(0.1, color1, 0, 0, numClusters, clusters, 1, switchDirection, 0, 1, phase, switchDirection[9]);
      int Direction2[] = {switchDirection[1], switchDirection[1]};
      float phase2[] = {0, 0.5};
      Poles.upDown(1, 1, 0, 0, 1, Direction2, 1, phase2);
      break;}  
    case 13: {// sine wave trough the letters with fillup by setting phase to 1/9, Poles fast up down out of phase
      Poles.freqdiv = 2;
      uint8_t clusters[] = {2, 2, 2, 3, 2, 2, 2, 2, 2};
      int Direction[] = {1, -1, 1, -1, -1, 1, 1, -1, 1};
      uint8_t numClusters = sizeof(clusters);
      float phase[] = {0, 0.111, 0.222, 0.333, 0.444, 0.555, 0.666, 0.777, 0.888};
      KRVN.fillUp(color1, 0, 0, numClusters, clusters, 1, switchDirection, 1, phase, switchDirection[9]);
      int Direction2[] = {switchDirection[1], switchDirection[1]};
      float phase2[] = {0, 0.5};
      Poles.upDown(1, 1, 0, 0, 1, Direction2, 1, phase2);
      break;}
    case 14: {// KRVN updown with small band switching to inverse every 15 seconds, poles updown in phase
      uint8_t clusters[] = {2, 2, 2, 3, 2, 2, 2, 2, 2};
      int Direction[] = {1, -1, 1, -1, -1, 1, 1, -1, 1};
      uint8_t numClusters = sizeof(clusters);
      float phase[] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
      KRVN.upDown(0.1, color1, 0, 0, numClusters, clusters, 1, Direction, randInverse, 1, phase, switchDirection[0]);
      int Direction2[] = {switchDirection[1], switchDirection[1]};
      float phase2[] = {0, 0};
      Poles.upDown(1, 1, 0, 0, 1, Direction2, 1, phase2);
      break;}  
    case 15: {// KRVN updown with small band in wave switching to inverse every 15 seconds, poles updown out of phase
      uint8_t clusters[] = {2, 2, 2, 3, 2, 2, 2, 2, 2};
      int Direction[] = {1, -1, 1, -1, -1, 1, 1, -1, 1};
      uint8_t numClusters = sizeof(clusters);
      float phase[] = {0, 0.111, 0.222, 0.333, 0.444, 0.555, 0.666, 0.777, 0.888};
      KRVN.upDown(0.12, color1, 0, 0, numClusters, clusters, 1, Direction, randInverse, 1, phase, switchDirection[0]);
      int Direction2[] = {switchDirection[1], switchDirection[1]};
      float phase2[] = {0, 0.5};
      Poles.upDown(1, 1, 0, 0, 1, Direction2, 1, phase2);
      break;}  
    case 16 : { // travel sides per letter, in sync with poles, all fade
      Poles.freqdiv = 2;
      KRVN.freqdiv = 2;
      uint8_t clusters[] = {4, 5, 4, 6};
      uint8_t numClusters = sizeof(clusters);
      KRVN.travelSides(color1, 1, 0, randDirection[0], numClusters, clusters, 0);
      Poles.travelSides(1);
      break;}  
    case 17 : { // travel sides per letter, changing to random color and holding old color, in sync with poles, poles fade 
      Poles.freqdiv = 2;
      KRVN.freqdiv = 2; 
      uint8_t clusters[] = {4, 5, 4, 6};
      uint8_t numClusters = sizeof(clusters);
      KRVN.travelSides(color1, 1, 0, randDirection[0], numClusters, clusters, 1);
      Poles.travelSides(1);
      break;} 
    case 18 : { // travel sides per letter, changing to random color and holding old color, poles up and down
      Poles.freqdiv = 2;
      KRVN.freqdiv = 2; 
      uint8_t clusters[] = {4, 5, 4, 6};
      uint8_t numClusters = sizeof(clusters);
      KRVN.travelSides(color1, 1, 0, randDirection[0], numClusters, clusters, 1);
      Poles.upDown(1, 1);
      break;}   
    case 19 : { // fill up letters out of phase, travel sides poles with fade
      Poles.freqdiv = 2;
      KRVN.freqdiv = 2;
      uint8_t clusters[] = {2, 2, 2, 3, 2, 2, 2, 2, 2};
      int Direction[] = {1, -1, 1, -1, -1, 1, 1, -1, 1};
      uint8_t numClusters = sizeof(clusters);
      float phase[] = {0, 0, 0.5, 0.5, 0, 0, 0.5, 0.5, 0.5};
      KRVN.fillUp(color1, 0, 0, numClusters, clusters, 1, Direction, 1, phase);
      Poles.travelSides(1);
      break;}  
    case 20 : { // 30% chance flashing pixels, single flashing bulb per pole
      KRVN.flashingPixels(color1, color2, 30);
      Poles.flashingBulbs(1, 1, 1);
      break;}  
    case 21 : { // 50% chance flashing pixels, 50% flashing chance for all bulbs
      KRVN.flashingPixels(color1, color2, 50);
      Poles.flashingBulbs(1, 0, 0, 50);
      break;}  
    case 22 : { // wave of 1 letter with random direction, single random flashing bulb per pole
      KRVN.freqdiv = 2;
      KRVN.travelingWave(color1, 1, randDirection[0], 0.5);
      Poles.flashingBulbs(1, 1, 1);
      break;}  
    case 23 : {// KRVN fill up with random phase and direction per letter side, poles with random phase updown
      Poles.freqdiv = 2;
      KRVN.freqdiv = 2;
      uint8_t clusters[] = {2, 2, 2, 3, 2, 2, 2, 2, 2};
      int Direction[] = {1, -1, 1, -1, -1, 1, 1, -1, 1};
      uint8_t numClusters = sizeof(clusters);
      KRVN.fillUp(color1, 0, 0, numClusters, clusters, 1, randDirection, 1, randPhase);
      Poles.upDown(2, 1, 0, 0, 0, randDirection, 1, randPhase);
      break;} 
    case 24 : { // single random letter flashing in random color, single random flashing bulb per pole
      uint8_t clusters[] = {4, 5, 4, 6};
      uint8_t numClusters = sizeof(clusters);
      KRVN.travelSides(color1, 1, 0, 1, numClusters, clusters, 0, 1);
      Poles.flashingBulbs(1, 1, 1);
      break;}   
  }
}

// Task for handling the LEDs on core 1
void LEDTaskcode( void * pvParameters ){
  for(;;){
    if(millis()-loopTime >= Ts){
    
      loopTime = millis();
      setMode(stageData.Mode);
      KRVN.activateColor();
      Poles.setLevels();
      
    }
  }
}

// Task for handling the controller on core 0
void ControllerTaskcode( void * pvParameters ){
  delay(1000);
}


void setup() {
  Serial.begin(115200);

  // set the starting states of the LEDs
  float dimValue1 = stageData.Bright1;
  KRVN.setDimmer(dimValue1/100);
  KRVN.setBPM(stageData.BPM);

  float dimValue2 = stageData.Bright2;
  Poles.setDimmer(dimValue2/100);
  Poles.setBPM(stageData.BPM);

  //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
                    ControllerTaskcode, /* Task function. */
                    "SMSTask",   /* name of task. */
                    20000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &SMSTask,    /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */                  
  delay(500);

  //create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
                    LEDTaskcode, /* Task function. */
                    "LEDTask",   /* name of task. */
                    20000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &LEDTask,     /* Task handle to keep track of created task */
                    1);          /* pin task to core 1 */
  delay(500); 

}

void loop() {
}