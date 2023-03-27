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
 * @brief  Captive portal request handler
 * @author Andreas Merkle <web@blue-andi.de>
 * 
 * @addtogroup web
 *
 * @{
 */

#ifndef CAPTIVE_PORTAL_HANDLER_H
#define CAPTIVE_PORTAL_HANDLER_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <ESPAsyncWebServer.h>
#include <stdint.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Captive portal request handler.
 */
class CaptivePortalHandler : public AsyncWebHandler
{
public:

    /**
     * Prototype for a reset request handler.
     */
    typedef std::function<void(void)> ResetReqHandler;

    /**
     * Constructs the captive portal request handler.
     * 
     * @param[in] resetReqHandler   Reset request handler
     */
    CaptivePortalHandler(ResetReqHandler resetReqHandler) :
        AsyncWebHandler(),
        m_resetReqHandler(resetReqHandler)
    {
    }

    /**
     * Destroys the captive portal request handler.
     */
    ~CaptivePortalHandler()
    {
    }

    /**
     * Checks whether the request can be handled.
     *
     * @param[in] request   Web request
     *
     * @return If request can be handled, it will return true otherwise false.
     */
    bool canHandle(AsyncWebServerRequest* request) final
    {
        /* The captive portal handles every request. */
        return true;
    }

    /**
     * Handles the request.
     *
     * @param[in] request   Web request, which to handle.
     */
    void handleRequest(AsyncWebServerRequest* request) final;

    /**
     * Non-trivial handler.
     * This is important to control the HTTP body parsing. In case of a non-trivial
     * handler, the webserver will parse the body and provides encoded parameters to
     * the request handler.
     */
    bool isRequestHandlerTrivial() final
    {
        return false;
    }

private:

    ResetReqHandler m_resetReqHandler;  /**< Reset request handler */

    CaptivePortalHandler();
    CaptivePortalHandler(const CaptivePortalHandler& handler);
    CaptivePortalHandler& operator=(const CaptivePortalHandler& handler);

    /**
     * Processor for index page template.
     * It is responsible for the data binding.
     *
     * @param[in] var   Name of variable in the template
     */
    static String captivePortalPageProcessor(const String& var);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* CAPTIVE_PORTAL_HANDLER_H */

/** @} */