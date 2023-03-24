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
 * @brief  Key value pair with bool type
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup settings
 *
 * @{
 */

#ifndef KEY_VALUE_BOOL_H
#define KEY_VALUE_BOOL_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "KeyValue.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Key value pair with bool value type.
 */
class KeyValueBool : public KeyValue
{
public:

    /**
     * Constructs a key value pair.
     */
    KeyValueBool(const char* key, const char* name, bool defValue) :
        KeyValue(),
        m_key(key),
        m_name(name),
        m_defValue(defValue)
    {
    }

    /**
     * Constructs a key value pair.
     */
    KeyValueBool(Preferences& pref, const char* key, const char* name, bool defValue) :
        KeyValue(pref),
        m_key(key),
        m_name(name),
        m_defValue(defValue)
    {
    }

    /**
     * Destroys a key value pair.
     */
    virtual ~KeyValueBool()
    {
    }

    /**
     * Get value type.
     *
     * @return Value type
     */
    Type getValueType() const final
    {
        return TYPE_BOOL;
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
     * Get value.
     *
     * @return Value
     */
    bool getValue() const
    {
        return m_preferences->getBool(m_key, getDefault());
    }

    /**
     * Set value.
     *
     * @param[in] value Value
     */
    void setValue(bool value)
    {
        m_preferences->putBool(m_key, value);
    }

    /**
     * Get default value.
     *
     * @return Default value
     */
    bool getDefault() const
    {
        return m_defValue;
    }

private:

    const char*     m_key;      /**< Key */
    const char*     m_name;     /**< Name */
    bool            m_defValue; /**< Default value */

    /* An instance shall not be copied. */
    KeyValueBool(const KeyValueBool& kv);
    KeyValueBool& operator=(const KeyValueBool& kv);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* KEY_VALUE_BOOL_H */

/** @} */