---
name: html5-reference
description: Use when authoring, reviewing, or validating HTML markup — choosing the right element, deciding what may nest inside what, writing forms/inputs, adding accessible semantics, or checking a document against the WHATWG living standard. Covers elements, content categories, global attributes, forms, and HTML-defined APIs.
---

# HTML5 Reference (WHATWG Living Standard)

## Overview

HTML5 is not "a set of tags you can put anywhere." The [WHATWG HTML Living Standard](https://html.spec.whatwg.org/multipage/) defines, for every element, **which content category it belongs to**, **where it is allowed to appear**, and **what it is allowed to contain**. Correct HTML is markup that satisfies these *content model* rules.

**Core principle:** Pick the element whose semantics match the content's meaning, then verify the nesting is valid per the content model. Meaning first, appearance never (styling is CSS's job).

This skill is the authoritative-but-condensed reference. It is version-current with the spec (living standard) — when in doubt, the linked multipage spec is the source of truth.

## When to Use

- Deciding **which element** is correct (`<section>` vs `<div>` vs `<article>`; `<b>` vs `<strong>`; `<figure>` vs `<img>`).
- Checking whether a nesting is **valid** ("can a `<div>` go inside a `<p>`?" — no; "can an `<a>` wrap block content?" — yes, it's transparent).
- Building **forms**: choosing an `<input>` type, wiring up labels, native constraint validation.
- Adding **accessible semantics**: native element vs ARIA, landmarks, labeling.
- Reviewing HTML for **conformance** / removing deprecated markup.

**Not for:** CSS layout/styling, JS framework patterns, or build tooling. For this repo's web-dashboard conventions (Bootstrap/vanilla JS), that's a separate concern.

## The Content Model in One Picture

Every element declares content **categories** (what it *is*) and a **content model** (what it may *contain*). An element may appear only where its categories are allowed.

| Category | Meaning | Examples |
|----------|---------|----------|
| **Metadata** | Configures the document / links resources | `base` `link` `meta` `style` `title` `script` `noscript` |
| **Flow** | Almost everything usable in `<body>` | `p` `div` `section` `table` `img` `a` `span` `input` … |
| **Sectioning** | Scopes headings/footers into a section | `article` `aside` `nav` `section` |
| **Heading** | Section headings | `h1`–`h6` `hgroup` |
| **Phrasing** | Text-level markup (roughly "inline") | `a` `em` `strong` `span` `code` `img` `input` `br` … |
| **Embedded** | Imports external/other-namespace content | `img` `video` `audio` `canvas` `iframe` `svg` `object` |
| **Interactive** | User-operable / focusable | `a`(href) `button` `input` `select` `textarea` `details` `label` |
| **Palpable** | Non-empty, perceivable content | most flow/phrasing that isn't hidden/empty |
| **Script-supporting** | Doesn't render; supports scripting | `script` `template` |

**Golden nesting rules (memorize these):**
- **Phrasing-only parents** (`<p>`, `<h1>`–`<h6>`, `<span>`, `<label>`, `<button>`, `<em>`, …) may contain **only phrasing content** — never `<div>`, `<ul>`, `<section>`, or another `<p>`.
- **Transparent** elements (`<a>`, `<ins>`, `<del>`, `<map>`, `<object>`, `<video>`/`<audio>` content, `<slot>`, autonomous custom elements) inherit their parent's content model — so `<a>` *can* wrap block-level content when it sits in a flow context.
- **`<a>` and `<button>` must not contain interactive content** — no `<a>`, `<button>`, `<input>`, `<select>`, etc. (nor any element with `tabindex`) inside them. So no `<a>` in `<a>`, no `<button>` in `<a>`. (This restriction is specific to `a`/`button` — it is *not* a blanket "no interactive in interactive" rule.)
- **`<label>` may contain at most one labelable control** and no nested `<label>`. Wrapping its own control is fine (`<label>Name <input></label>`); adding a second control, or one the label doesn't label, is not.
- **Inter-element whitespace** is always allowed and ignored when checking content models.

Full details → **[elements.md](elements.md)** (every element: category, allowed parents, content model) and **[content-model.md](content-model.md)** (categories, transparency, sectioning, validity rules).

## Document Skeleton

```html
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Required — the only mandatory metadata</title>
</head>
<body>
  <!-- flow content -->
</body>
</html>
```

Conformance essentials: `<!DOCTYPE html>` first, a single `<html>` root, a `<head>` containing exactly one `<title>` (unless `<title>` is supplied via a higher-level protocol), `<meta charset>` within the first 1024 bytes. `<html>`, `<head>`, `<body>` tags themselves are omissible per the parsing rules but keep them for clarity.

## Global Attributes (allowed on every element)

| Attribute | Purpose |
|-----------|---------|
| `id` `class` `style` `title` | Identity, styling hook, inline CSS, advisory tooltip |
| `lang` `dir` `translate` | Language, text direction (`ltr`/`rtl`/`auto`), translation opt-out |
| `hidden` `inert` | Hide from rendering / remove from interaction + a11y tree |
| `tabindex` `accesskey` `autofocus` | Focus order/focusability, shortcut key, initial focus |
| `contenteditable` `spellcheck` `draggable` | Editable content, spellcheck, drag-and-drop |
| `inputmode` `enterkeyhint` `autocapitalize` `autocorrect` | Virtual-keyboard hints |
| `popover` | Marks an element as a popover (Popover API) |
| `is` | Instantiate a customized built-in custom element |
| `data-*` | Custom author data (read via `dataset`) |
| `role` / `aria-*` | ARIA role/state/property overrides (see [accessibility.md](accessibility.md)) |
| `itemid` `itemprop` `itemref` `itemscope` `itemtype` | Microdata |
| `slot` `nonce` | Shadow-DOM slot assignment, CSP nonce |
| `on*` | Event-handler content attributes (`onclick`, `oninput`, …) — prefer `addEventListener` |

## Reference Files

| File | Contents |
|------|----------|
| **[elements.md](elements.md)** | Every element grouped by function: categories, allowed contexts, content model, key attributes. |
| **[content-model.md](content-model.md)** | Content categories in depth, transparency, sectioning/outline, conformance & validity rules, deprecated features. |
| **[forms.md](forms.md)** | `<form>`, all `<input>` types + their attributes, other controls, native constraint validation & the Constraint Validation API. |
| **[accessibility.md](accessibility.md)** | Accessible authoring, ARIA vs native semantics, landmarks, labeling, plus HTML-defined JS APIs (canvas, media, dialog, template, custom elements, storage). |

## Common Mistakes

| Mistake | Correct approach |
|---------|------------------|
| Block element inside `<p>` (`<div>`, `<ul>`, `<table>`) | `<p>` holds phrasing only; close it first, or use `<div>`. |
| `<div>`/`<span>` where a semantic element fits | Use `<button>`, `<nav>`, `<section>`, `<figure>`, etc.; div/span are last resort. |
| Multiple `<main>`, or `<main>` inside `article`/`aside`/`nav`/`header`/`footer` | One visible `<main>` per document, as a top-level flow container. |
| `<section>` used purely for styling | A `<section>` needs a heading and represents a thematic grouping; otherwise use `<div>`. |
| Interactive content inside `<a>`/`<button>` (`<a>` in `<a>`, `<button>` in `<a>`) | `<a>`/`<button>` forbid interactive (and `tabindex`) descendants; restructure. |
| Two controls in one `<label>`, or a nested `<label>` | A `<label>` labels exactly one control; split them. |
| Choosing input type by appearance | Choose by data semantics (`email`, `number`, `date`) — you get validation + right mobile keyboard free. |
| `role`/`aria-*` duplicating native semantics | Prefer native elements; ARIA only fills gaps. "No ARIA is better than bad ARIA." |
| Deprecated presentational markup (`<font>`, `<center>`, `align=`, `<marquee>`) | Use CSS. |

## Spec Navigation

Jump to the source of truth:
- Elements: [semantics](https://html.spec.whatwg.org/multipage/semantics.html) · [sections](https://html.spec.whatwg.org/multipage/sections.html) · [grouping](https://html.spec.whatwg.org/multipage/grouping-content.html) · [text-level](https://html.spec.whatwg.org/multipage/text-level-semantics.html) · [edits](https://html.spec.whatwg.org/multipage/edits.html) · [embedded](https://html.spec.whatwg.org/multipage/embedded-content.html) · [tables](https://html.spec.whatwg.org/multipage/tables.html) · [forms](https://html.spec.whatwg.org/multipage/forms.html) · [interactive](https://html.spec.whatwg.org/multipage/interactive-elements.html) · [scripting](https://html.spec.whatwg.org/multipage/scripting.html)
- Concepts: [content models & categories](https://html.spec.whatwg.org/multipage/dom.html#content-models) · [global attributes](https://html.spec.whatwg.org/multipage/dom.html#global-attributes) · [element index](https://html.spec.whatwg.org/multipage/indices.html)
