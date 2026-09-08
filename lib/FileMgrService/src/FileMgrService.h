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
 * @file   FileMgrService.h
 * @brief  File manager service
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup FILE_MGR_SERVICE
 *
 * @{
 */

#ifndef FILE_MGR_SERVICE_H
#define FILE_MGR_SERVICE_H

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <IService.hpp>
#include <WString.h>
#include <ArduinoJson.h>

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * The file manager service provides access to files in the filesystem and
 * downloads on request files.
 */
class FileMgrService : public IService
{
public:

    /**
     * Get the file manager service instance.
     *
     * @return File manager service instance
     */
    static FileMgrService& getInstance()
    {
        static FileMgrService instance; /* idiom */

        return instance;
    }

    /**
     * Start the service.
     *
     * @return If successful started, it will return true otherwise false.
     */
    bool start() final;

    /**
     * Stop the service.
     */
    void stop() final;

    /**
     * Process the service.
     */
    void process() final;

private:

    /**
     * The working directory for the file manager.
     */
    static const char* WORKING_DIRECTORY;

    /**
     * The entity id.
     */
    static const char* ENTITY_ID;

    /**
     * The topic to list all files in the file table.
     */
    static const char* TOPIC_FILES;

    /**
     * The topic to upload files.
     */
    static const char* TOPIC_UPLOAD;

    /**
     * The topic to remove files.
     */
    static const char* TOPIC_REMOVE;

    String             m_deviceId;            /**< Device id used for topic handling. */
    bool               m_hasFileTableChanged; /**< The file table has changed since last request? */

    /**
     * Constructs the service instance.
     */
    FileMgrService() :
        IService(),
        m_deviceId(),
        m_hasFileTableChanged(false)
    {
    }

    /**
     * Destroys the service instance.
     */
    ~FileMgrService()
    {
    }

    /* An instance shall not be copied. */
    FileMgrService(const FileMgrService& service);
    FileMgrService& operator=(const FileMgrService& service);

    /**
     * Get file table.
     *
     * @param[in]       topic       The topic name.
     * @param[in,out]   jsonValue   The JSON value.
     *
     * @return If successful, it will return true otherwise false.
     */
    bool getTopic(const String& topic, JsonObject& jsonValue);

    /**
     * Has the file table changed since last request?
     *
     * @param[in] topic The topic name.
     *
     * @return If changed, it will return true otherwise false.
     */
    bool hasTopicChanged(const String& topic);

    /**
     * Upload topic data.
     * Note, currently only JSON format is supported.
     *
     * @param[in]   topic   The topic which data shall be retrieved.
     * @param[in]   value   The topic value in JSON format.
     *
     * @return If successful it will return true otherwise false.
     */
    bool uploadTopic(const String& topic, const JsonObjectConst& value);

    /**
     * Is a upload request accepted or rejected?
     *
     * @param[in] topic         The topic which the upload belongs to.
     * @param[in] srcFilename   Name of the file, which will be uploaded if accepted.
     * @param[in] dstFilename   The destination filename, after storing the uploaded file.
     *
     * @return If accepted it will return true otherwise false.
     */
    bool isUploadAccepted(const String& topic, const String& srcFilename, String& dstFilename);

    /**
     * Remove topic data.
     * Note, currently only JSON format is supported.
     *
     * @param[in]   topic   The topic which data shall be retrieved.
     * @param[in]   value   The topic value in JSON format.
     *
     * @return If successful it will return true otherwise false.
     */
    bool removeTopic(const String& topic, const JsonObjectConst& value);
};

/******************************************************************************
 * Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* FILE_MGR_SERVICE_H */

/** @} */