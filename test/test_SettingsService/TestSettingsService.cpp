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
 * @file   TestSettingsService.cpp
 * @brief  Test the settings service.
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <unity.h>
#include <string.h>
#include <SettingsService.h>
#include <Util.h>

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

static void testServiceLifecycle();
static void testOpenClose();
static void testSettingsList();
static void testGetSettingByKey();
static void testRegisterSetting();
static void testDefaultValues();
static void testStringValue();
static void testNumberValues();
static void testBoolValue();
static void testUniqueId();
static void testClear();

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

    RUN_TEST(testServiceLifecycle);
    RUN_TEST(testOpenClose);
    RUN_TEST(testSettingsList);
    RUN_TEST(testGetSettingByKey);
    RUN_TEST(testRegisterSetting);
    RUN_TEST(testDefaultValues);
    RUN_TEST(testStringValue);
    RUN_TEST(testNumberValues);
    RUN_TEST(testBoolValue);
    RUN_TEST(testUniqueId);
    RUN_TEST(testClear);

    return UNITY_END();
}

/**
 * Setup a test. This function will be called before every test by unity.
 */
extern void setUp(void)
{
    SettingsService& settings = SettingsService::getInstance();

    /* The persistent storage is shared by all tests, therefore every test
     * starts with the factory defaults.
     */
    if (true == settings.open(false))
    {
        (void)settings.clear();
        settings.close();
    }
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
 * Test the service lifecycle.
 */
static void testServiceLifecycle()
{
    SettingsService& settings = SettingsService::getInstance();

    /* The instance is a singleton. */
    TEST_ASSERT_EQUAL_PTR(&settings, &SettingsService::getInstance());

    TEST_ASSERT_TRUE(settings.start());
    settings.process();
    settings.stop();

    /* The clean up removes obsolete keys from the persistent storage. On the
     * host there is no non-volatile storage to iterate, therefore it shall just
     * not cause any trouble.
     */
    if (true == settings.open(false))
    {
        settings.cleanUp();
        settings.close();
    }
}

/**
 * Test opening and closing the persistent storage.
 */
static void testOpenClose()
{
    SettingsService& settings = SettingsService::getInstance();

    /* Open read-only. If the storage doesn't exist yet, it will be created. */
    TEST_ASSERT_TRUE(settings.open(true));

    /* It can not be opened twice. */
    TEST_ASSERT_FALSE(settings.open(true));

    settings.close();

    /* Open for writing. */
    TEST_ASSERT_TRUE(settings.open(false));
    settings.close();

    /* Closing a already closed storage shall not cause any trouble. */
    settings.close();
}

/**
 * Test the list of settings.
 */
static void testSettingsList()
{
    SettingsService& settings = SettingsService::getInstance();
    size_t           count    = 0U;
    KeyValue**       list     = settings.getList(count);
    size_t           idx      = 0U;

    TEST_ASSERT_NOT_NULL(list);
    TEST_ASSERT_TRUE(0U < count);

    for (idx = 0U; idx < count; ++idx)
    {
        KeyValue* keyValue = list[idx];
        size_t    idx2     = 0U;

        TEST_ASSERT_NOT_NULL(keyValue);
        TEST_ASSERT_NOT_NULL(keyValue->getKey());
        TEST_ASSERT_NOT_NULL(keyValue->getName());
        TEST_ASSERT_TRUE(0U < strlen(keyValue->getKey()));
        TEST_ASSERT_TRUE(0U < strlen(keyValue->getName()));
        TEST_ASSERT_NOT_EQUAL(KeyValue::TYPE_UNKNOWN, keyValue->getValueType());

        /* Every setting shall be found by its own key. */
        TEST_ASSERT_EQUAL_PTR(keyValue, settings.getSettingByKey(keyValue->getKey()));

        /* The keys must be unique, otherwise a setting is not addressable. */
        for (idx2 = idx + 1U; idx2 < count; ++idx2)
        {
            TEST_ASSERT_TRUE(0 != strcmp(keyValue->getKey(), list[idx2]->getKey()));
        }
    }
}

/**
 * Test to get a setting by its key.
 */
static void testGetSettingByKey()
{
    SettingsService& settings = SettingsService::getInstance();

    /* A known key provides the corresponding setting. */
    TEST_ASSERT_EQUAL_PTR(&settings.getWifiSSID(), settings.getSettingByKey(settings.getWifiSSID().getKey()));
    TEST_ASSERT_EQUAL_PTR(&settings.getBrightness(), settings.getSettingByKey(settings.getBrightness().getKey()));
    TEST_ASSERT_EQUAL_PTR(&settings.getQuietMode(), settings.getSettingByKey(settings.getQuietMode().getKey()));

    /* A unknown key provides nothing. */
    TEST_ASSERT_NULL(settings.getSettingByKey("thisKeyDoesNotExist"));
}

/**
 * Test to register and unregister a setting.
 */
static void testRegisterSetting()
{
    SettingsService& settings = SettingsService::getInstance();
    KeyValueString   testSetting("testKey", "Test setting", "testDefault", 0U, 32U);
    size_t           countBefore = 0U;
    size_t           countAfter  = 0U;

    (void)settings.getList(countBefore);

    /* A invalid setting is rejected. */
    TEST_ASSERT_FALSE(settings.registerSetting(nullptr));

    /* Register it once. */
    TEST_ASSERT_TRUE(settings.registerSetting(&testSetting));
    TEST_ASSERT_FALSE(settings.registerSetting(&testSetting));

    (void)settings.getList(countAfter);
    TEST_ASSERT_EQUAL_size_t(countBefore + 1U, countAfter);
    TEST_ASSERT_EQUAL_PTR(&testSetting, settings.getSettingByKey("testKey"));

    /* The registration provides the persistent storage, therefore the value is
     * usable now.
     */
    if (true == settings.open(false))
    {
        testSetting.setValue("testValue");
        TEST_ASSERT_EQUAL_STRING("testValue", testSetting.getValue().c_str());
        settings.close();
    }

    /* Unregister it. Note, this is mandatory here, because the setting is
     * destroyed at the end of this function, but the service is a singleton
     * which lives longer.
     */
    settings.unregisterSetting(&testSetting);

    (void)settings.getList(countAfter);
    TEST_ASSERT_EQUAL_size_t(countBefore, countAfter);
    TEST_ASSERT_NULL(settings.getSettingByKey("testKey"));

    /* Unregistering a not registered setting shall not cause any trouble. */
    settings.unregisterSetting(&testSetting);
    settings.unregisterSetting(nullptr);
}

/**
 * Test that the default values are provided, as long as nothing is stored.
 */
static void testDefaultValues()
{
    SettingsService& settings = SettingsService::getInstance();

    TEST_ASSERT_TRUE(settings.open(true));

    TEST_ASSERT_EQUAL_STRING(settings.getHostname().getDefault().c_str(), settings.getHostname().getValue().c_str());
    TEST_ASSERT_EQUAL_UINT8(settings.getBrightness().getDefault(), settings.getBrightness().getValue());
    TEST_ASSERT_EQUAL_UINT32(settings.getScrollPause().getDefault(), settings.getScrollPause().getValue());
    TEST_ASSERT_EQUAL_INT32(settings.getLinearGradientOffset().getDefault(), settings.getLinearGradientOffset().getValue());
    TEST_ASSERT_EQUAL(settings.getQuietMode().getDefault(), settings.getQuietMode().getValue());

    settings.close();
}

/**
 * Test a string setting.
 */
static void testStringValue()
{
    SettingsService& settings = SettingsService::getInstance();
    KeyValueString&  hostname = settings.getHostname();

    /* Check the meta data. */
    TEST_ASSERT_EQUAL(KeyValue::TYPE_STRING, hostname.getValueType());
    TEST_ASSERT_TRUE(hostname.getMinLength() <= hostname.getMaxLength());
    TEST_ASSERT_FALSE(hostname.isSecret());

    /* A passphrase is a secret and shall never be shown to the user. */
    TEST_ASSERT_TRUE(settings.getWifiPassphrase().isSecret());

    /* Without any open persistent storage the value can not be changed. */
    hostname.setValue("notStored");
    TEST_ASSERT_EQUAL_STRING(hostname.getDefault().c_str(), hostname.getValue().c_str());

    /* Store and read it back. */
    TEST_ASSERT_TRUE(settings.open(false));
    hostname.setValue("pixelix-test");
    TEST_ASSERT_EQUAL_STRING("pixelix-test", hostname.getValue().c_str());
    settings.close();

    /* The value is persistent, therefore its still available after re-opening. */
    TEST_ASSERT_TRUE(settings.open(true));
    TEST_ASSERT_EQUAL_STRING("pixelix-test", hostname.getValue().c_str());
    settings.close();
}

/**
 * Test the number settings.
 */
static void testNumberValues()
{
    SettingsService& settings       = SettingsService::getInstance();
    KeyValueUInt8&   brightness     = settings.getBrightness();
    KeyValueUInt32&  scrollPause    = settings.getScrollPause();
    KeyValueInt32&   gradientOffset = settings.getLinearGradientOffset();

    /* Check the meta data. */
    TEST_ASSERT_EQUAL(KeyValue::TYPE_UINT8, brightness.getValueType());
    TEST_ASSERT_EQUAL(KeyValue::TYPE_UINT32, scrollPause.getValueType());
    TEST_ASSERT_EQUAL(KeyValue::TYPE_INT32, gradientOffset.getValueType());

    TEST_ASSERT_TRUE(brightness.getMin() <= brightness.getMax());
    TEST_ASSERT_TRUE(scrollPause.getMin() <= scrollPause.getMax());
    TEST_ASSERT_TRUE(gradientOffset.getMin() <= gradientOffset.getMax());

    /* The default value shall be in the valid range. */
    TEST_ASSERT_TRUE(brightness.getMin() <= brightness.getDefault());
    TEST_ASSERT_TRUE(brightness.getDefault() <= brightness.getMax());

    TEST_ASSERT_TRUE(settings.open(false));

    brightness.setValue(brightness.getMax());
    TEST_ASSERT_EQUAL_UINT8(brightness.getMax(), brightness.getValue());

    scrollPause.setValue(scrollPause.getMax());
    TEST_ASSERT_EQUAL_UINT32(scrollPause.getMax(), scrollPause.getValue());

    gradientOffset.setValue(gradientOffset.getMin());
    TEST_ASSERT_EQUAL_INT32(gradientOffset.getMin(), gradientOffset.getValue());

    settings.close();
}

/**
 * Test a boolean setting.
 */
static void testBoolValue()
{
    SettingsService& settings  = SettingsService::getInstance();
    KeyValueBool&    quietMode = settings.getQuietMode();

    TEST_ASSERT_EQUAL(KeyValue::TYPE_BOOL, quietMode.getValueType());

    TEST_ASSERT_TRUE(settings.open(false));

    quietMode.setValue(true);
    TEST_ASSERT_TRUE(quietMode.getValue());

    quietMode.setValue(false);
    TEST_ASSERT_FALSE(quietMode.getValue());

    settings.close();
}

/**
 * Test the unique id, which makes a default value unique per device.
 */
static void testUniqueId()
{
    SettingsService& settings = SettingsService::getInstance();
    KeyValueString&  apSSID   = settings.getWifiApSSID();
    String           defaultWithoutId;

    apSSID.setUniqueId("");
    defaultWithoutId = apSSID.getDefault();

    apSSID.setUniqueId("1234");
    TEST_ASSERT_EQUAL_STRING((defaultWithoutId + "1234").c_str(), apSSID.getDefault().c_str());

    /* Without a stored value the unique default is provided. */
    TEST_ASSERT_TRUE(settings.open(true));
    TEST_ASSERT_EQUAL_STRING(apSSID.getDefault().c_str(), apSSID.getValue().c_str());
    settings.close();

    /* Restore, because the service is a singleton and shared with other tests. */
    apSSID.setUniqueId("");
    TEST_ASSERT_EQUAL_STRING(defaultWithoutId.c_str(), apSSID.getDefault().c_str());
}

/**
 * Test to clear all settings.
 */
static void testClear()
{
    SettingsService& settings   = SettingsService::getInstance();
    KeyValueString&  hostname   = settings.getHostname();
    KeyValueUInt8&   brightness = settings.getBrightness();

    /* Change some settings. */
    TEST_ASSERT_TRUE(settings.open(false));
    hostname.setValue("willBeCleared");
    brightness.setValue(brightness.getMax());
    TEST_ASSERT_EQUAL_STRING("willBeCleared", hostname.getValue().c_str());
    settings.close();

    /* Clearing requires write access. */
    TEST_ASSERT_TRUE(settings.open(false));
    TEST_ASSERT_TRUE(settings.clear());
    settings.close();

    /* Every setting is back to its factory default. */
    TEST_ASSERT_TRUE(settings.open(true));
    TEST_ASSERT_EQUAL_STRING(hostname.getDefault().c_str(), hostname.getValue().c_str());
    TEST_ASSERT_EQUAL_UINT8(brightness.getDefault(), brightness.getValue());
    settings.close();
}
