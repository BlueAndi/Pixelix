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
 * @file   WsCmdGetDisp.cpp
 * @brief  Websocket command get display content
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "WsCmdGetDisp.h"
#include "DisplayMgr.h"
#include "SlotList.h"

#include <Logging.h>
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

void WsCmdGetDisp::execute(AsyncWebSocket* server, uint32_t clientId)
{
    if (nullptr == server)
    {
        return;
    }

    /* Any error happended? */
    if (true == m_isError)
    {
        sendNegativeResponse(server, clientId, "\"Parameter invalid.\"");
    }
    else if (false == ensureFramebuffer())
    {
        LOG_WARNING("Websocket GETDISP failed: framebuffer allocation failed.");
        sendNegativeResponse(server, clientId, "\"Internal error.\"");
    }
    else
    {
        String         msg;
        uint8_t        slotId = SlotList::SLOT_ID_INVALID;

        uint32_t       lastColor;                          /* The color that started a repeat sequence.   */
        uint32_t       color      = 0U;                    /* Actual color in read order.                 */
        size_t         index      = 1U;                    /* Next value from framebuffer to used.        */
        uint32_t       repeat     = 0U;                    /* Repeat count for current color.             */
        const uint32_t REPEAT_MAX = 0xFFU;                 /* Maximum repeat color counter value.         */
        GetDispState   state      = STATE_GETDISP_COLLECT; /* Frame buffer reading state. */

        DisplayMgr::getInstance().getFBCopy(m_framebuffer, FRAMEBUFFER_LENGTH, &slotId);

        preparePositiveResponse(msg);
        msg       += slotId;

        /* RGB data is send in a "compressed" format using a repeat counter in
         * the upper 8 bits. The send values are <rep>:<r>:<g>:<b>.
         * The repeat counter indicates how often the same color shall be used
         * in subsequent pixels. Use a small state machine to calculate the
         * repeat counter.
         *
         * Example:
         * A black only 32x8 framebuffer would be send as a single 0xFF000000 value.
         *
         */
        lastColor  = m_framebuffer[0];

        while (state != STATE_GETDISP_FINISH)
        {
            if (STATE_GETDISP_COLLECT == state)
            {
                if (FRAMEBUFFER_LENGTH > index)
                {
                    color = m_framebuffer[index];
                    if (color != lastColor)
                    {
                        /* Color has changed, send out current sequence */
                        state = STATE_GETDISP_SEND;
                    }
                    else
                    {
                        ++repeat;
                        if (REPEAT_MAX == repeat)
                        {
                            /* 8-bit repeat counter maximum reached, send color sequence. */
                            state = STATE_GETDISP_SEND;
                        }
                    }
                }
                else
                {
                    /* End of frame buffer reached, send final color sequence. */
                    state = STATE_GETDISP_SEND;
                }
                ++index;
            }
            else /* STATE_GETDISP_SEND */
            {
                msg += DELIMITER;

                /* Lower 24 bits is RGB, upper 8 bits repeat count. */
                msg += Util::uint32ToHex(lastColor | (repeat << 24U));

                if (FRAMEBUFFER_LENGTH > index)
                {
                    lastColor = color;
                    repeat    = 0U;
                    state     = STATE_GETDISP_COLLECT;
                }
                else
                {
                    /* Frame buffer length consumed, terminate state machine loop. */
                    state = STATE_GETDISP_FINISH;
                }
            }
        }

        sendResponse(server, clientId, msg);
    }

    m_isError = false;
}

void WsCmdGetDisp::setPar(const char* par)
{
    UTIL_NOT_USED(par);

    m_isError = true;
}

bool WsCmdGetDisp::ensureFramebuffer()
{
    bool isAvailable = false;

    if (nullptr != m_framebuffer)
    {
        isAvailable = true;
    }
    else
    {
        m_framebuffer = new (std::nothrow) uint32_t[FRAMEBUFFER_LENGTH];

        if (nullptr != m_framebuffer)
        {
            isAvailable = true;
        }
    }

    return isAvailable;
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
