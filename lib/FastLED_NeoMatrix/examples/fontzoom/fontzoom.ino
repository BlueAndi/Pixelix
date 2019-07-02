// By Marc MERLIN <marc_soft@merlins.org>
// License: Apache v2.0
//

#include <Adafruit_GFX.h>
#include <FastLED_NeoMatrix.h>
#include <FastLED.h>
#include "fonts.h"

#define MATRIXPIN D6

#define MATRIX_TILE_WIDTH   8 // width of EACH NEOPIXEL MATRIX (not total display)
#define MATRIX_TILE_HEIGHT  32 // height of each matrix
#define MATRIX_TILE_H       3  // number of matrices arranged horizontally
#define MATRIX_TILE_V       1  // number of matrices arranged vertically

// Used by NeoMatrix
#define mw (MATRIX_TILE_WIDTH *  MATRIX_TILE_H)
#define mh (MATRIX_TILE_HEIGHT * MATRIX_TILE_V)
#define NUMMATRIX (mw*mh)

uint8_t matrix_brightness = 32;

CRGB matrixleds[NUMMATRIX];

FastLED_NeoMatrix *matrix = new FastLED_NeoMatrix(matrixleds, MATRIX_TILE_WIDTH, MATRIX_TILE_HEIGHT, MATRIX_TILE_H, MATRIX_TILE_V, 
  NEO_MATRIX_TOP     + NEO_MATRIX_RIGHT +
    NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG + 
    NEO_TILE_TOP + NEO_TILE_LEFT +  NEO_TILE_PROGRESSIVE);

bool matrix_reset_demo = 1;
int8_t matrix_loop = -1;

// ---------------------------------------------------------------------------
// Shared functions
// ---------------------------------------------------------------------------

uint16_t Color24toColor16(uint32_t color) {
  return ((uint16_t)(((color & 0xFF0000) >> 16) & 0xF8) << 8) |
         ((uint16_t)(((color & 0x00FF00) >>  8) & 0xFC) << 3) |
                    (((color & 0x0000FF) >>  0)         >> 3);
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
    uint32_t wheel=0;

    // Serial.print(WheelPos);
    WheelPos = 255 - WheelPos;
    if (WheelPos < 85) {
	wheel = (((uint32_t)(255 - WheelPos * 3)) << 16) + (WheelPos * 3);
    }
    if (!wheel && WheelPos < 170) {
	WheelPos -= 85;
	wheel = (((uint32_t)(WheelPos * 3)) << 8) + (255 - WheelPos * 3);
    }
    if (!wheel) {
	WheelPos -= 170;
	wheel = (((uint32_t)(WheelPos * 3)) << 16) + (((uint32_t)(255 - WheelPos * 3)) << 8);
    }
    // Serial.print(" -> ");
    // Serial.println(wheel, HEX);
    return (wheel);
}

// ---------------------------------------------------------------------------
// Matrix Code
// ---------------------------------------------------------------------------

// type 0 = up, type 1 = up and down
// The code is complicated looking, but the state machine is so that
// you can call this function to do a frame update and then switch to 
// other work
// There are some magic numbers I had to hand tune for a 24x32 array.
// You'll have to adjust for your own array size
uint8_t font_zoom(uint8_t zoom_type, uint8_t speed) {
    static uint16_t state;
    static uint16_t direction;
    static uint16_t size;
    static uint8_t l;
    static int16_t faster = 0;
    static bool dont_exit;
    static uint16_t delayframe;
    char letters[] = { 'H', 'E', 'L', 'l', 'O' };
    bool done = 0;
    uint8_t repeat = 3;

    if (matrix_reset_demo == 1) {
	matrix_reset_demo = 0;
	state = 1;
	direction = 1;
	size = 3;
	l = 0;
	if (matrix_loop == -1) { dont_exit = 1; delayframe = 2; faster = 0; };
    }

    matrix->setTextSize(1);

    if (--delayframe) {
	// reset how long a frame is shown before we switch to the next one
	// Serial.println("delayed frame");
	matrix->show(); // make sure we still run at the same speed.
	return repeat;
    }
    delayframe = max((speed / 10) - faster , 1);
    // before exiting, we run the full delay to show the last frame long enough
    if (dont_exit == 0) { dont_exit = 1; return 0; }
    if (direction == 1) {
	int8_t offset = 0; // adjust some letters left or right as needed
	if (letters[l] == 'H') offset = -3 * size/15;
	if (letters[l] == 'O') offset = -3 * size/15;
	if (letters[l] == 'l') offset = +5 * size/15;

	uint16_t txtcolor = Color24toColor16(Wheel(map(letters[l], '0', 'Z', 0, 255)));
	matrix->setTextColor(txtcolor); 

	matrix->clear();
	matrix->setFont( &Century_Schoolbook_L_Bold[size] );
	matrix->setCursor(10-size*0.55+offset, 17+size*0.75);
	matrix->print(letters[l]);
	if (size<18) size++; 
	else if (zoom_type == 0) { done = 1; delayframe = max((speed - faster*10) * 1, 3); } 
	     else direction = 2;

    } else if (zoom_type == 1) {
	int8_t offset = 0; // adjust some letters left or right as needed
	uint16_t txtcolor = Color24toColor16(Wheel(map(letters[l], '0', 'Z', 255, 0)));
	if (letters[l] == 'H') offset = -3 * size/15;
	if (letters[l] == 'O') offset = -3 * size/15;
	if (letters[l] == 'l') offset = +5 * size/15;

	matrix->setTextColor(txtcolor); 
	matrix->clear();
	matrix->setFont( &Century_Schoolbook_L_Bold[size] );
	matrix->setCursor(10-size*0.55+offset, 17+size*0.75);
	matrix->print(letters[l]);
	if (size>3) size--; else { done = 1; direction = 1; delayframe = max((speed-faster*10)/2, 3); };
    }

    matrix->show();
    //Serial.println("done?");
    if (! done) return repeat;
    direction = 1;
    size = 3;
    //Serial.println("more letters?");
    if (++l < sizeof(letters)) return repeat;
    l = 0;
    //Serial.println("reverse pattern?");
    if (zoom_type == 1 && direction == 2) return repeat;

    //Serial.println("Done with font animation");
    faster++;
    matrix_reset_demo = 1;
    dont_exit =  0;
    // Serial.print("delayframe on last letter ");
    // Serial.println(delayframe);
    // After showing the last letter, pause longer 
    // unless it's a zoom in zoom out.
    if (zoom_type == 0) delayframe *= 5; else delayframe *= 3;
    return repeat;
}


void loop() {
    uint8_t ret;
    static uint8_t cnt = 0;

    delay(5);
    // this code is meant to be run at some interval and keep its on
    // state while you do something else.
    
    if (cnt % 2) {
	ret = font_zoom(1, 25);
    } else {
	ret = font_zoom(0, 30);
    }
    if (matrix_loop == -1) matrix_loop = ret;
    if (ret) return;
    Serial.print("Finished run #");
    Serial.println(matrix_loop);
    if (matrix_loop-- > 0) return;
    Serial.println("Animation loop done, switching to other demo");
    cnt++;
    //delay(1000);
    matrix_reset_demo = 1;
}


void setup() {
    delay(1000);
    Serial.begin(115200);

    // Init Matrix
    // Serialized, 768 pixels takes 26 seconds for 1000 updates or 26ms per refresh
    // FastLED.addLeds<NEOPIXEL,MATRIXPIN>(matrixleds, NUMMATRIX).setCorrection(TypicalLEDStrip);
    // https://github.com/FastLED/FastLED/wiki/Parallel-Output
    // WS2811_PORTA - pins 12, 13, 14 and 15 or pins 6,7,5 and 8 on the NodeMCU
    // This is much faster 1000 updates in 10sec
    //FastLED.addLeds<NEOPIXEL,PIN>(leds, NUMMATRIX); 
    FastLED.addLeds<WS2811_PORTA,3>(matrixleds, NUMMATRIX/3).setCorrection(TypicalLEDStrip);
    Serial.print("Matrix Size: ");
    Serial.print(mw);
    Serial.print(" ");
    Serial.println(mh);
    matrix->begin();
    matrix->setBrightness(matrix_brightness);
    matrix->setTextWrap(false);
}

// vim:sts=4:sw=4
