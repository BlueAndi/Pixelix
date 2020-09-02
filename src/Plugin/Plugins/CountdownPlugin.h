
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
#include <SimpleTimer.hpp>

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
    } TargetDayDescription;

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
        m_textWidget("\\calign?"),
        m_configurationFilename(""),
        m_currentDate(),
        m_targetDate(),
        m_targetDateInformation(),
        m_remainingDays(""),
        m_url(),
        m_callbackWebHandler(nullptr),
        m_xMutex(nullptr),
        m_cfgReloadTimer()
    {
        /* Example data, used to generate the very first configuration file. */
        m_targetDate.day                    = 29;
        m_targetDate.month                  = 8;
        m_targetDate.year                   = 2019;
        m_targetDateInformation.plural      = "DAYS";
        m_targetDateInformation.singular    = "DAY";

        /* Move the text widget one line lower for better look. */
        m_textWidget.move(0, 1);

        m_xMutex = xSemaphoreCreateMutex();
    }

    /**
     * Destroys the plugin.
     */
    ~CountdownPlugin()
    {
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

        if (nullptr != m_xMutex)
        {
            vSemaphoreDelete(m_xMutex);
            m_xMutex = nullptr;
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
     * Register web interface, e.g. REST API functionality.
     *
     * @param[in] srv       Webserver
     * @param[in] baseUri   Base URI, use this and append plugin specific part.
     */
    void registerWebInterface(AsyncWebServer& srv, const String& baseUri) override;

    /**
     * Unregister web interface.
     *
     * @param[in] srv   Webserver
     */
    void unregisterWebInterface(AsyncWebServer& srv) override;

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
     * Stop the plugin.
     * Overwrite it if your plugin needs to know that it will be uninstalled.
     */
    void stop() override;

    /**
     * Start the plugin.
     * Overwrite it if your plugin needs to know that it was installed.
     */
    void start() override;

    /**
     * Set target date for countdown.
     *
     * @param[in] targetDate    Target date
     */
    void setTargetDate(const DateDMY& targetDate);

    /**
     * Set language depended strings for the unit.
     *
     * @param[in] plural    Unit in plural form, e.g. "days".
     * @param[in] singular  Unit in singular form, e.g. "day".
     */
    void setUnitDescription(const String& plural, const String& singular);

private:

    /**
     * Icon width in pixels.
     */
    static const int16_t    ICON_WIDTH     = 8;

    /**
     * Icon height in pixels.
     */
    static const int16_t    ICON_HEIGHT    = 8;

    /**
     * Image path within the SPIFFS.
     */
    static const char*      IMAGE_PATH;

    /**
     * Configuration path within the SPIFFS.
     */
    static const char*      CONFIG_PATH;

   /**
    * Offset to translate the month of the tm struct (time.h)
    * to a human readable value, since months since January are used (0-11).
    */
    static const int16_t    TM_OFFSET_MONTH = 1;

   /**
    * Offset to translate the year of the tm struct (time.h)
    * to a human readable value, since years since 1900 are used.
    */
    static const int16_t    TM_OFFSET_YEAR  = 1900;

    /**
     * The configuration in the persistent memory shall be cyclic loaded.
     * This mechanism ensure that manual changes in the file are considered.
     * This is the reload period in ms.
     */
    static const uint32_t   CFG_RELOAD_PERIOD   = 30000U;

    Canvas*                     m_textCanvas;               /**< Canvas used for the text widget. */
    Canvas*                     m_iconCanvas;               /**< Canvas used for the bitmap widget. */
    BitmapWidget                m_bitmapWidget;             /**< Bitmap widget, used to show the icon. */
    TextWidget                  m_textWidget;               /**< Text widget, used for showing the text. */
    String                      m_configurationFilename;    /**< String used for specifying the configuration filename. */
    DateDMY                     m_currentDate;              /**< Date structure to hold the current date. */
    DateDMY                     m_targetDate;               /**< Date structure to hold the target date from the configuration data. */
    TargetDayDescription        m_targetDateInformation;    /**< String used for configured additional target date information. */
    String                      m_remainingDays;            /**< String used for displaying the remaining days untril the target date. */
    String                      m_url;                      /**< REST API URL */
    AsyncCallbackWebHandler*    m_callbackWebHandler;       /**< Callback web handler */
    SemaphoreHandle_t           m_xMutex;                   /**< Mutex to protect against concurrent access. */
    SimpleTimer                 m_cfgReloadTimer;           /**< Timer is used to cyclic reload the configuration from persistent memory. */

    /**
     * Instance specific web request handler, called by the static web request
     * handler. It will really handle the request.
     *
     * @param[in] request   Web request
     */
    void webReqHandler(AsyncWebServerRequest *request);

    /**
     * Saves current configuration to JSON file.
     */
    bool saveConfiguration();

    /**
     * Load configuration from JSON file.
     */
    bool loadConfiguration();

    /**
     * If configuration directory doesn't exists, it will be created.
     * Otherwise nothing happens.
     */
    void createConfigDirectory();

    /**
     * Calculates the difference between m_targetTime and m_currentTime in days.
     */
    void calculateDifferenceInDays(void);

    /**
     * Counts the number of leap years.
     */
    uint16_t countLeapYears(DateDMY date);

    /**
     * Convert a given date in days starting from year 0.;
     */
    uint32_t dateToDays(DateDMY date);

    /**
     * Protect against concurrent access.
     */
    void lock(void);

    /**
     * Unprotect against concurrent access.
     */
    void unlock(void);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __COUNTDOWNPLUGIN__ */

/** @} */
