/* MIT License
 *
 * Copyright (c) 2019 Andreas Merkle <web@blue-andi.de>
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
@brief  LED matrix
@author Andreas Merkle <web@blue-andi.de>

@section desc Description
@see LedMatrix.h

*******************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "DisplayMgr.h"
#include "Board.h"
#include "LedMatrix.h"

#include <TextWidget.hpp>
#include <BitmapWidget.hpp>
#include <LampWidget.hpp>

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/** Get number of array elements. */
#define ARRAY_NUM(__arr)    (sizeof(__arr) / sizeof((__arr)[0]))

/******************************************************************************
 * Types and classes
 *****************************************************************************/

/******************************************************************************
 * Prototypes
 *****************************************************************************/

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/* Initialize LED matrix instance. */
DisplayMgr  DisplayMgr::m_instance;

/* Initialize text widget name. */
const char* DisplayMgr::TEXT_WIDGET_NAME    = "text";

/* Initialize bitmap widget name. */
const char* DisplayMgr::BMP_WIDGET_NAME     = "bitmap";

/* Initialize lamp widget name. */
const char* DisplayMgr::LAMP_WIDGET_NAME    = "lamp";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void DisplayMgr::setLayout(uint8_t slotId, LayoutId layoutId)
{
    Widget* widget = NULL;

    if ((MAX_SLOTS <= slotId) ||
        (LAYOUT_ID_COUNT <= layoutId))
    {
        return;
    }

    widget = m_slots[slotId];
    destroyWidget(widget);

    switch(layoutId)
    {
    case LAYOUT_ID_0:
        createLayout0(m_slots[slotId]);
        break;

    case LAYOUT_ID_1:
        createLayout1(m_slots[slotId]);
        break;

    case LAYOUT_ID_2:
        createLayout2(m_slots[slotId]);
        break;

    case LAYOUT_ID_COUNT:
        /* Don't care about it. */
        break;

    default:
        /* Don't care about it. */
        break;
    }

    return;
}

void DisplayMgr::setText(uint8_t slotId, const String& str)
{
    Widget* widget = NULL;

    if ((MAX_SLOTS > slotId) &&
        (NULL != m_slots[slotId]))
    {
        widget = m_slots[slotId]->find(TEXT_WIDGET_NAME);
    }

    if (NULL != widget)
    {
        TextWidget* textWidget = NULL;

        if (0 == strcmp(widget->getType(), TextWidget::WIDGET_TYPE))
        {
            textWidget = static_cast<TextWidget*>(widget);

            textWidget->setStr(str);
        }
    }

    return;
}

void DisplayMgr::setBitmap(uint8_t slotId, const uint16_t* bitmap, uint16_t width, uint16_t heigth)
{
    Widget* widget = NULL;

    if ((MAX_SLOTS > slotId) &&
        (NULL != m_slots[slotId]))
    {
        widget = m_slots[slotId]->find(BMP_WIDGET_NAME);
    }

    if (NULL != widget)
    {
        BitmapWidget* bitmapWidget = NULL;

        if (0 == strcmp(widget->getType(), BitmapWidget::WIDGET_TYPE))
        {
            bitmapWidget = static_cast<BitmapWidget*>(widget);

            bitmapWidget->set(bitmap, width, heigth);
        }
    }

    return;
}

void DisplayMgr::setLamp(uint8_t slotId, uint8_t lampId, bool onState)
{
    Widget* widget = NULL;
    String  widgetName  = LAMP_WIDGET_NAME;
    
    widgetName += lampId;

    if ((MAX_SLOTS > slotId) &&
        (NULL != m_slots[slotId]))
    {
        widget = m_slots[slotId]->find(widgetName.c_str());
    }

    if (NULL != widget)
    {
        LampWidget* lampWidget = NULL;

        if (0 == strcmp(widget->getType(), LampWidget::WIDGET_TYPE))
        {
            lampWidget = static_cast<LampWidget*>(widget);

            lampWidget->setOnState(onState);
        }
    }

    return;
}

void DisplayMgr::process(void)
{
    uint8_t         nextSlot    = (m_activeSlotId + 1u) % MAX_SLOTS;
    LedMatrix&      matrix      = LedMatrix::getInstance();
    unsigned long   timestampMs = millis();
    uint32_t        deltaTimeS  = static_cast<uint32_t>((timestampMs - m_timestampOfLastChange) / 1000u);

    /* Jump to next slot? */
    if (m_period <= deltaTimeS)
    {
        /* Set next slot active */
        m_activeSlotId = nextSlot;

        /* Wait another period */
        m_timestampOfLastChange = timestampMs;
    }

    /* Anything to show? */
    if (NULL != m_slots[m_activeSlotId])
    {
        matrix.clear();
        m_slots[m_activeSlotId]->update(matrix);
    }

    return;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

DisplayMgr::DisplayMgr() :
    m_slots(),
    m_period(DEFAULT_PERIOD),
    m_activeSlotId(0u),
    m_timestampOfLastChange(0ul)
{
    uint8_t index = 0u;

    /* Initialize all slots */
    for(index = 0u; index < MAX_SLOTS; ++index)
    {
        m_slots[index] = NULL;
    }
}

DisplayMgr::~DisplayMgr()
{
    /* Destroy the widgets of all slots */
    clearSlots();
}

void DisplayMgr::destroyWidget(Widget*& widget)
{
    if (NULL != widget)
    {
        /* If the widget is a canvas, its children will be destroyed as well. */
        if (0 == strcmp(Canvas::WIDGET_TYPE, widget->getType()))
        {
            Canvas*             canvas      = static_cast<Canvas*>(widget);
            LinkedList<Widget*> children    = canvas->children();
            
            if (true == children.selectFirstElement())
            {
                do
                {
                    destroyWidget(*children.current());

                } while (true == children.next());
            }
        }
        else
        {
            delete widget;
            widget = NULL;
        }
    }

    return;
}

void DisplayMgr::clearSlots(void)
{
    uint8_t index = 0u;

    for(index = 0u; index < ARRAY_NUM(m_slots); ++index)
    {
        Widget* widget = m_slots[index];
        destroyWidget(widget);
    }

    return;
}

bool DisplayMgr::createLayout0(Canvas*& canvas) const
{
    bool        success     = true;
    TextWidget* textWidget  = new TextWidget();

    canvas = new Canvas(Board::LedMatrix::width, Board::LedMatrix::heigth, 0, 0);

    if ((NULL == canvas) ||
        (NULL == textWidget))
    {
        success = false;
    }
    else if (false == canvas->addWidget(*textWidget))
    {
        success = false;
    }
    else
    {
        textWidget->setName(TEXT_WIDGET_NAME);
    }
    
    if (false == success)
    {
        if (NULL != textWidget)
        {
            delete textWidget;
        }

        if (NULL != canvas)
        {
            delete canvas;
        }
    }

    return success;
}

bool DisplayMgr::createLayout1(Canvas*& canvas) const
{
    bool            success         = true;
    const uint16_t  BITMAP_WIDTH    = 8u;
    const uint16_t  BITMAP_HEIGHT   = 8u;
    const uint16_t  TEXT_WIDTH      = Board::LedMatrix::width - BITMAP_WIDTH - 1u;
    const uint16_t  TEXT_HEIGHT     = Board::LedMatrix::heigth;
    Canvas*         bitmapCanvas    = new Canvas(BITMAP_WIDTH, BITMAP_HEIGHT, 0, 0);
    BitmapWidget*   bitmapWidget    = new BitmapWidget();
    Canvas*         textCanvas      = new Canvas(TEXT_WIDTH, TEXT_HEIGHT, BITMAP_WIDTH, 0);
    TextWidget*     textWidget      = new TextWidget();

    canvas = new Canvas(Board::LedMatrix::width, Board::LedMatrix::heigth, 0, 0);

    if ((NULL == canvas) ||
        (NULL == bitmapCanvas) ||
        (NULL == bitmapWidget) ||
        (NULL == textCanvas) ||
        (NULL == textWidget))
    {
        success = false;
    }
    else if (false == canvas->addWidget(*bitmapCanvas))
    {
        success = false;
    }
    else if (false == canvas->addWidget(*textCanvas))
    {
        success = false;
    }
    else if (false == bitmapCanvas->addWidget(*bitmapWidget))
    {
        success = false;
    }
    else if (false == textCanvas->addWidget(*textWidget))
    {
        success = false;
    }
    else
    {
        bitmapWidget->setName(BMP_WIDGET_NAME);
        textWidget->setName(TEXT_WIDGET_NAME);
    }

    if (false == success)
    {
        if (NULL != canvas)
        {
            delete canvas;
        }

        if (NULL != bitmapCanvas)
        {
            delete bitmapCanvas;
        }

        if (NULL != bitmapWidget)
        {
            delete bitmapWidget;
        }

        if (NULL != textCanvas)
        {
            delete textCanvas;
        }

        if (NULL != textWidget)
        {
            delete textWidget;
        }
    }

    return success;
}

bool DisplayMgr::createLayout2(Canvas*& canvas) const
{
    bool            success         = true;
    const uint16_t  BITMAP_WIDTH    = 8u;
    const uint16_t  BITMAP_HEIGHT   = 8u;
    const uint16_t  TEXT_WIDTH      = Board::LedMatrix::width - BITMAP_WIDTH - 1u;
    const uint16_t  TEXT_HEIGHT     = 5u;
    const uint16_t  LAMP_WIDTH      = Board::LedMatrix::width - BITMAP_WIDTH;
    const uint16_t  LAMP_HEIGHT     = 1u;
    Canvas*         bitmapCanvas    = new Canvas(BITMAP_WIDTH, BITMAP_HEIGHT, 0, 0);
    BitmapWidget*   bitmapWidget    = new BitmapWidget();
    Canvas*         textCanvas      = new Canvas(TEXT_WIDTH, TEXT_HEIGHT, static_cast<int16_t>(BITMAP_WIDTH) + 1, 1);
    TextWidget*     textWidget      = new TextWidget();
    Canvas*         lampCanvas      = new Canvas(LAMP_WIDTH, LAMP_HEIGHT, static_cast<int16_t>(BITMAP_WIDTH), 0);
    LampWidget*     lampWidgets[]   = { NULL, NULL, NULL, NULL };
    uint8_t         index           = 0u;
    char            lampName[strlen(LAMP_WIDGET_NAME) + 2];
    uint8_t         lampNameNumPos  = 0;

    canvas = new Canvas(Board::LedMatrix::width, Board::LedMatrix::heigth, 0, 0);

    for(index = 0u; index < ARRAY_NUM(lampWidgets); ++index)
    {
        lampWidgets[index] = new LampWidget();

        if (NULL == lampWidgets[index])
        {
            break;
        }
    }

    if ((ARRAY_NUM(lampWidgets) <= index) ||
        (NULL == canvas) ||
        (NULL == bitmapCanvas) ||
        (NULL == bitmapWidget) ||
        (NULL == textCanvas) ||
        (NULL == textWidget))
    {
        success = false;
    }
    else if (false == canvas->addWidget(*bitmapCanvas))
    {
        success = false;
    }
    else if (false == canvas->addWidget(*textCanvas))
    {
        success = false;
    }
    else if (false == canvas->addWidget(*lampCanvas))
    {
        success = false;
    }
    else if (false == bitmapCanvas->addWidget(*bitmapWidget))
    {
        success = false;
    }
    else if (false == textCanvas->addWidget(*textWidget))
    {
        success = false;
    }
    else
    {
        bitmapWidget->setName(BMP_WIDGET_NAME);
        textWidget->setName(TEXT_WIDGET_NAME);

        strncpy(lampName, LAMP_WIDGET_NAME, ARRAY_NUM(lampName) - 1);
        lampName[ARRAY_NUM(lampName) - 1] = '\0';
        lampNameNumPos = strlen(lampName);

        for(index = 0u; index < ARRAY_NUM(lampWidgets); ++index)
        {
            if (false == lampCanvas->addWidget(*lampWidgets[index]))
            {
                success = false;
                break;
            }
            else
            {
                lampName[lampNameNumPos] = '\0' + index;
                lampWidgets[index]->setName(lampName);
            }
            
        }
    }

    if (false == success)
    {
        if (NULL != canvas)
        {
            delete canvas;
        }
        
        if (NULL != bitmapCanvas)
        {
            delete bitmapCanvas;
        }

        if (NULL != bitmapWidget)
        {
            delete bitmapWidget;
        }

        if (NULL != textCanvas)
        {
            delete textCanvas;
        }

        if (NULL != textWidget)
        {
            delete textWidget;
        }

        for(index = 0u; index < ARRAY_NUM(lampWidgets); ++index)
        {
            if (NULL != lampWidgets[index])
            {
                delete lampWidgets[index];
            }
        }
    }

    return success;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
