#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "travel_mode.h"
#include <cmath>
#include <cstdlib> 
#include <ctime>
#include <random>

Travel_Mode::Travel_Mode(uint8_t numberOfStrips_, uint8_t bpm_, uint8_t travel_mode_) {

    uint8_t activeStrips[] = {0};

    void Travel_Mode::clear() {
        activeStrips.clear();
    }

    void Travel_Mode::allTogether() {
        for (int x = 0; x < numberOfStrips; ++x) {
            activeStrips.push_back(x);
        }
    }

    void Travel_Mode::clockwise() {
        activeStrips.clear();
        for (int x = 0; x < numberOfStrips; ++x) {
            activeStrips.push_back(x);
        }
    }
}
