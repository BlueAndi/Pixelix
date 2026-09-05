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
 * @file   Preferences.cpp
 * @brief  Preferences for test purposes only
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Preferences.h"

#include <cstdlib>
#include <map>

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and classes
 *****************************************************************************/

/** All key/value pairs of a single namespace. */
typedef std::map<std::string, std::string> KeyValueMap;

/** All namespaces. */
typedef std::map<std::string, KeyValueMap> NamespaceMap;

/******************************************************************************
 * Prototypes
 *****************************************************************************/

static NamespaceMap& getStorage();

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/******************************************************************************
 * Public Methods
 *****************************************************************************/

Preferences::Preferences() :
    m_namespace(),
    m_isOpen(false),
    m_isReadOnly(false)
{
}

Preferences::~Preferences()
{
    end();
}

bool Preferences::begin(const char* name, bool readOnly)
{
    bool isSuccessful = false;

    if (nullptr == name)
    {
        /* Guard: invalid namespace. */
    }
    else if (true == m_isOpen)
    {
        /* Guard: already open. */
    }
    else
    {
        m_namespace  = name;
        m_isReadOnly = readOnly;
        m_isOpen     = true;
        isSuccessful = true;
    }

    return isSuccessful;
}

void Preferences::end()
{
    m_isOpen     = false;
    m_isReadOnly = false;
    m_namespace.clear();
}

bool Preferences::clear()
{
    bool isSuccessful = false;

    if (false == isWritable())
    {
        /* Guard: not open or read-only. */
    }
    else
    {
        getStorage().erase(m_namespace);
        isSuccessful = true;
    }

    return isSuccessful;
}

bool Preferences::remove(const char* key)
{
    bool isSuccessful = false;

    if (nullptr == key)
    {
        /* Guard: invalid key. */
    }
    else if (false == isWritable())
    {
        /* Guard: not open or read-only. */
    }
    else
    {
        isSuccessful = (0U < getStorage()[m_namespace].erase(key));
    }

    return isSuccessful;
}

bool Preferences::getBool(const char* key, bool defValue)
{
    std::string value;
    bool        result = defValue;

    if (true == getValue(key, value))
    {
        result = ("1" == value);
    }

    return result;
}

size_t Preferences::putBool(const char* key, bool value)
{
    return putValue(key, (false == value) ? "0" : "1");
}

int32_t Preferences::getInt(const char* key, int32_t defValue)
{
    std::string value;
    int32_t     result = defValue;

    if (true == getValue(key, value))
    {
        result = static_cast<int32_t>(std::strtol(value.c_str(), nullptr, 10));
    }

    return result;
}

size_t Preferences::putInt(const char* key, int32_t value)
{
    return putValue(key, std::to_string(value));
}

uint32_t Preferences::getUInt(const char* key, uint32_t defValue)
{
    std::string value;
    uint32_t    result = defValue;

    if (true == getValue(key, value))
    {
        result = static_cast<uint32_t>(std::strtoul(value.c_str(), nullptr, 10));
    }

    return result;
}

size_t Preferences::putUInt(const char* key, uint32_t value)
{
    return putValue(key, std::to_string(value));
}

uint8_t Preferences::getUChar(const char* key, uint8_t defValue)
{
    return static_cast<uint8_t>(getUInt(key, defValue));
}

size_t Preferences::putUChar(const char* key, uint8_t value)
{
    return putUInt(key, value);
}

String Preferences::getString(const char* key, const String& defValue)
{
    std::string value;
    String      result = defValue;

    if (true == getValue(key, value))
    {
        result = value;
    }

    return result;
}

size_t Preferences::putString(const char* key, const String& value)
{
    return putValue(key, value.c_str());
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

bool Preferences::isWritable() const
{
    return (true == m_isOpen) && (false == m_isReadOnly);
}

bool Preferences::getValue(const char* key, std::string& value) const
{
    bool isFound = false;

    if (nullptr == key)
    {
        /* Guard: invalid key. */
    }
    else if (false == m_isOpen)
    {
        /* Guard: no namespace opened. */
    }
    else
    {
        NamespaceMap::const_iterator nsIt = getStorage().find(m_namespace);

        if (getStorage().end() != nsIt)
        {
            KeyValueMap::const_iterator kvIt = nsIt->second.find(key);

            if (nsIt->second.end() != kvIt)
            {
                value   = kvIt->second;
                isFound = true;
            }
        }
    }

    return isFound;
}

size_t Preferences::putValue(const char* key, const std::string& value)
{
    size_t stored = 0U;

    if (nullptr == key)
    {
        /* Guard: invalid key. */
    }
    else if (false == isWritable())
    {
        /* Guard: not open or read-only. */
    }
    else
    {
        getStorage()[m_namespace][key] = value;
        stored                         = value.length();
    }

    return stored;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

/**
 * Get the storage of all namespaces.
 *
 * The storage is shared by all instances, because the ESP32 counterpart stores
 * the key/value pairs in the non-volatile storage, which is shared as well.
 * A function local static is used to avoid any static initialization order
 * problem.
 *
 * @return Storage
 */
static NamespaceMap& getStorage()
{
    static NamespaceMap storage;

    return storage;
}
