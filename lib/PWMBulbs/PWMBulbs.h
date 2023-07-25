#ifndef PWMBULBS_H
#define PWMBULBS_H
 
#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#define MAXBULBS 15
#define PWMFREQUENCY 1000
#define PWMRANGEBIT 8
#define PWMRANGE 255 //2^PWMRANGEBIT-1

class Bulbs {

    public:
        // constructor
        Bulbs(uint8_t Bulbpins_[], uint8_t numCombinations_, uint8_t BulbsPerCombination_[]);

        // update colors
        void setBrightnessAll(float extraDim = 1);

        // set single pixel
        void setBrightnessIndividual(int k, float extraDimmer = 1);

        // set range of lights 
        // set start and end
        void setRange(uint8_t startLED = 0, uint8_t endLED = 1, float extraDim = 1);
        // center oriented
        void setRangeCenter(uint8_t center = 1, uint8_t tail = 1,  bool fade = 0);

        // set te lights to states
        void setBulbs();

        // states of the lights
        float brightnessState[MAXBULBS];

        float dimmer, prevDimmer;

        uint8_t numBulbs, numCombinations, travelIndex = 0;

        uint8_t Bulbpins[MAXBULBS], BulbsPerCombination[MAXBULBS], PWM_channel[MAXBULBS];

        uint8_t dutyCycle;

};




#endif