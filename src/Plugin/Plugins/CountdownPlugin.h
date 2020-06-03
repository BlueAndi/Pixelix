
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
 * @brief  Countdown plugin
 * @author Yann Le Glaz <yann_le@web.de>
 *
 * @addtogroup plugin
 *
 * @{
 */

#ifndef __COUNTDOWNPLUGIN__
#define __COUNTDOWNPLUGIN__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Plugin.hpp"

#include "time.h"
#include <Canvas.h>
#include <BitmapWidget.h>
#include <stdint.h>
#include <TextWidget.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Shows the remaining days until a configured target date.
 * 
 * At the first installation a json document is generated to the SPIFFS /configuration/UUID.json
 * where the target date has to be configured.
 * 
 */
class CountdownPlugin : public Plugin
{
public:

    /** Date data. */
    typedef struct 
    {
        uint8_t day;    /**< Day of month. */
        uint8_t month;  /**< Month of year. */
        uint16_t year;  /**< Year. */
    } DateDMY;

    /** The target date description. */
    typedef struct 
    {
        String plural;      /**< The description in plural form e.g. Days. */
        String singular;    /**< The description in singular form e.g. day */
    }TargetDayDescription;

    /**
     * Constructs the plugin.
     *
     * @param[in] name  Plugin name
     * @param[in] uid   Unique id
     */
    CountdownPlugin(const String& name, uint16_t uid) :
        Plugin(name, uid),
        m_textCanvas(nullptr),
        m_iconCanvas(nullptr),
        m_bitmapWidget(),
        m_textWidget(),
        m_fd(),
        m_isConfigured(false),
        m_configurationFilename(""),
        m_currentDate(),
        m_targetDate(),
        m_targetDateInformation(),
        m_isUpdateAvailable(false),
        m_remainingDays("")
    {
        /* Move the text widget one line lower for better look. */
        m_textWidget.move(0, 1);
    }

    /**
     * Destroys the plugin.
     */
    ~CountdownPlugin()
    {
    
        uint16_t        bitmapWidth     = 0U;
        uint16_t        bitmapHeight    = 0U;
        const uint16_t* bitmapBuffer    = m_bitmapWidget.get(bitmapWidth, bitmapHeight);

        if (nullptr != m_iconCanvas)
        {
            delete m_iconCanvas;
            m_iconCanvas = nullptr;
        }

        if (nullptr != m_textCanvas)
        {
            delete m_textCanvas;
            m_textCanvas = nullptr;
        }

        if (nullptr != bitmapBuffer)
        {
            delete[] bitmapBuffer;
            bitmapBuffer = nullptr;
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
        return new CountdownPlugin(name, uid);
    }

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
     * Stop the plugin.
     * Overwrite it if your plugin needs to know that it will be uninstalled.
     */
    void stop() override;

    /**
     * Start the plugin.
     * Overwrite it if your plugin needs to know that it was installed.
     */
    void start() override;

private:

    /**
     * Icon width in pixels.
     */
    static const int16_t ICON_WIDTH     = 8;

    /**
     * Icon height in pixels.
     */
    static const int16_t ICON_HEIGHT    = 8;

    /**
     * Image path within the SPIFFS.
     */
    static const char*  IMAGE_PATH;

    /**
     * Configuration path within the SPIFFS.
     */
    static const char* CONFIG_PATH;

   /**
     * Offste to translate the month of the tm struct (time.h)
     * to a human readable value, since months since January are used (0-11).
     */
    static const int16_t TM_OFFSET_MONTH     = 1;
    
   /**
     * Offste to translate the year of the tm struct (time.h)
     * to a human readable value, since years since 1900 are used.
     */
    static const int16_t TM_OFFSET_YEAR     = 1900;


    Canvas*                 m_textCanvas;               /**< Canvas used for the text widget. */
    Canvas*                 m_iconCanvas;               /**< Canvas used for the bitmap widget. */
    BitmapWidget            m_bitmapWidget;             /**< Bitmap widget, used to show the icon. */
    TextWidget              m_textWidget;               /**< Text widget, used for showing the text. */
    File                    m_fd;                       /**< File descriptor, used for bitmap file upload. */
    bool                    m_isConfigured;             /**< Flag to indicate a configured plugin. */
    String                  m_configurationFilename;    /**< String used for specifying the configuration filename. */
    DateDMY                 m_currentDate;              /**< Date structure to hold the current date. */
    DateDMY                 m_targetDate;               /**< Date structure to hold the target date from the configuration data. */
    TargetDayDescription    m_targetDateInformation;    /**< String used for configured additional target date information. */
    bool                    m_isUpdateAvailable;        /**< Flag to indicate whether an update is available or not. */
    String                  m_remainingDays;            /**< String used for displaying the remaining days untril the target date. */
    
    /**
     * Tries to load the plugin config file.
     * If the file doesn't exist it will be generated with the plugin 
     * UID as filename.
     * 
     * @return If loaded successfully it will return true otherwise false.
     */
    bool loadOrGenerateConfigFile(void);

    /**
     * Calculates the differnece between m_targetTime and m_currentTime in days.
     */
    void calculateDifferenceInDays(void);

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __COUNTDOWNPLUGIN__ */

/** @} */
