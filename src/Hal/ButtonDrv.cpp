/* MIT License
 *
 * Copyright (c) 2019 - 2020 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Button driver
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "ButtonDrv.h"
#include "Board.h"

#include <Logging.h>

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

static void IRAM_ATTR isrButton(void* arg);

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/* Instantiate the button driver singleton. */
ButtonDrv ButtonDrv::m_instance;

/******************************************************************************
 * Public Methods
 *****************************************************************************/

ButtonDrv::Ret ButtonDrv::init()
{
    Ret         ret     = RET_OK;
    BaseType_t  osRet   = pdFAIL;

    /* Create semaphore to protect button state member. */
    m_semaphore = xSemaphoreCreateBinary();

    if (nullptr == m_semaphore)
    {
        ret = RET_ERROR;
    }
    /* The semaphore must be given, right after the creation! */
    else if (pdTRUE != xSemaphoreGive(m_semaphore))
    {
        ret = RET_ERROR;
    }
    else
    {
        /* Create button task for debouncing */
        osRet = xTaskCreateUniversal(   buttonTask,
                                        "buttonTask",
                                        BUTTON_TASK_STACKE_SIZE,
                                        this,
                                        1,
                                        &m_buttonTaskHandle,
                                        BUTTON_TASK_RUN_CORE);

        /* Failed to create task? */
        if (pdPASS != osRet)
        {
            ret = RET_ERROR;

            vSemaphoreDelete(m_semaphore);
            m_semaphore = nullptr;
        }
    }

    return ret;
}

ButtonDrv::State ButtonDrv::getState()
{
    State state = STATE_UNKNOWN;

    if (pdTRUE != xSemaphoreTake(m_semaphore, portMAX_DELAY))
    {
        LOG_WARNING("Update of button state not possible.");
    }
    else
    {
        state = m_state;

        if (STATE_TRIGGERED == m_state)
        {
            m_state = STATE_RELEASED;
        }

        if (pdTRUE != xSemaphoreGive(m_semaphore))
        {
            LOG_FATAL("Can't give semaphore back.");
        }
    }

    return state;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void ButtonDrv::buttonTask(void *parameters)
{
    ButtonDrv* buttonDrv = reinterpret_cast<ButtonDrv*>(parameters);

    /* The ISR shall notify about on change to determine whether the
     * pin state is stable or not.
     */
    attachInterruptArg( Board::userButtonIn.getPinNo(),
                        isrButton,
                        buttonDrv->m_buttonTaskHandle,
                        CHANGE);

    LOG_INFO("ButtonDrv task is ready.");

    /* The main loop scans several times during one debounce period
     * for any pin change. If there is no change, the state is
     * considered as stable.
     */
    for(;;)
    {
        /* Is button pin value stable? */
        if (0U == ulTaskNotifyTake(pdTRUE, BUTTON_DEBOUNCE_TIME))
        {
            /* Button state is stable, update it if applicable. */
            if (pdTRUE != xSemaphoreTake(buttonDrv->m_semaphore, portMAX_DELAY))
            {
                LOG_WARNING("Update of button state not possible.");
            }
            else
            {
                uint8_t buttonValue = Board::userButtonIn.read();

                /* If the button state is unknown, just check and set. */
                if (STATE_UNKNOWN == buttonDrv->m_state)
                {
                    if (HIGH == buttonValue)
                    {
                        buttonDrv->m_state = STATE_RELEASED;
                    }
                    else
                    {
                        buttonDrv->m_state = STATE_PRESSED;
                    }
                }
                /* Overwrite a triggered state is bad, because the application
                 * would miss it.
                 */
                else if (STATE_TRIGGERED != buttonDrv->m_state)
                {
                    /* Button pressed now? */
                    if ((STATE_RELEASED == buttonDrv->m_state) &&
                        (LOW == buttonValue))
                    {
                        buttonDrv->m_state = STATE_PRESSED;
                    }
                    /* Button released now? */
                    else if ((STATE_PRESSED == buttonDrv->m_state) &&
                             (HIGH == buttonValue))
                    {
                        buttonDrv->m_state = STATE_TRIGGERED;
                    }
                    else
                    {
                        ;
                    }
                }
                else
                {
                    ;
                }

                if (pdTRUE != xSemaphoreGive(buttonDrv->m_semaphore))
                {
                    LOG_FATAL("Can't give semaphore back.");
                }
            }
        }
    }

    return;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

/**
 * Button ISR which is called on change (falling- or rising-edge).
 *
 * @param[in] arg   Trigger counter for the button.
 */
static void IRAM_ATTR isrButton(void* arg)
{
    TaskHandle_t    taskHandle                  = reinterpret_cast<TaskHandle_t>(arg);
    BaseType_t      xHigherPriorityTaskWoken    = pdFALSE;

    vTaskNotifyGiveFromISR(taskHandle, &xHigherPriorityTaskWoken);

    /* If xHigherPriorityTaskWoken is now set to pdTRUE then a context switch
     * should be performed to ensure the interrupt returns directly to the highest
     * priority task. The macro used for this purpose is dependent on the port in
     * use and may be called portEND_SWITCHING_ISR().
     */
    if (pdTRUE == xHigherPriorityTaskWoken)
    {
        portYIELD_FROM_ISR();
    }

    return;
}
