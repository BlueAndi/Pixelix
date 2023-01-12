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
 * @brief  Audio signal detector plugin
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup plugin
 *
 * @{
 */

#ifndef __SIGNAL_DETECTOR_PLUGIN_H__
#define __SIGNAL_DETECTOR_PLUGIN_H__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include "Plugin.hpp"
#include "AsyncHttpClient.h"

#include <SimpleTimer.hpp>
#include <Mutex.hpp>
#include <TextWidget.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * The sound reactive plugin shows a bar graph, which represents the frequency
 * bands of audio input.
 */
class SignalDetectorPlugin : public Plugin
{
public:

    /**
     * Constructs the plugin.
     *
     * @param[in] name  Plugin name
     * @param[in] uid   Unique id
     */
    SignalDetectorPlugin(const String& name, uint16_t uid) :
        Plugin(name, uid),
        m_fontType(Fonts::FONT_TYPE_DEFAULT),
        m_textWidget(),
        m_mutex(),
        m_isDetected(false),
        m_pushUrl(),
        m_client(),
        m_isUpdateReq(false),
        m_timer(),
        m_slotInterf(nullptr)
    {
        (void)m_mutex.create();
        m_textWidget.setFormatStr(DEFAULT_TEXT);
    }

    /**
     * Destroys the plugin.
     */
    ~SignalDetectorPlugin()
    {
        m_mutex.destroy();
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
        return new SignalDetectorPlugin(name, uid);
    }

    /**
     * Is plugin enabled or not?
     *
     * @return If plugin is enabled, it will return true otherwise false.
     */
    bool isEnabled() const final;

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
     * Get plugin topics, which can be get/set via different communication
     * interfaces like REST, websocket, MQTT, etc.
     * 
     * Example:
     * {
     *     "topics": [
     *         "/text"
     *     ]
     * }
     * 
     * @param[out] topics   Topis in JSON format
     */
    void getTopics(JsonArray& topics) const final;

    /**
     * Get a topic data.
     * Note, currently only JSON format is supported.
     * 
     * @param[in]   topic   The topic which data shall be retrieved.
     * @param[out]  value   The topic value in JSON format.
     * 
     * @return If successful it will return true otherwise false.
     */
    bool getTopic(const String& topic, JsonObject& value) const final;

    /**
     * Set a topic data.
     * Note, currently only JSON format is supported.
     * 
     * @param[in]   topic   The topic which data shall be retrieved.
     * @param[in]   value   The topic value in JSON format.
     * 
     * @return If successful it will return true otherwise false.
     */
    bool setTopic(const String& topic, const JsonObject& value) final;

    /**
     * Set the slot interface, which the plugin can used to request information
     * from the slot, it is plugged in.
     *
     * @param[in] slotInterf    Slot interface
     */
    void setSlot(const ISlotPlugin* slotInterf) final;

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
     * Process the plugin.
     * Overwrite it if your plugin has cyclic stuff to do without being in a
     * active slot.
     * 
     * @param[in] isConnected   The network connection status. If network
     *                          connection is established, it will be true otherwise false.
     */
    void process(bool isConnected) final;

    /**
     * Update the display.
     * The scheduler will call this method periodically.
     *
     * @param[in] gfx   Display graphics interface
     */
    void update(YAGfx& gfx) final;

    /**
     * Get target frequency in Hz.
     * 
     * 0 Hz means this tone detector is disabled.
     * 
     * @param[in] idx   Index of the tone detector.
     * 
     * @return Target frequency in Hz.
     */
    float getTargetFreq(uint8_t idx) const;

    /**
     * Set target frequency in Hz.
     * 
     * 0 Hz means this tone detector is disabled.
     * 
     * @param[in] idx   Index of the tone detector.
     * @param[in] freq  Target frequency in Hz
     */
    void setTargetFreq(uint8_t idx, float freq);

    /**
     * Get min. duration in ms which the audio signal must be present.
     * 
     * @param[in] idx   Index of the tone detector.
     * 
     * @return Min. duration in ms
     */
    uint32_t getMinDuration(uint8_t idx) const;

    /**
     * Set min. duration in ms which the audio signal must be present.
     * 
     * @param[in] idx       Index of the tone detector.
     * @param[in] duration  Min. duration in ms
     */
    void setMinDuration(uint8_t idx, uint32_t duration);

    /**
     * Get the magntiude threshold.
     * 
     * @param[in] idx   Index of the tone detector.
     * 
     * @return Magnitude threshold
     */
    float getThreshold(uint8_t idx) const;

    /**
     * Set the magnitude threshold. The audio signal must be greater than the
     * threshold to be recognized.
     * 
     * @param[in] idx       Index of the tone detector.
     * @param[in] threshold Magnitude threshold
     */
    void setThreshold(uint8_t idx, float threshold);

    /**
     * Get text.
     * 
     * @return Formatted text
     */
    String getText() const;

    /**
     * Set text, which may contain format tags.
     *
     * @param[in] formatText    Text, which may contain format tags.
     */
    void setText(const String& formatText);

    /**
     * Get push URL.
     * 
     * @return Push URL
     */
    String getPushUrl() const;

    /**
     * Set push URL.
     *
     * @param[in] url   Push URL
     */
    void setPushUrl(const String& url);

private:

    /**
     * Plugin topic, used for parameter exchange.
     */
    static const char*      TOPIC_CFG;

    /**
     * Plugin topic, used to get/set the text which is shown if signal is detected.
     */
    static const char*      TOPIC_TEXT;

    /**
     * Plugin topic, used to get/set the URL which is triggered if signal is detected.
     */
    static const char*      TOPIC_PUSH_URL;

    /**
     * Default text which is shown until user set a different text.
     */
    static const char*      DEFAULT_TEXT;

    Fonts::FontType         m_fontType;     /**< Font type which shall be used if there is no conflict with the layout. */
    TextWidget              m_textWidget;   /**< If signal is detected, it will show a corresponding text. */
    mutable MutexRecursive  m_mutex;        /**< Mutex to protect against concurrent access. */
    bool                    m_isDetected;   /**< Shows that the signal was detected. */
    String                  m_pushUrl;      /**< Push URL which will be triggered if signal is detected. */
    AsyncHttpClient         m_client;       /**< HTTP(S) client used for push notification. */
    bool                    m_isUpdateReq;  /**< Display update request, by changing the text. */
    SimpleTimer             m_timer;        /**< Timer used for slot duration timeout detection in case deactivate() is not called. */
    const ISlotPlugin*      m_slotInterf;   /**< Slot interface */

    /**
     * Request new data.
     * 
     * @return If successful it will return true otherwise false.
     */
    bool startHttpRequest(void);

    /**
     * Register callback function on response reception.
     */
    void initHttpClient(void);

    /**
     * Saves current configuration to JSON file.
     */
    bool saveConfiguration() const;

    /**
     * Load configuration from JSON file.
     */
    bool loadConfiguration();

    /**
     * Is the audio signal detected?
     * 
     * @return If detected, it will return true otherwise false.
     */
    bool isSignalDetected();
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __SIGNAL_DETECTOR_PLUGIN_H__ */

/** @} */