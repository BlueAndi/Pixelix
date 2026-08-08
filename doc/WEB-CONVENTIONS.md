# Web Frontend Conventions

Naming and structural conventions for the Pixelix web UI (HTML, CSS, JavaScript served from `data/` and generated from `lib/<Name>/web/`).

The goal is **consistency**. Most rules below simply codify the dominant pattern already in the codebase; where the codebase is inconsistent (notably element-ID prefixes), a single standard is chosen.

## Stack

- **HTML5**, semantic where practical (`<header>`, `<nav>`, `<main>`, `<footer>`, `<section>`).
- **Bootstrap 5.3** for layout, components and dark mode (`data-bs-theme="dark"` on `<html>`).
- **Vanilla JavaScript (ES2015+)** — no jQuery, no additional runtime libraries. Third-party libs are only used where unavoidable (Ace editor, JSZip/FileSaver for backup/restore).
- **JSON** for the REST API payloads.

## Naming

| Thing | Convention | Examples |
|-------|-----------|----------|
| Element `id` | `camelCase` | `pluginUid`, `currentBrightness`, `modalDialog`, `fileTree` |
| Generated `id` (per entity) | `camelCase` base + entity key/index, no separator before a numeric/uid suffix | `slotCard<uid>`, `setting<index>`, `mqtt<index>Broker`, `timer<index>Enabled` |
| **Interactive-control `id` (buttons)** | **`btn` prefix + `PascalCase` action** | `btnStore`, `btnRestart`, `btnSave`, `btnUndo`, `btnUpload`, `btnInstall`, `btnNextSlot`, `btnCoredumpRefresh` |
| Form control `name` | `camelCase`, matching the JSON key sent to REST | `ssid`, `passphrase`, `timeFormat`, `dayOnColor` |
| Custom CSS class | `kebab-case` | `slot-card`, `control-panel`, `icon-btn`, `tree-icon` |
| CSS state class | `kebab-case`, adjective | `active`, `disabled`, `drag-over`, `branch` |
| `data-*` attribute | `kebab-case` in HTML (spec-mandated), read as `el.dataset.camelCase` | `data-full-path` → `el.dataset.fullPath` |
| Bootstrap hooks | leave verbatim | `data-bs-toggle`, `data-bs-target`, `data-bs-theme` |
| JS variables/functions | `camelCase`; classes `PascalCase`; module-level constants `UPPER_SNAKE_CASE` | `restClient`, `getConfig()`, `class Client {}`, `DISPLAY_WIDTH` |
| App page file | `lowercase.html` | `about.html`, `settings.html`, `index.html` |
| Plugin/Service page file | `PascalCase.html` (matches the C++ class name) | `BatteryPlugin.html`, `MqttService.html` |
| JS/CSS asset file | `kebab-case` | `app-api.js`, `app-menu.js`, `btn-custom.css` |

### Element-ID prefix rule (the one to enforce)

Use **`btn` + PascalCase** for every button/clickable that needs an `id`. Do **not** use the older mixed forms found in the codebase (`buttonSave`, `bInstall`, `bNextSlot`) for new code — migrate them opportunistically when a file is touched.

```html
<!-- Good -->
<button id="btnRestart" class="btn btn-light" type="button">Restart</button>

<!-- Avoid (legacy) -->
<button id="bRestart"    ...>   <!-- single-letter prefix: not searchable -->
<button id="buttonRestart" ...> <!-- inconsistent prefix -->
```

## Structural rules

- **IDs are unique per document** and semantic: prefer `pluginUid` over positional `select1`. Add a control-type suffix only to disambiguate a label/control pair (`pluginUidSelect`), not by default.
- **State lives in `data-*`, not parsed out of `id` strings.** Store lookup/state as `data-slot-id`, `data-full-path`, `data-loaded` and read via `dataset`. Keep the `id` a stable identifier.
- **No inline event handlers.** No `onclick=`, `onchange=`, `oninput=`, or `action="javascript:..."`. Give the element an `id` (or a class/selector) and attach the listener in JavaScript inside `DOMContentLoaded`.
- **No inline behavior-carrying attributes generated from JS** either — build elements with `document.createElement` and `addEventListener`, not `.setAttribute("onclick", ...)`.
- **Every form control has a programmatic label** (`<label for="…">`, a wrapping `<label>`, or `aria-label`). `placeholder` is not a label.
- **Accessibility baseline:** one `<h1>` per page, meaningful heading ranks, `alt` on images (`alt=""` for decorative), visible focus, and `data-bs-theme` for dark mode (do not use the deprecated `navbar-dark`/`navbar-light`).

## JavaScript conventions

- `const` by default, `let` when reassigned; never `var` (except the shared `window`-namespace roots that must be redeclarable across classic scripts — see `app-api.js`/`ws.js`, where `var pixelix = window.pixelix || {}` is intentional).
- Prefer `async`/`await` over `.then()` chains. Reserve the `new Promise(...)` wrapper for event-based APIs (XHR upload progress, WebSocket, `FileReader`), which cannot use `await` directly.
- Use `document.getElementById` / `querySelector` / `classList` / `textContent` / `addEventListener`; avoid `innerHTML` for untrusted content.
- Attach page init via `document.addEventListener("DOMContentLoaded", () => { … })`.
- Suggested attribute order in markup (readability, not enforced): `class` → `id` → `type`/`name` → `value` → `data-*` → `aria-*` → boolean state (`disabled`, `checked`).

## Source vs. build output

- **Edit the sources**, not the build output:
  - Plugin/service pages: `lib/<Name>/web/<Name>.html` (copied verbatim to `data/plugins/` and `data/services/` by `scripts/configure_plugins.py` / `configure_services.py`).
  - Menu logic: `scripts/menu_base.js` (concatenated into the generated `data/js/app-menu.js`).
- `data/plugins/`, `data/services/` and `data/js/app-menu.js` are **generated** (git-ignored) and regenerated on `platformio run --target buildfs`. Changes made only there are lost on the next build.
- Shared, hand-maintained assets that ARE sources: `data/js/app-api.js`, `data/js/ws.js`, and the top-level app pages (`data/*.html`).
