/*
 * Aurora: https://github.com/pixelmatix/aurora
 * Copyright (c) 2014 Jason Coon
 *
 * Portions of this code are adapted from Craig Lindley's LightAppliance: https://github.com/CraigLindley/LightAppliance
 * Copyright (c) 2014 Craig A. Lindley
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef GifPlayer_H
#define GifPlayer_H

#include "config.h"

#define DEBUG 0

#include "pixeltypes.h"

class GifPlayer {
  public:

    // Error codes
#define ERROR_NONE		0
#define ERROR_FILEOPEN		1
#define ERROR_FILENOTGIF	2
#define ERROR_BADGIFFORMAT      3
#define ERROR_UNKNOWNCONTROLEXT 4
#define ERROR_FINISHED       	5

  private:

#define GIFHDRTAGNORM   "GIF87a"  // tag in valid GIF file
#define GIFHDRTAGNORM1  "GIF89a"  // tag in valid GIF file
#define GIFHDRSIZE 6

    // Global GIF specific definitions
#define COLORTBLFLAG    0x80
#define INTERLACEFLAG   0x40
#define TRANSPARENTFLAG 0x01

#define NO_TRANSPARENT_INDEX -1

    // Disposal methods
#define DISPOSAL_NONE       0
#define DISPOSAL_LEAVE      1
#define DISPOSAL_BACKGROUND 2
#define DISPOSAL_RESTORE    3

    // RGB data structure
    typedef struct {
      byte Red;
      byte Green;
      byte Blue;
    }
    _RGB;

    // Logical screen descriptor attributes
    int lsdWidth;
    int lsdHeight;
    int lsdPackedField;
    int lsdAspectRatio;
    int lsdBackgroundIndex;

    // Table based image attributes
    int tbiImageX;
    int tbiImageY;
    int tbiWidth;
    int tbiHeight;
    int tbiPackedBits;
    boolean tbiInterlaced;

    int frameDelay;
    int transparentColorIndex;
    int prevBackgroundIndex;
    int prevDisposalMethod;
    int disposalMethod;
    int lzwCodeSize;
    boolean keyFrame;
    int rectX;
    int rectY;
    int rectWidth;
    int rectHeight;

    int colorCount;
    _RGB gifPalette[256];

    byte lzwImageData[1280];
    char tempBuffer[260];

    File file;

    byte imageData[1024];
    byte imageDataBU[1024];

    // Backup the read stream by n bytes
    void backUpStream(int n) {
      file.seek(file.position() - n);
    }

    // Read a file byte
    int readByte() {

      int b = file.read();
#if DEBUG == 1
      if (b == -1) {
        Serial.println(F("Read error or EOF occurred"));
      }
#endif
      return b;
    }

    // Read a file word
    int readWord() {

      int b0 = readByte();
      int b1 = readByte();
      return (b1 << 8) | b0;
    }

    // Read the specified number of bytes into the specified buffer
    int readIntoBuffer(void *buffer, int numberOfBytes) {

      //int result = file.read(buffer, numberOfBytes);
      int result = file.read((uint8_t *)buffer, (size_t)numberOfBytes);
#if DEBUG == 1
      if (result == -1) {
        Serial.println(F("Read error or EOF occurred"));
      }
#endif
      return result;
    }

    // Fill a portion of imageData buffer with a color index
    void fillImageDataRect(byte colorIndex, int x, int y, int width, int height) {

      int yOffset;

      for (int yy = y; yy < height + y; yy++) {
        yOffset = yy * MATRIX_WIDTH;
        for (int xx = x; xx < width + x; xx++) {
          imageData[yOffset + xx] = colorIndex;
        }
      }
    }

    // Fill entire imageData buffer with a color index
    void fillImageData(byte colorIndex) {

      memset(imageData, colorIndex, sizeof(imageData));
    }

    // Copy image data in rect from a src to a dst
    void copyImageDataRect(byte *src, byte *dst, int x, int y, int width, int height) {

      int yOffset, offset;

      for (int yy = y; yy < height + y; yy++) {
        yOffset = yy * MATRIX_WIDTH;
        for (int xx = x; xx < width + x; xx++) {
          offset = yOffset + xx;
          dst[offset] = src[offset];
        }
      }
    }

    // Parse plain text extension and dispose of it
    void parsePlainTextExtension() {

#if DEBUG == 1
      Serial.println(F("\nProcessing Plain Text Extension"));
#endif
      // Read plain text header length
      byte len = readByte();

      // Consume plain text header data
      readIntoBuffer(tempBuffer, len);

      // Consume the plain text data in blocks
      len = readByte();
      while (len != 0) {
        readIntoBuffer(tempBuffer, len);
        len = readByte();
      }
    }

    // Parse a graphic control extension
    void parseGraphicControlExtension() {

#if DEBUG == 1
      Serial.println(F("\nProcessing Graphic Control Extension"));
      int len = readByte();	// Check length
      if (len != 4) {
        Serial.println(F("Bad graphic control extension"));
      }
#else
      readByte();
#endif

      int packedBits = readByte();
      frameDelay = readWord();
      transparentColorIndex = readByte();

      if ((packedBits & TRANSPARENTFLAG) == 0) {
        // Indicate no transparent index
        transparentColorIndex = NO_TRANSPARENT_INDEX;
      }
      disposalMethod = (packedBits >> 2) & 7;
      if (disposalMethod > 3) {
        disposalMethod = 0;
#if DEBUG == 1
        Serial.println(F("Invalid disposal value"));
#endif
      }

      readByte();	// Toss block end

#if DEBUG == 1
      Serial.print(F("PacketBits: "));
      Serial.println(packedBits, HEX);
      Serial.print(F("Frame delay: "));
      Serial.println(frameDelay);
      Serial.print(F("transparentColorIndex: "));
      Serial.println(transparentColorIndex);
      Serial.print(F("disposalMethod: "));
      Serial.println(disposalMethod);
#endif
    }

    // Parse application extension
    void parseApplicationExtension() {

      memset(tempBuffer, 0, sizeof(tempBuffer));

#if DEBUG == 1
      Serial.println(F("\nProcessing Application Extension"));
#endif

      // Read block length
      byte len = readByte();

      // Read app data
      readIntoBuffer(tempBuffer, len);

#if DEBUG == 1
      // Conditionally display the application extension string
      if (strlen(tempBuffer) != 0) {
        Serial.print(F("Application Extension: "));
        Serial.println(tempBuffer);
      }
#endif

      // Consume any additional app data
      len = readByte();
      while (len != 0) {
        readIntoBuffer(tempBuffer, len);
        len = readByte();
      }
    }

    // Parse comment extension
    void parseCommentExtension() {

#if DEBUG == 1
      Serial.println(F("\nProcessing Comment Extension"));
#endif

      // Read block length
      byte len = readByte();
      while (len != 0) {
        // Clear buffer
        memset(tempBuffer, 0, sizeof(tempBuffer));

        // Read len bytes into buffer
        readIntoBuffer(tempBuffer, len);

#if DEBUG == 1
        // Display the comment extension string
        if (strlen(tempBuffer) != 0) {
          Serial.print(F("Comment Extension: "));
          Serial.println(tempBuffer);
        }
#endif
        // Read the new block length
        len = readByte();
      }
    }

    // Parse file terminator
    int parseGIFFileTerminator() {

#if DEBUG == 1
      Serial.println(F("\nProcessing file terminator"));
#endif

      byte b = readByte();
      if (b != 0x3B) {

#if DEBUG == 1
        Serial.print(F("Terminator byte: "));
        Serial.println(b, HEX);
        Serial.println(F("Bad GIF file format - Bad terminator"));
#endif
        return ERROR_BADGIFFORMAT;
      }
      else	{
        return ERROR_NONE;
      }
    }

    // Parse table based image data
    unsigned long parseTableBasedImage() {

#if DEBUG == 1
      Serial.println(F("\nProcessing Table Based Image Descriptor"));
#endif

      // Parse image descriptor
      tbiImageX = readWord();
      tbiImageY = readWord();
      tbiWidth = readWord();
      tbiHeight = readWord();
      tbiPackedBits = readByte();

#if DEBUG == 1
      Serial.print(F("tbiImageX: "));
      Serial.println(tbiImageX);
      Serial.print(F("tbiImageY: "));
      Serial.println(tbiImageY);
      Serial.print(F("tbiWidth: "));
      Serial.println(tbiWidth);
      Serial.print(F("tbiHeight: "));
      Serial.println(tbiHeight);
      Serial.print(F("PackedBits: "));
      Serial.println(tbiPackedBits, HEX);
#endif

      // Is this image interlaced ?
      tbiInterlaced = ((tbiPackedBits & INTERLACEFLAG) != 0);

#if DEBUG == 1
      Serial.print(F("Image interlaced: "));
      Serial.println((tbiInterlaced != 0) ? "Yes" : "No");
#endif

      // Does this image have a local color table ?
      boolean localColorTable = ((tbiPackedBits & COLORTBLFLAG) != 0);

      if (localColorTable) {
        int colorBits = ((tbiPackedBits & 7) + 1);
        colorCount = 1 << colorBits;

#if DEBUG == 1
        Serial.print(F("Local color table with "));
        Serial.print(colorCount);
        Serial.println(F(" colors present"));
#endif
        // Read colors into palette
        int colorTableBytes = sizeof(_RGB) * colorCount;
        readIntoBuffer(gifPalette, colorTableBytes);
      }

      // One time initialization of imageData before first frame
      if (keyFrame) {
        if (transparentColorIndex == NO_TRANSPARENT_INDEX) {
          fillImageData(lsdBackgroundIndex);
        }
        else    {
          fillImageData(transparentColorIndex);
        }
        keyFrame = false;

        rectX = 0;
        rectY = 0;
        rectWidth = MATRIX_WIDTH;
        rectHeight = MATRIX_HEIGHT;
      }
      // Don't clear matrix screen for these disposal methods
      if ((prevDisposalMethod != DISPOSAL_NONE) && (prevDisposalMethod != DISPOSAL_LEAVE)) {
        //backgroundLayer.fillScreen({ 0, 0, 0 });
	matrix->clear();
      }

      // Process previous disposal method
      if (prevDisposalMethod == DISPOSAL_BACKGROUND) {
        // Fill portion of imageData with previous background color
        fillImageDataRect(prevBackgroundIndex, rectX, rectY, rectWidth, rectHeight);
      }
      else if (prevDisposalMethod == DISPOSAL_RESTORE) {
        copyImageDataRect(imageDataBU, imageData, rectX, rectY, rectWidth, rectHeight);
      }

      // Save disposal method for this frame for next time
      prevDisposalMethod = disposalMethod;

      if (disposalMethod != DISPOSAL_NONE) {
        // Save dimensions of this frame
        rectX = tbiImageX;
        rectY = tbiImageY;
        rectWidth = tbiWidth;
        rectHeight = tbiHeight;

        if (disposalMethod == DISPOSAL_BACKGROUND) {
          if (transparentColorIndex != NO_TRANSPARENT_INDEX) {
            prevBackgroundIndex = transparentColorIndex;
          }
          else    {
            prevBackgroundIndex = lsdBackgroundIndex;
          }
        }
        else if (disposalMethod == DISPOSAL_RESTORE) {
          copyImageDataRect(imageData, imageDataBU, rectX, rectY, rectWidth, rectHeight);
        }
      }

      // Read the min LZW code size
      lzwCodeSize = readByte();

#if DEBUG == 1
      Serial.print(F("LzwCodeSize: "));
      Serial.println(lzwCodeSize);
#endif

      // Gather the lzw image data
      // NOTE: the dataBlockSize byte is left in the data as the lzw decoder needs it
      int offset = 0;
      int dataBlockSize = readByte();
      while (dataBlockSize != 0) {
#if DEBUG == 1
        Serial.print(F("dataBlockSize: "));
        Serial.println(dataBlockSize);
#endif
        backUpStream(1);
        dataBlockSize++;
        // quick fix to prevent a crash if lzwImageData is not large enough
        if (offset + dataBlockSize <= (int) sizeof(lzwImageData)) {
          readIntoBuffer(lzwImageData + offset, dataBlockSize);
        }
        else {
          int i;
          // discard the data block that would cause a buffer overflow
          for (i = 0; i < dataBlockSize; i++)
            file.read();
#if DEBUG == 1
          Serial.print(F("******* Prevented lzwImageData Overflow ******"));
#endif
        }

        offset += dataBlockSize;
        dataBlockSize = readByte();
      }

#if DEBUG == 1
      Serial.print(F("total lzwImageData Size: "));
      Serial.println(offset);
#endif

      // Process the animation frame for display

      // Initialize the LZW decoder for this frame
      lzw_decode_init(lzwCodeSize, lzwImageData);

      // Decompress LZW data and display the frame
      decompressAndDisplayFrame();

      // Graphic control extension is for a single frame
      // Remove its influence
      transparentColorIndex = NO_TRANSPARENT_INDEX;
      disposalMethod = DISPOSAL_NONE;

      // Make sure there is at least some delay between frames
      if (frameDelay < 1) {
        frameDelay = 1;
      }

      // delay(frameDelay * 10);
      return frameDelay * 10;
    }
    // parseTableBasedImage

    // LZW constants
    // NOTE: LZW_MAXBITS set to 11 to support more GIFs with 6k RAM increase (initially 10 to save memory)
#define LZW_MAXBITS    11
#define LZW_SIZTABLE  (1 << LZW_MAXBITS)

    // Masks for 0 .. 16 bits
    unsigned int mask[17] = {
      0x0000, 0x0001, 0x0003, 0x0007,
      0x000F, 0x001F, 0x003F, 0x007F,
      0x00FF, 0x01FF, 0x03FF, 0x07FF,
      0x0FFF, 0x1FFF, 0x3FFF, 0x7FFF,
      0xFFFF
    };

    // LZW variables
    byte *pbuf;
    int bbits;
    int bbuf;
    int cursize;                // The current code size
    int curmask;
    int codesize;
    int clear_code;
    int end_code;
    int newcodes;               // First available code
    int top_slot;               // Highest code for current size
    int extra_slot;
    int slot;                   // Last read code
    int fc, oc;
    int bs;                     // Current buffer size for GIF
    byte *sp;
    byte stack[LZW_SIZTABLE];
    byte suffix[LZW_SIZTABLE];
    unsigned int prefix[LZW_SIZTABLE];

    // Initialize LZW decoder
    //   csize initial code size in bits
    //   buf input data
    void lzw_decode_init(int csize, byte *buf) {

      // Initialize read buffer variables
      pbuf = buf;
      bbuf = 0;
      bbits = 0;
      bs = 0;

      // Initialize decoder variables
      codesize = csize;
      cursize = codesize + 1;
      curmask = mask[cursize];
      top_slot = 1 << cursize;
      clear_code = 1 << codesize;
      end_code = clear_code + 1;
      slot = newcodes = clear_code + 2;
      oc = fc = -1;
      sp = stack;
    }

    //  Get one code of given number of bits from stream
    int lzw_get_code() {

      while (bbits < cursize) {
        if (!bs) {
          bs = *pbuf++;
        }
        bbuf |= (*pbuf++) << bbits;
        bbits += 8;
        bs--;
      }
      int c = bbuf;
      bbuf >>= cursize;
      bbits -= cursize;
      return c & curmask;
    }

    // Decode given number of bytes
    //   buf 8 bit output buffer
    //   len number of pixels to decode
    //   returns the number of bytes decoded
    int lzw_decode(byte *buf, int len) {
      int l, c, code;

      if (end_code < 0) {
        return 0;
      }
      l = len;

      for (;;) {
        while (sp > stack) {
          *buf++ = *(--sp);
          if ((--l) == 0) {
            goto the_end;
          }
        }
        c = lzw_get_code();
        if (c == end_code) {
          break;

        }
        else if (c == clear_code) {
          cursize = codesize + 1;
          curmask = mask[cursize];
          slot = newcodes;
          top_slot = 1 << cursize;
          fc = oc = -1;

        }
        else	{

          code = c;
          if ((code == slot) && (fc >= 0)) {
            *sp++ = fc;
            code = oc;
          }
          else if (code >= slot) {
            break;
          }
          while (code >= newcodes) {
            *sp++ = suffix[code];
            code = prefix[code];
          }
          *sp++ = code;
          if ((slot < top_slot) && (oc >= 0)) {
            suffix[slot] = code;
            prefix[slot++] = oc;
          }
          fc = code;
          oc = c;
          if (slot >= top_slot) {
            if (cursize < LZW_MAXBITS) {
              top_slot <<= 1;
              curmask = mask[++cursize];
            }
            else {
#if DEBUG == 1
              Serial.println(F("****** cursize >= MAXBITS *******"));
#endif
            }
          }
        }
      }
      end_code = -1;
the_end:
      return len - l;
    }
    // lzw_decode

    // Decompress LZW data and display animation frame
    void decompressAndDisplayFrame() {

      // Each pixel of image is 8 bits and is an index into the palette

      // How the image is decoded depends upon whether it is interlaced or not
      // Decode the interlaced LZW data into the image buffer
      if (tbiInterlaced) {
        // Decode every 8th line starting at line 0
        for (int line = tbiImageY + 0; line < tbiHeight + tbiImageY; line += 8) {
          lzw_decode(imageData + (line * MATRIX_WIDTH) + tbiImageX, tbiWidth);
        }
        // Decode every 8th line starting at line 4
        for (int line = tbiImageY + 4; line < tbiHeight + tbiImageY; line += 8) {
          lzw_decode(imageData + (line * MATRIX_WIDTH) + tbiImageX, tbiWidth);
        }
        // Decode every 4th line starting at line 2
        for (int line = tbiImageY + 2; line < tbiHeight + tbiImageY; line += 4) {
          lzw_decode(imageData + (line * MATRIX_WIDTH) + tbiImageX, tbiWidth);
        }
        // Decode every 2nd line starting at line 1
        for (int line = tbiImageY + 1; line < tbiHeight + tbiImageY; line += 2) {
          lzw_decode(imageData + (line * MATRIX_WIDTH) + tbiImageX, tbiWidth);
        }
      }
      else	{
        // Decode the non interlaced LZW data into the image data buffer
        for (int line = tbiImageY; line < tbiHeight + tbiImageY; line++) {
          lzw_decode(imageData + (line * MATRIX_WIDTH) + tbiImageX, tbiWidth);
        }
      }

      // Image data is decompressed, now display portion of image affected by frame

      CRGB color;
      int yOffset, pixel;
      for (int y = tbiImageY; y < tbiHeight + tbiImageY; y++) {
        yOffset = y * MATRIX_WIDTH;
        for (int x = tbiImageX; x < tbiWidth + tbiImageX; x++) {
          // Get the next pixel
          pixel = imageData[yOffset + x];

          // Check pixel transparency
          if (pixel == transparentColorIndex) {
            continue;
          }

          // Pixel not transparent so get color from palette
          color.red = gifPalette[pixel].Red;
          color.green = gifPalette[pixel].Green;
          color.blue = gifPalette[pixel].Blue;

          // Draw the pixel
          //backgroundLayer.drawPixel(x, y, color);
#if 0
	  Serial.print(x);
	  Serial.print(" ");
	  Serial.print(y);
	  Serial.print(" > ");
	  Serial.println(color.r*65536+color.g*256+color.b, HEX);

#endif
	  matrix->setPassThruColor(color.r*65536+color.g*256+color.b);
          matrix->drawPixel(x, y, color);
	  matrix->setPassThruColor();
	  //matrixleds[XY(x,y)] = color;
        }
      }
      //// Make animation frame visible
      //backgroundLayer.swapBuffers();
    }

  public:
    void setFile(File imageFile) {
      file = imageFile;
    }

    // Make sure the file is a Gif file
    boolean parseGifHeader() {

      char buffer[10];

      readIntoBuffer(buffer, GIFHDRSIZE);

      if ((strncmp(buffer, GIFHDRTAGNORM, GIFHDRSIZE) != 0) &&
          (strncmp(buffer, GIFHDRTAGNORM1, GIFHDRSIZE) != 0))  {
        return false;
      }
      else    {
        return true;
      }
    }

    // Parse the logical screen descriptor
    void parseLogicalScreenDescriptor() {

      lsdWidth = readWord();
      lsdHeight = readWord();
      lsdPackedField = readByte();
      lsdBackgroundIndex = readByte();
      lsdAspectRatio = readByte();

#if DEBUG == 1
      Serial.print(F("lsdWidth: "));
      Serial.println(lsdWidth);
      Serial.print(F("lsdHeight: "));
      Serial.println(lsdHeight);
      Serial.print(F("lsdPackedField: "));
      Serial.println(lsdPackedField, HEX);
      Serial.print(F("lsdBackgroundIndex: "));
      Serial.println(lsdBackgroundIndex);
      Serial.print(F("lsdAspectRatio: "));
      Serial.println(lsdAspectRatio);
#endif
    }

    // Parse the global color table
    void parseGlobalColorTable() {

      // Does a global color table exist?
      if (lsdPackedField & COLORTBLFLAG) {

        // A GCT was present determine how many colors it contains
        colorCount = 1 << ((lsdPackedField & 7) + 1);

#if DEBUG == 1
        Serial.print(F("Global color table with "));
        Serial.print(colorCount);
        Serial.println(F(" colors present"));
#endif
        // Read color values into the palette array
        int colorTableBytes = sizeof(_RGB) * colorCount;
        readIntoBuffer(gifPalette, colorTableBytes);
      }
    }

    unsigned long drawFrame() {

#if DEBUG == 1
      Serial.println(F("\nParsing Data Block"));
#endif

      boolean done = false;
      while (!done) {

#if 0 && DEBUG == 1
        Serial.println(F("\nPress Key For Next"));
        while (Serial.read() <= 0);
#endif

        // Determine what kind of data to process
        byte b = readByte();

        if (b == 0x2c) {
          // Parse table based image
#if DEBUG == 1
          Serial.println(F("\nParsing Table Based"));
#endif
          unsigned int fdelay = parseTableBasedImage();
          return fdelay;
        }
        else if (b == 0x21) {
          // Parse extension
          b = readByte();

          // Determine which kind of extension to parse
          switch (b) {
            case 0x01:
              // Plain test extension
              parsePlainTextExtension();
              break;
            case 0xf9:
              // Graphic control extension
              parseGraphicControlExtension();
              break;
            case 0xfe:
              // Comment extension
              parseCommentExtension();
              break;
            case 0xff:
              // Application extension
              parseApplicationExtension();
              break;
            default:
#if DEBUG == 1
              Serial.print(F("Unknown control extension: "));
              Serial.println(b, HEX);
#endif
              return ERROR_UNKNOWNCONTROLEXT;
          }
        }
        else	{
#if DEBUG == 1
          Serial.println(F("\nParsing Done"));
#endif
          done = true;

          // Push unprocessed byte back into the stream for later processing
          backUpStream(1);

          return ERROR_FINISHED;
        }
      }
      return ERROR_NONE;
    }
};

#endif
