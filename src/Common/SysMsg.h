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
 * @brief  System message
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup common
 *
 * @{
 */

#ifndef SYSMSG_HPP
#define SYSMSG_HPP

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <SysMsgPlugin.h>
#include <WString.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * System message handler.
 */
class SysMsg
{
public:

    /**
     * Get system message handler instance.
     *
     * @return System message handler instance
     */
    static SysMsg& getInstance()
    {
        static SysMsg instance; /* singleton idiom to force initialization in the first usage. */

        return instance;
    }

    /**
     * Initialize system message handler.
     * It will hook into the display manager.
     *
     * @return If initialization is successful, it will return true otherwise false.
     */
    bool init(void);

    /**
     * Show message with the given duration. If the duration is infinite, it will be shown infinite.
     *
     * @param[in] msg       Message to show
     * @param[in] duration  Duration in ms, how long a non-scrolling message shall be shown.
     * @param[in] max       How often shall a scrolling message be shown.
     */
    void show(const String& msg, uint32_t duration = 0U, uint32_t max = 0U);

    /**
     * Enable signal in the corners as additional user information.
     */
    void enableSignal();

    /**
     * Disable signal in the corners as additional user information.
     */
    void disableSignal();

    /**
     * Is a system message shown in this moment?
     * 
     * @return If a system message is shown, it will return true otherwise false.
     */
    bool isActive() const;

    /**
     * Show next message in the queue.
     */
    void next();

private:

    SysMsgPlugin*   m_plugin;   /**< Plugin, used to show system messages */

    /**
     * Constructs the system message handler.
     */
    SysMsg() :
        m_plugin(nullptr)
    {
    }

    /**
     * Destroys the system message handler.
     */
    ~SysMsg()
    {
        /* Will never be called. */
    }

    SysMsg(const SysMsg& sysMsg);
    SysMsg& operator=(const SysMsg& sysMsg);

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* SYSMSG_HPP */

/** @} */