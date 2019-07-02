/*--------------------------------------------------------------------
  Arduino library based on Adafruit_Neomatrix but modified to work with SmartMatrix
  by Marc MERLIN <marc_soft@merlins.org>

  Original notice and license from Adafruit_Neomatrix:
  NeoMatrix is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation, either version 3 of
  the License, or (at your option) any later version.

  NeoMatrix is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with NeoMatrix.  If not, see
  <http://www.gnu.org/licenses/>.
  --------------------------------------------------------------------*/

#ifndef _FRAMEBUFFER_GFX_H_
#define _FRAMEBUFFER_GFX_H_

#if ARDUINO >= 100
 #include <Arduino.h>
#else
 #include <WProgram.h>
 #include <pins_arduino.h>
#endif
#include <Adafruit_GFX.h>
// Ideally this lib wouldn't require FastLED, but in order to be compatible
// with FastLED Matrix code, it's much easier to just use its CRGB definition
#include "FastLED.h"

// Matrix layout information is passed in the 'matrixType' parameter for
// each constructor (the parameter immediately following is the LED type
// from NeoPixel.h).

// These define the layout for a single 'unified' matrix (e.g. one made
// from NeoPixel strips, or a single NeoPixel shield), or for the pixels
// within each matrix of a tiled display (e.g. multiple NeoPixel shields).

#define NEO_MATRIX_TOP         0x00 // Pixel 0 is at top of matrix
#define NEO_MATRIX_BOTTOM      0x01 // Pixel 0 is at bottom of matrix
#define NEO_MATRIX_LEFT        0x00 // Pixel 0 is at left of matrix
#define NEO_MATRIX_RIGHT       0x02 // Pixel 0 is at right of matrix
#define NEO_MATRIX_CORNER      0x03 // Bitmask for pixel 0 matrix corner
#define NEO_MATRIX_ROWS        0x00 // Matrix is row major (horizontal)
#define NEO_MATRIX_COLUMNS     0x04 // Matrix is column major (vertical)
#define NEO_MATRIX_AXIS        0x04 // Bitmask for row/column layout
#define NEO_MATRIX_PROGRESSIVE 0x00 // Same pixel order across each line
#define NEO_MATRIX_ZIGZAG      0x08 // Pixel order reverses between lines
#define NEO_MATRIX_SEQUENCE    0x08 // Bitmask for pixel line order

// These apply only to tiled displays (multiple matrices):

#define NEO_TILE_TOP           0x00 // First tile is at top of matrix
#define NEO_TILE_BOTTOM        0x10 // First tile is at bottom of matrix
#define NEO_TILE_LEFT          0x00 // First tile is at left of matrix
#define NEO_TILE_RIGHT         0x20 // First tile is at right of matrix
#define NEO_TILE_CORNER        0x30 // Bitmask for first tile corner
#define NEO_TILE_ROWS          0x00 // Tiles ordered in rows
#define NEO_TILE_COLUMNS       0x40 // Tiles ordered in columns
#define NEO_TILE_AXIS          0x40 // Bitmask for tile H/V orientation
#define NEO_TILE_PROGRESSIVE   0x00 // Same tile order across each line
#define NEO_TILE_ZIGZAG        0x80 // Tile order reverses between lines
#define NEO_TILE_SEQUENCE      0x80 // Bitmask for tile line order

// used for show_free_mem
#ifdef ESP8266
    extern "C" { 
#include "user_interface.h"
}
#endif

class Framebuffer_GFX : public Adafruit_GFX {

 public:
  // pre-computed gamma table
  uint8_t gamma[256];

  Framebuffer_GFX(CRGB *, const uint16_t w, const uint16_t h, void (* showptr)());

  int XY(int16_t x, int16_t y); // compat with FastLED code, returns 1D offset
  void
    drawPixel(int16_t x, int16_t y, uint16_t color),
    drawPixel(int16_t x, int16_t y, uint32_t color),
    drawPixel(int16_t x, int16_t y, CRGB color),
    fillScreen(uint16_t color),
    setPassThruColor(CRGB c),
    setPassThruColor(uint32_t c),
    setPassThruColor(void),
    setRemapFunction(uint16_t (*fn)(uint16_t, uint16_t)),
    precal_gamma(float);

  static uint16_t Color(uint8_t r, uint8_t g, uint8_t b);
  static uint16_t Color24to16(uint32_t color);
  static uint32_t CRGBtoint32(CRGB color);

  void clear() { fillScreen(0); };
  void show() { if (_show) _show(); else 
    Serial.println("Cannot run show(), no function pointer, not inherited and shadowed"); };

  // This is implemented for FastLED in the superclass
  // For SmartMatrix, brightness is done outside this object
  void setBrightness(int b) { 
    b = b; // squelch unused warning
    Serial.println("Not Implemented in Framebuffer::GFX");
  };

  void begin();
  void newLedsPtr(CRGB *);

  static void show_free_mem(const char *pre=NULL) {
    if (pre) {
      Serial.print(pre);
      Serial.print(": ");
    }
#ifdef ESP8266
    Serial.print( F("Heap Memory Available: ") ); Serial.println(system_get_free_heap_size());
#endif
#ifdef ESP32
    Serial.print("Heap/32-bit Memory Available: ");
    Serial.print(heap_caps_get_free_size(0));
    Serial.print(" bytes total, ");
    Serial.print(heap_caps_get_largest_free_block(0));
    Serial.println(" bytes largest free block");

    Serial.print("8-bit/DMA Memory Available  : ");
    Serial.print(heap_caps_get_free_size(MALLOC_CAP_DMA));
    Serial.print(" bytes total, ");
    Serial.print(heap_caps_get_largest_free_block(MALLOC_CAP_DMA));
    Serial.println(" bytes largest free block");
#endif
   }


 protected:
  uint8_t type, tilesX, tilesY;
  uint16_t matrixWidth, matrixHeight;

  // Because SmartMatrix uses templates so heavily, its object cannot be passed to us
  // However the main function can create a show function that copies our data from _fb
  // into the SmartMatrix object, and pass that function to us by pointer.
  void (* _show)();
  CRGB *_fb;


 private:


  uint16_t (*remapFn)(uint16_t x, uint16_t y);
  uint32_t numpix;
  uint32_t passThruColor;
  boolean  passThruFlag = false;
};

#endif // _FRAMEBUFFER_GFX_H_
// vim:sts=2:sw=2
