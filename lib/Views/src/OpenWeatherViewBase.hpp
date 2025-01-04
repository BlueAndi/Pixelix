/* MIT License
 *
 * Copyright (c) 2019 - 2025 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Base class for view for OpenWeather.
 * @author Andreas Merkle <web@blue-andi.de>
 * @addtogroup PLUGIN
 *
 * @{
 */

#ifndef OPEN_WEATHER_VIEW_BASE_HPP
#define OPEN_WEATHER_VIEW_BASE_HPP

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Layouts.h"
//#include "./layouts/OpenWeatherViewGeneric.h"
#include "./layouts/OpenWeatherView32x8.h"
#include "./layouts/OpenWeatherView32x16.h"
#include "./layouts/OpenWeatherView64x64.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * View for OpenWeather.
 * 
 * @tparam option   Layout which to choose
 */
template< Layout option >
class OpenWeatherView : public OpenWeatherView32x8
{
public:
    /**
     * Destroys the view.
     */
    virtual ~OpenWeatherView() = default;
};

/**
 * View for OpenWeather for 32x8 display.
 */
template<>
class OpenWeatherView<LAYOUT_32X8> : public OpenWeatherView32x8
{
public:
    /**
     * Destroys the view.
     */
    virtual ~OpenWeatherView() = default;
};

/**
 * View for OpenWeather for 32x16 display.
 */
template<>
class OpenWeatherView<LAYOUT_32X16> : public OpenWeatherView32x16
{
public:
    /**
     * Destroys the view.
     */
    virtual ~OpenWeatherView() = default;
};

/**
 * View for OpenWeather for 64x64 display.
 */
template<>
class OpenWeatherView<LAYOUT_64X64> : public OpenWeatherView64x64
{
public:
    /**
     * Destroys the view.
     */
    virtual ~OpenWeatherView() = default;
};

/** View for OpenWeather, considering the display size. */
using OpenWeatherViewBase = OpenWeatherView<LAYOUT_TYPE>;

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* OPEN_WEATHER_VIEW_BASE_HPP */

/** @} */
