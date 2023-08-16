#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "bulb_auto_mode.h"
#include <cmath>
#include <cstdlib> 


Bulbgroups::Bulbgroups(uint8_t bulbpins_[], uint8_t numSides_, uint8_t bulbsPerSide_[], uint16_t Ts_) {

    Ts = Ts_; // sampling time
    
    numSides = numSides_;
    totalBulbs = 0;

    for (uint8_t k=0; k < numSides; k++) {
        bulbsPerSide[k] = bulbsPerSide_[k];
        totalBulbs += bulbsPerSide[k];
    }

    groups = new Bulbs(bulbpins_, numSides, bulbsPerSide); 

};

void Bulbgroups::setBPM(float BPM_) {
    BPM = BPM_;
}

void Bulbgroups::setDimmer(float dimmerValue) {
    groups->prevDimmer = groups->dimmer;
    groups->dimmer = dimmerValue;
};

// dimmer funtions
void Bulbgroups::dimUp(float increment) {

    groups->prevDimmer = groups->dimmer;

    if (groups->dimmer <= (1 - increment)) {
        groups->dimmer += increment;
    }
    else {
        groups->dimmer = 1;
    };
};

void Bulbgroups::dimDown(float increment) {

    groups->prevDimmer = groups->dimmer;

    if (groups->dimmer >= increment) {
        groups->dimmer -= increment;
    }
    else {
        groups->dimmer = 0;
    };
};

void Bulbgroups::setLevels() {
    groups->setBulbs();
};

void Bulbgroups::staticValue(bool on) {
    if(on){
        groups->setBrightnessAll(1);
    } else {
        groups->setBrightnessAll(0);
    }
}

// pulsing 
void Bulbgroups::pulse(bool fade, float onValue) {
    // make a sine wave, maybe with extended max time

    float Ts_ = Ts;
    pulseIndex += (Ts_ / 1000) * (BPM / 60) / freqdiv; // Ts*BPS (s^1 * s^-1)
    pulseIndex = (pulseIndex > 1) ? pulseIndex -= 1 : pulseIndex;

    float dimValue = 0.525 - 0.475 * cos(pulseIndex * 2 * PI);

    if (fade == 0) {
        dimValue = (pulseIndex > onValue) ? 1 : 0;
    }

    groups->setBrightnessAll(dimValue);

}

// move amount of bulbs down per side
void Bulbgroups::upDown(uint8_t tailLength, bool dimTail, bool fastUpDown, bool fastOnlyUpDown, bool setDirection, int direction_[MAXSIDES_B],
    bool setPhase, float phase_[MAXSIDES_B]) {

    float Ts_ = Ts;

    pulseIndex += ((Ts_ / 1000) * (BPM / 60)) / 2 / freqdiv; // Ts*BPS (s^1 * s^-1)
    pulseIndex = (pulseIndex > 1) ? pulseIndex -= 1 : pulseIndex;

    groups->setBrightnessAll(0);


    //set the correct and direction (direction for fastup etc.)
    float phase[numSides];
    int direction[numSides];

    for (uint16_t k = 0; k < numSides; k++) {
        if (setDirection) {
            direction[k] = direction_[k];
        }
        else {
            direction[k] = 0;
        }
        if (setPhase) {
            phase[k] = direction[k] * phase_[k];
        }
        else {
            phase[k] = 0;
        }
    }

    float pulseIndexCluster;

    for (uint16_t k = 0; k < numSides; k++) {

        if (direction[k] == 1 || direction[k] == 0) {
            pulseIndexCluster = pulseIndex;
        }
        else {
            pulseIndexCluster = 1 - pulseIndex;
        }

        float travelRange = bulbsPerSide[k]-1;
        float centerFloat;


        if (fastUpDown) {

            float sinInput = (pulseIndexCluster + phase[k]) / 2;
            sinInput = (sinInput > 0.5) ? 0 : sinInput;
            sinInput = (sinInput < 0) ? sinInput + 0.5 : sinInput;

            float sinOutput;
            if (direction[k] == 1) {
                sinOutput = sin(sinInput * 2 * PI);
            }
            else {
                sinOutput = 1 - sin(sinInput * 2 * PI);
            }

            centerFloat = travelRange * (sinOutput);


        }
        else if (fastOnlyUpDown) {

            float sinInput = (pulseIndexCluster + phase[k]) / 4;
            sinInput = (sinInput > 0.25) ? 0 : sinInput;
            sinInput = (sinInput < 0) ? sinInput + 0.25 : sinInput;

            float sinOutput;
            if (direction[k] == 1) {
                sinOutput = 1 + sin((sinInput - 0.25) * 2 * PI);
            }
            else {
                sinOutput = sin((sinInput) * 2 * PI);
            }

            centerFloat = travelRange * (sinOutput);

        }
        else {
            centerFloat = travelRange / 2 + travelRange * sin((pulseIndexCluster + phase[k]) * 2 * PI) / 2;
        }

        // endLastSide is the end of the last side
        uint8_t startSide = 0;
        for (uint16_t l = 0; l < k; l++) {
            startSide += bulbsPerSide[l];
        }

        if (dimTail) {
            for (float l = 0; l < bulbsPerSide[k]; l++) {
                float tailFloat = static_cast<float>(tailLength);
                if (l >= (centerFloat - tailFloat) && l <= (centerFloat  + tailFloat)) {
                    float relPos = (l - (centerFloat - tailFloat)) / (tailFloat * 2);
                    float bulbValue = sin(relPos * PI);
                    
                    groups->setBrightnessIndividual(startSide + l, bulbValue);
                }
                else {
                    groups->setBrightnessIndividual(startSide + l, 0);
                }
            }
        }
        else {
            // center is the center of the traveling light
            uint8_t center = startSide + static_cast<uint8_t>(centerFloat);

            int startLED = center - tailLength;
            int endLED = center + tailLength;

            startLED = (startLED < startSide) ? startSide : startLED;
            endLED = (endLED >= (startSide + bulbsPerSide[k])) ? (startSide + bulbsPerSide[k] - 1) : endLED;
            groups->setRange(startLED, endLED, 1);
        }

    }

}

void Bulbgroups::travelingWave(float numOfSines, int direction, float offset) {

    float Ts_ = Ts;

    if (direction == 1) {
        pulseIndex += ((Ts_ / 1000) * (BPM / 60)) / freqdiv; // Ts*BPS (s^1 * s^-1)
        pulseIndex = (pulseIndex > 1) ? pulseIndex - 1 : pulseIndex;
    }
    else if (direction == -1) {
        pulseIndex -= ((Ts_ / 1000) * (BPM / 60)) / freqdiv; // Ts*BPS (s^1 * s^-1)
        pulseIndex = (pulseIndex < 0) ? pulseIndex + 1 : pulseIndex;
    }

    float travelRange = totalBulbs;
    travelRange = numOfSines / (travelRange);

    for (uint16_t k = 0; k < totalBulbs; k++) {

        float bulbValue = (0.5 + 0.5 * sin((k * travelRange + pulseIndex) * 2 * PI) - offset) / (1 - offset);
        bulbValue = (bulbValue < 0) ? 0 : bulbValue;

        groups->setBrightnessIndividual(k, bulbValue);

    }

}

// switch from side to side
void Bulbgroups::travelSides(bool fade, float onValue, int direction) {

    float Ts_ = Ts;
    pulseIndex += (Ts_ / 1000) * (BPM / 60) / freqdiv; // Ts*BPS (s^1 * s^-1)
    if (pulseIndex > 1) {
        pulseIndex -= 1;
        sideIndex += direction;
    }

    float dimValue = 0.5 + 0.5 * sin((pulseIndex - 0.25) * 2 * PI);

    if (direction == 1) {
        sideIndex = (sideIndex >= numSides) ? 0 : sideIndex;
    }
    else {
        sideIndex = (sideIndex < 0) ? numSides - 1 : sideIndex;
    }

    uint16_t bulbStart = 0;
    for (uint8_t k = 0; k < sideIndex; k++) {
        bulbStart += bulbsPerSide[k];
    }

    uint16_t bulbEnd = bulbStart + bulbsPerSide[sideIndex] - 1;

    if (fade == 0) {
        dimValue = (dimValue > onValue) ? 1 : 0;
    }

    groups->setBrightnessAll(0);
    groups->setRange(bulbStart, bulbEnd, dimValue);

}

// random or sequential flashing bulbs, per side or total
void Bulbgroups::flashingBulbs(bool fade, bool single, bool perSide, uint8_t flashChance) {

    float Ts_ = Ts;

    pulseIndex += (Ts_ / 1000) * (BPM / 60) / freqdiv; // Ts*BPS (s^1 * s^-1)

    float dimValue;

    if (!fade) {
        dimValue = 1;
    }
    else {
        dimValue = 0.5 + 0.5 * sin((pulseIndex - 0.25) * 2 * PI);
    }

    if (pulseIndex > 1) {
        pulseIndex -= 1;

        // select a single bulb to flash
        if (single) {
            for (int k = 0; k < totalBulbs; k++) {
                selectBulb[k] = 0;
            }
            if (perSide) {
                for (int k = 0; k < numSides; k++) {
                    // determine the starting bulb
                    uint8_t startSide = 0;
                    for (uint16_t l = 0; l < k; l++) {
                        startSide += bulbsPerSide[l];
                    }
                    uint8_t bulb = rand() % bulbsPerSide[k];
                    selectBulb[startSide + bulb] = 1;
                }
            }
            else {
                // select bulb to flash
                uint8_t bulb = rand() % totalBulbs;
                selectBulb[bulb] = 1;
            }
        }
        else {
            for (int k = 0; k < totalBulbs; k++) {
                // chance of a bulb to flash
                if ((rand() % 100) < flashChance) {
                    selectBulb[k] = 1;
                }
                else {
                    selectBulb[k] = 0;
                }
            }
        }
    }
    // first set all to off
    groups->setBrightnessAll(0);

    // set the bulbs that are switched on
    for (int k = 0; k < totalBulbs; k++) {
        if (selectBulb[k]) {
            groups->setBrightnessIndividual(k, dimValue);
        }
        else {
            Serial.print(0);
        }
    }
}



float Bulbgroups::randomFloat() {

    uint8_t randnum = rand() % 100;
    return 0.1 + static_cast<float>(randnum) / 125;

}