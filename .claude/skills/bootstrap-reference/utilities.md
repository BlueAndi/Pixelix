# Utilities

Source: [Utilities docs](https://getbootstrap.com/docs/5.3/utilities/). Small single-purpose classes. **Most accept a breakpoint infix** for responsiveness: `{property}-{bp}-{value}` (e.g. `text-md-center`, `d-lg-none`, `px-xl-5`).

## Spacing (margin & padding)

Format: `{m|p}{sides}-{size}` and responsive `{m|p}{sides}-{bp}-{size}`.

- **property:** `m` = margin, `p` = padding.
- **sides:** *(blank)* = all, `t` top, `b` bottom, `s` start (left in LTR), `e` end (right), `x` = left+right, `y` = top+bottom.
- **size:** `0`=0 · `1`=.25rem · `2`=.5rem · `3`=1rem · `4`=1.5rem · `5`=3rem · `auto` (margins only).
- **Negative margins:** `m*-n1` … `m*-n5` (margin only) — **opt-in**: not generated in the default/CDN build; enable with `$enable-negative-margins: true` in Sass.

Examples: `mt-3` (margin-top 1rem), `px-md-4` (h-padding 1.5rem from md), `mx-auto` (center a fixed-width block), `mb-0`, `py-2`.

## Colors — theme colors & 5.3 subtle/emphasis

Semantic colors: `primary secondary success danger warning info light dark`. Plus body/neutral tokens.

| Family | Classes |
|--------|---------|
| Text | `text-{color}`, `text-{color}-emphasis`, `text-body`, `text-body-secondary`, `text-body-tertiary`, `text-black`, `text-white` (`text-muted` is **deprecated** since 5.3 — use `text-body-secondary`) |
| Background | `bg-{color}`, `bg-{color}-subtle`, `bg-body`, `bg-body-secondary`, `bg-body-tertiary`, `bg-transparent` |
| Text-on-bg pair | `text-bg-{color}` (sets bg + a readable contrasting text color) |
| Border | `border-{color}`, `border-{color}-subtle` |
| Links | `link-{color}`, `link-underline-{color}`, `link-opacity-{25..100}`, `link-offset-{1..3}` |
| Opacity | `opacity-{0,25,50,75,100}`; `text-opacity-*` and `bg-opacity-*` (multiply the color) |

5.3 idiom for tinted panels: `bg-primary-subtle text-primary-emphasis border border-primary-subtle`. These automatically adapt under `data-bs-theme="dark"`.

## Display & visibility

- `d-{none|inline|inline-block|block|grid|inline-grid|flex|inline-flex|table|table-row|table-cell}` + breakpoint infixes. Responsive show/hide: `d-none d-md-block`, `d-md-none` (hide from md).
- Print: `d-print-none`, `d-print-block`.
- `visible` / `invisible` (keeps layout space). `visually-hidden` / `visually-hidden-focusable` (screen-reader-only). Toggle pointer events with `pe-none` / `pe-auto`.

## Flexbox & gap

`d-flex` then: `flex-{row|column}(-reverse)`, `flex-{wrap|nowrap|wrap-reverse}`, `justify-content-{start|end|center|between|around|evenly}`, `align-items-{start|end|center|baseline|stretch}`, `align-content-*`, `align-self-*`, `flex-fill`, `flex-{grow|shrink}-{0|1}`, `flex-{bp}-*`, `order-{0..5|first|last}`. Gaps: `gap-{0..5}`, `row-gap-*`, `column-gap-*` (require a flex or grid parent).

## Text & typography

- Align: `text-{start|center|end}` (+ `-{bp}-`). Wrap: `text-wrap`, `text-nowrap`, `text-break`, `text-truncate` (needs a block/inline-block with a width).
- Transform: `text-{lowercase|uppercase|capitalize}`.
- Weight/style: `fw-{light|lighter|normal|medium|semibold|bold|bolder}`, `fst-{italic|normal}`.
- Size/line-height: `fs-{1..6}` (h1-h6 scale), `lh-{1|sm|base|lg}`. Monospace: `font-monospace`.
- Decoration: `text-decoration-{underline|line-through|none}`.

## Borders, radius, shadows

- Presence: `border`, `border-0`, `border-{top|end|bottom|start}`, `border-{top…}-0`.
- Width/color: `border-{1..5}`, `border-{color}`, `border-{color}-subtle`, `border-opacity-{10..100}`.
- Radius: `rounded`, `rounded-{0..5}`, `rounded-{top|end|bottom|start|circle|pill}`.
- Shadow: `shadow-none`, `shadow-sm`, `shadow`, `shadow-lg`.

## Sizing, position, overflow, misc

- Size: `w-{25|50|75|100|auto}`, `h-*`, `mw-100`, `mh-100`, `vw-100`, `vh-100`, `min-vw-100`, `min-vh-100`.
- Position: `position-{static|relative|absolute|fixed|sticky}`; edges `top-0`/`start-0`/`end-0`/`bottom-0` (+ `50`,`100`); `translate-middle(-x|-y)`; `fixed-top`, `fixed-bottom`, `sticky-top`, `sticky-{bp}-top`.
- Z-index: `z-{n1|0|1|2|3}`.
- Overflow: `overflow-{auto|hidden|visible|scroll}`, `overflow-x-*`, `overflow-y-*`.
- Object fit (images/video): `object-fit-{contain|cover|fill|scale|none}` (+ breakpoint).
- Aspect ratio: wrap in `ratio ratio-{1x1|4x3|16x9|21x9}`.
- Vertical align (inline/table): `align-{baseline|top|middle|bottom|text-top|text-bottom}`.
- Interaction: `user-select-{all|auto|none}`, `pe-none`.
- Float: `float-{start|end|none}` (+ breakpoint).

## Customizing the utilities (Sass API)

Utilities are generated from the `$utilities` Sass map, so you can **add, modify, or remove** any of them. Import Bootstrap's functions/variables/maps first, then merge:

```scss
@import "bootstrap/scss/functions";
@import "bootstrap/scss/variables";
@import "bootstrap/scss/variables-dark";
@import "bootstrap/scss/maps";
@import "bootstrap/scss/mixins";
@import "bootstrap/scss/utilities";

// Add a new "cursor" utility with responsive + a custom value
$utilities: map-merge($utilities, (
  "cursor": (
    property: cursor,
    class: cursor,
    responsive: true,
    values: auto pointer grab,
  ),
));

@import "bootstrap/scss/utilities/api"; // generate the classes
```
- Set `responsive: true` to get breakpoint variants; `state: hover focus` to get `.class-hover` style pseudo-variants.
- Remove a utility: `map-merge($utilities, ("float": null))`.
- This is preferable to writing one-off custom CSS when you need a class repeated across the app.

## When to reach for a utility vs custom CSS

Prefer utilities for spacing, alignment, color, display, borders, and one-off tweaks — they're consistent and theme-aware. Drop to custom CSS (or the Sass utility API for repeated needs) only when no utility expresses the intent. Avoid editing `bootstrap.min.css` directly.
