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
 * @brief  OpenWeather plugin
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "OpenWeatherPlugin.h"
#include "OpenWeatherCurrent.h"
#include "OpenWeatherOneCall.h"

#include <Logging.h>
#include <ArduinoJson.h>
#include <Util.h>
#include <math.h>
#include <HttpStatus.h>

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/** Divider to convert ms in s */
#define MS_TO_SEC_DIVIDER                       (1000U)

/**
 * Toggle counter value to switch between general weather data and additional information.
 * if DURATION_INFINITE was set for the plugin.
 */
#define MAX_COUNTER_VALUE_FOR_DURATION_INFINITE (15U)

/******************************************************************************
 * Types and classes
 *****************************************************************************/

/** UV-index element */
typedef struct
{
    uint8_t     lower;  /**< Lower UV-index value */
    uint8_t     upper;  /**< Upper UV-index value */
    const char* color;  /**< Color to show in this UV-index range */

} UvIndexElem;

/******************************************************************************
 * Prototypes
 *****************************************************************************/

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/* Initialize image path for standard icon. */
const char* OpenWeatherPlugin::IMAGE_PATH_STD_ICON      = "/plugins/OpenWeatherPlugin/openWeather.bmp";

/* Initialize image path for uvi icon. */
const char* OpenWeatherPlugin::IMAGE_PATH_UVI_ICON      = "/plugins/OpenWeatherPlugin/uvi.bmp";

/* Initialize image path for humidity icon. */
const char* OpenWeatherPlugin::IMAGE_PATH_HUMIDITY_ICON = "/plugins/OpenWeatherPlugin/hum.bmp";

/* Initialize image path for uvi icon. */
const char* OpenWeatherPlugin::IMAGE_PATH_WIND_ICON     = "/plugins/OpenWeatherPlugin/wind.bmp";

/* Initialize image path for the weather condition icons. */
const char* OpenWeatherPlugin::IMAGE_PATH               = "/plugins/OpenWeatherPlugin/";

/* Initialize OpenWeather base URI.
 * Use http:// instead of https:// for less required heap memory for SSL connection.
 */
const char* OpenWeatherPlugin::OPEN_WEATHER_BASE_URI    = "http://api.openweathermap.org";

/* Initialize plugin topic. */
const char* OpenWeatherPlugin::TOPIC_CONFIG             = "/weather";

/* Initialize bitmap image filename extension. */
const char* OpenWeatherPlugin::FILE_EXT_BITMAP          = ".bmp";

/* Initialize sprite sheet parameter filename extension. */
const char* OpenWeatherPlugin::FILE_EXT_SPRITE_SHEET    = ".sprite";

/** UV-index table */
static const UvIndexElem uvIndexTable[] =
{
    { 0U,   3U,     "\\#c0ffa0" },
    { 3U,   6U,     "\\#f8f140" },
    { 6U,   8U,     "\\#f77820" },
    { 8U,   11U,    "\\#d80020" }
};

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void OpenWeatherPlugin::getTopics(JsonArray& topics) const
{
    (void)topics.add(TOPIC_CONFIG);
}

bool OpenWeatherPlugin::getTopic(const String& topic, JsonObject& value) const
{
    bool isSuccessful = false;

    if (0U != topic.equals(TOPIC_CONFIG))
    {
        getConfiguration(value);
        isSuccessful = true;
    }

    return isSuccessful;
}

bool OpenWeatherPlugin::setTopic(const String& topic, const JsonObjectConst& value)
{
    bool isSuccessful = false;

    if (0U != topic.equals(TOPIC_CONFIG))
    {
        const size_t        JSON_DOC_SIZE           = 512U;
        DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
        JsonObject          jsonCfg                 = jsonDoc.to<JsonObject>();
        JsonVariantConst    jsonSourceId            = value["sourceId"];
        JsonVariantConst    jsonUpdatePeriod        = value["updatePeriod"];
        JsonVariantConst    jsonApiKey              = value["apiKey"];
        JsonVariantConst    jsonLatitude            = value["latitude"];
        JsonVariantConst    jsonLongitude           = value["longitude"];
        JsonVariantConst    jsonOther               = value["other"];
        JsonVariantConst    jsonUnits               = value["units"];

        /* The received configuration may not contain all single key/value pair.
         * Therefore read first the complete internal configuration and
         * overwrite them with the received ones.
         */
        getConfiguration(jsonCfg);

        /* Note:
         * Check only for the key/value pair availability.
         * The type check will follow in the setConfiguration().
         */

        if (false == jsonSourceId.isNull())
        {
            jsonCfg["sourceId"] = jsonSourceId.as<int>();
            isSuccessful = true;
        }

        if (false == jsonUpdatePeriod.isNull())
        {
            jsonCfg["updatePeriod"] = jsonUpdatePeriod.as<uint32_t>();
            isSuccessful = true;
        }

        if (false == jsonApiKey.isNull())
        {
            jsonCfg["apiKey"] = jsonApiKey.as<String>();
            isSuccessful = true;
        }

        if (false == jsonLatitude.isNull())
        {
            jsonCfg["latitude"] = jsonLatitude.as<String>();
            isSuccessful = true;
        }
        
        if (false == jsonLongitude.isNull())
        {
            jsonCfg["longitude"] = jsonLongitude.as<String>();
            isSuccessful = true;
        }

        if (false == jsonOther.isNull())
        {
            jsonCfg["other"] = jsonOther.as<int>();
            isSuccessful = true;
        }

        if (false == jsonUnits.isNull())
        {
            jsonCfg["units"] = jsonUnits.as<String>();
            isSuccessful = true;
        }

        if (true == isSuccessful)
        {
            JsonObjectConst jsonCfgConst = jsonCfg;

            isSuccessful = setConfiguration(jsonCfgConst);

            if (true == isSuccessful)
            {
                requestStoreToPersistentMemory();
            }
        }
    }

    return isSuccessful;
}

bool OpenWeatherPlugin::hasTopicChanged(const String& topic)
{
    MutexGuard<MutexRecursive>  guard(m_mutex);
    bool                        hasTopicChanged = m_hasTopicChanged;

    /* Only a single topic, therefore its not necessary to check. */
    PLUGIN_NOT_USED(topic);

    m_hasTopicChanged = false;

    return hasTopicChanged;
}

void OpenWeatherPlugin::setSlot(const ISlotPlugin* slotInterf)
{
    m_slotInterf = slotInterf;
}

void OpenWeatherPlugin::start(uint16_t width, uint16_t height)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_iconCanvas.setPosAndSize(0, 0, ICON_WIDTH, ICON_HEIGHT);
    (void)m_iconCanvas.addWidget(m_bitmapWidget);

    (void)m_bitmapWidget.load(FILESYSTEM, IMAGE_PATH_STD_ICON);

    /* The text canvas is left aligned to the icon canvas and it spans over
     * the whole display height.
     */
    m_textCanvas.setPosAndSize(ICON_WIDTH, 0, width - ICON_WIDTH, height);
    (void)m_textCanvas.addWidget(m_textWidget);

    /* Choose font. */
    m_textWidget.setFont(Fonts::getFontByType(m_fontType));
    
    /* The text widget inside the text canvas is left aligned on x-axis and
     * aligned to the center of y-axis.
     */
    if (height > m_textWidget.getFont().getHeight())
    {
        uint16_t diffY = height - m_textWidget.getFont().getHeight();
        uint16_t offsY = diffY / 2U;

        m_textWidget.move(0, offsY);
    }

    /* Try to load configuration. If there is no configuration available, a default configuration
     * will be created.
     */
    if (false == loadConfiguration())
    {
        if (false == saveConfiguration())
        {
            LOG_WARNING("Failed to create initial configuration file %s.", getFullPathToConfiguration().c_str());
        }
    }
    else
    {
        /* Remember current timestamp to detect updates of the configuration in the
         * filesystem without using the plugin API.
         */
        updateTimestampLastUpdate();
    }

    m_cfgReloadTimer.start(CFG_RELOAD_PERIOD);

    initHttpClient();
}

void OpenWeatherPlugin::stop()
{
    String                      configurationFilename = getFullPathToConfiguration();
    MutexGuard<MutexRecursive>  guard(m_mutex);

    m_cfgReloadTimer.stop();
    m_requestTimer.stop();

    if (false != FILESYSTEM.remove(configurationFilename))
    {
        LOG_INFO("File %s removed", configurationFilename.c_str());
    }
}

void OpenWeatherPlugin::process(bool isConnected)
{
    Msg                         msg;
    MutexGuard<MutexRecursive>  guard(m_mutex);

    /* Configuration in persistent memory updated? */
    if ((true == m_cfgReloadTimer.isTimerRunning()) &&
        (true == m_cfgReloadTimer.isTimeout()))
    {
        if (true == isConfigurationUpdated())
        {
            m_reloadConfigReq = true;
        }

        m_cfgReloadTimer.restart();
    }

    if (true == m_storeConfigReq)
    {
        if (false == saveConfiguration())
        {
            LOG_WARNING("Failed to save configuration: %s", getFullPathToConfiguration().c_str());
        }

        m_storeConfigReq = false;
    }
    else if (true == m_reloadConfigReq)
    {
        LOG_INFO("Reload configuration: %s", getFullPathToConfiguration().c_str());

        if (true == loadConfiguration())
        {
            updateTimestampLastUpdate();
        }

        m_reloadConfigReq = false;
    }
    else
    {
        ;
    }

    /* Only if a network connection is established the required information
     * shall be periodically requested via REST API.
     */
    if (false == m_requestTimer.isTimerRunning())
    {
        if (true == isConnected)
        {
            if (false == startHttpRequest())
            {
                LOG_WARNING("Failed to request weather info.");
                
                m_requestTimer.start(UPDATE_PERIOD_SHORT);
            }
            else
            {
                m_requestTimer.start(m_updatePeriod);
            }
        }
    }
    else
    {
        /* If the connection is lost, stop periodically requesting information
         * via REST API.
         */
        if (false == isConnected)
        {
            m_requestTimer.stop();
        }
        /* Network connection is available and next request may be necessary for
         * information update.
         */
        else if (true == m_requestTimer.isTimeout())
        {
            if (false == startHttpRequest())
            {
                LOG_WARNING("Failed to request weather info.");

                m_requestTimer.start(UPDATE_PERIOD_SHORT);
            }
            else
            {
                m_requestTimer.start(m_updatePeriod);
            }
        }
    }

    if ((true == m_updateContentTimer.isTimerRunning()) &&
        (true == m_updateContentTimer.isTimeout()))
    {
        updateDisplay(false);
        m_updateContentTimer.restart();
    }

    if (true == m_taskProxy.receive(msg))
    {
        switch(msg.type)
        {
        case MSG_TYPE_INVALID:
            /* Should never happen. */
            break;

        case MSG_TYPE_RSP:
            if (nullptr != msg.rsp)
            {
                handleWebResponse(*msg.rsp);
                delete msg.rsp;
                msg.rsp = nullptr;
            }
            break;

        case MSG_TYPE_CONN_CLOSED:
            LOG_INFO("Connection closed.");

            if (true == m_isConnectionError)
            {
                m_requestTimer.start(UPDATE_PERIOD_SHORT);
            }
            m_isConnectionError = false;
            break;

        case MSG_TYPE_CONN_ERROR:
            LOG_WARNING("Connection error.");
            m_isConnectionError = true;
            break;

        default:
            /* Should never happen. */
            break;
        }
    }
}

void OpenWeatherPlugin::active(YAGfx& gfx)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    /* Force immediate weather update on activation.
     * By setting the duration counter to 0, start showing
     * the general weather information first.
     */
    m_durationCounter = 0U;
    updateDisplay(true);

    /* Start time to check cyclic whether an update of the
     * display is necessary.
     */
    m_updateContentTimer.start(DURATION_TICK_PERIOD);
}

void OpenWeatherPlugin::inactive()
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_updateContentTimer.stop();
}

void OpenWeatherPlugin::update(YAGfx& gfx)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    if (false != m_isUpdateAvailable)
    {
        gfx.fillScreen(ColorDef::BLACK);
        m_textCanvas.update(gfx);

        m_isUpdateAvailable = false;
    }

    /* Update the icon always, as it may be animated. */
    m_iconCanvas.update(gfx);
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void OpenWeatherPlugin::createOpenWeatherSource(OpenWeatherSource id)
{
    destroyOpenWeatherSource();

    switch(id)
    {
    case OPENWEATHER_SOURCE_CURRENT:
        m_source = new(std::nothrow) OpenWeatherCurrent();
        break;

    case OPENWEATHER_SOURCE_ONE_CALL_25:
        m_source = new(std::nothrow) OpenWeatherOneCall("2.5");
        break;

    case OPENWEATHER_SOURCE_ONE_CALL_30:
        m_source = new(std::nothrow) OpenWeatherOneCall("3.0");
        break;

    default:
        break;
    }
}

void OpenWeatherPlugin::destroyOpenWeatherSource()
{
    if (nullptr != m_source)
    {
        delete m_source;
        m_source = nullptr;
    }
}

void OpenWeatherPlugin::requestStoreToPersistentMemory()
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_storeConfigReq = true;
}

void OpenWeatherPlugin::getConfiguration(JsonObject& jsonCfg) const
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    if (nullptr == m_source)
    {
        LOG_ERROR("No OpenWeather source available.");
    }
    else
    {
        jsonCfg["sourceId"]     = static_cast<int>(m_sourceId);
        jsonCfg["updatePeriod"] = m_updatePeriod / (60U * 1000U); /* Conversion from ms to minutes. */
        jsonCfg["apiKey"]       = m_source->getApiKey();
        jsonCfg["latitude"]     = m_source->getLatitude();
        jsonCfg["longitude"]    = m_source->getLongitude();
        jsonCfg["units"]        = m_source->getUnits();
        jsonCfg["other"]        = static_cast<int>(m_additionalInformation);
    }
}

bool OpenWeatherPlugin::setConfiguration(JsonObjectConst& jsonCfg)
{
    bool                status              = false;
    JsonVariantConst	jsonSourceId        = jsonCfg["sourceId"];
    JsonVariantConst    jsonUpdatePeriod    = jsonCfg["updatePeriod"];
    JsonVariantConst    jsonApiKey          = jsonCfg["apiKey"];
    JsonVariantConst    jsonLatitude        = jsonCfg["latitude"];
    JsonVariantConst    jsonLongitude       = jsonCfg["longitude"];
    JsonVariantConst    jsonOther           = jsonCfg["other"];
    JsonVariantConst    jsonUnits           = jsonCfg["units"];

    const uint32_t      UPDATE_PERIOD_LOWER_LIMIT   = 1U;   /* minutes */
    const uint32_t      UPDATE_PERIOD_UPPER_LIMIT   = 120U; /* minutes */

    if (false == jsonSourceId.is<int>())
    {
        LOG_WARNING("Source id not found or invalid type.");
    }
    else if (false == jsonUpdatePeriod.is<uint32_t>())
    {
        LOG_WARNING("Update period not found or invalid type.");
    }
    else if (false == jsonApiKey.is<String>())
    {
        LOG_WARNING("API key not found or invalid type.");
    }
    else if (false == jsonLatitude.is<String>())
    {
        LOG_WARNING("Latitude not found or invalid type.");
    }
    else if (false == jsonLongitude.is<String>())
    {
        LOG_WARNING("Longitude not found or invalid type.");
    }
    else if (false == jsonOther.is<int>())
    {
        LOG_WARNING("other not found or invalid type.");
    }
    else if (false == jsonUnits.is<String>())
    {
        LOG_WARNING("Units not found or invalid type.");
    }
    else
    {
        MutexGuard<MutexRecursive>  guard(m_mutex);
        OpenWeatherSource           sourceId        = static_cast<OpenWeatherSource>(jsonSourceId.as<int>());

        if (m_sourceId != sourceId)
        {
            destroyOpenWeatherSource();
            m_sourceId = sourceId;
            createOpenWeatherSource(m_sourceId);
        }

        m_updatePeriod = jsonUpdatePeriod.as<uint32_t>();

        if ((UPDATE_PERIOD_LOWER_LIMIT > m_updatePeriod) ||
            (UPDATE_PERIOD_UPPER_LIMIT < m_updatePeriod))
        {
            m_updatePeriod = UPDATE_PERIOD;
        }
        else
        {
            m_updatePeriod = SIMPLE_TIMER_MINUTES(m_updatePeriod);
        }

        if (nullptr == m_source)
        {
            LOG_ERROR("No OpenWeather source available.");
        }
        else
        {
            m_source->setApiKey(jsonApiKey.as<String>());
            m_source->setLatitude(jsonLatitude.as<String>());
            m_source->setLongitude(jsonLongitude.as<String>());
            m_source->setUnits(jsonUnits.as<String>());
        }

        m_additionalInformation = static_cast<OtherWeatherInformation>(jsonOther.as<int>());

        /* Force update on display */
        m_requestTimer.start(UPDATE_PERIOD_SHORT);

        m_hasTopicChanged = true;

        status = true;
    }

    return status;
}

const char* OpenWeatherPlugin::uvIndexToColor(uint8_t uvIndex)
{
    uint8_t     idx     = 0U;
    const char* color   = "\\#a80081"; /* Default color */

    while(UTIL_ARRAY_NUM(uvIndexTable) > idx)
    {
        if ((uvIndexTable[idx].lower <= uvIndex) &&
            (uvIndexTable[idx].upper > uvIndex))
        {
            color = uvIndexTable[idx].color;
            break;
        }

        ++idx;
    }

    return color;
}

void OpenWeatherPlugin::updateDisplay(bool force)
{
    bool showWeather        = true;
    bool isUpdateNecessary  = force;

    /* Handle additional weather information only if enabled. */
    if (OTHER_WEATHER_INFO_OFF != m_additionalInformation)
    {
        const uint32_t  INFINITE            = 0U;
        uint32_t        slotDuration        = INFINITE;
        uint32_t        durationOfEachPart  = INFINITE;

        /* Get the slot duration periodically, because the user can change it dynamically. */
        if (nullptr != m_slotInterf)
        {
            slotDuration = m_slotInterf->getDuration();
        }

        /* If slot duration is infinite, additional weather information will be shown periodically for a constant time. */
        if (INFINITE == slotDuration)
        {
            durationOfEachPart = MAX_COUNTER_VALUE_FOR_DURATION_INFINITE;
        }
        /* Otherwise additional weather information will be shown half of the slot duration. */
        else
        {
            durationOfEachPart = slotDuration / (2U * MS_TO_SEC_DIVIDER);
        }

        /* The additional weather information shall be shown after a dedicated time. */
        if (durationOfEachPart <= m_durationCounter)
        {
            showWeather = false;
        }

        /* Update necessary?
         * Avoid updating everytime, because it might destroy the animation of icons.
         */
        if ((0U == m_durationCounter) ||
            (durationOfEachPart == m_durationCounter))
        {
            isUpdateNecessary = true;
        }

        ++m_durationCounter;

        /* Reset duration counter after the general weather and the additional weather information
         * were shown for the same amount of time.
         */
        if ((2U * durationOfEachPart) <= m_durationCounter)
        {
            m_durationCounter = 0U;
        }
    }

    if (true == isUpdateNecessary)
    {
        String text;

        if (true == showWeather)
        {
            if (true == m_hasWeatherIconChanged)
            {
                String spriteSheetPath = m_currentWeatherIconFullPath.substring(0U, m_currentWeatherIconFullPath.length() - strlen(FILE_EXT_BITMAP)) + FILE_EXT_SPRITE_SHEET;

                /* If there is an icon in the filesystem, it will be loaded otherwise
                 * the standard icon. First check whether it is a animated sprite sheet
                 * and if not, try to load just the bitmap image.
                 */
                if (false == m_bitmapWidget.loadSpriteSheet(FILESYSTEM, spriteSheetPath, m_currentWeatherIconFullPath))
                {
                    if (false == m_bitmapWidget.load(FILESYSTEM, m_currentWeatherIconFullPath))
                    {
                        LOG_WARNING("Icon doesn't exists: %s", m_currentWeatherIconFullPath.c_str());

                        (void)m_bitmapWidget.load(FILESYSTEM, IMAGE_PATH_STD_ICON);
                    }
                }

                m_hasWeatherIconChanged = false;
            }

            text = m_currentTemp;
        }
        else
        {
            String  iconPath;

            switch (m_additionalInformation)
            {
            case OTHER_WEATHER_INFO_UVI:
                text = m_currentUvIndex;
                iconPath = IMAGE_PATH_UVI_ICON;
                break;

            case OTHER_WEATHER_INFO_HUMIDITY:
                text = m_currentHumidity;
                iconPath = IMAGE_PATH_HUMIDITY_ICON;
                break;

            case OTHER_WEATHER_INFO_WIND:
                text = m_currentWindspeed;
                iconPath = IMAGE_PATH_WIND_ICON;
                break;

            case OTHER_WEATHER_INFO_OFF:
                /* Should never reach here. */
                break;

            default:
                /* Should never reach here. */
                m_additionalInformation = OTHER_WEATHER_INFO_OFF;
                break;
            }

            if (false == iconPath.isEmpty())
            {
                if (false == m_bitmapWidget.load(FILESYSTEM, iconPath))
                {
                    LOG_WARNING("Icon doesn't exists: %s", iconPath.c_str());

                    (void)m_bitmapWidget.load(FILESYSTEM, IMAGE_PATH_STD_ICON);
                }

                /* Ensure that in case the weather icon shall be shown again,
                 * it will be loaded.
                 */
                m_hasWeatherIconChanged = true;
            }
        }

        m_textWidget.setFormatStr(text);

        m_isUpdateAvailable = true;
    }
}

bool OpenWeatherPlugin::startHttpRequest()
{
    bool status = false;

    if ((nullptr != m_source) &&
        (false == m_source->getApiKey().isEmpty()) &&
        (false == m_source->getLatitude().isEmpty()) &&
        (false == m_source->getLongitude().isEmpty()) &&
        (false == m_source->getUnits().isEmpty()))
    {
        String url = OPEN_WEATHER_BASE_URI;

        m_source->getUrl(url);

        if (true == m_client.begin(url))
        {
            if (false == m_client.GET())
            {
                LOG_WARNING("GET %s failed.", url.c_str());
            }
            else
            {
                status = true;
            }
        }
    }

    return status;
}

void OpenWeatherPlugin::initHttpClient()
{
    /* Note: All registered callbacks are running in a different task context!
     *       Therefore it is not allowed to access a member here directly.
     *       The processing must be deferred via task proxy.
     */
    m_client.regOnResponse(
        [this](const HttpResponse& rsp)
        {
            handleAsyncWebResponse(rsp);
        }
    );

    m_client.regOnClosed(
        [this]()
        {
            Msg msg;

            msg.type = MSG_TYPE_CONN_CLOSED;

            (void)this->m_taskProxy.send(msg);
        }
    );

    m_client.regOnError(
        [this]()
        {
            Msg msg;

            msg.type = MSG_TYPE_CONN_ERROR;

            (void)this->m_taskProxy.send(msg);
        }
    );
}

void OpenWeatherPlugin::handleAsyncWebResponse(const HttpResponse& rsp)
{
    if (HttpStatus::STATUS_CODE_OK == rsp.getStatusCode())
    {
        if (nullptr != m_source)
        {
            const size_t            JSON_DOC_SIZE   = 512U;
            DynamicJsonDocument*    jsonDoc         = new(std::nothrow) DynamicJsonDocument(JSON_DOC_SIZE);

            if (nullptr != jsonDoc)
            {
                size_t                          payloadSize     = 0U;
                const void*                     vPayload        = rsp.getPayload(payloadSize);
                const char*                     payload         = static_cast<const char*>(vPayload);
                const size_t                    FILTER_SIZE     = 128U;
                StaticJsonDocument<FILTER_SIZE> jsonFilterDoc;

                m_source->getFilter(jsonFilterDoc);

                if (true == jsonFilterDoc.overflowed())
                {
                    LOG_ERROR("Less memory for filter available.");
                }
                else if ((nullptr == payload) ||
                         (0U == payloadSize))
                {
                    LOG_ERROR("No payload.");
                }
                else
                {
                    DeserializationError error = deserializeJson(*jsonDoc, payload, payloadSize, DeserializationOption::Filter(jsonFilterDoc));

                    if (DeserializationError::Ok != error.code())
                    {
                        LOG_WARNING("JSON parse error: %s", error.c_str());
                    }
                    else
                    {
                        Msg msg;

                        msg.type    = MSG_TYPE_RSP;
                        msg.rsp     = jsonDoc;

                        if (false == this->m_taskProxy.send(msg))
                        {
                            delete jsonDoc;
                            jsonDoc = nullptr;
                        }
                    }
                }
            }
        }
    }
}

void OpenWeatherPlugin::handleWebResponse(const DynamicJsonDocument& jsonDoc)
{
    if (nullptr != m_source)
    {
        m_source->parse(jsonDoc);
        prepareDataToShow();
    }
}

void OpenWeatherPlugin::prepareDataToShow()
{
    if (nullptr != m_source)
    {
        float   temperature                     = m_source->getTemperature();
        String  weatherIconId                   = m_source->getWeatherIconId();
        float   uvIndex                         = m_source->getUvIndex();
        int     humidity                        = m_source->getHumidity();
        float   windSpeed                       = m_source->getWindSpeed();
        String  weatherConditionIconFullPath;

        /* Generate UV-Index string and adapt color of string accordingly. */
        if (true == std::isnan(uvIndex))
        {
            m_currentUvIndex = "\\calignN/A";
        }
        else
        {
            m_currentUvIndex = "\\calign";
            m_currentUvIndex += uvIndexToColor(static_cast<uint8_t>(uvIndex));
            m_currentUvIndex += uvIndex;
        }

        if (true == std::isnan(temperature))
        {
            m_currentUvIndex = "\\calignN/A";
        }
        else
        {
            const char* reducePrecision         = (temperature < -9.9F) ? "%.0f" : "%.1f";
            char        tempReducedPrecison[6]  = { 0 };

            /* Generate temperature string with reduced precision and add unit °C/°F. */
            (void)snprintf(tempReducedPrecison, sizeof(tempReducedPrecison), reducePrecision, temperature);

            m_currentTemp  = "\\calign";
            m_currentTemp += tempReducedPrecison;
            m_currentTemp += "\x8E";
            m_currentTemp += (m_source->getUnits() == "metric") ? "C" : "F";
        }

        /* Generate humidity string */
        m_currentHumidity = "\\calign";
        m_currentHumidity += humidity;
        m_currentHumidity += "%";

        /* Generate windapeed string and add unit.*/
        if (true == std::isnan(windSpeed))
        {
            m_currentUvIndex = "\\calignN/A";
        }
        else
        {
            char windReducedPrecison[5] = { 0 };

            (void)snprintf(windReducedPrecison, sizeof(windReducedPrecison), "%.1f", windSpeed);

            m_currentWindspeed = "\\calign";
            m_currentWindspeed += windReducedPrecison;
            m_currentWindspeed += "m/s";
        }

        /* Handle icon depended on weather icon id.
         * See https://openweathermap.org/weather-conditions
         * 
         * First check whether there is a specific icon available.
         * If not, check for a generic weather icon.
         * If this is not available too, use the standard OpenWeather icon.
         */
        weatherConditionIconFullPath = IMAGE_PATH + weatherIconId + FILE_EXT_BITMAP;
        if (false == FILESYSTEM.exists(weatherConditionIconFullPath))
        {
            weatherConditionIconFullPath  = IMAGE_PATH + weatherIconId.substring(0U, weatherIconId.length() - 1U);
            weatherConditionIconFullPath += FILE_EXT_BITMAP;
        }

        /* If there is really a change, the display shall be updated otherwise
         * not to not destroy running animations.
         */
        if (weatherConditionIconFullPath != m_currentWeatherIconFullPath)
        {
            m_hasWeatherIconChanged = true;
            m_currentWeatherIconFullPath = weatherConditionIconFullPath;
        }

        updateDisplay(true);
    }
}

void OpenWeatherPlugin::clearQueue()
{
    Msg msg;

    while(true == m_taskProxy.receive(msg))
    {
        if (MSG_TYPE_RSP == msg.type)
        {
            delete msg.rsp;
            msg.rsp = nullptr;
        }
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
