/* MIT License
 *
 * Copyright (c) 2019 - 2023 Andreas Merkle <web@blue-andi.de>
 * Copyright (c) 2026 Julie Hill <queenkjuul@pm.me>
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
 * @brief  Gets Chicago bus arrival times
 * @author Julie Hill <queenkjuul@pm.me>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "AsyncHttpClient.h"
#include "ChicagoBusTrackerPlugin.h"
#include "FileSystem.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Logging.h>
#include <JsonFile.h>
#include <Util.h>
#include <HTTPClient.h>
#include <FS.h>
#include <LittleFS.h>

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and classes
 *****************************************************************************/

/******************************************************************************
 * Prototypes
 *****************************************************************************/

/******************************************************************************
 * Local Variables
 *****************************************************************************/

const char *ChicagoBusTrackerPlugin::CHICAGO_BUS_BASE_URI = "http://ctabustracker.com/bustime/api/v2";

const char *ChicagoBusTrackerPlugin::TOPIC_CONFIG = "/ctabus";

const char *ChicagoBusTrackerPlugin::TOPIC_ROUTES = "/ctabusroutes";

const char *ChicagoBusTrackerPlugin::TOPIC_DIRS = "/ctabusdirections";

const char *ChicagoBusTrackerPlugin::TOPIC_STOPS = "/ctabusstops";

const size_t FILTER_SIZE = 128U;

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void ChicagoBusTrackerPlugin::getTopics(JsonArray &topics) const
{
    (void)topics.add(TOPIC_CONFIG);
    (void)topics.add(TOPIC_ROUTES);
    (void)topics.add(TOPIC_STOPS);
    (void)topics.add(TOPIC_DIRS);
}

bool ChicagoBusTrackerPlugin::getTopic(const String &topic, JsonObject &value) const
{
    bool isSuccessful = false;

    if (0U != topic.equals(TOPIC_CONFIG))
    {
        getConfiguration(value);
        isSuccessful = true;
    }
    if (0U != topic.equals(TOPIC_ROUTES))
    {
        const_cast<ChicagoBusTrackerPlugin*>(this)->getRoutes(value);
        isSuccessful = true;
    }
    if (0U != topic.equals(TOPIC_DIRS))
    {
        const_cast<ChicagoBusTrackerPlugin*>(this)->getDirections(value);
        isSuccessful = true;
    }
    if (0U != topic.equals(TOPIC_STOPS))
    {
        const_cast<ChicagoBusTrackerPlugin*>(this)->getStops(value);
        isSuccessful = true;
    }

    return isSuccessful;
}

bool ChicagoBusTrackerPlugin::setTopic(const String &topic, const JsonObject &value)
{
    for (JsonPair kv : value) {
        LOG_DEBUG("Key: %s", kv.key().c_str());
    }

    bool isSuccessful = false;

    if (0U != topic.equals(TOPIC_CONFIG))
    {
        const size_t JSON_DOC_SIZE = 1024U;
        DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
        JsonObject jsonCfg = jsonDoc.to<JsonObject>();
        JsonVariantConst jsonApiKey = value["apiKey"];
        JsonVariantConst jsonRte = value["rte"];
        JsonVariantConst jsonDir = value["dir"];
        JsonVariantConst jsonStpid = value["stpid"];
        JsonVariantConst jsonOrig = value["orig"];
        JsonVariantConst jsonDest = value["dest"];
        JsonVariantConst jsonTwo = value["two"];

        /* The received configuration may not contain all single key/value pair.
         * Therefore read first the complete internal configuration and
         * overwrite them with the received ones.
         */
        getConfiguration(jsonCfg);

        /* Note:
         * Check only for the key/value pair availability.
         * The type check will follow in the setConfiguration().
         */
        if (false == jsonApiKey.isNull())
        {
            m_apiKey = jsonApiKey.as<String>();
            jsonCfg["apiKey"] = m_apiKey;

            File sharedFile = LittleFS.open(m_apiKeyFilename, "w");
            if (sharedFile) {
                DynamicJsonDocument sharedDoc(256);
                sharedDoc["apiKey"] = m_apiKey;
                serializeJson(sharedDoc, sharedFile);
                sharedFile.close();
            }
            isSuccessful = true;
        }
        if (false == jsonRte.isNull())
        {
            jsonCfg["rte"] = jsonRte.as<String>();
            isSuccessful = true;
        }
        if (false == jsonDir.isNull())
        {
            jsonCfg["dir"] = jsonDir.as<String>();
            isSuccessful = true;
        }
        if (false == jsonStpid.isNull())
        {
            jsonCfg["stpid"] = jsonStpid.as<String>();
            isSuccessful = true;
        }
        if (false == jsonOrig.isNull())
        {
            jsonCfg["orig"] = jsonOrig.as<String>() == "true";
            isSuccessful = true;
        }
        if (false == jsonDest.isNull())
        {
            jsonCfg["dest"] = jsonDest.as<String>() == "true";
            isSuccessful = true;
        }
        if (false == jsonTwo.isNull())
        {
            jsonCfg["two"] = jsonTwo.as<String>() == "true";
            isSuccessful = true;
        } else {
            LOG_DEBUG("TWO was null");
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

bool ChicagoBusTrackerPlugin::hasTopicChanged(const String &topic)
{
    MutexGuard<MutexRecursive> guard(m_mutex);
    bool hasTopicChanged = m_hasTopicChanged;

    // only a single topic
    PLUGIN_NOT_USED(topic);

    m_hasTopicChanged = false;

    return hasTopicChanged;
}

void ChicagoBusTrackerPlugin::setSlot(const ISlotPlugin *slotInterf)
{
    m_slotInterf = slotInterf;
}

void ChicagoBusTrackerPlugin::start(uint16_t width, uint16_t height)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_routeCanvas.setPosAndSize(0, 0, RTE_SECTION_WIDTH, RTE_SECTION_HEIGHT);

    (void)m_routeCanvas.addWidget(m_routeTextWidget);

    /* The text canvas is left aligned to the icon canvas and it spans over
     * the whole display height.
     */
    m_arrivalsCanvas.setPosAndSize(RTE_SECTION_WIDTH + 1, 0, width - RTE_SECTION_WIDTH, height);
    (void)m_arrivalsCanvas.addWidget(m_arrivalsTextWidget);

    /* Choose font. */
    m_arrivalsTextWidget.setFont(Fonts::getFontByType(m_fontType));
    m_routeTextWidget.setFont(Fonts::getFontByType(m_fontType));

    /* The text widget inside the text canvas is left aligned on x-axis and
     * aligned to the center of y-axis.
     */
    if (height > m_arrivalsTextWidget.getFont().getHeight())
    {
        uint16_t diffY = height - m_arrivalsTextWidget.getFont().getHeight();
        uint16_t offsY = diffY / 2U;

        m_arrivalsTextWidget.move(0, offsY);
        m_routeTextWidget.move(0, offsY);
    }

    if (false == loadConfiguration())
    {
        if (false == saveConfiguration())
        {
            LOG_WARNING("Failed to create initial config file %s", getFullPathToConfiguration().c_str());
        }
    }
    else
    {
        updateTimestampLastUpdate();
    }

    m_cfgReloadTimer.start(CFG_RELOAD_PERIOD);

    initHttpClient();
}

void ChicagoBusTrackerPlugin::stop()
{
    String configurationFilename = getFullPathToConfiguration();
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_requestTimer.stop();
    m_cfgReloadTimer.stop();

    if (false != FILESYSTEM.remove(configurationFilename.c_str())) 
    {
        LOG_INFO("File %s removed", configurationFilename.c_str());
    };
}

void ChicagoBusTrackerPlugin::process(bool isConnected)
{
    Msg msg;
    MutexGuard<MutexRecursive> guard(m_mutex);

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
                m_requestTimer.start(UPDATE_PERIOD_SHORT);
            }
            else
            {
                m_requestTimer.start(UPDATE_PERIOD);
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
                m_requestTimer.start(UPDATE_PERIOD_SHORT);
            }
            else
            {
                m_requestTimer.start(UPDATE_PERIOD);
            }
        }
    }

    if (true == m_taskProxy.receive(msg))
    {
        switch (msg.type)
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

        default:
            /* Should never happen. */
            break;
        }
    }
}

void ChicagoBusTrackerPlugin::update(YAGfx &gfx)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    gfx.fillScreen(ColorDef::BLACK);
    m_routeCanvas.update(gfx);
    m_arrivalsCanvas.update(gfx);
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

String ChicagoBusTrackerPlugin::getApiKey()
{
    if (m_apiKey.isEmpty() || m_apiKey == "null") 
    {
        if (LittleFS.exists(m_apiKeyFilename))
        {
            fs::File sharedFile = LittleFS.open(m_apiKeyFilename, "r");
            if (sharedFile)
            {
                DynamicJsonDocument sharedDoc(512);
                deserializeJson(sharedDoc, sharedFile);
                m_apiKey = sharedDoc["apiKey"].as<String>();
                sharedFile.close();
            }
        }
    }
    return m_apiKey;
}

void ChicagoBusTrackerPlugin::requestStoreToPersistentMemory()
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_storeConfigReq = true;
}

bool ChicagoBusTrackerPlugin::startHttpRequest()
{
    bool status = false;

    m_apiKey = getApiKey();

    if ((false == m_apiKey.isEmpty()) && (false == m_rte.isEmpty()) && (false == m_dir.isEmpty()) && (false == m_stpid.isEmpty()))
    {
        String url = CHICAGO_BUS_BASE_URI;

        url += "/getpredictions?";
        url += "key=";
        url += m_apiKey;
        url += "&format=json";
        url += "&rt=";
        url += m_rte;
        url += "&dir=";
        url += m_dir;
        url += "&stpid=";
        url += m_stpid;
        url += "&top=2";

        if (true == m_client.begin(url))
        {
            if (false == m_client.GET())
            {
                LOG_WARNING("GET failed %s .", url.c_str());
            }
            else
            {
                LOG_INFO("GET success %s .", url.c_str());
                status = true;
            }
        }
    }

    return status;
}

void ChicagoBusTrackerPlugin::initHttpClient()
{
    /* Note: All registered callbacks are running in a different task context!
     *       Therefore it is not allowed to access a member here directly.
     *       The processing must be deferred via task proxy.
     */
    m_client.regOnResponse(
        [this](const HttpResponse &rsp)
        {
            const size_t JSON_DOC_SIZE = 2048U;
            DynamicJsonDocument *jsonDoc = new (std::nothrow) DynamicJsonDocument(JSON_DOC_SIZE);

            if (nullptr != jsonDoc)
            {
                size_t payloadSize = 0U;
                const char *payload = reinterpret_cast<const char *>(rsp.getPayload(payloadSize));
                StaticJsonDocument<FILTER_SIZE> filter;
                DeserializationError error;

                filter["bustime-response"]["prd"][0]["rt"] = true;
                filter["bustime-response"]["prd"][0]["prdctdn"] = true;
                filter["bustime-response"]["prd"][0]["rtdir"] = true;
                filter["bustime-response"]["prd"][0]["des"] = true;
                filter["bustime-response"]["prd"][0]["stpnm"] = true;

                if (true == filter.overflowed())
                {
                    LOG_ERROR("Less memory for filter available.");
                }

                error = deserializeJson(*jsonDoc, payload, payloadSize, DeserializationOption::Filter(filter));

                if (DeserializationError::Ok != error.code())
                {
                    LOG_ERROR("Invalid JSON message received: %s", error.c_str());
                }
                else
                {
                    Msg msg;

                    msg.type = MSG_TYPE_RSP;
                    msg.rsp = jsonDoc;

                    if (false == this->m_taskProxy.send(msg))
                    {
                        delete jsonDoc;
                        jsonDoc = nullptr;
                    }
                }
            }
        });
}

void ChicagoBusTrackerPlugin::handleWebResponse(DynamicJsonDocument &jsonDoc)
{
    JsonVariantConst response = jsonDoc["bustime-response"]["prd"];
    JsonVariantConst currentEstimate = response[0];
    JsonVariantConst nextEstimate = response[1];
    String rte = currentEstimate["rt"].as<String>();
    String est = currentEstimate["prdctdn"].as<String>();
    String dir = currentEstimate["rtdir"].as<String>();
    String des = currentEstimate["des"].as<String>();
    String stpnm = currentEstimate["stpnm"].as<String>();
    String nxt = nextEstimate["prdctdn"].as<String>();

    if (des != "null" && des != "" && true == m_dest)
    {
        m_routeResponsePart = m_displayColor + rte + " to " + des;
    }
    else if (rte == "null" || rte == "")
    {
        m_routeResponsePart = m_delayColor + m_rte;
    }
    else
    {
        m_routeResponsePart = m_displayColor + rte;
    }

    if (true == m_orig && stpnm != "" && stpnm != "null")
    {
        m_routeResponsePart += " from " + stpnm;
    }

    if (est == "DUE")
    {
        m_relevantResponsePart = m_dueColor + est;
    }
    else if (est == "DLY")
    {
        if (nxt != "null" && nxt != "")
        {
            m_relevantResponsePart = m_delayColor + est + m_displayColor + " / " + nxt + " min";
        }
        else
        {
            m_relevantResponsePart = m_delayColor + est;
        }
    }
    else if (est == "null" || est == "")
    {
        m_relevantResponsePart = m_delayColor + " NO DATA ";
    }
    else
    {
        m_relevantResponsePart = m_displayColor + est + " min";
    }

    if (true == m_two && nxt != "null" && nxt != "")
    {
        m_relevantResponsePart += m_displayColor + " / " + (nxt == "DLY" ? (m_delayColor + nxt) : nxt + " min");
    }

    LOG_DEBUG("Time prediction to print %s", m_relevantResponsePart.c_str());
    LOG_DEBUG("Route number received %s", m_routeResponsePart.c_str());

    m_arrivalsTextWidget.setFormatStr(m_relevantResponsePart);
    m_routeTextWidget.setFormatStr(m_routeResponsePart);
}

void ChicagoBusTrackerPlugin::clearQueue()
{
    Msg msg;

    while (true == m_taskProxy.receive(msg))
    {
        if (MSG_TYPE_RSP == msg.type)
        {
            delete msg.rsp;
            msg.rsp = nullptr;
        }
    }
}

void ChicagoBusTrackerPlugin::getRoutes(JsonObject &jsonRtes)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_requestTimer.start(UPDATE_PERIOD_SHORT);

    HTTPClient http;

    m_apiKey = getApiKey();

    String url = CHICAGO_BUS_BASE_URI;
    url += "/getroutes?";
    url += "key=";
    url += m_apiKey;
    url += "&format=json";

    http.begin(url);
    http.useHTTP10();
    int httpCode = http.GET();
    if (httpCode > 0)
    {
        if (httpCode == HTTP_CODE_OK)
        {
            Stream& input = http.getStream();
            DynamicJsonDocument jsonDoc(10240U);
            StaticJsonDocument<FILTER_SIZE> filter;
            
            filter["bustime-response"]["routes"][0]["rt"] = true;
            filter["bustime-response"]["routes"][0]["rtnm"] = true;
            
            DeserializationError error = deserializeJson(jsonDoc, input, DeserializationOption::Filter(filter));

            if (error)
            {
                LOG_ERROR(String(error.c_str()));
                jsonRtes.createNestedObject("error");
                jsonRtes["error"]["msg"] = String(error.f_str());
            }
            JsonArray routesOut = jsonRtes.createNestedArray("routes");
            JsonArray routes = jsonDoc["bustime-response"]["routes"].as<JsonArray>();
            for (int i = 0; i < routes.size(); i++)
            {
                JsonObject obj = routesOut.createNestedObject();
                obj["rt"] = routes[i]["rt"].as<String>();
                obj["rtnm"] = routes[i]["rtnm"];
            }
        }
            else
        {
            LOG_ERROR("HTTP %s Did not understand API Response", httpCode);
        }
    }
    else
    {
        LOG_ERROR("Negative HTTP response code, HttpClient error");
    }
    http.end();
}

void ChicagoBusTrackerPlugin::getDirections(JsonObject &jsonDirs)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_requestTimer.start(UPDATE_PERIOD_SHORT);

    String rt = jsonDirs["pars"]["rt"].as<String>();
    jsonDirs.remove("pars");
    jsonDirs["dirs"] = JsonArray();

    HTTPClient http;

    m_apiKey = getApiKey();

    String url = CHICAGO_BUS_BASE_URI;
    url += "/getdirections?";
    url += "key=";
    url += m_apiKey;
    url += "&rt=";
    url += rt;
    url += "&format=json";

    http.begin(url);
    http.useHTTP10();
    int httpCode = http.GET();
    if (httpCode > 0)
    {
        if (httpCode == HTTP_CODE_OK)
        {
            Stream& input = http.getStream();
            DynamicJsonDocument jsonDoc(192U);
            StaticJsonDocument<FILTER_SIZE> filter;
            
            filter["bustime-response"]["directions"][0]["dir"] = true;

            DeserializationError error = deserializeJson(jsonDoc, input);
            if (error)
            {
                LOG_ERROR(String(error.c_str()));
                jsonDirs.createNestedObject("error");
                jsonDirs["error"]["msg"] = String(error.f_str());
            }
            JsonArray dirsOut = jsonDirs.createNestedArray("dirs");
            JsonArray directions = jsonDoc["bustime-response"]["directions"].as<JsonArray>();
            for (int i = 0; i < directions.size(); i++)
            {
                dirsOut.add(directions[i]["dir"].as<String>());
            }
        }
            else
        {
            LOG_ERROR("HTTP %s Did not understand API Response", httpCode);
        }
    }
    else
    {
        LOG_ERROR("Negative HTTP response code, HttpClient error");
    }
    http.end();
}

void ChicagoBusTrackerPlugin::getStops(JsonObject &jsonStops)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_requestTimer.start(UPDATE_PERIOD_SHORT);

    String rt = jsonStops["pars"]["rt"].as<String>();
    String dir = jsonStops["pars"]["dir"].as<String>();
    jsonStops.remove("pars");
    jsonStops["stops"] = JsonArray();

    HTTPClient http;

    m_apiKey = getApiKey();

    String url = CHICAGO_BUS_BASE_URI;
    url += "/getstops?";
    url += "key=";
    url += m_apiKey;
    url += "&rt=";
    url += rt;
    url += "&dir=";
    url += dir;
    url += "&format=json";

    http.begin(url);
    http.useHTTP10();
    int httpCode = http.GET();
    if (httpCode > 0)
    {
        if (httpCode == HTTP_CODE_OK)
        {
            Stream& input = http.getStream();
            DynamicJsonDocument jsonDoc(8192U);
            StaticJsonDocument<FILTER_SIZE> filter;

            filter["bustime-response"]["stops"][0]["stpid"] = true;
            filter["bustime-response"]["stops"][0]["stpnm"] = true;

            DeserializationError error = deserializeJson(jsonDoc, input, DeserializationOption::Filter(filter));
            if (error)
            {
                LOG_ERROR(String(error.c_str()));
                jsonStops.createNestedObject("error");
                jsonStops["error"]["msg"] = String(error.f_str());
            }
            JsonArray stopsOut = jsonStops.createNestedArray("stops");
            JsonArray stops = jsonDoc["bustime-response"]["stops"].as<JsonArray>();
            for (int i = 0; i < stops.size(); i++)
            {
                JsonObject obj = stopsOut.createNestedObject();
                obj["stpid"] = stops[i]["stpid"].as<String>();
                obj["stpnm"] = stops[i]["stpnm"].as<String>();
            }
        }
        else
        {
            LOG_ERROR("HTTP %s Did not understand API Response", httpCode);
        }
    }
    else
    {
        LOG_ERROR("Negative HTTP response code, HttpClient error");
    }
    http.end();
}

void ChicagoBusTrackerPlugin::getConfiguration(JsonObject &jsonCfg) const
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    jsonCfg["apiKey"] = m_apiKey;
    jsonCfg["rte"] = m_rte;
    jsonCfg["dir"] = m_dir;
    jsonCfg["stpid"] = m_stpid;
    jsonCfg["orig"] = m_orig;
    jsonCfg["dest"] = m_dest;
    jsonCfg["two"] = m_two;
}

bool ChicagoBusTrackerPlugin::setConfiguration(JsonObjectConst &jsonCfg)
{
    // LOG_DEBUG("Set Config: rte: %s\ndir: %s\nstpid: %s\norig: %s\ndest: %s\ntwo: %s\n", jsonCfg["rte"].as<String>().c_str(), jsonCfg["dir"].as<String>().c_str(), jsonCfg["stpid"].as<String>().c_str(), jsonCfg["orig"].as<String>().c_str(), jsonCfg["dest"].as<String>().c_str(), jsonCfg["two"].as<String>().c_str());
    bool status = false;
    JsonVariantConst jsonApiKey = jsonCfg["apiKey"];
    JsonVariantConst jsonRte = jsonCfg["rte"];
    JsonVariantConst jsonDir = jsonCfg["dir"];
    JsonVariantConst jsonStpid = jsonCfg["stpid"];
    JsonVariantConst jsonOrig = jsonCfg["orig"];
    JsonVariantConst jsonDest = jsonCfg["dest"];
    JsonVariantConst jsonTwo = jsonCfg["two"];
    // LOG_DEBUG("ParsedConfig: rte: %s\ndir: %s\nstpid: %s\norig: %s\ndest: %s\ntwo: %s\n", jsonRte.as<String>().c_str(), jsonDir.as<String>().c_str(), jsonStpid.as<String>().c_str(), jsonOrig.as<String>().c_str(), jsonDest.as<String>().c_str(), jsonTwo.as<String>().c_str());

    if (false == jsonApiKey.is<String>())
    {
        LOG_WARNING("No API key");
    }
    else if (false == jsonRte.is<String>())
    {
        LOG_WARNING("No route provided");
    }
    else if (false == jsonDir.is<String>())
    {
        LOG_WARNING("No direction provided");
    }
    else if (false == jsonStpid.is<String>())
    {
        LOG_WARNING("No stop ID");
    }
    else if (false == jsonOrig.is<bool>())
    {
        LOG_WARNING("No origin preference provided");
    }
    else if (false == jsonDest.is<bool>())
    {
        LOG_WARNING("No destination preference provided");
    }
    else if (false == jsonTwo.is<bool>())
    {
        LOG_WARNING("No two predictions preference provided");
    }
    else
    {
        MutexGuard<MutexRecursive> guard(m_mutex);

        m_apiKey = jsonApiKey.as<String>();
        m_rte = jsonRte.as<String>();
        m_dir = jsonDir.as<String>();
        m_stpid = jsonStpid.as<String>();

        if (jsonOrig.as<bool>())
        {
            m_orig = true;
        }
        else
        {
            m_orig = false;
        }

        if (!jsonDest.as<bool>())
        {
            m_dest = false;
        }
        else
        {
            m_dest = true;
        }

        if (jsonTwo.as<bool>())
        {
            m_two = true;
        }
        else
        {
            m_two = false;
        }

        m_requestTimer.start(UPDATE_PERIOD_SHORT);
        m_hasTopicChanged = true;

        status = true;
    }
    return status;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
