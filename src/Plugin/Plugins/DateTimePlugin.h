/* MIT License
 *
 * Copyright (c) 2019 - 2020 Andreas Merkle <web@blue-andi.de>
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
 * @brief  DateTime plugin
 * @author Yann Le Glaz <yann_le@web.de>
 *
 * @addtogroup plugin
 *
 * @{
 */

#ifndef __DATETIMEPLUGIN_H__
#define __DATETIMEPLUGIN_H__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <time.h>
#include "Plugin.hpp"

#include <LampWidget.h>
#include <TextWidget.h>
#include <Canvas.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Shows the current data and time (alternately) over the whole display.
 *
 */
class DateTimePlugin : public Plugin
{
public:

    /**
     * Constructs the plugin.
     *
     * @param[in] name  Plugin name
     * @param[in] uid   Unique id
     */
    DateTimePlugin(const String& name, uint16_t uid) :
        Plugin(name, uid),
        m_textWidget("\\calignNo NTP"),
        m_textCanvas(nullptr),
        m_lampCanvas(nullptr),
        m_lampWidgets(),
        m_checkUpdateTimer(),
        m_durationCounter(0u),
        m_isUpdateAvailable(false)

    {
        /* Move the text widget one line lower for better look. */
        m_textWidget.move(0, 1);
    }

    /**
     * Destroys the plugin.
     */
    ~DateTimePlugin()
    {
        if (nullptr != m_textCanvas)
        {
            delete m_textCanvas;
            m_textCanvas = nullptr;
        }

        if (nullptr != m_lampCanvas)
        {
            delete m_lampCanvas;
            m_lampCanvas = nullptr;
        }
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
        return new DateTimePlugin(name, uid);
    }

    /**
     * Update the display.
     * The scheduler will call this method periodically.
     *
     * @param[in] gfx   Display graphics interface
     */
    void update(IGfx& gfx);

    /**
     * Set text, which may contain format tags.
     *
     * @param[in] formatText    Text, which may contain format tags.
     */
    void setText(const String& formatText);

   /**
     * This method will be called in case the plugin is set active, which means
     * it will be shown on the display in the next step.
     *
     * @param[in] gfx   Display graphics interface
     */
    void active(IGfx& gfx) override;

    /**
     * This method will be called in case the plugin is set inactive, which means
     * it won't be shown on the display anymore.
     */
    void inactive() override;

    /**
     * Set lamp state.
     *
     * @param[in] lampId    Lamp id
     * @param[in] state     Lamp state (true = on / false = off)
     */
    void setLamp(uint8_t lampId, bool state);

    /**
     * Process the plugin.
     * Overwrite it if your plugin has cyclic stuff to do without being in a
     * active slot.
     */
    void process(void) override;

private:

    /** Max. number of lamps. */
    static const uint8_t    MAX_LAMPS               = 7U;

    /** Size of lamp widgets used for weekday indication. */
    static const uint16_t   CUSTOM_LAMP_WIDTH       = 3U;

    /** Time to check date update period in ms */
    static const uint32_t   CHECK_UPDATE_PERIOD     = 1000U;

    TextWidget  m_textWidget;               /**< Text widget, used for showing the text. */
    Canvas*     m_textCanvas;               /**< Canvas used for the text widget. */
    Canvas*     m_lampCanvas;               /**< Canvas used for the lamp widget. */
    LampWidget  m_lampWidgets[MAX_LAMPS];   /**< Lamp widgets, used to signal the day of week. */
    SimpleTimer m_checkUpdateTimer;         /**< Timer, used for cyclic check if date/time update is necessarry. */
    uint8_t     m_durationCounter;          /**< Variable to count the Plugin duration in CHECK_UPDATE_PERIOD ticks . */
    bool        m_isUpdateAvailable;        /**< Flag to indicate an updated date value. */

    /**
     * Get current date/time and update the text, which to be displayed.
     * The update takes only place, if the date changed.
     *
     * @param[in] force Force update independent of date.
     */
    void updateDateTime(bool force);

    /**
     * Set weekday indicator dependend on the given timeinfo.
     *
     *
     * @param[in] timeinfo the current timeinfo.
     */
    void setWeekdayIndicator(tm timeinfo);

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __DATETIMEPLUGIN_H__ */

/** @} */