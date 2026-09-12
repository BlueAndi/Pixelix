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
 * @file   TestIconTextPlugin.cpp
 * @brief  Test the icon text plugin in the native environment.
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <unity.h>
#include <IconTextPlugin.h>
#include <FileSystem.h>
#include <ColorDef.hpp>
#include <Util.h>

#include "../common/YAGfxTest.hpp"

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

static void testLifecycle();
static void testText();
static void testConfigPersistence();
static void testTopics();
static void testRender();

static bool isCanvasTouched(YAGfxTest& gfx);
static void removeConfiguration(uint16_t uid);

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/** Plugin name, which must exist over the plugin lifetime. */
static const char* PLUGIN_NAME       = "IconTextPlugin";

/** Plugin unique id, used for the lifecycle tests. */
static const uint16_t PLUGIN_UID     = 1U;

/** Plugin unique id, used for the persistence test. */
static const uint16_t PLUGIN_UID_CFG = 2U;

/** The topic which the plugin provides. */
static const char* TOPIC_TEXT        = "iconText";

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

    /* The plugin stores its configuration in the filesystem. Provide the
     * directory the target would have.
     */
    (void)FILESYSTEM.begin(".");
    (void)FILESYSTEM.mkdir(Plugin::CONFIG_PATH);

    UNITY_BEGIN();

    RUN_TEST(testLifecycle);
    RUN_TEST(testText);
    RUN_TEST(testConfigPersistence);
    RUN_TEST(testTopics);
    RUN_TEST(testRender);

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
    /* Every test shall start with a clean persistent configuration. */
    removeConfiguration(PLUGIN_UID);
    removeConfiguration(PLUGIN_UID_CFG);
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/

/**
 * Test the plugin lifecycle.
 */
static void testLifecycle()
{
    YAGfxTest      gfx;
    IconTextPlugin plugin(PLUGIN_NAME, PLUGIN_UID);

    TEST_ASSERT_EQUAL_STRING(PLUGIN_NAME, plugin.getName());
    TEST_ASSERT_EQUAL_UINT16(PLUGIN_UID, plugin.getUID());

    /* A plugin is disabled by default. */
    TEST_ASSERT_FALSE(plugin.isEnabled());

    plugin.start(YAGfxTest::WIDTH, YAGfxTest::HEIGHT);

    /* Enabled, but without any text the plugin shall not be scheduled. */
    plugin.enable();
    TEST_ASSERT_FALSE(plugin.isEnabled());

    plugin.setText("Test", false);
    TEST_ASSERT_TRUE(plugin.isEnabled());

    /* Disabled again, the text doesn't matter anymore. */
    plugin.disable();
    TEST_ASSERT_FALSE(plugin.isEnabled());
    plugin.enable();

    /* Run through a complete slot cycle. */
    plugin.active(gfx);
    plugin.update(gfx);
    plugin.inactive();
    plugin.process(false);

    plugin.stop();
}

/**
 * Test the plugin text.
 */
static void testText()
{
    IconTextPlugin plugin(PLUGIN_NAME, PLUGIN_UID);

    plugin.start(YAGfxTest::WIDTH, YAGfxTest::HEIGHT);

    /* No text after startup. */
    TEST_ASSERT_TRUE(plugin.getText().isEmpty());

    plugin.setText("Hello World", false);
    TEST_ASSERT_EQUAL_STRING("Hello World", plugin.getText().c_str());

    /* The format text is provided unmodified, incl. any format specifier. */
    plugin.setText("{hc}Colored", false);
    TEST_ASSERT_EQUAL_STRING("{hc}Colored", plugin.getText().c_str());

    plugin.setText("", false);
    TEST_ASSERT_TRUE(plugin.getText().isEmpty());

    plugin.stop();
}

/**
 * Test that the text is stored persistent and restored again.
 */
static void testConfigPersistence()
{
    const char* TEXT = "Persistent";

    /* Store the text persistent. The store request is handled in process(). */
    {
        IconTextPlugin plugin(PLUGIN_NAME, PLUGIN_UID_CFG);

        plugin.start(YAGfxTest::WIDTH, YAGfxTest::HEIGHT);
        plugin.setText(TEXT, true);
        plugin.process(false);

        /* Note, stop() would remove the configuration file. */
    }

    TEST_ASSERT_TRUE(FILESYSTEM.exists(Plugin::generateFullPath(PLUGIN_UID_CFG, ".json")));

    /* A new plugin instance with the same uid shall restore the text. */
    {
        IconTextPlugin plugin(PLUGIN_NAME, PLUGIN_UID_CFG);

        plugin.start(YAGfxTest::WIDTH, YAGfxTest::HEIGHT);
        TEST_ASSERT_EQUAL_STRING(TEXT, plugin.getText().c_str());

        /* The plugin removes its configuration during stop(). */
        plugin.stop();
    }

    TEST_ASSERT_FALSE(FILESYSTEM.exists(Plugin::generateFullPath(PLUGIN_UID_CFG, ".json")));

    /* A text which is not stored, shall not be restored. */
    {
        IconTextPlugin plugin(PLUGIN_NAME, PLUGIN_UID_CFG);

        plugin.start(YAGfxTest::WIDTH, YAGfxTest::HEIGHT);
        plugin.setText("Volatile", false);
        plugin.process(false);
    }

    {
        IconTextPlugin plugin(PLUGIN_NAME, PLUGIN_UID_CFG);

        plugin.start(YAGfxTest::WIDTH, YAGfxTest::HEIGHT);
        TEST_ASSERT_TRUE(plugin.getText().isEmpty());
        plugin.stop();
    }
}

/**
 * Test the plugin topic interface.
 */
static void testTopics()
{
    const size_t        JSON_DOC_SIZE = 1024U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
    IconTextPlugin      plugin(PLUGIN_NAME, PLUGIN_UID);

    plugin.start(YAGfxTest::WIDTH, YAGfxTest::HEIGHT);

    /* The plugin provides exactly one topic, incl. Home Assistant support. */
    {
        JsonArray jsonTopics = jsonDoc.to<JsonArray>();

        plugin.getTopics(jsonTopics);

        TEST_ASSERT_EQUAL_size_t(1U, jsonTopics.size());
        TEST_ASSERT_EQUAL_STRING(TOPIC_TEXT, jsonTopics[0]["name"].as<const char*>());
        TEST_ASSERT_FALSE(jsonTopics[0]["extra"]["ha"].isNull());
    }

    /* A unknown topic is rejected. */
    {
        JsonObject jsonValue = jsonDoc.to<JsonObject>();

        TEST_ASSERT_FALSE(plugin.getTopic("unknown", jsonValue));
        TEST_ASSERT_FALSE(plugin.setTopic("unknown", jsonValue));
    }

    /* Set the text via topic. */
    {
        JsonObject jsonValue = jsonDoc.to<JsonObject>();

        jsonValue["text"]    = "Topic text";

        TEST_ASSERT_TRUE(plugin.setTopic(TOPIC_TEXT, jsonValue));
        TEST_ASSERT_EQUAL_STRING("Topic text", plugin.getText().c_str());
    }

    /* The change is reported once. */
    TEST_ASSERT_TRUE(plugin.hasTopicChanged(TOPIC_TEXT));
    TEST_ASSERT_FALSE(plugin.hasTopicChanged(TOPIC_TEXT));

    /* Read the text back via topic. */
    {
        JsonObject jsonValue = jsonDoc.to<JsonObject>();

        TEST_ASSERT_TRUE(plugin.getTopic(TOPIC_TEXT, jsonValue));
        TEST_ASSERT_EQUAL_STRING("Topic text", jsonValue["text"].as<const char*>());
    }

    /* The scroll behaviour is accepted as boolean ... */
    {
        JsonObject jsonValue    = jsonDoc.to<JsonObject>();

        jsonValue["scrollIcon"] = true;

        TEST_ASSERT_TRUE(plugin.setTopic(TOPIC_TEXT, jsonValue));
    }

    {
        JsonObject jsonValue = jsonDoc.to<JsonObject>();

        TEST_ASSERT_TRUE(plugin.getTopic(TOPIC_TEXT, jsonValue));
        TEST_ASSERT_TRUE(jsonValue["scrollIcon"].as<bool>());
    }

    /* ... as well as string, e.g. in case of a form encoded REST request. */
    {
        JsonObject jsonValue    = jsonDoc.to<JsonObject>();

        jsonValue["scrollIcon"] = "false";

        TEST_ASSERT_TRUE(plugin.setTopic(TOPIC_TEXT, jsonValue));
    }

    {
        JsonObject jsonValue = jsonDoc.to<JsonObject>();

        TEST_ASSERT_TRUE(plugin.getTopic(TOPIC_TEXT, jsonValue));
        TEST_ASSERT_FALSE(jsonValue["scrollIcon"].as<bool>());
    }

    plugin.stop();
}

/**
 * Test that the plugin renders something.
 */
static void testRender()
{
    YAGfxTest      gfx;
    IconTextPlugin plugin(PLUGIN_NAME, PLUGIN_UID);

    plugin.start(YAGfxTest::WIDTH, YAGfxTest::HEIGHT);
    plugin.enable();

    /* Without any text nothing shall be shown. */
    gfx.fillScreen(ColorDef::BLACK);
    plugin.active(gfx);
    plugin.update(gfx);
    TEST_ASSERT_FALSE(isCanvasTouched(gfx));

    /* With text the plugin shall draw. Note, the text fades in and the fade
     * effect is driven by the update() calls. Therefore the text is not
     * visible with the very first update.
     */
    plugin.setText("Test", false);

    const uint32_t MAX_UPDATES = 100U;
    uint32_t       updateCount = 0U;
    bool           isTouched   = false;

    while ((MAX_UPDATES > updateCount) &&
           (false == isTouched))
    {
        gfx.fillScreen(ColorDef::BLACK);
        plugin.update(gfx);

        isTouched = isCanvasTouched(gfx);
        ++updateCount;
    }

    TEST_ASSERT_TRUE(isTouched);

    plugin.inactive();
    plugin.stop();
}

/**
 * Is any pixel of the canvas not black anymore?
 *
 * @param[in] gfx   Graphics interface
 *
 * @return If any pixel is drawn, it will return true otherwise false.
 */
static bool isCanvasTouched(YAGfxTest& gfx)
{
    bool     isTouched = false;
    uint16_t x         = 0U;
    uint16_t y         = 0U;

    for (y = 0U; y < YAGfxTest::HEIGHT; ++y)
    {
        for (x = 0U; x < YAGfxTest::WIDTH; ++x)
        {
            if (ColorDef::BLACK != gfx.getColor(x, y))
            {
                isTouched = true;
            }
        }
    }

    return isTouched;
}

/**
 * Remove the persistent configuration of the plugin with the given uid.
 *
 * @param[in] uid   Plugin unique id
 */
static void removeConfiguration(uint16_t uid)
{
    (void)FILESYSTEM.remove(Plugin::generateFullPath(uid, ".json"));
}
