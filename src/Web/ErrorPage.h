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
@brief  Pages
@author Andreas Merkle <web@blue-andi.de>

@section desc Description
This module provides HTML5 web pages.

*******************************************************************************/
/** @defgroup pages Pages
 * This module contains all HTML5 web pages, which can be accessed by a browser.
 *
 * @{
 */

#ifndef __ERRORPAGE_H__
#define __ERRORPAGE_H__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "IWebPage.hpp"
#include <stdint.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Common error page, used to have the same
 * look and feel for every kind of error.
 */
class ErrorPage : public IWebPage
{
public:

    /**
     * Builds a error page.
     * 
     * @param[in] errorCode     The error code corresponds to HTTP status code and will be send the client.
     * @param[in] errorMessage  The error message will be shown on the page.
     */
    ErrorPage(uint8_t errorCode, const char* errorMessage) :
        IWebPage(),
        m_errorCode(errorCode),
        m_errorMessage(errorMessage)
    {
    }

    /**
     * Destroys the error page.
     */
    ~ErrorPage()
    {
    }

    /**
     * Show the web page.
     * With the web server reference, the web page is able to
     * retrieve arguments or request headers.
     * 
     * If an authentication handler is available, every access will be authenticated.
     * 
     * @param[in] srv           Web server
     * @param[in] authHandler   Authentication handler (optional)
     */
    void show(WebServer& srv, IAuthHandler* authHandler);

private:

    uint8_t         m_errorCode;    /**< Error code, equal to web status code. */
    const String    m_errorMessage; /**< Error message, shown on page. */

    ErrorPage(const ErrorPage& page);
    ErrorPage& operator=(const ErrorPage& page);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __ERRORPAGE_H__ */

/** @} */