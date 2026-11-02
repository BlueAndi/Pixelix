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
 * @file   PluginAlias.hpp
 * @brief  Plugin alias rules
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * The alias is the only externally visible identity of a plugin instance. It is
 * part of the MQTT topic, the REST URI and the Home Assistant object id, therefore
 * the allowed character set is the intersection of what all three accept.
 *
 * @addtogroup PLUGIN
 *
 * @{
 */

#ifndef PLUGIN_ALIAS_HPP
#define PLUGIN_ALIAS_HPP

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <stddef.h>
#include <WString.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Rules for a plugin instance alias.
 *
 * All functions work byte-wise. The length limits count bytes, not characters,
 * because the byte length is what bounds the MQTT topic and the REST URI. A
 * multi-byte UTF-8 character is therefore replaced by one underscore per byte.
 */
namespace PluginAlias
{

/** Minimum alias length in byte. */
static const size_t MIN_LENGTH      = 3U;

/** Maximum alias length in byte. */
static const size_t MAX_LENGTH      = 20U;

/** Suffix of a plugin type name, which is stripped for a generated alias. */
static const char* TYPE_NAME_SUFFIX = "Plugin";

/** Result of an alias assignment. */
enum Result
{
    RESULT_OK = 0,  /**< Alias accepted. */
    RESULT_INVALID, /**< Alias violates the length or the character set. */
    RESULT_IN_USE   /**< Alias is already used by another plugin instance. */
};

/**
 * Is the given character allowed in a plugin alias?
 *
 * @param[in] ch    Character to check
 *
 * @return If the character is allowed, it will return true otherwise false.
 */
inline bool isCharAllowed(char ch)
{
    bool isAllowed = false;

    if (('A' <= ch) && ('Z' >= ch))
    {
        isAllowed = true;
    }
    else if (('a' <= ch) && ('z' >= ch))
    {
        isAllowed = true;
    }
    else if (('0' <= ch) && ('9' >= ch))
    {
        isAllowed = true;
    }
    else if ('_' == ch)
    {
        isAllowed = true;
    }
    else if ('-' == ch)
    {
        isAllowed = true;
    }
    else
    {
        /* Not allowed. */
        ;
    }

    return isAllowed;
}

/**
 * Is the given alias valid regarding length and character set?
 *
 * @param[in] alias Alias to check
 *
 * @return If the alias is valid, it will return true otherwise false.
 */
inline bool isValid(const String& alias)
{
    bool         isAliasValid = true;
    const size_t LENGTH       = alias.length();
    size_t       idx          = 0U;

    if ((MIN_LENGTH > LENGTH) || (MAX_LENGTH < LENGTH))
    {
        isAliasValid = false;
    }
    else
    {
        while ((LENGTH > idx) && (true == isAliasValid))
        {
            if (false == isCharAllowed(alias.charAt(idx)))
            {
                isAliasValid = false;
            }
            else
            {
                ++idx;
            }
        }
    }

    return isAliasValid;
}

/**
 * Replace every forbidden character by an underscore and truncate to the
 * maximum length.
 *
 * Note, the result is not guaranteed to be valid. An input shorter than
 * MIN_LENGTH stays too short. Check the result with isValid().
 *
 * @param[in] alias Alias to sanitize
 *
 * @return Sanitized alias
 */
inline String sanitize(const String& alias)
{
    String       result;
    const size_t LENGTH = alias.length();
    size_t       idx    = 0U;

    while ((LENGTH > idx) && (MAX_LENGTH > result.length()))
    {
        char ch = alias.charAt(idx);

        if (false == isCharAllowed(ch))
        {
            ch = '_';
        }

        result += ch;
        ++idx;
    }

    return result;
}

/**
 * Generate a default alias from a plugin type name and an index.
 *
 * The trailing "Plugin" of the type name is stripped, the remainder is
 * truncated so that the index still fits, and the index is appended. The
 * result is always valid.
 *
 * @param[in] pluginTypeName    Plugin type name, may be nullptr
 * @param[in] index             Index, which makes the alias unique
 *
 * @return Generated alias
 */
inline String makeDefault(const char* pluginTypeName, uint8_t index)
{
    const size_t SUFFIX_LENGTH = 6U; /* strlen("Plugin") */
    String       base;
    String       indexStr = String(static_cast<unsigned int>(index));
    String       result;

    if (nullptr != pluginTypeName)
    {
        base = pluginTypeName;
    }

    if (true == base.endsWith(TYPE_NAME_SUFFIX))
    {
        base = base.substring(0U, base.length() - SUFFIX_LENGTH);
    }

    if (true == base.isEmpty())
    {
        base = TYPE_NAME_SUFFIX;
    }

    base = sanitize(base);

    if (MAX_LENGTH < (base.length() + indexStr.length()))
    {
        base = base.substring(0U, MAX_LENGTH - indexStr.length());
    }

    while (MIN_LENGTH > (base.length() + indexStr.length()))
    {
        base += '_';
    }

    result  = base;
    result += indexStr;

    return result;
}

} /* namespace PluginAlias */

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* PLUGIN_ALIAS_HPP */

/** @} */
