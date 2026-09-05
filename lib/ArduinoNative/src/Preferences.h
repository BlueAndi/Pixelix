/* MIT License
 *
 * Copyright (c) 2019 - 2026 Andreas Merkle <web@blue-andi.de>
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
 * @file   Preferences.h
 * @brief  Preferences for test purposes only
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * Counterpart of the ESP32 Arduino Preferences, which stores the key/value
 * pairs in the non-volatile storage. On the host they are kept in memory,
 * therefore they are lost after the program ended.
 *
 * @addtogroup TEST
 *
 * @{
 */

#ifndef PREFERENCES_H
#define PREFERENCES_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <stddef.h>
#include <string>

#include "WString.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Preferences class for test purposes only.
 *
 * Every value is kept as string in memory, like the ESP32 counterpart does it
 * in the non-volatile storage. The storage is shared by all instances and
 * survives begin()/end() cycles, but not the program end.
 */
class Preferences
{
public:

    /**
     * Constructs the preferences.
     */
    Preferences();

    /**
     * Destroys the preferences.
     */
    ~Preferences();

    /**
     * Open the given namespace.
     *
     * @param[in] name      Namespace name.
     * @param[in] readOnly  Open read-only or read/write.
     *
     * @return If successful opened, it will return true otherwise false.
     */
    bool begin(const char* name, bool readOnly = false);

    /**
     * Close the namespace.
     */
    void end();

    /**
     * Remove all key/value pairs of the namespace.
     *
     * @return If successful removed, it will return true otherwise false.
     */
    bool clear();

    /**
     * Remove the key/value pair with the given key.
     *
     * @param[in] key   Key of the key/value pair.
     *
     * @return If successful removed, it will return true otherwise false.
     */
    bool remove(const char* key);

    /**
     * Get boolean value.
     *
     * @param[in] key       Key of the key/value pair.
     * @param[in] defValue  Default value, used if the key is unknown.
     *
     * @return Value
     */
    bool getBool(const char* key, bool defValue = false);

    /**
     * Set boolean value.
     *
     * @param[in] key   Key of the key/value pair.
     * @param[in] value Value
     *
     * @return Number of stored bytes.
     */
    size_t putBool(const char* key, bool value);

    /**
     * Get signed 32-bit value.
     *
     * @param[in] key       Key of the key/value pair.
     * @param[in] defValue  Default value, used if the key is unknown.
     *
     * @return Value
     */
    int32_t getInt(const char* key, int32_t defValue = 0);

    /**
     * Set signed 32-bit value.
     *
     * @param[in] key   Key of the key/value pair.
     * @param[in] value Value
     *
     * @return Number of stored bytes.
     */
    size_t putInt(const char* key, int32_t value);

    /**
     * Get unsigned 32-bit value.
     *
     * @param[in] key       Key of the key/value pair.
     * @param[in] defValue  Default value, used if the key is unknown.
     *
     * @return Value
     */
    uint32_t getUInt(const char* key, uint32_t defValue = 0U);

    /**
     * Set unsigned 32-bit value.
     *
     * @param[in] key   Key of the key/value pair.
     * @param[in] value Value
     *
     * @return Number of stored bytes.
     */
    size_t putUInt(const char* key, uint32_t value);

    /**
     * Get unsigned 8-bit value.
     *
     * @param[in] key       Key of the key/value pair.
     * @param[in] defValue  Default value, used if the key is unknown.
     *
     * @return Value
     */
    uint8_t getUChar(const char* key, uint8_t defValue = 0U);

    /**
     * Set unsigned 8-bit value.
     *
     * @param[in] key   Key of the key/value pair.
     * @param[in] value Value
     *
     * @return Number of stored bytes.
     */
    size_t putUChar(const char* key, uint8_t value);

    /**
     * Get string value.
     *
     * @param[in] key       Key of the key/value pair.
     * @param[in] defValue  Default value, used if the key is unknown.
     *
     * @return Value
     */
    String getString(const char* key, const String& defValue = String());

    /**
     * Set string value.
     *
     * @param[in] key   Key of the key/value pair.
     * @param[in] value Value
     *
     * @return Number of stored bytes.
     */
    size_t putString(const char* key, const String& value);

private:

    std::string m_namespace;  /**< Name of the opened namespace. */
    bool        m_isOpen;     /**< Is a namespace opened? */
    bool        m_isReadOnly; /**< Is the opened namespace read-only? */

    Preferences(const Preferences& pref);
    Preferences& operator=(const Preferences& pref);

    /**
     * Is a namespace opened for writing?
     *
     * @return If writable, it will return true otherwise false.
     */
    bool isWritable() const;

    /**
     * Get the raw value of the given key.
     *
     * @param[in]  key      Key of the key/value pair.
     * @param[out] value    Value of the key/value pair.
     *
     * @return If the key is known, it will return true otherwise false.
     */
    bool getValue(const char* key, std::string& value) const;

    /**
     * Set the raw value of the given key.
     *
     * @param[in] key   Key of the key/value pair.
     * @param[in] value Value of the key/value pair.
     *
     * @return Number of stored bytes.
     */
    size_t putValue(const char* key, const std::string& value);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* PREFERENCES_H */

/** @} */
