/* MIT License
 *
 * Copyright (c) 2019 - 2026 Andreas Merkle <web@blue-andi.de>
 * Copyright (c) 2026        Julie Hill <queenkjuul@pm.me>
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
 * @file   ChicagoBusTrackerPlugin.cpp
 * @brief  Chicago city bus tracker
 * @author Julie Hill <queenkjuul@pm.me>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "ChicagoBusTrackerPlugin.h"

#include <Logging.h>
#include <ArduinoJson.h>
#include <Util.h>
#include <math.h>
#include <HttpStatus.h>
#include <HTTPClient.h>
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

/* Initialize CTA Bus Tracker base URI.
 * Use http:// instead of https:// for less required heap memory for SSL connection.
 */
const char* ChicagoBusTrackerPlugin::CHICAGO_BUS_BASE_URI = "http://ctabustracker.com/bustime/api/v3";

/* Initialize shared API key config file. */
const char* ChicagoBusTrackerPlugin::API_KEY_FILE_PATH    = "/configuration/chicago_bus_api_key.json";

/* Initialize plugin topics. */
const char* ChicagoBusTrackerPlugin::TOPIC_CONFIG         = "chicagobus";
const char* ChicagoBusTrackerPlugin::TOPIC_ROUTES         = "chicagobusroutes";
const char* ChicagoBusTrackerPlugin::TOPIC_DIRS           = "chicagobusdirections";
const char* ChicagoBusTrackerPlugin::TOPIC_STOPS          = "chicagobusstops";

/* Initialize display colors. */
const char* ChicagoBusTrackerPlugin::COLOR_DISPLAY        = "{#FF5500}";
const char* ChicagoBusTrackerPlugin::COLOR_DELAY          = "{#FD1000}";
const char* ChicagoBusTrackerPlugin::COLOR_DUE            = "{#00AF02}";

// Share the key across all instances.
String ChicagoBusTrackerPlugin::apiKey                    = "";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void ChicagoBusTrackerPlugin::getTopics(JsonArray& topics) const
{
    (void)topics.add(TOPIC_CONFIG);
    (void)topics.add(TOPIC_ROUTES);
    (void)topics.add(TOPIC_STOPS);
    (void)topics.add(TOPIC_DIRS);
}

bool ChicagoBusTrackerPlugin::getTopic(const String& topic, JsonObject& value) const
{
    bool isSuccessful = false;

    if (true == topic.equals(TOPIC_CONFIG))
    {
        getConfiguration(value);
        isSuccessful = true;
    }
    else if (true == topic.equals(TOPIC_ROUTES))
    {
        const_cast<ChicagoBusTrackerPlugin*>(this)->getRoutes(value);
        isSuccessful = true;
    }
    else if (true == topic.equals(TOPIC_STOPS))
    {
        const_cast<ChicagoBusTrackerPlugin*>(this)->getStops(value);
        isSuccessful = true;
    }
    else if (true == topic.equals(TOPIC_DIRS))
    {
        const_cast<ChicagoBusTrackerPlugin*>(this)->getDirections(value);
        isSuccessful = true;
    }
    else
    {
        /* Not supported topic. */
    }

    return isSuccessful;
}

bool ChicagoBusTrackerPlugin::setTopic(const String& topic, const JsonObjectConst& value)
{
    bool isSuccessful = false;

    if (true == topic.equals(TOPIC_CONFIG))
    {
        const size_t        JSON_DOC_SIZE = 1024U;

        DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);

        JsonObject          jsonCfg    = jsonDoc.to<JsonObject>();
        JsonVariantConst    jsonApiKey = value["apiKey"];
        JsonVariantConst    jsonRte    = value["rte"];
        JsonVariantConst    jsonDir    = value["dir"];
        JsonVariantConst    jsonStpid  = value["stpid"];
        JsonVariantConst    jsonOrig   = value["orig"];
        JsonVariantConst    jsonDest   = value["dest"];
        JsonVariantConst    jsonTwo    = value["two"];

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
            apiKey            = jsonApiKey.as<String>();
            jsonCfg["apiKey"] = apiKey;

            File sharedFile   = LittleFS.open(API_KEY_FILE_PATH, "w");
            if (sharedFile)
            {
                DynamicJsonDocument sharedDoc(256);
                sharedDoc["apiKey"] = apiKey;
                serializeJson(sharedDoc, sharedFile);
                sharedFile.close();
            }
            isSuccessful = true;
        }
        if (false == jsonRte.isNull())
        {
            jsonCfg["rte"] = jsonRte.as<String>();
            isSuccessful   = true;
        }
        if (false == jsonDir.isNull())
        {
            jsonCfg["dir"] = jsonDir.as<String>();
            isSuccessful   = true;
        }
        if (false == jsonStpid.isNull())
        {
            jsonCfg["stpid"] = jsonStpid.as<String>();
            isSuccessful     = true;
        }
        if (false == jsonOrig.isNull())
        {
            jsonCfg["orig"] = jsonOrig.as<String>() == "true";
            isSuccessful    = true;
        }
        if (false == jsonDest.isNull())
        {
            jsonCfg["dest"] = jsonDest.as<String>() == "true";
            isSuccessful    = true;
        }
        if (false == jsonTwo.isNull())
        {
            jsonCfg["two"] = jsonTwo.as<String>() == "true";
            isSuccessful   = true;
        }

        if (true == isSuccessful)
        {
            JsonObjectConst jsonCfgConst = jsonCfg;

            isSuccessful                 = setConfiguration(jsonCfgConst);

            if (true == isSuccessful)
            {
                requestStoreToPersistentMemory();
            }
        }
    }

    return isSuccessful;
}

bool ChicagoBusTrackerPlugin::hasTopicChanged(const String& topic)
{
    MutexGuard<MutexRecursive> guard(m_mutex);
    bool                       hasTopicChanged = m_hasTopicChanged;

    /* Only a single topic, therefore its not necessary to check. */
    PLUGIN_NOT_USED(topic);

    m_hasTopicChanged = false;

    return hasTopicChanged;
}

void ChicagoBusTrackerPlugin::setSlot(const ISlotPlugin* slotInterf)
{
    m_slotInterf = slotInterf;
}

void ChicagoBusTrackerPlugin::start(uint16_t width, uint16_t height)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_view.init(width, height);

    PluginWithConfig::start(width, height);

    apiKey = getApiKey();
}

void ChicagoBusTrackerPlugin::stop()
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_requestTimer.stop();

    PluginWithConfig::stop();

    m_isAllowedToSend = false;

    if (RestService::INVALID_REST_ID != m_dynamicRestId)
    {
        RestService::getInstance().abortRequest(m_dynamicRestId);
        m_dynamicRestId = RestService::INVALID_REST_ID;
    }
}

void ChicagoBusTrackerPlugin::active(YAGfx& gfx)
{
    UTIL_NOT_USED(gfx);
}

void ChicagoBusTrackerPlugin::inactive()
{
    /* Nothing to do. */
}

void ChicagoBusTrackerPlugin::process(bool isConnected)
{
    uint32_t dynamicRestId;

    /* Acquire mutex for initial state check and update. */
    {
        MutexGuard<MutexRecursive> guard(m_mutex);
        bool                       isRestRequestRequired = false;

        PluginWithConfig::process(isConnected);

        /* Only if a network connection is established the required information
         * shall be periodically requested via REST API.
         */
        if (false == m_requestTimer.isTimerRunning())
        {
            if (true == isConnected)
            {
                isRestRequestRequired = true;
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
                isRestRequestRequired = true;
            }
        }

        /* Request of new bus arrival information via REST API required? */
        if (true == isRestRequestRequired)
        {
            /* Only one request can be sent at a time. */
            if (true == m_isAllowedToSend)
            {
                if (false == startHttpRequest())
                {
                    m_requestTimer.start(UPDATE_PERIOD_SHORT);
                }
                else
                {
                    m_requestTimer.start(UPDATE_PERIOD);
                    m_isAllowedToSend = false;
                }
            }
        }

        dynamicRestId = m_dynamicRestId;
    } /* Mutex released here to avoid lock inversion deadlock with RestService. */

    if (RestService::INVALID_REST_ID != dynamicRestId)
    {
        DynamicJsonDocument jsonDoc(0U);
        bool                isValidResponse;

        /* Get the response from the REST service. */
        if (true == RestService::getInstance().getResponse(dynamicRestId, isValidResponse, jsonDoc))
        {
            if (true == isValidResponse)
            {
                handleWebResponse(jsonDoc);
            }
            else
            {
                LOG_WARNING("Connection error.");

                MutexGuard<MutexRecursive> guard(m_mutex);
                m_requestTimer.start(UPDATE_PERIOD_SHORT);
            }

            MutexGuard<MutexRecursive> guard(m_mutex);
            m_dynamicRestId   = RestService::INVALID_REST_ID;
            m_isAllowedToSend = true;
        }
    }
}

void ChicagoBusTrackerPlugin::update(YAGfx& gfx)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_view.update(gfx);
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

String ChicagoBusTrackerPlugin::getApiKey() const
{
    if (apiKey.isEmpty() || apiKey == "null")
    {
        if (LittleFS.exists(API_KEY_FILE_PATH))
        {
            fs::File sharedFile = LittleFS.open(API_KEY_FILE_PATH, "r");
            if (sharedFile)
            {
                DynamicJsonDocument sharedDoc(512);
                deserializeJson(sharedDoc, sharedFile);
                apiKey = sharedDoc["apiKey"].as<String>();
                sharedFile.close();
            }
        }
    }
    return apiKey;
}

// TODO: Add locale option to config for CTA API V3
void ChicagoBusTrackerPlugin::getConfiguration(JsonObject& jsonCfg) const
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    jsonCfg["apiKey"] = apiKey;
    jsonCfg["rte"]    = m_rte;
    jsonCfg["dir"]    = m_dir;
    jsonCfg["stpid"]  = m_stpid;
    jsonCfg["orig"]   = m_orig;
    jsonCfg["dest"]   = m_dest;
    jsonCfg["two"]    = m_two; // TODO: multi-row layouts
}

bool ChicagoBusTrackerPlugin::setConfiguration(const JsonObjectConst& jsonCfg)
{
    bool             status     = false;
    JsonVariantConst jsonApiKey = jsonCfg["apiKey"];
    JsonVariantConst jsonRte    = jsonCfg["rte"];
    JsonVariantConst jsonDir    = jsonCfg["dir"];
    JsonVariantConst jsonStpid  = jsonCfg["stpid"];
    JsonVariantConst jsonOrig   = jsonCfg["orig"];
    JsonVariantConst jsonDest   = jsonCfg["dest"];
    JsonVariantConst jsonTwo    = jsonCfg["two"];

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

        apiKey  = jsonApiKey.as<String>();
        m_rte   = jsonRte.as<String>();
        m_dir   = jsonDir.as<String>();
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

        status            = true;
    }
    return status;
}

bool ChicagoBusTrackerPlugin::startHttpRequest()
{
    bool                            status = false;
    RestService::PreProcessCallback preProcessCallback =
        [this](const char* payload, size_t size, DynamicJsonDocument& doc) {
            return this->preProcessAsyncWebResponse(payload, size, doc);
        };

    if ((false == apiKey.isEmpty()) &&
        (false == m_rte.isEmpty()) &&
        (false == m_stpid.isEmpty()) &&
        (false == m_dir.isEmpty()))
    {
        String url       = CHICAGO_BUS_BASE_URI;

        /* Documentation:
         * https://www.transitchicago.com/developers/bustracker/
         */
        url             += "/getpredictions?key=";
        url             += apiKey;
        url             += "&format=json";
        url             += "&rt=";
        url             += m_rte;
        url             += "&dir=";
        url             += m_dir;
        url             += "&stpid=";
        url             += m_stpid;
        url             += "&top=3";

        m_dynamicRestId  = RestService::getInstance().get(url, preProcessCallback);

        if (RestService::INVALID_REST_ID == m_dynamicRestId)
        {
            LOG_WARNING("GET %s failed.", url.c_str());
        }
        else
        {
            status = true;
        }
    }

    return status;
}

bool ChicagoBusTrackerPlugin::preProcessAsyncWebResponse(const char* payload, size_t payloadSize, DynamicJsonDocument& jsonDoc)
{
    bool                isSuccessful = false;
    DynamicJsonDocument jsonFilterDoc(FILTER_SIZE);

    /* Example:
    {
        "bustime-response": {
            "prd": [
                {
                "tmstmp": "20250421 16:04",
                "typ": "A",
                "stpnm": "Madison & Jefferson",
                "stpid": "456",
                "vid": "8184",
                "dstp": 686,
                "rt": "20",
                "rtdd": "20",
                "rtdir": "Westbound",
                "des": "Austin",
                "prdtm": "20250421 16:06",
                "tablockid": "20 -803",
                "tatripid": "1040713",
                "origtatripno": "262522629",
                "dly": false,
                "dyn": 0,
                "prdctdn": "DUE",
                "zone": "",
                "psgld": "",
                "stst": 57120,
                "stsd": "2025-04-21",
                "flagstop": 0
                },
                ...
            ]
        }
    }

    */

    jsonFilterDoc["bustime-response"]["prd"][0]["rt"]      = true;
    jsonFilterDoc["bustime-response"]["prd"][0]["prdctdn"] = true;
    jsonFilterDoc["bustime-response"]["prd"][0]["rtdir"]   = true;
    jsonFilterDoc["bustime-response"]["prd"][0]["des"]     = true;
    jsonFilterDoc["bustime-response"]["prd"][0]["stpnm"]   = true;

    if (true == jsonFilterDoc.overflowed())
    {
        LOG_ERROR("JSON document size exceeded.");
    }
    else if ((nullptr == payload) ||
             (0U == payloadSize))
    {
        LOG_ERROR("No payload.");
    }
    else
    {
        DeserializationError error = deserializeJson(jsonDoc, payload, payloadSize, DeserializationOption::Filter(jsonFilterDoc));

        if (DeserializationError::Ok != error.code())
        {
            LOG_WARNING("JSON parse error: %s", error.c_str());
        }
        else
        {
            isSuccessful = true;
        }
    }

    return isSuccessful;
}

void ChicagoBusTrackerPlugin::handleWebResponse(const DynamicJsonDocument& jsonDoc)
{
    JsonVariantConst response        = jsonDoc["bustime-response"]["prd"];
    JsonVariantConst currentEstimate = response[0];
    JsonVariantConst nextEstimate    = response[1];
    // TODO: 32px-tall layout could fit 3 arrival times...
    // JsonVariantConst thirdEstimate = response[2];
    String rte                       = currentEstimate["rt"].as<String>();
    String est                       = currentEstimate["prdctdn"].as<String>();
    String dir                       = currentEstimate["rtdir"].as<String>();
    String des                       = currentEstimate["des"].as<String>();
    String stpnm                     = currentEstimate["stpnm"].as<String>();
    String nxt                       = nextEstimate["prdctdn"].as<String>();
    // String thd                       = thirdEstimate["prdctdn"].as<String>();

    if (des != "null" && des != "" && true == m_dest)
    {
        m_routeInfoText = COLOR_DISPLAY + rte + " to " + des;
    }
    else if (rte == "null" || rte == "")
    {
        m_routeInfoText = COLOR_DELAY + m_rte;
    }
    else
    {
        m_routeInfoText = COLOR_DISPLAY + rte;
    }

    if (true == m_orig && stpnm != "" && stpnm != "null")
    {
        m_routeInfoText += " from " + stpnm;
    }

    if (est == "DUE")
    {
        m_arrivalsInfotext = COLOR_DUE + est;
    }
    else if (est == "DLY")
    {
        if (nxt != "null" && nxt != "")
        {
            m_arrivalsInfotext = COLOR_DELAY + est + m_displayColor + " / " + nxt + " min";
        }
        else
        {
            m_arrivalsInfotext = COLOR_DELAY + est;
        }
    }
    else if (est == "null" || est == "")
    {
        m_arrivalsInfotext  = COLOR_DELAY;
        m_arrivalsInfotext += " NO DATA ";
    }
    else
    {
        m_arrivalsInfotext = COLOR_DISPLAY + est + " min";
    }

    if (true == m_two && nxt != "null" && nxt != "")
    {
        m_arrivalsInfotext += COLOR_DISPLAY;
        m_arrivalsInfotext += " / " +
                              (nxt == "DLY"
                                      ? (COLOR_DELAY + nxt)
                                  : nxt == "DUE"
                                      ? (COLOR_DUE + nxt)
                                      : nxt + " min");
    }

    LOG_DEBUG("Time prediction to print %s", m_arrivalsInfotext.c_str());
    LOG_DEBUG("Route number received %s", m_routeInfoText.c_str());

    m_view.setRouteInfoText(m_routeInfoText);
    m_view.setArrivalsInfoText(m_arrivalsInfotext);
}

void ChicagoBusTrackerPlugin::getRoutes(JsonObject& jsonRtes)
{
    MutexGuard<MutexRecursive> guard(m_mutex);
    HTTPClient                 http;

    apiKey      = getApiKey();

    String url  = CHICAGO_BUS_BASE_URI;
    url        += "/getroutes?";
    url        += "key=";
    url        += apiKey;
    url        += "&format=json";

    http.begin(url);
    http.useHTTP10();
    int httpCode = http.GET();
    if (httpCode > 0)
    {
        if (httpCode == HTTP_CODE_OK)
        {
            Stream&                         input = http.getStream();
            DynamicJsonDocument             jsonDoc(10240U);
            StaticJsonDocument<FILTER_SIZE> filter;

            filter["bustime-response"]["routes"][0]["rt"]   = true;
            filter["bustime-response"]["routes"][0]["rtnm"] = true;

            DeserializationError error                      = deserializeJson(jsonDoc, input, DeserializationOption::Filter(filter));

            if (error)
            {
                LOG_ERROR(String(error.c_str()));
                jsonRtes.createNestedObject("error");
                jsonRtes["error"]["msg"] = String(error.f_str());
            }
            JsonArray routesOut = jsonRtes.createNestedArray("routes");
            JsonArray routes    = jsonDoc["bustime-response"]["routes"].as<JsonArray>();

            for (int i = 0; i < routes.size(); i++)
            {
                JsonObject obj = routesOut.createNestedObject();
                obj["rt"]      = routes[i]["rt"];
                obj["rtnm"]    = routes[i]["rtnm"];
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


void ChicagoBusTrackerPlugin::getDirections(JsonObject& jsonDirs)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_requestTimer.start(UPDATE_PERIOD_SHORT);

    String rt = jsonDirs["pars"]["rt"].as<String>();
    jsonDirs.remove("pars");

    HTTPClient http;

    apiKey      = getApiKey();

    String url  = CHICAGO_BUS_BASE_URI;
    url        += "/getdirections?";
    url        += "key=";
    url        += apiKey;
    url        += "&rt=";
    url        += rt;
    url        += "&format=json";

    http.begin(url);
    http.useHTTP10();
    int httpCode = http.GET();
    if (httpCode > 0)
    {
        if (httpCode == HTTP_CODE_OK)
        {
            Stream&                         input = http.getStream();
            DynamicJsonDocument             jsonDoc(224U);
            StaticJsonDocument<FILTER_SIZE> filter;

            filter["bustime-response"]["directions"][0]["id"] = true;
            // TODO: return 'name' (localized) to web UI as well
            // filter["bustime-response"]["directions"][0]["name"] = true;

            DeserializationError error                        = deserializeJson(jsonDoc, input);
            if (error)
            {
                LOG_ERROR(String(error.c_str()));
                jsonDirs.createNestedObject("error");
                jsonDirs["error"]["msg"] = String(error.f_str());
            }
            JsonArray dirsOut    = jsonDirs.createNestedArray("dirs");
            JsonArray directions = jsonDoc["bustime-response"]["directions"].as<JsonArray>();
            for (int i = 0; i < directions.size(); i++)
            {
                dirsOut.add(directions[i]["id"]);
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


void ChicagoBusTrackerPlugin::getStops(JsonObject& jsonStops)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_requestTimer.start(UPDATE_PERIOD_SHORT);

    String rt  = jsonStops["pars"]["rt"].as<String>();
    String dir = jsonStops["pars"]["dir"].as<String>();
    jsonStops.remove("pars");
    jsonStops["stops"] = JsonArray();

    HTTPClient http;

    apiKey      = getApiKey();

    String url  = CHICAGO_BUS_BASE_URI;
    url        += "/getstops?";
    url        += "key=";
    url        += apiKey;
    url        += "&rt=";
    url        += rt;
    url        += "&dir=";
    url        += dir;
    url        += "&format=json";

    http.begin(url);
    http.useHTTP10();
    int httpCode = http.GET();
    if (httpCode > 0)
    {
        if (httpCode == HTTP_CODE_OK)
        {
            Stream&                         input = http.getStream();
            DynamicJsonDocument             jsonDoc(8192U);
            StaticJsonDocument<FILTER_SIZE> filter;

            filter["bustime-response"]["stops"][0]["stpid"] = true;
            filter["bustime-response"]["stops"][0]["stpnm"] = true;

            DeserializationError error                      = deserializeJson(jsonDoc, input, DeserializationOption::Filter(filter));
            if (error)
            {
                LOG_ERROR(String(error.c_str()));
                jsonStops.createNestedObject("error");
                jsonStops["error"]["msg"] = String(error.f_str());
            }
            JsonArray stopsOut = jsonStops.createNestedArray("stops");
            JsonArray stops    = jsonDoc["bustime-response"]["stops"].as<JsonArray>();
            for (int i = 0; i < stops.size(); i++)
            {
                JsonObject obj = stopsOut.createNestedObject();
                obj["stpid"]   = stops[i]["stpid"];
                obj["stpnm"]   = stops[i]["stpnm"];
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


/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
