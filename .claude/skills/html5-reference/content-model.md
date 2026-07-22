# Content Model, Categories & Conformance

The single most important idea in HTML5. Source: [content models](https://html.spec.whatwg.org/multipage/dom.html#content-models).

## Content categories

Each element belongs to zero or more categories. A parent's *content model* is expressed in terms of these categories: an element may appear somewhere only if the surrounding content model allows one of its categories there.

### Metadata content
Sets up presentation/behavior of the document or its relationship to other resources.
`base` `link` `meta` `noscript` `script` `style` `title` `template`

### Flow content
Most elements used in the document body.
`a` `abbr` `address` `article` `aside` `audio` `b` `bdi` `bdo` `blockquote` `br` `button` `canvas` `cite` `code` `data` `datalist` `del` `details` `dfn` `dialog` `div` `dl` `em` `embed` `fieldset` `figure` `footer` `form` `h1`–`h6` `header` `hgroup` `hr` `i` `iframe` `img` `input` `ins` `kbd` `label` `main` `map` `mark` `menu` `meter` `nav` `noscript` `object` `ol` `output` `p` `picture` `pre` `progress` `q` `ruby` `s` `samp` `script` `search` `section` `select` `slot` `small` `span` `strong` `sub` `sup` `table` `template` `textarea` `time` `u` `ul` `var` `video` `wbr` · plus `svg`/`math`, text, and autonomous custom elements. (`area` is flow only when a descendant of `map`; `link`/`meta` only when `body`-allowed.)

### Sectioning content
Defines the scope of headers/footers.
`article` `aside` `nav` `section`

### Heading content
`h1` `h2` `h3` `h4` `h5` `h6` `hgroup`

### Phrasing content
Text and text-level markup — roughly the old "inline" set. This is the content model of `<p>`, `<h1>`–`<h6>`, `<span>`, `<label>`, `<button>`, `<em>`, etc.
`a`* `abbr` `area`* `audio` `b` `bdi` `bdo` `br` `button` `canvas` `cite` `code` `data` `datalist` `del`* `dfn` `em` `embed` `i` `iframe` `img` `input` `ins`* `kbd` `label` `link`* `map`* `mark` `meta`* `meter` `noscript` `object` `output` `picture` `progress` `q` `ruby` `s` `samp` `script` `select` `slot` `small` `span` `strong` `sub` `sup` `template` `textarea` `time` `u` `var` `video` `wbr` · plus `svg`/`math`, text, autonomous custom elements. (`a`/`del`/`ins`/`map`/`slot` are phrasing only when they contain phrasing — they're transparent.)

### Embedded content
Imports another resource or a foreign namespace.
`audio` `canvas` `embed` `iframe` `img` `math` `object` `picture` `svg` `video`

### Interactive content
Designed for user interaction.
`a` (with `href`) · `audio`/`video` (with `controls`) · `button` `details` `embed` `iframe` `img` (with `usemap`) · `input` (not `hidden`) · `label` `object` `select` `textarea`

### Palpable content
Non-empty, non-hidden — something the user can perceive. Elements with a "palpable" note should have meaningful content (not just whitespace/empty).

### Script-supporting elements
Render nothing themselves but support scripting: `script` `template`.

## Transparent content model

Some elements have a **transparent** content model: their allowed contents are exactly whatever the *parent* would allow in the transparent element's place. Transparent elements: `a`, `ins`, `del`, `map`, `object`, `slot`, media (`audio`/`video`) after their `source`/`track`, `canvas` fallback, and autonomous custom elements.

Consequence: `<a>` can legally wrap block-level content —
```html
<a href="/product/42">      <!-- transparent: valid because <a> sits in flow -->
  <article>
    <h3>Widget</h3>
    <p>Great widget.</p>
  </article>
</a>
```
but the same `<a>` inside a `<p>` may contain only phrasing.

**Transparency does not relax `<a>`/`<button>`'s extra restriction:** both must have **no interactive-content descendant and no descendant with `tabindex`** — so you can't nest another link, button, or control inside them even where the content model is otherwise transparent. This is a restriction specific to `a`/`button` (and `form` for nested forms), **not** a global "no interactive in interactive" rule: `<label>` is itself interactive content yet legally *wraps* its control. `<label>`'s own rule is different — at most one labelable descendant and no nested `<label>`.

## Special content-model rules to remember

- **`<p>` auto-closes.** The `<p>` end tag is implied when a block-level (non-phrasing flow) element follows. Nesting a `<div>`/`<ul>`/`<table>` "inside" a `<p>` actually closes the paragraph first — a validity error and usually a bug.
- **Inter-element whitespace** and comments are always permitted and ignored for content-model checking.
- **`<figcaption>`** — at most one per `<figure>`, and it must be the first or last child.
- **`<summary>`** must be the first child of `<details>` (it may hold phrasing *or* heading content).
- **`<legend>`** must be the first child of `<fieldset>`.
- **`<label>`** contains at most one labelable descendant (its associated control) and no nested `<label>`.
- **`<li>`** only in `ol`/`ul`/`menu`; **`<dt>`/`<dd>`** only in `dl` (optionally grouped by `div`); **`<option>`** only in `select`/`datalist`/`optgroup`.
- **One `<main>`** rendered at a time; not a descendant of `article`, `aside`, `header`, `footer`, or `nav`.
- **Heading ranks are the real outline.** There is no algorithmic document outline; assistive tech uses `h1`–`h6` ranks. Don't restart at `h1` per `<section>` expecting an implied outline — choose ranks that reflect nesting depth.

## Conformance & validity

A *conforming* document obeys every content model, uses attributes only where allowed with valid values, and avoids obsolete features. Distinct from *rendering*: browsers' error-recovery parser will display invalid markup, but conformance is what tooling, accessibility, and maintainability depend on.

**Validate with:**
- The Nu Html Checker — <https://validator.nu/> (also the backend of validator.w3.org).
- Editor/CI linters (e.g. `html-validate`).

**Boolean attributes:** presence = true. Write `disabled`, `disabled=""`, or `disabled="disabled"` — never `disabled="true"`/`"false"`.

**Enumerated attributes** accept a fixed keyword set (e.g. `dir="ltr|rtl|auto"`, `contenteditable="true|false|plaintext-only"`); invalid keywords fall back to a default.

## Obsolete / non-conforming features (do not emit)

- Presentational elements: `<font>` `<center>` `<big>` `<strike>` `<tt>` `<marquee>` `<blink>` `<acronym>` `<applet>` `<frame>`/`<frameset>` `<dir>`.
- Presentational attributes: `align` `bgcolor` `border` (on most elements) `color` `width`/`height` on tables via attributes for layout, `cellpadding`/`cellspacing`, `nowrap`, `valign`, `hspace`/`vspace`, `<body>` `background`/`text`/`link`. Use CSS.
- `<a name>` (use `id`), `longdesc`, `<table summary>` (use `<caption>`/`aria-describedby`).
- The removed application-cache (`manifest` on `<html>`) — use Service Workers.

## Quick decision aids

**Which sectioning/grouping element?**
- Standalone, redistributable unit (post, comment, card) → `article`
- Thematic group within a flow, has a heading → `section`
- Set of navigation links → `nav`
- Tangential/complementary aside → `aside`
- The page's primary unique content → `main`
- Purely a styling/scripting hook, no semantics → `div`

**Which text-level element?**
- Meaning-changing emphasis → `em`; importance/urgency → `strong`
- Stylistically offset, no added importance → `i`/`b`; annotate → `u`/`mark`
- Code → `code`; variable → `var`; output → `samp`; keys → `kbd`
- No applicable semantics, need a hook → `span`
