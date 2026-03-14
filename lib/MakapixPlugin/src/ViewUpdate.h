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
 * @file   ViewUpdate.h
 * @brief  View update used to send view updates via MQTT periodically.
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup PLUGIN
 *
 * @{
 */

#ifndef VIEW_UPDATE_H
#define VIEW_UPDATE_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <WString.h>
#include "ViewUpdateTimer.h"
#include "Channel.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * The view update class is used to send view updates via MQTT periodically.
 */
class ViewUpdate
{
public:

    /**
     * Constructs the view update.
     *
     * Channel shall exist during the whole lifetime of the view update,
     * otherwise the behavior is undefined.
     *
     * @param[in] channel       Channel to play artworks from.
     */
    ViewUpdate(Channel& channel) :
        m_channel(channel),
        m_mqttInstance(0U),
        m_playerKey(),
        m_viewUpdateTimer(),
        m_artworkPostId(0U)
    {
    }

    /**
     * Destroys the view update.
     */
    ~ViewUpdate()
    {
    }

    /**
     * Configure the command handler.
     * It will unsubscribe any previous subscription.
     *
     * @param[in] playerKey      Player key
     * @param[in] mqttInstance   MQTT instance index
     */
    void configure(const String& playerKey, uint8_t mqttInstance);

    /**
     * Process the view update.
     */
    void process();

    /**
     * Set the artwork post ID, which is currently shown on the display.
     *
     * @param[in] postId   Artwork post ID
     */
    void setPostId(uint32_t postId);

    /**
     * Pause the playback.
     * It will stop the view update.
     */
    void pause();

    /**
     * Resume the playback.
     * It will resume the view update.
     */
    void play();

public:

    Channel&        m_channel;         /**< Channel to play artworks from. */
    uint8_t         m_mqttInstance;    /**< MQTT instance index. */
    String          m_playerKey;       /**< Player key. */
    ViewUpdateTimer m_viewUpdateTimer; /**< Timer for view updates via MQTT. */
    uint32_t        m_artworkPostId;   /**< Artwork post ID, which is currently shown on the display. */

    ViewUpdate()                                        = delete;
    ViewUpdate(const ViewUpdate& viewUpdate)            = delete;
    ViewUpdate& operator=(const ViewUpdate& viewUpdate) = delete;

    /**
     * Notify view update via MQTT.
     *
     * @return If view update notification was sent, it will return true otherwise false.
     */
    bool notifyViewUpdate();
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* VIEW_UPDATE_H */

/** @} */
