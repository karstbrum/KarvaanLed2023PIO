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

Travel_Mode::Travel_Mode(uint8_t numberOfStrips_, uint8_t bpm_) {

    uint8_t activeStrips[] = {0};
    int currentStripIndex = 0;

    void Travel_Mode::clear() {
        activeStrips.clear();
    }
    // set all strips to active
    void Travel_Mode::allTogether(int numberOfStrips) {
        for (int x = 0; x < numberOfStrips; ++x) {
            activeStrips.push_back(x);
        }
    }

    // set strips to active in clockwise direction, per beat
    void Travel_Mode::clockwise(int numberOfStrips, int bpm) {
        currentStripIndex = 0;

        int delayInMilliseconds = static_cast<int>(60000.0 / bpm);

         while (true) {
            activeStrips.push_back(currentStripIndex);
            currentStripIndex = (currentStripIndex + 1) % numberOfStrips;

            std::this_thread::sleep_for(std::chrono::milliseconds(delayInMilliseconds));
        }
    }

    // set strip to active, one at a time in clockwise direction, per beat
    void Travel_Mode::clockwiseFastMoving(int numberOfStrips, int bpm) {
        currentStripIndex = 0;

        int delayInMilliseconds = static_cast<int>(60000.0 / bpm);

         while (true) {
            activeStrips = currentStripIndex;
            currentStripIndex = (currentStripIndex + 1) % numberOfStrips;

            std::this_thread::sleep_for(std::chrono::milliseconds(delayInMilliseconds));
        }
    }

    // set strips to active, going back and forth in both directions together
    void Travel_Mode::circleMotion(int numberOfStrips, int bpm) {
        startStripIndex = 0;
        endStripIndex = numberOfStrips - 1;

        int delayInMilliseconds = static_cast<int>(60000.0 / bpm); // Calculate delay between beats

        while (true) {
            activeStrips.push_back(startStripIndex);
            activeStrips.push_back(endStripIndex);

            startStripIndex = (startStripIndex + 1) % numberOfStrips;
            endStripIndex = (endStripIndex - 1 + numberOfStrips) % numberOfStrips;

            std::this_thread::sleep_for(std::chrono::milliseconds(delayInMilliseconds));
        }
    }

    void Travel_Mode::randomMotion(int numberOfStrips, int bpm) {
        std::vector<int> remainingStrips(numberOfStrips);
        std::iota(remainingStrips.begin(), remainingStrips.end(), 0);

        int delayInMilliseconds = static_cast<int>(60000.0 / bpm); // Calculate delay between beats

        std::random_device rd;
        std::mt19937 gen(rd());

        while (!remainingStrips.empty()) {
            int randomIndex = std::uniform_int_distribution<int>(0, remainingStrips.size() - 1)(gen);
            int randomStrip = remainingStrips[randomIndex];
            activeStrips.push_back(randomStrip);
            remainingStrips.erase(remainingStrips.begin() + randomIndex);

            std::this_thread::sleep_for(std::chrono::milliseconds(delayInMilliseconds));
        }

        while (!activeStrips.empty()) {
            int randomIndex = std::uniform_int_distribution<int>(0, activeStrips.size() - 1)(gen);
            activeStrips.erase(activeStrips.begin() + randomIndex);

            std::this_thread::sleep_for(std::chrono::milliseconds(delayInMilliseconds));
        }
    }
}
