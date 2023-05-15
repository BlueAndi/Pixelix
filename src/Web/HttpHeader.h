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
 * @brief  HTTP header
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup web
 *
 * @{
 */

#ifndef HTTP_HEADER_H
#define HTTP_HEADER_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <WString.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Request/Response http header
 */
class HttpHeader
{
public:

    /**
     * Constructs a empty header.
     */
    HttpHeader() :
        m_name(),
        m_value()
    {
    }

    /**
     * Constructs header, based on the given values.
     *
     * @param[in] name  Field name
     * @param[in] value Field value
     */
    HttpHeader(const String& name, const String& value) :
        m_name(name),
        m_value(value)
    {
    }

    /**
     * Constructs a header, based on a single header line.
     *
     * @param[in] line  Header line, which contains name and value.
     */
    HttpHeader(const String& line) :
        m_name(),
        m_value()
    {
        parse(line);
    }

    /**
     * Destroys the header.
     */
    ~HttpHeader()
    {
    }

    /**
     * Get field name.
     *
     * @return Field name
     */
    const String& getName() const
    {
        return m_name;
    }

    /**
     * Get field value.
     *
     * @return Field value
     */
    const String& getValue() const
    {
        return m_value;
    }

    /**
     * Set field name.
     *
     * @param[in] name  Field name
     */
    void setName(const String& name)
    {
        m_name = name;
    }

    /**
     * Set field value.
     *
     * @param[in] value Field value
     */
    void setValue(const String& value)
    {
        m_value = value;
    }

    /**
     * Assign a header.
     *
     * @param[in] hdr   Header, which to assign.
     *
     * @return Header
     */
    HttpHeader& operator=(const HttpHeader& hdr);

    /**
     * Parse "<name>: <value>" string.
     *
     * @param[in] line  String line
     */
    void parse(const String& line);

    /**
     * Get header as single line.
     *
     * @return Header
     */
    String toString() const;

private:

    String  m_name;     /**< Header field name */
    String  m_value;    /**< Header field value */
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* HTTP_HEADER_H */

/** @} */