# Project Guidelines

## Code Style
- Follow existing C/C++ style in `src/` and `lib/`: keep current naming, spacing, and brace conventions.
- Keep file headers and Doxygen-style comments consistent with neighboring files when editing existing modules.
- Prefer small, focused changes; avoid reformatting unrelated code.
- For new plugins, follow `doc/PLUGIN-DEV.md` (name must end with `Plugin`, required `src/` and `web/` layout).

## Architecture
- Treat the codebase as layered: application logic, services, and HAL. See `doc/architecture/README.md`.
- `src/main.cpp` owns bootstrapping, state machine processing, watchdog handling, and terminal processing.
- Most features are plugin- or service-based under `lib/`; preserve boundaries instead of adding cross-module shortcuts.
- Network request flow in RestService uses HttpService job polling (not old AsyncHttpClient callback flow).

## Build and Test
- Toolchain and build docs:
  - `doc/config/TOOLCHAIN-INSTALLATION.md`
  - `doc/config/SW-BUILD.md`
  - `doc/config/SW-UPDATE.md`
- Common local commands:
  - Build firmware: `platformio run --environment <env>`
  - Build filesystem image: `platformio run --environment <env> --target buildfs`
  - Clean env: `platformio run --target clean --environment <env>`
  - Run tests: `platformio test --environment test`
  - Static analysis: `platformio check --environment <env> --fail-on-defect=medium --fail-on-defect=high`
- CI also enforces formatting on C/C++ files with clang-format dry-run. Match existing formatting in touched files.

## Conventions
- Keep `platformio.ini` and `config/*.ini` aligned when introducing board/feature changes.
- If you add a plugin:
  - Register/build-enable it in relevant config files under `config/` (alphabetical order where applicable).
  - Add or update user docs in `doc/PLUGINS.md`.
  - Include plugin web assets in `lib/<PluginName>/web`.
- Web UI/static resources live under `data/`; avoid duplicating plugin-specific assets there unless intentionally shared.

## Documentation Map
- Core overview and setup: `README.md`, `doc/README.md`
- Architecture: `doc/architecture/README.md`
- Plugin development: `doc/PLUGIN-DEV.md`, `doc/PLUGINS.md`
- APIs and integrations: `doc/MQTT.md`, `doc/WEBSOCKET.md`, `doc/HOMEASSISTANT.md`, `doc/SENSORS.md`
- Board and config references: `doc/boards/README.md`, `doc/config/README.md`, `doc/grabConfigs/README.md`
