/* MIT License
 *
 * Copyright (c) 2019 - 2024 Andreas Merkle <web@blue-andi.de>
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
#include <SimpleTimer.hpp>
#include <Mutex.hpp>

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

    /** File id used for unique file identification. */
    typedef size_t FileId;

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

    /**
     * Get file id by file/path name or part of it.
     * 
     * @param[in] name  File/Path name or part of it.
     * 
     * @return If found, it will return its file id otherwise FILE_ID_INVALID.
     */
    FileId getFileIdByName(const String& name);

    /**
     * Get the full path by the file id.
     * 
     * @param[out]  fullPath    The full path to the file.
     * @param[in]   fileId      The file id of the file.
     * 
     * @return If successful found, it will return true otherwise false.
     */
    bool getFileFullPathById(String& fullPath, FileId fileId);

    /**
     * Invalid file id.
     */
    static const FileId FILE_ID_INVALID = 0xffU;

private:

    /** File table entry. */
    struct FileTableEntry
    {
        String  fullPath;   /**< Full path to file. */
        bool    removeReq;  /**< Is file removal requested? */
    
        /**
         * Construct a file table enty.
         */
        FileTableEntry() :
            fullPath(),
            removeReq(false)
        {
        }

        /**
         * Destroy the file table entry.
         * 
         */
        ~FileTableEntry()
        {
        }

        /**
         * Clear entry.
         */
        void clear()
        {
            fullPath.clear();
            removeReq = false;
        }
    };

    /**
     * Max. number of manageable files.
     */
    static const size_t MAX_FILES           = 20U;

    /**
     * The working directory for the file manager.
     */
    static const char*  WORKING_DIRECTORY;

    /**
     * File name of the configuration file.
     */
    static const char*  CONFIG_FILE_NAME;

    /**
     * The entity id.
     */
    static const char*  ENTITY_ID;

    /**
     * The topic to upload files.
     */
    static const char*  TOPIC_UPLOAD;

    /**
     * The topic to remove files.
     */
    static const char*  TOPIC_REMOVE;

    /**
     * Supported file extensions.
     */
    static const char*  FILE_EXTENSIONS[];

    /**
     * Timer period in ms to check whether the file table is dirty and needs
     * to be saved to the configuration file.
     * 
     * Note: If the period is too high, the icons web page might reload an old one
     * right after a new icon was uploaded.
     * 
     */
    static const uint32_t   TIMER_PERIOD    = 100U;

    String                  m_deviceId;                 /**< Device id used for topic handling. */
    FileTableEntry          m_fileTable[MAX_FILES];     /**< File table used for application requests. */
    FileTableEntry          m_tmpFileTable[MAX_FILES];  /**< File table with the uploaded files, only internal used. */
    bool                    m_isDirty;                  /**< The dirty flag signals that the file table is different than the configuration file. */
    SimpleTimer             m_timer;                    /**< Timer is used to check the dirty flag periodically. */
    mutable MutexRecursive  m_mutex;                    /**< Mutex used for concurrent access protection. */

    /**
     * Constructs the service instance.
     */
    FileMgrService() :
        IService(),
        m_deviceId(),
        m_fileTable(),
        m_tmpFileTable(),
        m_isDirty(false),
        m_timer(),
        m_mutex()
    {
        (void)m_mutex.create();
    }

    /**
     * Destroys the service instance.
     */
    ~FileMgrService()
    {
        (void)m_mutex.destroy();
    }

    /* An instance shall not be copied. */
    FileMgrService(const FileMgrService& service);
    FileMgrService& operator=(const FileMgrService& service);

    /**
     * Add file table entry to file table.
     * 
     * @param[in] fileTable File table where to add the entry.
     * @param[in] fullPath  Full path to file.
     * 
     * @return If successful it will return true otherwise false.
     */
    bool addFileEntry(FileTableEntry* fileTable, const String& fullPath);

    /**
     * Remove file table entry by file id.
     * 
     * @param[in] fileTable File table where to add the entry.
     * @param[in] fileId File id
     */
    void removeFileEntry(FileTableEntry* fileTable, FileId fileId);

    /**
     * Clear while file table.
     * 
     * @param[in] fileTable File table where to add the entry.
     */
    void clearFileTable(FileTableEntry* fileTable);

    /**
     * Scan for files and setup file table.
     * 
     * @param[in] fileTable File table where to add the entry.
     * @param[in] fileExtension File extensions
     * @param[in] count         Number of file extensions
     * 
     * @return If any new file found and added, it will return true otherwise false.
     */
    bool scanForFiles(FileTableEntry* fileTable, const char* fileExtension[], size_t count);

    bool checkForFiles(FileTableEntry* fileTable);

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

    /**
     * Get file entry by file id.
     * 
     * @param[in] fileTable File table where to add the entry.
     * @param[in] fileId    The id of the file which to get.
     * 
     * @return If found, it will return the file entry otherwise nullptr.
     */
    FileTableEntry* getFileEntry(FileTableEntry* fileTable, FileId fileId);

    /**
     * Get file id by full path or a part of it.
     * 
     * @param[in] fileTable File table where to add the entry.
     * @param[in] fullPath  Full path or part of it.
     * 
     * @return If found, it will return the file id or FILE_ID_INVALID.
     */
    FileId getFileId(FileTableEntry* fileTable, const String& fullPath);

    /**
     * Load file table from persistent memory.
     * 
     * @return If successful, it will return true otherwise false.
     */
    bool load();

    /**
     * Save file table to persistent memory.
     */
    void save();

    /**
     * Remove files on request from the regular file table.
     */
    void removeFiles();
};

/******************************************************************************
 * Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* FILE_MGR_SERVICE_H */

/** @} */