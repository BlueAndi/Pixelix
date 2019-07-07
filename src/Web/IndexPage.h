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
@brief  Index page
@author Andreas Merkle <web@blue-andi.de>

@section desc Description
This module provides the root page.

*******************************************************************************/
/** @defgroup index Index page
 * This module provides the root page.
 *
 * @{
 */

#ifndef __INDEXPAGE_H__
#define __INDEXPAGE_H__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <WebServer.h>
#include <stdint.h>
#include "IWebPage.hpp"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Index web page (root page).
 */
class IndexPage : public IWebPage
{
public:

    /**
     * Constructs the index page.
     */
    IndexPage() :
        IWebPage()
    {
    }

    /**
     * Destroys the index page.
     */
    ~IndexPage()
    {
    }

    /**
     * Get instance of page.
     * 
     * @return Instance
     */
    static IndexPage& getInstance(void)
    {
        return m_instance;
    }

private:

    static IndexPage    m_instance; /**< Singleton instance */

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

    IndexPage(const IndexPage& page);
    IndexPage& operator=(const IndexPage& page);

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __INDEXPAGE_H__ */

/** @} */