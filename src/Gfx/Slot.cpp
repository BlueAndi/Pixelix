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
 * @brief  Slot
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Slot.h"

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

/******************************************************************************
 * Public Methods
 *****************************************************************************/

Slot::Slot() :
    m_plugin(nullptr),
    m_duration(DURATION_DEFAULT),
    m_isLocked(false)
{
}

Slot::~Slot()
{
}

Slot::Slot(const Slot& slot) :
    m_plugin(slot.m_plugin),
    m_duration(slot.m_duration),
    m_isLocked(slot.m_isLocked)
{
}

Slot& Slot::operator=(const Slot& slot)
{
    if (this != (&slot))
    {
        m_plugin    = slot.m_plugin;
        m_duration  = slot.m_duration;
        m_isLocked  = slot.m_isLocked;
    }

    return *this;
}

IPluginMaintenance* Slot::getPlugin()
{
    return m_plugin;
}

bool Slot::setPlugin(IPluginMaintenance* plugin)
{
    bool status = false;

    if (false == m_isLocked)
    {
        if (nullptr != m_plugin)
        {
            m_plugin->setSlot(nullptr);
        }

        m_plugin = plugin;

        if (nullptr != m_plugin)
        {
            m_plugin->setSlot(this);
        }

        status = true;
    }

    return status;
}

bool Slot::isEmpty() const
{
    return (nullptr == m_plugin) ? true : false;
}

uint32_t Slot::getDuration() const
{
    return m_duration;
}

void Slot::setDuration(uint32_t duration)
{
    m_duration = duration;
}

void Slot::lock()
{
    m_isLocked = true;
}

void Slot::unlock()
{
    m_isLocked = false;
}

bool Slot::isLocked() const
{
    return m_isLocked;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
