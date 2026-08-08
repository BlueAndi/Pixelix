# Components

Source: [Components docs](https://getbootstrap.com/docs/5.3/components/). Markup patterns with required classes and `data-bs-*` attributes. Components marked **[JS]** need the Bootstrap JS bundle; **[Popper]** also need Popper (use `bootstrap.bundle.min.js`).

## Buttons

```html
<button type="button" class="btn btn-primary">Primary</button>
<button class="btn btn-outline-secondary btn-sm">Small outline</button>
<a class="btn btn-link" href="#">Link-style</a>
```
- Variants: `btn-{primary|secondary|success|danger|warning|info|light|dark|link}` and `btn-outline-{color}`.
- Sizes: `btn-lg`, `btn-sm`. Full width: `.d-grid` wrapper + `.btn` or `w-100`.
- States: `disabled` attribute (or `.disabled` on `<a>`), `.active`. Toggle: `data-bs-toggle="button"` **[JS]**.
- Button group: `<div class="btn-group" role="group">…</div>`.

## Navbar **[JS for toggler/collapse]**

```html
<nav class="navbar navbar-expand-lg bg-body-tertiary">
  <div class="container-fluid">
    <a class="navbar-brand" href="#">Brand</a>
    <button class="navbar-toggler" type="button"
            data-bs-toggle="collapse" data-bs-target="#nav1"
            aria-controls="nav1" aria-expanded="false" aria-label="Toggle navigation">
      <span class="navbar-toggler-icon"></span>
    </button>
    <div class="collapse navbar-collapse" id="nav1">
      <ul class="navbar-nav me-auto mb-2 mb-lg-0">
        <li class="nav-item"><a class="nav-link active" aria-current="page" href="#">Home</a></li>
        <li class="nav-item"><a class="nav-link" href="#">Link</a></li>
      </ul>
    </div>
  </div>
</nav>
```
- `navbar-expand-{sm|md|lg|xl|xxl}` sets where it switches from collapsed (hamburger) to horizontal. Omit to always be collapsed.
- Color: use background utilities (`bg-body-tertiary`, `bg-dark`) + `data-bs-theme="dark"` for dark navbars. The old `navbar-light` (deprecated 5.2) and `navbar-dark` (deprecated 5.3) are superseded by `data-bs-theme`.

## Nav & tabs **[JS for tabs]**

```html
<ul class="nav nav-tabs" role="tablist">
  <li class="nav-item" role="presentation">
    <button class="nav-link active" data-bs-toggle="tab" data-bs-target="#t1"
            type="button" role="tab" aria-selected="true">Tab 1</button>
  </li>
  <li class="nav-item" role="presentation">
    <button class="nav-link" data-bs-toggle="tab" data-bs-target="#t2" type="button" role="tab">Tab 2</button>
  </li>
</ul>
<div class="tab-content">
  <div class="tab-pane fade show active" id="t1" role="tabpanel">One</div>
  <div class="tab-pane fade" id="t2" role="tabpanel">Two</div>
</div>
```
Variants: `.nav-tabs`, `.nav-pills`, `.nav-underline`, `.nav-fill`, `.nav-justified`.

## Dropdown **[Popper]**

```html
<div class="dropdown">
  <button class="btn btn-secondary dropdown-toggle" data-bs-toggle="dropdown" aria-expanded="false">Menu</button>
  <ul class="dropdown-menu">
    <li><a class="dropdown-item" href="#">Action</a></li>
    <li><hr class="dropdown-divider"></li>
    <li><a class="dropdown-item" href="#">Other</a></li>
  </ul>
</div>
```
Direction: wrap in `.dropup`/`.dropend`/`.dropstart`. Align menu with `.dropdown-menu-end`.

## Card

```html
<div class="card" style="width: 18rem;">
  <img src="…" class="card-img-top" alt="…">
  <div class="card-body">
    <h5 class="card-title">Title</h5>
    <p class="card-text">Body.</p>
    <a href="#" class="btn btn-primary">Go</a>
  </div>
</div>
```
Pieces: `.card-header`, `.card-body`, `.card-footer`, `.card-title/subtitle/text/link`, `.card-img-top/bottom`, `.card-img-overlay`. Grid of cards: `row row-cols-* g-*` with a `.card` per `.col`, or `.card-group`.

## Modal **[JS]**

```html
<button class="btn btn-primary" data-bs-toggle="modal" data-bs-target="#m1">Open</button>

<div class="modal fade" id="m1" tabindex="-1" aria-labelledby="m1Label" aria-hidden="true">
  <div class="modal-dialog modal-dialog-centered modal-lg">
    <div class="modal-content">
      <div class="modal-header">
        <h1 class="modal-title fs-5" id="m1Label">Title</h1>
        <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
      </div>
      <div class="modal-body">…</div>
      <div class="modal-footer">
        <button class="btn btn-secondary" data-bs-dismiss="modal">Close</button>
        <button class="btn btn-primary">Save</button>
      </div>
    </div>
  </div>
</div>
```
Options via attrs on the `.modal`: `data-bs-backdrop="static"`, `data-bs-keyboard="false"`. Sizes `.modal-{sm|lg|xl}`, `.modal-fullscreen(-{bp}-down)`, `.modal-dialog-scrollable`. Programmatic: `bootstrap.Modal.getOrCreateInstance('#m1').show()`.

## Offcanvas **[JS]**

```html
<button class="btn btn-primary" data-bs-toggle="offcanvas" data-bs-target="#oc">Open</button>
<div class="offcanvas offcanvas-start" tabindex="-1" id="oc" aria-labelledby="ocLabel">
  <div class="offcanvas-header">
    <h5 class="offcanvas-title" id="ocLabel">Panel</h5>
    <button class="btn-close" data-bs-dismiss="offcanvas" aria-label="Close"></button>
  </div>
  <div class="offcanvas-body">…</div>
</div>
```
Placement: `offcanvas-{start|end|top|bottom}`. Responsive show-as-normal-above-bp: `.offcanvas-{bp}`.

## Collapse & accordion **[JS]**

```html
<a class="btn btn-primary" data-bs-toggle="collapse" href="#c1" role="button" aria-expanded="false" aria-controls="c1">Toggle</a>
<div class="collapse" id="c1"><div class="card card-body">Hidden content</div></div>
```
Accordion (single-open when `data-bs-parent` points at the accordion):
```html
<div class="accordion" id="acc">
  <div class="accordion-item">
    <h2 class="accordion-header">
      <button class="accordion-button collapsed" data-bs-toggle="collapse" data-bs-target="#a1"
              aria-expanded="false" aria-controls="a1">Item 1</button>
    </h2>
    <div id="a1" class="accordion-collapse collapse" data-bs-parent="#acc">
      <div class="accordion-body">…</div>
    </div>
  </div>
</div>
```
`.accordion-flush` removes borders/background. Keep multiple open by omitting `data-bs-parent`.

## Alerts **[JS to dismiss]**

```html
<div class="alert alert-warning alert-dismissible fade show" role="alert">
  Watch out!
  <button type="button" class="btn-close" data-bs-dismiss="alert" aria-label="Close"></button>
</div>
```
Colors: `alert-{color}`. `.alert-link` for links inside.

## Toast **[JS]**

```html
<div class="toast" role="alert" aria-live="assertive" aria-atomic="true">
  <div class="toast-header"><strong class="me-auto">App</strong><small>now</small>
    <button class="btn-close" data-bs-dismiss="toast" aria-label="Close"></button></div>
  <div class="toast-body">Saved.</div>
</div>
```
Show with `bootstrap.Toast.getOrCreateInstance(el).show()`. Position with a `.toast-container position-fixed`.

## Badges, list group, pagination, breadcrumb, progress, spinners

```html
<span class="badge text-bg-primary">New</span>
<span class="badge rounded-pill text-bg-danger">9</span>

<ul class="list-group">
  <li class="list-group-item active">One</li>
  <li class="list-group-item d-flex justify-content-between">Two <span class="badge text-bg-primary">2</span></li>
</ul>

<nav><ul class="pagination">
  <li class="page-item disabled"><a class="page-link">«</a></li>
  <li class="page-item active"><a class="page-link" href="#">1</a></li>
</ul></nav>

<nav aria-label="breadcrumb"><ol class="breadcrumb">
  <li class="breadcrumb-item"><a href="#">Home</a></li>
  <li class="breadcrumb-item active" aria-current="page">Now</li>
</ol></nav>

<div class="progress" role="progressbar" aria-valuenow="50" aria-valuemin="0" aria-valuemax="100">
  <div class="progress-bar" style="width: 50%">50%</div>
</div>

<div class="spinner-border text-primary" role="status"><span class="visually-hidden">Loading…</span></div>
```

## Carousel **[JS]**

```html
<div id="car" class="carousel slide" data-bs-ride="carousel">
  <div class="carousel-inner">
    <div class="carousel-item active"><img src="…" class="d-block w-100" alt="…"></div>
    <div class="carousel-item"><img src="…" class="d-block w-100" alt="…"></div>
  </div>
  <button class="carousel-control-prev" type="button" data-bs-target="#car" data-bs-slide="prev">
    <span class="carousel-control-prev-icon" aria-hidden="true"></span><span class="visually-hidden">Prev</span></button>
  <button class="carousel-control-next" type="button" data-bs-target="#car" data-bs-slide="next">
    <span class="carousel-control-next-icon" aria-hidden="true"></span><span class="visually-hidden">Next</span></button>
</div>
```
`data-bs-ride="carousel"` autoplays immediately on load; `data-bs-ride="true"` only cycles after the first user interaction. Add `.carousel-indicators` for dots and `data-bs-touch`/`data-bs-interval` to tune behavior.

## Tooltip & popover **[Popper] — opt-in, must init in JS**

```html
<button class="btn btn-secondary" data-bs-toggle="tooltip" data-bs-title="Hi!">Hover</button>
```
```js
document.querySelectorAll('[data-bs-toggle="tooltip"]')
  .forEach(el => new bootstrap.Tooltip(el));
```
Popovers are the same with `data-bs-toggle="popover"`, `data-bs-title`, `data-bs-content`, and `new bootstrap.Popover(el)`. HTML content is sanitized by default.

## Forms

```html
<form class="row g-3 needs-validation" novalidate>
  <div class="col-md-6">
    <label for="email" class="form-label">Email</label>
    <input type="email" class="form-control" id="email" required>
    <div class="invalid-feedback">Enter a valid email.</div>
  </div>
  <div class="col-md-6">
    <label for="role" class="form-label">Role</label>
    <select class="form-select" id="role" required>
      <option value="">Choose…</option><option>Admin</option>
    </select>
  </div>
  <div class="col-12">
    <div class="form-check">
      <input class="form-check-input" type="checkbox" id="agree" required>
      <label class="form-check-label" for="agree">Agree</label>
    </div>
  </div>
  <div class="col-12"><button class="btn btn-primary" type="submit">Submit</button></div>
</form>
```
- Controls: `.form-control` (text/textarea/file), `.form-select`, `.form-check` + `.form-check-input`/`.form-check-label`, `.form-check.form-switch` (toggle), `.form-range`, `.form-control-color`.
- Sizing: `.form-control-lg`/`-sm`. **Every control needs a `<label>`** (or `aria-label`) — see `html5-reference`.
- **Input group:** `<div class="input-group"><span class="input-group-text">@</span><input class="form-control"></div>`.
- **Floating labels:** `<div class="form-floating"><input class="form-control" id="x" placeholder="x"><label for="x">X</label></div>` (placeholder required).
- **Layout:** forms are just grid — use `.row`/`.col` and spacing utilities. Horizontal: `.row` + `.col-form-label`.
- **Validation:** add `.needs-validation novalidate`, then JS toggles `.was-validated` on submit; style hooks `.is-valid`/`.is-invalid` + `.valid-feedback`/`.invalid-feedback`. Native constraint validation drives it (see `javascript-reference` forms).

## Tables

```html
<table class="table table-striped table-hover align-middle">
  <thead><tr><th scope="col">#</th><th scope="col">Name</th></tr></thead>
  <tbody><tr><th scope="row">1</th><td>Ada</td></tr></tbody>
</table>
```
Modifiers: `.table-bordered`, `.table-borderless`, `.table-striped`(`-columns`), `.table-hover`, `.table-sm`, contextual `.table-{color}` on rows/cells, `.table-responsive` wrapper for horizontal scroll, `data-bs-theme="dark"` for dark tables.
