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

public:
    // on each travel mode, clear first
    void clear();

    void allTogether(int numberOfStrips);

    void clockwise(int numberOfStripsm, int bpm);

    void clockwiseFastMoving(int numberOfStrips, int bpm);

    void circleMotion(int numberOfStrips, int bpm);
}