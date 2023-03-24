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
 * @brief  Key value pair with JSON type
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup settings
 *
 * @{
 */

#ifndef KEY_VALUE_JSON_H
#define KEY_VALUE_JSON_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "KeyValue.h"
#include <ArduinoJson.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Key value pair with JSON value.
 */
class KeyValueJson : public KeyValue
{
public:

    /**
     * Constructs a key value pair.
     */
    KeyValueJson(const char* key, const char* name, const char* defValue, size_t min, size_t max) :
        KeyValue(),
        m_key(key),
        m_name(name),
        m_defValue(defValue),
        m_min(min),
        m_max(max)
    {
    }

    /**
     * Constructs a key value pair.
     */
    KeyValueJson(Preferences& pref, const char* key, const char* name, const char* defValue, size_t min, size_t max) :
        KeyValue(pref),
        m_key(key),
        m_name(name),
        m_defValue(defValue),
        m_min(min),
        m_max(max)
    {
    }

    /**
     * Destroys a key value pair.
     */
    virtual ~KeyValueJson()
    {
    }

    /**
     * Get value type.
     *
     * @return Value type
     */
    Type getValueType() const final
    {
        return TYPE_JSON;
    }

    /**
     * Get user friendly name of key value pair.
     *
     * @return User friendly name
     */
    const char* getName() const final
    {
        return m_name;
    }

    /**
     * Get key.
     *
     * @return Key
     */
    const char* getKey() const final
    {
        return m_key;
    }

    /**
     * Get minimum string length.
     *
     * @return Minimum string length
     */
    size_t getMinLength() const
    {
        return m_min;
    }

    /**
     * Get maximum string length.
     *
     * @return Maximum string length
     */
    size_t getMaxLength() const
    {
        return m_max;
    }

    /**
     * Get value.
     *
     * @return Value
     */
    String getValue() const
    {
        return m_preferences->getString(m_key, getDefault());
    }

    /**
     * Set value.
     *
     * @param[in] value Value
     */
    void setValue(const String& value)
    {
        m_preferences->putString(m_key, value);
    }

    /**
     * Get default value.
     *
     * @return Default value
     */
    String getDefault() const
    {
        return String(m_defValue);
    }

private:

    const char*     m_key;      /**< Key */
    const char*     m_name;     /**< Name */
    const char*     m_defValue; /**< Default value */
    size_t          m_min;      /**< Min. length */
    size_t          m_max;      /**< Max. length */

    /* An instance shall not be copied. */
    KeyValueJson(const KeyValueJson& kv);
    KeyValueJson& operator=(const KeyValueJson& kv);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* KEY_VALUE_JSON_H */

/** @} */