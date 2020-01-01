/* MIT License
 *
 * Copyright (c) 2019 Andreas Merkle <web@blue-andi.de>
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
     */
    SysMsgPlugin() :
        Plugin(),
        m_textWidget(),
        m_duration(0u)
    {
    }

    /**
     * Destroys the plugin.
     */
    ~SysMsgPlugin()
    {
    }

    /**
     * Get the plugin name.
     *
     * @return Name of the plugin.
     */
    const char* getName(void) const
    {
        return "SysMsgPlugin";
    }

    /**
     * Get duration how long the plugin shall be active.
     * If the plugin want to be displayed infinite, it will
     * return DURATION_INFINITE.
     * 
     * @return Duration in ms
     */
    uint32_t getDuration(void)
    {
        return m_duration;
    }

    /**
     * This method will be called in case the plugin is set active, which means
     * it will be shown on the display in the next step.
     */
    void active(void);

    /**
     * This method will be called in case the plugin is set inactive, which means
     * it won't be shown on the display anymore.
     */
    void inactive(void);

    /**
     * Update the display.
     * The scheduler will call this method periodically.
     * 
     * @param[in] gfx   Display graphics interface
     */
    void update(Adafruit_GFX& gfx);

    /**
     * Show message with the given duration. If the duration is 0, it will be shown infinite.
     * 
     * @param[in] msg       Message to show
     * @param[in] duration  Duration in ms, how long the message shall be shown. Note, a 0 means infinite.
     */
    void show(const String& msg, uint32_t duration = DURATION_INFINITE);

private:

    TextWidget  m_textWidget;   /**< Text widget, used for showing the text. */
    uint32_t    m_duration;     /**< Duration how long the message shall be shown. */

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __SYSMSGPLUGIN_H__ */

/** @} */