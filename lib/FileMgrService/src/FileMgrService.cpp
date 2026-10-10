/* MIT License
 *
 * Copyright (c) 2019 - 2026 Andreas Merkle <web@blue-andi.de>
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
 * @file   FileMgrService.cpp
 * @brief  File manager service
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "FileMgrService.h"

#include <Logging.h>
#include <FileSystem.h>
#include <FileUtil.h>
#include <Util.h>
#include <SettingsService.h>
#include <TopicHandlerService.h>
#include <BitmapWidget.h>

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

/* Initialize static variables. */
const char* FileMgrService::WORKING_DIRECTORY = "/configuration";
const char* FileMgrService::ENTITY_ID         = "fileMgrService";
const char* FileMgrService::TOPIC_FILES       = "files";
const char* FileMgrService::TOPIC_UPLOAD      = "upload";
const char* FileMgrService::TOPIC_REMOVE      = "remove";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

bool FileMgrService::start()
{
    bool                        isSuccessful        = true;
    SettingsService&            settings            = SettingsService::getInstance();
    TopicHandlerService&        topicHandlerService = TopicHandlerService::getInstance();
    JsonObjectConst             jsonExtra; /* Empty */
    ITopicHandler::GetTopicFunc getTopicFunc =
        [this](const String& topic, JsonObject& jsonValue) -> bool {
        return this->getTopic(topic, jsonValue);
    };
    TopicHandlerService::HasChangedFunc hasChangedFunc =
        [this](const String& topic) -> bool {
        return this->hasTopicChanged(topic);
    };
    ITopicHandler::SetTopicFunc uploadTopicFunc =
        [this](const String& topic, const JsonObjectConst& value) -> bool {
        return this->uploadTopic(topic, value);
    };
    ITopicHandler::UploadReqFunc uploadReqFunc =
        [this](const String& topic, const String& srcFilename, String& dstFilename) -> bool {
        return this->isUploadAccepted(topic, srcFilename, dstFilename);
    };
    ITopicHandler::SetTopicFunc removeTopicFunc =
        [this](const String& topic, const JsonObjectConst& value) -> bool {
        return this->removeTopic(topic, value);
    };
    ITopicHandler::UploadReqFunc uploadRejectFunc =
        [this](const String& topic, const String& srcFilename, String& dstFilename) -> bool {
        UTIL_NOT_USED(topic);
        UTIL_NOT_USED(srcFilename);
        UTIL_NOT_USED(dstFilename);

        return false;
    };
    /* Use the hostname as physical device id. */
    if (false == settings.open(true))
    {
        m_deviceId = settings.getHostname().getDefault();
    }
    else
    {
        m_deviceId = settings.getHostname().getValue();
        settings.close();
    }

    /* Register file upload and file remove topic. */
    topicHandlerService.registerTopic(m_deviceId, ENTITY_ID, TOPIC_FILES, jsonExtra, getTopicFunc, hasChangedFunc, nullptr, nullptr);
    topicHandlerService.registerTopic(m_deviceId, ENTITY_ID, TOPIC_UPLOAD, jsonExtra, nullptr, nullptr, uploadTopicFunc, uploadReqFunc);
    topicHandlerService.registerTopic(m_deviceId, ENTITY_ID, TOPIC_REMOVE, jsonExtra, nullptr, nullptr, removeTopicFunc, uploadRejectFunc);

    LOG_INFO("File manager service started.");

    return isSuccessful;
}

void FileMgrService::stop()
{
    TopicHandlerService& topicHandlerService = TopicHandlerService::getInstance();

    /* Unregister file upload and file remove topic. */
    topicHandlerService.unregisterTopic(m_deviceId, ENTITY_ID, TOPIC_UPLOAD);
    topicHandlerService.unregisterTopic(m_deviceId, ENTITY_ID, TOPIC_REMOVE);

    LOG_INFO("File manager service stopped.");
}

void FileMgrService::process()
{
    /* Nothing to do. */
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

bool FileMgrService::hasTopicChanged(const String& topic)
{
    bool hasChanged = m_hasFileTableChanged;

    /* The callback is dedicated to a topic, therefore the
     * topic parameter is not used.
     */
    UTIL_NOT_USED(topic);

    m_hasFileTableChanged = false;

    return hasChanged;
}

bool FileMgrService::getTopic(const String& topic, JsonObject& jsonValue)
{
    JsonArray jsonFileTable = jsonValue.createNestedArray("files");
    File      dir           = FILESYSTEM.open(WORKING_DIRECTORY, "r");
    File      fd            = dir.openNextFile();

    /* The callback is dedicated to a topic, therefore the
     * topic parameter is not used.
     */
    UTIL_NOT_USED(topic);

    /* List all files from the working directory. */
    while (true == fd)
    {
        if (false == fd.isDirectory())
        {
            const String fileName = fd.name();

            if (true == BitmapWidget::isImageTypeSupported(fileName))
            {
                String fullPath = fd.path();

                (void)jsonFileTable.add(fullPath);
            }
        }

        fd.close();
        fd = dir.openNextFile();
    }

    return true;
}

bool FileMgrService::uploadTopic(const String& topic, const JsonObjectConst& value)
{
    bool             isSuccessful = false;
    JsonVariantConst jsonFullPath = value["fullPath"];

    if (true == topic.equals(TOPIC_UPLOAD))
    {
        /* File upload? */
        if (false == jsonFullPath.isNull())
        {
            String fullPath = jsonFullPath.as<const char*>();

            if (false == fullPath.isEmpty())
            {
                LOG_INFO("File \"%s\" uploaded.", fullPath.c_str());

                /* Notify the clients about the changed file table. */
                m_hasFileTableChanged = true;
                isSuccessful          = true;
            }
        }
    }

    return isSuccessful;
}

bool FileMgrService::isUploadAccepted(const String& topic, const String& srcFilename, String& dstFilename)
{
    bool isAccepted = false;

    if (true == topic.equals(TOPIC_UPLOAD))
    {
        /* Accept only files with the right file extension. */
        if (false == BitmapWidget::isImageTypeSupported(srcFilename))
        {
            LOG_WARNING("File \"%s\" not supported.", srcFilename.c_str());
        }
        else
        {
            dstFilename  = WORKING_DIRECTORY;
            dstFilename += "/";
            dstFilename += srcFilename;

            /* Does the file already exist? */
            if (true == FILESYSTEM.exists(dstFilename))
            {
                LOG_WARNING("File \"%s\" already exists.", dstFilename.c_str());
                isAccepted = false;
            }
            else
            {
                LOG_INFO("File \"%s\" accepted.", dstFilename.c_str());
                isAccepted = true;
            }
        }
    }

    return isAccepted;
}

bool FileMgrService::removeTopic(const String& topic, const JsonObjectConst& value)
{
    bool isSuccessful = false;

    if (true == topic.equals(TOPIC_REMOVE))
    {
        JsonVariantConst jsonFileName = value["fileName"];

        if (false == jsonFileName.is<String>())
        {
            LOG_WARNING("JSON file name missing.");
        }
        else
        {
            const String fileName = FileUtil::getFileName(jsonFileName.as<const char*>());

            /* Accept only supported image files. Otherwise any other file in the
             * working directory, e.g. a plugin configuration, could be removed.
             */
            if (false == BitmapWidget::isImageTypeSupported(fileName))
            {
                LOG_WARNING("File \"%s\" not supported.", fileName.c_str());
            }
            else
            {
                String fullPath  = WORKING_DIRECTORY;

                fullPath        += "/";
                fullPath        += fileName;

                if (false == FILESYSTEM.remove(fullPath))
                {
                    LOG_WARNING("Remove file \"%s\" failed.", fullPath.c_str());
                }
                else
                {
                    LOG_INFO("Remove file \"%s\" successful.", fullPath.c_str());

                    /* Notify the clients about the changed file table. */
                    m_hasFileTableChanged = true;
                    isSuccessful          = true;
                }
            }
        }
    }

    return isSuccessful;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
