# Project Guidelines

## Architecture
- Pixelix has ESP32 firmware environments and a native host simulator (`native`). `platformio.ini` composes board environments from MCU, display, and feature-profile settings in `config/`.
- The application in `src/` owns bootstrapping and the system state machine. Long-lived functionality is implemented as singleton `IService` services in `lib/`; the generated `Services` facade manages their lifecycle and processing.
- Display content is implemented as plugins in `lib/*Plugin/`. `DisplayMgr` installs plugins into slots and schedules them. Each plugin has a unique alias and exposes topics through the shared topic model.
- REST and MQTT both route through `TopicHandlerService`: a plugin or service registers its topics once, and the corresponding topic handlers expose them over each transport.
- The display driver and board-specific peripherals are selected through the HAL libraries and board configuration. Keep application, service, and HAL responsibilities in their existing layers; shared graphics and general libraries support those layers.
- Outgoing REST work uses `RestService` jobs polled through `HttpService`; do not reintroduce the old `AsyncHttpClient` callback flow.
- ESP32 execution is split across FreeRTOS tasks. Plugin processing and web-server requests can access the same plugin state, so protect shared state at those boundaries.

## Code and Asset Generation
- Build scripts generate `src/Generated/PluginList.cpp`, `Services.cpp`, and `TopicHandlers.cpp`; edit the configuration or generator sources, not generated C++.
- `platformio run --target buildfs` generates filesystem content, including plugin/service pages and the shared app menu. Edit source pages under `lib/<Name>/web/` and menu sources under `scripts/`, not generated `data/plugins/`, `data/services/`, or `data/js/app-menu.js`.
- The top-level app pages and shared frontend sources under `data/` are maintained directly. See `doc/WEB-CONVENTIONS.md` for source/build-output details.
- `scripts/app-tag-editor.js` has a committed compressed copy at `data/js/app-tag-editor.js.gz`; after changing the source, regenerate it from the repository root and run its consistency test:

  ```bash
  python -c "import gzip, pathlib; pathlib.Path('data/js/app-tag-editor.js.gz').write_bytes(gzip.compress(pathlib.Path('scripts/app-tag-editor.js').read_bytes(), mtime=0))"
  node --test scripts/test_app_tag_editor.js
  ```

## Build and Test

The default PlatformIO environment is `adafruit_feather_esp32_v2-LED-32x8`; available board environments are listed in `platformio.ini`.

```bash
# Build firmware or the filesystem image for one environment
platformio run --environment <env>
platformio run --environment <env> --target buildfs

# Build the native simulator
platformio run --environment native

# Run all PlatformIO unit-test suites, or select one suite directory
platformio test --environment test
platformio test --environment test --filter test_Color

# Static analysis (the CI severity threshold)
platformio check --environment <env> --fail-on-defect=medium --fail-on-defect=high

# Check formatting of a touched C/C++ file
clang-format --dry-run --Werror <file>
```

- Python utility tests live in `scripts/`; run them with `python -m pytest scripts/test_prepare_usb_installer.py scripts/test_native_release.py scripts/test_configure_common.py`. A single test can be selected by node ID, e.g. `python -m pytest scripts/test_prepare_usb_installer.py::test_assemble`.
- The app-tag-editor test is `node --test scripts/test_app_tag_editor.js`. The USB installer is built from `usb-installer/` with `npm ci` followed by `npm run build`.
- Home Assistant integration tests run from `doc/homeassistant/integration/`: install `requirements_test.txt`, then run `python -m pytest`. A single test example is `python -m pytest tests/test_notify.py::test_only_message`.
- Python linting uses Pylint and `.pylintrc`: `python -m pylint --rcfile=.pylintrc <file.py>`. CI also validates `doc/OpenAPI/OpenAPI.yaml` with `openapi-spec-validator` and YAML with `yamllint`.
- Run those documentation checks directly with `openapi-spec-validator doc/OpenAPI/OpenAPI.yaml` and `python -m yamllint -d '{rules: {key-duplicates: enable}}' <file.yml`.
- Toolchain and build references: `doc/config/TOOLCHAIN-INSTALLATION.md`, `doc/config/SW-BUILD.md`, and `doc/config/SW-UPDATE.md`.

## Conventions
- Keep board, MCU, display, and feature-profile declarations in `platformio.ini` and `config/*.ini` aligned when adding or changing build variants. Feature profiles control which services and plugins are compiled for a flash-size class.
- `lib_ldf_mode = off`: dependencies must be declared explicitly in the relevant `library.json` and PlatformIO configuration; do not rely on automatic library discovery.
- The order of services in a feature profile determines generated lifecycle order; preserve the existing order when changing the service list.
- If you add a plugin:
  - Follow `doc/PLUGIN-DEV.md`: use a `*Plugin` name and provide `src/`, `web/`, and `library.json`.
  - Add it to the relevant feature profiles in `config/` in alphabetical order, and document it in `doc/PLUGINS.md`.
  - Keep its web page and assets under `lib/<PluginName>/web/`.
- For plugins, put one-time setup and initial configuration work in `start()`, periodic I/O and configuration handling in `process()`, and keep `update()` efficient and limited to display updates.
- Outgoing network requests belong in the established REST/HTTP services; avoid blocking network or filesystem work in display updates.
- Keep C/C++ file headers, Doxygen tags, and formatting consistent with adjacent code. The repository formatter is configured by `.clang-format`; CI formats C/C++ under `src/`, `lib/`, and `include/`, excluding bundled imgui and libwebp.
- C/C++ is built as C++17, uses four-space indentation, and file headers include the MIT notice and Doxygen `@file`, `@brief`, and `@author` tags.
- Python conventions are enforced by `.pylintrc` (including a 140-character line limit).
- Home Assistant Python lives in `doc/homeassistant/integration/`; its test environment and requirements are separate from the firmware tests.

## Documentation Map
- Core overview and setup: `README.md`, `doc/README.md`
- Architecture: `doc/architecture/README.md`
- Plugin development: `doc/PLUGIN-DEV.md`, `doc/PLUGINS.md`
- Web frontend conventions: `doc/WEB-CONVENTIONS.md`
- APIs and integrations: `doc/MQTT.md`, `doc/WEBSOCKET.md`, `doc/HOMEASSISTANT.md`, `doc/SENSORS.md`
- Board and config references: `doc/boards/README.md`, `doc/config/README.md`, `doc/grabConfigs/README.md`

## Copilot Skills
- For Python creation, changes, or review, read `.github/skills/python-coding-guidelines/SKILL.md`. For Home Assistant integration code, also read `.claude/skills/ha-integration-dev/SKILL.md`.
- For embedded C/C++ firmware work in `src/`, `lib/`, or `test/`, read `.github/skills/embedded-cpp-misra/SKILL.md`.
- For HTML and Bootstrap UI work, read `.github/skills/html5-bootstrap/SKILL.md`.
- For JavaScript work in `data/` or frontend assets, read `.github/skills/javascript-coding-guidelines/SKILL.md`.
- For OpenAPI specification work, read `.github/skills/openapi-spec/SKILL.md`.
- For OpenSCAD mechanical design work, read `.github/skills/openscad-mechanical/SKILL.md`.
- For PlantUML diagram work, read `.github/skills/plantuml-diagrams/SKILL.md`.
