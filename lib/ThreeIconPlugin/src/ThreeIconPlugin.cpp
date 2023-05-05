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
 * @brief  Three icon plugin
 * @author Yann Le Glaz <yann_le@web.de>

 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "ThreeIconPlugin.h"
#include "FileSystem.h"

#include <Logging.h>
#include <Util.h>

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

/* Initialize bitmap control topic. */
const char* ThreeIconPlugin::TOPIC_BITMAP               = "/bitmap";

/* Initialize bitmap image filename extension. */
const char* ThreeIconPlugin::FILE_EXT_BITMAP            = ".bmp";

/* Initialize sprite sheet parameter filename extension. */
const char* ThreeIconPlugin::FILE_EXT_SPRITE_SHEET      = ".sprite";

/* Initialize animation control topic. */
const char* ThreeIconPlugin::TOPIC_ANIMATION            = "/animation";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void ThreeIconPlugin::getTopics(JsonArray& topics) const
{
    uint8_t iconId = 0U;

    for(iconId = 0U; iconId < MAX_ICONS; ++iconId)
    {
        JsonObject  jsonIcon    = topics.createNestedObject();

        jsonIcon["name"]    = String(TOPIC_BITMAP) + "/" + iconId;
        jsonIcon["access"]  = "w"; /* Only icon upload is supported. */

        (void)topics.add(String(TOPIC_ANIMATION) + "/" + iconId);
    }
}

bool ThreeIconPlugin::getTopic(const String& topic, JsonObject& value) const
{
    bool isSuccessful = false;

    if (0U != topic.startsWith(String(TOPIC_BITMAP) + "/"))
    {
        isSuccessful = true;
    }
    else if (0U != topic.startsWith(String(TOPIC_ANIMATION) + "/"))
    {
        uint32_t    indexBeginIconId    = topic.lastIndexOf("/") + 1U;
        String      iconIdStr           = topic.substring(indexBeginIconId);
        uint8_t     iconId              = MAX_ICONS;
        bool        status              = Util::strToUInt8(iconIdStr, iconId);

        if ((true == status) &&
            (MAX_ICONS > iconId))
        {
            value["id"]         = iconId;
            value["repeat"]     = getIsRepeat(iconId);
            value["forward"]    = getIsForward(iconId);

            isSuccessful = true;
        }
    }
    else
    {
        ;
    }

    return isSuccessful;
}

bool ThreeIconPlugin::setTopic(const String& topic, const JsonObject& value)
{
    bool isSuccessful = false;

    if (0U != topic.startsWith(String(TOPIC_BITMAP) + "/"))
    {
        uint32_t            indexBeginIconId    = topic.lastIndexOf("/") + 1U;
        String              iconIdStr           = topic.substring(indexBeginIconId);
        uint8_t             iconId              = MAX_ICONS;
        bool                status              = Util::strToUInt8(iconIdStr, iconId);

        if ((true == status) &&
            (MAX_ICONS > iconId))
        {
            JsonVariantConst jsonIconPath = value["fullPath"];

            /* File upload? */
            if (false == jsonIconPath.isNull())
            {
                String iconPath = jsonIconPath.as<String>();
                isSuccessful = loadBitmap(iconPath, iconId);  
            }
            /* Control command */
            else
            {
                JsonVariantConst jsonClear = value["clear"];

                if (jsonClear.as<String>() == "true")
                {
                    clearBitmap(iconId);
                    isSuccessful = true;
                }
            }
        }
    }
    else if (0U != topic.startsWith(String(TOPIC_ANIMATION) + "/"))
    {
        uint32_t    indexBeginIconId    = topic.lastIndexOf("/") + 1U;
        String      iconIdStr           = topic.substring(indexBeginIconId);
        uint8_t     iconId              = MAX_ICONS;
        bool        status              = Util::strToUInt8(iconIdStr, iconId);

        if ((true == status) &&
            (MAX_ICONS > iconId) &&
            (false != m_isSpriteSheetAvailable[iconId]))
        {
            JsonVariantConst    jsonIsForward   = value["forward"];
            JsonVariantConst    jsonIsRepeat    = value["repeat"];

            if (false == jsonIsForward.isNull())
            {
                if (jsonIsForward.as<String>() == "false")
                {
                    setIsForward(iconId, false);
                    isSuccessful = true;
                }
                else if (jsonIsForward.as<String>() == "true")
                {
                    setIsForward(iconId, true);
                    isSuccessful = true;
                }
                else
                {
                    ;
                }
            }

            if (false == jsonIsRepeat.isNull())
            {
                if (jsonIsRepeat.as<String>() == "false")
                {
                    setIsRepeat(iconId, false);
                    isSuccessful = true;
                }
                else if (jsonIsRepeat.as<String>() == "true")
                {
                    setIsRepeat(iconId, true);
                    isSuccessful = true;
                }
                else
                {
                    ;
                }
            }
        }
    }
    else
    {
        ;
    }
    
    return isSuccessful;
}

bool ThreeIconPlugin::isUploadAccepted(const String& topic, const String& srcFilename, String& dstFilename)
{
    bool isAccepted = false;

    if (0U != topic.startsWith(String(TOPIC_BITMAP) + "/"))
    {
        uint32_t    indexBeginIconId    = topic.lastIndexOf("/") + 1U;
        String      iconIdStr           = topic.substring(indexBeginIconId);
        uint8_t     iconId              = MAX_ICONS;
        bool        status              = Util::strToUInt8(iconIdStr, iconId);
     
         /* Accept upload of bitmap file. */
        if ((false != status) && (0U != srcFilename.endsWith(FILE_EXT_BITMAP)))
        {
            dstFilename = getFileName(iconId, FILE_EXT_BITMAP);

            isAccepted = true;
        }
        /* Accept upload of a sprite sheet file. */
        else if ( (false != status) && (0U != srcFilename.endsWith(FILE_EXT_SPRITE_SHEET)))
        {
            dstFilename = getFileName(iconId, FILE_EXT_SPRITE_SHEET);

            isAccepted = true;
        }
        else
        {
            /* Not accepted. */
            ;
        }
    }

    return isAccepted;
}

void ThreeIconPlugin::start(uint16_t width, uint16_t height)
{
    uint8_t                     iconId      = 0U;
    const int16_t               DISTANCE    = (width - (MAX_ICONS * ICON_WIDTH)) / MAX_ICONS;
    MutexGuard<MutexRecursive>  guard(m_mutex);

    m_threeIconCanvas.setPosAndSize(0, 0, width, height);

    for(iconId = 0U; iconId < MAX_ICONS; ++iconId)
    { 
        int16_t x = (ICON_WIDTH + DISTANCE) * iconId + DISTANCE;

        (void)m_threeIconCanvas.addWidget(m_bitmapWidget[iconId]);
        m_bitmapWidget[iconId].move(x, 0);
    
        /* If there is already an icon in the filesystem for the respective icon-slot, it will be loaded.
         * First check whether it is a animated sprite sheet and if not, try
         * to load just a bitmap image.
         */
        m_isSpriteSheetAvailable[iconId] = m_bitmapWidget[iconId].loadSpriteSheet(FILESYSTEM, getFileName(iconId, FILE_EXT_SPRITE_SHEET), getFileName(iconId, FILE_EXT_BITMAP));

        if (false == m_isSpriteSheetAvailable[iconId])
        {   
            (void)m_bitmapWidget[iconId].load(FILESYSTEM, getFileName(iconId, FILE_EXT_BITMAP));
        }
    }
}

void ThreeIconPlugin::stop()
{
    uint8_t                     iconId = 0U;
    MutexGuard<MutexRecursive>  guard(m_mutex);

    for(iconId = 0U; iconId < MAX_ICONS; ++iconId)
    { 
        if (false != FILESYSTEM.remove(getFileName(iconId, FILE_EXT_BITMAP)))
        {
            LOG_INFO("File %s removed", getFileName(iconId, FILE_EXT_BITMAP).c_str());
        }

        if (false != FILESYSTEM.remove(getFileName(iconId, FILE_EXT_SPRITE_SHEET)))
        {
            LOG_INFO("File %s removed", getFileName(iconId, FILE_EXT_SPRITE_SHEET).c_str());
        }
    }
}

void ThreeIconPlugin::update(YAGfx& gfx)
{
    MutexGuard<MutexRecursive>  guard(m_mutex);

    gfx.fillScreen(ColorDef::BLACK);
    m_threeIconCanvas.update(gfx);
}

bool ThreeIconPlugin::loadBitmap(const String& filename, uint8_t iconId)
{
    bool status = false;

    if (MAX_ICONS > iconId)
    {
        MutexGuard<MutexRecursive> guard(m_mutex);

        if (0U != filename.endsWith(FILE_EXT_BITMAP))
        {
            status = m_bitmapWidget[iconId].load(FILESYSTEM, filename);

            /* Ensure that only the bitmap image file exists in the filesystem,
             * otherwise after a restart, the obsolete sprite sheet will
             * be loaded.
             */
            if (false != status)
            {
                (void)FILESYSTEM.remove(getFileName(iconId, FILE_EXT_SPRITE_SHEET));

                m_isSpriteSheetAvailable[iconId] = false;
            }
        }
        else if (0U != filename.endsWith(FILE_EXT_SPRITE_SHEET))
        {
            String bmpFilename = filename;

            bmpFilename.replace(FILE_EXT_SPRITE_SHEET, FILE_EXT_BITMAP);

            status = m_bitmapWidget[iconId].loadSpriteSheet(FILESYSTEM, filename,  bmpFilename);
            
            m_isSpriteSheetAvailable[iconId] = status;
        }
        else
        {
            /* Not supported. */
            ;
        }
    }

    return status;
}

bool ThreeIconPlugin::getIsForward(uint8_t iconId) const
{
    bool                        state = false;
    MutexGuard<MutexRecursive>  guard(m_mutex);

    if (MAX_ICONS > iconId)
    {
        state = m_bitmapWidget[iconId].isSpriteSheetForward();
    }
    
    return state;
}

void ThreeIconPlugin::setIsForward(uint8_t iconId, bool state)
{
    MutexGuard<MutexRecursive>  guard(m_mutex);

    if (MAX_ICONS > iconId)
    {
        m_bitmapWidget[iconId].setSpriteSheetForward(state);
    }
}

bool ThreeIconPlugin::getIsRepeat(uint8_t iconId) const
{
    bool                        state = false;
    MutexGuard<MutexRecursive>  guard(m_mutex);

    if (MAX_ICONS > iconId)
    {
        state = m_bitmapWidget[iconId].isSpriteSheetRepeatInfinite();
    }
    
    return state;
}

void ThreeIconPlugin::setIsRepeat(uint8_t iconId, bool state)
{
    MutexGuard<MutexRecursive>  guard(m_mutex);

    if (MAX_ICONS > iconId)
    {
        m_bitmapWidget[iconId].setSpriteSheetRepeatInfinite(state);
    }
}

void ThreeIconPlugin::clearBitmap(uint8_t iconId)
{
    if (MAX_ICONS > iconId)
    {
        MutexGuard<MutexRecursive> guard(m_mutex);

         m_bitmapWidget[iconId].clear(ColorDef::BLACK);

         (void)FILESYSTEM.remove(getFileName(iconId, FILE_EXT_BITMAP));

         if (true == m_isSpriteSheetAvailable[iconId])
         {
            (void)FILESYSTEM.remove(getFileName(iconId, FILE_EXT_SPRITE_SHEET));
            m_isSpriteSheetAvailable[iconId] = false;
         }
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

String ThreeIconPlugin::getFileName(uint8_t iconId, const String& ext)
{
    return PluginConfigFsHandler::generateFullPath(getUID(), "_" + String(iconId) + ext);
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
