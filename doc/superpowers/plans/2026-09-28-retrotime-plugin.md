# RetroTime Plugin Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Ein Pixelix-Plugin, das auf 64x64-Displays Datum und Uhrzeit zeigt und den Minutenwechsel als Arcade-Spielszene inszeniert, beginnend mit Space Invaders.

**Architecture:** Eine neue, plugin-unabhaengige Bibliothek `lib/RetroGfx` stellt Sprites, Actors mit Bewegungsverhalten, ein Partikelsystem und einen Script-Interpreter bereit. Das Plugin `lib/RetroTimePlugin` liefert Layout, Ziffernfeld und die Animationsdaten. Eine Animation ist damit ein Datensatz aus Sprites, Actors und zwei verzweigungsfreien Scripts, kein Code.

**Tech Stack:** C++14, PlatformIO, ESP32 (HUB75), Unity (Unit-Tests), ArduinoJson, YAGfx.

**Spec:** [doc/superpowers/specs/2026-09-28-retrotime-plugin-design.md](../specs/2026-09-28-retrotime-plugin-design.md)

## Global Constraints

- Haus-Stil ist verbindlich: Yoda-Bedingungen, genau ein `return` je Funktion, Guard-Pfade zuerst, Fixed-Width-Typen mit `U`-Suffix, alle Member in Deklarationsreihenfolge in der Initialisiererliste, Doxygen auf Datei, Klasse, jeder oeffentlichen Methode und jedem Member, Sektions-Banner in fester Reihenfolge. Siehe Skill `embedded-cpp14-misra` und die Vorlagen unter `.claude/skills/embedded-cpp144-misra/assets/`.
- Jede neue Datei beginnt mit dem MIT-Block und `@file`, `@brief`, `@author`. Autor ist `Andreas Merkle <web@blue-andi.de>`. Header tragen `@addtogroup GFX` (RetroGfx) beziehungsweise `@addtogroup PLUGIN` (RetroTimePlugin) mit `@{` / `@}`.
- Keine dynamische Speicherallokation in RetroGfx. Alle Pools sind fest dimensioniert.
- `clang-format` v18.1.3 entscheidet ueber die Formatierung. Nach jeder Aenderung `clang-format -i` auf die geaenderten Dateien, vor dem Commit `clang-format --dry-run -Werror` pruefen.
- `src/Generated/PluginList.cpp`, `src/Generated/Services.cpp` und `src/Generated/TopicHandlers.cpp` werden generiert und nie von Hand bearbeitet.
- Zeit wird immer als Delta in Millisekunden verarbeitet, nie als Frame-Zaehler. Der Update-Task laeuft adaptiv zwischen 20 ms und 200 ms.
- Klassennamen von Plugins enden auf `Plugin`.
- Jede Commit-Nachricht endet mit der Zeile `Co-Authored-By: Claude Opus 5 (1M context) <noreply@anthropic.com>`.
- Displaygroesse zur Compile-Zeit: `CONFIG_LED_MATRIX_WIDTH` / `CONFIG_LED_MATRIX_HEIGHT`. Die Layout-Auswahl laeuft ueber `LAYOUT_TYPE` aus `lib/Views/src/Layouts.h`.

## Review Focus

Diese Faelle ergeben sich aus dem Spec, wuerden aber ohne ausdrueckliche Tests durchrutschen. Jede Zeile hat einen Test in der Aufgabe, die den Code besitzt.

1. **Keine Zeit verfuegbar (`--:--`)** — kein NTP, keine RTC. Erwartet: Anzeige `--:--`, keine Animation, kein Absturz beim Parsen. Test in Aufgabe 6.
2. **Partikel-Pool erschoepft** — vier Ziffern explodieren kurz hintereinander. Erwartet: ueberzaehlige Pixel werden ausgelassen, der Pool laeuft nicht ueber und die aktive Anzahl bleibt `<= MAX_PARTICLES`. Test in Aufgabe 4.
3. **Extreme Zeitschritte** — `deltaMs` von 0 und von 2000 (Task-Verzoegerung). Erwartet: Actor schiesst nicht ueber sein Ziel hinaus, der ScriptRunner haengt nicht und ueberspringt keinen Step. Tests in Aufgabe 3 und Aufgabe 5.
4. **Slot wird mitten in der Kill-Sequenz inaktiv** — Erwartet: Warteschlange wird verworfen, beim naechsten `active()` wird hart synchronisiert und keine veraltete Ziffer animiert. Test in Aufgabe 6.
5. **Unbekannter oder leerer Animationsname in der Konfiguration** — Erwartet: Rueckfall auf die erste registrierte Animation, Log-Warnung, niemals `nullptr`-Zugriff. Test in Aufgabe 9.

---

## Dateistruktur

Vor den Aufgaben der Ueberblick, welche Datei wofuer zustaendig ist. Diese Aufteilung ist die Entkopplungsentscheidung; die Aufgaben folgen ihr.

### Neue Bibliothek `lib/RetroGfx`

| Datei | Verantwortung |
|---|---|
| `library.json` | Metadaten, Abhaengigkeit auf `YAGfx` |
| `src/FixP.h` | Fixkomma-Typ 1/16 Pixel und Umrechnung |
| `src/Sprite.h` / `.cpp` | 1-bpp-Bitmap, Pixelabfrage, Zeichnen mit Farbe und optionaler Spiegelung |
| `src/SpriteAnim.h` / `.cpp` | Frame-Sequenz mit Dauer je Frame |
| `src/Easing.h` | `EASING_LINEAR`, `EASING_OUT`, Wurfparabel-Hilfsfunktion |
| `src/Actor.h` / `.cpp` | Position, Sichtbarkeit, Farbe, Sprite-Animation, Bewegungsverhalten |
| `src/ParticleSystem.h` / `.cpp` | Fester Partikel-Pool, Emitter, Schwerkraft |
| `src/ITargetField.h` | Schnittstelle der Engine auf das Zielfeld, plus `Rect`, `SpawnStyle` |
| `src/Script.h` | `StepType`, `Step`, `AnimDef`, `ActorDef` |
| `src/Stage.h` / `.cpp` | Actors, Partikel, Zielfeld, Register `target`, Anim-Tabelle |
| `src/ScriptRunner.h` / `.cpp` | Fuehrt ein Script gegen eine Stage aus |

`FixP.h` steht so nicht im Spec. Der Spec nennt Fixkomma als Eigenschaft von `Actor` und `ParticleSystem`; da beide es brauchen, bekommt es einen eigenen Header statt doppelt definiert zu werden.

### Neues Plugin `lib/RetroTimePlugin`

| Datei | Verantwortung |
|---|---|
| `library.json`, `pixelix.json` | Metadaten und Web-Dateien je Layout |
| `src/RetroTimePlugin.h` / `.cpp` | Zeitbeschaffung, Konfiguration, Topics, Slot-Lebenszyklus |
| `src/internal/View.h` | Layout-Auswahl ueber `LAYOUT_TYPE` |
| `src/internal/RetroTimeViewGeneric.h` / `.cpp` | Fallback: Datum und Uhrzeit ohne Animation |
| `src/internal/RetroTimeView64x64.h` / `.cpp` | Layout, Stage, Zustandsautomat |
| `src/internal/DigitSprites.h` / `.cpp` | Zehn Ziffern-Sprites, Doppelpunkt |
| `src/internal/DigitField.h` / `.cpp` | Vier Ziffern als `ITargetField`, Zustand je Ziffer |
| `src/internal/TimeSequencer.h` / `.cpp` | Vergleich alt/neu, Entscheidung animieren oder hart setzen, Warteschlange |
| `src/internal/AnimationRegistry.h` / `.cpp` | Tabelle der Animationen, Auswahl nach Name oder zufaellig |
| `src/internal/anim/SpaceInvaders.h` / `.cpp` | Sprites, Actors, Idle- und Kill-Script als Daten |
| `web/RetroTimePlugin.html`, `.jpg` | Web-UI-Fragment und Screenshot |

### Tests

| Datei | Verantwortung |
|---|---|
| `test/test_RetroGfx/TestRetroGfx.cpp` | Sprite, SpriteAnim, Actor, ParticleSystem, ScriptRunner |
| `test/test_RetroTime/TestRetroTime.cpp` | DigitField, TimeSequencer, AnimationRegistry |

### Geaenderte Dateien

| Datei | Aenderung |
|---|---|
| `platformio.ini` | `[env:native-64x64]`, `RetroGfx` in `[env:test].lib_deps` |
| `config/display.ini` | `[display:led_matrix_native_64x64]` |
| `config/configNative64x64.ini` | neue Datei, Feature-Satz der nativen 64x64-Umgebung |
| `config/board.ini` | `[board:native-64x64]` |
| `config/configNormal.ini` | `RetroTimePlugin` aufnehmen |
| `doc/PLUGINS.md` | Abschnitt zum Plugin |

---

## Aufgabe 1: RetroGfx-Bibliothek anlegen und `Sprite`

**Files:**
- Create: `lib/RetroGfx/library.json`
- Create: `lib/RetroGfx/src/FixP.h`
- Create: `lib/RetroGfx/src/Sprite.h`
- Create: `lib/RetroGfx/src/Sprite.cpp`
- Create: `test/test_RetroGfx/TestRetroGfx.cpp`
- Modify: `platformio.ini` (`[env:test]` → `lib_deps`)

**Interfaces:**
- Consumes: `YAGfx`, `Color` aus `lib/YAGfx/src/YAGfx.h`; `YAGfxTest` aus `test/common/YAGfxTest.hpp` (32x8 Zeichenflaeche, `verify(x, y, w, h, color)`).
- Produces:
  - `using FixP = int32_t;`, `FIXP_SHIFT = 4U`, `fixpFromPixel(int16_t)`, `fixpToPixel(FixP)`
  - `class Sprite` mit `Sprite()`, `Sprite(const uint8_t* bitmap, uint8_t width, uint8_t height)`, `getWidth()`, `getHeight()`, `isPixelSet(uint8_t x, uint8_t y) const`, `draw(YAGfx& gfx, int16_t x, int16_t y, const Color& color, bool isFlipped) const`

Bitmap-Format: zeilenweise, je Zeile auf volle Bytes aufgefuellt, most significant bit links. Zeilenschrittweite `(width + 7U) / 8U`.

- [ ] **Step 1: `RetroGfx` in der Test-Umgebung bekannt machen**

In `platformio.ini`, Abschnitt `[env:test]`, die Liste `lib_deps` alphabetisch ergaenzen:

```ini
lib_deps =
    Allocator
    ArduinoNative
    HalNative
    IconTextPlugin
    Os
    RetroGfx
    SettingsService
    StateMachine
    unity
    Utilities
    YAWidgets
    bblanchon/ArduinoJson @ ~6.21.6
```

- [ ] **Step 2: `library.json` anlegen**

```json
{
    "name": "RetroGfx",
    "version": "0.1.0",
    "description": "Sprite, actor, particle and script engine for retro style animations.",
    "authors": [{
        "name": "Andreas Merkle",
        "email": "web@blue-andi.de",
        "url": "https://github.com/BlueAndi",
        "maintainer": true
    }],
    "license": "MIT",
    "dependencies": [{
        "name": "YAGfx"
    }],
    "frameworks": "*",
    "platforms": "*"
}
```

- [ ] **Step 3: `FixP.h` anlegen**

Datei nach der `.h`-Vorlage, Bannerreihenfolge Includes / Compiler Switches / Macros / Types and Classes / Variables / Functions, `@addtogroup GFX`. Inhalt der Typ-Sektion:

```cpp
/** Fixed point type with 1/16 pixel resolution. */
using FixP = int32_t;

/** Number of fractional bits of @ref FixP. */
static const uint8_t FIXP_SHIFT = 4U;

/** Value of one pixel in @ref FixP resolution. */
static const FixP FIXP_ONE = static_cast<FixP>(1) << FIXP_SHIFT;
```

und in der Funktions-Sektion:

```cpp
/**
 * Convert a pixel coordinate to fixed point.
 *
 * @param[in] value Pixel coordinate.
 *
 * @return Fixed point value.
 */
static inline FixP fixpFromPixel(int16_t value)
{
    return static_cast<FixP>(value) << FIXP_SHIFT;
}

/**
 * Convert a fixed point value to a pixel coordinate. The value is truncated
 * towards negative infinity, so that negative coordinates behave like positive
 * ones.
 *
 * @param[in] value Fixed point value.
 *
 * @return Pixel coordinate.
 */
static inline int16_t fixpToPixel(FixP value)
{
    return static_cast<int16_t>(value >> FIXP_SHIFT);
}
```

- [ ] **Step 4: Den fehlschlagenden Test schreiben**

`test/test_RetroGfx/TestRetroGfx.cpp` nach dem Muster von `test/test_SimpleTimer/TestSimpleTimer.cpp` anlegen: MIT-Block, `@file TestRetroGfx.cpp`, `@brief Test the retro graphics engine.`, Sektions-Banner, `main()` mit `UNITY_BEGIN()` / `RUN_TEST(...)` / `UNITY_END()`, leere `setUp()` und `tearDown()`.

```cpp
#include <unity.h>
#include <Sprite.h>
#include <FixP.h>
#include "../common/YAGfxTest.hpp"

/** 4x4 test sprite: a filled ring with a hole in the middle. */
static const uint8_t TEST_SPRITE_DATA[] = {
    0xF0U, /* 1111 */
    0x90U, /* 1001 */
    0x90U, /* 1001 */
    0xF0U  /* 1111 */
};

static void testFixP()
{
    TEST_ASSERT_EQUAL_INT32(16, FIXP_ONE);
    TEST_ASSERT_EQUAL_INT32(32, fixpFromPixel(2));
    TEST_ASSERT_EQUAL_INT16(2, fixpToPixel(fixpFromPixel(2)));
    TEST_ASSERT_EQUAL_INT16(-3, fixpToPixel(fixpFromPixel(-3)));
}

static void testSprite()
{
    Sprite    sprite(TEST_SPRITE_DATA, 4U, 4U);
    YAGfxTest testGfx;
    Color     color = ColorDef::WHITE;

    TEST_ASSERT_EQUAL_UINT8(4U, sprite.getWidth());
    TEST_ASSERT_EQUAL_UINT8(4U, sprite.getHeight());

    /* Corners set, hole in the middle. */
    TEST_ASSERT_TRUE(sprite.isPixelSet(0U, 0U));
    TEST_ASSERT_TRUE(sprite.isPixelSet(3U, 0U));
    TEST_ASSERT_FALSE(sprite.isPixelSet(1U, 1U));
    TEST_ASSERT_FALSE(sprite.isPixelSet(2U, 2U));

    /* Out of range must not read out of the bitmap. */
    TEST_ASSERT_FALSE(sprite.isPixelSet(4U, 0U));
    TEST_ASSERT_FALSE(sprite.isPixelSet(0U, 4U));

    testGfx.fillScreen(ColorDef::BLACK);
    sprite.draw(testGfx, 0, 0, color, false);
    TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(color), static_cast<uint32_t>(testGfx.getColor(0, 0)));
    TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(ColorDef::BLACK), static_cast<uint32_t>(testGfx.getColor(1, 1)));
}

static void testSpriteClipping()
{
    /* Review focus: a sprite may leave the display at every edge. */
    Sprite    sprite(TEST_SPRITE_DATA, 4U, 4U);
    YAGfxTest testGfx;

    testGfx.fillScreen(ColorDef::BLACK);
    sprite.draw(testGfx, -2, -2, ColorDef::WHITE, false);
    sprite.draw(testGfx, static_cast<int16_t>(YAGfxTest::WIDTH - 2U), 0, ColorDef::WHITE, false);
    sprite.draw(testGfx, 0, static_cast<int16_t>(YAGfxTest::HEIGHT - 2U), ColorDef::WHITE, false);

    /* Reaching this point without an assertion inside YAGfxTest::drawPixel is the result. */
    TEST_ASSERT_TRUE(true);
}

static void testSpriteFlipped()
{
    /* Asymmetric sprite: only the left column of the top row is set. */
    static const uint8_t ASYM_DATA[] = { 0x80U, 0x00U, 0x00U, 0x00U };
    Sprite               sprite(ASYM_DATA, 4U, 4U);
    YAGfxTest            testGfx;

    testGfx.fillScreen(ColorDef::BLACK);
    sprite.draw(testGfx, 0, 0, ColorDef::WHITE, true);
    TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(ColorDef::BLACK), static_cast<uint32_t>(testGfx.getColor(0, 0)));
    TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(ColorDef::WHITE), static_cast<uint32_t>(testGfx.getColor(3, 0)));
}
```

`RUN_TEST` fuer `testFixP`, `testSprite`, `testSpriteClipping`, `testSpriteFlipped` in `main()` eintragen.

- [ ] **Step 5: Test laufen lassen und Fehlschlag bestaetigen**

Run: `platformio test --environment test --filter test_RetroGfx`
Expected: FAIL, `Sprite.h: No such file or directory`.

- [ ] **Step 6: `Sprite.h` und `Sprite.cpp` schreiben**

`Sprite.h` nach der `.h`-Vorlage, `@addtogroup GFX`. Klassendeklaration:

```cpp
/**
 * A sprite is a 1 bit per pixel bitmap, stored in flash. It has no position;
 * the caller decides where and in which color it is drawn.
 */
class Sprite
{
public:

    /**
     * Constructs an empty sprite, which draws nothing.
     */
    Sprite();

    /**
     * Constructs a sprite.
     *
     * @param[in] bitmap    Bitmap data, row wise, each row padded to full
     *                      bytes, most significant bit is the left pixel.
     *                      Must exist over the sprite lifetime.
     * @param[in] width     Width in pixel. [1; 64]
     * @param[in] height    Height in pixel. [1; 64]
     */
    Sprite(const uint8_t* bitmap, uint8_t width, uint8_t height);

    /**
     * Get the sprite width.
     *
     * @return Width in pixel.
     */
    uint8_t getWidth() const;

    /**
     * Get the sprite height.
     *
     * @return Height in pixel.
     */
    uint8_t getHeight() const;

    /**
     * Is the pixel at the given sprite local position set?
     * Out of range coordinates return false.
     *
     * @param[in] x Sprite local x coordinate.
     * @param[in] y Sprite local y coordinate.
     *
     * @return If the pixel is set, it will return true otherwise false.
     */
    bool isPixelSet(uint8_t x, uint8_t y) const;

    /**
     * Draw the sprite. Pixels outside the graphic area are skipped.
     *
     * @param[in] gfx       Graphic functionality to draw on.
     * @param[in] x         Display x coordinate of the sprite left edge.
     * @param[in] y         Display y coordinate of the sprite top edge.
     * @param[in] color     Color of every set pixel.
     * @param[in] isFlipped If true, the sprite is mirrored horizontally.
     */
    void draw(YAGfx& gfx, int16_t x, int16_t y, const Color& color, bool isFlipped) const;

private:

    const uint8_t* m_bitmap; /**< Bitmap data in flash or nullptr. */
    uint8_t        m_width;  /**< Width in pixel. */
    uint8_t        m_height; /**< Height in pixel. */
};
```

`Sprite.cpp` nach der `.cpp`-Vorlage. `isPixelSet()` mit Guard-Pfaden und einem Rueckgabewert:

```cpp
bool Sprite::isPixelSet(uint8_t x, uint8_t y) const
{
    bool isSet = false;

    if (nullptr == m_bitmap)
    {
        /* Guard path: empty sprite. */
    }
    else if ((m_width <= x) || (m_height <= y))
    {
        /* Guard path: out of range. */
    }
    else
    {
        const uint8_t  stride    = static_cast<uint8_t>((m_width + 7U) / 8U);
        const uint16_t byteIndex = static_cast<uint16_t>(y) * stride + (x / 8U);
        const uint8_t  bitMask   = static_cast<uint8_t>(0x80U >> (x % 8U));

        if (0U != (m_bitmap[byteIndex] & bitMask))
        {
            isSet = true;
        }
    }

    return isSet;
}
```

`draw()` iteriert ueber alle Sprite-Pixel, berechnet bei `isFlipped` die gespiegelte Spalte `m_width - 1U - sx` und ruft `gfx.drawPixel()` nur fuer gesetzte Pixel innerhalb von `gfx.getWidth()` / `gfx.getHeight()` auf. Die Bereichspruefung liegt bewusst hier und nicht im Aufrufer.

- [ ] **Step 7: Test laufen lassen und Erfolg bestaetigen**

Run: `platformio test --environment test --filter test_RetroGfx`
Expected: PASS, vier Tests.

- [ ] **Step 8: Formatierung pruefen und committen**

```bash
clang-format -i lib/RetroGfx/src/FixP.h lib/RetroGfx/src/Sprite.h lib/RetroGfx/src/Sprite.cpp test/test_RetroGfx/TestRetroGfx.cpp
clang-format --dry-run -Werror lib/RetroGfx/src/FixP.h lib/RetroGfx/src/Sprite.h lib/RetroGfx/src/Sprite.cpp test/test_RetroGfx/TestRetroGfx.cpp
git add lib/RetroGfx test/test_RetroGfx platformio.ini
git commit -m @'
feat(RetroGfx): add library skeleton with fixed point and sprite

Co-Authored-By: Claude Opus 5 (1M context) <noreply@anthropic.com>
'@
```

---

## Aufgabe 2: `SpriteAnim`

**Files:**
- Create: `lib/RetroGfx/src/SpriteAnim.h`, `lib/RetroGfx/src/SpriteAnim.cpp`
- Modify: `test/test_RetroGfx/TestRetroGfx.cpp`

**Interfaces:**
- Consumes: `Sprite` aus Aufgabe 1.
- Produces:
  - `struct SpriteFrame { const Sprite* sprite; uint16_t durationMs; };`
  - `class SpriteAnim` mit `SpriteAnim()`, `set(const SpriteFrame* frames, uint8_t frameCount, bool isLooping)`, `restart()`, `update(uint32_t deltaMs)`, `getSprite() const` (liefert `const Sprite*`, `nullptr` wenn nicht gesetzt), `isFinished() const`

- [ ] **Step 1: Den fehlschlagenden Test schreiben**

An `TestRetroGfx.cpp` anhaengen und `RUN_TEST(testSpriteAnim)` eintragen:

```cpp
static void testSpriteAnim()
{
    static const uint8_t FRAME_A_DATA[] = { 0x80U };
    static const uint8_t FRAME_B_DATA[] = { 0x40U };
    static const Sprite  frameA(FRAME_A_DATA, 2U, 1U);
    static const Sprite  frameB(FRAME_B_DATA, 2U, 1U);

    static const SpriteFrame FRAMES[] = {
        { &frameA, 100U },
        { &frameB, 100U }
    };

    SpriteAnim anim;

    /* Without frames nothing is delivered and nothing crashes. */
    TEST_ASSERT_NULL(anim.getSprite());
    anim.update(1000U);
    TEST_ASSERT_NULL(anim.getSprite());

    anim.set(FRAMES, 2U, true);
    TEST_ASSERT_EQUAL_PTR(&frameA, anim.getSprite());

    anim.update(99U);
    TEST_ASSERT_EQUAL_PTR(&frameA, anim.getSprite());

    anim.update(1U);
    TEST_ASSERT_EQUAL_PTR(&frameB, anim.getSprite());

    /* Looping wraps around. */
    anim.update(100U);
    TEST_ASSERT_EQUAL_PTR(&frameA, anim.getSprite());
    TEST_ASSERT_FALSE(anim.isFinished());

    /* A single large step must not skip the whole animation. */
    anim.restart();
    anim.update(1000U);
    TEST_ASSERT_NOT_NULL(anim.getSprite());

    /* Non looping stops on the last frame and reports finished. */
    anim.set(FRAMES, 2U, false);
    anim.update(250U);
    TEST_ASSERT_EQUAL_PTR(&frameB, anim.getSprite());
    TEST_ASSERT_TRUE(anim.isFinished());
}
```

- [ ] **Step 2: Test laufen lassen und Fehlschlag bestaetigen**

Run: `platformio test --environment test --filter test_RetroGfx`
Expected: FAIL, `SpriteAnim.h: No such file or directory`.

- [ ] **Step 3: `SpriteAnim` implementieren**

Member: `const SpriteFrame* m_frames`, `uint8_t m_frameCount`, `uint8_t m_frameIndex`, `uint32_t m_elapsedMs`, `bool m_isLooping`, `bool m_isFinished`.

`update()` addiert `deltaMs` auf `m_elapsedMs` und schaltet in einer **begrenzten** Schleife weiter, solange die Framedauer erreicht ist. Die Begrenzung ist wesentlich: bei grossem `deltaMs` und kurzen Frames darf die Schleife nicht unbegrenzt laufen.

```cpp
void SpriteAnim::update(uint32_t deltaMs)
{
    if (nullptr == m_frames)
    {
        /* Guard path: no animation set. */
    }
    else if (true == m_isFinished)
    {
        /* Guard path: non looping animation already at its end. */
    }
    else
    {
        uint8_t guard = 0U;

        m_elapsedMs += deltaMs;

        while ((m_elapsedMs >= m_frames[m_frameIndex].durationMs) &&
               (MAX_STEPS_PER_UPDATE > guard))
        {
            m_elapsedMs -= m_frames[m_frameIndex].durationMs;
            ++guard;

            if ((m_frameIndex + 1U) < m_frameCount)
            {
                ++m_frameIndex;
            }
            else if (true == m_isLooping)
            {
                m_frameIndex = 0U;
            }
            else
            {
                m_isFinished = true;
                m_elapsedMs  = 0U;
            }
        }
    }
}
```

mit `static const uint8_t MAX_STEPS_PER_UPDATE = 8U;` als privater Konstante. Eine Framedauer von `0U` wird in `set()` abgewiesen — in dem Fall bleibt die Animation ungesetzt und es wird `LOG_WARNING` ausgegeben.

- [ ] **Step 4: Test laufen lassen und Erfolg bestaetigen**

Run: `platformio test --environment test --filter test_RetroGfx`
Expected: PASS.

- [ ] **Step 5: Formatieren und committen**

```bash
clang-format -i lib/RetroGfx/src/SpriteAnim.h lib/RetroGfx/src/SpriteAnim.cpp test/test_RetroGfx/TestRetroGfx.cpp
git add lib/RetroGfx test/test_RetroGfx
git commit -m @'
feat(RetroGfx): add sprite animation with bounded frame stepping

Co-Authored-By: Claude Opus 5 (1M context) <noreply@anthropic.com>
'@
```

---

## Aufgabe 3: `Actor` mit Bewegungsverhalten

**Files:**
- Create: `lib/RetroGfx/src/Easing.h`, `lib/RetroGfx/src/Actor.h`, `lib/RetroGfx/src/Actor.cpp`
- Modify: `test/test_RetroGfx/TestRetroGfx.cpp`

**Interfaces:**
- Consumes: `FixP.h`, `Sprite`, `SpriteAnim`, `SpriteFrame`.
- Produces:
  - `enum EasingType { EASING_LINEAR = 0, EASING_OUT };`
  - `class Actor` mit `setPosition(int16_t x, int16_t y)`, `getX() const`, `getY() const`, `setVisible(bool)`, `isVisible() const`, `setColor(const Color&)`, `setAnim(const SpriteFrame* frames, uint8_t frameCount, bool isLooping)`, `patrolX(int16_t x0, int16_t x1, uint16_t speed)`, `moveToX(int16_t x, uint16_t speed, EasingType easing)`, `ballisticTo(int16_t x, int16_t y, uint16_t speed, uint8_t arc)`, `fallTo(int16_t groundY, uint16_t speed)`, `stopBehaviour()`, `isBehaviourDone() const`, `update(uint32_t deltaMs)`, `draw(YAGfx& gfx) const`

`speed` ist immer in Pixel pro Sekunde. `arc` ist die Scheitelhoehe der Wurfparabel in Pixel ueber der Verbindungslinie; `0U` bedeutet gerade Bahn. `isBehaviourDone()` liefert bei `BEHAVIOUR_PATROL_X` nie `true` und bei `BEHAVIOUR_NONE` immer `true`.

- [ ] **Step 1: Den fehlschlagenden Test schreiben**

```cpp
static void testActorMoveToX()
{
    Actor actor;

    actor.setPosition(0, 10);
    actor.moveToX(20, 20U, EASING_LINEAR);
    TEST_ASSERT_FALSE(actor.isBehaviourDone());

    /* 20 px at 20 px/s takes 1000 ms. */
    actor.update(500U);
    TEST_ASSERT_EQUAL_INT16(10, actor.getX());
    TEST_ASSERT_FALSE(actor.isBehaviourDone());

    actor.update(500U);
    TEST_ASSERT_EQUAL_INT16(20, actor.getX());
    TEST_ASSERT_TRUE(actor.isBehaviourDone());
}

static void testActorTimeStepIndependence()
{
    /* Review focus: 20 ms and 200 ms ticks must reach the same end state. */
    Actor    fine;
    Actor    coarse;
    uint32_t elapsed = 0U;

    fine.setPosition(0, 0);
    coarse.setPosition(0, 0);
    fine.moveToX(30, 30U, EASING_LINEAR);
    coarse.moveToX(30, 30U, EASING_LINEAR);

    while (1000U > elapsed)
    {
        fine.update(20U);
        elapsed += 20U;
    }

    elapsed = 0U;
    while (1000U > elapsed)
    {
        coarse.update(200U);
        elapsed += 200U;
    }

    TEST_ASSERT_EQUAL_INT16(30, fine.getX());
    TEST_ASSERT_EQUAL_INT16(30, coarse.getX());
    TEST_ASSERT_TRUE(fine.isBehaviourDone());
    TEST_ASSERT_TRUE(coarse.isBehaviourDone());
}

static void testActorExtremeTimeStep()
{
    /* Review focus: deltaMs of 0 and of 2000 must not overshoot. */
    Actor actor;

    actor.setPosition(0, 0);
    actor.moveToX(10, 10U, EASING_LINEAR);

    actor.update(0U);
    TEST_ASSERT_EQUAL_INT16(0, actor.getX());
    TEST_ASSERT_FALSE(actor.isBehaviourDone());

    actor.update(2000U);
    TEST_ASSERT_EQUAL_INT16(10, actor.getX());
    TEST_ASSERT_TRUE(actor.isBehaviourDone());
}

static void testActorPatrol()
{
    Actor actor;

    actor.setPosition(0, 0);
    actor.patrolX(0, 10, 10U);

    /* Patrol never finishes. */
    actor.update(1000U);
    TEST_ASSERT_EQUAL_INT16(10, actor.getX());
    TEST_ASSERT_FALSE(actor.isBehaviourDone());

    /* It reverses at the limit instead of running away. */
    actor.update(500U);
    TEST_ASSERT_EQUAL_INT16(5, actor.getX());
    TEST_ASSERT_FALSE(actor.isBehaviourDone());
}

static void testActorBallistic()
{
    Actor actor;

    actor.setPosition(0, 40);
    actor.ballisticTo(0, 10, 30U, 0U);

    actor.update(500U);
    TEST_ASSERT_EQUAL_INT16(25, actor.getY());
    TEST_ASSERT_FALSE(actor.isBehaviourDone());

    actor.update(500U);
    TEST_ASSERT_EQUAL_INT16(10, actor.getY());
    TEST_ASSERT_TRUE(actor.isBehaviourDone());
}

static void testActorFall()
{
    Actor actor;

    actor.setPosition(5, 10);
    actor.fallTo(30, 50U);

    actor.update(2000U);
    TEST_ASSERT_EQUAL_INT16(30, actor.getY());
    TEST_ASSERT_EQUAL_INT16(5, actor.getX());
    TEST_ASSERT_TRUE(actor.isBehaviourDone());
}
```

`RUN_TEST` fuer alle sechs Funktionen eintragen.

- [ ] **Step 2: Test laufen lassen und Fehlschlag bestaetigen**

Run: `platformio test --environment test --filter test_RetroGfx`
Expected: FAIL, `Actor.h: No such file or directory`.

- [ ] **Step 3: `Easing.h` implementieren**

```cpp
/** Supported easing curves. */
enum EasingType
{
    EASING_LINEAR = 0, /**< Constant speed. */
    EASING_OUT         /**< Fast at the start, slow at the end. */
};

/**
 * Apply an easing curve to a normalized progress value.
 *
 * @param[in] easing    Easing curve.
 * @param[in] progress  Normalized progress. [0; FIXP_ONE]
 *
 * @return Eased progress. [0; FIXP_ONE]
 */
static inline FixP easingApply(EasingType easing, FixP progress)
{
    FixP result = progress;

    if (EASING_OUT == easing)
    {
        /* 1 - (1 - p)^2 */
        const FixP inverse = FIXP_ONE - progress;

        result = FIXP_ONE - ((inverse * inverse) >> FIXP_SHIFT);
    }

    return result;
}
```

- [ ] **Step 4: `Actor` implementieren**

Private Member in Deklarationsreihenfolge: `FixP m_x`, `FixP m_y`, `SpriteAnim m_anim`, `Color m_color`, `bool m_isVisible`, `BehaviourType m_behaviour`, `FixP m_startX`, `FixP m_startY`, `FixP m_targetX`, `FixP m_targetY`, `uint16_t m_speed`, `uint32_t m_durationMs`, `uint32_t m_elapsedMs`, `EasingType m_easing`, `uint8_t m_arc`, `int16_t m_patrolMin`, `int16_t m_patrolMax`, `bool m_isFlipped`, `bool m_isBehaviourDone`.

Das gemeinsame Muster aller zielgerichteten Verhalten: Beim Start werden Startpunkt, Zielpunkt und die Gesamtdauer aus Strecke und Geschwindigkeit berechnet. `update()` erhoeht `m_elapsedMs`, **begrenzt auf `m_durationMs`**, und interpoliert. Dadurch gibt es kein Ueberschiessen, unabhaengig von der Schrittweite.

```cpp
void Actor::update(uint32_t deltaMs)
{
    m_anim.update(deltaMs);

    switch (m_behaviour)
    {
    case BEHAVIOUR_NONE:
        /* Nothing to do. */
        break;

    case BEHAVIOUR_PATROL_X:
        updatePatrol(deltaMs);
        break;

    case BEHAVIOUR_MOVE_TO_X:
    /* fall through */
    case BEHAVIOUR_BALLISTIC:
    /* fall through */
    case BEHAVIOUR_FALL:
        updateInterpolated(deltaMs);
        break;

    default:
        /* Nothing to do. */
        break;
    }
}
```

`updateInterpolated()`:

```cpp
void Actor::updateInterpolated(uint32_t deltaMs)
{
    FixP progress = FIXP_ONE;

    m_elapsedMs += deltaMs;

    if (m_durationMs <= m_elapsedMs)
    {
        m_elapsedMs        = m_durationMs;
        m_isBehaviourDone  = true;
    }
    else
    {
        progress = static_cast<FixP>((static_cast<uint64_t>(m_elapsedMs) * FIXP_ONE) / m_durationMs);
    }

    progress = easingApply(m_easing, progress);

    m_x      = m_startX + (((m_targetX - m_startX) * progress) >> FIXP_SHIFT);
    m_y      = m_startY + (((m_targetY - m_startY) * progress) >> FIXP_SHIFT);

    if (0U != m_arc)
    {
        /* Parabola: 4 * arc * p * (1 - p), subtracted because y grows downwards. */
        const FixP inverse = FIXP_ONE - progress;
        const FixP bow     = (fixpFromPixel(static_cast<int16_t>(4U * m_arc)) * ((progress * inverse) >> FIXP_SHIFT)) >> FIXP_SHIFT;

        m_y -= bow;
    }
}
```

`m_durationMs` von `0U` wird beim Start des Verhaltens auf `1U` angehoben, damit die Division sicher ist; bei Strecke `0` ist das Verhalten sofort abgeschlossen.

`updatePatrol()` bewegt mit `m_speed` in die aktuelle Richtung, dreht an `m_patrolMin` / `m_patrolMax` um, setzt dort `m_isFlipped` um und setzt `m_isBehaviourDone` nie.

`draw()` zeichnet nur bei `true == m_isVisible` und nur wenn `nullptr != m_anim.getSprite()`.

- [ ] **Step 5: Test laufen lassen und Erfolg bestaetigen**

Run: `platformio test --environment test --filter test_RetroGfx`
Expected: PASS, alle bisherigen Tests plus sechs neue.

- [ ] **Step 6: Formatieren und committen**

```bash
clang-format -i lib/RetroGfx/src/Easing.h lib/RetroGfx/src/Actor.h lib/RetroGfx/src/Actor.cpp test/test_RetroGfx/TestRetroGfx.cpp
git add lib/RetroGfx test/test_RetroGfx
git commit -m @'
feat(RetroGfx): add actor with patrol, move, ballistic and fall behaviour

Co-Authored-By: Claude Opus 5 (1M context) <noreply@anthropic.com>
'@
```

---

## Aufgabe 4: `ParticleSystem`

**Files:**
- Create: `lib/RetroGfx/src/ParticleSystem.h`, `lib/RetroGfx/src/ParticleSystem.cpp`
- Modify: `test/test_RetroGfx/TestRetroGfx.cpp`

**Interfaces:**
- Consumes: `FixP.h`, `Sprite`.
- Produces:
  - `enum ParticleStyle { PARTICLE_STYLE_BLAST = 0, PARTICLE_STYLE_BRICK, PARTICLE_STYLE_SPARK };`
  - `class ParticleSystem` mit `static const uint8_t MAX_PARTICLES = 128U;`, `clear()`, `getActiveCount() const`, `explodeSprite(const Sprite& sprite, int16_t x, int16_t y, ParticleStyle style)`, `burst(int16_t x, int16_t y, uint8_t count, ParticleStyle style)`, `update(uint32_t deltaMs)`, `draw(YAGfx& gfx) const`

- [ ] **Step 1: Den fehlschlagenden Test schreiben**

```cpp
static void testParticleSystem()
{
    static const uint8_t BLOCK_DATA[] = { 0xF0U, 0xF0U, 0xF0U, 0xF0U };
    Sprite               block(BLOCK_DATA, 4U, 4U);
    ParticleSystem       particles;

    TEST_ASSERT_EQUAL_UINT8(0U, particles.getActiveCount());

    particles.explodeSprite(block, 10, 10, PARTICLE_STYLE_BLAST);

    /* 16 set pixels result in at most 16 particles. */
    TEST_ASSERT_EQUAL_UINT8(16U, particles.getActiveCount());

    /* Particles expire. */
    particles.update(10000U);
    TEST_ASSERT_EQUAL_UINT8(0U, particles.getActiveCount());

    particles.clear();
    TEST_ASSERT_EQUAL_UINT8(0U, particles.getActiveCount());
}

static void testParticlePoolExhausted()
{
    /* Review focus: four digits exploding in a row must not overflow the pool. */
    static const uint8_t FULL_DATA[8U * 8U / 8U] = {
        0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU
    };
    Sprite         full(FULL_DATA, 8U, 8U);
    ParticleSystem particles;
    uint8_t        round = 0U;

    while (8U > round)
    {
        particles.explodeSprite(full, 0, 0, PARTICLE_STYLE_BLAST);
        ++round;
    }

    TEST_ASSERT_LESS_OR_EQUAL_UINT8(ParticleSystem::MAX_PARTICLES, particles.getActiveCount());

    /* And it still renders without touching anything outside the display. */
    YAGfxTest testGfx;
    particles.draw(testGfx);
    TEST_ASSERT_TRUE(true);
}
```

`RUN_TEST(testParticleSystem)` und `RUN_TEST(testParticlePoolExhausted)` eintragen.

- [ ] **Step 2: Test laufen lassen und Fehlschlag bestaetigen**

Run: `platformio test --environment test --filter test_RetroGfx`
Expected: FAIL, `ParticleSystem.h: No such file or directory`.

- [ ] **Step 3: `ParticleSystem` implementieren**

```cpp
/**
 * A single particle. Position and velocity are in fixed point, velocity is
 * given in fixed point pixel per second.
 */
struct Particle
{
    FixP     x;       /**< Horizontal position. */
    FixP     y;       /**< Vertical position. */
    FixP     vx;      /**< Horizontal velocity in pixel per second. */
    FixP     vy;      /**< Vertical velocity in pixel per second. */
    Color    color;   /**< Particle color. */
    uint16_t ttlMs;   /**< Remaining lifetime in ms. */
    bool     isAlive; /**< Is the slot in use? */
};
```

`explodeSprite()` laeuft ueber alle Sprite-Pixel und legt fuer jedes gesetzte Pixel ein Partikel an. Ist kein freier Slot mehr vorhanden, wird das Pixel **ausgelassen** — das ist die im Spec festgelegte Zusage, nicht ein stiller Ueberlauf:

```cpp
void ParticleSystem::explodeSprite(const Sprite& sprite, int16_t x, int16_t y, ParticleStyle style)
{
    uint8_t sy = 0U;

    while (sprite.getHeight() > sy)
    {
        uint8_t sx = 0U;

        while (sprite.getWidth() > sx)
        {
            if (true == sprite.isPixelSet(sx, sy))
            {
                Particle* particle = allocate();

                if (nullptr == particle)
                {
                    /* Guard path: pool exhausted, skip this pixel. */
                }
                else
                {
                    spawn(*particle,
                          static_cast<int16_t>(x + sx),
                          static_cast<int16_t>(y + sy),
                          style);
                }
            }

            ++sx;
        }

        ++sy;
    }
}
```

`spawn()` setzt Farbe, Lebensdauer und einen Zufallsimpuls gemaess Style. Die Zufallszahlen kommen aus einem eigenen, linearen Kongruenzgenerator als privater Member, damit das Verhalten nicht von `rand()` und damit nicht vom Rest des Systems abhaengt.

Style-Tabelle als `static const` im `.cpp`:

| Style | Farbe | Lebensdauer | Streuung |
|---|---|---|---|
| `PARTICLE_STYLE_BLAST` | Weiss nach Rot ueber die Lebensdauer | 600 ms | +/- 40 px/s horizontal, -60 bis -10 px/s vertikal |
| `PARTICLE_STYLE_BRICK` | Braun | 800 ms | +/- 25 px/s horizontal, -70 bis -30 px/s vertikal |
| `PARTICLE_STYLE_SPARK` | Gelb nach Orange | 250 ms | +/- 60 px/s in beide Richtungen |

`update()` bewegt jedes lebende Partikel, addiert die Schwerkraft (`GRAVITY = 120` Pixel pro Sekunde im Quadrat) auf `vy` und zieht `deltaMs` von `ttlMs` ab; bei Unterlauf wird `isAlive` auf `false` gesetzt. `draw()` zeichnet nur lebende Partikel und ueberlaesst die Bereichspruefung `gfx.drawPixel()` nicht, sondern prueft selbst gegen `gfx.getWidth()` / `gfx.getHeight()`.

- [ ] **Step 4: Test laufen lassen und Erfolg bestaetigen**

Run: `platformio test --environment test --filter test_RetroGfx`
Expected: PASS.

- [ ] **Step 5: Formatieren und committen**

```bash
clang-format -i lib/RetroGfx/src/ParticleSystem.h lib/RetroGfx/src/ParticleSystem.cpp test/test_RetroGfx/TestRetroGfx.cpp
git add lib/RetroGfx test/test_RetroGfx
git commit -m @'
feat(RetroGfx): add bounded particle system with sprite explosion emitter

Co-Authored-By: Claude Opus 5 (1M context) <noreply@anthropic.com>
'@
```

---

## Aufgabe 5: `Script`, `Stage`, `ITargetField` und `ScriptRunner`

Dies ist das Herzstueck. Die Typen `Script.h`, `Stage.h` und `ITargetField.h` haben fuer sich genommen kein Verhalten und werden deshalb zusammen mit dem `ScriptRunner` geliefert, der sie braucht.

**Files:**
- Create: `lib/RetroGfx/src/ITargetField.h`, `lib/RetroGfx/src/Script.h`, `lib/RetroGfx/src/Stage.h`, `lib/RetroGfx/src/Stage.cpp`, `lib/RetroGfx/src/ScriptRunner.h`, `lib/RetroGfx/src/ScriptRunner.cpp`
- Modify: `test/test_RetroGfx/TestRetroGfx.cpp`

**Interfaces:**
- Consumes: `Actor`, `ParticleSystem`, `SpriteFrame`.
- Produces:

```cpp
/** Rectangular area on the display. */
struct Rect
{
    int16_t  x;      /**< Left edge. */
    int16_t  y;      /**< Top edge. */
    uint16_t width;  /**< Width in pixel. */
    uint16_t height; /**< Height in pixel. */
};

/** How a new target appears. */
enum SpawnStyle
{
    SPAWN_STYLE_FLASH_IN = 0, /**< Flash up and fade in. */
    SPAWN_STYLE_POP_UP        /**< Push up from below. */
};

/** A field of numbered targets an animation can act on. */
class ITargetField
{
public:
    virtual ~ITargetField() {}
    virtual uint8_t getTargetCount() const                              = 0;
    virtual bool    getTargetBounds(uint8_t index, Rect& bounds) const   = 0;
    virtual void    explodeTarget(uint8_t index, ParticleStyle style)    = 0;
    virtual void    commitTarget(uint8_t index)                          = 0;
    virtual void    spawnTarget(uint8_t index, SpawnStyle style)         = 0;
    virtual bool    isTargetBusy(uint8_t index) const                    = 0;
};
```

```cpp
/** One animation of an actor, referenced by id from a script. */
struct AnimDef
{
    const SpriteFrame* frames;     /**< Frame table. */
    uint8_t            frameCount; /**< Number of frames. */
    bool               isLooping;  /**< Repeat the sequence? */
};

/** Initial setup of one actor. */
struct ActorDef
{
    int16_t  x;         /**< Initial x position. */
    int16_t  y;         /**< Initial y position. */
    uint8_t  animId;    /**< Index into the animation table. */
    uint32_t colorRaw;  /**< Color as raw RGB888 value. */
    bool     isVisible; /**< Initially visible? */
};

/** Step types of a script. */
enum StepType
{
    STEP_END = 0,           /**< End of script. */
    STEP_WAIT,              /**< p1: duration in ms. Blocking. */
    STEP_PATROL_X,          /**< p1: actor, p2: x0, p3: x1, p4: speed. */
    STEP_MOVE_TO_TARGET_X,  /**< p1: actor, p2: speed, p3: easing. Blocking. */
    STEP_SET_ANIM,          /**< p1: actor, p2: animation id. */
    STEP_SHOW,              /**< p1: actor, p2: 0 = hide, 1 = show. */
    STEP_SPAWN_AT,          /**< p1: actor, p2: reference actor, p3: dx, p4: dy. */
    STEP_LAUNCH_TO,         /**< p1: actor, p2: x or LAUNCH_X_SELF, p3: y, p4: speed, p5: arc. */
    STEP_LAUNCH_TO_TARGET,  /**< p1: actor, p2: speed, p3: arc. */
    STEP_AWAIT_IMPACT,      /**< p1: actor. Blocking. */
    STEP_EXPLODE_TARGET,    /**< p1: particle style. */
    STEP_COMMIT_TARGET,     /**< No parameter. */
    STEP_SPAWN_TARGET,      /**< p1: spawn style. */
    STEP_RETURN_TO_GROUND,  /**< p1: actor, p2: ground y, p3: speed. Blocking. */
    STEP_EMIT               /**< p1: actor, p2: particle style, p3: count. */
};

/** One step of a script. */
struct Step
{
    StepType type; /**< Step type. */
    int16_t  p1;   /**< First parameter, meaning depends on the type. */
    int16_t  p2;   /**< Second parameter. */
    int16_t  p3;   /**< Third parameter. */
    int16_t  p4;   /**< Fourth parameter. */
    int16_t  p5;   /**< Fifth parameter. */
};

/** Use the actors own x position as launch x coordinate. */
static const int16_t LAUNCH_X_SELF = INT16_MIN;
```

```cpp
class Stage
{
public:
    static const uint8_t MAX_ACTORS = 8U;

    Stage();
    void            setTargetField(ITargetField* targetField);
    ITargetField*   getTargetField();
    void            setAnimTable(const AnimDef* anims, uint8_t animCount);
    bool            applyActorDefs(const ActorDef* actors, uint8_t actorCount);
    Actor&          getActor(uint8_t index);
    ParticleSystem& getParticles();
    void            setTarget(uint8_t index);
    uint8_t         getTarget() const;
    void            update(uint32_t deltaMs);
    void            draw(YAGfx& gfx) const;
};

class ScriptRunner
{
public:
    ScriptRunner();
    void start(const Step* script, Stage& stage, bool isLooping);
    void stop();
    bool isRunning() const;
    bool isFinished() const;
    void update(uint32_t deltaMs);
};
```

`getActor()` gibt bei einem Index ausserhalb `[0; MAX_ACTORS)` den Actor `0` zurueck und loggt eine Warnung, damit ein fehlerhaftes Script nicht in undefiniertes Verhalten laeuft.

- [ ] **Step 1: Den fehlschlagenden Test schreiben**

Zuerst ein Test-Zielfeld, das die Aufrufe mitschreibt:

```cpp
/** Target field stub which records what a script did to it. */
class TargetFieldStub : public ITargetField
{
public:
    TargetFieldStub() :
        ITargetField(),
        m_explodeCount(0U),
        m_commitCount(0U),
        m_spawnCount(0U),
        m_lastIndex(0xFFU)
    {
    }

    uint8_t getTargetCount() const override
    {
        return 4U;
    }

    bool getTargetBounds(uint8_t index, Rect& bounds) const override
    {
        bool isSuccessful = false;

        if (4U > index)
        {
            bounds.x      = static_cast<int16_t>(2 + index * 16);
            bounds.y      = 12;
            bounds.width  = 12U;
            bounds.height = 20U;
            isSuccessful  = true;
        }

        return isSuccessful;
    }

    void explodeTarget(uint8_t index, ParticleStyle style) override
    {
        UTIL_NOT_USED(style);
        ++m_explodeCount;
        m_lastIndex = index;
    }

    void commitTarget(uint8_t index) override
    {
        UTIL_NOT_USED(index);
        ++m_commitCount;
    }

    void spawnTarget(uint8_t index, SpawnStyle style) override
    {
        UTIL_NOT_USED(index);
        UTIL_NOT_USED(style);
        ++m_spawnCount;
    }

    bool isTargetBusy(uint8_t index) const override
    {
        UTIL_NOT_USED(index);
        return false;
    }

    uint8_t m_explodeCount; /**< Number of explodeTarget() calls. */
    uint8_t m_commitCount;  /**< Number of commitTarget() calls. */
    uint8_t m_spawnCount;   /**< Number of spawnTarget() calls. */
    uint8_t m_lastIndex;    /**< Index of the last exploded target. */
};

static void testScriptRunnerSequence()
{
    static const Step SCRIPT[] = {
        { STEP_MOVE_TO_TARGET_X, 0, 40, EASING_LINEAR, 0, 0 },
        { STEP_WAIT, 100, 0, 0, 0, 0 },
        { STEP_EXPLODE_TARGET, PARTICLE_STYLE_BLAST, 0, 0, 0, 0 },
        { STEP_COMMIT_TARGET, 0, 0, 0, 0, 0 },
        { STEP_SPAWN_TARGET, SPAWN_STYLE_FLASH_IN, 0, 0, 0, 0 },
        { STEP_END, 0, 0, 0, 0, 0 }
    };

    TargetFieldStub field;
    Stage           stage;
    ScriptRunner    runner;

    stage.setTargetField(&field);
    stage.setTarget(2U);
    stage.getActor(0U).setPosition(0, 56);

    runner.start(SCRIPT, stage, false);
    TEST_ASSERT_TRUE(runner.isRunning());
    TEST_ASSERT_FALSE(runner.isFinished());

    /* Nothing happened to the target while the actor is still moving. */
    runner.update(100U);
    TEST_ASSERT_EQUAL_UINT8(0U, field.m_explodeCount);

    /* Enough time for move, wait and the remaining setting steps. */
    runner.update(5000U);
    TEST_ASSERT_EQUAL_UINT8(1U, field.m_explodeCount);
    TEST_ASSERT_EQUAL_UINT8(1U, field.m_commitCount);
    TEST_ASSERT_EQUAL_UINT8(1U, field.m_spawnCount);
    TEST_ASSERT_EQUAL_UINT8(2U, field.m_lastIndex);
    TEST_ASSERT_TRUE(runner.isFinished());
    TEST_ASSERT_FALSE(runner.isRunning());
}

static void testScriptRunnerLooping()
{
    static const Step SCRIPT[] = {
        { STEP_EMIT, 0, PARTICLE_STYLE_SPARK, 1, 0, 0 },
        { STEP_WAIT, 100, 0, 0, 0, 0 },
        { STEP_END, 0, 0, 0, 0, 0 }
    };

    TargetFieldStub field;
    Stage           stage;
    ScriptRunner    runner;

    stage.setTargetField(&field);
    runner.start(SCRIPT, stage, true);

    runner.update(1000U);

    /* A looping script never finishes. */
    TEST_ASSERT_TRUE(runner.isRunning());
    TEST_ASSERT_FALSE(runner.isFinished());
}

static void testScriptRunnerExtremeTimeStep()
{
    /* Review focus: deltaMs of 0 must not advance, a huge one must not skip
     * setting steps.
     */
    static const Step SCRIPT[] = {
        { STEP_WAIT, 50, 0, 0, 0, 0 },
        { STEP_EXPLODE_TARGET, PARTICLE_STYLE_BLAST, 0, 0, 0, 0 },
        { STEP_WAIT, 50, 0, 0, 0, 0 },
        { STEP_COMMIT_TARGET, 0, 0, 0, 0, 0 },
        { STEP_END, 0, 0, 0, 0, 0 }
    };

    TargetFieldStub field;
    Stage           stage;
    ScriptRunner    runner;

    stage.setTargetField(&field);
    runner.start(SCRIPT, stage, false);

    runner.update(0U);
    TEST_ASSERT_EQUAL_UINT8(0U, field.m_explodeCount);

    /* One huge step must execute both setting steps, not only the first. */
    runner.update(2000U);
    TEST_ASSERT_EQUAL_UINT8(1U, field.m_explodeCount);
    TEST_ASSERT_EQUAL_UINT8(1U, field.m_commitCount);
    TEST_ASSERT_TRUE(runner.isFinished());
}

static void testScriptRunnerStop()
{
    static const Step SCRIPT[] = {
        { STEP_WAIT, 1000, 0, 0, 0, 0 },
        { STEP_EXPLODE_TARGET, PARTICLE_STYLE_BLAST, 0, 0, 0, 0 },
        { STEP_END, 0, 0, 0, 0, 0 }
    };

    TargetFieldStub field;
    Stage           stage;
    ScriptRunner    runner;

    stage.setTargetField(&field);
    runner.start(SCRIPT, stage, false);
    runner.update(100U);
    runner.stop();

    /* After stop nothing happens any more. */
    runner.update(5000U);
    TEST_ASSERT_EQUAL_UINT8(0U, field.m_explodeCount);
    TEST_ASSERT_FALSE(runner.isRunning());
}
```

`RUN_TEST` fuer die vier Funktionen eintragen.

- [ ] **Step 2: Test laufen lassen und Fehlschlag bestaetigen**

Run: `platformio test --environment test --filter test_RetroGfx`
Expected: FAIL, `Stage.h: No such file or directory`.

- [ ] **Step 3: `ITargetField.h`, `Script.h` und `Stage` implementieren**

`Stage` haelt `Actor m_actors[MAX_ACTORS]`, `ParticleSystem m_particles`, `ITargetField* m_targetField`, `const AnimDef* m_anims`, `uint8_t m_animCount`, `uint8_t m_target`. `update()` reicht `deltaMs` an alle Actors und an das Partikelsystem weiter. `draw()` zeichnet erst die Partikel, dann die Actors.

- [ ] **Step 4: `ScriptRunner` implementieren**

Kernpunkt: `update()` fuehrt in **einer begrenzten Schleife** so lange Steps aus, bis ein blockierender Step noch nicht abgeschlossen ist oder das Script endet. Die verbleibende Zeit wird an den blockierenden Step weitergereicht, damit ein grosser `deltaMs` mehrere Steps abarbeiten kann, ohne einen zu ueberspringen.

```cpp
void ScriptRunner::update(uint32_t deltaMs)
{
    if (nullptr == m_script)
    {
        /* Guard path: nothing to run. */
    }
    else if (nullptr == m_stage)
    {
        /* Guard path: no stage assigned. */
    }
    else
    {
        uint32_t remainingMs = deltaMs;
        uint8_t  guard       = 0U;

        while ((nullptr != m_script) && (MAX_STEPS_PER_UPDATE > guard))
        {
            const bool isStepDone = processStep(remainingMs);

            ++guard;

            if (false == isStepDone)
            {
                break;
            }

            /* A blocking step consumed its share; the rest goes to the next one. */
            remainingMs = m_carryOverMs;
            advance();
        }
    }
}
```

`processStep()` gibt `true` zurueck, wenn der aktuelle Step abgeschlossen ist. Setzende Steps schliessen sofort ab und setzen `m_carryOverMs` auf den unveraenderten Rest. Blockierende Steps verbrauchen Zeit; `STEP_WAIT` verbraucht hoechstens die Restdauer und legt den Ueberschuss in `m_carryOverMs`, `STEP_MOVE_TO_TARGET_X` / `STEP_AWAIT_IMPACT` / `STEP_RETURN_TO_GROUND` pruefen `Actor::isBehaviourDone()` und setzen `m_carryOverMs` auf `0U`.

Wichtig fuer `testScriptRunnerExtremeTimeStep`: `MAX_STEPS_PER_UPDATE` muss gross genug sein (`16U`), damit eine Folge kurzer Steps in einem grossen Tick durchlaeuft, und klein genug, damit ein fehlerhaft geschriebenes Endlos-Script das System nicht blockiert.

`advance()` schaltet auf den naechsten Step. Bei `STEP_END` wird das Script bei `true == m_isLooping` neu gestartet, sonst `m_script = nullptr` und `m_isFinished = true` gesetzt.

Die Aufloesung der Step-Parameter:

- `STEP_MOVE_TO_TARGET_X`: `getTargetBounds(stage.getTarget(), bounds)`; Ziel-x ist `bounds.x + bounds.width / 2 - actorWidth / 2`.
- `STEP_LAUNCH_TO_TARGET`: Zielpunkt ist die Mitte der Unterkante, also `bounds.x + bounds.width / 2` und `bounds.y + bounds.height`.
- `STEP_LAUNCH_TO` mit `p2 == LAUNCH_X_SELF`: die aktuelle x-Position des Actors.
- `STEP_AWAIT_IMPACT`: blockiert, bis `isBehaviourDone()` des Actors `true` liefert.
- `STEP_EXPLODE_TARGET`, `STEP_COMMIT_TARGET`, `STEP_SPAWN_TARGET`: wirken immer auf `stage.getTarget()`.

- [ ] **Step 5: Test laufen lassen und Erfolg bestaetigen**

Run: `platformio test --environment test --filter test_RetroGfx`
Expected: PASS.

- [ ] **Step 6: Formatieren und committen**

```bash
clang-format -i lib/RetroGfx/src/ITargetField.h lib/RetroGfx/src/Script.h lib/RetroGfx/src/Stage.h lib/RetroGfx/src/Stage.cpp lib/RetroGfx/src/ScriptRunner.h lib/RetroGfx/src/ScriptRunner.cpp test/test_RetroGfx/TestRetroGfx.cpp
git add lib/RetroGfx test/test_RetroGfx
git commit -m @'
feat(RetroGfx): add stage, script types and branch free script runner

Co-Authored-By: Claude Opus 5 (1M context) <noreply@anthropic.com>
'@
```

---

## Aufgabe 6: `TimeSequencer`

Reine Logik, ohne Grafik: Welche Ziffern haben sich geaendert, und darf der Wechsel ueberhaupt animiert werden?

**Files:**
- Create: `lib/RetroTimePlugin/library.json`, `lib/RetroTimePlugin/src/internal/TimeSequencer.h`, `lib/RetroTimePlugin/src/internal/TimeSequencer.cpp`
- Create: `test/test_RetroTime/TestRetroTime.cpp`
- Modify: `platformio.ini` (`[env:test]` → `lib_deps`)

**Interfaces:**
- Consumes: nichts aus vorherigen Aufgaben.
- Produces:

```cpp
class TimeSequencer
{
public:
    /** Number of digits of a HH:MM time. */
    static const uint8_t DIGIT_COUNT = 4U;

    /** What shall happen with a newly fed time. */
    enum Decision
    {
        DECISION_NONE = 0, /**< Time did not change, do nothing. */
        DECISION_SNAP,     /**< Take it over without animation. */
        DECISION_ANIMATE   /**< Animate the changed digits. */
    };

    TimeSequencer();
    void        reset();
    Decision    feed(const char* timeStr);
    uint8_t     getQueueCount() const;
    bool        popNextDigit(uint8_t& index);
    const char* getShownTime() const;
};
```

`feed()` erwartet einen Zeiger auf einen null-terminierten String der Form `HH:MM`. Jeder andere Inhalt — auch `--:--` — fuehrt zu `DECISION_SNAP`, weil dann keine sinnvolle Minutenfolge bestimmbar ist. `DECISION_ANIMATE` gibt es ausschliesslich dann, wenn die neue Zeit genau die auf die zuletzt angezeigte folgende Minute ist. `reset()` verwirft die Warteschlange und die zuletzt angezeigte Zeit.

- [ ] **Step 1: `RetroTimePlugin` in der Test-Umgebung bekannt machen**

In `platformio.ini`, `[env:test]`, `lib_deps` ergaenzen (alphabetisch nach `RetroGfx`):

```ini
    RetroGfx
    RetroTimePlugin
```

- [ ] **Step 2: `library.json` des Plugins anlegen**

```json
{
    "name": "RetroTimePlugin",
    "version": "0.1.0",
    "description": "Shows date and time on a 64x64 display with retro game animations.",
    "authors": [{
        "name": "Andreas Merkle",
        "email": "web@blue-andi.de",
        "url": "https://github.com/BlueAndi",
        "maintainer": true
    }],
    "license": "MIT",
    "dependencies": [{
        "name": "Logging"
    }, {
        "name": "LittleFS"
    }, {
        "name": "Plugin"
    }, {
        "name": "IRtc"
    }, {
        "name": "Utilities"
    }, {
        "name": "Views"
    }, {
        "name": "RetroGfx"
    }],
    "frameworks": "*",
    "platforms": "*"
}
```

- [ ] **Step 3: Den fehlschlagenden Test schreiben**

`test/test_RetroTime/TestRetroTime.cpp` nach dem Muster aus Aufgabe 1 anlegen.

```cpp
#include <unity.h>
#include <TimeSequencer.h>

static void testTimeSequencerFirstFeed()
{
    TimeSequencer sequencer;

    /* The very first time is always taken over hard. */
    TEST_ASSERT_EQUAL_INT(TimeSequencer::DECISION_SNAP, sequencer.feed("12:34"));
    TEST_ASSERT_EQUAL_STRING("12:34", sequencer.getShownTime());
    TEST_ASSERT_EQUAL_UINT8(0U, sequencer.getQueueCount());
}

static void testTimeSequencerNextMinute()
{
    TimeSequencer sequencer;
    uint8_t       index = 0U;

    (void)sequencer.feed("12:34");

    /* 12:34 -> 12:35 changes only the last digit. */
    TEST_ASSERT_EQUAL_INT(TimeSequencer::DECISION_ANIMATE, sequencer.feed("12:35"));
    TEST_ASSERT_EQUAL_UINT8(1U, sequencer.getQueueCount());
    TEST_ASSERT_TRUE(sequencer.popNextDigit(index));
    TEST_ASSERT_EQUAL_UINT8(3U, index);
    TEST_ASSERT_FALSE(sequencer.popNextDigit(index));
}

static void testTimeSequencerMidnight()
{
    TimeSequencer sequencer;
    uint8_t       index = 0U;

    (void)sequencer.feed("23:59");

    /* 23:59 -> 00:00 wraps and changes all four digits, left to right. */
    TEST_ASSERT_EQUAL_INT(TimeSequencer::DECISION_ANIMATE, sequencer.feed("00:00"));
    TEST_ASSERT_EQUAL_UINT8(4U, sequencer.getQueueCount());

    TEST_ASSERT_TRUE(sequencer.popNextDigit(index));
    TEST_ASSERT_EQUAL_UINT8(0U, index);
    TEST_ASSERT_TRUE(sequencer.popNextDigit(index));
    TEST_ASSERT_EQUAL_UINT8(1U, index);
    TEST_ASSERT_TRUE(sequencer.popNextDigit(index));
    TEST_ASSERT_EQUAL_UINT8(2U, index);
    TEST_ASSERT_TRUE(sequencer.popNextDigit(index));
    TEST_ASSERT_EQUAL_UINT8(3U, index);
}

static void testTimeSequencerJump()
{
    TimeSequencer sequencer;

    (void)sequencer.feed("12:34");

    /* NTP sync jumps forward: not the next minute, so snap. */
    TEST_ASSERT_EQUAL_INT(TimeSequencer::DECISION_SNAP, sequencer.feed("14:02"));
    TEST_ASSERT_EQUAL_UINT8(0U, sequencer.getQueueCount());
    TEST_ASSERT_EQUAL_STRING("14:02", sequencer.getShownTime());

    /* Time running backwards also snaps. */
    TEST_ASSERT_EQUAL_INT(TimeSequencer::DECISION_SNAP, sequencer.feed("14:01"));
    TEST_ASSERT_EQUAL_UINT8(0U, sequencer.getQueueCount());
}

static void testTimeSequencerNoChange()
{
    TimeSequencer sequencer;

    (void)sequencer.feed("12:34");
    TEST_ASSERT_EQUAL_INT(TimeSequencer::DECISION_NONE, sequencer.feed("12:34"));
    TEST_ASSERT_EQUAL_UINT8(0U, sequencer.getQueueCount());
}

static void testTimeSequencerNoTimeAvailable()
{
    /* Review focus: no NTP and no RTC. */
    TimeSequencer sequencer;

    TEST_ASSERT_EQUAL_INT(TimeSequencer::DECISION_SNAP, sequencer.feed("--:--"));
    TEST_ASSERT_EQUAL_STRING("--:--", sequencer.getShownTime());
    TEST_ASSERT_EQUAL_UINT8(0U, sequencer.getQueueCount());

    /* Coming back from no time must not animate. */
    TEST_ASSERT_EQUAL_INT(TimeSequencer::DECISION_SNAP, sequencer.feed("07:05"));
    TEST_ASSERT_EQUAL_UINT8(0U, sequencer.getQueueCount());

    /* A nullptr and a short string must be survived. */
    TEST_ASSERT_EQUAL_INT(TimeSequencer::DECISION_NONE, sequencer.feed(nullptr));
    TEST_ASSERT_EQUAL_INT(TimeSequencer::DECISION_SNAP, sequencer.feed("7:5"));
}

static void testTimeSequencerResetDropsQueue()
{
    /* Review focus: the slot went inactive in the middle of a sequence. */
    TimeSequencer sequencer;
    uint8_t       index = 0U;

    (void)sequencer.feed("23:59");
    (void)sequencer.feed("00:00");
    TEST_ASSERT_EQUAL_UINT8(4U, sequencer.getQueueCount());

    sequencer.reset();
    TEST_ASSERT_EQUAL_UINT8(0U, sequencer.getQueueCount());
    TEST_ASSERT_FALSE(sequencer.popNextDigit(index));

    /* After a reset the next time is taken over hard, not animated. */
    TEST_ASSERT_EQUAL_INT(TimeSequencer::DECISION_SNAP, sequencer.feed("00:01"));
}
```

Alle sieben in `main()` mit `RUN_TEST` eintragen.

- [ ] **Step 4: Test laufen lassen und Fehlschlag bestaetigen**

Run: `platformio test --environment test --filter test_RetroTime`
Expected: FAIL, `TimeSequencer.h: No such file or directory`.

- [ ] **Step 5: `TimeSequencer` implementieren**

Member: `char m_shownTime[DIGIT_COUNT + 2U]` (`"HH:MM"` plus Nullterminierung), `int16_t m_shownMinuteOfDay` (`-1`, wenn ungueltig), `uint8_t m_queue[DIGIT_COUNT]`, `uint8_t m_queueCount`, `uint8_t m_queueIndex`.

`feed()`:

1. Guard: `nullptr == timeStr` → `DECISION_NONE`.
2. Guard: Laenge ungleich 5 oder `':' != timeStr[2]` → `DECISION_SNAP` mit Uebernahme des Strings und `m_shownMinuteOfDay = -1`.
3. Gleicher String wie bisher → `DECISION_NONE`.
4. Ziffernpositionen sind `0`, `1`, `3`, `4` im String; Index `0..3` bezeichnet die Ziffer. Warteschlange aus allen abweichenden Positionen von links nach rechts fuellen.
5. Neue Minute des Tages berechnen. Ist einer der vier Zeichen keine Ziffer, gilt die Zeit als ungueltig → `DECISION_SNAP`.
6. `DECISION_ANIMATE` genau dann, wenn `0 <= m_shownMinuteOfDay` und `newMinuteOfDay == ((m_shownMinuteOfDay + 1) % 1440)`. Sonst Warteschlange leeren und `DECISION_SNAP`.
7. In jedem Fall `m_shownTime` und `m_shownMinuteOfDay` aktualisieren.

`popNextDigit()` liefert `false`, wenn die Warteschlange leer ist, und laesst `index` dann unveraendert.

- [ ] **Step 6: Test laufen lassen und Erfolg bestaetigen**

Run: `platformio test --environment test --filter test_RetroTime`
Expected: PASS, sieben Tests.

- [ ] **Step 7: Formatieren und committen**

```bash
clang-format -i lib/RetroTimePlugin/src/internal/TimeSequencer.h lib/RetroTimePlugin/src/internal/TimeSequencer.cpp test/test_RetroTime/TestRetroTime.cpp
git add lib/RetroTimePlugin test/test_RetroTime platformio.ini
git commit -m @'
feat(RetroTimePlugin): add time sequencer deciding snap versus animate

Co-Authored-By: Claude Opus 5 (1M context) <noreply@anthropic.com>
'@
```

---

## Aufgabe 7: Native 64x64-Umgebung und Plugin-Grundgeruest

Ziel dieser Aufgabe ist das erste sichtbare Ergebnis: eine statische Uhr im SDL-Fenster. Damit ist die gesamte Build-Integration belegt, bevor Animationslogik dazukommt.

**Files:**
- Create: `config/configNative64x64.ini`
- Modify: `config/display.ini`, `config/board.ini`, `platformio.ini`
- Create: `lib/RetroTimePlugin/src/RetroTimePlugin.h`, `lib/RetroTimePlugin/src/RetroTimePlugin.cpp`
- Create: `lib/RetroTimePlugin/src/internal/View.h`
- Create: `lib/RetroTimePlugin/src/internal/RetroTimeViewGeneric.h`, `.cpp`
- Create: `lib/RetroTimePlugin/pixelix.json`

**Interfaces:**
- Consumes: `PluginWithConfig` aus `lib/Plugin/src/PluginWithConfig.hpp`, `ClockDrv` aus `src/`, `TextWidget` aus `lib/YAWidgets`, `Layout` / `LAYOUT_TYPE` aus `lib/Views/src/Layouts.h`, `TimeSequencer` aus Aufgabe 6.
- Produces:
  - `class RetroTimePlugin : public PluginWithConfig` mit `create()`, `start()`, `stop()`, `process()`, `active()`, `inactive()`, `update()`, `getTopics()`, `getTopic()`, `setTopic()`, `hasTopicChanged()`
  - `class RetroTimeViewGeneric` mit `init(uint16_t width, uint16_t height)`, `setDate(const String& date)`, `setTime(const String& time)`, `update(YAGfx& gfx)`
  - `_RetroTimePlugin::View` als Alias auf die zum `LAYOUT_TYPE` passende Variante

- [ ] **Step 1: Natives 64x64-Display in `config/display.ini` ergaenzen**

Neuer Abschnitt, Vorlage ist `[display:led_matrix_native]` bei Zeile 314:

```ini
; ********************************************************************************
; Native LED matrix simulation with 64x64 pixel - only for development
; ********************************************************************************
[display:led_matrix_native_64x64]
build_flags =
    ${display:common.build_flags}
    -D CONFIG_LED_MATRIX_WIDTH=64U
    -D CONFIG_LED_MATRIX_HEIGHT=64U
    -D CONFIG_LED_TOPO=RowMajorAlternatingLayout
    ; Only 32 (RGB888) or 16 (RGB565) are supported.
    -D CONFIG_COLOR_DEPTH=32
lib_deps_builtin =
    HalLedMatrixNative
lib_deps_external =
lib_ignore_builtin =
    HalHub75Esp32
    HalLedMatrix
    HalTftDisplay
lib_ignore_external =
```

- [ ] **Step 2: `config/configNative64x64.ini` anlegen**

Kopie von `config/configNative.ini` mit dem Abschnittsnamen `[config:native64x64]`, dem zusaetzlichen Eintrag `RetroTimePlugin @ ~0.1.0` in der Plugin-Liste und dem `build_flags`-Eintrag fuer den Demo-Modus:

```ini
; Configuration for the native 64x64 simulation.
[config:native64x64]
build_flags =
    -D CONFIG_RETROTIME_FAST_DEMO=1
lib_deps =
    ...
    RetroTimePlugin @ ~0.1.0
    ...
extra_scripts =
    pre:./scripts/configure_normal.py
```

Der `extra_scripts`-Eintrag muss dem entsprechen, den `[config:native]` verwendet — beim Anlegen aus `configNative.ini` uebernehmen, nicht raten.

- [ ] **Step 3: `[board:native-64x64]` in `config/board.ini` ergaenzen**

Kopie von `[board:native]` bei Zeile 988, mit `extends = mcu:native, display:led_matrix_native_64x64, config:native64x64` und entsprechend angepassten `build_flags`-Referenzen. Der Webserver-Port wird auf `8081` gesetzt, damit beide nativen Umgebungen gleichzeitig laufen koennen.

- [ ] **Step 4: `[env:native-64x64]` in `platformio.ini` ergaenzen**

Direkt nach `[env:native]`:

```ini
; ********************************************************************************
; Native desktop platform with 64x64 display - Only for development
; ********************************************************************************
[env:native-64x64]
extends = board:native-64x64, mode:selected
build_flags =
    ${board:native-64x64.build_flags}
    ${mode:selected.build_flags}
extra_scripts =
    ${board:native-64x64.extra_scripts}
```

- [ ] **Step 5: `pixelix.json` anlegen**

```json
{
    "pixelix": {
        "type": "plugin",
        "name": "RetroTimePlugin",
        "web": {
            "files": [
                "./web/RetroTimePlugin.html",
                "./web/RetroTimePlugin.jpg"
            ]
        }
    }
}
```

- [ ] **Step 6: `RetroTimeViewGeneric` implementieren**

Zwei `TextWidget`, einer fuer Datum, einer fuer Uhrzeit, beide horizontal zentriert. `init()` verteilt die verfuegbare Hoehe: Datum in der oberen, Uhrzeit in der unteren Haelfte. Font ist `Fonts::getFontByType(Fonts::FONT_TYPE_DEFAULT)`. Keine Animation, kein Partikelsystem.

- [ ] **Step 7: `View.h` mit der Layout-Auswahl anlegen**

```cpp
#include <Layouts.h>

#include "RetroTimeViewGeneric.h"
#include "RetroTimeView64x64.h"

/** Internal plugin functionality. */
namespace _RetroTimePlugin
{

/**
 * RetroTimePlugin view.
 *
 * @tparam option   Layout which to choose
 */
template<Layout option>
class RetroTimeView : public RetroTimeViewGeneric
{
public:

    /**
     * Destroys the view.
     */
    virtual ~RetroTimeView() = default;
};

/**
 * RetroTimePlugin view for a 64x64 display.
 */
template<>
class RetroTimeView<LAYOUT_64X64> : public RetroTimeView64x64
{
public:

    /**
     * Destroys the view.
     */
    virtual ~RetroTimeView() = default;
};

/** View considering the display size. */
using View = RetroTimeView<LAYOUT_TYPE>;

} // namespace _RetroTimePlugin
```

In dieser Aufgabe existiert `RetroTimeView64x64` noch nicht. Damit die Umgebung sofort baubar bleibt, wird `RetroTimeView64x64.h` in dieser Aufgabe als **minimale Ableitung von `RetroTimeViewGeneric` ohne eigene Logik** angelegt und in Aufgabe 8 ausgebaut. So ist nach jeder Aufgabe ein lauffaehiger Stand vorhanden.

- [ ] **Step 8: `RetroTimePlugin` implementieren**

Am Aufbau von `lib/DateTimePlugin/src/DateTimePlugin.h` orientieren. In dieser Aufgabe noch ohne Animationsbezug:

- Member: `_RetroTimePlugin::View m_view`, `SimpleTimer m_checkUpdateTimer`, `TimeSequencer m_sequencer`, `String m_dateFormat`, `String m_timeZone`, `String m_animationName`, `uint32_t m_lastUpdateMs`, `mutable MutexRecursive m_mutex`, `bool m_hasTopicChanged`.
- `start(width, height)` ruft `m_view.init(width, height)`.
- `process(isConnected)` prueft im Sekundentakt (`CHECK_UPDATE_PERIOD = SIMPLE_TIMER_SECONDS(1U)`) die Zeit ueber `ClockDrv` und legt Datum und Uhrzeit unter `m_mutex` ab.
- `update(gfx)` berechnet `deltaMs` aus `millis()` gegen `m_lastUpdateMs` und ruft `m_view.update(gfx)`.
- `getTopics()` / `getTopic()` / `setTopic()` / `hasTopicChanged()` analog zu `DateTimePlugin`, mit `TOPIC_CONFIG[] = "config"`.
- `getConfiguration()` / `setConfiguration()` schreiben und lesen `animation`, `dateFormat`, `timeZone`. Ein leerer oder unbekannter Animationsname wird in Aufgabe 9 abgefangen; hier wird der Wert nur gehalten.

Steht keine Zeit zur Verfuegung, wird `"--:--"` gesetzt. Das ist die Zusage aus dem Review-Fokus und muss hier tatsaechlich verdrahtet sein.

- [ ] **Step 9: Plugin in die 64x64-Konfiguration aufnehmen**

In `config/configNormal.ini` in der Plugin-Liste alphabetisch einfuegen:

```ini
    RetroTimePlugin @ ~0.1.0 # 64x64 displays only.
```

- [ ] **Step 10: Bauen und sichtbar pruefen**

```bash
platformio run --environment native-64x64
```

Die erzeugte Binaerdatei starten. Erwartet: ein 64x64-Fenster, das Plugin ist ueber die Weboberflaeche unter `http://localhost:8081` in einen Slot installierbar und zeigt Datum und Uhrzeit ohne Animation.

- [ ] **Step 11: Regressionen ausschliessen**

```bash
platformio run --environment esp32doit-devkit-v1-LED-32x8
platformio test --environment test
```

Expected: beide erfolgreich. Der erste Lauf belegt, dass der Generic-Fallback auf 32x8 uebersetzt.

- [ ] **Step 12: Formatieren und committen**

```bash
clang-format -i lib/RetroTimePlugin/src/RetroTimePlugin.h lib/RetroTimePlugin/src/RetroTimePlugin.cpp lib/RetroTimePlugin/src/internal/View.h lib/RetroTimePlugin/src/internal/RetroTimeViewGeneric.h lib/RetroTimePlugin/src/internal/RetroTimeViewGeneric.cpp
git add lib/RetroTimePlugin config platformio.ini
git commit -m @'
feat(RetroTimePlugin): add plugin skeleton and native 64x64 environment

Co-Authored-By: Claude Opus 5 (1M context) <noreply@anthropic.com>
'@
```

---

## Aufgabe 8: `DigitSprites`, `DigitField` und das 64x64-Layout

**Files:**
- Create: `lib/RetroTimePlugin/src/internal/DigitSprites.h`, `.cpp`
- Create: `lib/RetroTimePlugin/src/internal/DigitField.h`, `.cpp`
- Modify: `lib/RetroTimePlugin/src/internal/RetroTimeView64x64.h`, `.cpp` (aus Aufgabe 7)
- Modify: `test/test_RetroTime/TestRetroTime.cpp`

**Interfaces:**
- Consumes: `Sprite`, `ITargetField`, `ParticleSystem`, `Rect`, `SpawnStyle` aus RetroGfx.
- Produces:
  - `DigitSprites::getDigit(uint8_t value)` liefert `const Sprite&` fuer `0..9`, fuer alles andere ein leeres Sprite; `DigitSprites::getBlank()` fuer die Anzeige `--:--`.
  - `class DigitField : public ITargetField` mit `DigitField()`, `setParticleSystem(ParticleSystem* particles)`, `setColor(const Color& color)`, `setTimeHard(const char* timeStr)`, `setTimePending(const char* timeStr)`, `update(uint32_t deltaMs)`, `draw(YAGfx& gfx) const`, `isBlinkOn() const` sowie die `ITargetField`-Methoden.

Geometrie fest nach Spec Abschnitt 4: Ziffernbreite `12U`, Ziffernhoehe `20U`, Ziffer-x `{ 2, 16, 36, 50 }`, Ziffer-y `12`, Doppelpunkt bei x `30`, Breite `4`.

- [ ] **Step 1: Den fehlschlagenden Test schreiben**

```cpp
static void testDigitFieldStates()
{
    ParticleSystem particles;
    DigitField     field;
    Rect           bounds;

    field.setParticleSystem(&particles);
    field.setTimeHard("12:34");

    TEST_ASSERT_EQUAL_UINT8(4U, field.getTargetCount());
    TEST_ASSERT_TRUE(field.getTargetBounds(0U, bounds));
    TEST_ASSERT_EQUAL_INT16(2, bounds.x);
    TEST_ASSERT_EQUAL_INT16(12, bounds.y);
    TEST_ASSERT_EQUAL_UINT16(12U, bounds.width);
    TEST_ASSERT_EQUAL_UINT16(20U, bounds.height);

    TEST_ASSERT_TRUE(field.getTargetBounds(3U, bounds));
    TEST_ASSERT_EQUAL_INT16(50, bounds.x);

    /* Out of range is refused instead of returning garbage. */
    TEST_ASSERT_FALSE(field.getTargetBounds(4U, bounds));

    /* A stable digit is not busy. */
    TEST_ASSERT_FALSE(field.isTargetBusy(3U));

    /* Exploding makes it busy and fills the particle system. */
    field.setTimePending("12:35");
    field.explodeTarget(3U, PARTICLE_STYLE_BLAST);
    TEST_ASSERT_TRUE(field.isTargetBusy(3U));
    TEST_ASSERT_LESS_OR_EQUAL_UINT8(ParticleSystem::MAX_PARTICLES, particles.getActiveCount());
    TEST_ASSERT_GREATER_THAN_UINT8(0U, particles.getActiveCount());

    /* After the decay it is empty, then it is committed and spawned. */
    field.update(2000U);
    field.commitTarget(3U);
    field.spawnTarget(3U, SPAWN_STYLE_FLASH_IN);
    TEST_ASSERT_TRUE(field.isTargetBusy(3U));

    field.update(2000U);
    TEST_ASSERT_FALSE(field.isTargetBusy(3U));
}

static void testDigitFieldNoTime()
{
    DigitField field;

    /* Review focus: no time available must be displayable. */
    field.setTimeHard("--:--");
    TEST_ASSERT_EQUAL_UINT8(4U, field.getTargetCount());
    TEST_ASSERT_FALSE(field.isTargetBusy(0U));
}

static void testDigitFieldBlink()
{
    DigitField field;

    field.setTimeHard("12:34");
    TEST_ASSERT_TRUE(field.isBlinkOn());

    field.update(500U);
    TEST_ASSERT_FALSE(field.isBlinkOn());

    field.update(500U);
    TEST_ASSERT_TRUE(field.isBlinkOn());
}
```

`RUN_TEST` fuer die drei Funktionen eintragen.

- [ ] **Step 2: Test laufen lassen und Fehlschlag bestaetigen**

Run: `platformio test --environment test --filter test_RetroTime`
Expected: FAIL, `DigitField.h: No such file or directory`.

- [ ] **Step 3: `DigitSprites` anlegen**

Zehn Ziffern zu 12x20 Pixel als 1-bpp-Bitmaps, Zeilenschrittweite 2 Byte, also 40 Byte je Ziffer. Arcade-Look: Strichstaerke 3 Pixel, eckige Ecken, keine Diagonalen. Beispiel fuer die `1`, die bewusst mit Fuss und Fahne gezeichnet wird, damit sie nicht als Strich wirkt:

```cpp
/** Sprite data of digit 1, 12 x 20 pixel, 2 byte per row. */
static const uint8_t DIGIT_1_DATA[] = {
    0x03U, 0x00U, /* ......XX.... */
    0x0FU, 0x00U, /* ....XXXX.... */
    0x3FU, 0x00U, /* ..XXXXXX.... */
    0x33U, 0x00U, /* ..XX..XX.... */
    0x03U, 0x00U, /* ......XX.... */
    0x03U, 0x00U,
    0x03U, 0x00U,
    0x03U, 0x00U,
    0x03U, 0x00U,
    0x03U, 0x00U,
    0x03U, 0x00U,
    0x03U, 0x00U,
    0x03U, 0x00U,
    0x03U, 0x00U,
    0x03U, 0x00U,
    0x03U, 0x00U,
    0x03U, 0x00U,
    0x3FU, 0xF0U, /* ..XXXXXXXXXX */
    0x3FU, 0xF0U,
    0x3FU, 0xF0U
};
```

Die uebrigen neun Ziffern nach demselben Schema. Jede Zeile bekommt einen Kommentar mit dem Pixelmuster, damit die Grafik im Review lesbar ist. Zusaetzlich ein `BLANK_DATA` (alle Bytes `0x00U`) fuer `--:--` und ein `COLON_DATA` mit 4x20 Pixel: zwei Bloecke zu 4x4 bei y 4 und y 12.

Zugriff ueber eine Funktion mit Bereichspruefung:

```cpp
const Sprite& DigitSprites::getDigit(uint8_t value)
{
    const Sprite* sprite = &BLANK_SPRITE;

    if (10U > value)
    {
        sprite = &DIGIT_SPRITES[value];
    }

    return *sprite;
}
```

- [ ] **Step 4: `DigitField` implementieren**

```cpp
/** State of a single digit. */
enum DigitState
{
    DIGIT_STATE_STABLE = 0, /**< Shown, nothing going on. */
    DIGIT_STATE_EXPLODING,  /**< Decaying into particles. */
    DIGIT_STATE_EMPTY,      /**< Not shown. */
    DIGIT_STATE_SPAWNING    /**< Appearing. */
};

/** One digit of the time. */
struct Digit
{
    uint8_t    value;        /**< Currently shown value. [0; 9] or BLANK_VALUE */
    uint8_t    pendingValue; /**< Value taken over on commit. */
    DigitState state;        /**< Current state. */
    uint16_t   elapsedMs;    /**< Time in the current state. */
};
```

- `setTimeHard()` setzt `value` und `pendingValue` beider gleich und `state` auf `DIGIT_STATE_STABLE`. Alle laufenden Zustaende werden verworfen.
- `setTimePending()` setzt nur `pendingValue`.
- `explodeTarget()` ruft `m_particles->explodeSprite()` mit dem Sprite des aktuellen Wertes an der Ziffernposition und geht nach `DIGIT_STATE_EXPLODING`. Ist `nullptr == m_particles`, wird direkt nach `DIGIT_STATE_EMPTY` gewechselt — die Ziffer verschwindet dann ohne Partikel, statt dass nichts passiert.
- `commitTarget()` uebernimmt `pendingValue` nach `value`.
- `spawnTarget()` geht nach `DIGIT_STATE_SPAWNING`.
- `update()` fuehrt die Zeitzaehler und schaltet `EXPLODING` nach `EXPLODE_DURATION_MS = 300U` auf `EMPTY` und `SPAWNING` nach `SPAWN_DURATION_MS = 250U` auf `STABLE`. Ausserdem wird der Doppelpunkt-Blinker mit `BLINK_PERIOD_MS = 500U` gefuehrt.
- `isTargetBusy()` liefert `true` fuer alles ausser `DIGIT_STATE_STABLE`.
- `draw()` zeichnet Ziffern in `STABLE` normal, in `SPAWNING` je nach Style (bei `SPAWN_STYLE_FLASH_IN` in den ersten 80 ms in Weiss, danach in der Zielfarbe; bei `SPAWN_STYLE_POP_UP` von unten hereingeschoben), in `EXPLODING` und `EMPTY` gar nicht. Der Doppelpunkt wird nur bei `true == isBlinkOn()` gezeichnet.

- [ ] **Step 5: `RetroTimeView64x64` auf das Layout ausbauen**

Die in Aufgabe 7 angelegte Minimalableitung wird ersetzt. Member: `TextWidget m_dateWidget`, `DigitField m_digitField`, `Stage m_stage`, `ScriptRunner m_runner`. In dieser Aufgabe wird noch kein Script gestartet; es geht nur um das statische Layout nach Spec Abschnitt 4.

`init()` setzt das Datums-Widget auf `(0, 0, 64, 7)` mit `Alignment::HORIZONTAL_CENTER`, verbindet `m_digitField` mit `m_stage.getParticles()` und setzt `m_stage.setTargetField(&m_digitField)`.

`update(YAGfx& gfx)` fuellt den Hintergrund schwarz, zeichnet das Datums-Widget, dann `m_digitField.draw(gfx)`, dann `m_stage.draw(gfx)` und zuletzt die Bodenlinie bei `y = 63`.

- [ ] **Step 6: Tests laufen lassen und Erfolg bestaetigen**

Run: `platformio test --environment test --filter test_RetroTime`
Expected: PASS.

- [ ] **Step 7: Sichtprobe**

```bash
platformio run --environment native-64x64
```

Erwartet: Datum oben klein, vier grosse Ziffern mit blinkendem Doppelpunkt in der Mitte, Bodenlinie unten.

- [ ] **Step 8: Formatieren und committen**

```bash
clang-format -i lib/RetroTimePlugin/src/internal/DigitSprites.h lib/RetroTimePlugin/src/internal/DigitSprites.cpp lib/RetroTimePlugin/src/internal/DigitField.h lib/RetroTimePlugin/src/internal/DigitField.cpp lib/RetroTimePlugin/src/internal/RetroTimeView64x64.h lib/RetroTimePlugin/src/internal/RetroTimeView64x64.cpp test/test_RetroTime/TestRetroTime.cpp
git add lib/RetroTimePlugin test/test_RetroTime
git commit -m @'
feat(RetroTimePlugin): add digit sprites, digit field and 64x64 layout

Co-Authored-By: Claude Opus 5 (1M context) <noreply@anthropic.com>
'@
```

---

## Aufgabe 9: Space-Invaders-Daten und `AnimationRegistry`

**Files:**
- Create: `lib/RetroTimePlugin/src/internal/anim/SpaceInvaders.h`, `.cpp`
- Create: `lib/RetroTimePlugin/src/internal/AnimationRegistry.h`, `.cpp`
- Modify: `lib/RetroTimePlugin/src/internal/RetroTimeView64x64.h`, `.cpp`
- Modify: `test/test_RetroTime/TestRetroTime.cpp`

**Interfaces:**
- Consumes: `AnimDef`, `ActorDef`, `Step`, `StepType`, `Stage`, `ScriptRunner` aus RetroGfx.
- Produces:

```cpp
/** A complete animation as data. */
struct AnimationDef
{
    const char*     name;       /**< Configuration name, must be unique. */
    const AnimDef*  anims;      /**< Animation table, referenced by SET_ANIM. */
    uint8_t         animCount;  /**< Number of animations. */
    const ActorDef* actors;     /**< Initial actor setup. */
    uint8_t         actorCount; /**< Number of actors. */
    const Step*     idleScript; /**< Looping idle show. */
    const Step*     killScript; /**< One shot per changed digit. */
};

class AnimationRegistry
{
public:
    static uint8_t             getCount();
    static const AnimationDef* getByIndex(uint8_t index);
    static const AnimationDef* getByName(const char* name);
    static const AnimationDef* getRandom();
};
```

`getByName()` liefert bei unbekanntem oder leerem Namen und bei `nullptr` den ersten Eintrag und gibt `LOG_WARNING` aus. Die Registry ist damit nie `nullptr`, solange mindestens eine Animation einkompiliert ist.

Actor-Indizes von Space Invaders: `0` Held, `1` Schuss, `2` Gegnergruppe. Animations-Ids: `0` Held ruhend, `1` Schuss, `2` Gegner marschierend.

- [ ] **Step 1: Den fehlschlagenden Test schreiben**

```cpp
static void testAnimationRegistry()
{
    TEST_ASSERT_GREATER_THAN_UINT8(0U, AnimationRegistry::getCount());

    const AnimationDef* invaders = AnimationRegistry::getByName("spaceInvaders");

    TEST_ASSERT_NOT_NULL(invaders);
    TEST_ASSERT_EQUAL_STRING("spaceInvaders", invaders->name);
    TEST_ASSERT_NOT_NULL(invaders->idleScript);
    TEST_ASSERT_NOT_NULL(invaders->killScript);
    TEST_ASSERT_GREATER_THAN_UINT8(0U, invaders->actorCount);
    TEST_ASSERT_NOT_NULL(invaders->actors);
}

static void testAnimationRegistryFallback()
{
    /* Review focus: unknown or empty configuration values. */
    const AnimationDef* first = AnimationRegistry::getByIndex(0U);

    TEST_ASSERT_NOT_NULL(first);
    TEST_ASSERT_EQUAL_PTR(first, AnimationRegistry::getByName("doesNotExist"));
    TEST_ASSERT_EQUAL_PTR(first, AnimationRegistry::getByName(""));
    TEST_ASSERT_EQUAL_PTR(first, AnimationRegistry::getByName(nullptr));

    /* Out of range index is refused. */
    TEST_ASSERT_NULL(AnimationRegistry::getByIndex(AnimationRegistry::getCount()));

    /* Random always delivers something valid. */
    TEST_ASSERT_NOT_NULL(AnimationRegistry::getRandom());
}

static void testSpaceInvadersScriptsTerminate()
{
    /* Both scripts must end with STEP_END, otherwise the runner reads beyond
     * the array.
     */
    const AnimationDef* invaders = AnimationRegistry::getByName("spaceInvaders");
    uint8_t             index    = 0U;
    bool                hasEnd   = false;

    while ((64U > index) && (false == hasEnd))
    {
        if (STEP_END == invaders->killScript[index].type)
        {
            hasEnd = true;
        }

        ++index;
    }

    TEST_ASSERT_TRUE(hasEnd);

    index  = 0U;
    hasEnd = false;

    while ((64U > index) && (false == hasEnd))
    {
        if (STEP_END == invaders->idleScript[index].type)
        {
            hasEnd = true;
        }

        ++index;
    }

    TEST_ASSERT_TRUE(hasEnd);
}
```

`RUN_TEST` fuer die drei Funktionen eintragen.

- [ ] **Step 2: Test laufen lassen und Fehlschlag bestaetigen**

Run: `platformio test --environment test --filter test_RetroTime`
Expected: FAIL, `AnimationRegistry.h: No such file or directory`.

- [ ] **Step 3: Sprites von Space Invaders anlegen**

In `SpaceInvaders.cpp`:

- Held (Geschuetz), 11x8 Pixel, klassische Form mit Lauf in der Mitte.
- Schuss, 1x4 Pixel, alle Pixel gesetzt.
- Gegnergruppe, 40x8 Pixel: vier Gegner zu je 8x8 mit 2 Pixel Abstand, zwei Frames mit unterschiedlicher Armstellung. Die Gruppe ist bewusst ein Sprite, damit sie mit einem einzigen `PATROL_X` marschiert.

Jede Zeile mit Pixelmuster-Kommentar wie in Aufgabe 8.

- [ ] **Step 4: Actors und Scripts als Daten anlegen**

```cpp
/** Actor indices of the space invaders animation. */
enum ActorIndex
{
    ACTOR_HERO = 0, /**< The cannon at the bottom. */
    ACTOR_SHOT,     /**< The projectile. */
    ACTOR_FOES,     /**< The marching enemy row. */
    ACTOR_COUNT     /**< Number of actors. */
};

/** Animation ids of the space invaders animation. */
enum AnimIndex
{
    ANIM_HERO = 0, /**< Cannon, single frame. */
    ANIM_SHOT,     /**< Projectile, single frame. */
    ANIM_FOES,     /**< Enemy row, two frames. */
    ANIM_COUNT     /**< Number of animations. */
};

/** Initial actor setup. */
static const ActorDef ACTORS[ACTOR_COUNT] = {
    /*  x,  y, animId,     colorRaw,           isVisible */
    {   2, 55, ANIM_HERO,  ColorDef::GREEN,    true  },
    {   0,  0, ANIM_SHOT,  ColorDef::WHITE,    false },
    {   4, 36, ANIM_FOES,  ColorDef::CYAN,     true  }
};

/** Kill script: shoot the digit which is currently targeted. */
static const Step KILL_SCRIPT[] = {
    { STEP_MOVE_TO_TARGET_X, ACTOR_HERO, 40, EASING_OUT, 0, 0 },
    { STEP_WAIT, 120, 0, 0, 0, 0 },
    { STEP_SPAWN_AT, ACTOR_SHOT, ACTOR_HERO, 5, -4, 0 },
    { STEP_SHOW, ACTOR_SHOT, 1, 0, 0, 0 },
    { STEP_EMIT, ACTOR_HERO, PARTICLE_STYLE_SPARK, 4, 0, 0 },
    { STEP_LAUNCH_TO_TARGET, ACTOR_SHOT, 90, 0, 0, 0 },
    { STEP_AWAIT_IMPACT, ACTOR_SHOT, 0, 0, 0, 0 },
    { STEP_SHOW, ACTOR_SHOT, 0, 0, 0, 0 },
    { STEP_EXPLODE_TARGET, PARTICLE_STYLE_BLAST, 0, 0, 0, 0 },
    { STEP_WAIT, 250, 0, 0, 0, 0 },
    { STEP_COMMIT_TARGET, 0, 0, 0, 0, 0 },
    { STEP_SPAWN_TARGET, SPAWN_STYLE_FLASH_IN, 0, 0, 0, 0 },
    { STEP_WAIT, 150, 0, 0, 0, 0 },
    { STEP_END, 0, 0, 0, 0, 0 }
};

/** Idle script: the show between two minute changes. */
static const Step IDLE_SCRIPT[] = {
    { STEP_SET_ANIM, ACTOR_FOES, ANIM_FOES, 0, 0, 0 },
    { STEP_SHOW, ACTOR_FOES, 1, 0, 0, 0 },
    { STEP_PATROL_X, ACTOR_FOES, 4, 20, 10, 0 },
    { STEP_PATROL_X, ACTOR_HERO, 2, 51, 18, 0 },
    { STEP_WAIT, 3000, 0, 0, 0, 0 },
    { STEP_SPAWN_AT, ACTOR_SHOT, ACTOR_HERO, 5, -4, 0 },
    { STEP_SHOW, ACTOR_SHOT, 1, 0, 0, 0 },
    { STEP_LAUNCH_TO, ACTOR_SHOT, LAUNCH_X_SELF, 32, 90, 0 },
    { STEP_WAIT, 600, 0, 0, 0, 0 },
    { STEP_SHOW, ACTOR_SHOT, 0, 0, 0, 0 },
    { STEP_WAIT, 2500, 0, 0, 0, 0 },
    { STEP_END, 0, 0, 0, 0, 0 }
};
```

- [ ] **Step 5: `AnimationRegistry` implementieren**

Statische Tabelle im `.cpp`:

```cpp
/** All compiled in animations. The first one is the fallback. */
static const AnimationDef ANIMATIONS[] = {
    SpaceInvaders::getDefinition()
};
```

`getRandom()` nutzt `random()` beziehungsweise auf der nativen Plattform `rand()`; bei genau einer Animation wird diese geliefert.

- [ ] **Step 6: Die Leerlauf-Show in der View starten**

In `RetroTimeView64x64`:

- `setAnimation(const AnimationDef* def)` legt die Anim-Tabelle und die Actor-Startwerte in die Stage und merkt sich `def`.
- `init()` startet nach `setAnimation()` das Idle-Script mit `m_runner.start(def->idleScript, m_stage, true)`.
- `update(YAGfx& gfx)` ruft vor dem Zeichnen `m_runner.update(deltaMs)` und `m_stage.update(deltaMs)`.

Das Plugin uebergibt in `start()` beziehungsweise bei einer Konfigurationsaenderung die Animation aus der Registry: `AnimationRegistry::getByName(m_animationName.c_str())`, bei `"random"` stattdessen `AnimationRegistry::getRandom()` bei jedem `active()`.

- [ ] **Step 7: Tests laufen lassen und Erfolg bestaetigen**

Run: `platformio test --environment test --filter test_RetroTime`
Expected: PASS.

- [ ] **Step 8: Sichtprobe**

```bash
platformio run --environment native-64x64
```

Erwartet: Das Geschuetz patrouilliert unten, die Gegnerreihe marschiert im Band darueber, alle paar Sekunden ein Schuss ins Leere. Die Uhrzeit bleibt unveraendert stehen.

- [ ] **Step 9: Formatieren und committen**

```bash
clang-format -i lib/RetroTimePlugin/src/internal/anim/SpaceInvaders.h lib/RetroTimePlugin/src/internal/anim/SpaceInvaders.cpp lib/RetroTimePlugin/src/internal/AnimationRegistry.h lib/RetroTimePlugin/src/internal/AnimationRegistry.cpp lib/RetroTimePlugin/src/internal/RetroTimeView64x64.h lib/RetroTimePlugin/src/internal/RetroTimeView64x64.cpp test/test_RetroTime/TestRetroTime.cpp
git add lib/RetroTimePlugin test/test_RetroTime
git commit -m @'
feat(RetroTimePlugin): add space invaders animation data and registry

Co-Authored-By: Claude Opus 5 (1M context) <noreply@anthropic.com>
'@
```

---

## Aufgabe 10: Kill-Sequenz, Zustandsautomat und Demo-Modus

**Files:**
- Modify: `lib/RetroTimePlugin/src/internal/RetroTimeView64x64.h`, `.cpp`
- Modify: `lib/RetroTimePlugin/src/RetroTimePlugin.h`, `.cpp`

**Interfaces:**
- Consumes: `TimeSequencer` (Aufgabe 6), `DigitField` (Aufgabe 8), `AnimationRegistry` (Aufgabe 9), `ScriptRunner` (Aufgabe 5).
- Produces:
  - `RetroTimeView64x64::setTime(const String& time)` verarbeitet die Zeit ueber den `TimeSequencer` und stoesst die Sequenz an.
  - `RetroTimeView64x64::onActivated()` synchronisiert hart und setzt den Sequencer zurueck.

- [ ] **Step 1: Den Zustandsautomaten in der View ergaenzen**

```cpp
/** State of the view. */
enum ViewState
{
    VIEW_STATE_IDLE = 0, /**< Idle show is running. */
    VIEW_STATE_KILL      /**< Working through the queue of changed digits. */
};
```

`setTime()`:

```cpp
void RetroTimeView64x64::setTime(const String& time)
{
    const TimeSequencer::Decision decision = m_sequencer.feed(time.c_str());

    if (TimeSequencer::DECISION_NONE == decision)
    {
        /* Guard path: nothing changed. */
    }
    else if (TimeSequencer::DECISION_SNAP == decision)
    {
        m_digitField.setTimeHard(time.c_str());
        startIdle();
    }
    else
    {
        m_digitField.setTimePending(time.c_str());
        m_state = VIEW_STATE_KILL;
        startNextKill();
    }
}
```

`startNextKill()` holt den naechsten Ziffernindex aus dem Sequencer. Ist die Warteschlange leer, wird `startIdle()` aufgerufen und `m_state` auf `VIEW_STATE_IDLE` gesetzt. Sonst wird `m_stage.setTarget(index)` gesetzt und `m_runner.start(m_animation->killScript, m_stage, false)`.

In `update()` wird nach `m_runner.update(deltaMs)` geprueft: Ist `m_state` gleich `VIEW_STATE_KILL` und `true == m_runner.isFinished()`, dann `startNextKill()`.

`onActivated()` ruft `m_sequencer.reset()`, `m_digitField.setTimeHard(...)` mit der zuletzt bekannten Zeit und `startIdle()`. Das ist die Umsetzung der Zusage aus dem Review-Fokus, dass ein mitten in der Sequenz inaktiv gewordener Slot keine veraltete Ziffer animiert.

- [ ] **Step 2: Plugin-Lebenszyklus verdrahten**

- `active(YAGfx& gfx)` ruft `m_view.onActivated()` und, bei `"random"` als Animationsname, vorher `m_view.setAnimation(AnimationRegistry::getRandom())`.
- `inactive()` ruft `m_view.onDeactivated()`, das den Runner stoppt und den Sequencer zuruecksetzt.
- `setConfiguration()` setzt bei geaendertem Animationsnamen `m_view.setAnimation(AnimationRegistry::getByName(...))`.

- [ ] **Step 3: Demo-Modus ergaenzen**

In `RetroTimePlugin.cpp`, in der Funktion, die die Uhrzeit ermittelt:

```cpp
#if (0 != CONFIG_RETROTIME_FAST_DEMO)

    /* Development aid: advance the shown time by one minute every few seconds,
     * so that the animation does not have to be waited for.
     */
    timeInfo.tm_min += static_cast<int>(m_demoMinuteOffset);
    (void)mktime(&timeInfo);

#endif /* (0 != CONFIG_RETROTIME_FAST_DEMO) */
```

mit `m_demoMinuteOffset`, das im Sekundentakt alle `DEMO_PERIOD_S = 4U` Sekunden um eins erhoeht wird. Der gesamte Block steht hinter dem Compile-Switch; in `RetroTimePlugin.h` wird der Switch mit

```cpp
#ifndef CONFIG_RETROTIME_FAST_DEMO

/** Fast demo mode is disabled by default. */
#define CONFIG_RETROTIME_FAST_DEMO 0

#endif /* CONFIG_RETROTIME_FAST_DEMO */
```

in der Sektion "Compiler Switches" vorbelegt, damit alle anderen Umgebungen unveraendert bauen.

- [ ] **Step 4: Tests laufen lassen**

Run: `platformio test --environment test`
Expected: PASS, alle Tests beider Test-Ordner.

- [ ] **Step 5: Sichtprobe mit Demo-Modus**

```bash
platformio run --environment native-64x64
```

Erwartet, alle vier Sekunden: Das Geschuetz unterbricht die Patrouille, faehrt unter die geaenderte Ziffer, schiesst, die Ziffer zerfaellt in ihre eigenen Pixel, die neue blitzt auf, danach laeuft die Leerlauf-Show weiter. Beim Uebergang von `:59` auf `:00` werden nacheinander mehrere Ziffern abgeschossen.

- [ ] **Step 6: Formatieren und committen**

```bash
clang-format -i lib/RetroTimePlugin/src/RetroTimePlugin.h lib/RetroTimePlugin/src/RetroTimePlugin.cpp lib/RetroTimePlugin/src/internal/RetroTimeView64x64.h lib/RetroTimePlugin/src/internal/RetroTimeView64x64.cpp
git add lib/RetroTimePlugin
git commit -m @'
feat(RetroTimePlugin): shoot changed digits on minute change

Co-Authored-By: Claude Opus 5 (1M context) <noreply@anthropic.com>
'@
```

---

## Aufgabe 11: Web-UI, Dokumentation und Abschlusspruefung

**Files:**
- Create: `lib/RetroTimePlugin/web/RetroTimePlugin.html`, `lib/RetroTimePlugin/web/RetroTimePlugin.jpg`
- Modify: `doc/PLUGINS.md`

**Interfaces:**
- Consumes: Config-Topic aus Aufgabe 7.
- Produces: nichts, was weitere Aufgaben nutzen.

- [ ] **Step 1: Web-UI-Fragment schreiben**

`lib/DateTimePlugin/web/DateTimePlugin.html` als Vorlage nehmen und daran ausrichten. Formularfelder:

- Auswahlfeld `animation` mit den Eintraegen `spaceInvaders` und `random`.
- Textfeld `dateFormat` mit Vorbelegung `%d.%m.%Y`.
- Textfeld `timeZone`, leer erlaubt.

Die Seite liest und schreibt das Topic `/rest/api/v1/plugin/<uid>/config` genau wie die Vorlage.

- [ ] **Step 2: Screenshot aufnehmen**

Aus der nativen 64x64-Simulation einen Screenshot waehrend der Abschusssequenz aufnehmen und als `lib/RetroTimePlugin/web/RetroTimePlugin.jpg` ablegen.

- [ ] **Step 3: `doc/PLUGINS.md` ergaenzen**

Abschnitt in der bestehenden Struktur, mit Screenshot, Beschreibung, der Angabe "nur fuer 64x64-Displays", einem Beispiel fuer das Config-Topic und dem Hinweis, dass weitere Animationen ueber die `AnimationRegistry` hinzukommen.

- [ ] **Step 4: Flash-Bedarf messen**

```bash
platformio run --environment adafruit_matrixportal_s3-HUB75-64x64
```

Den ausgewiesenen Flash-Verbrauch mit einem Build ohne das Plugin vergleichen (Eintrag in `config/configNormal.ini` auskommentieren, bauen, wieder aktivieren). Das Ergebnis im Abschnitt "Offene Punkte" des Specs festhalten und dort entscheiden, ob `configTiny.ini` das Plugin aufnehmen kann.

- [ ] **Step 5: Alle Pflichtlaeufe**

```bash
clang-format --dry-run -Werror $(find lib/RetroGfx lib/RetroTimePlugin test/test_RetroGfx test/test_RetroTime -name '*.h' -o -name '*.hpp' -o -name '*.cpp')
platformio test --environment test
platformio run --environment native-64x64
platformio run --environment esp32doit-devkit-v1-HUB75-64x64
platformio run --environment esp32doit-devkit-v1-LED-32x8
platformio check --environment esp32doit-devkit-v1-HUB75-64x64 --fail-on-defect=medium
```

Expected: alle ohne Fehler. Der 32x8-Build belegt den Generic-Fallback, der HUB75-Build die Zielplattform.

- [ ] **Step 6: Doxygen pruefen**

Den Doxygen-Lauf so ausfuehren, wie es der letzte Commit `fd55e56a "Fixed doxygen warnings."` tat, und sicherstellen, dass keine neuen Warnungen entstehen.

- [ ] **Step 7: Committen**

```bash
git add lib/RetroTimePlugin/web doc/PLUGINS.md doc/superpowers/specs
git commit -m @'
docs(RetroTimePlugin): add web ui, plugin documentation and size measurement

Co-Authored-By: Claude Opus 5 (1M context) <noreply@anthropic.com>
'@
```

---

## Selbstpruefung des Plans

**Abdeckung des Specs**

| Spec-Abschnitt | Aufgabe |
|---|---|
| 3.1 Ablage | 1, 6, 7 |
| 4 Layout 64x64 | 8 |
| 5 Sprite | 1 |
| 5 SpriteAnim | 2 |
| 5 Actor | 3 |
| 5 ParticleSystem | 4 |
| 5 ITargetField / DigitField | 5, 8 |
| 5 Stage | 5 |
| 6 Script-Vorrat | 5 |
| 6.3 Kill-Script Space Invaders | 9 |
| 6.4 Idle-Script | 9 |
| 7 Plugin und Ablaufsteuerung | 7, 10 |
| 7.2 Sonderfaelle | 6 (Logik), 10 (Verdrahtung) |
| 7.4 Konfiguration | 7, 9, 11 |
| 8 Build-Integration | 7, 11 |
| 9 Verifikation | 1-11, abschliessend 11 |

Der Generic-Fallback aus Spec Abschnitt 4 wird in Aufgabe 7 angelegt und in Aufgabe 7 Step 11 auf einem 32x8-Board nachweislich gebaut.

**Typ-Konsistenz**

Die in Aufgabe 5 definierten Namen `getTargetCount`, `getTargetBounds`, `explodeTarget`, `commitTarget`, `spawnTarget`, `isTargetBusy` werden in Aufgabe 8 von `DigitField` unveraendert implementiert und in Aufgabe 9 von den Scripts unveraendert angesprochen. `ParticleStyle` und `SpawnStyle` stammen aus Aufgabe 4 beziehungsweise 5 und werden in den Scripts der Aufgabe 9 als `int16_t`-Parameter uebergeben; die Umwandlung geschieht im `ScriptRunner` aus Aufgabe 5.

**Review-Fokus-Abdeckung**

| Fall | Test | Aufgabe |
|---|---|---|
| Keine Zeit verfuegbar | `testTimeSequencerNoTimeAvailable`, `testDigitFieldNoTime` | 6, 8 |
| Partikel-Pool erschoepft | `testParticlePoolExhausted` | 4 |
| Extreme Zeitschritte | `testActorExtremeTimeStep`, `testActorTimeStepIndependence`, `testScriptRunnerExtremeTimeStep` | 3, 5 |
| Slot mitten in der Sequenz inaktiv | `testTimeSequencerResetDropsQueue` | 6 |
| Unbekannter Animationsname | `testAnimationRegistryFallback` | 9 |

Zusaetzlich deckt `testSpriteClipping` in Aufgabe 1 das Zeichnen ueber den Displayrand hinaus ab, und `testSpaceInvadersScriptsTerminate` in Aufgabe 9 verhindert, dass ein Script ohne `STEP_END` den Runner ueber das Array hinaus lesen laesst.
