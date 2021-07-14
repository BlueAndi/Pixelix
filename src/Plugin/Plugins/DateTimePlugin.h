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
#include <Mutex.hpp>

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
        m_isUpdateAvailable(false),
        m_slotInterf(nullptr),
        m_mutex()

    {
        /* Move the text widget one line lower for better look. */
        m_textWidget.move(0, 1);

        (void)m_mutex.create();
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
        return new DateTimePlugin(name, uid);
    }

    /**
     * Set the slot interface, which the plugin can used to request information
     * from the slot, it is plugged in.
     *
     * @param[in] slotInterf    Slot interface
     */
    void setSlot(const ISlotPlugin* slotInterf) final;

    /**
     * Start the plugin.
     * Overwrite it if your plugin needs to know that it was installed.
     * 
     * @param[in] width     Display width in pixel
     * @param[in] height    Display height in pixel
     */
    void start(uint16_t width, uint16_t height) final;

   /**
     * Stop the plugin.
     * Overwrite it if your plugin needs to know that it will be uninstalled.
     */
    void stop() final;

    /**
     * Process the plugin.
     * Overwrite it if your plugin has cyclic stuff to do without being in a
     * active slot.
     */
    void process(void) final;

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
     * Set text, which may contain format tags.
     *
     * @param[in] formatText    Text, which may contain format tags.
     */
    void setText(const String& formatText);

    /**
     * Set lamp state.
     *
     * @param[in] lampId    Lamp id
     * @param[in] state     Lamp state (true = on / false = off)
     */
    void setLamp(uint8_t lampId, bool state);

private:

    /** Max. number of lamps. */
    static const uint8_t    MAX_LAMPS           = 7U;

    /** Time to check date update period in ms */
    static const uint32_t   CHECK_UPDATE_PERIOD = 1000U;

    TextWidget          m_textWidget;               /**< Text widget, used for showing the text. */
    Canvas*             m_textCanvas;               /**< Canvas used for the text widget. */
    Canvas*             m_lampCanvas;               /**< Canvas used for the lamp widget. */
    LampWidget          m_lampWidgets[MAX_LAMPS];   /**< Lamp widgets, used to signal the day of week. */
    SimpleTimer         m_checkUpdateTimer;         /**< Timer, used for cyclic check if date/time update is necessarry. */
    uint8_t             m_durationCounter;          /**< Variable to count the Plugin duration in CHECK_UPDATE_PERIOD ticks . */
    bool                m_isUpdateAvailable;        /**< Flag to indicate an updated date value. */
    const ISlotPlugin*  m_slotInterf;               /**< Slot interface */
    MutexRecursive      m_mutex;                    /**< Mutex to protect against concurrent access. */

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

    /**
     * Calculates the optimal layout for several elements, which shall be aligned.
     * 
     * @param[in]   width           Max. available width in pixel.
     * @param[in]   cnt             Number of elements in a row.
     * @param[in]   minDistance     The minimal distance in pixel between each element.
     * @param[in]   minBorder       The minimal border left and right of all elements.
     * @param[out]  elementWidth    The calculated optimal element width in pixel.
     * @param[out]  elementDistance The calculated optimal element distance in pixel.
     * 
     * @return If the calculation is successful, it will return true otherwise false.
     */
    bool calcLayout(uint16_t width, uint16_t cnt, uint16_t minDistance, uint16_t minBorder, uint16_t& elementWidth, uint16_t& elementDistance);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __DATETIMEPLUGIN_H__ */

/** @} */