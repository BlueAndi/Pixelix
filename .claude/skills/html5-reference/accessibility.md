# Accessibility & HTML-defined APIs

## Part 1 — Accessible authoring

HTML's semantics *are* the accessibility layer: native elements expose the correct role, name, states, and keyboard behavior to assistive technology for free. Related spec: [ARIA in HTML](https://www.w3.org/TR/html-aria/) · [WAI-ARIA](https://www.w3.org/TR/wai-aria-1.2/).

### First rule of ARIA: don't use ARIA
Prefer a native element over a `<div>` + `role`. A real `<button>` is focusable, keyboard-operable, and announced correctly; `<div role="button">` requires you to re-add `tabindex="0"`, key handlers, and states by hand.

> "No ARIA is better than bad ARIA." Only add ARIA to fill a gap native HTML can't express, and never contradict native semantics (`<button role="heading">` is broken).

### Landmarks (navigation for screen-reader users)
Native elements map to landmark roles automatically:

| Element | Landmark role |
|---------|---------------|
| `header` (top-level) | `banner` |
| `nav` | `navigation` |
| `main` | `main` |
| `aside` | `complementary` |
| `footer` (top-level) | `contentinfo` |
| `section` with accessible name | `region` |
| `form` with accessible name | `form` |
| `search` | `search` |

Give repeated landmarks distinct names via `aria-label`/`aria-labelledby` (e.g. two `<nav aria-label="Primary">` / `aria-label="Footer">`).

### Accessible names & descriptions
- Interactive elements need an accessible name: visible text, `alt` (images), `<label>` (form controls), or `aria-label`/`aria-labelledby`.
- `aria-describedby` adds supplementary description (hint/error text) by id.
- Accessible name computation order (simplified): `aria-labelledby` → `aria-label` → native (label/`alt`/`caption`) → `title`.

### Common ARIA states/properties
`aria-expanded`, `aria-current`, `aria-selected`, `aria-checked`, `aria-disabled`, `aria-hidden` (removes from a11y tree — never on a focusable element), `aria-live` (`polite`/`assertive` for dynamic regions), `aria-controls`, `aria-haspopup`, `aria-invalid`, `aria-required`, `aria-pressed`, `aria-busy`.

### Keyboard & focus
- Everything operable by mouse must be operable by keyboard.
- `tabindex="0"` adds an element to the natural tab order; `tabindex="-1"` makes it focusable only programmatically; avoid positive `tabindex`.
- Manage focus for dynamic UI (move focus into an opened `<dialog>`; return it on close — `showModal()` handles the focus trap).
- Visible focus indicators are required (don't `outline: none` without a replacement).

### Images & media
- Informative `<img>`: meaningful `alt`. Decorative: `alt=""` (empty, not omitted). Complex: describe nearby or via `aria-describedby`.
- Provide `<track kind="captions">` for video; transcripts for audio.
- Respect `prefers-reduced-motion`; don't autoplay audio.

### Quick a11y checklist
- One `<h1>`; heading ranks not skipped for styling.
- Language set (`<html lang>`); direction where needed (`dir`).
- Color contrast ≥ 4.5:1 (normal text); never encode meaning in color alone.
- All controls labeled; errors linked via `aria-describedby`.
- Landmarks present and uniquely named.
- Fully keyboard-navigable with a visible focus ring.

## Part 2 — HTML-defined JavaScript APIs

The spec defines both markup and the DOM interfaces these elements expose. Highlights:

### Canvas — `<canvas>`
```js
const ctx = canvas.getContext('2d'); // or 'webgl2', 'bitmaprenderer'
ctx.fillRect(0, 0, 100, 100);
```
Immediate-mode bitmap drawing; also `OffscreenCanvas` for workers. Provide fallback content and an accessible alternative — canvas pixels are opaque to AT.

### Media — `<audio>` / `<video>`
`HTMLMediaElement`: `.play()`/`.pause()`, `.currentTime`, `.volume`, `.muted`, `.playbackRate`, events `loadedmetadata`/`timeupdate`/`ended`. Multiple `<source>` for format fallback; `<track>` for captions (`TextTrack` API).

### Dialog — `<dialog>`
```js
dlg.showModal();  // modal: backdrop, focus trap, top layer, Esc to close
dlg.show();       // non-modal
dlg.close(returnValue);
dlg.addEventListener('close', () => dlg.returnValue);
```
A `<form method="dialog">` inside closes the dialog on submit and sets `returnValue`.

### Template & shadow DOM — `<template>`, `<slot>`
```js
const frag = tpl.content.cloneNode(true); // inert until cloned
host.attachShadow({ mode: 'open' });      // encapsulated shadow tree with <slot>s
```
Declarative shadow DOM: `<template shadowrootmode="open">`.

### Custom elements
```js
class MyWidget extends HTMLElement {
  static observedAttributes = ['label'];
  connectedCallback() { /* mounted */ }
  attributeChangedCallback(name, old, val) { /* react */ }
}
customElements.define('my-widget', MyWidget); // name MUST contain a hyphen
```
Customized built-ins: `class FancyButton extends HTMLButtonElement` + `define('fancy-button', FancyButton, { extends: 'button' })`, used as `<button is="fancy-button">`.

### Popover API
`<div popover id="menu">` + `<button popovertarget="menu">`; or JS `el.showPopover()`/`hidePopover()`/`togglePopover()`. Uses the top layer and light-dismiss.

### Drag & drop
`draggable="true"` + `dragstart`/`dragover`/`drop` events and the `DataTransfer` object.

### Storage & offline
- `localStorage` / `sessionStorage` (synchronous key/value strings).
- `history.pushState()`/`replaceState()` + `popstate` for client-side routing.
- Service Workers + Cache API for offline (replaced the removed AppCache).

### Editing & forms
- `contenteditable` + the Selection/Range APIs and `execCommand` (legacy) / newer editing primitives.
- Constraint Validation API — see [forms.md](forms.md).

### Microdata
`itemscope`/`itemtype`/`itemprop`/`itemid`/`itemref` expose a structured `document.getItems()` model for machine-readable metadata (alternative to JSON-LD).

For full interface definitions, consult the spec's [scripting](https://html.spec.whatwg.org/multipage/scripting.html), [interactive elements](https://html.spec.whatwg.org/multipage/interactive-elements.html), and [web application APIs](https://html.spec.whatwg.org/multipage/webappapis.html) sections.
