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
 * @brief  Update manager
 * @author Andreas Merkle <web@blue-andi.de>
 * 
 * @addtogroup update
 *
 * @{
 */

#ifndef UPDATEMGR_H
#define UPDATEMGR_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <ArduinoOTA.h>
#include <TextWidget.h>
#include <ProgressBar.h>
#include <SimpleTimer.hpp>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * The update manager handles everything around an on-the-air update.
 */
class UpdateMgr
{
public:

    /**
     * Get update manager instance.
     * 
     * @return Update manager
     */
    static UpdateMgr& getInstance()
    {
        static UpdateMgr instance; /* singleton idiom to force initialization in the first usage. */

        return instance;
    }

    /**
     * Initialize update manager, to be able to receive updates over-the-air.
     * 
     * @return If initialization is successful, it will return true otherwise false.
     */
    bool init(void);

    /**
     * Start over-the-air server.
     */
    void begin(void);

    /**
     * Stop over-the-air server.
     */
    void end(void);

    /**
     * Is an update in progress?
     * 
     * @return If an update is running it returns true otherwise false.
     */
    bool isUpdateRunning() const
    {
        return m_updateIsRunning;
    }

    /**
     * Is a restart requested?
     * This will be requested after a successful received new firmware
     * or filesystem.
     * 
     * @return If restart is requested, it will return true otherwise false.
     */
    bool isRestartRequested() const
    {
        return m_isRestartReq;
    }

    /**
     * Handle over-the-air update.
     */
    void process(void);

    /**
     * Request a restart.
     * 
     * @param[in] delay How long the restart shall be delayed in ms.
     */
    void reqRestart(uint32_t delay)
    {
        if (0U == delay)
        {
            m_isRestartReq = true;
        }
        else
        {
            m_timer.start(delay);
        }
    }

    /**
     * Show the user that the update starts.
     */
    void beginProgress(void);

    /**
     * Show the user the current update progress.
     * 
     * @param[in] progress  Progress in [0; 100] %
     */
    void updateProgress(uint8_t progress);

    /**
     * Show the user that the update is finished.
     */
    void endProgress(void);

    /** Over-the-air update password */
    static const char*  OTA_PASSWORD;

    /**
     * Fixed slot, which to use in the display manager.
     */
    static const uint8_t SLOT_ID = 1U;

private:

    /** Is the over-the-air update initialized? */
    bool                m_isInitialized;

    /** Is an update in progress? */
    bool                m_updateIsRunning;

    /**
     * Current update status in percent. Only used to avoid permanent logging
     * output during status update.
     */
    uint32_t            m_progress;

    /** Restart requested? */
    bool                m_isRestartReq;

    /** During the update the user shall be informed about whats going on. */
    TextWidget          m_textWidget;

    /** During the update the user shall be informed about the update progress. */
    ProgressBar         m_progressBar;

    /** Timer used to delay a restart request. */
    SimpleTimer         m_timer;

    /**
     * Constructs the update manager.
     */
    UpdateMgr();

    /**
     * Destroys the update manager.
     */
    ~UpdateMgr();

    /**
     * Over-the-air update start.
     */
    static void onStart(void);

    /**
     * Over-the-air update end.
     */
    static void onEnd(void);

    /**
     * On progress of over-the-air update.
     * 
     * @param[in] progress  Number of written bytes.
     * @param[in] total     Total size of the whole binary, which to update.
     */
    static void onProgress(unsigned int progress, unsigned int total);

    /**
     * On error of over-the-air update.
     * 
     * @param[in] error Error information
     */
    static void onError(ota_error_t error);

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* UPDATEMGR_H */

/** @} */