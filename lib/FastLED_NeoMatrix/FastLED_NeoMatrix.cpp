/*-------------------------------------------------------------------------
  Arduino library based on Adafruit_Neomatrix but modified to work with FastLED
  by Marc MERLIN <marc_soft@merlins.org>

  Original notice and license from Adafruit_Neomatrix:
  ------------------------------------------------------------------------
  Arduino library to control single and tiled matrices of WS2811- and
  WS2812-based RGB LED devices such as the Adafruit NeoPixel Shield or
  displays assembled from NeoPixel strips, making them compatible with
  the Adafruit_GFX graphics library.  Requires both the FastLED_NeoPixel
  and Adafruit_GFX libraries.

  Written by Phil Burgess / Paint Your Dragon for Adafruit Industries.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing products
  from Adafruit!

  -------------------------------------------------------------------------
  This file is part of the Adafruit NeoMatrix library.

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
  -------------------------------------------------------------------------*/

#include <Adafruit_GFX.h>
#include <FastLED_NeoMatrix.h>

// Constructor for single matrix:
FastLED_NeoMatrix::FastLED_NeoMatrix(CRGB *leds, uint8_t w, uint8_t h, uint8_t matrixType): 
  Framebuffer_GFX(leds, w, h, NULL) {
    type = matrixType;
    tilesX = 0;
    tilesY = 0;
  }

// Constructor for tiled matrices:
FastLED_NeoMatrix::FastLED_NeoMatrix(CRGB *leds, uint8_t mW, uint8_t mH, 
    uint8_t tX, uint8_t tY, uint8_t matrixType) :
  Framebuffer_GFX(leds, mW * tX, mH * tY, NULL) {
    matrixWidth = mW;
    matrixHeight = mH;
    type = matrixType;
    tilesX = tX;
    tilesY = tY;
}

void FastLED_NeoMatrix::show() {
  if (_show) { _show(); return; };
  #ifdef ESP8266
// Disable watchdog interrupt so that it does not trigger in the middle of
// updates. and break timing of pixels, causing random corruption on interval
// https://github.com/esp8266/Arduino/issues/34
    ESP.wdtDisable();
#endif
    FastLED.show();
#ifdef ESP8266
    ESP.wdtEnable(1000);
#endif
}


// vim:sts=2:sw=2
