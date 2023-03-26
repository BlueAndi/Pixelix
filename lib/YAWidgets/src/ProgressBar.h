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
 * @brief  Progress bar
 * @author Andreas Merkle <web@blue-andi.de>
 * 
 * @addtogroup gfx
 *
 * @{
 */

#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <Widget.hpp>
#include <YAColor.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * A progress bar widget, showing the progress from 0 to 100%.
 */
class ProgressBar : public Widget
{
public:

    /**
     * Progress bar algorithm.
     */
    enum Algorithm
    {
        ALGORITHM_PROGRESS_BAR = 0, /**< Show progress bar always over the longer dimension. */
        ALGORITHM_PIXEL_WISE,       /**< Show progress pixel wise over the display. */
        ALGORITHM_MAX               /**< Number of algorithms */
    };

    /**
     * Constructs a progress bar widget.
     */
    ProgressBar() :
        Widget(WIDGET_TYPE),
        m_progress(0U),
        m_color(ColorDef::RED),
        m_algorithm(ALGORITHM_PROGRESS_BAR)
    {
    }

    /**
     * Constructs the progress bar, by assigning another.
     * 
     * @param[in] widget Progress bar, which to assign.
     */
    ProgressBar(const ProgressBar& widget) :
        Widget(WIDGET_TYPE),
        m_progress(widget.m_progress),
        m_color(widget.m_color),
        m_algorithm(widget.m_algorithm)
    {
    }

    /**
     * Destroys the progress bar.
     */
    ~ProgressBar()
    {
    }

    /**
     * Assign the content of a progress bar widget.
     * 
     * @param[in] widget Widget, which to assign
     */
    ProgressBar& operator=(const ProgressBar& widget)
    {
        if (&widget != this)
        {
            Widget::operator=(widget);
            
            m_progress  = widget.m_progress;
            m_color     = widget.m_color;
            m_algorithm = widget.m_algorithm;
        }

        return *this;
    }

    /**
     * Set progress in % [0; 100].
     * 
     * @param[in] progress  Progress as number from 0 to 100.
     */
    void setProgress(uint8_t progress)
    {
        if (100 < progress)
        {
            m_progress = 100;
        }
        else
        {
            m_progress = progress;
        }
    }

    /**
     * Set progress bar color.
     * 
     * @param[in] color Progress bar color
     */
    void setColor(const Color& color)
    {
        m_color = color;
        return;
    }

    /**
     * Set algorithm how to show progress on the display.
     * 
     * @param[in] algorithm Algorithm
     */
    void setAlgo(Algorithm algorithm)
    {
        if (ALGORITHM_MAX > algorithm)
        {
            m_algorithm = algorithm;
        }
    }

    /** Widget type string */
    static const char*  WIDGET_TYPE;

private:

    uint8_t     m_progress;     /**< Progress in % [0; 100]. */
    Color       m_color;        /**< Color of the progress bar */
    Algorithm   m_algorithm;    /**< Choosen algorithm */

    /**
     * Paint the widget with the given graphics interface.
     * 
     * @param[in] gfx   Graphics interface
     */
    void paint(YAGfx& gfx) override;
    
    /**
     * Fill the display pixel wise depended on current progress.
     * 
     * @param[in] gfx Graphics interface
     */
    void showProgressPixel(YAGfx& gfx);

    /**
     * Show current progress as bar over the longer dimension.
     * 
     * @param[in] gfx Graphics interface
     */
    void showProgressBar(YAGfx& gfx);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* PROGRESSBAR_H */

/** @} */