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
 * @brief  Plugin view for 32x8 LED matrix
 * @author Andreas Merkle <web@blue-andi.de>
 * @addtogroup plugin
 *
 * @{
 */

#ifndef VOLUMIO_PLUGIN_VIEW_32X8_H
#define VOLUMIO_PLUGIN_VIEW_32X8_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <YAGfx.h>
#include <BitmapWidget.h>
#include <TextWidget.h>
#include <ProgressBar.h>
#include <FileSystem.h>
#include <Util.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** Internal plugin functionality. */
namespace _VolumioPlugin
{

/**
 * View for 32x8 LED matrix.
 */
class View
{
public:

    /**
     * Construct the layout.
     */
    View() :
        m_stdIconWidget(BITMAP_WIDTH, BITMAP_HEIGHT, 0, 0),
        m_stopIconWidget(BITMAP_WIDTH, BITMAP_HEIGHT, 0, 0),
        m_playIconWidget(BITMAP_WIDTH, BITMAP_HEIGHT, 0, 0),
        m_pauseIconWidget(BITMAP_WIDTH, BITMAP_HEIGHT, 0, 0),
        m_textWidget(TEXT_WIDTH, TEXT_HEIGHT, TEXT_X, 0),
        m_progressBar(PROGRESS_BAR_WIDTH, PROGRESS_BAR_HEIGHT, PROGRESS_BAR_X, PROGRESS_BAR_Y)
    {
        m_progressBar.setColor(ColorDef::RED);
        m_progressBar.setAlgo(ProgressBar::ALGORITHM_PROGRESS_BAR);
    }

    /**
     * Destroy the layout.
     */
    ~View()
    {
    }

    /**
     * Initialize view, which will prepare the widgets and the default values.
     */
    void init(uint16_t width, uint16_t height)
    {
        UTIL_NOT_USED(width);
        UTIL_NOT_USED(height);

        m_textWidget.setFormatStr("\\calign?");
        (void)m_stdIconWidget.load(FILESYSTEM, IMAGE_PATH_STD_ICON);
        (void)m_stopIconWidget.load(FILESYSTEM, IMAGE_PATH_STOP_ICON);
        (void)m_playIconWidget.load(FILESYSTEM, IMAGE_PATH_PLAY_ICON);
        (void)m_pauseIconWidget.load(FILESYSTEM, IMAGE_PATH_PAUSE_ICON);

        /* Disable all, except the standard icon. */
        m_stopIconWidget.disable();
        m_playIconWidget.disable();
        m_pauseIconWidget.disable();

        /* Move the text into the middle of the height. */
        if (height > m_textWidget.getFont().getHeight())
        {
            uint16_t diffY = height - m_textWidget.getFont().getHeight();
            uint16_t offsY = diffY / 2U;

            m_textWidget.move(TEXT_X, offsY);
        }
    }

    /**
     * Update the underlying canvas.
     * 
     * @param[in] gfx   Graphic functionality to draw on the underlying canvas.
     */
    void update(YAGfx& gfx)
    {
        gfx.fillScreen(ColorDef::BLACK);
        m_stdIconWidget.update(gfx);
        m_stopIconWidget.update(gfx);
        m_playIconWidget.update(gfx);
        m_pauseIconWidget.update(gfx);
        m_textWidget.update(gfx);
        m_progressBar.update(gfx);
    }

    /**
     * Get text (non-formatted).
     * 
     * @return Text
     */
    String getText() const
    {
        return m_textWidget.getStr();
    }

    /**
     * Get text (formatted).
     * 
     * @return Text
     */
    String getFormatText() const
    {
        return m_textWidget.getFormatStr();
    }

    /**
     * Set text (formatted).
     * 
     * @param[in] formatText    Formatted text to show.
     */
    void setFormatText(const String& formatText)
    {
        m_textWidget.setFormatStr(formatText);
    }

    /**
     * Get progress bar widget.
     * 
     * @return Progress bar widget
     */
    ProgressBar& getProgressBar()
    {
        return m_progressBar;
    }

    /**
     * Supported icons.
     */
    enum Icon
    {
        ICON_STD = 0,   /**< Standard Volumio icon. */
        ICON_STOP,      /**< Stop icon. */
        ICON_PLAY,      /**< Play icon. */
        ICON_PAUSE,     /**< Pause icon. */
        ICON_MAX        /**< Max. number of icons. */
    };

    /**
     * Load dedicated icon.
     * 
     * @param[in] type  The icon type which to set.
     */
    void showIcon(Icon type);

private:

    /**
     * Image path within the filesystem to standard icon.
     */
    static const char*      IMAGE_PATH_STD_ICON;

    /**
     * Image path within the filesystem to "stop" icon.
     */
    static const char*      IMAGE_PATH_STOP_ICON;

    /**
     * Image path within the filesystem to "play" icon.
     */
    static const char*      IMAGE_PATH_PLAY_ICON;

    /**
     * Image path within the filesystem to "pause" icon.
     */
    static const char*      IMAGE_PATH_PAUSE_ICON;

    /**
     * Bitmap width in pixels.
     */
    static const uint16_t   BITMAP_WIDTH        = CONFIG_LED_MATRIX_HEIGHT;

    /**
     * Bitmap height in pixels.
     */
    static const uint16_t   BITMAP_HEIGHT       = CONFIG_LED_MATRIX_HEIGHT;

    /**
     * Text width in pixels.
     */
    static const uint16_t   TEXT_WIDTH          = CONFIG_LED_MATRIX_WIDTH - BITMAP_WIDTH;

    /**
     * Text height in pixels.
     */
    static const uint16_t   TEXT_HEIGHT         = CONFIG_LED_MATRIX_HEIGHT;

    /**
     * Text widget x-coordinate in pixels.
     */
    static const int16_t    TEXT_X              = BITMAP_WIDTH;

    /**
     * Progress bar width in pixels.
     */
    static const uint16_t   PROGRESS_BAR_WIDTH  = TEXT_WIDTH;

    /**
     * Progress bar height in pixels.
     */
    static const uint16_t   PROGRESS_BAR_HEIGHT = 1U;

    /**
     * Progress bar x-coordinate in pixels.
     */
    static const int16_t    PROGRESS_BAR_X      = BITMAP_WIDTH;

    /**
     * Progress bar y-coordinate in pixels.
     */
    static const int16_t    PROGRESS_BAR_Y      = CONFIG_LED_MATRIX_HEIGHT - 1;

    BitmapWidget    m_stdIconWidget;    /**< Bitmap widget, used to show the standard icon. */
    BitmapWidget    m_stopIconWidget;   /**< Bitmap widget, used to show the stop icon. */
    BitmapWidget    m_playIconWidget;   /**< Bitmap widget, used to show the play icon. */
    BitmapWidget    m_pauseIconWidget;  /**< Bitmap widget, used to show the pause icon. */
    TextWidget      m_textWidget;       /**< Text widget used to show some text. */
    ProgressBar     m_progressBar;      /**< Progress bar for the music. */

    View(const View& other);
    View& operator=(const View& other);
};

} /* _VolumioPlugin */

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* VOLUMIO_PLUGIN_VIEW_32X8_H */

/** @} */
