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
 * @file   TestOsNative.cpp
 * @brief  Test the operating system abstraction for the native environment.
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <unity.h>
#include <Mutex.hpp>
#include <CriticalSection.hpp>
#include <Queue.hpp>
#include <Task.hpp>
#include <Util.h>
#include <thread>
#include <chrono>

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and classes
 *****************************************************************************/

/**
 * A task which counts up the given counter by the overridden process method.
 */
class CounterTask : public Task<uint32_t>
{
public:

    /**
     * Constructs the counter task.
     */
    CounterTask() :
        Task<uint32_t>("counterTask")
    {
    }

    /**
     * Destroys the counter task.
     */
    ~CounterTask()
    {
    }

protected:

    /**
     * Count the given counter up.
     *
     * @param[in] parameters    The counter.
     */
    void process(uint32_t* parameters) final
    {
        if (nullptr != parameters)
        {
            ++(*parameters);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1U));
    }
};

/******************************************************************************
 * Prototypes
 *****************************************************************************/

static void testOsTypes();
static void testMutex();
static void testMutexRecursive();
static void testMutexGuard();
static void testCriticalSection();
static void testQueue();
static void testQueueBlocking();
static void testTaskFunction();
static void testTaskProcess();

/******************************************************************************
 * Local Variables
 *****************************************************************************/

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
 * Main entry point
 *
 * @param[in] argc  Number of command line arguments
 * @param[in] argv  Command line arguments
 */
extern int main(int argc, char** argv)
{
    UTIL_NOT_USED(argc);
    UTIL_NOT_USED(argv);

    UNITY_BEGIN();

    RUN_TEST(testOsTypes);
    RUN_TEST(testMutex);
    RUN_TEST(testMutexRecursive);
    RUN_TEST(testMutexGuard);
    RUN_TEST(testCriticalSection);
    RUN_TEST(testQueue);
    RUN_TEST(testQueueBlocking);
    RUN_TEST(testTaskFunction);
    RUN_TEST(testTaskProcess);

    return UNITY_END();
}

/**
 * Setup a test. This function will be called before every test by unity.
 */
extern void setUp(void)
{
    /* Not used. */
}

/**
 * Clean up test. This function will be called after every test by unity.
 */
extern void tearDown(void)
{
    /* Not used. */
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/

/**
 * Test the freeRTOS compatible types.
 */
static void testOsTypes()
{
    /* One tick is one millisecond. */
    TEST_ASSERT_EQUAL_UINT32(0U, pdMS_TO_TICKS(0U));
    TEST_ASSERT_EQUAL_UINT32(100U, pdMS_TO_TICKS(100U));
    TEST_ASSERT_EQUAL_UINT32(100U, pdTICKS_TO_MS(100U));

    /* The conversion must not overflow. */
    TEST_ASSERT_EQUAL_UINT32(3600000U, pdMS_TO_TICKS(3600000U));

    TEST_ASSERT_EQUAL_INT32(1, pdTRUE);
    TEST_ASSERT_EQUAL_INT32(0, pdFALSE);
    TEST_ASSERT_TRUE(PRO_CPU_NUM != APP_CPU_NUM);
}

/**
 * Test the mutex.
 */
static void testMutex()
{
    Mutex testMutex;

    /* A not created mutex can not be used. */
    TEST_ASSERT_FALSE(testMutex.isAllocated());
    TEST_ASSERT_FALSE(testMutex.take(portMAX_DELAY));
    TEST_ASSERT_FALSE(testMutex.give());

    /* Create it once. */
    TEST_ASSERT_TRUE(testMutex.create());
    TEST_ASSERT_TRUE(testMutex.isAllocated());
    TEST_ASSERT_FALSE(testMutex.create());

    /* A mutex which is not taken, can not be given. */
    TEST_ASSERT_FALSE(testMutex.give());

    /* Take and give it. */
    TEST_ASSERT_TRUE(testMutex.take(portMAX_DELAY));
    TEST_ASSERT_TRUE(testMutex.give());
    TEST_ASSERT_FALSE(testMutex.give());

    /* Another thread must not be able to take it. */
    TEST_ASSERT_TRUE(testMutex.take(portMAX_DELAY));

    bool        isTakenByOther = true;
    std::thread otherThread([&testMutex, &isTakenByOther]() {
        isTakenByOther = testMutex.take(pdMS_TO_TICKS(10U));
    });

    otherThread.join();
    TEST_ASSERT_FALSE(isTakenByOther);

    /* Another thread must not be able to give it. */
    bool        isGivenByOther = true;
    std::thread otherThread2([&testMutex, &isGivenByOther]() {
        isGivenByOther = testMutex.give();
    });

    otherThread2.join();
    TEST_ASSERT_FALSE(isGivenByOther);

    /* After giving it back, another thread must be able to take it. */
    TEST_ASSERT_TRUE(testMutex.give());

    std::thread otherThread3([&testMutex, &isTakenByOther]() {
        isTakenByOther = testMutex.take(pdMS_TO_TICKS(10U));

        if (true == isTakenByOther)
        {
            (void)testMutex.give();
        }
    });

    otherThread3.join();
    TEST_ASSERT_TRUE(isTakenByOther);

    /* Destroy it. */
    testMutex.destroy();
    TEST_ASSERT_FALSE(testMutex.isAllocated());
}

/**
 * Test the recursive mutex.
 */
static void testMutexRecursive()
{
    MutexRecursive testMutex;

    TEST_ASSERT_TRUE(testMutex.create());

    /* The owner is able to take it several times. */
    TEST_ASSERT_TRUE(testMutex.take(portMAX_DELAY));
    TEST_ASSERT_TRUE(testMutex.take(portMAX_DELAY));
    TEST_ASSERT_TRUE(testMutex.take(pdMS_TO_TICKS(10U)));

    /* As long as it is not given back as often as taken, another thread must
     * not be able to take it.
     */
    bool        isTakenByOther = true;
    std::thread otherThread([&testMutex, &isTakenByOther]() {
        isTakenByOther = testMutex.take(pdMS_TO_TICKS(10U));
    });

    otherThread.join();
    TEST_ASSERT_FALSE(isTakenByOther);

    TEST_ASSERT_TRUE(testMutex.give());
    TEST_ASSERT_TRUE(testMutex.give());
    TEST_ASSERT_TRUE(testMutex.give());
    TEST_ASSERT_FALSE(testMutex.give());

    /* Now another thread must be able to take it. */
    std::thread otherThread2([&testMutex, &isTakenByOther]() {
        isTakenByOther = testMutex.take(pdMS_TO_TICKS(10U));

        if (true == isTakenByOther)
        {
            (void)testMutex.give();
        }
    });

    otherThread2.join();
    TEST_ASSERT_TRUE(isTakenByOther);

    testMutex.destroy();
}

/**
 * Test the mutex guard.
 */
static void testMutexGuard()
{
    Mutex testMutex;

    TEST_ASSERT_TRUE(testMutex.create());

    {
        MutexGuard<Mutex> guard(testMutex);

        /* The mutex is taken by the guard, therefore another thread must not be
         * able to take it.
         */
        bool        isTakenByOther = true;
        std::thread otherThread([&testMutex, &isTakenByOther]() {
            isTakenByOther = testMutex.take(pdMS_TO_TICKS(10U));
        });

        otherThread.join();
        TEST_ASSERT_FALSE(isTakenByOther);
    }

    /* The guard gave the mutex back. */
    TEST_ASSERT_TRUE(testMutex.take(pdMS_TO_TICKS(10U)));
    TEST_ASSERT_TRUE(testMutex.give());

    testMutex.destroy();
}

/**
 * Test the critical section.
 */
static void testCriticalSection()
{
    CriticalSection testCritSec;
    uint32_t        counter    = 0U;
    const uint32_t  MAX_LOOPS  = 10000U;
    const uint32_t  MAX_THREAD = 4U;
    std::thread     threads[MAX_THREAD];
    uint32_t        idx = 0U;

    /* The critical section is nestable. */
    testCritSec.enter();
    testCritSec.enter();
    testCritSec.exit();
    testCritSec.exit();

    /* The counter must be protected against concurrent access. */
    for (idx = 0U; idx < MAX_THREAD; ++idx)
    {
        threads[idx] = std::thread([&testCritSec, &counter, MAX_LOOPS]() {
            uint32_t loop = 0U;

            for (loop = 0U; loop < MAX_LOOPS; ++loop)
            {
                CriticalSectionGuard guard(testCritSec);

                ++counter;
            }
        });
    }

    for (idx = 0U; idx < MAX_THREAD; ++idx)
    {
        threads[idx].join();
    }

    TEST_ASSERT_EQUAL_UINT32(MAX_THREAD * MAX_LOOPS, counter);
}

/**
 * Test the queue.
 */
static void testQueue()
{
    Queue<uint32_t> testQueue;
    uint32_t        item  = 0U;
    uint32_t        value = 0U;

    /* A not created queue can not be used. */
    TEST_ASSERT_FALSE(testQueue.sendToBack(item, 0U));
    TEST_ASSERT_FALSE(testQueue.receive(&item, 0U));
    TEST_ASSERT_FALSE(testQueue.peek(&item, 0U));

    /* Create it once, a queue without any item is not supported. */
    TEST_ASSERT_FALSE(testQueue.create(0U));
    TEST_ASSERT_TRUE(testQueue.create(3U));
    TEST_ASSERT_FALSE(testQueue.create(3U));

    /* Nothing to receive yet. */
    TEST_ASSERT_FALSE(testQueue.receive(&item, 0U));
    TEST_ASSERT_FALSE(testQueue.peek(&item, 0U));

    /* Invalid item buffer. */
    TEST_ASSERT_FALSE(testQueue.receive(nullptr, 0U));
    TEST_ASSERT_FALSE(testQueue.peek(nullptr, 0U));

    /* Fill the queue up. */
    value = 1U;
    TEST_ASSERT_TRUE(testQueue.sendToBack(value, 0U));
    value = 2U;
    TEST_ASSERT_TRUE(testQueue.sendToBack(value, 0U));
    value = 0U;
    TEST_ASSERT_TRUE(testQueue.sendToFront(value, 0U));

    /* The queue is full now. */
    value = 3U;
    TEST_ASSERT_FALSE(testQueue.sendToBack(value, pdMS_TO_TICKS(10U)));

    /* Peek must not remove the item. */
    item = 0xFFFFFFFFU;
    TEST_ASSERT_TRUE(testQueue.peek(&item, 0U));
    TEST_ASSERT_EQUAL_UINT32(0U, item);
    TEST_ASSERT_TRUE(testQueue.peek(&item, 0U));
    TEST_ASSERT_EQUAL_UINT32(0U, item);

    /* Receive in the expected order. */
    TEST_ASSERT_TRUE(testQueue.receive(&item, 0U));
    TEST_ASSERT_EQUAL_UINT32(0U, item);
    TEST_ASSERT_TRUE(testQueue.receive(&item, 0U));
    TEST_ASSERT_EQUAL_UINT32(1U, item);
    TEST_ASSERT_TRUE(testQueue.receive(&item, 0U));
    TEST_ASSERT_EQUAL_UINT32(2U, item);

    /* The queue is empty again. */
    TEST_ASSERT_FALSE(testQueue.receive(&item, 0U));

    testQueue.destroy();

    /* A destroyed queue can not be used, but created again. */
    TEST_ASSERT_FALSE(testQueue.receive(&item, 0U));
    TEST_ASSERT_TRUE(testQueue.create(1U));

    testQueue.destroy();
}

/**
 * Test the blocking behaviour of the queue.
 */
static void testQueueBlocking()
{
    Queue<uint32_t> testQueue;
    uint32_t        item       = 0U;
    bool            isReceived = false;

    TEST_ASSERT_TRUE(testQueue.create(1U));

    /* A blocked receiver must be released by a sender. */
    std::thread receiverThread([&testQueue, &item, &isReceived]() {
        isReceived = testQueue.receive(&item, portMAX_DELAY);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(20U));

    uint32_t value = 42U;
    TEST_ASSERT_TRUE(testQueue.sendToBack(value, portMAX_DELAY));

    receiverThread.join();
    TEST_ASSERT_TRUE(isReceived);
    TEST_ASSERT_EQUAL_UINT32(42U, item);

    /* A blocked sender must be released by a receiver. */
    bool isSent = false;

    TEST_ASSERT_TRUE(testQueue.sendToBack(value, 0U));

    std::thread senderThread([&testQueue, &isSent]() {
        uint32_t nextValue = 43U;

        isSent             = testQueue.sendToBack(nextValue, portMAX_DELAY);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(20U));

    TEST_ASSERT_TRUE(testQueue.receive(&item, portMAX_DELAY));
    TEST_ASSERT_EQUAL_UINT32(42U, item);

    senderThread.join();
    TEST_ASSERT_TRUE(isSent);

    TEST_ASSERT_TRUE(testQueue.receive(&item, portMAX_DELAY));
    TEST_ASSERT_EQUAL_UINT32(43U, item);

    /* A blocked receiver must be released by destroying the queue. */
    isReceived = true;

    std::thread receiverThread2([&testQueue, &item, &isReceived]() {
        isReceived = testQueue.receive(&item, portMAX_DELAY);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(20U));

    testQueue.destroy();

    receiverThread2.join();
    TEST_ASSERT_FALSE(isReceived);
}

/**
 * Test a task with a custom task function.
 */
static void testTaskFunction()
{
    uint32_t       counter = 0U;
    Task<uint32_t> testTask("testTask", [](uint32_t* parameters) {
        if (nullptr != parameters)
        {
            ++(*parameters);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1U));
    });

    TEST_ASSERT_FALSE(testTask.isRunning());

    /* Stopping a not running task will fail. */
    TEST_ASSERT_FALSE(testTask.stop());

    TEST_ASSERT_TRUE(testTask.start(&counter));
    TEST_ASSERT_TRUE(testTask.isRunning());

    /* Starting a running task will fail. */
    TEST_ASSERT_FALSE(testTask.start(&counter));

    std::this_thread::sleep_for(std::chrono::milliseconds(50U));

    TEST_ASSERT_TRUE(testTask.stop());
    TEST_ASSERT_FALSE(testTask.isRunning());

    /* The task function was called at least once. */
    TEST_ASSERT_TRUE(0U < counter);

    /* The task doesn't run anymore. */
    const uint32_t counterAfterStop = counter;

    std::this_thread::sleep_for(std::chrono::milliseconds(20U));
    TEST_ASSERT_EQUAL_UINT32(counterAfterStop, counter);

    /* A stopped task can be started again. */
    TEST_ASSERT_TRUE(testTask.start(&counter));
    std::this_thread::sleep_for(std::chrono::milliseconds(20U));
    TEST_ASSERT_TRUE(testTask.stop());
    TEST_ASSERT_TRUE(counterAfterStop < counter);
}

/**
 * Test a task with a overridden process method.
 */
static void testTaskProcess()
{
    uint32_t    counter = 0U;
    CounterTask testTask;

    TEST_ASSERT_FALSE(testTask.isRunning());
    TEST_ASSERT_TRUE(testTask.start(&counter));
    TEST_ASSERT_TRUE(testTask.isRunning());

    std::this_thread::sleep_for(std::chrono::milliseconds(50U));

    TEST_ASSERT_TRUE(testTask.stop());
    TEST_ASSERT_FALSE(testTask.isRunning());
    TEST_ASSERT_TRUE(0U < counter);
}
