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
 * @brief  Key value pair
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup settings
 *
 * @{
 */

#ifndef KEY_VALUE_H
#define KEY_VALUE_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <Preferences.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Key value pair interface.
 */
class KeyValue
{
public:

    /** Value types */
    enum Type
    {
        TYPE_UNKNOWN = 0,   /**< Unknown type */
        TYPE_UINT8,         /**< uint8_t type */
        TYPE_STRING,        /**< String type */
        TYPE_BOOL,          /**< bool type */
        TYPE_INT32,         /**< int32_t type */
        TYPE_JSON,          /**< JSON type */
        TYPE_UINT32         /**< uint32_t type */
    };

    /**
     * Destroys a key value pair.
     */
    virtual ~KeyValue()
    {
    }

    /**
     * Set the persistent storage where the key value pair shall be read/write.
     * 
     * @param[in] pref  Persistent storage
     */
    void setPersistentStorage(Preferences& pref)
    {
        m_preferences = &pref;
    }

    /**
     * Get value type.
     *
     * @return Value type
     */
    virtual Type getValueType() const = 0;

    /**
     * Get user friendly name of key value pair.
     *
     * @return User friendly name
     */
    virtual const char* getName() const = 0;

    /**
     * Get unique key.
     *
     * @return Key
     */
    virtual const char* getKey() const = 0;

protected:

    Preferences*    m_preferences;  /**< Persistent storage */

    /**
     * Constructs a key value pair.
     */
    KeyValue() :
        m_preferences(nullptr)
    {
    }

    /**
     * Constructs a key value pair.
     * 
     * @param[in] pref  Persistent storage
     */
    KeyValue(Preferences& pref) :
        m_preferences(&pref)
    {
    }

};

/**
 * Key value pair with number as value.
 */
template < typename T >
class KeyValueNumber : public KeyValue
{
public:

    /**
     * Constructs a key value pair.
     */
    KeyValueNumber(const char* key, const char* name, T defValue, T min, T max) :
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
    KeyValueNumber(Preferences& pref, const char* key, const char* name, T defValue, T min, T max) :
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
    virtual ~KeyValueNumber()
    {
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
     * Get minimum value.
     *
     * @return Minimum value
     */
    T getMin() const
    {
        return m_min;
    }

    /**
     * Get maximum value.
     *
     * @return Maximum value
     */
    T getMax() const
    {
        return m_max;
    }

    /**
     * Get value.
     *
     * @return Value
     */
    virtual T getValue() const = 0;

    /**
     * Set value.
     *
     * @param[in] value Value
     */
    virtual void setValue(T value) = 0;

    /**
     * Get default value.
     *
     * @return Default value
     */
    T getDefault() const
    {
        return m_defValue;
    }

protected:

    const char*     m_key;      /**< Key */
    const char*     m_name;     /**< Name */
    T               m_defValue; /**< Default value */
    T               m_min;      /**< Min. length */
    T               m_max;      /**< Max. length */

private:

    /* An instance shall not be copied. */
    KeyValueNumber(const KeyValueNumber& kv);
    KeyValueNumber& operator=(const KeyValueNumber& kv);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* KEY_VALUE_H */

/** @} */