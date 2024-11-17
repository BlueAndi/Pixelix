/* MIT License
 *
 * Copyright (c) 2024 - 2024 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Json Configuration interface
 * @author Andreas Merkle <web@blue-andi.de>
 * 
 * @addtogroup hal
 *
 * @{
 */

#ifndef IJSON_CONFIG_H
#define IJSON_CONFIG_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <ArduinoJson.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * This interface defines the functions required for JSON bases configuration
 * updates. It is used to provide and apply configuration settings to and from
 * a REST API.
 */
class IJsonConfig
{
public:

    /**
     * Destroys the interface object
     */
    virtual ~IJsonConfig()
    {
    }

    /**
     * Get current active configuration in JSON format.
     * 
     * @param[out] cfg  Configuration
     */
    virtual void getConfiguration(JsonObject& jsonCfg) const = 0;

    /**
     * Apply configuration from JSON.
     * 
     * @param[in] cfg   Configuration
     * 
     * @return If successful set, it will return true otherwise false.
     */
    virtual bool setConfiguration(const JsonObjectConst& jsonCfg) = 0;

    /**
     * Merge JSON configuration with local settings to create a complete set.
     *
     * The received configuration may not contain all single key/value pair.
     * Therefore create a complete internal configuration and overwrite it
     * with the received one.
     *  
     * @param[out] jsonMerged  The complete config set with merge content from jsonSource.
     * @param[in]  jsonSource  The recevied congi set, which may not cover all keys.
     * @return     true        Keys needed merging.
     * @return     false       Nothing needed merging.
     */
    virtual bool mergeConfiguration(JsonObject& jsonMerged, const JsonObjectConst& jsonSource) = 0;
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* IJSON_CONFIG_H */

/** @} */