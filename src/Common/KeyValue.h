/* MIT License
 *
 * Copyright (c) 2019 - 2020 Andreas Merkle <web@blue-andi.de>
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
 * @addtogroup utilities
 *
 * @{
 */

#ifndef __KEY_VALUE_H__
#define __KEY_VALUE_H__

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
    };

    /**
     * Constructs a key value pair.
     */
    KeyValue()
    {
    }

    /**
     * Destroys a key value pair.
     */
    virtual ~KeyValue()
    {
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

};

/**
 * Key value pair with string value.
 */
class KeyValueString : public KeyValue
{
public:

    /**
     * Constructs a key value pair.
     */
    KeyValueString(Preferences& pref, const char* key, const char* name, const char* defValue, size_t min, size_t max) :
        KeyValue(),
        m_pref(pref),
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
    virtual ~KeyValueString()
    {
    }

    /**
     * Get value type.
     *
     * @return Value type
     */
    Type getValueType() const
    {
        return TYPE_STRING;
    }

    /**
     * Get user friendly name of key value pair.
     *
     * @return User friendly name
     */
    const char* getName() const
    {
        return m_name;
    }

    /**
     * Get key.
     *
     * @return Key
     */
    const char* getKey() const
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
        return m_pref.getString(m_key, getDefault());
    }

    /**
     * Set value.
     *
     * @param[in] value Value
     */
    void setValue(const String& value)
    {
        m_pref.putString(m_key, value);
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

    Preferences&    m_pref;     /**< Preferences */
    const char*     m_key;      /**< Key */
    const char*     m_name;     /**< Name */
    const char*     m_defValue; /**< Default value */
    size_t          m_min;      /**< Min. length */
    size_t          m_max;      /**< Max. length */

    /* An instance shall not be copied. */
    KeyValueString(const KeyValueString& kv);
    KeyValueString& operator=(const KeyValueString& kv);
};

/**
 * Key value pair with bool value type.
 */
class KeyValueBool : public KeyValue
{
public:

    /**
     * Constructs a key value pair.
     */
    KeyValueBool(Preferences& pref, const char* key, const char* name, bool defValue) :
        KeyValue(),
        m_pref(pref),
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
    Type getValueType() const
    {
        return TYPE_BOOL;
    }

    /**
     * Get user friendly name of key value pair.
     *
     * @return User friendly name
     */
    const char* getName() const
    {
        return m_name;
    }

    /**
     * Get key.
     *
     * @return Key
     */
    const char* getKey() const
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
        return m_pref.getBool(m_key, getDefault());
    }

    /**
     * Set value.
     *
     * @param[in] value Value
     */
    void setValue(bool value)
    {
        m_pref.putBool(m_key, value);
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

    Preferences&    m_pref;     /**< Preferences */
    const char*     m_key;      /**< Key */
    const char*     m_name;     /**< Name */
    bool            m_defValue; /**< Default value */

    /* An instance shall not be copied. */
    KeyValueBool(const KeyValueBool& kv);
    KeyValueBool& operator=(const KeyValueBool& kv);
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
    KeyValueNumber(Preferences& pref, const char* key, const char* name, T defValue, T min, T max) :
        KeyValue(),
        m_pref(pref),
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
    const char* getName() const
    {
        return m_name;
    }

    /**
     * Get key.
     *
     * @return Key
     */
    const char* getKey() const
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

    Preferences&    m_pref;     /**< Preferences */
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

/**
 * Key value pair with uint8_t value type.
 */
class KeyValueUInt8 : public KeyValueNumber<uint8_t>
{
public:

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
    Type getValueType() const
    {
        return TYPE_UINT8;
    }

    /**
     * Get value.
     *
     * @return Value
     */
    uint8_t getValue() const
    {
        return m_pref.getUChar(m_key, m_defValue);
    }

    /**
     * Set value.
     *
     * @param[in] value Value
     */
    void setValue(uint8_t value)
    {
        m_pref.putUChar(m_key, value);
    }

private:

    /* An instance shall not be copied. */
    KeyValueUInt8(const KeyValueUInt8& kv);
    KeyValueUInt8& operator=(const KeyValueUInt8& kv);
};

/**
 * Key value pair with int32_t value type.
 */
class KeyValueInt32 : public KeyValueNumber<int32_t>
{
public:

    /**
     * Constructs a key value pair.
     */
    KeyValueInt32(Preferences& pref, const char* key, const char* name, int32_t defValue, size_t min, size_t max) :
        KeyValueNumber(pref, key, name, defValue, min, max)
    {
    }

    /**
     * Destroys a key value pair.
     */
    virtual ~KeyValueInt32()
    {
    }

    /**
     * Get value type.
     *
     * @return Value type
     */
    Type getValueType() const
    {
        return TYPE_INT32;
    }

    /**
     * Get value.
     *
     * @return Value
     */
    int32_t getValue() const
    {
        return m_pref.getInt(m_key, m_defValue);
    }

    /**
     * Set value.
     *
     * @param[in] value Value
     */
    void setValue(int32_t value)
    {
        m_pref.putInt(m_key, value);
    }

private:

    /* An instance shall not be copied. */
    KeyValueInt32(const KeyValueInt32& kv);
    KeyValueInt32& operator=(const KeyValueInt32& kv);
};
/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __KEY_VALUE_H__ */

/** @} */