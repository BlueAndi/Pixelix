# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Pixelix is ESP32 firmware for RGB LED matrix displays (WS2812B, HUB75) and TFT displays. It provides a multi-plugin display system controlled via REST API, MQTT, and a web dashboard, with Home Assistant integration.

## Build Commands

Uses PlatformIO. The default environment is `esp32doit-devkit-v1-LED-32x8`.

```bash
# Build firmware
platformio run --environment <env>

# Build filesystem image (LittleFS)
platformio run --environment <env> --target buildfs

# Upload firmware
platformio run --environment <env> --target upload

# Upload filesystem
platformio run --environment <env> --target uploadfs

# Run unit tests
platformio test --environment test

# Run static analysis
platformio check --environment <env> --fail-on-defect=medium

# Check clang-format compliance (dry run)
clang-format --dry-run --Werror <file>
```

Available environments are defined in [platformio.ini](platformio.ini) (15+ boards). Key ones:
- `esp32doit-devkit-v1-LED-32x8` — 32×8 LED matrix, ESP32 DevKit
- `esp32-4MB-LED-32x8` — 4 MB flash variant
- `ulanzi-TC001` — Ulanzi TC001 smart pixel clock
- `lilygo-t-display-s3-LED-32x8` — LILYGO T-Display S3

## Architecture

### Layers

1. **Application** (`src/`): State machine (`SystemState/`), display manager (`DisplayMgr/`), button handlers (`ButtonHandler/`), web handlers (`Web/`)
2. **Services** (`lib/*Service/`): Plugin manager, REST/MQTT APIs, audio, timers, settings, filesystem — each implements `IService`
3. **Plugins** (`lib/*Plugin/`): Display content units, each implements the plugin interface and exposes web UI assets
4. **HAL** (`lib/Hal*/`, `src/Hal/`): Board-specific drivers for LEDs, sensors, RTC, buttons
5. **Graphics** (`lib/BaseGfx/`, `lib/Widgets/`): Widget system (text, bitmap, lamp, progress bar) with fade effects

### Key Design Patterns

- **State machine**: System lifecycle in `src/SystemState/` — InitState, ErrorState, RestartState, etc.
- **Slot system**: `DisplayMgr` cycles through plugin slots with configurable durations and fade transitions
- **Topic routing**: Both REST and MQTT route through `TopicHandlerService` using a unified topic model
- **Code generation**: Build scripts in `scripts/` generate `src/Generated/PluginList.cpp`, `Services.cpp`, and `TopicHandlers.cpp` — do not edit these files manually
- **Dual-core tasks**: Display updates, audio processing, and WiFi run as separate FreeRTOS tasks

### Plugin System

Each plugin lives in `lib/<Name>Plugin/`. A plugin:
- Inherits from the plugin base class and registers via the generated `PluginList.cpp`
- Provides a web UI fragment in `data/plugins/<name>/`
- Optionally exposes REST/MQTT topics through `TopicHandlerService`

See [doc/PLUGIN-DEV.md](doc/PLUGIN-DEV.md) for the plugin development guide and boilerplate.

### Configuration Files

Build configuration is split across `config/`:
- `buildMode.ini` — Debug / Release / Trace
- `display.ini` — Display topology (matrix size, panel type)
- `board.ini` — Per-board settings
- `configNormal.ini`, `configSmall.ini`, etc. — Feature sets sized to flash budget

## Code Style

- **Formatter**: clang-format v18.1.3, config in [.clang-format](.clang-format). Enforced in CI; VSCode formats on save.
- **Indentation**: 4 spaces, no tabs
- **Line length**: No hard limit (ColumnLimit: 0)
- **Headers**: Every file starts with the MIT license block, `@file`, `@brief`, `@author` doxygen tags
- **Naming**: Plugins must end with `Plugin` suffix; services follow the `IService` interface
- **Python scripts**: max line length 140 (`.pylintrc`)

## Testing

Unit tests are in `test/test_*/` and use the PlatformIO Unit Testing framework. Shared stubs and mocks live in `test/stub/` and `test/common/`. Run all tests with `platformio test --environment test`.

## Documentation

Architecture diagrams (SVG) are in `doc/architecture/`. Key references:
- [doc/PLUGIN-DEV.md](doc/PLUGIN-DEV.md) — plugin development
- [doc/PLUGINS.md](doc/PLUGINS.md) — all plugins documented
- [doc/MQTT.md](doc/MQTT.md) / [doc/WEBSOCKET.md](doc/WEBSOCKET.md) — API docs
- [doc/HOMEASSISTANT.md](doc/HOMEASSISTANT.md) — Home Assistant integration
- [doc/COREDUMP-ANALYSIS.md](doc/COREDUMP-ANALYSIS.md) — crash debugging
