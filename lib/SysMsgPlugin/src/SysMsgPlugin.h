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
 * @file   SysMsgPlugin.h
 * @brief  System message plugin
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup PLUGIN
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
#include "./internal/View.h"

#include <stdint.h>
#include <functional>
#include <Plugin.hpp>
#include <SimpleTimer.hpp>
#include <Mutex.hpp>

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
     * Handler which plays a melody belonging to a notification.
     *
     * It is called when the notification becomes the current one, not when it is
     * queued. The plugin itself has no audio knowledge, the application connects
     * it to the melody service.
     *
     * @param[in] melody    The melody in RTTTL format.
     * @param[in] volume    The volume.
     */
    typedef std::function<void(const String& melody, uint8_t volume)> MelodyHandler;

    /**
     * Constructs the plugin.
     *
     * @param[in] name  Plugin name (must exist over lifetime)
     * @param[in] uid   Unique id
     */
    SysMsgPlugin(const char* name, uint16_t uid) :
        Plugin(name, uid),
        m_view(),
        m_timer(),
        m_duration(0U),
        m_max(0U),
        m_isInit(true),
        m_isDurationHard(false),
        m_messages(),
        m_wrIndex(0U),
        m_rdIndex(0U),
        m_melodyHandler(nullptr),
        m_mutex()
    {
        (void)m_mutex.create();
    }

    /**
     * Destroys the plugin.
     */
    ~SysMsgPlugin()
    {
        m_mutex.destroy();
    }

    /**
     * Plugin creation method, used to register on the plugin manager.
     *
     * @param[in] name  Plugin name (must exist over lifetime)
     * @param[in] uid   Unique id
     *
     * @return If successful, it will return the pointer to the plugin instance, otherwise nullptr.
     */
    static IPluginMaintenance* create(const char* name, uint16_t uid)
    {
        return new (std::nothrow) SysMsgPlugin(name, uid);
    }

    /**
     * Get font type.
     *
     * @return The font type the plugin uses.
     */
    Fonts::FontType getFontType() const final
    {
        MutexGuard<MutexRecursive> guard(m_mutex);

        return m_view.getFontType();
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
        MutexGuard<MutexRecursive> guard(m_mutex);

        m_view.setFontType(fontType);
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
     *
     * @return If the message is queued, it will return true otherwise false.
     */
    bool show(const String& msg, uint32_t duration, uint32_t max);

    /**
     * Show a notification.
     *
     * In contrast to show() the duration is a hard limit. It ends the message
     * independent of the text scrolling. A optional icon is shown left of the
     * text, without a icon the text uses the full display width.
     *
     * @param[in] msg           Message to show
     * @param[in] duration      Duration in ms, how long the message shall be shown.
     * @param[in] iconFileName  Full path of the icon file. Empty means no icon.
     *
     * @param[in] melody        Melody in RTTTL format, played when the notification appears. Empty means no melody.
     * @param[in] volume        Volume of the melody.
     *
     * @return If the notification is queued, it will return true otherwise false.
     */
    bool showNotification(const String& msg, uint32_t duration, const String& iconFileName, const String& melody, uint8_t volume);

    /**
     * Register the handler which plays the melody of a notification.
     *
     * Without a handler a melody is silently ignored, which is the case on a
     * board without a buzzer.
     *
     * @param[in] handler   The melody handler.
     */
    void registerMelodyHandler(MelodyHandler handler);

    /**
     * Show next message in the queue and abort the current one.
     * If no message is in the queue anymore, the plugin will be disabled.
     */
    void next()
    {
        MutexGuard<MutexRecursive> guard(m_mutex);

        if (false == nextMessage())
        {
            m_view.clear();
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
        String   msg;            /**< Message */
        uint32_t duration;       /**< Duration in ms, how long the text shall be shown. */
        uint32_t max;            /**< Maximum number how often a scrolling text shall be shown. */
        String   iconFileName;   /**< Full path of the icon file. Empty means no icon. */
        String   melody;         /**< Melody in RTTTL format. Empty means no melody. */
        uint8_t  volume;         /**< Volume of the melody. */
        bool     isDurationHard; /**< Shall the duration end the message even if the text scrolls? */

        /**
         * Construct a system message.
         */
        SysMsg() :
            msg(),
            duration(0U),
            max(0U),
            iconFileName(),
            melody(),
            volume(0U),
            isDurationHard(false)
        {
        }

        /**
         * Destroy a system message.
         */
        ~SysMsg()
        {
        }
    };

    _SysMsgPlugin::View m_view;                  /**< View with all widgets. */
    SimpleTimer         m_timer;                 /**< Timer used to observer minimum duration */
    uint32_t            m_duration;              /**< Duration in ms, how long a non-scrolling text shall be shown. */
    uint32_t            m_max;                   /**< Maximum number how often a scrolling text shall be shown. */
    bool                m_isInit;                /**< Is initialization phase? Leaving this phase means to have duration and etc. handled. */
    bool                m_isDurationHard;        /**< Shall the duration end the current message even if the text scrolls? */
    SysMsg              m_messages[MAX_SYS_MSG]; /**< System message buffer */
    size_t              m_wrIndex;               /**< System message buffer write index. */
    size_t              m_rdIndex;               /**< System message buffer read index. */
    MelodyHandler       m_melodyHandler;         /**< Plays the melody of a notification. May be empty. */

    /** Mutex to protect against concurrent access. A notification may arrive from a different task than the display update. */
    mutable MutexRecursive m_mutex;

    /**
     * Queue a message.
     *
     * @param[in] msg               Message to show
     * @param[in] duration          Duration in ms.
     * @param[in] max               Maximum number how often a scrolling text shall be shown.
     * @param[in] iconFileName      Full path of the icon file. Empty means no icon.
     * @param[in] melody            Melody in RTTTL format. Empty means no melody.
     * @param[in] volume            Volume of the melody.
     * @param[in] isDurationHard    Shall the duration end the message even if the text scrolls?
     *
     * @return If the message is queued, it will return true otherwise false.
     */
    bool enqueue(const String& msg, uint32_t duration, uint32_t max, const String& iconFileName, const String& melody, uint8_t volume, bool isDurationHard);

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

#endif /* SYSMSGPLUGIN_H */

/** @} */