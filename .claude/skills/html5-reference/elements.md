# HTML Elements Reference

Every element grouped by function. Columns: **Categories** (what the element *is*), **Contexts** (where it may appear — the parent's content model must allow these categories), **Content** (what it may contain). `*` = there are attribute/position-conditional exceptions; follow the spec link for edge cases. `transparent` = the element's allowed content equals whatever is allowed in its own position.

Legend for content models: `phrasing` / `flow` / `metadata` etc. refer to the categories in [content-model.md](content-model.md). `empty` = must contain nothing. `text` = character data only.

---

## Document metadata (`<head>`)

| Element | Description | Categories | Contexts | Content |
|---------|-------------|-----------|----------|---------|
| `html` | Root element | — | root (or `template`) | one `head` then one `body` |
| `head` | Metadata container | — | as first child of `html` | metadata content; exactly one `title`* |
| `title` | Document title (mandatory*) | metadata | in `head` | text |
| `base` | Base URL / default target (max one) | metadata | in `head` | empty |
| `link` | External resource / relationship | metadata; flow*; phrasing* | `head`; or body if `itemprop`/allowed rel | empty |
| `meta` | Name/value, charset, http-equiv, viewport | metadata; flow*; phrasing* | `head`; `meta charset`/`http-equiv` in head only | empty |
| `style` | Embedded CSS | metadata | `head` (or where metadata allowed) | text (CSS) |

Key: `<meta charset="utf-8">`, `<meta name="viewport" content="width=device-width, initial-scale=1">`, `<link rel="stylesheet" href>`, `<link rel="icon">`, `<link rel="preload/preconnect/modulepreload">`.

## Sections & document structure

| Element | Description | Categories | Contexts | Content |
|---------|-------------|-----------|----------|---------|
| `body` | Document body | — | second child of `html` | flow |
| `article` | Self-contained, syndicatable composition | flow; sectioning; palpable | flow | flow |
| `section` | Generic thematic section (needs a heading) | flow; sectioning; palpable | flow | flow |
| `nav` | Major navigation block | flow; sectioning; palpable | flow | flow |
| `aside` | Tangential / sidebar content | flow; sectioning; palpable | flow | flow |
| `h1`–`h6` | Headings (rank 1–6) | flow; heading; palpable | `legend`, `summary`, or flow | phrasing |
| `hgroup` | Heading + subheading grouping | flow; palpable | flow, `legend`, `summary` | one `h1`–`h6` + `p`s |
| `header` | Introductory/navigational aids | flow; palpable | flow (not inside `header`/`footer`/`address`) | flow* (no `header`/`footer`/`main`) |
| `footer` | Footer for page/section | flow; palpable | flow (not inside `header`/`footer`/`address`) | flow* |
| `main` | Dominant content (one per doc) | flow; palpable | flow, as a top-level container* | flow |
| `address` | Contact info for nearest article/body | flow; palpable | flow | flow* (no heading/sectioning/`header`/`footer`/`address`) |

**Sectioning note:** the modern outline is *not* auto-generated from sectioning elements — screen readers expose headings by their rank (`h1`–`h6`), so use ranks meaningfully. Give every `section`/`article`/`nav`/`aside` a heading.

## Grouping content

| Element | Description | Categories | Contexts | Content |
|---------|-------------|-----------|----------|---------|
| `p` | Paragraph | flow; palpable | flow | **phrasing only** |
| `hr` | Thematic break | flow | flow (also `select`) | empty |
| `pre` | Preformatted text (whitespace preserved) | flow; palpable | flow | phrasing |
| `blockquote` | Block quotation (`cite` attr = source URL) | flow; palpable | flow | flow |
| `ol` | Ordered list (`type`, `start`, `reversed`) | flow; palpable* | flow | `li`; script-supporting |
| `ul` | Unordered list | flow; palpable* | flow | `li`; script-supporting |
| `menu` | List of commands (list variant) | flow; palpable* | flow | `li`; script-supporting |
| `li` | List item (`value` in `ol`) | — | `ol`, `ul`, `menu` | flow |
| `dl` | Description list (name-value groups) | flow; palpable* | flow | `dt`/`dd` groups, or `div` groups |
| `dt` | Term/name | — | `dl`, `div` in `dl` | flow* (no sectioning/heading/`header`/`footer`) |
| `dd` | Description/value | — | `dl`, `div` in `dl` | flow |
| `figure` | Self-contained figure | flow; palpable | flow | optional `figcaption` + flow |
| `figcaption` | Caption for `figure` | — | first/last child of `figure` | flow |
| `main` | (see Sections) | | | |
| `search` | Grouping of search/filter controls | flow; palpable | flow | flow |
| `div` | Generic flow container (no semantics) | flow; palpable | flow; also grouping child of `dl` | flow* |

## Text-level semantics (phrasing)

All below are `flow; phrasing; palpable`, live in phrasing contexts, and contain phrasing (unless noted).

| Element | Use it for |
|---------|-----------|
| `a` | Hyperlink. **transparent** content (may wrap flow when in flow). Attrs: `href`, `target`, `rel`, `download`, `hreflang`, `type`, `referrerpolicy`. Interactive only when `href` present. |
| `em` | Stress emphasis (meaning changes if moved). |
| `strong` | Strong importance/seriousness/urgency. |
| `small` | Side comments / fine print. |
| `s` | No-longer-accurate/relevant (not `del`). |
| `cite` | Title of a work. |
| `q` | Inline quotation (`cite` attr = source). |
| `dfn` | Defining instance of a term. |
| `abbr` | Abbreviation (`title` = expansion). |
| `ruby`/`rt`/`rp` | Ruby annotations (E. Asian). |
| `data` | Machine-readable `value` for text. |
| `time` | Machine-readable `datetime`. |
| `code` | Code fragment. |
| `var` | Variable / math symbol. |
| `samp` | Sample program output. |
| `kbd` | User keyboard input. |
| `sub`/`sup` | Subscript / superscript. |
| `i` | Alternate voice/mood/technical term (no extra importance). |
| `b` | Draw attention without extra importance (keywords). |
| `u` | Non-textual annotation (e.g. proper-name/misspelling underline). |
| `mark` | Highlight for relevance/reference. |
| `bdi` | Isolate for bidi. |
| `bdo` | Override bidi (`dir` required). |
| `span` | Generic phrasing container (no semantics). |
| `br` | Line break (meaningful, e.g. verse/address). `empty`. |
| `wbr` | Line-break opportunity. `empty`. |

## Edits

| Element | Description | Content |
|---------|-------------|---------|
| `ins` | Added content (`cite`, `datetime`) | **transparent** |
| `del` | Removed content (`cite`, `datetime`) | **transparent** |

## Embedded content

| Element | Description | Categories | Content | Key attrs |
|---------|-------------|-----------|---------|-----------|
| `img` | Image | flow; phrasing; embedded; (interactive if `usemap`); form-associated; palpable | empty | `src` (req), `alt` (req unless decorative), `srcset`, `sizes`, `width`, `height`, `loading`, `decoding`, `fetchpriority` |
| `picture` | Art-directed/responsive image wrapper | flow; phrasing; embedded; palpable | `source`s + one `img` | — |
| `source` | Candidate resource | — | empty | `srcset`/`sizes`/`media`/`type` (in `picture`); `src`/`type` (in media) |
| `iframe` | Nested browsing context | flow; phrasing; embedded; interactive; palpable | empty | `src`, `srcdoc`, `sandbox`, `allow`, `loading`, `referrerpolicy`, `width`, `height` |
| `embed` | Integration point for external app/plugin | flow; phrasing; embedded; interactive; palpable | empty | `src`, `type`, `width`, `height` |
| `object` | External resource / nested context | flow; phrasing; embedded; (interactive*); listed; form-associated; palpable | **transparent** | `data`, `type`, `name`, `form`, `width`, `height` |
| `video` | Video player | flow; phrasing; embedded; interactive*; palpable | `source`*/`track`* + **transparent** | `src`, `controls`, `autoplay`, `loop`, `muted`, `poster`, `preload`, `playsinline`, `crossorigin`, `width`, `height` |
| `audio` | Audio player | flow; phrasing; embedded; interactive*; palpable* | `source`*/`track`* + **transparent** | `src`, `controls`, `autoplay`, `loop`, `muted`, `preload`, `crossorigin` |
| `track` | Timed text (captions/subtitles) | — | empty | `kind`, `src`, `srclang`, `label`, `default` |
| `map` | Image map | flow; phrasing*; palpable | **transparent**; `area`s | `name` (req) |
| `area` | Hotspot in a map | flow; phrasing | empty | `shape`, `coords`, `href`, `alt` |
| `canvas` | Scriptable bitmap | flow; phrasing; embedded; palpable | **transparent** (fallback) | `width`, `height` |
| `svg` / `math` | Foreign-namespace content | embedded; flow; phrasing; palpable | per SVG/MathML | — |

## Tables

| Element | Description | Contexts | Content |
|---------|-------------|----------|---------|
| `table` | Table | flow | `caption`?, `colgroup`*, `thead`?, (`tbody`* \| `tr`*), `tfoot`? |
| `caption` | Table caption (first child) | `table` | flow* |
| `colgroup` | Column group (`span`) | `table` | `col`* / `template` |
| `col` | Column (`span`) | `colgroup` | empty |
| `thead` / `tbody` / `tfoot` | Row groups | `table` | `tr`; script-supporting |
| `tr` | Row | `table`, `thead`, `tbody`, `tfoot` | `th`*, `td` |
| `th` | Header cell (`scope`, `colspan`, `rowspan`, `headers`, `abbr`) | `tr` | flow* |
| `td` | Data cell (`colspan`, `rowspan`, `headers`) | `tr` | flow |

Correct order matters: `caption` → `colgroup` → `thead` → `tbody`/`tr` → `tfoot` (though `tfoot` may also come last). Use `<th scope="col|row">` for accessible headers.

## Forms

See **[forms.md](forms.md)** for the full treatment. Elements: `form`, `label`, `input`, `button`, `select`, `datalist`, `optgroup`, `option`, `textarea`, `output`, `progress`, `meter`, `fieldset`, `legend`.

## Interactive elements

| Element | Description | Categories | Content |
|---------|-------------|-----------|---------|
| `details` | Disclosure widget (`open`, `name` for exclusive groups) | flow; interactive; palpable | one `summary` + flow |
| `summary` | Caption/toggle for `details` (first child) | — | phrasing or heading content |
| `dialog` | Dialog/modal (`open`; use `showModal()`/`show()`/`close()`) | flow | flow |

## Scripting

| Element | Description | Categories | Content |
|---------|-------------|-----------|---------|
| `script` | Embedded/external script (`src`, `type="module"`, `async`, `defer`, `nomodule`, `nonce`, `integrity`, `crossorigin`) | metadata; flow; phrasing; script-supporting | script/data/documentation |
| `noscript` | Fallback when scripting disabled | metadata; flow; phrasing | varies by context |
| `template` | Inert content fragment (`.content` DocumentFragment; `shadowrootmode` for declarative shadow DOM) | metadata; flow; phrasing; script-supporting | (template contents — not children) |
| `slot` | Shadow-tree slot (`name`) | flow; phrasing | **transparent** |

## Custom elements

Autonomous custom elements (`<my-widget>`) are `flow; phrasing; palpable`, live in flow/phrasing, and have **transparent** content. Names must contain a hyphen. Customized built-ins use the `is` attribute (`<button is="fancy-button">`). Defined via `customElements.define()` — see [accessibility.md](accessibility.md).
