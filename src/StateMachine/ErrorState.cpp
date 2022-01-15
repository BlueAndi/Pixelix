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
 * @brief  System state: Error
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "ErrorState.h"

#include <Logging.h>
#include <Util.h>
#include <Display.h>
#include <YAFont.h>
#include <TomThumb.h>

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

void ErrorState::entry(StateMachine& sm)
{
    Display&    display = Display::getInstance();

    UTIL_NOT_USED(sm);

    LOG_INFO("Going in error state.");

    /* Any low level error happended and no error message can be shown
     * by the system message handler?
     */
    if (ERROR_ID_NO_ERROR != m_errorId)
    {
        LOG_INFO("Low level error %u.", m_errorId);

        /* Lets try to show the error cause on the display.
         * We can not assume that the display manager is still running.
         * Therefore (re-)initialize the display and try to draw directly.
         */
        if (false == display.begin())
        {
            LOG_FATAL("Could not (re-)initilize the display.");
        }
        else
        {
            YAFont  font(&TomThumb);
            int16_t cursorX     = 0;
            int16_t cursorY     = display.getHeight() - 1;
            Color   fontColor(ColorDef::RED);
            String  errorIdStr;
            uint8_t idx         = 0U;
            
            errorIdStr = m_errorId;

            /* The 'E' in front is the error prefix. */
            font.drawChar(display, cursorX, cursorY, 'E', fontColor);

            /* Show the error id. */
            for(idx = 0U; idx < errorIdStr.length(); ++idx)
            {
                font.drawChar(display, cursorX, cursorY, errorIdStr.c_str()[idx], fontColor);
            }

            display.show();
        }
    }
    else
    {
        LOG_INFO("No low level error, see display for more information.");
    }

    return;
}

void ErrorState::process(StateMachine& sm)
{
    UTIL_NOT_USED(sm);

    /* Wait for manual reset. */
    return;
}

void ErrorState::exit(StateMachine& sm)
{
    UTIL_NOT_USED(sm);

    /* Nothing to do. */
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
