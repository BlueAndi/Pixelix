# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Pixelix is ESP32 firmware for RGB LED matrix displays (WS2812B, HUB75) and TFT displays. It provides a multi-plugin display system controlled via REST API, MQTT, and a web dashboard, with Home Assistant integration. Besides the ESP32 targets there is a `native` environment which runs the complete application on a Windows/Linux host (simulated LED grid in an SDL3 window, web interface at `http://localhost:8080`).

## Build Commands

Uses PlatformIO. The default environment is `adafruit_feather_esp32_v2-LED-32x8`. Environment names follow `<board>-<LED|HUB75|TFT>-<width>x<height>[-makapix]` and are listed in [platformio.ini](platformio.ini), e.g. `ulanzi-tc001-LED-32x8`, `lilygo-t-display-s3-TFT-32x8`, `esp32doit-devkit-v1-HUB75-64x64`.

```bash
# Build firmware / filesystem image (LittleFS)
platformio run --environment <env>
platformio run --environment <env> --target buildfs

# Upload firmware / filesystem
platformio run --environment <env> --target upload
platformio run --environment <env> --target uploadfs

# Build the native host simulator (needs SDL3)
platformio run --environment native

# Unit tests: all suites, or a single suite directory from test/
platformio test --environment test
platformio test --environment test --filter test_Color

# Static analysis (clang-tidy, CI threshold)
platformio check --environment <env> --fail-on-defect=medium --fail-on-defect=high

# Check clang-format compliance (dry run)
clang-format --dry-run --Werror <file>
```

On Windows `platformio check` exceeds the command line length limit for the ESP32 environments; use `python scripts/check_clangtidy.py --environment <env> --fail-on-defect=medium` instead, which runs clang-tidy per file with the same `config/checker.ini` configuration.

Other checks which CI runs (see [doc/WORKFLOWS.md](doc/WORKFLOWS.md) for the staged pipeline and its path filters):

```bash
# Python: lint and build-script tests (single test via node ID)
python -m pylint --rcfile=.pylintrc <file.py>
python -m pytest scripts/test_configure_common.py scripts/test_prepare_usb_installer.py scripts/test_native_release.py
python -m pytest scripts/test_prepare_usb_installer.py::test_assemble

# Home Assistant integration tests (run from doc/homeassistant/integration/, install requirements_test.txt first)
python -m pytest
python -m pytest tests/test_notify.py::test_only_message

# Web tag editor test
node --test scripts/test_app_tag_editor.js

# Web frontend lint: html-validate (markup) and ESLint (JavaScript incl. inline scripts), install once with "npm ci"
npm run lint

# OpenAPI specification and YAML
openapi-spec-validator doc/OpenAPI/OpenAPI.yaml
python -m yamllint -d '{rules: {key-duplicates: enable}}' <file.yml>
```

The USB installer in `usb-installer/` is built with `npm ci` followed by `npm run build`.

## Architecture

### Layers

1. **Application** (`src/`): Bootstrapping and system state machine (`SystemState/`), display manager (`DisplayMgr/`), button handlers (`ButtonHandler/`), web server, pages and websocket (`Web/`)
2. **Services** (`lib/*Service/`): Long-lived singletons implementing `IService` (settings, REST/MQTT, HTTP, audio, timers, file manager). The generated `Services` facade manages their lifecycle and processing.
3. **Plugins** (`lib/*Plugin/`): Display content units. `DisplayMgr` installs them into slots and schedules them with configurable durations and fade transitions. Each plugin has a unique alias and exposes topics through the shared topic model.
4. **HAL** (`lib/Hal*/`, `src/Hal/`): Board-specific drivers for displays, sensors, RTC, buttons; selected through the board configuration. The `*Native` libraries (`HalNative`, `ArduinoNative`, `AsyncTcpNative`, ...) provide the host counterparts for the `native` and `test` environments.
5. **Graphics** (`lib/YAGfx/`, `lib/YAWidgets/`, `lib/Views/`, `lib/FadeEffects/`): Graphics primitives, widgets (text, bitmap, lamp, progress bar), layout-dependent views and fade effects

### Key Design Patterns

- **Topic routing**: A plugin or service registers its topics once at `TopicHandlerService`; the topic handlers (`RestApiTopicHandler`, `MqttApiTopicHandler`) expose them over each transport.
- **Outgoing HTTP**: Uses `RestService` jobs polled through `HttpService`; do not reintroduce the old `AsyncHttpClient` callback flow. No blocking network or filesystem work in display updates.
- **Concurrency**: ESP32 execution is split across FreeRTOS tasks (display update, audio, WiFi/web). Plugin processing and web-server requests can access the same plugin state, so protect shared state at those boundaries.
- **Plugin lifecycle**: One-time setup and initial configuration in `start()`, periodic I/O and configuration handling in `process()`, and keep `update()` efficient and limited to drawing.

### Build Configuration and Code Generation

`platformio.ini` composes each environment from sections in `config/`: `mcu.ini`, `board.ini`, `display.ini` (topology, panel type), `buildMode.ini` (Debug / Release / Trace, selected via `[mode:selected]` in `platformio.ini`), `checker.ini` and one feature profile (`configNormal.ini`, `configSmall.ini`, `configTiny.ini`, `configNative.ini`, ...) sized to the flash budget. Keep these aligned when adding or changing build variants.

- `lib_ldf_mode = off`: dependencies must be declared explicitly in the relevant `library.json` and PlatformIO configuration; automatic library discovery is disabled.
- The feature profile's `lib_deps` list determines which services, topic handlers and plugins are compiled. The **order of services** determines the generated lifecycle order — preserve it. Plugins are listed alphabetically.
- The profile's pre-build script (`scripts/configure_*.py` → `scripts/configure.py`) generates from that list, all of it git-ignored — never edit these, edit the configuration or generator sources instead:
  - `src/Generated/PluginList.cpp`, `Services.cpp`, `TopicHandlers.cpp`
  - `data/plugins/`, `data/services/` (copied from `lib/<Name>/web/`)
  - `data/js/app-menu.js` (assembled from `scripts/menu_base.js` and generated sub-menus)
- Top-level app pages and shared frontend sources under `data/` are maintained directly. See [doc/WEB-CONVENTIONS.md](doc/WEB-CONVENTIONS.md).
- `scripts/app-tag-editor.js` has a committed compressed copy at `data/js/app-tag-editor.js.gz`. After changing the source, regenerate it from the repository root and run its consistency test:

  ```bash
  python -c "import gzip, pathlib; pathlib.Path('data/js/app-tag-editor.js.gz').write_bytes(gzip.compress(pathlib.Path('scripts/app-tag-editor.js').read_bytes(), mtime=0))"
  node --test scripts/test_app_tag_editor.js
  ```

### Adding a Plugin

Follow [doc/PLUGIN-DEV.md](doc/PLUGIN-DEV.md):

- Create `lib/<Name>Plugin/` (name must end with `Plugin`) with `src/`, `web/` (page and assets) and `library.json`.
- Add it to the relevant feature profiles in `config/` in alphabetical order.
- Document it in [doc/PLUGINS.md](doc/PLUGINS.md).

## Code Style

- **Formatter**: clang-format v18.1.3, config in [.clang-format](.clang-format). Enforced in CI for `src/`, `lib/` and `include/` (bundled imgui and libwebp excluded); VSCode formats on save.
- **C/C++**: C++17, 4 spaces, no tabs, no line length limit (ColumnLimit: 0)
- **Headers**: Every file starts with the MIT license block, `@file`, `@brief`, `@author` doxygen tags
- **Python scripts**: max line length 140 (`.pylintrc`); CI runs Pylint over all tracked Python files
- **Python skill**: For Python creation, changes, or review, use [python-coding-guidelines](.claude/skills/python-coding-guidelines/SKILL.md). For Home Assistant integration code (`doc/homeassistant/integration/`), also use [ha-integration-dev](.claude/skills/ha-integration-dev/SKILL.md).
- **C/C++ skill**: For writing/refactoring/reviewing embedded C/C++17 code (MISRA-oriented rules, defensive programming, Yoda conditions, single-exit "pathfinder" rule, Doxygen, section-banner layout, clang-format), use the [embedded-cpp-misra](.claude/skills/embedded-cpp-misra/SKILL.md) skill, which includes `.h`/`.hpp`/`.cpp` templates.
- **Web skills**: For the web frontend (`data/`, `lib/<Name>/web/`) use the `html5-reference`, `bootstrap-reference` and `javascript-reference` skills; for diagrams the `plantuml-reference` skill.

## Testing

Unit tests are in `test/test_*/` (one suite per directory, Unity framework) and run on the host in the `test` environment. Shared test helpers live in `test/common/`. A library must be listed in the `lib_deps` of `[env:test]` in [platformio.ini](platformio.ini) to be testable, because of `lib_ldf_mode = off`.

REST API behaviour is verified in CI by starting the native application and running Schemathesis against [doc/OpenAPI/OpenAPI.yaml](doc/OpenAPI/OpenAPI.yaml) — keep the specification in sync with REST API changes.

## Documentation

Architecture diagrams (SVG, PlantUML sources) are in `doc/architecture/`. Key references:

- [doc/README.md](doc/README.md) — documentation overview
- [doc/PLUGIN-DEV.md](doc/PLUGIN-DEV.md) / [doc/PLUGINS.md](doc/PLUGINS.md) — plugin development, all plugins documented
- [doc/WEB-CONVENTIONS.md](doc/WEB-CONVENTIONS.md) — web frontend sources vs. build output
- [doc/MQTT.md](doc/MQTT.md) / [doc/WEBSOCKET.md](doc/WEBSOCKET.md) — API docs
- [doc/HOMEASSISTANT.md](doc/HOMEASSISTANT.md) — Home Assistant integration
- [doc/config/](doc/config/README.md) — toolchain installation, software build and update
- [doc/boards/](doc/boards/README.md) — supported boards and displays
- [doc/WORKFLOWS.md](doc/WORKFLOWS.md) — CI and release workflows
- [doc/COREDUMP-ANALYSIS.md](doc/COREDUMP-ANALYSIS.md) — crash debugging
