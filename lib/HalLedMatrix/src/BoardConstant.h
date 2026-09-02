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
 * @file   BoardConstant.h
 * @brief  Board constants
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup HAL
 *
 * @{
 */

#ifndef BOARD_CONSTANT_H
#define BOARD_CONSTANT_H

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/******************************************************************************
 * Variables
 *****************************************************************************/

/** Board constants. */
namespace BoardConstant
{
/** ADC resolution in digits */
constexpr uint16_t ADC_RESOLUTION      = 4096U;

/** ADC reference voltage in mV */
constexpr uint16_t ADC_REF_VOLTAGE     = 3300U;

/** Pixelix supply voltage in volt */
constexpr uint8_t SUPPLY_VOLTAGE       = 5U;

/** Pixelix max. supply current in mA. */
constexpr uint32_t SUPPLY_CURRENT_MAX  = CONFIG_SUPPLY_CURRENT;

/** Max. current in mA per LED. */
constexpr uint32_t MAX_CURRENT_PER_LED = 60U;

} /* namespace BoardConstant */

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* BOARD_CONSTANT_H */

/** @} */