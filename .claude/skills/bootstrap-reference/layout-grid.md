# Layout & Grid

Source: [Layout docs](https://getbootstrap.com/docs/5.3/layout/grid/). Mobile-first; see breakpoint table in [SKILL.md](SKILL.md).

## Containers

Every layout starts in a container (provides responsive gutters/padding and centers content).

| Class | Behavior |
|-------|----------|
| `.container` | Responsive fixed max-width, steps at each breakpoint (see SKILL breakpoint table). |
| `.container-fluid` | Full width (100%) at every breakpoint. |
| `.container-{sm\|md\|lg\|xl\|xxl}` | 100% wide until the named breakpoint, then fixed max-width. |

Containers add horizontal padding (`--bs-gutter-x`, default 1.5rem total). Don't nest a `.container` inside a `.container` unnecessarily.

## The grid

`.container` → `.row` → `.col*`. Rows are flex containers with negative horizontal margins that cancel container padding; columns are flex children padded by half the gutter. **Always put columns inside a `.row`.**

```html
<div class="container">
  <div class="row">
    <div class="col-8">8/12 wide</div>
    <div class="col-4">4/12 wide</div>
  </div>
</div>
```

### Column sizing

| Class | Meaning |
|-------|---------|
| `.col` | Equal-width; splits remaining space among all `.col` siblings. |
| `.col-{1..12}` | Explicit span out of 12. |
| `.col-auto` | Width = content. |
| `.col-{bp}-{1..12\|auto}` | Applies from that breakpoint up (e.g. `.col-md-6`). |

Columns are **mobile-first and cumulative**: `class="col-12 col-md-8 col-lg-6"` = full width on xs/sm, 8/12 from md, 6/12 from lg. If the columns in a row exceed 12, the extras **wrap** to a new line. Mixing one explicit `col-*` with `col`s lets the rest share the leftover space.

### `row-cols` (equal columns per row)

Set how many columns per line without sizing each child:
```html
<div class="row row-cols-1 row-cols-md-2 row-cols-lg-4 g-3">
  <div class="col"><div class="card">…</div></div>
  … <!-- 1 per row on mobile, 2 from md, 4 from lg -->
</div>
```

### Gutters

Gutters are the gaps between columns (and rows), controlled on the `.row`:

| Class | Effect |
|-------|--------|
| `.g-{0..5}` | Both axes. `.g-0` removes gutters. |
| `.gx-{0..5}` | Horizontal only. |
| `.gy-{0..5}` | Vertical only (adds top margin to wrapped rows). |
| `.g-{bp}-{n}` | Responsive. |

Default gutter width is 1.5rem. Gutters are implemented via padding + `--bs-gutter-x/y`.

### Offsets, ordering, alignment

- **Offset:** `.offset-{bp}-{1..11}` pushes a column right by n columns. Or use margin utilities (`.ms-auto`, `.me-auto`) to push within a flex row.
- **Order:** `.order-{0..5}`, `.order-{bp}-*`, `.order-first` (-1), `.order-last` (6). Visual reordering only.
- **Vertical align (whole row):** `.align-items-{start|center|end}`; per-column `.align-self-*`.
- **Horizontal distribute:** `.justify-content-{start|center|end|between|around|evenly}` on `.row`.
- **Nesting:** put a new `.row` inside a `.col` to subdivide; the nested row re-splits into 12.

## Layout utilities (beyond the grid)

Bootstrap 5 leans on flex utilities for layout; the grid is one tool among many.

**Display:** `.d-{none|inline|inline-block|block|flex|inline-flex|grid|table|…}` and responsive `.d-{bp}-{value}`. Common: `.d-none .d-md-block` (show from md), `.d-flex`.

**Flexbox:** `.flex-{row|column}(-reverse)`, `.flex-wrap`/`.flex-nowrap`, `.justify-content-*`, `.align-items-*`, `.align-content-*`, `.flex-fill`, `.flex-grow-{0|1}`, `.flex-shrink-{0|1}`, `.gap-{0..5}` (and `.row-gap-*`/`.column-gap-*`). All accept breakpoint infixes.

**Position:** `.position-{static|relative|absolute|fixed|sticky}`, edge helpers `.top-0 .start-0 .end-0 .bottom-0`, centering with `.translate-middle`. `.sticky-top`, `.fixed-top`, `.fixed-bottom`, `.sticky-{bp}-top`.

**Sizing:** `.w-{25|50|75|100|auto}`, `.h-*`, `.mw-100`, `.mh-100`, `.vw-100`, `.vh-100`, `.min-vh-100`.

**Z-index & stacking:** utilities `.z-{0|1|2|3|n1}`; component layers use fixed CSS variables (modal, offcanvas, tooltip, etc.).

**Full-height app shell pattern:**
```html
<body class="d-flex flex-column min-vh-100">
  <header>…</header>
  <main class="flex-fill container py-4">…</main>   <!-- grows to fill -->
  <footer class="mt-auto">…</footer>
</body>
```
