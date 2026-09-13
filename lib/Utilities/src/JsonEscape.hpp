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
 * @file   JsonEscape.hpp
 * @brief  JSON control character escaping
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * ArduinoJson escapes only the control characters which have a short escape
 * sequence (\" \\ \b \f \n \r \t). Every other control character is part of the
 * output as raw byte, which is not allowed by RFC 8259 and destroys the JSON
 * format for the receiver.
 *
 * A string value may contain such a character, because it is received from the
 * REST API, the MQTT API or the webinterface. Escaping during serialization
 * covers all of them, independent of where the value came from.
 *
 * Note, this works for compact serialization only. In that case a raw control
 * character in the output can only be part of a string value, because the
 * structural characters do not contain any of them.
 *
 * @addtogroup UTILITIES
 *
 * @{
 */

#ifndef JSON_ESCAPE_HPP
#define JSON_ESCAPE_HPP

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <stdio.h>
#include <Print.h>
#include <WString.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Print decorator, which replaces every control character by its unicode
 * escape sequence before it is forwarded to the decorated print interface.
 */
class JsonEscapePrint : public Print
{
public:

    /**
     * Constructs the print decorator.
     *
     * @param[in] output    The decorated print interface. Every byte is forwarded to it.
     */
    JsonEscapePrint(Print& output) :
        Print(),
        m_output(output)
    {
    }

    /**
     * Destroys the print decorator.
     */
    ~JsonEscapePrint()
    {
    }

    /* The write() overloads of the base class shall not be hidden by the
     * single byte overload below. They loop over every single byte, therefore
     * the escaping takes place for them as well.
     */
    using Print::write;

    /**
     * Write a single byte, escaped if necessary.
     *
     * @param[in] data  The byte to write.
     *
     * @return Number of consumed bytes, which is 1 on success and 0 on failure.
     */
    size_t write(uint8_t data) override
    {
        size_t consumed = 0U;

        if (LAST_CONTROL_CHARACTER < data)
        {
            if (0U < m_output.write(data))
            {
                consumed = 1U;
            }
        }
        else
        {
            /* 6 characters for the escape sequence and one for the termination. */
            char escaped[7U];
            int  length = snprintf(escaped, sizeof(escaped), "\\u%04X", static_cast<unsigned int>(data));

            if (0 < length)
            {
                if (static_cast<size_t>(length) == m_output.write(reinterpret_cast<const uint8_t*>(escaped), static_cast<size_t>(length)))
                {
                    consumed = 1U;
                }
            }
        }

        return consumed;
    }

private:

    /** Last character which must be escaped, see RFC 8259. */
    static const uint8_t LAST_CONTROL_CHARACTER = 0x1FU;

    Print&               m_output; /**< The decorated print interface. */

    JsonEscapePrint();
    JsonEscapePrint(const JsonEscapePrint& print);
    JsonEscapePrint& operator=(const JsonEscapePrint& print);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

/**
 * JSON control character escaping.
 */
namespace JsonEscape
{

/**
 * Replace every control character in a serialized JSON string by its unicode
 * escape sequence.
 *
 * Use it for a JSON document which is serialized into a string. Use the
 * JsonEscapePrint decorator if the document is serialized into a stream.
 *
 * @param[in,out] jsonStr   The compact serialized JSON document.
 */
inline void escapeControlCharacters(String& jsonStr)
{
    const uint8_t LAST_CONTROL_CHARACTER = 0x1FU;
    size_t        idx                    = 0U;

    while (jsonStr.length() > idx)
    {
        uint8_t character = static_cast<uint8_t>(jsonStr[idx]);

        if (LAST_CONTROL_CHARACTER < character)
        {
            ++idx;
        }
        else
        {
            /* 6 characters for the escape sequence and one for the termination. */
            char escaped[7U];

            (void)snprintf(escaped, sizeof(escaped), "\\u%04X", static_cast<unsigned int>(character));

            jsonStr  = jsonStr.substring(0U, idx) + escaped + jsonStr.substring(idx + 1U);
            idx     += 6U;
        }
    }
}

} /* namespace JsonEscape */

#endif /* JSON_ESCAPE_HPP */

/** @} */
