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
@brief  Display manager
@author Andreas Merkle <web@blue-andi.de>

@section desc Description
The display manager is responsible for showing stuff in the right time on the
display. For this several time slots are provided. Each time slot can be
configured with a specific layout and contains the content to show.

*******************************************************************************/
/** @defgroup displaymgr Display manager
 * This module provides the display manager.
 *
 * @{
 */

#ifndef __DISPLAYMGR_H__
#define __DISPLAYMGR_H__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <Canvas.hpp>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * The display manager is responsible for showing stuff in the right time on the
 * display. For this several time slots are provided. Each time slot can be
 * configured with a specific layout and contains the content to show.
 */
class DisplayMgr
{
public:

    /**
     * Get LED matrix instance.
     * 
     * @return LED matrix
     */
    static DisplayMgr& getInstance(void)
    {
        return m_instance;
    }

    /**
     * Layouts to choose.
     */
    enum LayoutId
    {
        LAYOUT_ID_0 = 0,    /**< One text box */
        LAYOUT_ID_1,        /**< A bitmap icon box and a text box */
        LAYOUT_ID_2,        /**< A bitmap icon box, a text box and box with lampes */
        LAYOUT_ID_COUNT     /**< Number of layouts */
    };

    /**
     * Set layout for a slot.
     * 
     * @param[in] slotId    Slot id [0; N]
     * @param[in] layoutId  Layout id
     */
    void setLayout(uint8_t slotId, LayoutId layoutId);

    /**
     * Set text in the text widget of a slot.
     * 
     * @param[in] slotId    Slot id
     * @param[in] str       Text
     */
    void setText(uint8_t slotId, const String& str);

    /**
     * Set bitmap in the bitmap widget of a slot.
     * 
     * @param[in] slotId    Slot id
     * @param[in] bitmap    Bitmap buffer
     * @param[in] width     Bitmap width in pixel
     * @param[in] heigth    Bitmap heigth in pixel
     */
    void setBitmap(uint8_t slotId, const uint16_t* bitmap, uint16_t width, uint16_t heigth);

    /**
     * Set lamp state in the lamp widget of a slot.
     * 
     * @param[in] slotId    Slot id
     * @param[in] lampId    Lamp id
     * @param[in] onState   New lamp state on (true) or off (false)
     */
    void setLamp(uint8_t slotId, uint8_t lampId, bool onState);

    /**
     * Process the slots. This shall be called periodically in
     * a higher period than the DEFAULT_PERIOD.
     * 
     * It will handle which slot to show on the display.
     */
    void process(void);

    /** Maximum number of supported slots. */
    static const uint8_t    MAX_SLOTS      = 4u;

    /** Default period for changing slots in s. */
    static const uint32_t   DEFAULT_PERIOD = 10u;

    /** Text widget name, used for identification. */
    static const char*      TEXT_WIDGET_NAME;

    /** Bitmap widget name, used for identification. */
    static const char*      BMP_WIDGET_NAME;

    /** Lamp widget name, used for identification. */
    static const char*      LAMP_WIDGET_NAME;

private:

    /** Display manager instance */
    static DisplayMgr   m_instance;

    /** List of all slots. A slot is based on a canvas over the full LED matrix. */
    Canvas*             m_slots[MAX_SLOTS];

    /** Time duration between slots in s. */
    uint32_t            m_period;

    /** Id of the current active slot */
    uint8_t             m_activeSlotId;

    /** Timestamp of the last slot change in ms */
    uint64_t            m_timestampOfLastChange;

    /**
     * Construct LED matrix.
     */
    DisplayMgr();

    /**
     * Destroys LED matrix.
     */
    ~DisplayMgr();

    /* Prevent copying */
    DisplayMgr(const DisplayMgr& mgr);
    DisplayMgr& operator=(const DisplayMgr& mgr);

    /**
     * Destroy widget and all children.
     * 
     * @param[in] widget Widget, which to destroy.
     */
    void destroyWidget(Widget*& widget);

    /**
     * Clear all slots.
     */
    void clearSlots(void);

    /**
     * Create layout 0:
     * - One text box over the whole display.
     * - The text widget name is "text".
     * 
     * @param[in] canvas    Pointer to layout, based on a canvas.
     * 
     * @return Successful (true) or not (false).
     */
    bool createLayout0(Canvas*& canvas) const;

    /**
     * Create layout 1:
     * - A 8x8 bitmap box left. The bitmap widget name is "bitmap".
     * - Right side as one text box. The text widget name is "text".
     * 
     * @param[in] canvas    Pointer to layout, based on a canvas.
     * 
     * @return Successful (true) or not (false).
     */
    bool createLayout1(Canvas*& canvas) const;

    /**
     * Create layout 2:
     * - A 8x8 bitmap box left. The bitmap widget name is "bitmap".
     * - Right side as one text box. The text widget name is "text".
     * - Lower right side with signal lamps. The lamp widgets name is "lampN".
     * 
     * @param[in] canvas    Pointer to layout, based on a canvas.
     * 
     * @return Successful (true) or not (false).
     */
    bool createLayout2(Canvas*& canvas) const;

    /**
     * Find a widget by its name.
     * 
     * @param[in] widget        Start to find it with this widget.
     * @param[in] widgetName    Find the widget with this name.
     * 
     * @return If the widget is found, it will be returned otherwise NULL.
     */
    Widget* findWidget(Widget* widget, const char* widgetName);

    /**
     * Find a widget by its name.
     * 
     * @param[in] slotId        Search in this slot.
     * @param[in] widgetName    Find the widget with this name.
     * 
     * @return If the widget is found, it will be returned otherwise NULL.
     */
    Widget* findWidget(uint8_t slotId, const char* widgetName);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __DISPLAYMGR_H__ */

/** @} */