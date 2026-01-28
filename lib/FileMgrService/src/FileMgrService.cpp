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
#include <Util.h>
#include <SettingsService.h>
#include <TopicHandlerService.h>
#include <JsonFile.h>
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
const char* FileMgrService::CONFIG_FILE_NAME  = "fileMgr.json";
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

    /* Setup file tables and create a configuration file on demand. */
    if (false == load())
    {
        m_hasFileTableChanged = true;
        m_isDirty             = true;
    }
    else
    {
        /* Remove file entries, where the file is missing. */
        if (true == checkForFiles(m_fileTable))
        {
            m_hasFileTableChanged = true;
            m_isDirty             = true;
        }
    }

    /* Add new files to file table. */
    if (true == scanForFiles(m_fileTable))
    {
        m_hasFileTableChanged = true;
        m_isDirty             = true;
    }

    m_timer.start(TIMER_PERIOD);

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

    m_timer.stop();

    /* Unregister file upload and file remove topic. */
    topicHandlerService.unregisterTopic(m_deviceId, ENTITY_ID, TOPIC_UPLOAD);
    topicHandlerService.unregisterTopic(m_deviceId, ENTITY_ID, TOPIC_REMOVE);

    /* Destroy file table. */
    clearFileTable(m_fileTable);
    clearFileTable(m_tmpFileTable);

    LOG_INFO("File manager service stopped.");
}

void FileMgrService::process()
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    if (true == m_timer.isTimeout())
    {
        removeFiles();

        m_timer.restart();
    }

    if (true == m_isDirty)
    {
        save();
    }
}

FileMgrService::FileId FileMgrService::getFileIdByName(const String& name)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    return getFileId(m_fileTable, name);
}

bool FileMgrService::getFileFullPathById(String& fullPath, FileId fileId)
{
    MutexGuard<MutexRecursive> guard(m_mutex);
    FileTableEntry*            entry = getFileEntry(m_fileTable, fileId);

    if (nullptr != entry)
    {
        if (true == entry->fullPath.isEmpty())
        {
            entry = nullptr;
        }
        else
        {
            fullPath = entry->fullPath;
        }
    }

    return (nullptr != entry);
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

bool FileMgrService::addFileEntry(FileTableEntry* fileTable, const String& fullPath)
{
    bool   isAdded = false;
    FileId fileId;

    for (fileId = 0U; fileId < MAX_FILES; ++fileId)
    {
        FileTableEntry* entry = &fileTable[fileId];

        if (true == entry->fullPath.isEmpty())
        {
            entry->fullPath  = fullPath;
            entry->removeReq = false;
            isAdded          = true;

            LOG_DEBUG("[%08X][%u] %s - added.", fileTable, fileId, fullPath.c_str());
            break;
        }
    }

    return isAdded;
}

void FileMgrService::removeFileEntry(FileTableEntry* fileTable, FileId fileId)
{
    if (MAX_FILES > fileId)
    {
        FileTableEntry* entry = &fileTable[fileId];

        if (false == entry->fullPath.isEmpty())
        {
            LOG_DEBUG("[%08X][%u] %s - removed.", fileTable, fileId, entry->fullPath.c_str());
        }

        entry->clear();
    }
}

void FileMgrService::clearFileTable(FileTableEntry* fileTable)
{
    FileId fileId;

    for (fileId = 0U; fileId < MAX_FILES; ++fileId)
    {
        removeFileEntry(fileTable, fileId);
    }
}

bool FileMgrService::scanForFiles(FileTableEntry* fileTable)
{
    bool anyChange = false;
    File fdRoot    = FILESYSTEM.open(WORKING_DIRECTORY, "r");
    File fd        = fdRoot.openNextFile();

    /* Scan for files only flat! */
    while (true == fd)
    {
        /* Filter for files with the requested extension. */
        if (false == fd.isDirectory())
        {
            String fullPath = fd.path();

            if (true == BitmapWidget::isImageTypeSupported(fullPath))
            {
                /* Add only new file to file table. */
                if (FILE_ID_INVALID != getFileId(fileTable, fullPath))
                {
                    /* Nothing to do. */
                    ;
                }
                /* Add file to file table. */
                else if (false == addFileEntry(fileTable, fd.path()))
                {
                    LOG_WARNING("File table full.");
                }
                else
                {
                    anyChange = true;
                }
                break;
            }
        }

        fd.close();
        fd = fdRoot.openNextFile();
    }

    /* Cleanup */
    if (true == fd)
    {
        fd.close();
    }

    if (true == fdRoot)
    {
        fdRoot.close();
    }

    return anyChange;
}

bool FileMgrService::checkForFiles(FileTableEntry* fileTable)
{
    bool   anyChange = false;
    FileId fileId;

    for (fileId = 0U; fileId < MAX_FILES; ++fileId)
    {
        FileTableEntry* entry = &fileTable[fileId];

        if ((false == entry->fullPath.isEmpty()) &&
            (false == FILESYSTEM.exists(entry->fullPath)))
        {
            entry->clear();

            anyChange = true;
        }
    }

    return anyChange;
}

bool FileMgrService::hasTopicChanged(const String& topic)
{
    MutexGuard<MutexRecursive> guard(m_mutex);
    bool                       hasChanged = m_hasFileTableChanged;

    m_hasFileTableChanged                 = false;

    return hasChanged;
}

bool FileMgrService::getTopic(const String& topic, JsonObject& jsonValue)
{
    FileId                     fileId;
    JsonArray                  jsonFileTable = jsonValue.createNestedArray("fileTable");
    MutexGuard<MutexRecursive> guard(m_mutex);

    /* The callback is dedicated to a topic, therefore the
     * topic parameter is not used.
     */
    UTIL_NOT_USED(topic);

    for (fileId = 0U; fileId < MAX_FILES; ++fileId)
    {
        FileTableEntry* entry         = &m_fileTable[fileId];
        JsonObject      jsonFileEntry = jsonFileTable.createNestedObject();

        jsonFileEntry["fileId"]       = fileId;

        if (true == entry->fullPath.isEmpty())
        {
            jsonFileEntry["fileName"] = "";
        }
        else
        {
            int lastSlash = entry->fullPath.lastIndexOf('/');

            /* Only the filename without path? */
            if (0 > lastSlash)
            {
                jsonFileEntry["fileName"] = entry->fullPath;
            }
            /* Filename with path. */
            else
            {
                /* Remove path. */
                jsonFileEntry["fileName"] = entry->fullPath.substring(lastSlash + 1U);
            }
        }
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
                MutexGuard<MutexRecursive> guard(m_mutex);
                FileId                     fileId = getFileIdByName(fullPath);

                /* New file uploaded? */
                if (FILE_ID_INVALID == fileId)
                {
                    if (false == addFileEntry(m_fileTable, fullPath))
                    {
                        LOG_WARNING("File table full.");

                        /* Avoid file flooding. */
                        if (false == FILESYSTEM.remove(fullPath))
                        {
                            LOG_WARNING("Failed to remove uploaded file: %s", fullPath.c_str());
                        }
                    }
                    else
                    {
                        m_hasFileTableChanged = true;
                        m_isDirty             = true;

                        isSuccessful          = true;
                    }
                }
                /* File uploaded which is already known. */
                else
                {
                    isSuccessful = true;
                }
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
        if (true == BitmapWidget::isImageTypeSupported(srcFilename))
        {
            dstFilename  = WORKING_DIRECTORY;
            dstFilename += "/";
            dstFilename += srcFilename;
        }
    }

    return isAccepted;
}

bool FileMgrService::removeTopic(const String& topic, const JsonObjectConst& value)
{
    bool isSuccessful = false;


    if (true == topic.equals(TOPIC_REMOVE))
    {
        JsonVariantConst jsonFileId = value["fileId"];

        if (true == jsonFileId.isNull())
        {
            LOG_WARNING("JSON file id missing.");
        }
        else
        {
            MutexGuard<MutexRecursive> guard(m_mutex);
            FileId                     fileId    = jsonFileId.as<String>().toInt();
            FileTableEntry*            fileEntry = getFileEntry(m_fileTable, fileId);

            if (nullptr == fileEntry)
            {
                LOG_WARNING("Invalid file id %u.", fileId);
            }
            else
            {
                fileEntry->removeReq = true;

                isSuccessful         = true;
            }
        }
    }

    return isSuccessful;
}

FileMgrService::FileTableEntry* FileMgrService::getFileEntry(FileTableEntry* fileTable, FileId fileId)
{
    FileTableEntry* entry = nullptr;

    if (MAX_FILES > fileId)
    {
        entry = &fileTable[fileId];
    }

    return entry;
}

FileMgrService::FileId FileMgrService::getFileId(FileTableEntry* fileTable, const String& fullPath)
{
    FileId fileId;

    for (fileId = 0U; fileId < MAX_FILES; ++fileId)
    {
        if (0 <= fileTable[fileId].fullPath.indexOf(fullPath))
        {
            break;
        }
    }

    if (MAX_FILES <= fileId)
    {
        fileId = FILE_ID_INVALID;
    }

    return fileId;
}

bool FileMgrService::load()
{
    bool                isSuccessful = true;
    JsonFile            jsonFile(FILESYSTEM);
    const size_t        JSON_DOC_SIZE = 4096U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
    String              fullConfigFileName  = WORKING_DIRECTORY;

    fullConfigFileName                     += "/";
    fullConfigFileName                     += CONFIG_FILE_NAME;

    if (false == jsonFile.load(fullConfigFileName, jsonDoc))
    {
        LOG_WARNING("Failed to load file %s.", fullConfigFileName.c_str());
        isSuccessful = false;
    }
    else if (true == jsonDoc.overflowed())
    {
        LOG_ERROR("JSON document size exceeded.");
        isSuccessful = false;
    }
    else
    {
        JsonArray jsonFileTable = jsonDoc["fileTable"].as<JsonArray>();
        FileId    fileId        = 0U;

        for (JsonObject jsonFileTableEntry : jsonFileTable)
        {
            JsonVariantConst jsonFullPath = jsonFileTableEntry["fullPath"];
            FileTableEntry*  entry        = &m_fileTable[fileId];

            entry->clear();

            if (true == jsonFullPath.is<String>())
            {
                entry->fullPath = jsonFullPath.as<const char*>();
            }

            ++fileId;
            if (MAX_FILES <= fileId)
            {
                break;
            }
        }
    }

    return isSuccessful;
}

void FileMgrService::save()
{
    String              installation;
    FileId              fileId;
    const size_t        JSON_DOC_SIZE = 4096U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
    JsonArray           jsonFileTable = jsonDoc.createNestedArray("fileTable");
    JsonFile            jsonFile(FILESYSTEM);
    String              fullConfigFileName  = WORKING_DIRECTORY;

    fullConfigFileName                     += "/";
    fullConfigFileName                     += CONFIG_FILE_NAME;

    for (fileId = 0U; fileId < MAX_FILES; ++fileId)
    {
        JsonObject jsonFileTableEntry  = jsonFileTable.createNestedObject();

        jsonFileTableEntry["fullPath"] = m_fileTable[fileId].fullPath;
    }

    if (true == jsonDoc.overflowed())
    {
        LOG_ERROR("JSON document size exceeded.");
    }
    else if (false == jsonFile.save(fullConfigFileName, jsonDoc))
    {
        LOG_ERROR("Couldn't save %s.", fullConfigFileName.c_str());
    }
    else
    {
        m_isDirty = false;
    }
}

void FileMgrService::removeFiles()
{
    FileId fileId;

    /* Walk through file table and check whether one entry is marked as
     * to be removed.
     */
    for (fileId = 0U; fileId < MAX_FILES; ++fileId)
    {
        FileTableEntry* entry = &m_fileTable[fileId];

        if ((true == entry->removeReq) &&
            (false == entry->fullPath.isEmpty()))
        {
            if (true == FILESYSTEM.remove(entry->fullPath))
            {
                removeFileEntry(m_fileTable, fileId);
                m_hasFileTableChanged = true;
                m_isDirty             = true;
            }
        }
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
