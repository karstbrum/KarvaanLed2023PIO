#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "led_auto_mode.h"
#include <cmath>
#include <cstdlib> 
#include <ctime>
#include <random>


Pixels::Pixels(uint8_t numSides_, uint8_t pixelsPerSide_[], uint8_t numPins_, uint8_t sidesPerPin_[], uint8_t LEDPin_[], uint16_t Ts_) {

    Ts = Ts_; // sampling time

    // number of pixels per group which are defined as a group
    numSides = numSides_;

    // pixels per pin are needed to setup the strip
    uint8_t pixelsPerPin[numPins_] = {0};

    // total amount of pixels
    totalPixels = 0;

    for (uint8_t k = 0; k < numSides_; k++) {
        pixelsPerSide[k] = pixelsPerSide_[k];
        totalPixels += pixelsPerSide[k];
    }

    // define number of pixels per pin
    uint8_t sideIndex = 0;
    for (uint8_t pinIndex = 0; pinIndex < numPins_; pinIndex++) {
        for (uint8_t k = 0; k < sidesPerPin_[pinIndex]; k++) {
            pixelsPerPin[pinIndex] += pixelsPerSide[sideIndex];
            sideIndex += 1;
        }
    }

    strip = new RGBW(pixelsPerPin, LEDPin_, numPins_);

    Pixels::defineFirstColors();    

    // set random seed
    srand(static_cast <unsigned> (time(0)));
};

void Pixels::defineFirstColors() { //colors defined clockwise circle -> rainbow definition?
    strip->addColor(255,   0,   0,   0); //warm white 0
    strip->addColor(  0, 255, 255, 255); //white 1
    strip->addColor(  0, 255,   0,   0); //red 2
    strip->addColor(  0,   0, 255,   0); //green 3
    strip->addColor(  0,   0,   0, 255); //blue 4
    strip->addColor(  0, 255,   0, 128); //rose 5
    strip->addColor(  0, 255, 255,   0); //chartreuse 6
    strip->addColor(  0,   0, 128, 255); //azure 7
    strip->addColor(  0, 255,   0, 255); //magenta 8
    strip->addColor(  0, 255, 200,   0); //yellow 9
    strip->addColor(  0,   0, 255, 255); //cyan 10
    strip->addColor(  0, 180,   0, 255); //violet 11 
    strip->addColor(  0,   0, 255, 80); //spring green 12  
    strip->addColor(  0, 255,  80,   0); //orange 13

    //strip->addColor(0, 255, 60, 128); //14
    //strip->addColor(0, 128, 255, 60); //15
    //strip->addColor(0, 60, 128, 255); //16
    //strip->addColor(0, 255, 60, 255); //17
    //strip->addColor(0, 255, 255, 60); //18
    //strip->addColor(0, 60, 255, 255); //19
    //strip->addColor(0, 128, 60, 255); //20
    //strip->addColor(0, 60, 255, 128); //21  
    //strip->addColor(0, 255, 128, 60); //22

};

void Pixels::setBPM(float BPM_) {
    BPM = BPM_;
}

void Pixels::setDimmer(float dimmerValue) {
    strip->prevDimmer = strip->dimmer;
    strip->dimmer = dimmerValue;
};

// dimmer funtions
void Pixels::dimUp(float increment) {

    strip->prevDimmer = strip->dimmer;

    if (strip->dimmer <= (1 - increment)) {
        strip->dimmer += increment;
    }
    else {
        strip->dimmer = 1;
    };
};

void Pixels::dimDown(float increment) {

    strip->prevDimmer = strip->dimmer;

    if (strip->dimmer >= increment) {
        strip->dimmer -= increment;
    }
    else {
        strip->dimmer = 0;
    };
};

// set color
void Pixels::setColor(uint8_t colorIndex) {
    strip->setColorsAll(colorIndex, 1);
};


// different modes
// rainbow
void Pixels::Rainbow(bool moving, int direction) {
    // for definition use only full colors (0 - 255) definitions
    // use 3 5 7 8 11 13
    // define for outer strips
    // check if direction is 1 or -1, if else, make it 1
    if ((direction != 1) && (direction != -1)) {
        direction = 1;
    };

    uint16_t startingPixel = 0;

    
    // set the whole range of pixels fit the entire color spectrum
    // interpolate between the defined colors
    // 6 colors defined, start and stop at the same color
    // after every 'fixed' color, there is a range of totalPixels/6 until the next fixed color
    // make function to make it always work, regardless of number of pixels

    // if moving
    float Ts_ = Ts;

    if (moving) {

        if (direction == 1) {
            pulseIndex += (Ts_ / 1000) * (BPM / 60) / 6 / freqdiv;
            pulseIndex = (pulseIndex > 1) ? pulseIndex - 1 : pulseIndex;
        }
        else if (direction == -1) {
            pulseIndex -= (Ts_ / 1000) * (BPM / 60) / 6 / freqdiv;
            pulseIndex = (pulseIndex < 0) ? pulseIndex + 1 : pulseIndex;
        }
        rainbowIndex = static_cast<uint8_t>(pulseIndex * totalPixels);
    }
    

    // for all pixels, set the color
    for (uint8_t k = startingPixel; k < startingPixel + totalPixels; k++) {

        // cast to float to do proper calculations, also start at 0
        float rainbowIndex_ = rainbowIndex;
        float k_ = k - startingPixel;
        float totalPixels_ = totalPixels;

        k_ += rainbowIndex;

        if (k_> totalPixels_){
            k_ -= totalPixels_;
        }

        float relativeColor = 6 * k_ / totalPixels_;

        // white state is always zero   
        float white = 0;
        float red, green, blue;

        // red
        if ((relativeColor > 5) || (relativeColor <= 1)) {
            red = 255;
        } 
        else if ((relativeColor > 2) && (relativeColor <= 4)) {
            red = 0;
        }
        else if ((relativeColor > 4) && (relativeColor <= 5)) { // ramp up
            red = 255 * (relativeColor - 4);
        }
        else if ((relativeColor > 1) && (relativeColor <= 2)) { // ramp down
            red = 255 * (2 - relativeColor); 
        }

        // green
        if ((relativeColor > 3) && (relativeColor <= 5)) {
            green = 255;
        }
        else if (relativeColor <= 2) {
            green = 0;
        }
        else if ((relativeColor > 2) && (relativeColor <= 3)) { // ramp up
            green = 255 * (relativeColor - 2);
        }
        else if (relativeColor > 5) { // ramp down
            green = 255 * (6 - relativeColor);
        }

        // blue
        if ((relativeColor > 1) && (relativeColor <= 3)) {
            blue = 255;
        }
        else if (relativeColor > 4) {
            blue = 0;
        }
        else if (relativeColor <= 1) { // ramp up
            blue = 255 * (relativeColor - 0);
        }
        else if ((relativeColor > 3) && (relativeColor <= 4)) { // ramp down
            blue = 255 * (4 - relativeColor);
        }

        blue *= 0.7;
        green *= 0.7;
       
        strip->setColorsIndividual(k, white, red, green, blue);
        
    };

};


// pulsating color same color
void Pixels::pulseSameColor(uint8_t colorIndex, bool fade, float onValue) {
    // make a sine wave, maybe with extended max time
    
    float Ts_ = Ts;
    pulseIndex += (Ts_ / 1000) * (BPM / 60) / freqdiv; // Ts*BPS (s^1 * s^-1)
    pulseIndex = (pulseIndex > 1) ? pulseIndex -= 1 : pulseIndex;

    float dimValue = 0.525 - 0.475 * cos(pulseIndex * 2 * PI);

    if (fade == 0) {
        dimValue = (pulseIndex > onValue) ? 1 : 0;
    }
    
    strip->setColorsAll(colorIndex, dimValue);

}

// pulsating color fade range
void Pixels::pulseFadeColor(uint8_t color1, uint8_t color2, bool fade, float onValue, uint8_t numClusters_, uint8_t clusters_[]) {
    // make a sine wave, maybe with extended max time

    // define clusters
    // numClusters_ defines the number of clusters defined in clusters_
    // clusters_ contains the number of consecuteve sides for the cluster
    uint8_t numClusters;
    uint8_t pixelsPerCluster[MAXSIDES_L];
    if (numClusters_ == 0) {
        numClusters = numSides;
        for (uint8_t k = 0; k < numClusters; k++) {
            pixelsPerCluster[k] = pixelsPerSide[k];
        }
    }
    else {
        numClusters = numClusters_;
        uint8_t sideIndex = 0;
        for (uint8_t k = 0; k < numClusters; k++) {
            pixelsPerCluster[k] = 0;
            for (uint8_t l = 0; l < clusters_[k]; l++) {
                pixelsPerCluster[k] += pixelsPerSide[sideIndex];
                sideIndex += 1;
            }
        }
    }

    float Ts_ = Ts;
    pulseIndex += (Ts_ / 1000) * (BPM / 60) / freqdiv; // Ts*BPS (s^1 * s^-1)
    pulseIndex = (pulseIndex > 1) ? pulseIndex - 1 : pulseIndex;

    float dimValue = 0.525 - 0.475 * cos(pulseIndex * 2 * PI);

    if (fade == 0) {
        dimValue = (pulseIndex > onValue) ? 1 : 0;
    }

    for (uint8_t k = 0; k < numClusters; k++) {

        uint16_t startLED = k * pixelsPerCluster[k];
        uint16_t endLED = ((k + 1) * pixelsPerCluster[k]) - 1;

        strip->setRangeColorFade(startLED, endLED, color1, color2, dimValue);
    }

}

// pulsating color to different color
void Pixels::pulseToOtherColor(bool random, bool fade, float onValue) {

    // make a sine wave, maybe with extended max time

    float Ts_ = Ts;
    pulseIndex += (Ts_ / 1000) * (BPM / 60) / freqdiv; // Ts*BPS (s^1 * s^-1)

    if (pulseIndex > 1) {
        pulseIndex -= 1;
        if (random) {
            pulseColorIndex = rand() % (strip->numOfColors - 1);
        }
        else {
            pulseColorIndex += 1;
            pulseColorIndex = (pulseColorIndex >= strip->numOfColors) ? 0 : pulseColorIndex;
        }
        
    }

    float dimValue = 0.525 - 0.475 * cos(pulseIndex * 2 * PI);

    if (fade == 0) {
        dimValue = (pulseIndex > onValue) ? 1 : 0;
    }

    strip->setColorsAll(pulseColorIndex, dimValue);

}

// move small row up and down per side
void Pixels::upDown(float tailLength, uint8_t colorIndex, bool fastUpDown, bool fastOnlyUpDown, uint8_t numClusters_, uint8_t clusters_[],
    bool setDirection, int direction_[], bool inverse , bool setPhase, float phase_[], int inverseDirection) {

    // define clusters
    // numClusters_ defines the number of clusters defined in clusters_
    // clusters_ contains the number of consecuteve sides for the cluster
    uint8_t numClusters;
    uint8_t pixelsPerCluster[MAXSIDES_L];
    if (numClusters_ == 0) {
        numClusters = numSides;
        for (uint8_t k = 0; k < numClusters; k++) {
            pixelsPerCluster[k] = pixelsPerSide[k];
        }
    }
    else {
        numClusters = numClusters_;
        uint8_t sideIndex = 0;
        for (uint8_t k = 0; k < numClusters; k++) {
            pixelsPerCluster[k] = 0;
            for (uint8_t l = 0; l < clusters_[k]; l++) {
                pixelsPerCluster[k] += pixelsPerSide[sideIndex];
                sideIndex += 1;
            }
        }
    }

    float Ts_ = Ts;

    if (inverseDirection == -1 || inverseDirection == 0) {
        pulseIndex -= ((Ts_ / 1000) * (BPM / 60)) / 2 / freqdiv; // Ts*BPS (s^1 * s^-1)
        pulseIndex = (pulseIndex < 0) ? pulseIndex += 1 : pulseIndex;
    }
    else {
        pulseIndex += ((Ts_ / 1000) * (BPM / 60)) / 2 / freqdiv; // Ts*BPS (s^1 * s^-1)
        pulseIndex = (pulseIndex > 1) ? pulseIndex -= 1 : pulseIndex;
    }
    
    
    if (inverse) {
        strip->setColorsAll(colorIndex, 1);
    }
    else {
        strip->setColorsAll(colorIndex, 0);
    }

    //set the correct and direction (direction for fastup etc.)
    float phase[numClusters];
    int direction[numClusters];

    for (uint16_t k = 0; k < numClusters; k++) {
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

    for (uint16_t k = 0; k < numClusters; k++) {

        if (direction[k] == 1 || direction[k] == 0) {
            pulseIndexCluster = pulseIndex;
        }
        else {
            pulseIndexCluster = 1 - pulseIndex;
        }

        float travelRange = pixelsPerCluster[k];
        uint16_t centerFloat;

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

            centerFloat = static_cast<uint8_t>(travelRange * (sinOutput));
            

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

            centerFloat = static_cast<uint8_t>(travelRange * (sinOutput));

        }
        else {
            centerFloat = static_cast<uint8_t>(travelRange / 2 + travelRange * sin((pulseIndexCluster + phase[k]) * 2 * PI) / 2);
        }

        // endLastCluster is the end of the last Cluster
        uint8_t startCluster = 0;
        for (uint16_t l = 0; l < k; l++) {
            startCluster += pixelsPerCluster[l];
        }

        // center is the center of the traveling light
        uint8_t center = startCluster + centerFloat;

        int startLED = center - pixelsPerCluster[k] * tailLength;
        int endLED = center + pixelsPerCluster[k] * tailLength;

        startLED = (startLED < startCluster) ? startCluster : startLED;
        endLED = (endLED >= (startCluster + pixelsPerCluster[k])) ? (startCluster + pixelsPerCluster[k] - 1) : endLED;

        if (inverse) {
            strip->setRange(startLED, endLED, colorIndex, 0);
        }
        else {
            strip->setRange(startLED, endLED, colorIndex, 1);
        }

    }

}

void Pixels::fillUp(uint8_t colorIndex, bool fastUpDown, bool fastOnlyUpDown,
    uint8_t numClusters_, uint8_t clusters_[], bool setDirection, int direction_[], bool setPhase, float phase_[], int inverseDirection) {
    // define clusters
    // numClusters_ defines the number of clusters defined in clusters_
    // clusters_ contains the number of consecuteve sides for the cluster
    uint8_t numClusters;
    uint8_t pixelsPerCluster[MAXSIDES_L];
    float clusterPhase[MAXSIDES_L];

    if (numClusters_ == 0) {
        numClusters = numSides;
        for (uint8_t k = 0; k < numClusters; k++) {
            pixelsPerCluster[k] = pixelsPerSide[k];
            clusterPhase[k] = 0;
        }
    }
    else {
        numClusters = numClusters_;
        uint8_t sideIndex = 0;
        for (uint8_t k = 0; k < numClusters; k++) {
            pixelsPerCluster[k] = 0;
            for (uint8_t l = 0; l < clusters_[k]; l++) {
                pixelsPerCluster[k] += pixelsPerSide[sideIndex];
                sideIndex += 1;
            }
        }
    }

    float Ts_ = Ts;
    if (inverseDirection == -1 || inverseDirection == 0) {
        pulseIndex -= ((Ts_ / 1000) * (BPM / 60)) / 2 / freqdiv; // Ts*BPS (s^1 * s^-1)
        pulseIndex = (pulseIndex < 0) ? pulseIndex += 1 : pulseIndex;
    }
    else {
        pulseIndex += ((Ts_ / 1000) * (BPM / 60)) / 2 / freqdiv; // Ts*BPS (s^1 * s^-1)
        pulseIndex = (pulseIndex > 1) ? pulseIndex -= 1 : pulseIndex;
    }

    strip->setColorsAll(colorIndex, 0);

    //set the correct and direction (direction for fastup etc.)
    float phase[numClusters];
    int direction[numClusters];

    for (uint16_t k = 0; k < numClusters; k++) {
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
    uint16_t leadingPixel;

    for (uint16_t k = 0; k < numClusters; k++) {

        if (direction[k] == 1 || direction[k] == 0) {
            pulseIndexCluster = pulseIndex;
        }
        else {
            pulseIndexCluster = 1 - pulseIndex;
        }

        float travelRange = pixelsPerCluster[k];

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

            leadingPixel = static_cast<uint8_t>(travelRange * (sinOutput));

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

            leadingPixel = static_cast<uint8_t>(travelRange * (sinOutput));

        }
        else {
            leadingPixel = static_cast<uint8_t>(travelRange / 2 + travelRange * sin((pulseIndexCluster + phase[k]) * 2 * PI) / 2);
        }

        // endLastCluster is the end of the last Cluster
        uint8_t startCluster = 0;
        for (uint16_t l = 0; l < k; l++) {
            startCluster += pixelsPerCluster[l];
        }

        // center is the center of the traveling light
        uint8_t center = startCluster + leadingPixel;

        int startLED = (direction[k] == 1) ? startCluster : startCluster + leadingPixel;
        int endLED = (direction[k] == 1) ? startCluster + leadingPixel : startCluster + pixelsPerCluster[k] - 1;

        startLED = (startLED < startCluster) ? startCluster : startLED;
        endLED = (endLED >= (startCluster + pixelsPerCluster[k])) ? (startCluster + pixelsPerCluster[k] - 1) : endLED;

        strip->setRange(startLED, endLED, colorIndex, 1);

    }

}

void Pixels::travelingWave(uint8_t colorIndex, float numOfSines, int direction, float offset) {

    float Ts_ = Ts;

    if (direction == 1) {
        pulseIndex += ((Ts_ / 1000) * (BPM / 60)) / freqdiv; // Ts*BPS (s^1 * s^-1)
        pulseIndex = (pulseIndex > 1) ? pulseIndex - 1 : pulseIndex;
    }
    else if (direction == -1) {
        pulseIndex -= ((Ts_ / 1000) * (BPM / 60)) / freqdiv; // Ts*BPS (s^1 * s^-1)
        pulseIndex = (pulseIndex < 0) ? pulseIndex + 1 : pulseIndex;
    }

    uint16_t startingPixel = 0;

    float travelRange = totalPixels;
    travelRange = numOfSines/(travelRange);

    for (uint16_t k = startingPixel; k < startingPixel + totalPixels; k++) {

        float pixelValue = (0.5 + 0.5 * sin(((k - startingPixel) * travelRange + pulseIndex)*2*PI) - offset) / (1 - offset);
        pixelValue = (pixelValue < 0) ? 0 : pixelValue;
        
        strip->setColorsIndividualFixed(k, colorIndex, pixelValue);

    }


}


void Pixels::travelSides(uint8_t colorIndex, bool fade, float onValue, int direction, uint8_t numClusters_, uint8_t clusters_[], bool overlapColor, bool randomCluster) {

    // define clusters
    // numClusters_ defines the number of clusters defined in clusters_
    // clusters_ contains the number of consecuteve sides for the cluster
    uint8_t numClusters;
    uint8_t pixelsPerCluster[MAXSIDES_L];
    if (numClusters_ == 0) {
        numClusters = numSides;
        for (uint8_t k = 0; k < numClusters; k++) {
            pixelsPerCluster[k] = pixelsPerSide[k];
        }
    }
    else {
        numClusters = numClusters_;
        uint8_t sideIndex = 0;
        for (uint8_t k = 0; k < numClusters; k++) {
            pixelsPerCluster[k] = 0;
            for (uint8_t l = 0; l < clusters_[k]; l++) {
                pixelsPerCluster[k] += pixelsPerSide[sideIndex];
                sideIndex += 1;
            }
        }
    }

    float Ts_ = Ts;

    static uint8_t randomColorIndex = 1;

    pulseIndex += (Ts_ / 1000) * (BPM / 60) / freqdiv; // Ts*BPS (s^1 * s^-1)
    if (pulseIndex > 1) {
        pulseIndex -= 1;
        if (randomCluster) {
            sideIndex = rand() % numClusters;
            randomColorIndex = rand() % (strip->numOfColors - 1);
        }
        else {
            sideIndex += direction;
        }
    }

    // do not go to zero for overlapping color
    float dimValue;
    if (overlapColor) {
        dimValue = 0.55 + 0.45 * sin((pulseIndex - 0.25) * 2 * PI);
    }
    else {
        dimValue = 0.5 + 0.5 * sin((pulseIndex - 0.25) * 2 * PI);
    }

    // set direction and random color for overlap
    if (direction == 1) {
        if (sideIndex >= numClusters) {
            sideIndex = 0;
            randomColorIndex = rand() % (strip->numOfColors - 1);
        } 
    }
    else {
        if (sideIndex < 0) {
            sideIndex = numClusters - 1;
            randomColorIndex = rand() % (strip->numOfColors - 1);
        }
    }

    // only set all colors to zero if there is no overlap required
    if (overlapColor || randomCluster) {
        colorIndex = randomColorIndex;
    }

    if (!overlapColor){
        strip->setColorsAll(colorIndex, 0);
    }
    
    uint16_t pixelStart = 0;
    for (uint8_t k = 0; k < sideIndex; k++) {
        pixelStart += pixelsPerCluster[k];
    }
    uint16_t pixelEnd = 0;
    for (uint8_t k = 0; k < sideIndex+1; k++) {
        pixelEnd += pixelsPerCluster[k];
    }
    pixelEnd -= 1;

    if (fade == 0 && overlapColor == 0) {
        dimValue = (pulseIndex > onValue) ? 1 : 0;
    }

    //strip->setColorsAll(colorIndex, 0);
    strip->setRange(pixelStart, pixelEnd, colorIndex, dimValue);

}


// standard color with flashing pixels
void Pixels::flashingPixels(uint8_t mainColor, uint8_t flashColor, uint8_t flashChance) {

    float Ts_ = Ts;
    pulseIndex += ((Ts_ / 1000) * (BPM / 60)) / freqdiv; // Ts*BPS (s^1 * s^-1)
    
    if (pulseIndex > 1) {

        pulseIndex -= 1;

        strip->setColorsAll(mainColor);

        uint16_t maxSideIndex = 0;
        for (uint8_t k = 0; k < numSides; k++) {
            maxSideIndex += pixelsPerSide[k];
        }
        maxSideIndex -= 1;

        for (int k = 0; k <= maxSideIndex; k++) {
            bool flashOn = (rand() % 100) < flashChance; 
            if (flashOn) {
                strip->setColorsIndividualFixed(k, flashColor);
            }
        }
    }

}

// opening light (start in the cent go to both sides
void Pixels::fillBoth(uint8_t colorIndex, float startPos, float lightSize, uint8_t flashChance, bool randPos, uint8_t numClusters_, uint8_t clusters_[], bool inverse) {

    // define clusters
    // numClusters_ defines the number of clusters defined in clusters_
    // clusters_ contains the number of consecuteve sides for the cluster
    uint8_t numClusters;
    uint8_t pixelsPerCluster[MAXSIDES_L];
    if (numClusters_ == 0) {
        numClusters = numSides;
        for (uint8_t k = 0; k < numClusters; k++) {
            pixelsPerCluster[k] = pixelsPerSide[k];
        }
    }
    else {
        numClusters = numClusters_;
        uint8_t sideIndex = 0;
        for (uint8_t k = 0; k < numClusters; k++) {
            pixelsPerCluster[k] = 0;
            for (uint8_t l = 0; l < clusters_[k]; l++) {
                pixelsPerCluster[k] += pixelsPerSide[sideIndex];
                sideIndex += 1;
            }
        }
    }

    float Ts_ = Ts;
    // change for different unit
    static float startPositions[MAXSIDES_L] = { };
    static bool activateCluster[MAXSIDES_L] = { };

    pulseIndex += ((Ts_ / 1000) * (BPM / 60)) / 2 / freqdiv; // Ts*BPS (s^1 * s^-1)
    pulseIndex = pulseIndex > 1 ? pulseIndex - 1 : pulseIndex;
    
    if (pulseIndex > 1) {

        pulseIndex -= 1;

        if (randPos) {
            
            // get modulus of rand number and divide by 100 to get number between 0 and 1
            for (uint8_t k = 0; k < numClusters; k++) {
                // rand() gives errors, so use something else
                startPositions[k] = randomFloat();
            }
        }
        else {
            for (uint8_t k = 0; k < numClusters; k++) {
                startPositions[k] = startPos;
            }
        }
        
        if (flashChance >= 100) {
            for (uint8_t k = 0; k < numClusters; k++) {
                activateCluster[k] = 1;
            }
        }
        else {
            for (uint8_t k = 0; k < numClusters; k++) {
                activateCluster[k] = (rand() % 100) < flashChance; 
            }
        }
        
    }

    if (inverse) {
        strip->setColorsAll(colorIndex, 1);
    }
    else {
        strip->setColorsAll(colorIndex, 0);
    }

    for (uint16_t k = 0; k < numClusters; k++) {

        if (activateCluster[k]) {

            float travelRange = static_cast<float>(pixelsPerCluster[k]);

            float sinInput = pulseIndex;
            float sinOutput;
            // shifted sine between 0 and 1
            sinOutput = (sin(sinInput * 2 * PI - 0.5 * PI) + 1) / 2;

            uint16_t centerSingle = static_cast<uint16_t>(startPositions[k] * travelRange);


            uint16_t startCluster = 0;
            uint16_t center = 0;

            for (uint16_t l = 0; l < k; l++) {
                startCluster += pixelsPerCluster[l];
            }
            center = startCluster + centerSingle;

            int startLED = center - static_cast<uint16_t>((travelRange*lightSize / 2) * sinOutput) + 1;
            int endLED = center + static_cast<uint16_t>((travelRange*lightSize / 2) * sinOutput) - 1;

            startLED = (startLED < startCluster) ? startCluster : startLED;
            endLED = (endLED >= startCluster + pixelsPerCluster[k]) ? startCluster + pixelsPerCluster[k] - 1 : endLED;

            if (inverse) {
                strip->setRange(startLED, endLED, colorIndex, 0);
            }
            else {
                strip->setRange(startLED, endLED, colorIndex, 1);
            }


        }
                    
    }

}


// set the color
void Pixels::activateColor() {
    strip->setStrip();
};

// get random number
float Pixels::randomFloat() {

    uint8_t randnum = rand() % 100;
    return 0.1 + static_cast<float>(randnum) / 125;

}
