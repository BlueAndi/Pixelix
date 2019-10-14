/* MIT License
 *
 * Copyright (c) 2019 Andreas Merkle <web@blue-andi.de>
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
 * @brief  HTML helper functions
 * @author Andreas Merkle <web@blue-andi.de>
 * 
 * @addtogroup web
 *
 * @{
 */

#ifndef __HTML_H__
#define __HTML_H__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <WString.h>
#include <stdint.h>

/** HTML helper functions */
namespace Html
{

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * This type defines supported HTTP response status codes.
 */
enum StatusCode
{
    STATUS_CODE_OK              = 200,  /**< Ok */
    STATUS_CODE_BAD_REQ         = 400,  /**< Bad request */
    STATUS_CODE_UNAUTHORIZED    = 401,  /**< Unauthorized */
    STATUS_CODE_FORBIDDEN       = 403,  /**< Forbidden */
    STATUS_CODE_NOT_FOUND       = 404   /**< Not found */
};

/******************************************************************************
 * Functions
 *****************************************************************************/

/**
 * Generate a HTML5 input field.
 * 
 * @param[in] name  Name of the input field.
 * @param[in] value Value, which shown in the input field.
 * @param[in] size  Input field size.
 * @param[in] min   Min. number of characters in the input field.
 * @param[in] max   Max. number of characters in the input field.
 * 
 * @return HTML5 input
 */
String inputText(const String& name, const String& value, uint8_t size, uint8_t min, uint8_t max);

}

#endif  /* __HTML_H__ */

/** @} */