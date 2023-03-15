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
 * @brief  Worm demo plugin
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "WormPlugin.h"

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

/* Initialize fixed colors */
const Color WormPlugin::WORM_HEAD_COLOR = ColorDef::BROWN;
const Color WormPlugin::MEAL_COLOR      = ColorDef::WHITE;

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void WormPlugin::start(uint16_t width, uint16_t height)
{
    m_wormMaxLen    = width * height * MAX_WORM_LENGTH / 100U;
    m_worms         = new(std::nothrow) Pos[MAX_WORMS * m_wormMaxLen];

    if (nullptr != m_worms)
    {
        createAllWorms(width, height);
    }
}

void WormPlugin::stop()
{
    if (nullptr != m_worms)
    {
        delete[] m_worms;
        m_worms = nullptr;
    }
}

void WormPlugin::active(YAGfx& gfx)
{
    gfx.fillScreen(ColorDef::BLACK);

    /* Force update */
    m_timer.start(0U);

    /* If in explosion phase, draw the last worm, meal and explosion state. */
    if (true == m_isExplosion)
    {
        drawAllWorms(gfx);
        drawMeal(gfx);
        drawExplosion(gfx);
    }
}

void WormPlugin::inactive()
{
    /* Nothing to do. */
}

void WormPlugin::update(YAGfx& gfx)
{    
    if (m_timer.isTimeout())
    {
        if (false == m_isExplosion)
        {
            gfx.fillScreen(ColorDef::BLACK);

            drawAllWorms(gfx);

            /* If no meal for the worm is available, some meal will be placed. */
            if (0U == m_mealLen)
            {
                placeMealRandom(gfx.getWidth(), gfx.getHeight());
            }

            drawMeal(gfx);

            /* Move worms. It may happen a worm eats something and is getting larger. */
            if (false == moveAllWormsRandom(gfx.getWidth(), gfx.getHeight()))
            {
                /* Force update */
                m_timer.start(0U);

                m_isExplosion = true;
            }
            else
            {
                m_timer.start(WORM_VELOCITY);
            }
        }
        else
        {
            const uint16_t  WIDTH           = gfx.getWidth();
            const uint16_t  HEIGHT          = gfx.getHeight();
            uint16_t        maxDisplayLen   = (WIDTH >= HEIGHT) ? WIDTH : HEIGHT;

            if (0U == m_explosionRadius)
            {
                m_timer.start(0U);
                ++m_explosionRadius;
            }
            else if ((maxDisplayLen / 2U) < m_explosionRadius)
            {
                createAllWorms(WIDTH, HEIGHT);

                m_isExplosion       = false;
                m_explosionRadius   = 0U;
            }
            else
            {
                drawExplosion(gfx);
                ++m_explosionRadius;
            }
        }
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void WormPlugin::placeMealRandom(uint16_t width, uint16_t height)
{
    uint8_t count       = 0U;
    uint8_t mealIndex   = 0U;

    while(MAX_MEALS > count)
    {
        m_meal[mealIndex] = { static_cast<int16_t>(random(width)), static_cast<int16_t>(random(height)) };

        if (false == isCollision(m_meal[mealIndex]))
        {
            ++mealIndex;
        }

        ++count;
    }

    m_mealLen = mealIndex;
}

void WormPlugin::eatMeal(uint16_t width, uint16_t height, uint8_t mealIndex)
{
    if (0U < m_mealLen)
    {
        uint8_t idx = mealIndex;    

        while((m_mealLen - 1U) > idx)
        {
            m_meal[idx].x = m_meal[idx + 1U].x;
            m_meal[idx].y = m_meal[idx + 1U].y;

            ++idx;
        }

        --m_mealLen;
    }
}

void WormPlugin::createWorm(uint8_t wormId, uint16_t width, uint16_t height)
{
    if ((nullptr != m_worms) &&
        (MAX_WORMS > wormId))
    {
        size_t  wormPos     = wormPosInArray(wormId);
        size_t  wormHeadIdx = wormPos + 0U;
        size_t  wormBodyIdx = wormPos + 1U;
        Pos     pos;
        uint8_t colorAngle  = static_cast<uint8_t>(random(UINT8_MAX + 1U));

        /* A worm has a random body color. */
        m_wormBodyColor[wormId].turnColorWheel(colorAngle);

        /* Its not allowed that a worm overlappes another worm. */
        do
        {
            uint8_t direction = random(4);

            /* Place head random */
            do
            {
                /* Worm shall not be overlapping the display border. */
                pos.x = static_cast<int16_t>(random(width - 2U) + 1U);
                pos.y = static_cast<int16_t>(random(height - 2U) + 1U);
            }
            while(true == isCollision(pos));

            m_worms[wormHeadIdx] = pos;

            /* Place one part of the body random */
            switch(direction)
            {
            case 0:
                pos.x += 1;
                break;

            case 1:
                pos.x -= 1;
                break;

            case 2:
                pos.y += 1;
                break;

            case 3:
                pos.y -= 1;
                break;

            default:
                break;
            }

        }
        while(true == isCollision(pos));

        m_worms[wormBodyIdx]    = pos;
        m_wormLen[wormId]       = MIN_WORM_LENGTH;
    }
}

void WormPlugin::createAllWorms(uint16_t width, uint16_t height)
{
    uint8_t wormId = 0U;

    while(MAX_WORMS > wormId)
    {
        createWorm(wormId, width, height);

        ++wormId;
    }
}

void WormPlugin::destroyWorm(uint8_t wormId)
{
    if (MAX_WORMS > wormId)
    {
        m_wormLen[wormId] = 0U;
    }
}

bool WormPlugin::isCollision(const Pos& pos)
{
    bool    isCollision = false;
    uint8_t wormId      = 0U;

    while((MAX_WORMS > wormId) && (false == isCollision))
    {
        size_t wormPos = wormPosInArray(wormId);
        size_t idx     = 0U;

        while((m_wormLen[wormId] > idx) && (false == isCollision))
        {
            if ((m_worms[wormPos + idx].x == pos.x) &&
                (m_worms[wormPos + idx].y == pos.y))
            {
                isCollision = true;
            }
            else
            {
                ++idx;
            }
        }

        ++wormId;
    }

    return isCollision;
}

bool WormPlugin::isMealFound(const Pos& pos, uint8_t& idx)
{
    bool    isSuccessful    = false;
    uint8_t mealIdx         = 0U;

    while((m_mealLen > mealIdx) && (false == isSuccessful))
    {
        if ((m_meal[mealIdx].x == pos.x) &&
            (m_meal[mealIdx].y == pos.y))
        {
            isSuccessful = true;
        }
        else
        {
            ++mealIdx;
        }
    }

    if (true == isSuccessful)
    {
        idx = mealIdx;
    }

    return isSuccessful;
}

bool WormPlugin::moveWormRandom(uint8_t wormId, uint16_t width, uint16_t height)
{
    bool    isSuccessful        = false;

    if ((nullptr != m_worms) &&
        (MAX_WORMS > wormId))
    {
        Pos     possibleMovements[4U];  /* 4 directions are possible left/right/up/down */
        uint8_t count               = 0U;
        size_t  wormPos             = wormPosInArray(wormId);

        /* Check for left */
        possibleMovements[count] = m_worms[wormPos];
        possibleMovements[count].x -= 1;

        if (0 > possibleMovements[count].x)
        {
            possibleMovements[count].x = width - 1;
        }

        if (false == isCollision(possibleMovements[count]))
        {
            ++count;
        }

        /* Check for right */
        possibleMovements[count] = m_worms[wormPos];
        possibleMovements[count].x += 1;

        if (width <= possibleMovements[count].x)
        {
            possibleMovements[count].x = 0;
        }

        if (false == isCollision(possibleMovements[count]))
        {
            ++count;
        }

        /* Check for up */
        possibleMovements[count] = m_worms[wormPos];
        possibleMovements[count].y -= 1;

        if (0 > possibleMovements[count].y)
        {
            possibleMovements[count].y = height - 1;
        }

        if (false == isCollision(possibleMovements[count]))
        {
            ++count;
        }

        /* Check for down */
        possibleMovements[count] =m_worms[wormPos];
        possibleMovements[count].y += 1;

        if (height <= possibleMovements[count].y)
        {
            possibleMovements[count].y = 0;
        }

        if (false == isCollision(possibleMovements[count]))
        {
            ++count;
        }

        /* If no movement is possible, the worm length will be decreased. */
        if (0U == count)
        {
            if (MIN_WORM_LENGTH < m_wormLen[wormId])
            {
                --m_wormLen[wormId];
                isSuccessful = true;
            }
        }
        /* The worm will move. */
        else
        {
            uint8_t possibilityIndex    = static_cast<uint8_t>(random(count));
            uint8_t mealIndex           = 0U;

            if (true == isMealFound(possibleMovements[possibilityIndex], mealIndex))
            {
                eatMeal(width, height, mealIndex);

                ++m_wormLen[wormId];

                if (m_wormMaxLen <= m_wormLen[wormId])
                {
                    m_isExplosion = true;
                }
            }

            if (false == m_isExplosion)
            {
                size_t idx = m_wormLen[wormId] - 1U;

                while(0 < idx)
                {
                    m_worms[wormPos + idx] = m_worms[wormPos + idx - 1U];

                    --idx;
                }

                m_worms[wormPos + 0U] = possibleMovements[possibilityIndex];
            }
            
            isSuccessful = true;
        }
    }

    return isSuccessful;
}

bool WormPlugin::moveAllWormsRandom(uint16_t width, uint16_t height)
{
    bool    isSuccessful    = false;
    uint8_t wormId          = 0U;

    while(MAX_WORMS > wormId)
    {
        if (true == moveWormRandom(wormId, width, height))
        {
            isSuccessful = true;
        }

        ++wormId;
    }

    return isSuccessful;
}

void WormPlugin::drawWorm(uint8_t wormId, YAGfx& gfx)
{
    if ((nullptr != m_worms) &&
        (MAX_WORMS > wormId))
    {
        size_t      wormPos         = wormPosInArray(wormId);
        size_t      idx             = 1U; /* 0 is the head, body starts at 1. */
        Color       bodyColor       = m_wormBodyColor[wormId];
        uint8_t     brightnessDelta = UINT8_MAX / (m_wormLen[wormId] - 1U); /* Consider only the body without head. */

        /* Draw worm head */       
        gfx.drawPixel(m_worms[wormPos + 0U].x,  m_worms[wormPos + 0U].y, WORM_HEAD_COLOR);

        /* Draw worm body */
        while(m_wormLen[wormId] > idx)
        {
            /* The body gets darker till the end. */
            bodyColor.setIntensity(UINT8_MAX - brightnessDelta * (idx - 1U));

            gfx.drawPixel(m_worms[wormPos + idx].x,  m_worms[wormPos + idx].y, bodyColor);
            ++idx;
        }
    }
}

void WormPlugin::drawAllWorms(YAGfx& gfx)
{
    uint8_t wormId = 0U;

    while(MAX_WORMS > wormId)
    {
        drawWorm(wormId, gfx);
        ++wormId;
    }
}

void WormPlugin::drawMeal(YAGfx& gfx)
{
    size_t idx = 0U;

    while(m_mealLen > idx)
    {
        gfx.drawPixel(m_meal[idx].x,  m_meal[idx].y, MEAL_COLOR);
        ++idx;
    }
}

void WormPlugin::drawExplosion(YAGfx& gfx)
{
    int16_t xMid = gfx.getWidth() / 2U;
    int16_t yMid = gfx.getHeight() / 2U;

    if (1U < m_explosionRadius)
    {
        gfx.fillCircle(xMid, yMid, m_explosionRadius - 1U, ColorDef::BLACK);
    }

    gfx.drawCircle(xMid, yMid, m_explosionRadius, ColorDef::RED);
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
