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
 *
 * @addtogroup common
 *
 * @{
 */

#ifndef JSON_FILE_H
#define JSON_FILE_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <ArduinoJson.h>
#include <FS.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * JSON file handler, which uses buffered i/o access to improve performance.
 */
class JsonFile
{
public:

    /**
     * Constructs the JSON file handler.
     * 
     * @param[in] fs    Filesystem
     */
    JsonFile(FS& fs) :
        m_fs(fs)
    {
    }

    /**
     * Constructs the JSON file handler, using the same filesystem as the
     * assigned file handler.
     * 
     * @param[in] jsonFile  JSON file handler
     */
    JsonFile(const JsonFile& jsonFile) :
        m_fs(jsonFile.m_fs)
    {
    }

    /**
     * Destroys the JSON file handler.
     */
    ~JsonFile()
    {
    }

    /**
     * Load JSON file.
     * 
     * @param[in] fileName  Name of the JSON file.
     * @param[in] doc       JSON document, which shall contain the loaded content.
     * 
     * @return If successful, it will return true otherwise false.
     */
    bool load(const String& fileName, JsonDocument& doc);

    /**
     * Save JSON file.
     * 
     * @param[in] fileName  Name of the JSON file.
     * @param[in] doc       JSON document, which contain the content to save.
     * 
     * @return If successful, it will return true otherwise false.
     */
    bool save(const String& fileName, const JsonDocument& doc);

protected:

private:

    /**
     * Chunk size in byte, used by buffered stream access.
     * This influences the file read performance.
     */
    static const size_t CHUNK_SIZE  = 64U;

    FS  m_fs;   /**< Filesystem */

    JsonFile();
    JsonFile& operator=(const JsonFile& jsonFile);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* JSON_FILE_H */

/** @} */