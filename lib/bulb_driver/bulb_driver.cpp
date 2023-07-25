#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "PWMBulbs.h"


Bulbs::Bulbs(uint8_t Bulbpins_[], uint8_t numCombinations_, uint8_t BulbsPerCombination_[]){

    // Bulpins contains the pin order per bulb combination
    // numCombinations contains the number of bulb combinations (use pin order!)
    // BulbsPerCombination contains the the numbers of pins per bulb combination (use pin order)
    // note that the number of entries in Bulbpins_ should match the sum of BulbsPerCombination_

    numBulbs = 0;
    numCombinations = numCombinations_;
    
    uint8_t channelIndex = 0;
    
    for (uint8_t k = 0;k < numCombinations;k++) {
        BulbsPerCombination[k] = BulbsPerCombination_[k];
        numBulbs += BulbsPerCombination[k];
    }

    for (uint8_t k = 0;k < numBulbs;k++) {
        Bulbpins[k] = Bulbpins_[k];
        PWM_channel[k] = k;
        ledcAttachPin(Bulbpins[k], k);
        ledcSetup(k, PWMFREQUENCY, PWMRANGEBIT);
    }

    // standard dimmer off, have to set in main code
    dimmer = 0;
    prevDimmer = 0;

    Bulbs::setBrightnessAll();

};

void Bulbs::setBrightnessIndividual(int k, float extraDimmer) {

    brightnessState[k] = dimmer*extraDimmer;

};

void Bulbs::setBrightnessAll(float extraDim) {

    for (int k = 0; k < numBulbs; k++) {

        Bulbs::setBrightnessIndividual(k, extraDim);

    };
};

void Bulbs::setRange(uint8_t startLED, uint8_t endLED, float extraDim) {

    for (int k = startLED; k <= endLED; k++) { // For each pixel in range

        Bulbs::setBrightnessIndividual(k, extraDim);

    };
};

void Bulbs::setRangeCenter(uint8_t center, uint8_t tail, bool fade) {

    float dim = 1;

    for (int k = center - tail; k <= center + tail; k++) { 
        if (fade) {
            float offDim = k - travelIndex;
            dim = 1 / (1 + abs(offDim));
        };

        Bulbs::setBrightnessIndividual(k, dim);

    };
};


void Bulbs::setBulbs() {

    //uint8_t dutyCycle;

    for (uint8_t k = 0; k < numBulbs; k++) {           // for each bulb  
        dutyCycle = static_cast<uint8_t>(brightnessState[k] * static_cast<float>(PWMRANGE));
        ledcWrite(PWM_channel[k], dutyCycle);       //  Set bulb brightness
    };

    
};
