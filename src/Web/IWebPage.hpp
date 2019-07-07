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
@brief  IWebPage
@author Andreas Merkle <web@blue-andi.de>

@section desc Description
This module provides the web page interface.

*******************************************************************************/
/** @defgroup iwebpage IWebPage
 * This module provides the web page interface.
 *
 * @{
 */

#ifndef __IWEBPAGE_HPP__
#define __IWEBPAGE_HPP__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <WebServer.h>
#include <stdint.h>
#include "IAuthHandler.hpp"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Web page interface.
 */
class IWebPage
{
public:

    /**
     * Constructor
     */
    IWebPage()
    {
    }

    /**
     * Destructor
     */
    virtual ~IWebPage()
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
    virtual void show(WebServer& srv, IAuthHandler* authHandler) = 0;

private:

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __IWEBPAGE_HPP__ */

/** @} */