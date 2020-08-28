#include <FastLED.h>
/*
    Yuriy Movchan 28/08/2020

    Healight color calculator and control
*/

// Table for rainbow + white
//   |    0    |    1    |    2    |    3    |     4   |    5    |      6    |    7    | > 7 (last value)
// R | 0...255 | 255     | 255...0 | 0       | 0       | 0...255 | 255       | 0...255 | 0
// G | 0       | 0...255 | 255     | 255     | 255...0 | 0       | 0         | 0...255 | 0
// B | 0       | 0       | 0       | 0...255 | 255     | 255     | 255...128 | 0...255 | 0

//#define HEADLIGHT_DEBUG 1

#define HEADLIGHT_PIXEL_COUNT 8                // We use 2 led strips connected in parallel
#define HEADLIGHT_PIXEL_PIN 6

CRGB leds[HEADLIGHT_PIXEL_COUNT];

#define RC_COLOR_MIN 1088                      // Minimum color PWM
#define RC_COLOR_MAX 1928                      // Maximum color PWM

const int INPUT_COLOR_VALUE_RANGE = RC_COLOR_MAX - RC_COLOR_MIN;                 // 840
const int INPUT_COLOR_RANGE_COUNT = 8;                                           // 8 is needed for table above
const int INPUT_COLOR_RANGE = INPUT_COLOR_VALUE_RANGE / INPUT_COLOR_RANGE_COUNT; // 105

// Frequnly used colors
CRGB redColor(255, 0, 0);
CRGB greenColor(0, 255, 0);
CRGB blackColor(0, 0, 0);
CRGB white50Color(127, 127, 127);
CRGB white100Color(255, 255, 255);

CRGB caclulatedColor;
CRGB currentColor;
boolean colorCalculated;

void configureHeadlight() {
  colorCalculated = false;

  FastLED.addLeds<NEOPIXEL, HEADLIGHT_PIXEL_PIN>(leds, HEADLIGHT_PIXEL_COUNT);  // GRB ordering is assumed

  setHeadlightColor(blackColor);
}

void setHeadlightWhite() {
  setHeadlightColor(white100Color);
}

void setHeadlightBlack() {
  setHeadlightColor(blackColor);
}

void setHeadlightRed() {
  setHeadlightColor(redColor);
}

void setHeadlightGreen() {
  setHeadlightColor(greenColor);
}

void setHeadlightColor(CRGB color) {
  if (currentColor != color) {
    for (int i = 0; i < HEADLIGHT_PIXEL_COUNT; i++) {
      leds[i] = color;
    }
    FastLED.show(); // This sends the updated pixels color to the hardware
  
    // Store new color
    currentColor = color;
  }
}

void calculateHeadlightColor(int inputValue) {
  uint16_t mappedValue, range, value;
  uint8_t mappedColor;
  uint8_t r, g, b;
  
  colorCalculated = false;

  r = g = b = 0;

  // Make sure that intput value belong to right range
  inputValue = constrain(inputValue, RC_COLOR_MIN, RC_COLOR_MAX);

  // Determine range
  mappedValue = inputValue - RC_COLOR_MIN;
  range = mappedValue / INPUT_COLOR_RANGE;

  // Calculate value in range from 0 .. INPUT_COLOR_RANGE - 1
  value = mappedValue - range * INPUT_COLOR_RANGE;

  mappedColor = map(value, 0, INPUT_COLOR_RANGE, 0, 255);

  if (range == 0) {
    r = mappedColor;
  } else if (range == 1) {
    r = 255;
    g = mappedColor;
  } else if (range == 2) {
    r = 255 - mappedColor;
    g = 255;
  } else if (range == 3) {
    g = 255;
    b =  mappedColor;
  } else if (range == 4) {
    g = 255 - mappedColor;
    b =  255;
  } else if (range == 5) {
    r =  mappedColor;
    b =  255;
  } else if (range == 6) {
    r =  255;
    b =  map(value, 0, INPUT_COLOR_RANGE, 255, 128); // Avoid cycle to red again
  } else if (range == 7) {
    // White color
    r =  mappedColor;
    g =  mappedColor;
    b =  mappedColor;
  } else {
    // Last value in range should be white
    r = 255;
    g = 255;
    b = 255;
  }

#ifdef HEADLIGHT_DEBUG
  Serial.print(mappedColor);
  Serial.print(" | ");
  Serial.print(range);
  Serial.print(" | ");
  Serial.print(r);
  Serial.print(" * ");
  Serial.print(g);
  Serial.print(" * ");
  Serial.print(b);
  Serial.print("\n");
#endif

  colorCalculated = true;
  caclulatedColor = CRGB(r, g, b);
}

void setCalculatedHeadlightColor() {
  if (colorCalculated) {
    setHeadlightColor(caclulatedColor);
    colorCalculated = false;
  }
}
