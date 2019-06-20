// FastLED_NeoMatrix + LEDMatrix example
//
// https://github.com/marcmerlin/FastLED_NeoMatrix
// https://github.com/Jorgen-VikingGod/LEDMatrix
//
// By Marc MERLIN <marc_soft@merlins.org>
// License: BSD-2

#include "config.h"


#if defined(ESP32) or defined(ESP8266)
#define PIN 5
#else
#define PIN 13
#endif

// cLEDMatrix defines 
cLEDMatrix<-MATRIX_TILE_WIDTH, -MATRIX_TILE_HEIGHT, HORIZONTAL_ZIGZAG_MATRIX,
    MATRIX_TILE_H, MATRIX_TILE_V, HORIZONTAL_BLOCKS> ledmatrix;


// Normally we would define this:
//CRGB leds[NUMMATRIX];

// cLEDMatrix creates a FastLED array inside its object and we need to retrieve
// a pointer to its first element to act as a regular FastLED array, necessary
// for NeoMatrix and other operations that may work directly on the array like FadeAll.
CRGB *leds = ledmatrix[0];

FastLED_NeoMatrix *matrix = new FastLED_NeoMatrix(leds, MATRIX_TILE_WIDTH, MATRIX_TILE_HEIGHT,
MATRIX_TILE_H, MATRIX_TILE_V,
  NEO_MATRIX_TOP     + NEO_MATRIX_RIGHT +
    NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG +
    NEO_TILE_TOP + NEO_TILE_LEFT +  NEO_TILE_PROGRESSIVE);

void matrix_clear() {
    // FastLED.clear does not work properly with multiple matrices connected via parallel inputs
    // on ESP8266 (not sure about other chips).
    memset(leds, 0, NUMMATRIX*3);
}


// This could also be defined as matrix->color(255,0,0) but those defines
// are meant to work for adafruit_gfx backends that are lacking color()
#define LED_BLACK		0

#define LED_RED_VERYLOW 	(3 <<  11)
#define LED_RED_LOW 		(7 <<  11)
#define LED_RED_MEDIUM 		(15 << 11)
#define LED_RED_HIGH 		(31 << 11)

#define LED_GREEN_VERYLOW	(1 <<  5)
#define LED_GREEN_LOW 		(15 << 5)
#define LED_GREEN_MEDIUM 	(31 << 5)
#define LED_GREEN_HIGH 		(63 << 5)

#define LED_BLUE_VERYLOW	3
#define LED_BLUE_LOW 		7
#define LED_BLUE_MEDIUM 	15
#define LED_BLUE_HIGH 		31

#define LED_ORANGE_VERYLOW	(LED_RED_VERYLOW + LED_GREEN_VERYLOW)
#define LED_ORANGE_LOW		(LED_RED_LOW     + LED_GREEN_LOW)
#define LED_ORANGE_MEDIUM	(LED_RED_MEDIUM  + LED_GREEN_MEDIUM)
#define LED_ORANGE_HIGH		(LED_RED_HIGH    + LED_GREEN_HIGH)

#define LED_PURPLE_VERYLOW	(LED_RED_VERYLOW + LED_BLUE_VERYLOW)
#define LED_PURPLE_LOW		(LED_RED_LOW     + LED_BLUE_LOW)
#define LED_PURPLE_MEDIUM	(LED_RED_MEDIUM  + LED_BLUE_MEDIUM)
#define LED_PURPLE_HIGH		(LED_RED_HIGH    + LED_BLUE_HIGH)

#define LED_CYAN_VERYLOW	(LED_GREEN_VERYLOW + LED_BLUE_VERYLOW)
#define LED_CYAN_LOW		(LED_GREEN_LOW     + LED_BLUE_LOW)
#define LED_CYAN_MEDIUM		(LED_GREEN_MEDIUM  + LED_BLUE_MEDIUM)
#define LED_CYAN_HIGH		(LED_GREEN_HIGH    + LED_BLUE_HIGH)

#define LED_WHITE_VERYLOW	(LED_RED_VERYLOW + LED_GREEN_VERYLOW + LED_BLUE_VERYLOW)
#define LED_WHITE_LOW		(LED_RED_LOW     + LED_GREEN_LOW     + LED_BLUE_LOW)
#define LED_WHITE_MEDIUM	(LED_RED_MEDIUM  + LED_GREEN_MEDIUM  + LED_BLUE_MEDIUM)
#define LED_WHITE_HIGH		(LED_RED_HIGH    + LED_GREEN_HIGH    + LED_BLUE_HIGH)

// In a case of a tile of neomatrices, this test is helpful to make sure that the
// pixels are all in sequence (to check your wiring order and the tile options you
// gave to the constructor).
void count_pixels() {
    matrix_clear();
    for (uint16_t i=0; i<mh; i++) {
	for (uint16_t j=0; j<mw; j++) {
	    matrix->drawPixel(j, i, i%3==0?LED_BLUE_HIGH:i%3==1?LED_RED_HIGH:LED_GREEN_HIGH);
	    // depending on the matrix size, it's too slow to display each pixel, so
	    // make the scan init faster. This will however be too fast on a small matrix.
	    #ifdef ESP8266
	    if (!(j%3)) matrix->show();
	    yield(); // reset watchdog timer
	    #elif ESP32
	    delay(1);
	    matrix->show();
	    #else
	    matrix->show();
	    #endif
	}
    }
}

void display_lines() {
    matrix_clear();

    // 4 levels of crossing red lines.
    matrix->drawLine(0,mh/2-2, mw-1,2, LED_RED_VERYLOW);
    matrix->drawLine(0,mh/2-1, mw-1,3, LED_RED_LOW);
    matrix->drawLine(0,mh/2,   mw-1,mh/2, LED_RED_MEDIUM);
    matrix->drawLine(0,mh/2+1, mw-1,mh/2+1, LED_RED_HIGH);

    // 4 levels of crossing green lines.
    matrix->drawLine(mw/2-2, 0, mw/2-2, mh-1, LED_GREEN_VERYLOW);
    matrix->drawLine(mw/2-1, 0, mw/2-1, mh-1, LED_GREEN_LOW);
    matrix->drawLine(mw/2+0, 0, mw/2+0, mh-1, LED_GREEN_MEDIUM);
    matrix->drawLine(mw/2+1, 0, mw/2+1, mh-1, LED_GREEN_HIGH);

    // Diagonal blue line.
    matrix->drawLine(0,0, mw-1,mh-1, LED_BLUE_HIGH);
    matrix->drawLine(0,mh-1, mw-1,0, LED_ORANGE_MEDIUM);
    matrix->show();
}

void display_boxes() {
    matrix_clear();
    matrix->drawRect(0,0, mw,mh, LED_BLUE_HIGH);
    matrix->drawRect(1,1, mw-2,mh-2, LED_GREEN_MEDIUM);
    matrix->fillRect(2,2, mw-4,mh-4, LED_RED_HIGH);
    matrix->fillRect(3,3, mw-6,mh-6, LED_ORANGE_MEDIUM);
    matrix->show();
}

void display_circles() {
    matrix_clear();
    matrix->drawCircle(mw/2,mh/2, 2, LED_RED_MEDIUM);
    matrix->drawCircle(mw/2-1-min(mw,mh)/8, mh/2-1-min(mw,mh)/8, min(mw,mh)/4, LED_BLUE_HIGH);
    matrix->drawCircle(mw/2+1+min(mw,mh)/8, mh/2+1+min(mw,mh)/8, min(mw,mh)/4-1, LED_ORANGE_MEDIUM);
    matrix->drawCircle(1,mh-2, 1, LED_GREEN_LOW);
    matrix->drawCircle(mw-2,1, 1, LED_GREEN_HIGH);
    if (min(mw,mh)>12) matrix->drawCircle(mw/2-1, mh/2-1, min(mh/2-1,mw/2-1), LED_CYAN_HIGH);
    matrix->show();
}


void loop() {
#if 0
    Serial.println("Count pixels");
    count_pixels();
    Serial.println("Count pixels done");
    delay(1000);
#endif

    Serial.println("Use LEDMatrix to display a flag");
    flag();
    delay(3000);
    matrix_clear();

    Serial.println("Back to NeoMatrix/GFX to Display lines, boxes and circles");
    display_lines();
    delay(3000);

    display_boxes();
    delay(3000);

    display_circles();
    delay(3000);
    matrix_clear();


    Serial.println("Demo loop done, starting over");
}

void setup() {
    // Normal output
    FastLED.addLeds<NEOPIXEL,PIN>(leds, NUMMATRIX).setCorrection(TypicalLEDStrip);
    // Parallel output on 3 lines (ESP8266)
    //FastLED.addLeds<WS2811_PORTA,3>(leds, NUMMATRIX/3).setCorrection(TypicalLEDStrip);
    // Time for serial port to work?
    delay(1000);
    Serial.begin(115200);
    Serial.print("Matrix Size: ");
    Serial.print(mw);
    Serial.print(" ");
    Serial.println(mh);
    matrix->begin();
    matrix->setTextWrap(false);
    matrix->setBrightness(BRIGHTNESS);
    // Mix in an init of LEDMatrix
    sprite_setup();
    delay(2000);
    Serial.println("If the code crashes here, decrease the brightness or turn off the all white display below");
    // Test full bright of all LEDs. If brightness is too high
    // for your current limit (i.e. USB), decrease it.
#ifndef DISABLE_WHITE
    matrix->fillScreen(LED_WHITE_HIGH);
    matrix->show();
    Serial.println("First matrix->show did not crash/hang, trying clear");
    delay(3000);
    matrix_clear();
    Serial.println("First matrix_clear done");
#endif
}

// vim:sts=4:sw=4
