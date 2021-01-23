/* MIT License
 *
 * Copyright (c) 2019 - 2021 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Websocket command to control fade effects
 * @author Yann Le Glaz <yann_le@web.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "WsCmdEffect.h"
#include "DisplayMgr.h"

#include <Util.h>

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

void WsCmdEffect::execute(AsyncWebSocket* server, AsyncWebSocketClient* client)
{
    if ((nullptr == server) ||
        (nullptr == client))
    {
        return;
    }

    /* Any error happended? */
    if (true == m_isError)
    {
        server->text(client->id(), "NACK;\"Parameter invalid.\"");
    }
    else
    {
        String rsp              = "ACK";
        const char  DELIMITER   = ';';

        if (1U == m_parCnt)
        {
            DisplayMgr::getInstance().activateNextFadeEffect((DisplayMgr::FadeEffect) m_fadeEffect);
        }

        rsp += DELIMITER;
        rsp += DisplayMgr::getInstance().getFadeEffect();

        server->text(client->id(), rsp);
    }

    m_isError = false;
    m_parCnt = 0U;
    return;
}

void WsCmdEffect::setPar(const char* par)
{
    if (0U == m_parCnt)
    {
        if (false == Util::strToUInt8(String(par), m_fadeEffect))
        {
            m_isError = true;
        }
         ++m_parCnt;
    }
    else
    {
        m_isError = true;
    }
    return;
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
