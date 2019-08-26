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
     * Get base canvas of slot.
     * 
     * @param[in] slotdId   Slot id [0; N]
     * 
     * @return If successful it returns the canvas, otherwise NULL.
     */
    Canvas* getSlot(uint8_t slotId)
    {
        Canvas* canvas = NULL;

        if (MAX_SLOTS > slotId)
        {
            canvas = m_slots[slotId];
        }

        return canvas;
    }

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
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __DISPLAYMGR_H__ */

/** @} */