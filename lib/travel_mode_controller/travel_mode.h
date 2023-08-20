#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

//#include "Adafruit_NeoPixel.h"
#include "led_driver.h"
#include <math.h>
#include <cstdlib> 
#include <ctime>
#include <random>


class Travel_Mode {
    // constructor
        Travel_Mode(uint8_t numberOfStrips_, uint8_t bpm_, uint8_t travel_mode_);

    // on each net mode, clear first
    void clear();
    
    void allTogether(float numberOfStrips, float travel_mode);

    void clockwise(float numberOfStrips, float travel_mode);
}