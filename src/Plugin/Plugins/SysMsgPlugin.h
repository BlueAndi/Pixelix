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
 * @brief  System message plugin
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup plugin
 *
 * @{
 */

#ifndef __SYSMSGPLUGIN_H__
#define __SYSMSGPLUGIN_H__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include "Plugin.hpp"

#include <TextWidget.h>
#include <SimpleTimer.hpp>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Shows system message over the whole display.
 * If the text is too long for the display width, it automatically scrolls.
 */
class SysMsgPlugin : public Plugin
{
public:

    /**
     * Constructs the plugin.
     *
     * @param[in] name  Plugin name
     * @param[in] uid   Unique id
     */
    SysMsgPlugin(const String& name, uint16_t uid) :
        Plugin(name, uid),
        m_textWidget(),
        m_timer(),
        m_duration(0U),
        m_max(0U),
        m_isInit(true)
    {
        /* Move the text widget one line lower for better look. */
        m_textWidget.move(0, 1);
    }

    /**
     * Destroys the plugin.
     */
    ~SysMsgPlugin()
    {
    }

    /**
     * Plugin creation method, used to register on the plugin manager.
     *
     * @param[in] name  Plugin name
     * @param[in] uid   Unique id
     *
     * @return If successful, it will return the pointer to the plugin instance, otherwise nullptr.
     */
    static IPluginMaintenance* create(const String& name, uint16_t uid)
    {
        return new SysMsgPlugin(name, uid);
    }

    /**
     * Update the display.
     * The scheduler will call this method periodically.
     *
     * @param[in] gfx   Display graphics interface
     */
    void update(YAGfx& gfx) final;

    /**
     * Show message.
     *
     * @param[in] msg       Message to show
     * @param[in] duration  Duration in ms, how long a non-scrolling text shall be shown.
     * @param[in] max       Maximum number how often a scrolling text shall be shown.
     */
    void show(const String& msg, uint32_t duration, uint32_t max);

private:

    TextWidget  m_textWidget;   /**< Text widget, used for showing the text. */
    SimpleTimer m_timer;        /**< Timer used to observer minimum duration */
    uint32_t    m_duration;     /**< Duration in ms, how long a non-scrolling text shall be shown. */
    uint32_t    m_max;          /**< Maximum number how often a scrolling text shall be shown. */
    bool        m_isInit;       /**< Is initialization phase? Leaving this phase means to have duration and etc. handled. */

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __SYSMSGPLUGIN_H__ */

/** @} */