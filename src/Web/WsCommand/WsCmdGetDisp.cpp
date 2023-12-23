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
 * @brief  Websocket command get display content
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "WsCmdGetDisp.h"
#include "DisplayMgr.h"
#include "SlotList.h"

#include <Util.h>
#include <Display.h>

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

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void WsCmdGetDisp::execute(AsyncWebSocket* server, AsyncWebSocketClient* client)
{
    if ((nullptr == server) ||
        (nullptr == client))
    {
        return;
    }

    /* Any error happended? */
    if (true == m_isError)
    {
        sendNegativeResponse(server, client, "\"Parameter invalid.\"");
    }
    else
    {
        IDisplay&   display     = Display::getInstance();
        size_t      fbLength    = display.getWidth() * display.getHeight();
        uint32_t*   framebuffer = new(std::nothrow) uint32_t[fbLength];

        if (nullptr == framebuffer)
        {
            sendNegativeResponse(server, client, "\"Internal error.\"");
        }
        else
        {
            uint32_t    index       = 0U;
            String      msg;
            uint8_t     slotId      = SlotList::SLOT_ID_INVALID;

            DisplayMgr::getInstance().getFBCopy(framebuffer, fbLength, &slotId);

            preparePositiveResponse(msg);

            msg += slotId;
            msg += DELIMITER;
            msg += display.getWidth();
            msg += DELIMITER;
            msg += display.getHeight();

            for(index = 0U; index <  fbLength; ++index)
            {
                msg += DELIMITER;
                msg += Util::uint32ToHex(framebuffer[index]);
            }

            delete[] framebuffer;
            
            sendResponse(server, client, msg);
        }
    }

    m_isError = false;
}

void WsCmdGetDisp::setPar(const char* par)
{
    UTIL_NOT_USED(par);

    m_isError = true;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
