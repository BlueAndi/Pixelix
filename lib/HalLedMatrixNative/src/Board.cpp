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
 * @file   Board.cpp
 * @brief  Board abstraction for the native environment
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Board.h"

#include <FS.h>
#include <Logging.h>

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

/** Root path of the filesystem on the host.
 *
 * The application uses absolute paths like "/js/app.js", which the target finds
 * in the filesystem image. On the host they are mapped below this path, which
 * is the folder the filesystem image is built from.
 */
static const char* FILESYSTEM_ROOT_PATH = "./data";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

bool Board::init()
{
    bool isSuccess = true;

    Pin::init();

    isSuccess = m_buttonDrv.init(Pin::buttonOkIn, Pin::buttonLeftIn, Pin::buttonRightIn);
    m_buzzerDrv.init(Pin::buzzerOut);
    m_ledDrv.init(Pin::onBoardLedOut);

    /* The filesystem of the host is rooted at the data folder, so the
     * webinterface and the plugin configurations are found.
     */
    if (false == NativeFS.begin(FILESYSTEM_ROOT_PATH))
    {
        LOG_ERROR("Failed to mount the filesystem at %s.", FILESYSTEM_ROOT_PATH);
        isSuccess = false;
    }

    return isSuccess;
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
