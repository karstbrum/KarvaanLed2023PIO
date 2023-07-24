
// for SMS processing
#include "Adafruit_FONA.h"

//#include <Arduino.h>

// LED effects library
#include <LEDdifferentsizes.h>

// PWM lights library
#include <PWMBulbgroups.h>

// define for interaction with SIM800 module
#define FONA_RX     27
#define FONA_TX     26
#define FONA_PWRKEY 4
#define FONA_RST    5
#define FONA_POWER  23

// number of colors used for LED
#define numOfColors 14

// maxes for texting
#define MAXCOLORS 15
#define MAXBRIGHT 100
#define MAXMODES 25
#define MAXBPM 300

// Sampling time (Ts)
#define Ts 25

// set up the different cores
TaskHandle_t SMSTask;
TaskHandle_t LEDTask;

// NOW FIRST SET UP THE SMS FUNCTION PART

#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
// For UNO and others without hardware serial,
// we default to using software serial. If you want to use hardware serial
// (because softserial isnt supported) comment out the following three lines 
// and uncomment the HardwareSerial line
#include <SoftwareSerial.h>

SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;

#else
// On Leonardo/M0/etc, others with hardware serial, use hardware serial!
HardwareSerial *fonaSerial = &Serial1;

#endif

// setup fona
Adafruit_FONA fona = Adafruit_FONA(FONA_PWRKEY);

// not really needed
uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout = 0);

char fonaNotificationBuffer[64];          //for notifications from the FONA
char smsBuffer[250];
char prevsmsBuffer[250];

char textPart[16] = {0};
char numberPart[16] = {0};


// NOW SETUP PART FOR LED

// Time spent in the main loop
int loopTime = 0;

// timer for random modes
int randomTimer = 0;
int modeTimer = 0;
int colorTimer = 0;

// Pixels(numSides, pixelsPerSide, pixelsRing, LEDpin, Ts)
uint8_t LEDsPerSide[] = {12, 13, 17, 17, 12, 12, 13, 13, 17, 
                         12, 13, 13, 12, 12, 13, 13, 13, 13, 12};
uint8_t numSides = sizeof(LEDsPerSide);
uint8_t sidesPerPin[] = {9, 10};
uint8_t LEDPins[] = {13, 14};
uint8_t numPins = sizeof(LEDPins);

Pixels KRVN(numSides, LEDsPerSide, numPins, sidesPerPin, LEDPins, Ts);

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

// create instance of state data to use and change
stateData stageData;

void setup() {

  pinMode(FONA_POWER, OUTPUT);

  digitalWrite(FONA_POWER, HIGH);
  
  while (!Serial);

  Serial.begin(115200);
  //Serial.println(F("FONA SMS caller ID test"));
  //Serial.println(F("Initializing....(May take 3 seconds)"));

  delay(2000);
  // make it slow so its easy to read!
  fonaSerial->begin(4800, SERIAL_8N1, FONA_TX, FONA_RX);
  if (! fona.begin(*fonaSerial)) {
    //Serial.println(F("Couldn't find FONA"));
    while(1);
  }
  //Serial.println(F("FONA is OK"));

  // Print SIM card IMEI number.
  char imei[16] = {0}; // MUST use a 16 character buffer for IMEI!
  uint8_t imeiLen = fona.getIMEI(imei);
  if (imeiLen > 0) {
    //Serial.print("SIM card IMEI: "); Serial.println(imei);
  }

  // enter pin number of sim
  fonaSerial->println("AT+CPIN=0000");
  delay(500);

  fonaSerial->println("AT+CNMI=2,1\r\n");  //set up the FONA to send a +CMTI notification when an SMS is received
  delay(500);

  // Configuring module in TEXT mode
  fonaSerial->println("AT+CMGF=1");
  delay(1000);

  // delete all sms still present
  fonaSerial->println("AT+CMGD=1,1");
  delay(1000);

  // setup is done
  Serial.println("Setup ready");

  // set the starting states of the LEDs
  float dimValue1 = stageData.Bright1;
  KRVN.setDimmer(dimValue1/100);
  KRVN.setBPM(stageData.BPM);

  float dimValue2 = stageData.Bright2;
  Poles.setDimmer(dimValue2/100);
  Poles.setBPM(stageData.BPM);

  //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
                    SMSTaskcode, /* Task function. */
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

// Task for processing SMS on core 0
void SMSTaskcode( void * pvParameters ){
  for(;;){

    // look for new SMS
    getSMS();
    
    if(strcmp(smsBuffer, prevsmsBuffer)){
      
      // change state of the LEDs
      changeState();
      
    }

    // assign array
    strcpy(prevsmsBuffer, smsBuffer);

    // delay to not let esp crash
    delay(10);
  }
}

// Task for handling the LEDs on core 1
void LEDTaskcode( void * pvParameters ){
  for(;;){
    if(millis()-loopTime >= Ts){
    
      loopTime = millis();
      colorMode(stageData.Mode);
      KRVN.activateColor();
      Poles.setLevels();
      
    }
  }
}


void loop() {
 
}



void getSMS(){
  
  char* bufPtr = fonaNotificationBuffer;    //handy buffer pointer
  
  if (fona.available())      //any data available from the FONA?
  {
    int slot = 1;            //this will be the slot number of the SMS
    int charCount = 0;
    //Read the notification into fonaInBuffer
    do  {
      *bufPtr = fona.read();
      //Serial.write(*bufPtr);
      delay(10);
    } while ((*bufPtr++ != '\n') && (fona.available()) && (++charCount < (sizeof(fonaNotificationBuffer)-1)));
    
    //Add a terminal NULL to the notification string
    *bufPtr = 0;
    
    //Scan the notification string for an SMS received notification.
    //  If it's an SMS message, we'll get the slot number in 'slot'
    if (sscanf(fonaNotificationBuffer, "+CMTI: " FONA_PREF_SMS_STORAGE ",%d", &slot)>0) {
      //Serial.print("slot: "); Serial.println(slot);
      
      char callerIDbuffer[32];  //we'll store the SMS sender number in here
      
      // Retrieve SMS sender address/phone number.
      if (! fona.getSMSSender(slot, callerIDbuffer, 31)) {
        //Serial.println("Didn't find SMS message in slot!");
      }
      //Serial.print(F("FROM: ")); Serial.println(callerIDbuffer);

      // Retrieve SMS value
      uint16_t smslen;
      if (fona.readSMS(slot, smsBuffer, 250, &smslen)) { // pass in buffer and max len!
        //Serial.println(smsBuffer);
      }

      
      // delete the original msg after it is processed
      //   otherwise, we will fill up all the slots
      //   and then we won't be able to receive SMS anymore
      fonaSerial->println("AT+CMGD=1,1");
    }
  }
}


// for the LED state changes:

void colorMode(uint16_t cmode){
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

void changeState() {
  /* test code for working changes
  Serial.print("Changed from ");
  Serial.print(prevsmsBuffer);
  Serial.print(" to ");
  Serial.println(smsBuffer);
  */

  //Serial.println(textPart);
  //Serial.println(numberPart);

  char* smsPtr = smsBuffer;    
  char* textPtr = textPart;
  char* numberPtr = numberPart;
  
  int charCount = 0;
  
  // if starting with text, extract the text part
  if (!isDigit(*smsPtr)){
    do  {
      *textPtr++ = toupper(*smsPtr++);
    } while (!isDigit(*smsPtr) &&  (++charCount < (sizeof(textPart)-1)));
  }
  
  // now extract the number part till a text character is found again
  charCount = 0;
  if (isDigit(*smsPtr)){
    do  {
      *numberPtr++ = *smsPtr++;
    } while (isDigit(*smsPtr) &&  (++charCount < (sizeof(numberPart)-1)));
  }

  //Serial.println(textPart);
  //Serial.println(numberPart);
  
  // if starting wiht BPM, set BPM
  if(strlen(textPart) != 0){
    if ((!strcmp(textPart,"BPM") || !strcmp(textPart,"B")) && (strlen(numberPart) != 0) && (atoi(numberPart) <= MAXBPM) && (atoi(numberPart) >= 1)){
      //Serial.println("BPM is gezet op: ");
      //Serial.print(atoi(numberPart));
      stageData.BPM = atoi(numberPart);
    } // else check if brightness is set
    else if ((!strcmp(textPart,"HELDERHEIDK") || !strcmp(textPart,"HLDRK") || !strcmp(textPart,"HK")) && (strlen(numberPart) != 0) && (atoi(numberPart) <= MAXBRIGHT) && (atoi(numberPart) >= 0)){ 
      stageData.Bright1 = atoi(numberPart);
    }
    else if ((!strcmp(textPart,"HELDERHEIDP") || !strcmp(textPart,"HLDRP") || !strcmp(textPart,"HP")) && (strlen(numberPart) != 0) && (atoi(numberPart) <= MAXBRIGHT) && (atoi(numberPart) >= 0)){ 
      stageData.Bright2 = atoi(numberPart)/2; // max to 50
    }
    else if ((!strcmp(textPart,"HBART") || !strcmp(textPart,"HKARST") || !strcmp(textPart,"HROBIN")) && (strlen(numberPart) != 0) && (atoi(numberPart) <= MAXBRIGHT) && (atoi(numberPart) >= 0)){ 
      stageData.Bright2 = atoi(numberPart); // max to 100   
    } // else check for the different colors, must use (else) ifs since switch case cannot be done with char/string
    else {
      if ((!strcmp(textPart,"KLEUR") || !strcmp(textPart,"K") || !strcmp(textPart,"COLOR")  || !strcmp(textPart,"C")) && (atoi(numberPart) <= MAXCOLORS) && (atoi(numberPart) >= 1)){
        stageData.color = atoi(numberPart);
      } else if (!strcmp(textPart,"WARMWIT")){
        stageData.color = 1;
      } else if (!strcmp(textPart,"WIT")){
        stageData.color = 2;
      } else if (!strcmp(textPart,"ROOD")){
        stageData.color = 3;
      } else if (!strcmp(textPart,"GROEN")){
        stageData.color = 4;
      } else if (!strcmp(textPart,"BLAUW")){
        stageData.color = 5;
      } else if (!strcmp(textPart,"ROZE")){
        stageData.color = 6;
      } else if (!strcmp(textPart,"GROENGEEL")){
        stageData.color = 7;
      } else if (!strcmp(textPart,"AZUUR")){
        stageData.color = 8;
      } else if (!strcmp(textPart,"PAARS")){
        stageData.color = 9;
      } else if (!strcmp(textPart,"GEEL")){
        stageData.color = 10;
      } else if (!strcmp(textPart,"CYAAN")){
        stageData.color = 11;
      } else if (!strcmp(textPart,"VIOLET")){
        stageData.color = 12;
      } else if (!strcmp(textPart,"LICHTGROEN")){
        stageData.color = 13;
      } else if (!strcmp(textPart,"ORANJE")){
        stageData.color = 14;
      } else if (!strcmp(textPart,"RANDOM")){
        stageData.color = 15;
      }          
    }

  } // if starts with digit, set the mode
  else if (strlen(numberPart) != 0 && atoi(numberPart)<=MAXMODES){
    //Serial.println("Het volgende nummer is alleen ingevoerd: ");
    //Serial.print(atoi(numberPart));
    stageData.Mode = atoi(numberPart);
    //set to zero to account for clusters (non cluster can have higher index)
    KRVN.sideIndex = 0;
    // sync KRVN and poles
    KRVN.pulseIndex = 0;
    Poles.pulseIndex = 0;
    // reset frequency divider
    KRVN.freqdiv = 1;
    Poles.freqdiv = 1;
    // reset the random timer (not necessary?)
    randomTimer = 0;
    modeTimer = 0;
  }

  // clear SMS buffer, otherwise there can be trouble
  memset(&smsBuffer[0], 0, sizeof(smsBuffer));
  memset(&textPart[0], 0, sizeof(textPart));
  memset(&numberPart[0], 0, sizeof(numberPart));

  // set the BPM and dimmer values  
  float dimValue1 = stageData.Bright1;
  KRVN.setDimmer(dimValue1/100);
  KRVN.setBPM(stageData.BPM);

  float dimValue2 = stageData.Bright2;
  Poles.setDimmer(dimValue2/100);
  Poles.setBPM(stageData.BPM);




}
