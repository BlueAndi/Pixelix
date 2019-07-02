#ifndef config_h
#define config_h
#include <Adafruit_GFX.h>
#include <FastLED_NeoMatrix.h>
#include <FastLED.h>


//---------------------------------------------------------------------------- 
//
// Used by LEDMatrix
#define MATRIX_TILE_WIDTH   8 // width of EACH NEOPIXEL MATRIX (not total display)
#define MATRIX_TILE_HEIGHT  32 // height of each matrix
#define MATRIX_TILE_H       3  // number of matrices arranged horizontally
#define MATRIX_TILE_V       1  // number of matrices arranged vertically

// Used by NeoMatrix
#define mw (MATRIX_TILE_WIDTH *  MATRIX_TILE_H)
#define mh (MATRIX_TILE_HEIGHT * MATRIX_TILE_V)
#define NUMMATRIX (mw*mh)

// Compat for some other demos
#define NUM_LEDS NUMMATRIX 
#define MATRIX_HEIGHT mh
#define MATRIX_WIDTH mw

CRGB matrixleds[NUMMATRIX];

FastLED_NeoMatrix *matrix = new FastLED_NeoMatrix(matrixleds, MATRIX_TILE_WIDTH, MATRIX_TILE_HEIGHT, MATRIX_TILE_H, MATRIX_TILE_V, 
  NEO_MATRIX_TOP     + NEO_MATRIX_RIGHT +
    NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG + 
    NEO_TILE_TOP + NEO_TILE_LEFT +  NEO_TILE_PROGRESSIVE);

uint8_t matrix_brightness = 32;

#endif
