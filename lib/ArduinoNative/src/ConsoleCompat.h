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
 * @file   ConsoleCompat.h
 * @brief  Terminal input abstraction for Windows and Linux
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * The serial interface of the target delivers raw characters: no echo and no
 * line editing, because the terminal user is expected to do that. The mini
 * terminal is written for exactly that contract, it echoes every character and
 * handles the backspace on its own.
 *
 * The standard input of the host behaves different, it is line buffered and
 * echoes every character by the operating system. Therefore the terminal is
 * switched to the raw mode, which brings it in line with the target. The
 * platform differences are hidden here, so the serial interface itself is free
 * of any platform switch.
 *
 * The platform specific headers are intentionally not exposed here, otherwise
 * every user of this module would pull in e.g. the whole windows.h.
 *
 * @addtogroup TEST
 *
 * @{
 */

#ifndef CONSOLE_COMPAT_H
#define CONSOLE_COMPAT_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Platform differences of the terminal input.
 */
namespace ConsoleCompat
{

/******************************************************************************
 * Functions
 *****************************************************************************/

/**
 * Is the standard input connected to a terminal, which can be used
 * interactively?
 *
 * It is not the case if the standard input is redirected from a file or a pipe,
 * like in the CI. On POSIX it is not the case as well if the process runs in
 * the background, because reading from the controlling terminal would raise a
 * SIGTTIN and stop the process.
 *
 * @return If the terminal can be used interactively, it will return true
 *         otherwise false.
 */
bool isInteractive();

/**
 * Switch the terminal to the raw mode, which means no echo and no line
 * editing by the operating system.
 *
 * The previous mode is saved and restored by restoreMode(), which is
 * registered for the program exit and for a termination by signal too. Without
 * that the shell of the user would stay without echo after the program ended.
 *
 * The key which interrupts the program is kept enabled, otherwise the
 * application couldn't be stopped with CTRL-C anymore.
 *
 * @return If successful switched, it will return true otherwise false.
 */
bool enterRawMode();

/**
 * Restore the terminal mode, which was active before entering the raw mode.
 * It does nothing if the raw mode is not active.
 */
void restoreMode();

/**
 * Read a single byte from the terminal without blocking.
 *
 * A carriage return is reported as line feed, independent of the platform.
 * That way the caller sees the same end of line as on the target.
 *
 * @return Data byte or -1 if nothing is available.
 */
int readByte();

} /* namespace ConsoleCompat */

/******************************************************************************
 * Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* CONSOLE_COMPAT_H */

/** @} */
