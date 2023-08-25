#ifndef BULB_AUTO_MODE_H
#define BULB_AUTO_MODE_H
 
#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "bulb_driver.h"
#include <math.h>

#define INCREMENT 0.01
#define MAXSIDES_B 10

class Bulbgroups {

    public:
        // constructor
        Bulbgroups(uint8_t Bulbpins_[], uint8_t numSides_, uint8_t bulbsPerSide_[], uint16_t Ts_);

        // set the levels
        void setLevels();

        // dimmer values
        void setDimmer(float dimmerValue);
        void dimUp(float increment = INCREMENT);
        void dimDown(float increment = INCREMENT);

        // set BPM
        void setBPM(float BPM_);

        // just on
        void staticValue(float dim = 1);

        // pulse all lights
        void pulse(bool fade = 1, float onValue = 0.4);

        // group of lights up and down
        void upDown(uint8_t tailLength, bool dimTail = 1, bool fastUpDown = 0, bool fastOnlyUpDown = 0, bool setDirection = 0, int direction_[MAXSIDES_B] = {},
            bool setPhase = 0, float phase[MAXSIDES_B] = {});

        // propogating wave
        void travelingWave(float numOfSines = 1, int direction = 1, float offset = 0);

        // switch sides
        void travelSides(bool fade = 0, float onValue = 0.6, int direction = 1);

        // flash random bulbs
        void flashingBulbs(bool fade = 1, bool single = 0, bool perSide = 0, uint8_t flashChance = 50);

        // set cluster
        void setClusters(bool use_sides[MAXSIDES_B], float dim);

        // variables for pulse and updown
        float pulseIndex = 0;
        int sideIndex = 0;

        // frequency divider
        float freqdiv = 1;

    private:

        float BPM = 100;
        uint16_t Ts;
        uint8_t totalBulbs, numSides, bulbsPerSide[MAXBULBS];

        // bulbs class (self made)
        Bulbs* groups;

        float randomFloat();

        bool selectBulb[50];

};

// seperate functions


#endif