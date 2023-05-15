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
 * @brief  Key value pair of uint8_t type
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup settings
 *
 * @{
 */

#ifndef KEY_VALUE_UINT8_H
#define KEY_VALUE_UINT8_H

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
 * Key value pair with uint8_t value type.
 */
class KeyValueUInt8 : public KeyValueNumber<uint8_t>
{
public:

    /**
     * Constructs a key value pair.
     */
    KeyValueUInt8(const char* key, const char* name, uint8_t defValue, size_t min, size_t max) :
        KeyValueNumber(key, name, defValue, min, max)
    {
    }

    /**
     * Constructs a key value pair.
     */
    KeyValueUInt8(Preferences& pref, const char* key, const char* name, uint8_t defValue, size_t min, size_t max) :
        KeyValueNumber(pref, key, name, defValue, min, max)
    {
    }

    /**
     * Destroys a key value pair.
     */
    virtual ~KeyValueUInt8()
    {
    }

    /**
     * Get value type.
     *
     * @return Value type
     */
    Type getValueType() const final
    {
        return TYPE_UINT8;
    }

    /**
     * Get value.
     *
     * @return Value
     */
    uint8_t getValue() const final
    {
        return m_preferences->getUChar(m_key, m_defValue);
    }

    /**
     * Set value.
     *
     * @param[in] value Value
     */
    void setValue(uint8_t value) final
    {
        m_preferences->putUChar(m_key, value);
    }

private:

    /* An instance shall not be copied. */
    KeyValueUInt8(const KeyValueUInt8& kv);
    KeyValueUInt8& operator=(const KeyValueUInt8& kv);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* KEY_VALUE_UINT8_H */

/** @} */