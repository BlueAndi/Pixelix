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
 * @brief  JSON file handler
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "JsonFile.h"

#ifndef NATIVE

#define STREAMUTILS_ENABLE_EEPROM 0
#include <StreamUtils.h>

#endif  /* NATIVE */

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and classes
 *****************************************************************************/

/******************************************************************************
 * Prototypes
 *****************************************************************************/

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/******************************************************************************
 * Public Methods
 *****************************************************************************/

bool JsonFile::load(const String& fileName, JsonDocument& doc)
{
    bool    isSuccessful    = false;
    File    fd              = m_fs.open(fileName, "r");

    if (true == fd)
    {
#ifdef NATIVE
        DeserializationError    error   = deserializeJson(doc, fd);
#else   /* NATIVE */
        ReadBufferingStream     bufferedStream(fd, CHUNK_SIZE);
        DeserializationError    error   = deserializeJson(doc, bufferedStream);
#endif  /* NATIVE */

        if (DeserializationError::Ok == error.code())
        {
            isSuccessful = true;
        }

        fd.close();
    }

    return isSuccessful;
}

bool JsonFile::save(const String& fileName, const JsonDocument& doc)
{
    bool    isSuccessful    = false;
    File    fd              = m_fs.open(fileName, "w");

    if (true == fd)
    {
#ifndef NATIVE
        WriteBufferingStream    bufferedStream(fd, CHUNK_SIZE);
#endif  /* NATIVE */

        size_t                  write   = measureJsonPretty(doc);
        
#ifdef NATIVE        
        if (write == serializeJsonPretty(doc, fd))
#else   /* NATIVE */
        if (write == serializeJsonPretty(doc, bufferedStream))
#endif  /* NATIVE */
        {
            isSuccessful = true;
        }

#ifndef NATIVE
        bufferedStream.flush();
#endif  /* NATIVE*/

        fd.close();
    }

    return isSuccessful;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/