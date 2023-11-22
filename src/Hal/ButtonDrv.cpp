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
 * @brief  Button driver
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "ButtonDrv.h"

#include <Board.h>
#include <Logging.h>
#include <esp_sleep.h>

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

const IoPin* ButtonDrv::BUTTON_PIN[BUTTON_ID_CNT] =
{
    &Board::buttonOkIn,
    &Board::buttonLeftIn,
    &Board::buttonRightIn
};

/**
 * If a button is triggered, the ISR will set it to true.
 * The task will set it to false and every time the task see true, it will
 * (re-)start the debounce timer.
 */
static ButtonId         gButtonId[BUTTON_ID_CNT] =
{
    BUTTON_ID_OK,
    BUTTON_ID_LEFT,
    BUTTON_ID_RIGHT
};

/** Number of elements in the button id queue. */
static const uint32_t   QUEUE_SIZE  = 10U;

/**
 * Button id queue, used to communicate from ISR to task.
 * Every time the task detects a pin level change, it will notify the task
 * about it by sending the corresponding button id via queue.
 */
static QueueHandle_t    gxQueue     = nullptr;

/******************************************************************************
 * Public Methods
 *****************************************************************************/

bool ButtonDrv::init()
{
    bool        isSuccessful    = true;
    BaseType_t  osRet           = pdFAIL;

    /* Create semaphore to protect the button trigger array, which is accessed
     * by the task and the ISR.
     */
    gxQueue = xQueueCreate(QUEUE_SIZE, sizeof(ButtonId));

    if (nullptr == gxQueue)
    {
        isSuccessful = false;
    }

    if (true == isSuccessful)
    {
        /* Create semaphore to protect button state member. */
        m_xSemaphore = xSemaphoreCreateBinary();

        if (nullptr == m_xSemaphore)
        {
            isSuccessful = false;
        }
        /* The semaphore must be given, right after the creation! */
        else if (pdTRUE != xSemaphoreGive(m_xSemaphore))
        {
            isSuccessful = false;
        }
        else
        {
            ;
        }
    }

    if (true == isSuccessful)
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
            isSuccessful = false;
        }
    }

    if (false == isSuccessful)
    {
        if (nullptr != gxQueue)
        {
            vQueueDelete(gxQueue);
            gxQueue = nullptr;
        }

        if (nullptr != m_xSemaphore)
        {
            vSemaphoreDelete(m_xSemaphore);
            m_xSemaphore = nullptr;
        }
    }

    return isSuccessful;
}

ButtonState ButtonDrv::getState(ButtonId buttonId)
{
    ButtonState state = BUTTON_STATE_UNKNOWN;

    if (BUTTON_ID_CNT > buttonId)
    {
        if (pdTRUE == xSemaphoreTake(m_xSemaphore, portMAX_DELAY))
        {
            state = m_state[buttonId];

            (void)xSemaphoreGive(m_xSemaphore);
        }
    }

    return state;
}

void ButtonDrv::registerObserver(IButtonObserver& observer)
{
    if (pdTRUE == xSemaphoreTake(m_xSemaphore, portMAX_DELAY))
    {
        uint8_t buttonIndex = 0U;

        m_observer = &observer;

        while(BUTTON_ID_CNT > buttonIndex)
        {
            m_observer->notify(static_cast<ButtonId>(buttonIndex), m_state[buttonIndex]);

            ++buttonIndex;
        }

        (void)xSemaphoreGive(m_xSemaphore);
    }
}

void ButtonDrv::unregisterObserver()
{
    if (pdTRUE == xSemaphoreTake(m_xSemaphore, portMAX_DELAY))
    {
        m_observer = nullptr;
        
        (void)xSemaphoreGive(m_xSemaphore);
    }
}

bool ButtonDrv::enableWakeUpSources()
{
    uint8_t buttonIdx           = 0U;
    bool    allButtonsReleased  = true;

    /* Ensure that no button is pressed anymore. */
    while(BUTTON_ID_CNT > buttonIdx)
    {
        uint8_t pinNo = BUTTON_PIN[buttonIdx]->getPinNo();

        if (IoPin::NC != pinNo)
        {
            gpio_num_t  gpioPinNum  = static_cast<gpio_num_t>(pinNo);

            if (0 == gpio_get_level(gpioPinNum))
            {
                allButtonsReleased = false;
            }
        }

        ++buttonIdx;
    }

    /* If no button is pressed anymore, enable them as wakeup source. */
    if (true == allButtonsReleased)
    {
        /* Use all available buttons as wakeup sources. */
        while(BUTTON_ID_CNT > buttonIdx)
        {
            uint8_t pinNo = BUTTON_PIN[buttonIdx]->getPinNo();

            if (IoPin::NC != pinNo)
            {
                gpio_num_t  gpioPinNum  = static_cast<gpio_num_t>(pinNo);

                /* Important: Buttons must be low active! */

                if (ESP_OK != gpio_wakeup_enable(gpioPinNum, GPIO_INTR_LOW_LEVEL))
                {
                    LOG_ERROR("Button %u can not be used as wakeup source.", buttonIdx);
                }
            }

            ++buttonIdx;
        }

        (void)esp_sleep_enable_gpio_wakeup();
    }

    return allButtonsReleased;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void ButtonDrv::setState(ButtonId buttonId, ButtonState state)
{
    if (BUTTON_ID_CNT > buttonId)
    {
        if (pdTRUE == xSemaphoreTake(m_xSemaphore, portMAX_DELAY))
        {
            m_state[buttonId] = state;

            (void)xSemaphoreGive(m_xSemaphore);
        }
    }
}

void ButtonDrv::buttonTask(void *parameters)
{
    ButtonDrv*  buttonDrv   = static_cast<ButtonDrv*>(parameters);
    uint8_t     buttonIdx   = 0U;

    /* The ISR shall notify about on change to determine whether the
     * pin state is stable or not.
     */
    while(BUTTON_ID_CNT > buttonIdx)
    {
        if (IoPin::NC != BUTTON_PIN[buttonIdx]->getPinNo())
        {
            attachInterruptArg( BUTTON_PIN[buttonIdx]->getPinNo(),
                                isrButton,
                                &gButtonId[buttonIdx],
                                CHANGE);
            
            /* Start the debouncing to get a stable initial button state. */
            buttonDrv->m_timer[buttonIdx].start(DEBOUNCING_TIME);
        }

        ++buttonIdx;
    }

    LOG_DEBUG("ButtonDrv task is ready.");

    buttonDrv->buttonTaskMainLoop();

    vTaskDelete(nullptr);
}

void ButtonDrv::buttonTaskMainLoop()
{
    /* The main loop scans several times during one debounce period
     * for any pin change. If there is no change, the state is
     * considered as stable.
     */
    for(;;)
    {
        ButtonId    buttonId    = BUTTON_ID_CNT;
        uint8_t     buttonIdx   = 0U;

        /* Wait 25% of debouncing time whether any button level changed. */
        if (pdTRUE == xQueueReceive(gxQueue, &buttonId, (DEBOUNCING_TIME / 4U) * portTICK_PERIOD_MS))
        {
            if (BUTTON_ID_CNT > buttonId)
            {
                m_timer[buttonId].start(DEBOUNCING_TIME);
            }
        }

        /* Debounce buttons */
        while(BUTTON_ID_CNT > buttonIdx)
        {
            if ((true == m_timer[buttonIdx].isTimerRunning()) &&
                (true == m_timer[buttonIdx].isTimeout()))
            {
                ButtonState buttonState = BUTTON_STATE_UNKNOWN;
                uint8_t     buttonValue = HIGH;

                switch(buttonIdx)
                {
                case BUTTON_ID_OK:
                    buttonValue = Board::buttonOkIn.read();
                    break;

                case BUTTON_ID_LEFT:
                    buttonValue = Board::buttonLeftIn.read();
                    break;

                case BUTTON_ID_RIGHT:
                    buttonValue = Board::buttonRightIn.read();
                    break;

                default:
                    break;
                }

                if (LOW == buttonValue)
                {
                    buttonState = BUTTON_STATE_PRESSED;
                }
                else
                {
                    buttonState = BUTTON_STATE_RELEASED;
                }

                if (BUTTON_STATE_NC != m_state[buttonIdx])
                {
                    if (m_state[buttonIdx] != buttonState)
                    {
                        buttonId = static_cast<ButtonId>(buttonIdx);

                        setState(buttonId, buttonState);

                        /* Notify observer */
                        if (nullptr != m_observer)
                        {
                            m_observer->notify(buttonId, m_state[buttonIdx]);
                        }
                    }
                }

                m_timer[buttonIdx].stop();
            }

            ++buttonIdx;
        }
    }
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
    ButtonId        buttonId                    = *static_cast<ButtonId*>(arg);
    BaseType_t      xHigherPriorityTaskWoken    = pdFALSE;

    (void)xQueueSendFromISR(gxQueue, &buttonId, &xHigherPriorityTaskWoken );

    /* If xHigherPriorityTaskWoken is now set to pdTRUE then a context switch
     * should be performed to ensure the interrupt returns directly to the highest
     * priority task. The macro used for this purpose is dependent on the port in
     * use and may be called portEND_SWITCHING_ISR().
     */
    if (pdTRUE == xHigherPriorityTaskWoken)
    {
        portYIELD_FROM_ISR();
    }
}
