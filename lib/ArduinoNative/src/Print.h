/* MIT License
 *
 * Copyright (c) 2019 - 2023 Andreas Merkle <web@blue-andi.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*******************************************************************************
    DESCRIPTION
*******************************************************************************/
/**
 * @brief  Print implementation for test
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup test
 *
 * @{
 */

#ifndef PRINT_H
#define PRINT_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>

#include "WString.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/** Decimal number base */
#define DEC (10)

/** Hexadecimal number base */
#define HEX (16)

/** Octal number base */
#define OCT (8)

/** Binary number base */
#define BIN (2)

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Print class for test purposes only.
 * Note, the source implementation is from the orignal Arduino source.
 */
class Print
{
public:

    /**
     * Constructs a print object.
     */
    Print()
    {
    }

    /**
     * Destroys a print object.
     */
    virtual ~Print()
    {
    }

    /**
     * Write a single byte to the output stream.
     *
     * @param[in] data  Data byte
     * @return Number of written data bytes.
     */
    virtual size_t write(uint8_t data) = 0;

    /**
     * Write several data bytes to the output stream.
     *
     * @param[in] buffer    Data buffer
     * @param[in] size      Data buffer size
     * @return Number of written data bytes.
     */
    virtual size_t write(const uint8_t *buffer, size_t size);

    /**
     * Write a single character to the output stream.
     *
     * @param[in] str   Character
     * @return Number of written characters.
     */
    size_t write(const char *str);

    /**
     * Write several string characters to the output stream.
     *
     * @param[in] buffer    Character buffer
     * @param[in] size      Character buffer size
     * @return Number of written characters.
     */
    size_t write(const char *buffer, size_t size);

    /**
     * Print a string.
     *
     * @param[in] s String
     * @return Number of written characters.
     */
    size_t print(const String &s);

    /**
     * Print a string.
     *
     * @param[in] str String
     * @return Number of written characters.
     */
    size_t print(const char str[]);

    /**
     * Print single character.
     *
     * @param[in] c Character
     * @return Number of written characters.
     */
    size_t print(char c);

    /**
     * Print a long integer.
     *
     * @param[in] n     Long integer
     * @param[in] base  Number base
     * @return Number of written characters.
     */
    size_t print(long n, int base = DEC);

    /**
     * Print a cariage return and new line.
     *
     * @return Number of written characters.
     */
    size_t println(void);

    /**
     * Print a string with a carriage return and new line at the end.
     *
     * @param[in] s String
     * @return Number of written characters.
     */
    size_t println(const String &s);

private:

    /**
     * Print any kind of number, depended on the number base.
     *
     * @param[in] n     Number to print
     * @param[in] base  Number base
     * @return Printed number of characters.
     */
    size_t printNumber(unsigned long n, uint8_t base);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* PRINT_H */

/** @} */