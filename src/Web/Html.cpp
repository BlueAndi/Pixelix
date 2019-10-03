/* MIT License
 *
 * Copyright (c) 2019 Andreas Merkle <web@blue-andi.de>
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
 * @brief  HTML helper function
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Html.h"

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and classes
 *****************************************************************************/

/******************************************************************************
 * Prototypes
 *****************************************************************************/

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/**
 * HTML5 head containing
 * - HTML5 main framework begin
 * - Language set to english
 * - Using UTF-8 character set
 * - Definition of the standard viewport
 */
static const char HEAD_BEGIN[] = 
    "<!doctype html>\r\n" \
    "<html lang=\"en\">\r\n" \
    "<head>\r\n" \
    "    <meta charset=\"utf-8\"/>\r\n" \
    "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\r\n";

/**
 * HTML5 head containing the closing part.
 */
static const char HEAD_END[] = 
    "</head>\r\n" \
    "<body>\r\n";

/**
 * HTML5 tail containing the main html framework closing tags.
 */
static const char TAIL[] =
    "</body>\r\n" \
    "</html>";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

/******************************************************************************
 * External Functions
 *****************************************************************************/

String Html::htmlHead(const String& title)
{
    String head;
    
    head  = HEAD_BEGIN;
    head += "<title>" + title + "</title>\r\n";
    head += HEAD_END;

    return head;
}

String Html::htmlTail(void)
{
    return String(TAIL);
}

String Html::heading(const String& title, uint8_t size)
{
    String heading;

    if (0 == size)
    {
        size = 1;
    }

    heading  = "<h";
    heading += size;
    heading += ">";
    heading += title;
    heading += "</h";
    heading += size;
    heading += ">";

    return heading;
}

String Html::paragraph(const String& text)
{
    String p = "<p>" + text + "</p>";

    return p;
}

String Html::input(const String& name, const String& value)
{
    String input = "<input name=\"";

    input += name;
    input += "\" value=\"";
    input += value;
    input += "\" />";

    return input;
}

String Html::nextLine(void)
{
    return "<br />";
}

String Html::form(const String& content, const String& action)
{
    String form = "<form action=\"";
    form += action;
    form += "\" method=\"post\">";
    form += content;
    form += "<input type=\"submit\" value=\"Save\">";
    form += "</form>";

    return form;
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/
