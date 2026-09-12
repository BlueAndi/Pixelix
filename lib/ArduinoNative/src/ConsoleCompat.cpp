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
 * @file   ConsoleCompat.cpp
 * @brief  Terminal input abstraction for Windows and Linux
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "ConsoleCompat.h"

#include <stdint.h>
#include <stdlib.h>
#include <signal.h>

#ifdef _WIN32

#include <windows.h>
#include <conio.h>

#else /* _WIN32 */

#include <unistd.h>
#include <termios.h>

#endif /* _WIN32 */

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

static void registerRestore();
static void handleTerminationSignal(int signalNumber);

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/** ASCII line feed value, which the caller expects as end of line. */
static const int ASCII_LF                     = 10;

/** ASCII carriage return value, which the enter key may deliver. */
static const int ASCII_CR                     = 13;

/** Is the raw mode active and therefore the saved terminal mode valid?
 * It is accessed by a signal handler, therefore the type is signal safe.
 */
static volatile sig_atomic_t gIsRawModeActive = 0;

#ifdef _WIN32

/** Console input mode, which was active before the raw mode was entered. */
static DWORD gSavedConsoleMode = 0U;

#else /* _WIN32 */

/** Terminal attributes of the standard input, which were active before the
 * raw mode was entered.
 */
static struct termios gSavedTermios;

#endif /* _WIN32 */

/******************************************************************************
 * Public Methods
 *****************************************************************************/

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

/******************************************************************************
 * External Functions
 *****************************************************************************/

bool ConsoleCompat::isInteractive()
{
    bool isAvailable = false;

#ifdef _WIN32

    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);

    if ((nullptr != hStdin) &&
        (INVALID_HANDLE_VALUE != hStdin))
    {
        /* A redirected standard input is a file or a pipe, only a console is a
         * character device.
         */
        if (FILE_TYPE_CHAR == GetFileType(hStdin))
        {
            isAvailable = true;
        }
    }

#else /* _WIN32 */

    if (0 != isatty(STDIN_FILENO))
    {
        /* Reading from the controlling terminal raises a SIGTTIN, which stops
         * the process, if it runs in the background. Changing the terminal
         * attributes raises a SIGTTOU for the same reason. Therefore the
         * terminal input is only used if the process is in the foreground.
         */
        if (getpgrp() == tcgetpgrp(STDIN_FILENO))
        {
            isAvailable = true;
        }
    }

#endif /* _WIN32 */

    return isAvailable;
}

bool ConsoleCompat::enterRawMode()
{
    bool isSuccessful = false;

    if (0 != gIsRawModeActive)
    {
        /* Guard: the raw mode is already active. */
        isSuccessful = true;
    }
    else
    {
#ifdef _WIN32

        HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);

        if ((nullptr != hStdin) &&
            (INVALID_HANDLE_VALUE != hStdin) &&
            (FALSE != GetConsoleMode(hStdin, &gSavedConsoleMode)))
        {
            /* No echo and no line editing by the console. The processed input
             * is kept enabled, so CTRL-C still terminates the application.
             */
            DWORD rawMode = gSavedConsoleMode & ~static_cast<DWORD>(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT);

            if (FALSE != SetConsoleMode(hStdin, rawMode))
            {
                gIsRawModeActive = 1;
                isSuccessful     = true;
            }
        }

#else /* _WIN32 */

        if (0 == tcgetattr(STDIN_FILENO, &gSavedTermios))
        {
            struct termios rawMode  = gSavedTermios;

            /* No echo and no line editing by the terminal driver. The signal
             * generation (ISIG) is kept enabled, so CTRL-C still terminates
             * the application.
             */
            rawMode.c_lflag        &= ~static_cast<tcflag_t>(ICANON | ECHO);

            /* The enter key delivers a carriage return, which is translated to
             * a line feed. That way it doesn't depend on the mode, which the
             * shell of the user had before.
             */
            rawMode.c_iflag        |= static_cast<tcflag_t>(ICRNL);

            /* Reading returns immediately, even if nothing is available. */
            rawMode.c_cc[VMIN]      = 0U;
            rawMode.c_cc[VTIME]     = 0U;

            if (0 == tcsetattr(STDIN_FILENO, TCSANOW, &rawMode))
            {
                gIsRawModeActive = 1;
                isSuccessful     = true;
            }
        }

#endif /* _WIN32 */

        if (true == isSuccessful)
        {
            registerRestore();
        }
    }

    return isSuccessful;
}

void ConsoleCompat::restoreMode()
{
    if (0 != gIsRawModeActive)
    {
        /* Cleared first, so a signal during the restore doesn't try it again. */
        gIsRawModeActive = 0;

#ifdef _WIN32

        HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);

        if ((nullptr != hStdin) &&
            (INVALID_HANDLE_VALUE != hStdin))
        {
            (void)SetConsoleMode(hStdin, gSavedConsoleMode);
        }

#else /* _WIN32 */

        (void)tcsetattr(STDIN_FILENO, TCSANOW, &gSavedTermios);

#endif /* _WIN32 */
    }
}

int ConsoleCompat::readByte()
{
    int data = -1;

#ifdef _WIN32

    /* _getch() reads the console input buffer directly and never echoes. */
    if (0 != _kbhit())
    {
        data = _getch();
    }

#else /* _WIN32 */

    uint8_t byte  = 0U;
    ssize_t count = read(STDIN_FILENO, &byte, sizeof(byte));

    if (0 < count)
    {
        data = static_cast<int>(byte);
    }

#endif /* _WIN32 */

    /* The enter key delivers a carriage return on Windows, but the caller
     * expects a line feed, like the terminal of the target delivers it.
     */
    if (ASCII_CR == data)
    {
        data = ASCII_LF;
    }

    return data;
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/

/**
 * Register the restore of the terminal mode for the program exit as well as
 * for a termination by signal. Every further call does nothing.
 */
static void registerRestore()
{
    static bool isRegistered = false;

    if (false == isRegistered)
    {
        isRegistered = true;

        (void)atexit(ConsoleCompat::restoreMode);

        /* A terminating signal doesn't call the atexit() handlers, therefore
         * the terminal is restored by an own handler. Without that the shell
         * of the user would stay without echo after a CTRL-C.
         *
         * Note, closing the console window on Windows is not handled, because
         * the console is gone anyway in that case.
         */
        (void)signal(SIGINT, handleTerminationSignal);
        (void)signal(SIGTERM, handleTerminationSignal);
    }
}

/**
 * Restore the terminal mode and continue with the default behaviour of the
 * signal, which terminates the process.
 *
 * @param[in] signalNumber  Number of the received signal.
 */
static void handleTerminationSignal(int signalNumber)
{
    ConsoleCompat::restoreMode();

    (void)signal(signalNumber, SIG_DFL);
    (void)raise(signalNumber);
}
