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
 * @file   ArduinoMain.cpp
 * @brief  Arduino entry point for the native environment
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * The Arduino framework provides the main() which calls setup() once and loop()
 * periodically. On the host it is provided here.
 *
 * Important: This file exports nothing else than the main(). A unit test
 * brings its own main() and is linked directly, therefore this object is never
 * pulled out of the library archive in the test environment. Adding any other
 * external symbol here would break that, because the linker would pull the
 * object in for that symbol and then report a duplicated main().
 *
 * The application works inside the directory given by the --work-dir option,
 * like PlatformIO starts it, see scripts/deploy_native_fs.py. Without the
 * option it works inside the current directory.
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Arduino.h"

#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

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

extern void        setup();
extern void        loop();

static const char* getWorkDir(int argc, char** argv);
static bool        changeWorkDir(const char* workDir);

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/** Command line option, which selects the working directory. */
static const char* WORK_DIR_OPTION = "--work-dir";

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

/**
 * Main entry point.
 *
 * @param[in] argc  Number of command line arguments
 * @param[in] argv  Command line arguments
 *
 * @return Program exit code
 */
extern int main(int argc, char** argv)
{
    int         exitCode = 0;
    const char* workDir  = getWorkDir(argc, argv);

    if ((nullptr != workDir) &&
        (false == changeWorkDir(workDir)))
    {
        /* Running in the wrong directory would litter it with runtime files,
         * therefore abort.
         */
        exitCode = 1;
    }
    else
    {
        setup();

        for (;;)
        {
            loop();
        }
    }

    return exitCode;
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/

/**
 * Get the working directory from the command line arguments.
 *
 * @param[in] argc  Number of command line arguments
 * @param[in] argv  Command line arguments
 *
 * @return Working directory, or nullptr if the option is not given.
 */
static const char* getWorkDir(int argc, char** argv)
{
    const char* workDir = nullptr;
    int         index   = 1;

    /* The option needs a value, therefore the last argument is never the option. */
    while ((nullptr == workDir) && ((argc - 1) > index))
    {
        if (0 == strcmp(argv[index], WORK_DIR_OPTION))
        {
            workDir = argv[index + 1];
        }

        ++index;
    }

    return workDir;
}

/**
 * Change the working directory of the process.
 *
 * @param[in] workDir   Working directory
 *
 * @return If successful, it will return true otherwise false.
 */
static bool changeWorkDir(const char* workDir)
{
    int result = 0;

#ifdef _WIN32
    result = _chdir(workDir);
#else
    result = chdir(workDir);
#endif

    if (0 != result)
    {
        (void)fprintf(stderr, "Failed to change the working directory to %s.\n", workDir);
    }

    return (0 == result);
}
