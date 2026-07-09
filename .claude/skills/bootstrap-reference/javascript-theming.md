# JavaScript API, Color Modes & Theming

Sources: [JavaScript](https://getbootstrap.com/docs/5.3/getting-started/javascript/), [Color modes](https://getbootstrap.com/docs/5.3/customize/color-modes/), [Customize](https://getbootstrap.com/docs/5.3/customize/overview/).

## JavaScript plugin API

Bootstrap 5 ships **no jQuery** — plugins are plain ES classes on the global `bootstrap` object (or importable from the `bootstrap` package).

### 1. Declarative (data attributes) — preferred

Toggle/configure without writing JS. All options use the **`data-bs-`** prefix (v4's `data-toggle` is gone).

```html
<button data-bs-toggle="modal" data-bs-target="#myModal">Open</button>
<button data-bs-dismiss="modal">Close</button>
<button data-bs-toggle="collapse" data-bs-target="#panel">Expand</button>
```
- Option names are kebab-case: `data-bs-backdrop`, `data-bs-custom-class` (not `customClass`).
- Complex config as JSON (≥5.2, marked experimental): `data-bs-config='{"delay":0,"animation":true}'`; individual `data-bs-*` attrs override it.

### 2. Programmatic (imperative)

```js
// Create (accepts a CSS selector or an element; second arg = options)
const modal = new bootstrap.Modal('#myModal', { keyboard: false });

// Retrieve an already-initialized instance (null if none) …
const inst = bootstrap.Modal.getInstance('#myModal');
// … or get-or-create in one call (best for "show this now")
bootstrap.Modal.getOrCreateInstance('#myModal').show();

// Methods (show/hide/toggle/dispose vary by component)
modal.hide();
modal.dispose();               // tear down; remove listeners/data
bootstrap.Modal.Default.keyboard = false; // change defaults globally
```
Methods are **asynchronous** (they return before CSS transitions finish); calls made while a component is transitioning are ignored.

### 3. Events

Naming: **`{action}.bs.{component}`**. Present-tense (`show`, `hide`) fire at the start and are cancelable with `preventDefault()`; past-participle (`shown`, `hidden`) fire after the transition completes.

```js
const el = document.querySelector('#myModal');
el.addEventListener('show.bs.modal', e => { /* cancelable */ });
el.addEventListener('shown.bs.modal', e => { /* fully open; e.relatedTarget = trigger */ });
```
Examples: `shown.bs.tab`, `slid.bs.carousel`, `hidden.bs.offcanvas`, `hide.bs.dropdown`.

### 4. Opt-in components & Popper

- **Tooltips and popovers are opt-in** (not auto-initialized, for performance) — you must construct them:
  ```js
  document.querySelectorAll('[data-bs-toggle="tooltip"]').forEach(el => new bootstrap.Tooltip(el));
  ```
- **Popper is required** for dropdowns, tooltips, popovers → use `bootstrap.bundle.min.js` (Popper included) or load Popper separately before `bootstrap.min.js`.

### 5. Sanitizer (XSS)

Tooltip/popover HTML is sanitized against `DefaultAllowList`. Extend it, or plug in DOMPurify:
```js
new bootstrap.Tooltip(el, { sanitizeFn: content => DOMPurify.sanitize(content) });
```
Set `sanitize: false` only for fully trusted content.

## Color modes (dark mode) — 5.3

Controlled by the **`data-bs-theme`** attribute; values **`light`** (default) or **`dark`**. No JS/library required for the styling itself.

```html
<html lang="en" data-bs-theme="dark">          <!-- whole page dark -->
<div class="dropdown" data-bs-theme="dark">…</div>  <!-- scope to one subtree -->
```
- **Scoping:** set it on `<html>` for the page, or on any element to theme just that subtree (e.g. a dark navbar on a light page). This replaces the deprecated `.navbar-dark`/`.navbar-light`.
- Everything built on Bootstrap's CSS variables adapts automatically (backgrounds, text, borders, subtle/emphasis colors).

**Toggle in JS** (Bootstrap ships no built-in picker — respect stored + OS preference):
```js
const setTheme = t => document.documentElement.setAttribute('data-bs-theme',
  t === 'auto' ? (matchMedia('(prefers-color-scheme: dark)').matches ? 'dark' : 'light') : t);
setTheme(localStorage.getItem('theme') ?? 'auto');
```

**Custom modes** are possible: define `[data-bs-theme="mytheme"] { --bs-body-bg: …; --bs-body-color: …; }` (Sass `color-mode()` mixin helps). Disable dark entirely with `$enable-dark-mode: false`.

## Theming & customization

Two layers, from lightest to deepest:

### CSS variables (runtime, no build step)

Bootstrap exposes `--bs-*` custom properties globally and per-component. Override them in your own stylesheet (loaded **after** Bootstrap) or inline:
```css
:root {
  --bs-primary: #6f42c1;              /* note: some component colors derive from Sass, not this var alone */
  --bs-body-font-family: "Inter", sans-serif;
  --bs-border-radius: .5rem;
}
.btn-primary { --bs-btn-bg: #6f42c1; --bs-btn-border-color: #6f42c1; }
```
Component-scoped vars (e.g. `--bs-btn-bg`, `--bs-card-spacer-y`) are the cleanest override points. Great for small tweaks and theming without recompiling.

### Sass (build time, full control) — recommended for real customization

Two rules that trip people up: override **scalar** variables (`$primary`, `$border-radius`) *before* `@import "variables"` (they use `!default`), but mutate **maps** (`$theme-colors`) *after* `variables`/`variables-dark` and *before* `maps` — the map doesn't exist yet until `variables` is imported, so a map-merge placed too early throws an "Undefined variable" compile error.

```scss
// 1. functions first
@import "bootstrap/scss/functions";

// 2. override SCALAR defaults BEFORE variables (they use !default)
$primary: #6f42c1;
$border-radius: .5rem;
$enable-shadows: true;

@import "bootstrap/scss/variables";
@import "bootstrap/scss/variables-dark";

// 3. mutate MAPS after variables/variables-dark, before maps
$theme-colors: map-merge($theme-colors, ("brand": #ff6600));  // adds .btn-brand, .text-brand, etc.

@import "bootstrap/scss/maps";
@import "bootstrap/scss/mixins";
@import "bootstrap/scss/root";

// 4. the utilities MAP is needed early; then reboot + only the components you use
@import "bootstrap/scss/utilities";
@import "bootstrap/scss/reboot";
@import "bootstrap/scss/type";
@import "bootstrap/scss/buttons";
// … other components you actually use …

// 5. generate the utility CLASSES last
@import "bootstrap/scss/utilities/api";
```
- Key global toggles: `$enable-rounded`, `$enable-shadows`, `$enable-gradients`, `$enable-dark-mode`, `$enable-negative-margins`, `$enable-rfs` (responsive font sizing), `$prefix` (change the `bs-` prefix), `$spacer` (base of the spacing scale), `$grid-breakpoints`, `$container-max-widths`, `$font-family-base`.
- Adding a color to `$theme-colors` generates the matching `.btn-*`, `.text-*`, `.bg-*`, `.alert-*`, `.border-*` etc.
- Cherry-picking imports (only the components you use) is the standard way to reduce CSS size.

### Order rule

**Scalar Sass overrides go before `@import "variables"`; map mutations go after `variables`/`variables-dark` (before `maps`); the `utilities` map imports early but `utilities/api` generates classes last. CSS-variable and plain-CSS overrides go after everything.** Never edit the compiled `bootstrap.min.css`.
