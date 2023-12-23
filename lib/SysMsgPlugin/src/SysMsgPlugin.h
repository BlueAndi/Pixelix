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
 * @brief  System message plugin
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup plugin
 *
 * @{
 */

#ifndef SYSMSGPLUGIN_H
#define SYSMSGPLUGIN_H

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
        m_fontType(Fonts::FONT_TYPE_DEFAULT),
        m_textWidget(),
        m_timer(),
        m_duration(0U),
        m_max(0U),
        m_isInit(true),
        m_messages(),
        m_wrIndex(0U),
        m_rdIndex(0U),
        m_isSignalEnabled(false)
    {
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
        return new(std::nothrow)SysMsgPlugin(name, uid);
    }

    /**
     * Get font type.
     * 
     * @return The font type the plugin uses.
     */
    Fonts::FontType getFontType() const final
    {
        return m_fontType;
    }

    /**
     * Set font type.
     * The plugin may skip the font type in case it gets conflicts with the layout.
     * 
     * A font type change will only be considered if it is set before the start()
     * method is called!
     * 
     * @param[in] fontType  The font type which the plugin shall use.
     */
    void setFontType(Fonts::FontType fontType) final
    {
        m_fontType = fontType;
        return;
    }

    /**
     * Start the plugin. This is called only once during plugin lifetime.
     * It can be used as deferred initialization (after the constructor)
     * and provides the canvas size.
     * 
     * If your display layout depends on canvas or font size, calculate it
     * here.
     * 
     * Overwrite it if your plugin needs to know that it was installed.
     * 
     * @param[in] width     Display width in pixel
     * @param[in] height    Display height in pixel
     */
    void start(uint16_t width, uint16_t height) final;
    
    /**
     * Stop the plugin. This is called only once during plugin lifetime.
     */
    void stop() final;

    /**
     * This method will be called in case the plugin is set active, which means
     * it will be shown on the display in the next step.
     *
     * @param[in] gfx   Display graphics interface
     */
    void active(YAGfx& gfx) final;

    /**
     * This method will be called in case the plugin is set inactive, which means
     * it won't be shown on the display anymore.
     */
    void inactive() final;

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

    /**
     * Enable signal in the corners as additional user information.
     */
    void enableSignal()
    {
        m_isSignalEnabled = true;
    }

    /**
     * Disable signal in the corners as additional user information.
     */
    void disableSignal()
    {
        m_isSignalEnabled = false;
    }

    /**
     * Show next message in the queue and abort the current one.
     * If no message is in the queue anymore, the plugin will be disabled.
     */
    void next()
    {
        if (false == nextMessage())
        {
            m_textWidget.clear();
            disable();
        }
    }

private:

    /** Max. number of system messages in the queue. */
    static const size_t MAX_SYS_MSG = 10U;

    /**
     * System message.
     */
    struct SysMsg
    {
        String      msg;        /**< Message */
        uint32_t    duration;   /**< Duration in ms, how long the text shall be shown. */
        uint32_t    max;        /**< Maximum number how often a scrolling text shall be shown. */

        /**
         * Construct a system message.
         */
        SysMsg() :
            msg(),
            duration(0U),
            max(0U)
        {
        }

        /**
         * Destroy a system message.
         */
        ~SysMsg()
        {
        }
    };

    Fonts::FontType m_fontType;             /**< Font type which shall be used if there is no conflict with the layout. */
    TextWidget      m_textWidget;           /**< Text widget, used for showing the text. */
    SimpleTimer     m_timer;                /**< Timer used to observer minimum duration */
    uint32_t        m_duration;             /**< Duration in ms, how long a non-scrolling text shall be shown. */
    uint32_t        m_max;                  /**< Maximum number how often a scrolling text shall be shown. */
    bool            m_isInit;               /**< Is initialization phase? Leaving this phase means to have duration and etc. handled. */
    SysMsg          m_messages[MAX_SYS_MSG];/**< System message buffer */
    size_t          m_wrIndex;              /**< System message buffer write index. */
    size_t          m_rdIndex;              /**< System message buffer read index. */
    bool            m_isSignalEnabled;      /**< Is signal enabled? */

    /**
     * Show next message from the queue.
     * 
     * @return If a message is available to be shown, it will return true otherwise false.
     */
    bool nextMessage();

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* SYSMSGPLUGIN_H */

/** @} */