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
 * @file   SysMsg.h
 * @brief  System message
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup APP_LAYER
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
#include <Widget.hpp>
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
     * Show a widget over the whole display for a given time.
     *
     * It is meant for content which the text and icon view can not express,
     * e.g. an animation. Like for a notification the duration is a hard limit.
     *
     * Note, the widget is not copied. The caller keeps the ownership and must
     * ensure that it exists as long as it may be shown.
     *
     * @param[in] widget    Widget to show.
     * @param[in] duration  Duration in ms, how long the widget shall be shown. 0 means infinite.
     */
    void showWidget(Widget& widget, uint32_t duration);

    /**
     * Show a notification for a given time.
     *
     * In contrast to show() the duration is a hard limit, it ends the
     * notification independent of the text scrolling. A optional icon is shown
     * left of the text, without a icon the text uses the full display width.
     *
     * @param[in] msg           Message to show
     * @param[in] duration      Duration in ms, how long the notification shall be shown.
     * @param[in] iconFileName  Full path of the icon file. Empty means no icon.
     * @param[in] melody        Melody in RTTTL format, played when the notification appears. Empty means no melody.
     * @param[in] volume        Volume of the melody.
     *
     * @return If the notification is queued, it will return true otherwise false.
     */
    bool showNotification(const String& msg, uint32_t duration, const String& iconFileName, const String& melody, uint8_t volume);

    /**
     * Register the handler which plays the melody of a notification.
     *
     * The handler is called when the notification appears, not when it is
     * queued. Without a handler a melody is silently ignored.
     *
     * @param[in] handler   The melody handler.
     */
    void registerMelodyHandler(SysMsgPlugin::MelodyHandler handler);

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

    SysMsgPlugin* m_plugin; /**< Plugin, used to show system messages */

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

    /**
     * Activate the slot of the system message plugin, so the queued message
     * is shown. A sticky slot is suspended for it.
     */
    void activateSlot();
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* SYSMSG_HPP */

/** @} */