/**
** The original 3x5 font is licensed under the 3-clause BSD license:
**
** Copyright 1999 Brian J. Swetland
** Copyright 1999 Vassilii Khachaturov
** Portions (of vt100.c/vt100.h) copyright Dan Marks
** 
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions, and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions, and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the authors may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
** 
** Modifications to Tom Thumb for improved readability are from Robey Pointer, 
** see:
** http://robey.lag.net/2010/01/23/tiny-monospace-font.html
** 
** The original author does not have any objection to relicensing of Robey 
** Pointer's modifications (in this file) in a more permissive license.  See 
** the discussion at the above blog, and also here:
** http://opengameart.org/forumtopic/how-to-submit-art-using-the-3-clause-bsd-license
**
** Feb 21, 2016: Conversion from Linux BDF --> Adafruit GFX font,
** with the help of this Python script:
** https://gist.github.com/skelliam/322d421f028545f16f6d
** William Skellenger (williamj@skellenger.net)
** Twitter: @skelliam
** 
** Jan 09, 2020: Bitmaps now compressed, to fix the bounding box problem,
** because non-compressed the calculated text width were wrong.
** Andreas Merkle (web@blue-andi.de)
**
** May 07, 2020: Increased readability by adapting the appearance of numbers 
** and uppercase letters.
** Yann Le Glaz (yann_le@web.de)
**
** Apr 07, 2021: Last character definition is now automatically set depended on
** the TOMTHUMB_USE_EXTENDED define.
** Andreas Merkle (web@blue-andi.de)
**
** Apr 09, 2021: Increased readability by adapting the appearance of the degree
** symbol (0xB0) for better readability
** Yann Le Glaz (yann_le@web.de)
*/

#define TOMTHUMB_USE_EXTENDED 1

#if TOMTHUMB_USE_EXTENDED
#define TOMTHUMB_LAST_CHAR_ID (0xEB)
#else
#define TOMTHUMB_LAST_CHAR_ID (0x7E)
#endif

const uint8_t TomThumbBitmaps[] PROGMEM = {
    0x00,                /* 0x20 space */
    0xE8,                /* 0x21 exclam */
    0xB4,                /* 0x22 quotedbl */
    0xBE, 0xFA,            /* 0x23 numbersign */
    0x79, 0xE4,            /* 0x24 dollar */
    0xA5, 0x4A,            /* 0x25 percent */
    0xDB, 0xD6,            /* 0x26 ampersand */
    0xC0,                /* 0x27 quotesingle */
    0x6A, 0x40,            /* 0x28 parenleft */
    0x95, 0x80,            /* 0x29 parenright */
    0xAA, 0x80,            /* 0x2A asterisk */
    0x5D, 0x00,            /* 0x2B plus */
    0x60,                /* 0x2C comma */
    0xE0,                /* 0x2D hyphen */
    0x80,                /* 0x2E period */
    0x25, 0x48,            /* 0x2F slash */
    0xF6, 0xDE,            /* 0x30 zero */
    0x59, 0x2E,            /* 0x31 one */
    0xE7, 0xCE,            /* 0x32 two */
    0xE7, 0x9E,            /* 0x33 three */
    0xB7, 0x92,            /* 0x34 four */
    0xF3, 0x9E,            /* 0x35 five */
    0xF3, 0xDE,            /* 0x36 six */
    0xE4, 0x92,            /* 0x37 seven */
    0xF7, 0xDE,            /* 0x38 eight */
    0xF7, 0x9E,            /* 0x39 nine */
    0xA0,                /* 0x3A colon */
    0x46,                /* 0x3B semicolon */
    0x2A, 0x22,            /* 0x3C less */
    0xE3, 0x80,            /* 0x3D equal */
    0x88, 0xA8,            /* 0x3E greater */
    0xE5, 0x04,            /* 0x3F question */
    0x57, 0xC6,            /* 0x40 at */
    0xD7, 0xDA,            /* 0x41 A */
    0xD7, 0x5C,            /* 0x42 B */
    0x56, 0x54,            /* 0x43 C */
    0xD6, 0xDC,            /* 0x44 D */
    0xF3, 0xCE,            /* 0x45 E */
    0xF3, 0xC8,            /* 0x46 F */
    0x72, 0xD6,            /* 0x47 G */
    0xB7, 0xDA,            /* 0x48 H */
    0xF8,                /* 0x49 I */
    0x24, 0xD4,            /* 0x4A J */
    0xB7, 0x5A,            /* 0x4B K */
    0x92, 0x4E,            /* 0x4C L */
    0x8E, 0xEB, 0x18, 0x80,    /* 0x4D M */
    0x9D, 0xB9, 0x90,    /* 0x4E N */
    0x56, 0xD4,            /* 0x4F O */
    0xF7, 0x48,            /* 0x50 P */
    0x4A, 0xAA, 0x70,    /* 0x51 Q */
    0xF7, 0x5A,            /* 0x52 R */
    0xF3, 0x9E,            /* 0x53 S */
    0xE9, 0x24,            /* 0x54 T */
    0xB6, 0xDE,            /* 0x55 U */
    0xB6, 0xD4,            /* 0x56 V */
    0x8C, 0x63, 0x55, 0x00,    /* 0x57 W */
    0xB5, 0x5A,            /* 0x58 X */
    0xB7, 0x9C,            /* 0x59 Y */
    0xE5, 0x4E,            /* 0x5A Z */
    0xF2, 0x4E,            /* 0x5B bracketleft */
    0x88, 0x80,            /* 0x5C backslash */
    0xE4, 0x9E,            /* 0x5D bracketright */
    0x54,                /* 0x5E asciicircum */
    0xE0,                /* 0x5F underscore */
    0x90,                /* 0x60 grave */
    0xCE, 0xF0,            /* 0x61 a */
    0x9A, 0xDC,            /* 0x62 b */
    0x72, 0x30,            /* 0x63 c */
    0x2E, 0xD6,            /* 0x64 d */
    0x77, 0x30,            /* 0x65 e */
    0x2B, 0xA4,            /* 0x66 f */
    0x77, 0x94,            /* 0x67 g */
    0x9A, 0xDA,            /* 0x68 h */
    0xB8,                /* 0x69 i */
    0x20, 0x9A, 0x80,    /* 0x6A j */
    0x97, 0x6A,            /* 0x6B k */
    0xC9, 0x2E,            /* 0x6C l */
    0xFF, 0xD0,            /* 0x6D m */
    0xD6, 0xD0,            /* 0x6E n */
    0x56, 0xA0,            /* 0x6F o */
    0xD6, 0xE8,            /* 0x70 p */
    0x76, 0xB2,            /* 0x71 q */
    0x72, 0x40,            /* 0x72 r */
    0x79, 0xE0,            /* 0x73 s */
    0x5D, 0x26,            /* 0x74 t */
    0xB6, 0xB0,            /* 0x75 u */
    0xB7, 0xA0,            /* 0x76 v */
    0xBF, 0xF0,            /* 0x77 w */
    0xA9, 0x50,            /* 0x78 x */
    0xB5, 0x94,            /* 0x79 y */
    0xEF, 0x70,            /* 0x7A z */
    0x6A, 0x26,            /* 0x7B braceleft */
    0xD8,                /* 0x7C bar */
    0xC8, 0xAC,            /* 0x7D braceright */
    0x78,                /* 0x7E asciitilde */
#if (TOMTHUMB_USE_EXTENDED)
    0xB8,                /* 0xA1 exclamdown */
    0x5E, 0x74,            /* 0xA2 cent */
    0x6B, 0xAE,            /* 0xA3 sterling */
    0xAB, 0xAA,            /* 0xA4 currency */
    0xB5, 0x74,            /* 0xA5 yen */
    0xD8,                /* 0xA6 brokenbar */
    0x6A, 0xAC,            /* 0xA7 section */
    0xA0,                /* 0xA8 dieresis */
    0x71, 0x80,            /* 0xA9 copyright */
    0x77, 0x8E,            /* 0xAA ordfeminine */
    0x64,                /* 0xAB guillemotleft */
    0xE4,                /* 0xAC logicalnot */
    0xC0,                /* 0xAD softhyphen */
    0xDA, 0x80,            /* 0xAE registered */
    0xE0,                /* 0xAF macron */
    0xC0, 0xC0, 0x00,    /* 0xB0 degree */
    0x5D, 0x0E,            /* 0xB1 plusminus */
    0xC9, 0x80,            /* 0xB2 twosuperior */
    0xEF, 0x80,            /* 0xB3 threesuperior */
    0x60,                /* 0xB4 acute */
    0xB6, 0xE8,            /* 0xB5 mu */
    0x75, 0xB6,            /* 0xB6 paragraph */
    0xFF, 0x80,            /* 0xB7 periodcentered */
    0x47, 0x00,            /* 0xB8 cedilla */
    0xE0,                /* 0xB9 onesuperior */
    0x55, 0x0E,            /* 0xBA ordmasculine */
    0x98,                /* 0xBB guillemotright */
    0x90, 0x32,            /* 0xBC onequarter */
    0x90, 0x66,            /* 0xBD onehalf */
    0xD8, 0x32,            /* 0xBE threequarters */
    0x41, 0x4E,            /* 0xBF questiondown */
    0x45, 0x7A,            /* 0xC0 Agrave */
    0x51, 0x7A,            /* 0xC1 Aacute */
    0xE1, 0x7A,            /* 0xC2 Acircumflex */
    0x79, 0x7A,            /* 0xC3 Atilde */
    0xAA, 0xFA,            /* 0xC4 Adieresis */
    0xDA, 0xFA,            /* 0xC5 Aring */
    0x7B, 0xEE,            /* 0xC6 AE */
    0x72, 0x32, 0x80,    /* 0xC7 Ccedilla */
    0x47, 0xEE,            /* 0xC8 Egrave */
    0x53, 0xEE,            /* 0xC9 Eacute */
    0xE3, 0xEE,            /* 0xCA Ecircumflex */
    0xA3, 0xEE,            /* 0xCB Edieresis */
    0x47, 0xAE,            /* 0xCC Igrave */
    0x53, 0xAE,            /* 0xCD Iacute */
    0xE3, 0xAE,            /* 0xCE Icircumflex */
    0xA3, 0xAE,            /* 0xCF Idieresis */
    0xD7, 0xDC,            /* 0xD0 Eth */
    0xCE, 0xFA,            /* 0xD1 Ntilde */
    0x47, 0xDE,            /* 0xD2 Ograve */
    0x53, 0xDE,            /* 0xD3 Oacute */
    0xE3, 0xDE,            /* 0xD4 Ocircumflex */
    0xCF, 0xDE,            /* 0xD5 Otilde */
    0xA3, 0xDE,            /* 0xD6 Odieresis */
    0xAA, 0x80,            /* 0xD7 multiply */
    0x77, 0xDC,            /* 0xD8 Oslash */
    0x8A, 0xDE,            /* 0xD9 Ugrave */
    0x2A, 0xDE,            /* 0xDA Uacute */
    0xE2, 0xDE,            /* 0xDB Ucircumflex */
    0xA2, 0xDE,            /* 0xDC Udieresis */
    0x2A, 0xF4,            /* 0xDD Yacute */
    0x9E, 0xF8,            /* 0xDE Thorn */
    0x77, 0x5D, 0x00,    /* 0xDF germandbls */
    0x45, 0xDE,            /* 0xE0 agrave */
    0x51, 0xDE,            /* 0xE1 aacute */
    0xE1, 0xDE,            /* 0xE2 acircumflex */
    0x79, 0xDE,            /* 0xE3 atilde */
    0xA1, 0xDE,            /* 0xE4 adieresis */
    0x6D, 0xDE,            /* 0xE5 aring */
    0x7F, 0xE0,            /* 0xE6 ae */
    0x71, 0x94,            /* 0xE7 ccedilla */
    0x45, 0xF6,            /* 0xE8 egrave */
    0x51, 0xF6,            /* 0xE9 eacute */
    0xE1, 0xF6,            /* 0xEA ecircumflex */
    0xA1, 0xF6,            /* 0xEB edieresis */
    0x9A, 0x80,            /* 0xEC igrave */
    0x65, 0x40,            /* 0xED iacute */
    0xE1, 0x24,            /* 0xEE icircumflex */
    0xA1, 0x24,            /* 0xEF idieresis */
    0x79, 0xD6,            /* 0xF0 eth */
    0xCF, 0x5A,            /* 0xF1 ntilde */
    0x45, 0x54,            /* 0xF2 ograve */
    0x51, 0x54,            /* 0xF3 oacute */
    0xE1, 0x54,            /* 0xF4 ocircumflex */
    0xCD, 0x54,            /* 0xF5 otilde */
    0xA1, 0x54,            /* 0xF6 odieresis */
    0x43, 0x84,            /* 0xF7 divide */
    0x7E, 0xE0,            /* 0xF8 oslash */
    0x8A, 0xD6,            /* 0xF9 ugrave */
    0x2A, 0xD6,            /* 0xFA uacute */
    0xE2, 0xD6,            /* 0xFB ucircumflex */
    0xA2, 0xD6,            /* 0xFC udieresis */
    0x2A, 0xB2, 0x80,    /* 0xFD yacute */
    0x9A, 0xE8,            /* 0xFE thorn */
    0xA2, 0xB2, 0x80,    /* 0xFF ydieresis */
    0x00,                /* 0x11D gcircumflex */
    0x7B, 0xE6,            /* 0x152 OE */
    0x7F, 0x70,            /* 0x153 oe */
    0xAF, 0x3C,            /* 0x160 Scaron */
    0xAF, 0x3C,            /* 0x161 scaron */
    0xA2, 0xA4,            /* 0x178 Ydieresis */
    0xBD, 0xEE,            /* 0x17D Zcaron */
    0xBD, 0xEE,            /* 0x17E zcaron */
    0x00,                /* 0xEA4 uni0EA4 */
    0x00,                /* 0x13A0 uni13A0 */
    0x80,                /* 0x2022 bullet */
    0xA0,                /* 0x2026 ellipsis */
    0x7F, 0xE6,            /* 0x20AC Euro */
    0xEA, 0xAA, 0xE0,   /* 0xFFFD uniFFFD */
#endif /* (TOMTHUMB_USE_EXTENDED)  */
  };


/* {offset, width, height, advance cursor, x offset, y offset} */
const GFXglyph TomThumbGlyphs[] PROGMEM = {
    { 0, 1, 1, 2, 0, -5 }, /* 0x20 space */
    { 1, 1, 5, 2, 0, -5 }, /* 0x21 exclam */
    { 2, 3, 2, 4, 0, -5 }, /* 0x22 quotedbl */
    { 3, 3, 5, 4, 0, -5 }, /* 0x23 numbersign */
    { 5, 3, 5, 4, 0, -5 }, /* 0x24 dollar */
    { 7, 3, 5, 4, 0, -5 }, /* 0x25 percent */
    { 9, 3, 5, 4, 0, -5 }, /* 0x26 ampersand */
    { 11, 1, 2, 2, 0, -5 }, /* 0x27 quotesingle */
    { 12, 2, 5, 3, 0, -5 }, /* 0x28 parenleft */
    { 14, 2, 5, 3, 0, -5 }, /* 0x29 parenright */
    { 16, 3, 3, 4, 0, -5 }, /* 0x2A asterisk */
    { 18, 3, 3, 4, 0, -4 }, /* 0x2B plus */
    { 20, 2, 2, 3, 0, -2 }, /* 0x2C comma */
    { 21, 3, 1, 4, 0, -3 }, /* 0x2D hyphen */
    { 22, 1, 1, 2, 0, -1 }, /* 0x2E period */
    { 23, 3, 5, 4, 0, -5 }, /* 0x2F slash */
    { 25, 3, 5, 4, 0, -5 }, /* 0x30 zero */
    { 27, 3, 5, 4, 0, -5 }, /* 0x31 one */
    { 29, 3, 5, 4, 0, -5 }, /* 0x32 two */
    { 31, 3, 5, 4, 0, -5 }, /* 0x33 three */
    { 33, 3, 5, 4, 0, -5 }, /* 0x34 four */
    { 35, 3, 5, 4, 0, -5 }, /* 0x35 five */
    { 37, 3, 5, 4, 0, -5 }, /* 0x36 six */
    { 39, 3, 5, 4, 0, -5 }, /* 0x37 seven */
    { 41, 3, 5, 4, 0, -5 }, /* 0x38 eight */
    { 43, 3, 5, 4, 0, -5 }, /* 0x39 nine */
    { 45, 1, 3, 2, 0, -4 }, /* 0x3A colon */
    { 46, 2, 4, 3, 0, -4 }, /* 0x3B semicolon */
    { 47, 3, 5, 4, 0, -5 }, /* 0x3C less */
    { 49, 3, 3, 4, 0, -4 }, /* 0x3D equal */
    { 51, 3, 5, 4, 0, -5 }, /* 0x3E greater */
    { 53, 3, 5, 4, 0, -5 }, /* 0x3F question */
    { 55, 3, 5, 4, 0, -5 }, /* 0x40 at */
    { 57, 3, 5, 4, 0, -5 }, /* 0x41 A */
    { 59, 3, 5, 4, 0, -5 }, /* 0x42 B */
    { 61, 3, 5, 4, 0, -5 }, /* 0x43 C */
    { 63, 3, 5, 4, 0, -5 }, /* 0x44 D */
    { 65, 3, 5, 4, 0, -5 }, /* 0x45 E */
    { 67, 3, 5, 4, 0, -5 }, /* 0x46 F */
    { 69, 3, 5, 4, 0, -5 }, /* 0x47 G */
    { 71, 3, 5, 4, 0, -5 }, /* 0x48 H */
    { 73, 1, 5, 2, 0, -5 }, /* 0x49 I */
    { 74, 3, 5, 4, 0, -5 }, /* 0x4A J */
    { 76, 3, 5, 4, 0, -5 }, /* 0x4B K */
    { 78, 3, 5, 4, 0, -5 }, /* 0x4C L */
    { 80, 5, 5, 6, 0, -5 }, /* 0x4D M */
    { 84, 4, 5, 5, 0, -5 }, /* 0x4E N */
    { 87, 3, 5, 4, 0, -5 }, /* 0x4F O */
    { 89, 3, 5, 4, 0, -5 }, /* 0x50 P */
    { 91, 4, 5, 5, 0, -5 }, /* 0x51 Q */
    { 94, 3, 5, 4, 0, -5 }, /* 0x52 R */
    { 96, 3, 5, 4, 0, -5 }, /* 0x53 S */
    { 98, 3, 5, 4, 0, -5 }, /* 0x54 T */
    { 100, 3, 5, 4, 0, -5 }, /* 0x55 U */
    { 102, 3, 5, 4, 0, -5 }, /* 0x56 V */
    { 104, 5, 5, 6, 0, -5 }, /* 0x57 W */
    { 108, 3, 5, 4, 0, -5 }, /* 0x58 X */
    { 110, 3, 5, 4, 0, -5 }, /* 0x59 Y */
    { 112, 3, 5, 4, 0, -5 }, /* 0x5A Z */
    { 114, 3, 5, 4, 0, -5 }, /* 0x5B bracketleft */
    { 116, 3, 3, 4, 0, -4 }, /* 0x5C backslash */
    { 118, 3, 5, 4, 0, -5 }, /* 0x5D bracketright */
    { 120, 3, 2, 4, 0, -5 }, /* 0x5E asciicircum */
    { 121, 3, 1, 4, 0, -1 }, /* 0x5F underscore */
    { 122, 2, 2, 3, 0, -5 }, /* 0x60 grave */
    { 123, 3, 4, 4, 0, -4 }, /* 0x61 a */
    { 125, 3, 5, 4, 0, -5 }, /* 0x62 b */
    { 127, 3, 4, 4, 0, -4 }, /* 0x63 c */
    { 129, 3, 5, 4, 0, -5 }, /* 0x64 d */
    { 131, 3, 4, 4, 0, -4 }, /* 0x65 e */
    { 133, 3, 5, 4, 0, -5 }, /* 0x66 f */
    { 135, 3, 5, 4, 0, -4 }, /* 0x67 g */
    { 137, 3, 5, 4, 0, -5 }, /* 0x68 h */
    { 139, 1, 5, 2, 0, -5 }, /* 0x69 i */
    { 140, 3, 6, 4, 0, -5 }, /* 0x6A j */
    { 143, 3, 5, 4, 0, -5 }, /* 0x6B k */
    { 145, 3, 5, 4, 0, -5 }, /* 0x6C l */
    { 147, 3, 4, 4, 0, -4 }, /* 0x6D m */
    { 149, 3, 4, 4, 0, -4 }, /* 0x6E n */
    { 151, 3, 4, 4, 0, -4 }, /* 0x6F o */
    { 153, 3, 5, 4, 0, -4 }, /* 0x70 p */
    { 155, 3, 5, 4, 0, -4 }, /* 0x71 q */
    { 157, 3, 4, 4, 0, -4 }, /* 0x72 r */
    { 159, 3, 4, 4, 0, -4 }, /* 0x73 s */
    { 161, 3, 5, 4, 0, -5 }, /* 0x74 t */
    { 163, 3, 4, 4, 0, -4 }, /* 0x75 u */
    { 165, 3, 4, 4, 0, -4 }, /* 0x76 v */
    { 167, 3, 4, 4, 0, -4 }, /* 0x77 w */
    { 169, 3, 4, 4, 0, -4 }, /* 0x78 x */
    { 171, 3, 5, 4, 0, -4 }, /* 0x79 y */
    { 173, 3, 4, 4, 0, -4 }, /* 0x7A z */
    { 175, 3, 5, 4, 0, -5 }, /* 0x7B braceleft */
    { 177, 1, 5, 2, 0, -5 }, /* 0x7C bar */
    { 178, 3, 5, 4, 0, -5 }, /* 0x7D braceright */
    { 180, 3, 2, 4, 0, -5 }, /* 0x7E asciitilde */
#if (TOMTHUMB_USE_EXTENDED)
    { 181, 1, 5, 2, 0, -5 }, /* 0xA1 exclamdown */
    { 182, 3, 5, 4, 0, -5 }, /* 0xA2 cent */
    { 184, 3, 5, 4, 0, -5 }, /* 0xA3 sterling */
    { 186, 3, 5, 4, 0, -5 }, /* 0xA4 currency */
    { 188, 3, 5, 4, 0, -5 }, /* 0xA5 yen */
    { 190, 1, 5, 2, 0, -5 }, /* 0xA6 brokenbar */
    { 191, 3, 5, 4, 0, -5 }, /* 0xA7 section */
    { 193, 3, 1, 4, 0, -5 }, /* 0xA8 dieresis */
    { 194, 3, 3, 4, 0, -5 }, /* 0xA9 copyright */
    { 196, 3, 5, 4, 0, -5 }, /* 0xAA ordfeminine */
    { 198, 2, 3, 3, 0, -5 }, /* 0xAB guillemotleft */
    { 199, 3, 2, 4, 0, -4 }, /* 0xAC logicalnot */
    { 200, 2, 1, 3, 0, -3 }, /* 0xAD softhyphen */
    { 201, 3, 3, 4, 0, -5 }, /* 0xAE registered */
    { 203, 3, 1, 4, 0, -5 }, /* 0xAF macron */
    { 204, 8, 3, 4, 0, -5 }, /* 0xB0 degree */
    { 206, 3, 5, 4, 0, -5 }, /* 0xB1 plusminus */
    { 208, 3, 3, 4, 0, -5 }, /* 0xB2 twosuperior */
    { 210, 3, 3, 4, 0, -5 }, /* 0xB3 threesuperior */
    { 212, 2, 2, 3, 0, -5 }, /* 0xB4 acute */
    { 213, 3, 5, 4, 0, -5 }, /* 0xB5 mu */
    { 215, 3, 5, 4, 0, -5 }, /* 0xB6 paragraph */
    { 217, 3, 3, 4, 0, -4 }, /* 0xB7 periodcentered */
    { 219, 3, 3, 4, 0, -3 }, /* 0xB8 cedilla */
    { 221, 1, 3, 2, 0, -5 }, /* 0xB9 onesuperior */
    { 222, 3, 5, 4, 0, -5 }, /* 0xBA ordmasculine */
    { 224, 2, 3, 3, 0, -5 }, /* 0xBB guillemotright */
    { 225, 3, 5, 4, 0, -5 }, /* 0xBC onequarter */
    { 227, 3, 5, 4, 0, -5 }, /* 0xBD onehalf */
    { 229, 3, 5, 4, 0, -5 }, /* 0xBE threequarters */
    { 231, 3, 5, 4, 0, -5 }, /* 0xBF questiondown */
    { 233, 3, 5, 4, 0, -5 }, /* 0xC0 Agrave */
    { 235, 3, 5, 4, 0, -5 }, /* 0xC1 Aacute */
    { 237, 3, 5, 4, 0, -5 }, /* 0xC2 Acircumflex */
    { 239, 3, 5, 4, 0, -5 }, /* 0xC3 Atilde */
    { 241, 3, 5, 4, 0, -5 }, /* 0xC4 Adieresis */
    { 243, 3, 5, 4, 0, -5 }, /* 0xC5 Aring */
    { 245, 3, 5, 4, 0, -5 }, /* 0xC6 AE */
    { 247, 3, 6, 4, 0, -5 }, /* 0xC7 Ccedilla */
    { 250, 3, 5, 4, 0, -5 }, /* 0xC8 Egrave */
    { 252, 3, 5, 4, 0, -5 }, /* 0xC9 Eacute */
    { 254, 3, 5, 4, 0, -5 }, /* 0xCA Ecircumflex */
    { 256, 3, 5, 4, 0, -5 }, /* 0xCB Edieresis */
    { 258, 3, 5, 4, 0, -5 }, /* 0xCC Igrave */
    { 260, 3, 5, 4, 0, -5 }, /* 0xCD Iacute */
    { 262, 3, 5, 4, 0, -5 }, /* 0xCE Icircumflex */
    { 264, 3, 5, 4, 0, -5 }, /* 0xCF Idieresis */
    { 266, 3, 5, 4, 0, -5 }, /* 0xD0 Eth */
    { 268, 3, 5, 4, 0, -5 }, /* 0xD1 Ntilde */
    { 270, 3, 5, 4, 0, -5 }, /* 0xD2 Ograve */
    { 272, 3, 5, 4, 0, -5 }, /* 0xD3 Oacute */
    { 274, 3, 5, 4, 0, -5 }, /* 0xD4 Ocircumflex */
    { 276, 3, 5, 4, 0, -5 }, /* 0xD5 Otilde */
    { 278, 3, 5, 4, 0, -5 }, /* 0xD6 Odieresis */
    { 280, 3, 3, 4, 0, -4 }, /* 0xD7 multiply */
    { 282, 3, 5, 4, 0, -5 }, /* 0xD8 Oslash */
    { 284, 3, 5, 4, 0, -5 }, /* 0xD9 Ugrave */
    { 286, 3, 5, 4, 0, -5 }, /* 0xDA Uacute */
    { 288, 3, 5, 4, 0, -5 }, /* 0xDB Ucircumflex */
    { 290, 3, 5, 4, 0, -5 }, /* 0xDC Udieresis */
    { 292, 3, 5, 4, 0, -5 }, /* 0xDD Yacute */
    { 294, 3, 5, 4, 0, -5 }, /* 0xDE Thorn */
    { 296, 3, 6, 4, 0, -5 }, /* 0xDF germandbls */
    { 299, 3, 5, 4, 0, -5 }, /* 0xE0 agrave */
    { 301, 3, 5, 4, 0, -5 }, /* 0xE1 aacute */
    { 303, 3, 5, 4, 0, -5 }, /* 0xE2 acircumflex */
    { 305, 3, 5, 4, 0, -5 }, /* 0xE3 atilde */
    { 307, 3, 5, 4, 0, -5 }, /* 0xE4 adieresis */
    { 309, 3, 5, 4, 0, -5 }, /* 0xE5 aring */
    { 311, 3, 4, 4, 0, -4 }, /* 0xE6 ae */
    { 313, 3, 5, 4, 0, -4 }, /* 0xE7 ccedilla */
    { 315, 3, 5, 4, 0, -5 }, /* 0xE8 egrave */
    { 317, 3, 5, 4, 0, -5 }, /* 0xE9 eacute */
    { 319, 3, 5, 4, 0, -5 }, /* 0xEA ecircumflex */
    { 321, 3, 5, 4, 0, -5 }, /* 0xEB edieresis */
    { 323, 2, 5, 3, 0, -5 }, /* 0xEC igrave */
    { 325, 2, 5, 3, 0, -5 }, /* 0xED iacute */
    { 327, 3, 5, 4, 0, -5 }, /* 0xEE icircumflex */
    { 329, 3, 5, 4, 0, -5 }, /* 0xEF idieresis */
    { 331, 3, 5, 4, 0, -5 }, /* 0xF0 eth */
    { 333, 3, 5, 4, 0, -5 }, /* 0xF1 ntilde */
    { 335, 3, 5, 4, 0, -5 }, /* 0xF2 ograve */
    { 337, 3, 5, 4, 0, -5 }, /* 0xF3 oacute */
    { 339, 3, 5, 4, 0, -5 }, /* 0xF4 ocircumflex */
    { 341, 3, 5, 4, 0, -5 }, /* 0xF5 otilde */
    { 343, 3, 5, 4, 0, -5 }, /* 0xF6 odieresis */
    { 345, 3, 5, 4, 0, -5 }, /* 0xF7 divide */
    { 347, 3, 4, 4, 0, -4 }, /* 0xF8 oslash */
    { 349, 3, 5, 4, 0, -5 }, /* 0xF9 ugrave */
    { 351, 3, 5, 4, 0, -5 }, /* 0xFA uacute */
    { 353, 3, 5, 4, 0, -5 }, /* 0xFB ucircumflex */
    { 355, 3, 5, 4, 0, -5 }, /* 0xFC udieresis */
    { 357, 3, 6, 4, 0, -5 }, /* 0xFD yacute */
    { 360, 3, 5, 4, 0, -4 }, /* 0xFE thorn */
    { 362, 3, 6, 4, 0, -5 }, /* 0xFF ydieresis */
    { 365, 1, 1, 2, 0, -1 }, /* 0x11D gcircumflex */
    { 366, 3, 5, 4, 0, -5 }, /* 0x152 OE */
    { 368, 3, 4, 4, 0, -4 }, /* 0x153 oe */
    { 370, 3, 5, 4, 0, -5 }, /* 0x160 Scaron */
    { 372, 3, 5, 4, 0, -5 }, /* 0x161 scaron */
    { 374, 3, 5, 4, 0, -5 }, /* 0x178 Ydieresis */
    { 376, 3, 5, 4, 0, -5 }, /* 0x17D Zcaron */
    { 378, 3, 5, 4, 0, -5 }, /* 0x17E zcaron */
    { 380, 1, 1, 2, 0, -1 }, /* 0xEA4 uni0EA4 */
    { 381, 1, 1, 2, 0, -1 }, /* 0x13A0 uni13A0 */
    { 382, 1, 1, 2, 0, -3 }, /* 0x2022 bullet */
    { 383, 3, 1, 4, 0, -1 }, /* 0x2026 ellipsis */
    { 384, 3, 5, 4, 0, -5 }, /* 0x20AC Euro */
    { 386, 4, 5, 5, 0, -5 }, /* 0xFFFD uniFFFD */
#endif /* (TOMTHUMB_USE_EXTENDED) */
};

const GFXfont TomThumb PROGMEM =
{
    (uint8_t *)TomThumbBitmaps,
    (GFXglyph*)TomThumbGlyphs,
    0x20,
    TOMTHUMB_LAST_CHAR_ID,
    6
};
