/*--------------------------------------------------------------------
  Arduino library based on Adafruit_Neomatrix but modified to work with FastLED
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

#ifndef _FASTLED_NEOMATRIX_H_
#define _FASTLED_NEOMATRIX_H_
#include "Framebuffer_GFX.h"

#if defined(ESP8266)
// If you get matrix flickering, modify platforms/esp/8266/clockless_esp8266.h 
// and platforms/esp/8266/clockless_block_esp8266.h to change WAIT_TIME to 20
//#pragma message "If you get matrix corruption, turn off FASTLED_ALLOW_INTERRUPTS"
//#pragma message "in this library, or modify WAIT_TIME in platforms/esp/8266/clockless_esp8266.h"
//#pragma message "(raise it from 5 to 20 or up to 50 if needed)"
// Or if you don't need interrupts, you can disable them here
//#define FASTLED_ALLOW_INTERRUPTS 0
#endif
#include <FastLED.h>

/* 
 * Ideally FastLED_NeoMatrix would multiple inherit from CFastLED too
 * I tried this, but on that path laid madness, apparent compiler bugs
 * and pain due to the unfortunate use of templates in FastLED, preventing
 * passing initalization arguments in the object since they need to be
 * hardcoded at compile time as template values :( -- merlin
 * See https://github.com/marcmerlin/FastLED_NeoMatrix/blob/cd739d471bbbe22336f281f1d1988aa7e7572340/FastLED_NeoMatrix.cpp#L92
 */
class FastLED_NeoMatrix : public Framebuffer_GFX {

public:
  // Constructor for single matrix:
  FastLED_NeoMatrix(CRGB *, uint8_t w, uint8_t h, 
    uint8_t matrixType = NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_ROWS);

  // Constructor for tiled matrices:
  FastLED_NeoMatrix(CRGB *, uint8_t matrixW, uint8_t matrixH, 
    uint8_t tX, uint8_t tY,
    uint8_t matrixType = NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_ROWS +
                         NEO_TILE_TOP + NEO_TILE_LEFT + NEO_TILE_ROWS);

  void show();
  void setBrightness(int b) { FastLED.setBrightness(b); };
};

#endif // _FASTLED_NEOMATRIX_H_
// vim:sts=2:sw=2
