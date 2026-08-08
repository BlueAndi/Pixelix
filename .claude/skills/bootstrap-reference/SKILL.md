---
name: bootstrap-reference
description: Use when building or reviewing UI with Bootstrap 5.3 — setting up the CDN/npm, using the grid & breakpoints, utility classes (spacing/color/flex/display), components (navbar, modal, dropdown, card, forms, offcanvas, toast, etc.), the JavaScript plugin API (data-bs-* attributes, events), dark mode / color modes, and Sass/CSS-variable theming. Framework-specific to Bootstrap v5.3.x.
---

# Bootstrap 5.3 Reference

## Overview

Bootstrap is a mobile-first CSS + JavaScript framework: a **12-column responsive grid**, a large set of **utility classes**, prebuilt **components**, and a small set of **JavaScript plugins** driven by `data-bs-*` attributes. Current version: **v5.3.8** (5.3 line; no jQuery, dark mode built in).

**Core principle:** Compose UI from utilities and components first; write custom CSS only when Bootstrap can't express it. Everything is **mobile-first** — unprefixed classes apply to all sizes, and breakpoint-infixed classes (`-sm`/`-md`/`-lg`/`-xl`/`-xxl`) apply from that breakpoint *up*.

This is the condensed-but-accurate reference. When behavior is contested, the [official 5.3 docs](https://getbootstrap.com/docs/5.3/) are the source of truth.

## When to Use

- Setting up Bootstrap (CDN vs npm/Sass) and the starter template.
- Building **layout** with containers, the grid, and flex/spacing utilities.
- Picking the right **utility class** (spacing, color, display, flex, text, borders, sizing).
- Wiring up **components** and their required markup + `data-bs-*` attributes.
- Using the **JavaScript API** (data attributes vs programmatic, events, tooltips/popovers opt-in).
- Enabling **dark mode** / color modes and **theming** via Sass or CSS variables.

**Not for:** plain HTML semantics (see the `html5-reference` skill), JavaScript language questions (see `javascript-reference`), or non-Bootstrap CSS frameworks (Tailwind, Bulma).

## Setup

**CDN (quickest)** — CSS in `<head>`, JS bundle before `</body>`:
```html
<link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.8/dist/css/bootstrap.min.css" rel="stylesheet"
      integrity="sha384-sRIl4kxILFvY47J16cr9ZwB07vP4J8+LH7qKQnuqkuIAvNWLzeN8tE5YBujZqJLB" crossorigin="anonymous">
<script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.8/dist/js/bootstrap.bundle.min.js"
        integrity="sha384-FKyoEForCGlyvwx9Hj09JcYn3nv7wiPVlz7YYwJrWVcXK/BmnVDxM+D2scQbITxI" crossorigin="anonymous"></script>
```
- **`bootstrap.bundle.min.js`** includes **Popper** (needed by dropdowns, tooltips, popovers). `bootstrap.min.js` does not — load Popper separately if you use it.
- **Integrity hashes are version-specific** — if you change the version, copy the matching hash from the docs or drop the `integrity`/`crossorigin` attributes.
- **npm:** `npm i bootstrap@5.3.8 @popperjs/core`, then `import * as bootstrap from 'bootstrap'` and either import the CSS or `@import "bootstrap/scss/bootstrap"` in your Sass (see [javascript-theming.md](javascript-theming.md)).

**Starter template** (note `data-bs-theme` on `<html>` for dark mode):
```html
<!doctype html>
<html lang="en" data-bs-theme="light">
  <head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Bootstrap demo</title>
    <link href="…/bootstrap@5.3.8/dist/css/bootstrap.min.css" rel="stylesheet" …>
  </head>
  <body>
    <h1>Hello, world!</h1>
    <script src="…/bootstrap@5.3.8/dist/js/bootstrap.bundle.min.js" …></script>
  </body>
</html>
```
The `viewport` meta tag is **required** for responsive behavior.

## Breakpoints (mobile-first)

| Name | Infix | Min-width | `.container` max-width |
|------|-------|-----------|------------------------|
| Extra small | *(none)* | <576px | 100% |
| Small | `sm` | ≥576px | 540px |
| Medium | `md` | ≥768px | 720px |
| Large | `lg` | ≥992px | 960px |
| X-Large | `xl` | ≥1200px | 1140px |
| XX-Large | `xxl` | ≥1400px | 1320px |

Breakpoints use `min-width` media queries. "Down" queries subtract `.02px` (e.g. `md` down = `max-width: 767.98px`). A class like `d-md-flex` means "flex from 768px up"; combine with an unprefixed base (`d-none d-md-block` = hidden on mobile, shown from md).

## Core Concepts Cheat-Sheet

**Grid:** `.container` → `.row` → `.col*`. 12 columns per row. `col` = equal width, `col-6` = half, `col-md-4` = ⅓ from md up, `col-auto` = content width. Gutters via `g-*`/`gx-*`/`gy-*`. → [layout-grid.md](layout-grid.md)

**Spacing utilities:** `{m|p}{t|b|s|e|x|y|<blank>}-{0..5|auto}`, responsive `…-{bp}-…`. `1`=.25rem, `2`=.5rem, `3`=1rem, `4`=1.5rem, `5`=3rem. Negative margins `m*-n1..n5` are **opt-in** (`$enable-negative-margins`, not in the CDN build). e.g. `mt-3`, `px-md-4`, `mx-auto`. → [utilities.md](utilities.md)

**Theme colors** (semantic): `primary secondary success danger warning info light dark`. Used across `text-*`, `bg-*`, `btn-*`, `alert-*`, `border-*`, `text-bg-*`. 5.3 adds subtle/emphasis variants: `bg-primary-subtle`, `text-primary-emphasis`, `border-primary-subtle`.

**JS plugins:** declarative via `data-bs-toggle` + `data-bs-target`, or `new bootstrap.Modal(el)`. Events: `shown.bs.modal` etc. **Tooltips & popovers must be initialized in JS** (opt-in). → [javascript-theming.md](javascript-theming.md)

## Reference Files

| File | Contents |
|------|----------|
| **[layout-grid.md](layout-grid.md)** | Containers, the 12-col grid, columns/offsets/ordering, gutters, `row-cols`, breakpoints, flex & positioning layout utilities. |
| **[components.md](components.md)** | All components with required markup and `data-bs-*` attributes: buttons, navbar/nav/tabs, dropdown, card, modal, offcanvas, collapse/accordion, alert, toast, badge, list group, forms & validation, tables, pagination, carousel, spinners, tooltip/popover, progress. |
| **[utilities.md](utilities.md)** | Utility classes by family (spacing, sizing, display, flex, gap, text, color/background, borders, shadows, position, overflow, z-index, visibility) + the Sass utility API. |
| **[javascript-theming.md](javascript-theming.md)** | JS plugin API (data attributes, programmatic instances, `getOrCreateInstance`, events, methods, sanitizer), dark mode / color modes (`data-bs-theme`), and customization via Sass and CSS variables. |

## Common Mistakes

| Mistake | Fix |
|---------|-----|
| Columns not in a `.row`, or `.row` not in a `.container` | Grid requires `.container > .row > .col*`; loose columns break gutters/negative margins. |
| Using `bootstrap.min.js` then dropdowns/tooltips silently fail | Those need Popper — use `bootstrap.bundle.min.js` or add Popper. |
| Expecting tooltips/popovers to work from data attributes alone | They're opt-in: `new bootstrap.Tooltip(el)` for each (or loop over `[data-bs-toggle="tooltip"]`). |
| Loading Bootstrap CSS/JS in the wrong order or JS in `<head>` without `defer` | CSS in `<head>`; JS bundle at end of `<body>` (or `defer`). |
| `col-4` meaning "4px/40%" | Columns are **out of 12**: `col-4` = 4/12 = ⅓ width. |
| Overriding Bootstrap by editing `bootstrap.min.css` | Override via CSS variables, your own stylesheet loaded *after*, or (best) Sass variables before `@import`. |
| Adding many custom-CSS one-offs | Check utilities first (`d-flex`, `gap-3`, `text-center`, `rounded`, `shadow`). |
| `data-toggle`/`data-target` (Bootstrap 4 syntax) | v5 uses the **`data-bs-`** prefix (`data-bs-toggle`, `data-bs-target`). |
| Forgetting the `viewport` meta tag | Required; without it responsive classes don't behave. |
| Custom colors by hardcoding hex everywhere | Extend `$theme-colors` in Sass or override `--bs-*` CSS variables. |

## Docs Navigation

[Getting started](https://getbootstrap.com/docs/5.3/getting-started/introduction/) · [Layout](https://getbootstrap.com/docs/5.3/layout/grid/) · [Content/Forms](https://getbootstrap.com/docs/5.3/forms/overview/) · [Components](https://getbootstrap.com/docs/5.3/components/buttons/) · [Utilities](https://getbootstrap.com/docs/5.3/utilities/spacing/) · [Customize](https://getbootstrap.com/docs/5.3/customize/overview/) · [Color modes](https://getbootstrap.com/docs/5.3/customize/color-modes/)
