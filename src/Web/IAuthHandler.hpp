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
@brief  IAuthHandler
@author Andreas Merkle <web@blue-andi.de>

@section desc Description
This module provides the web authentication handler interface.

*******************************************************************************/
/** @defgroup iauthhandler IAuthHandler
 * This module provides the web authentication handler interface.
 *
 * @{
 */

#ifndef __IAUTHHANDLER_HPP__
#define __IAUTHHANDLER_HPP__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <WebServer.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Authenticaton handler interface
 */
class IAuthHandler
{
public:

    /**
     * Constructor
     */
    IAuthHandler()
    {
    }

    /**
     * Destructor
     */
    virtual ~IAuthHandler()
    {
    }

    /**
     * This method will be called to authenticate the client.
     * A web page can request this on demand.
     * 
     * @param[in] srv   Web server
     * 
     * @return Authentication result
     * @retval false    Authentication failed
     * @retval true     Authentication successful
     */
    virtual bool authenticate(WebServer& srv) = 0;

private:
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __IAUTHHANDLER_HPP__ */

/** @} */