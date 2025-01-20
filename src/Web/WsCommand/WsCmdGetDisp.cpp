/* MIT License
 *
 * Copyright (c) 2019 - 2025 Andreas Merkle <web@blue-andi.de>
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
    else
    {
        constexpr size_t fbLength    = CONFIG_LED_MATRIX_WIDTH * CONFIG_LED_MATRIX_HEIGHT;
        uint32_t*        framebuffer = new (std::nothrow) uint32_t[fbLength];

        if (nullptr == framebuffer)
        {
            sendNegativeResponse(server, clientId, "\"Internal error.\"");
        }
        else
        {
            String         msg;
            uint8_t        slotId = SlotList::SLOT_ID_INVALID;

            uint32_t       lastColor;          /* The color that started a repeat sequence.   */
            uint32_t       color      = 0U;    /* Actual color in read order.                 */
            size_t         index      = 1U;    /* Next value from framebuffer to used.        */
            uint32_t       repeat     = 0U;    /* Repeat count for current color.             */
            const uint32_t REPEAT_MAX = 0xFFU; /* Maximum repeat color counter value.         */
            GetDispState   state      = STATE_GETDISP_COLLECT; /* Frame buffer reading state. */

            DisplayMgr::getInstance().getFBCopy(framebuffer, fbLength, &slotId);

            preparePositiveResponse(msg);
            msg += slotId;

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
            lastColor = framebuffer[0];

            while (state != STATE_GETDISP_FINISH)
            {
                if (STATE_GETDISP_COLLECT == state)
                {
                    if (index < fbLength)
                    {
                        color = framebuffer[index];
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

                    if (index < fbLength)
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

            delete[] framebuffer;

            sendResponse(server, clientId, msg);
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
