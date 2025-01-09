/**
** The original 3x5 font is licensed under the 3-clause BSD license:
**
** Copyright 1999 Brian J. Swetland
** Copyright 1999 Vassilii Khachaturov
** Portions (of vt100.c/vt100.h) copyright Dan Marks
** Copyright 2020 - 2025 Andreas Merkle <web@blue-andi.de>
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
**
** Jan 08, 2025: Beautified for better readability.
** Andreas Merkle (web@blue-andi.de)
**
** Jan 09, 2025: Fixed degree symbol (U+00B0) bitmap and glyph by using an
** encoding which needs only 2 bitmap bytes. This ensures that all glyph
** offsets match again.
** Andreas Merkle (web@blue-andi.de)
*/

#define TOMTHUMB_USE_EXTENDED 1

#if TOMTHUMB_USE_EXTENDED
#define TOMTHUMB_LAST_CHAR_ID (0xEB)
#else
#define TOMTHUMB_LAST_CHAR_ID (0x7E)
#endif

const uint8_t TomThumbBitmaps[] PROGMEM = {
    0x00,                   /* [000] U+0020 ' ' space */
    0xE8,                   /* [001] U+0021 '!' exclam */
    0xB4,                   /* [002] U+0022 '"' quotedbl */
    0xBE, 0xFA,             /* [003] U+0023 '#' numbersign */
    0x79, 0xE4,             /* [004] U+0024 '$' dollar */
    0xA5, 0x4A,             /* [005] U+0025 '%' percent */
    0xDB, 0xD6,             /* [006] U+0026 '&' ampersand */
    0xC0,                   /* [007] U+0027 ''' quotesingle */
    0x6A, 0x40,             /* [008] U+0028 '(' parenleft */
    0x95, 0x80,             /* [009] U+0029 ')' parenright */
    0xAA, 0x80,             /* [010] U+002A '*' asterisk */
    0x5D, 0x00,             /* [011] U+002B '+' plus */
    0x60,                   /* [012] U+002C ',' comma */
    0xE0,                   /* [013] U+002D '-' hyphen */
    0x80,                   /* [014] U+002E '.' period */
    0x25, 0x48,             /* [015] U+002F '/' slash */
    0xF6, 0xDE,             /* [016] U+0030 '0' zero */
    0x59, 0x2E,             /* [017] U+0031 '1' one */
    0xE7, 0xCE,             /* [018] U+0032 '2' two */
    0xE7, 0x9E,             /* [019] U+0033 '3' three */
    0xB7, 0x92,             /* [020] U+0034 '4' four */
    0xF3, 0x9E,             /* [021] U+0035 '5' five */
    0xF3, 0xDE,             /* [022] U+0036 '6' six */
    0xE4, 0x92,             /* [023] U+0037 '7' seven */
    0xF7, 0xDE,             /* [024] U+0038 '8' eight */
    0xF7, 0x9E,             /* [025] U+0039 '9' nine */
    0xA0,                   /* [026] U+003A ':' colon */
    0x46,                   /* [027] U+003B ';' semicolon */
    0x2A, 0x22,             /* [028] U+003C '<' less */
    0xE3, 0x80,             /* [029] U+003D '=' equal */
    0x88, 0xA8,             /* [030] U+003E '>' greater */
    0xE5, 0x04,             /* [031] U+003F '?' question */
    0x57, 0xC6,             /* [032] U+0040 '@' at */
    0xD7, 0xDA,             /* [033] U+0041 'A' A */
    0xD7, 0x5C,             /* [034] U+0042 'B' B */
    0x56, 0x54,             /* [035] U+0043 'C' C */
    0xD6, 0xDC,             /* [036] U+0044 'D' D */
    0xF3, 0xCE,             /* [037] U+0045 'E' E */
    0xF3, 0xC8,             /* [038] U+0046 'F' F */
    0x72, 0xD6,             /* [039] U+0047 'G' G */
    0xB7, 0xDA,             /* [040] U+0048 'H' H */
    0xF8,                   /* [041] U+0049 'I' I */
    0x24, 0xD4,             /* [042] U+004A 'J' J */
    0xB7, 0x5A,             /* [043] U+004B 'K' K */
    0x92, 0x4E,             /* [044] U+004C 'L' L */
    0x8E, 0xEB, 0x18, 0x80, /* [045] U+004D 'M' M */
    0x9D, 0xB9, 0x90,       /* [046] U+004E 'N' N */
    0x56, 0xD4,             /* [047] U+004F 'O' O */
    0xF7, 0x48,             /* [048] U+0050 'P' P */
    0x4A, 0xAA, 0x70,       /* [049] U+0051 'Q' Q */
    0xF7, 0x5A,             /* [050] U+0052 'R' R */
    0xF3, 0x9E,             /* [051] U+0053 'S' S */
    0xE9, 0x24,             /* [052] U+0054 'T' T */
    0xB6, 0xDE,             /* [053] U+0055 'U' U */
    0xB6, 0xD4,             /* [054] U+0056 'V' V */
    0x8C, 0x63, 0x55, 0x00, /* [055] U+0057 'W' W */
    0xB5, 0x5A,             /* [056] U+0058 'X' X */
    0xB7, 0x9C,             /* [057] U+0059 'Y' Y */
    0xE5, 0x4E,             /* [058] U+005A 'Z' Z */
    0xF2, 0x4E,             /* [059] U+005B '[' bracketleft */
    0x88, 0x80,             /* [060] U+005C '\' backslash */
    0xE4, 0x9E,             /* [061] U+005D ']' bracketright */
    0x54,                   /* [062] U+005E '^' asciicircum */
    0xE0,                   /* [063] U+005F '_' underscore */
    0x90,                   /* [064] U+0060 '`' grave */
    0xCE, 0xF0,             /* [065] U+0061 'a' a */
    0x9A, 0xDC,             /* [066] U+0062 'b' b */
    0x72, 0x30,             /* [067] U+0063 'c' c */
    0x2E, 0xD6,             /* [068] U+0064 'd' d */
    0x77, 0x30,             /* [069] U+0065 'e' e */
    0x2B, 0xA4,             /* [070] U+0066 'f' f */
    0x77, 0x94,             /* [071] U+0067 'g' g */
    0x9A, 0xDA,             /* [072] U+0068 'h' h */
    0xB8,                   /* [073] U+0069 'i' i */
    0x20, 0x9A, 0x80,       /* [074] U+006A 'j' j */
    0x97, 0x6A,             /* [075] U+006B 'k' k */
    0xC9, 0x2E,             /* [076] U+006C 'l' l */
    0xFF, 0xD0,             /* [077] U+006D 'm' m */
    0xD6, 0xD0,             /* [078] U+006E 'n' n */
    0x56, 0xA0,             /* [079] U+006F 'o' o */
    0xD6, 0xE8,             /* [080] U+0070 'p' p */
    0x76, 0xB2,             /* [081] U+0071 'q' q */
    0x72, 0x40,             /* [082] U+0072 'r' r */
    0x79, 0xE0,             /* [083] U+0073 's' s */
    0x5D, 0x26,             /* [084] U+0074 't' t */
    0xB6, 0xB0,             /* [085] U+0075 'u' u */
    0xB7, 0xA0,             /* [086] U+0076 'v' v */
    0xBF, 0xF0,             /* [087] U+0077 'w' w */
    0xA9, 0x50,             /* [088] U+0078 'x' x */
    0xB5, 0x94,             /* [089] U+0079 'y' y */
    0xEF, 0x70,             /* [090] U+007A 'z' z */
    0x6A, 0x26,             /* [091] U+007B '{' braceleft */
    0xD8,                   /* [092] U+007C '|' bar */
    0xC8, 0xAC,             /* [093] U+007D '}' braceright */
    0x78,                   /* [094] U+007E '~' asciitilde */
#if (TOMTHUMB_USE_EXTENDED)
    0xB8,                   /* [095] U+00A1 '¡' exclamdown */
    0x5E, 0x74,             /* [096] U+00A2 '¢' cent */
    0x6B, 0xAE,             /* [097] U+00A3 '£' sterling */
    0xAB, 0xAA,             /* [098] U+00A4 '¤' currency */
    0xB5, 0x74,             /* [099] U+00A5 '¥' yen */
    0xD8,                   /* [100] U+00A6 '¦' brokenbar */
    0x6A, 0xAC,             /* [101] U+00A7 '§' section */
    0xA0,                   /* [102] U+00A8 '¨' dieresis */
    0x71, 0x80,             /* [103] U+00A9 '©' copyright */
    0x77, 0x8E,             /* [104] U+00AA 'ª' ordfeminine */
    0x64,                   /* [105] U+00AB '«' guillemotleft */
    0xE4,                   /* [106] U+00AC '¬' logicalnot */
    0xC0,                   /* [107] U+00AD '­' softhyphen */
    0xDA, 0x80,             /* [108] U+00AE '®' registered */
    0xE0,                   /* [109] U+00AF '¯' macron */
    0xD8, 0x00,             /* [110] U+00B0 '°' degree */
    0x5D, 0x0E,             /* [111] U+00B1 '±' plusminus */
    0xC9, 0x80,             /* [112] U+00B2 '²' twosuperior */
    0xEF, 0x80,             /* [113] U+00B3 '³' threesuperior */
    0x60,                   /* [114] U+00B4 '´' acute */
    0xB6, 0xE8,             /* [115] U+00B5 'µ' mu */
    0x75, 0xB6,             /* [116] U+00B6 '¶' paragraph */
    0xFF, 0x80,             /* [117] U+00B7 '·' periodcentered */
    0x47, 0x00,             /* [118] U+00B8 '¸' cedilla */
    0xE0,                   /* [119] U+00B9 '¹' onesuperior */
    0x55, 0x0E,             /* [120] U+00BA 'º' ordmasculine */
    0x98,                   /* [121] U+00BB '»' guillemotright */
    0x90, 0x32,             /* [122] U+00BC '¼' onequarter */
    0x90, 0x66,             /* [123] U+00BD '½' onehalf */
    0xD8, 0x32,             /* [124] U+00BE '¾' threequarters */
    0x41, 0x4E,             /* [125] U+00BF '¿' questiondown */
    0x45, 0x7A,             /* [126] U+00C0 'À' Agrave */
    0x51, 0x7A,             /* [127] U+00C1 'Á' Aacute */
    0xE1, 0x7A,             /* [128] U+00C2 'Â' Acircumflex */
    0x79, 0x7A,             /* [129] U+00C3 'Ã' Atilde */
    0xAA, 0xFA,             /* [130] U+00C4 'Ä' Adieresis */
    0xDA, 0xFA,             /* [131] U+00C5 'Å' Aring */
    0x7B, 0xEE,             /* [132] U+00C6 'Æ' AE */
    0x72, 0x32, 0x80,       /* [133] U+00C7 'Ç' Ccedilla */
    0x47, 0xEE,             /* [134] U+00C8 'È' Egrave */
    0x53, 0xEE,             /* [135] U+00C9 'É' Eacute */
    0xE3, 0xEE,             /* [136] U+00CA 'Ê' Ecircumflex */
    0xA3, 0xEE,             /* [137] U+00CB 'Ë' Edieresis */
    0x47, 0xAE,             /* [138] U+00CC 'Ì' Igrave */
    0x53, 0xAE,             /* [139] U+00CD 'Í' Iacute */
    0xE3, 0xAE,             /* [140] U+00CE 'Î' Icircumflex */
    0xA3, 0xAE,             /* [141] U+00CF 'Ï' Idieresis */
    0xD7, 0xDC,             /* [142] U+00D0 'Ð' Eth */
    0xCE, 0xFA,             /* [143] U+00D1 'Ñ' Ntilde */
    0x47, 0xDE,             /* [144] U+00D2 'Ò' Ograve */
    0x53, 0xDE,             /* [145] U+00D3 'Ó' Oacute */
    0xE3, 0xDE,             /* [146] U+00D4 'Ô' Ocircumflex */
    0xCF, 0xDE,             /* [147] U+00D5 'Õ' Otilde */
    0xA3, 0xDE,             /* [148] U+00D6 'Ö' Odieresis */
    0xAA, 0x80,             /* [149] U+00D7 '×' multiply */
    0x77, 0xDC,             /* [150] U+00D8 'Ø' Oslash */
    0x8A, 0xDE,             /* [151] U+00D9 'Ù' Ugrave */
    0x2A, 0xDE,             /* [152] U+00DA 'Ú' Uacute */
    0xE2, 0xDE,             /* [153] U+00DB 'Û' Ucircumflex */
    0xA2, 0xDE,             /* [154] U+00DC 'Ü' Udieresis */
    0x2A, 0xF4,             /* [155] U+00DD 'Ý' Yacute */
    0x9E, 0xF8,             /* [156] U+00DE 'Þ' Thorn */
    0x77, 0x5D, 0x00,       /* [157] U+00DF 'ß' germandbls */
    0x45, 0xDE,             /* [158] U+00E0 'à' agrave */
    0x51, 0xDE,             /* [159] U+00E1 'á' aacute */
    0xE1, 0xDE,             /* [160] U+00E2 'â' acircumflex */
    0x79, 0xDE,             /* [161] U+00E3 'ã' atilde */
    0xA1, 0xDE,             /* [162] U+00E4 'ä' adieresis */
    0x6D, 0xDE,             /* [163] U+00E5 'å' aring */
    0x7F, 0xE0,             /* [164] U+00E6 'æ' ae */
    0x71, 0x94,             /* [165] U+00E7 'ç' ccedilla */
    0x45, 0xF6,             /* [166] U+00E8 'è' egrave */
    0x51, 0xF6,             /* [167] U+00E9 'é' eacute */
    0xE1, 0xF6,             /* [168] U+00EA 'ê' ecircumflex */
    0xA1, 0xF6,             /* [169] U+00EB 'ë' edieresis */
    0x9A, 0x80,             /* [170] U+00EC 'ì' igrave */
    0x65, 0x40,             /* [171] U+00ED 'í' iacute */
    0xE1, 0x24,             /* [172] U+00EE 'î' icircumflex */
    0xA1, 0x24,             /* [173] U+00EF 'ï' idieresis */
    0x79, 0xD6,             /* [174] U+00F0 'ð' eth */
    0xCF, 0x5A,             /* [175] U+00F1 'ñ' ntilde */
    0x45, 0x54,             /* [176] U+00F2 'ò' ograve */
    0x51, 0x54,             /* [177] U+00F3 'ó' oacute */
    0xE1, 0x54,             /* [178] U+00F4 'ô' ocircumflex */
    0xCD, 0x54,             /* [179] U+00F5 'õ' otilde */
    0xA1, 0x54,             /* [180] U+00F6 'ö' odieresis */
    0x43, 0x84,             /* [181] U+00F7 '÷' divide */
    0x7E, 0xE0,             /* [182] U+00F8 'ø' oslash */
    0x8A, 0xD6,             /* [183] U+00F9 'ù' ugrave */
    0x2A, 0xD6,             /* [184] U+00FA 'ú' uacute */
    0xE2, 0xD6,             /* [185] U+00FB 'û' ucircumflex */
    0xA2, 0xD6,             /* [186] U+00FC 'ü' udieresis */
    0x2A, 0xB2, 0x80,       /* [187] U+00FD 'ý' yacute */
    0x9A, 0xE8,             /* [188] U+00FE 'þ' thorn */
    0xA2, 0xB2, 0x80,       /* [189] U+00FF 'ÿ' ydieresis */
    0x00,                   /* [190] U+011D 'ĝ' gcircumflex */
    0x7B, 0xE6,             /* [191] U+0152 'Œ' OE */
    0x7F, 0x70,             /* [192] U+0153 'œ' oe */
    0xAF, 0x3C,             /* [193] U+0160 'Š' Scaron */
    0xAF, 0x3C,             /* [194] U+0161 'š' scaron */
    0xA2, 0xA4,             /* [195] U+0178 'Ÿ' Ydieresis */
    0xBD, 0xEE,             /* [196] U+017D 'Ž' Zcaron */
    0xBD, 0xEE,             /* [197] U+017E 'ž' zcaron */
    0x00,                   /* [198] U+0EA4 '຤' uni0EA4 */
    0x00,                   /* [199] U+13A0 'Ꭰ' uni13A0 */
    0x80,                   /* [200] U+2022 '•' bullet */
    0xA0,                   /* [201] U+2026 '…' ellipsis */
    0x7F, 0xE6,             /* [202] U+20AC '€' Euro */
    0xEA, 0xAA, 0xE0,       /* [203] U+FFFD '�' uniFFFD */
#endif /* (TOMTHUMB_USE_EXTENDED)  */
};


/* {offset, width, height, advance cursor, x offset, y offset} */
const GFXglyph TomThumbGlyphs[] PROGMEM = {
    {   0, 1, 1, 2, 0, -5 },    /* [000] U+0020 ' ' space */
    {   1, 1, 5, 2, 0, -5 },    /* [001] U+0021 '!' exclam */
    {   2, 3, 2, 4, 0, -5 },    /* [002] U+0022 '"' quotedbl */
    {   3, 3, 5, 4, 0, -5 },    /* [003] U+0023 '#' numbersign */
    {   5, 3, 5, 4, 0, -5 },    /* [004] U+0024 '$' dollar */
    {   7, 3, 5, 4, 0, -5 },    /* [005] U+0025 '%' percent */
    {   9, 3, 5, 4, 0, -5 },    /* [006] U+0026 '&' ampersand */
    {  11, 1, 2, 2, 0, -5 },    /* [007] U+0027 ''' quotesingle */
    {  12, 2, 5, 3, 0, -5 },    /* [008] U+0028 '(' parenleft */
    {  14, 2, 5, 3, 0, -5 },    /* [009] U+0029 ')' parenright */
    {  16, 3, 3, 4, 0, -5 },    /* [010] U+002A '*' asterisk */
    {  18, 3, 3, 4, 0, -4 },    /* [011] U+002B '+' plus */
    {  20, 2, 2, 3, 0, -2 },    /* [012] U+002C ',' comma */
    {  21, 3, 1, 4, 0, -3 },    /* [013] U+002D '-' hyphen */
    {  22, 1, 1, 2, 0, -1 },    /* [014] U+002E '.' period */
    {  23, 3, 5, 4, 0, -5 },    /* [015] U+002F '/' slash */
    {  25, 3, 5, 4, 0, -5 },    /* [016] U+0030 '0' zero */
    {  27, 3, 5, 4, 0, -5 },    /* [017] U+0031 '1' one */
    {  29, 3, 5, 4, 0, -5 },    /* [018] U+0032 '2' two */
    {  31, 3, 5, 4, 0, -5 },    /* [019] U+0033 '3' three */
    {  33, 3, 5, 4, 0, -5 },    /* [020] U+0034 '4' four */
    {  35, 3, 5, 4, 0, -5 },    /* [021] U+0035 '5' five */
    {  37, 3, 5, 4, 0, -5 },    /* [022] U+0036 '6' six */
    {  39, 3, 5, 4, 0, -5 },    /* [023] U+0037 '7' seven */
    {  41, 3, 5, 4, 0, -5 },    /* [024] U+0038 '8' eight */
    {  43, 3, 5, 4, 0, -5 },    /* [025] U+0039 '9' nine */
    {  45, 1, 3, 2, 0, -4 },    /* [026] U+003A ':' colon */
    {  46, 2, 4, 3, 0, -4 },    /* [027] U+003B ';' semicolon */
    {  47, 3, 5, 4, 0, -5 },    /* [028] U+003C '<' less */
    {  49, 3, 3, 4, 0, -4 },    /* [029] U+003D '=' equal */
    {  51, 3, 5, 4, 0, -5 },    /* [030] U+003E '>' greater */
    {  53, 3, 5, 4, 0, -5 },    /* [031] U+003F '?' question */
    {  55, 3, 5, 4, 0, -5 },    /* [032] U+0040 '@' at */
    {  57, 3, 5, 4, 0, -5 },    /* [033] U+0041 'A' A */
    {  59, 3, 5, 4, 0, -5 },    /* [034] U+0042 'B' B */
    {  61, 3, 5, 4, 0, -5 },    /* [035] U+0043 'C' C */
    {  63, 3, 5, 4, 0, -5 },    /* [036] U+0044 'D' D */
    {  65, 3, 5, 4, 0, -5 },    /* [037] U+0045 'E' E */
    {  67, 3, 5, 4, 0, -5 },    /* [038] U+0046 'F' F */
    {  69, 3, 5, 4, 0, -5 },    /* [039] U+0047 'G' G */
    {  71, 3, 5, 4, 0, -5 },    /* [040] U+0048 'H' H */
    {  73, 1, 5, 2, 0, -5 },    /* [041] U+0049 'I' I */
    {  74, 3, 5, 4, 0, -5 },    /* [042] U+004A 'J' J */
    {  76, 3, 5, 4, 0, -5 },    /* [043] U+004B 'K' K */
    {  78, 3, 5, 4, 0, -5 },    /* [044] U+004C 'L' L */
    {  80, 5, 5, 6, 0, -5 },    /* [045] U+004D 'M' M */
    {  84, 4, 5, 5, 0, -5 },    /* [046] U+004E 'N' N */
    {  87, 3, 5, 4, 0, -5 },    /* [047] U+004F 'O' O */
    {  89, 3, 5, 4, 0, -5 },    /* [048] U+0050 'P' P */
    {  91, 4, 5, 5, 0, -5 },    /* [049] U+0051 'Q' Q */
    {  94, 3, 5, 4, 0, -5 },    /* [050] U+0052 'R' R */
    {  96, 3, 5, 4, 0, -5 },    /* [051] U+0053 'S' S */
    {  98, 3, 5, 4, 0, -5 },    /* [052] U+0054 'T' T */
    { 100, 3, 5, 4, 0, -5 },    /* [053] U+0055 'U' U */
    { 102, 3, 5, 4, 0, -5 },    /* [054] U+0056 'V' V */
    { 104, 5, 5, 6, 0, -5 },    /* [055] U+0057 'W' W */
    { 108, 3, 5, 4, 0, -5 },    /* [056] U+0058 'X' X */
    { 110, 3, 5, 4, 0, -5 },    /* [057] U+0059 'Y' Y */
    { 112, 3, 5, 4, 0, -5 },    /* [058] U+005A 'Z' Z */
    { 114, 3, 5, 4, 0, -5 },    /* [059] U+005B '[' bracketleft */
    { 116, 3, 3, 4, 0, -4 },    /* [060] U+005C '\' backslash */
    { 118, 3, 5, 4, 0, -5 },    /* [061] U+005D ']' bracketright */
    { 120, 3, 2, 4, 0, -5 },    /* [062] U+005E '^' asciicircum */
    { 121, 3, 1, 4, 0, -1 },    /* [063] U+005F '_' underscore */
    { 122, 2, 2, 3, 0, -5 },    /* [064] U+0060 '`' grave */
    { 123, 3, 4, 4, 0, -4 },    /* [065] U+0061 'a' a */
    { 125, 3, 5, 4, 0, -5 },    /* [066] U+0062 'b' b */
    { 127, 3, 4, 4, 0, -4 },    /* [067] U+0063 'c' c */
    { 129, 3, 5, 4, 0, -5 },    /* [068] U+0064 'd' d */
    { 131, 3, 4, 4, 0, -4 },    /* [069] U+0065 'e' e */
    { 133, 3, 5, 4, 0, -5 },    /* [070] U+0066 'f' f */
    { 135, 3, 5, 4, 0, -4 },    /* [071] U+0067 'g' g */
    { 137, 3, 5, 4, 0, -5 },    /* [072] U+0068 'h' h */
    { 139, 1, 5, 2, 0, -5 },    /* [073] U+0069 'i' i */
    { 140, 3, 6, 4, 0, -5 },    /* [074] U+006A 'j' j */
    { 143, 3, 5, 4, 0, -5 },    /* [075] U+006B 'k' k */
    { 145, 3, 5, 4, 0, -5 },    /* [076] U+006C 'l' l */
    { 147, 3, 4, 4, 0, -4 },    /* [077] U+006D 'm' m */
    { 149, 3, 4, 4, 0, -4 },    /* [078] U+006E 'n' n */
    { 151, 3, 4, 4, 0, -4 },    /* [079] U+006F 'o' o */
    { 153, 3, 5, 4, 0, -4 },    /* [080] U+0070 'p' p */
    { 155, 3, 5, 4, 0, -4 },    /* [081] U+0071 'q' q */
    { 157, 3, 4, 4, 0, -4 },    /* [082] U+0072 'r' r */
    { 159, 3, 4, 4, 0, -4 },    /* [083] U+0073 's' s */
    { 161, 3, 5, 4, 0, -5 },    /* [084] U+0074 't' t */
    { 163, 3, 4, 4, 0, -4 },    /* [085] U+0075 'u' u */
    { 165, 3, 4, 4, 0, -4 },    /* [086] U+0076 'v' v */
    { 167, 3, 4, 4, 0, -4 },    /* [087] U+0077 'w' w */
    { 169, 3, 4, 4, 0, -4 },    /* [088] U+0078 'x' x */
    { 171, 3, 5, 4, 0, -4 },    /* [089] U+0079 'y' y */
    { 173, 3, 4, 4, 0, -4 },    /* [090] U+007A 'z' z */
    { 175, 3, 5, 4, 0, -5 },    /* [091] U+007B '{' braceleft */
    { 177, 1, 5, 2, 0, -5 },    /* [092] U+007C '|' bar */
    { 178, 3, 5, 4, 0, -5 },    /* [093] U+007D '}' braceright */
    { 180, 3, 2, 4, 0, -5 },    /* [094] U+007E '~' asciitilde */
#if (TOMTHUMB_USE_EXTENDED)
    { 181, 1, 5, 2, 0, -5 },    /* [095] U+00A1 '¡' exclamdown */
    { 182, 3, 5, 4, 0, -5 },    /* [096] U+00A2 '¢' cent */
    { 184, 3, 5, 4, 0, -5 },    /* [097] U+00A3 '£' sterling */
    { 186, 3, 5, 4, 0, -5 },    /* [098] U+00A4 '¤' currency */
    { 188, 3, 5, 4, 0, -5 },    /* [099] U+00A5 '¥' yen */
    { 190, 1, 5, 2, 0, -5 },    /* [100] U+00A6 '¦' brokenbar */
    { 191, 3, 5, 4, 0, -5 },    /* [101] U+00A7 '§' section */
    { 193, 3, 1, 4, 0, -5 },    /* [102] U+00A8 '¨' dieresis */
    { 194, 3, 3, 4, 0, -5 },    /* [103] U+00A9 '©' copyright */
    { 196, 3, 5, 4, 0, -5 },    /* [104] U+00AA 'ª' ordfeminine */
    { 198, 2, 3, 3, 0, -5 },    /* [105] U+00AB '«' guillemotleft */
    { 199, 3, 2, 4, 0, -4 },    /* [106] U+00AC '¬' logicalnot */
    { 200, 2, 1, 3, 0, -3 },    /* [107] U+00AD '­' softhyphen */
    { 201, 3, 3, 4, 0, -5 },    /* [108] U+00AE '®' registered */
    { 203, 3, 1, 4, 0, -5 },    /* [109] U+00AF '¯' macron */
    { 204, 3, 3, 4, 0, -5 },    /* [110] U+00B0 '°' degree */
    { 206, 3, 5, 4, 0, -5 },    /* [111] U+00B1 '±' plusminus */
    { 208, 3, 3, 4, 0, -5 },    /* [112] U+00B2 '²' twosuperior */
    { 210, 3, 3, 4, 0, -5 },    /* [113] U+00B3 '³' threesuperior */
    { 212, 2, 2, 3, 0, -5 },    /* [114] U+00B4 '´' acute */
    { 213, 3, 5, 4, 0, -5 },    /* [115] U+00B5 'µ' mu */
    { 215, 3, 5, 4, 0, -5 },    /* [116] U+00B6 '¶' paragraph */
    { 217, 3, 3, 4, 0, -4 },    /* [117] U+00B7 '·' periodcentered */
    { 219, 3, 3, 4, 0, -3 },    /* [118] U+00B8 '¸' cedilla */
    { 221, 1, 3, 2, 0, -5 },    /* [119] U+00B9 '¹' onesuperior */
    { 222, 3, 5, 4, 0, -5 },    /* [120] U+00BA 'º' ordmasculine */
    { 224, 2, 3, 3, 0, -5 },    /* [121] U+00BB '»' guillemotright */
    { 225, 3, 5, 4, 0, -5 },    /* [122] U+00BC '¼' onequarter */
    { 227, 3, 5, 4, 0, -5 },    /* [123] U+00BD '½' onehalf */
    { 229, 3, 5, 4, 0, -5 },    /* [124] U+00BE '¾' threequarters */
    { 231, 3, 5, 4, 0, -5 },    /* [125] U+00BF '¿' questiondown */
    { 233, 3, 5, 4, 0, -5 },    /* [126] U+00C0 'À' Agrave */
    { 235, 3, 5, 4, 0, -5 },    /* [127] U+00C1 'Á' Aacute */
    { 237, 3, 5, 4, 0, -5 },    /* [128] U+00C2 'Â' Acircumflex */
    { 239, 3, 5, 4, 0, -5 },    /* [129] U+00C3 'Ã' Atilde */
    { 241, 3, 5, 4, 0, -5 },    /* [130] U+00C4 'Ä' Adieresis */
    { 243, 3, 5, 4, 0, -5 },    /* [131] U+00C5 'Å' Aring */
    { 245, 3, 5, 4, 0, -5 },    /* [132] U+00C6 'Æ' AE */
    { 247, 3, 6, 4, 0, -5 },    /* [133] U+00C7 'Ç' Ccedilla */
    { 250, 3, 5, 4, 0, -5 },    /* [134] U+00C8 'È' Egrave */
    { 252, 3, 5, 4, 0, -5 },    /* [135] U+00C9 'É' Eacute */
    { 254, 3, 5, 4, 0, -5 },    /* [136] U+00CA 'Ê' Ecircumflex */
    { 256, 3, 5, 4, 0, -5 },    /* [137] U+00CB 'Ë' Edieresis */
    { 258, 3, 5, 4, 0, -5 },    /* [138] U+00CC 'Ì' Igrave */
    { 260, 3, 5, 4, 0, -5 },    /* [139] U+00CD 'Í' Iacute */
    { 262, 3, 5, 4, 0, -5 },    /* [140] U+00CE 'Î' Icircumflex */
    { 264, 3, 5, 4, 0, -5 },    /* [141] U+00CF 'Ï' Idieresis */
    { 266, 3, 5, 4, 0, -5 },    /* [142] U+00D0 'Ð' Eth */
    { 268, 3, 5, 4, 0, -5 },    /* [143] U+00D1 'Ñ' Ntilde */
    { 270, 3, 5, 4, 0, -5 },    /* [144] U+00D2 'Ò' Ograve */
    { 272, 3, 5, 4, 0, -5 },    /* [145] U+00D3 'Ó' Oacute */
    { 274, 3, 5, 4, 0, -5 },    /* [146] U+00D4 'Ô' Ocircumflex */
    { 276, 3, 5, 4, 0, -5 },    /* [147] U+00D5 'Õ' Otilde */
    { 278, 3, 5, 4, 0, -5 },    /* [148] U+00D6 'Ö' Odieresis */
    { 280, 3, 3, 4, 0, -4 },    /* [149] U+00D7 '×' multiply */
    { 282, 3, 5, 4, 0, -5 },    /* [150] U+00D8 'Ø' Oslash */
    { 284, 3, 5, 4, 0, -5 },    /* [151] U+00D9 'Ù' Ugrave */
    { 286, 3, 5, 4, 0, -5 },    /* [152] U+00DA 'Ú' Uacute */
    { 288, 3, 5, 4, 0, -5 },    /* [153] U+00DB 'Û' Ucircumflex */
    { 290, 3, 5, 4, 0, -5 },    /* [154] U+00DC 'Ü' Udieresis */
    { 292, 3, 5, 4, 0, -5 },    /* [155] U+00DD 'Ý' Yacute */
    { 294, 3, 5, 4, 0, -5 },    /* [156] U+00DE 'Þ' Thorn */
    { 296, 3, 6, 4, 0, -5 },    /* [157] U+00DF 'ß' germandbls */
    { 299, 3, 5, 4, 0, -5 },    /* [158] U+00E0 'à' agrave */
    { 301, 3, 5, 4, 0, -5 },    /* [159] U+00E1 'á' aacute */
    { 303, 3, 5, 4, 0, -5 },    /* [160] U+00E2 'â' acircumflex */
    { 305, 3, 5, 4, 0, -5 },    /* [161] U+00E3 'ã' atilde */
    { 307, 3, 5, 4, 0, -5 },    /* [162] U+00E4 'ä' adieresis */
    { 309, 3, 5, 4, 0, -5 },    /* [163] U+00E5 'å' aring */
    { 311, 3, 4, 4, 0, -4 },    /* [164] U+00E6 'æ' ae */
    { 313, 3, 5, 4, 0, -4 },    /* [165] U+00E7 'ç' ccedilla */
    { 315, 3, 5, 4, 0, -5 },    /* [166] U+00E8 'è' egrave */
    { 317, 3, 5, 4, 0, -5 },    /* [167] U+00E9 'é' eacute */
    { 319, 3, 5, 4, 0, -5 },    /* [168] U+00EA 'ê' ecircumflex */
    { 321, 3, 5, 4, 0, -5 },    /* [169] U+00EB 'ë' edieresis */
    { 323, 2, 5, 3, 0, -5 },    /* [170] U+00EC 'ì' igrave */
    { 325, 2, 5, 3, 0, -5 },    /* [171] U+00ED 'í' iacute */
    { 327, 3, 5, 4, 0, -5 },    /* [172] U+00EE 'î' icircumflex */
    { 329, 3, 5, 4, 0, -5 },    /* [173] U+00EF 'ï' idieresis */
    { 331, 3, 5, 4, 0, -5 },    /* [174] U+00F0 'ð' eth */
    { 333, 3, 5, 4, 0, -5 },    /* [175] U+00F1 'ñ' ntilde */
    { 335, 3, 5, 4, 0, -5 },    /* [176] U+00F2 'ò' ograve */
    { 337, 3, 5, 4, 0, -5 },    /* [177] U+00F3 'ó' oacute */
    { 339, 3, 5, 4, 0, -5 },    /* [178] U+00F4 'ô' ocircumflex */
    { 341, 3, 5, 4, 0, -5 },    /* [179] U+00F5 'õ' otilde */
    { 343, 3, 5, 4, 0, -5 },    /* [180] U+00F6 'ö' odieresis */
    { 345, 3, 5, 4, 0, -5 },    /* [181] U+00F7 '÷' divide */
    { 347, 3, 4, 4, 0, -4 },    /* [182] U+00F8 'ø' oslash */
    { 349, 3, 5, 4, 0, -5 },    /* [183] U+00F9 'ù' ugrave */
    { 351, 3, 5, 4, 0, -5 },    /* [184] U+00FA 'ú' uacute */
    { 353, 3, 5, 4, 0, -5 },    /* [185] U+00FB 'û' ucircumflex */
    { 355, 3, 5, 4, 0, -5 },    /* [186] U+00FC 'ü' udieresis */
    { 357, 3, 6, 4, 0, -5 },    /* [187] U+00FD 'ý' yacute */
    { 360, 3, 5, 4, 0, -4 },    /* [188] U+00FE 'þ' thorn */
    { 362, 3, 6, 4, 0, -5 },    /* [189] U+00FF 'ÿ' ydieresis */
    { 365, 1, 1, 2, 0, -1 },    /* [190] U+011D 'ĝ' gcircumflex */
    { 366, 3, 5, 4, 0, -5 },    /* [191] U+0152 'Œ' OE */
    { 368, 3, 4, 4, 0, -4 },    /* [192] U+0153 'œ' oe */
    { 370, 3, 5, 4, 0, -5 },    /* [193] U+0160 'Š' Scaron */
    { 372, 3, 5, 4, 0, -5 },    /* [194] U+0161 'š' scaron */
    { 374, 3, 5, 4, 0, -5 },    /* [195] U+0178 'Ÿ' Ydieresis */
    { 376, 3, 5, 4, 0, -5 },    /* [196] U+017D 'Ž' Zcaron */
    { 378, 3, 5, 4, 0, -5 },    /* [197] U+017E 'ž' zcaron */
    { 380, 1, 1, 2, 0, -1 },    /* [198] U+0EA4 '຤' uni0EA4 */
    { 381, 1, 1, 2, 0, -1 },    /* [199] U+13A0 'Ꭰ' uni13A0 */
    { 382, 1, 1, 2, 0, -3 },    /* [200] U+2022 '•' bullet */
    { 383, 3, 1, 4, 0, -1 },    /* [201] U+2026 '…' ellipsis */
    { 384, 3, 5, 4, 0, -5 },    /* [202] U+20AC '€' Euro */
    { 386, 4, 5, 5, 0, -5 },    /* [203] U+FFFD '�' uniFFFD */
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
