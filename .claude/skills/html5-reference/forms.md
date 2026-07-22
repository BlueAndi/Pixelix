# Forms, Controls & Constraint Validation

Source: [forms](https://html.spec.whatwg.org/multipage/forms.html) · [the input element](https://html.spec.whatwg.org/multipage/input.html) · [constraints](https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#constraints).

## The `<form>` and its controls

```html
<form action="/subscribe" method="post">
  <fieldset>
    <legend>Subscribe</legend>
    <label for="email">Email</label>
    <input id="email" name="email" type="email" required autocomplete="email">
    <button type="submit">Sign up</button>
  </fieldset>
</form>
```

`<form>` attributes: `action`, `method` (`get`|`post`|`dialog`), `enctype` (`application/x-www-form-urlencoded` | `multipart/form-data` | `text/plain`), `target`, `name`, `autocomplete`, `novalidate` (skip native validation *and* its error UI — only add when you fully replace it with custom validation), `rel`, `accept-charset`.

**Labelable elements** (what a `<label>` may be associated with, and the "one control per label" limit counts): `button`, `input` (except `type=hidden`), `meter`, `output`, `progress`, `select`, `textarea`.

**Every control needs a `name`** to be submitted. Controls associate with a form by being descendants, or via the `form="formId"` attribute (allows placing a control outside the form element).

| Element | Role |
|---------|------|
| `label` | Caption for **exactly one** control (at most one labelable descendant; no nested `<label>`). Associate via `for="controlId"` **or** by wrapping the control. Clicking the label focuses/activates the control. |
| `fieldset` | Groups controls; `disabled` disables all descendants; `name`; `form`. |
| `legend` | Caption for the `fieldset` (must be first child). |
| `button` | `type="submit"` (default in a form), `reset`, or `button`. Also `formaction`/`formmethod`/`formenctype`/`formtarget`/`formnovalidate`, `name`, `value`, `popovertarget`, `command`/`commandfor`. |
| `select` | Drop-down/list box. `multiple`, `size`, `required`, `name`, `autocomplete`, `disabled`. Contains `option`/`optgroup`. |
| `optgroup` | Group of options (`label`, `disabled`). |
| `option` | `value`, `selected`, `disabled`, `label`. |
| `datalist` | Suggestion list referenced by an input's `list=` attribute; contains `option`s. |
| `textarea` | Multiline text. `rows`, `cols`, `wrap` (`soft`/`hard`), `maxlength`, `minlength`, `placeholder`, `readonly`, `required`, `dirname`, `autocomplete`. Content = the default value (raw text). |
| `output` | Result of a calculation. `for` (space-separated ids), `name`, `form`. |
| `progress` | Task progress. `value`, `max` (indeterminate if no `value`). |
| `meter` | Scalar measurement within a range. `value` (req), `min`, `max`, `low`, `high`, `optimum`. Not for progress. |

## `<input>` types

Choose the type by **data meaning** — you get validation, the right virtual keyboard, and native pickers for free. `empty` content model. Common to text-like types: `autocomplete`, `dirname`, `list`, `maxlength`, `minlength`, `pattern`, `placeholder`, `readonly`, `required`, `size`.

| `type` | Purpose | Type-specific attributes |
|--------|---------|--------------------------|
| `text` | One-line plain text | list, maxlength, minlength, pattern, placeholder, readonly, required, size, dirname |
| `search` | Search field (styled) | same as `text` |
| `tel` | Telephone number (no format enforcement — use `pattern`) | same as `text` |
| `url` | Absolute URL | same as `text` |
| `email` | Email address(es) | + `multiple`; validates format |
| `password` | Obscured text | maxlength, minlength, pattern, placeholder, readonly, required, size, autocomplete |
| `number` | Numeric value | `min`, `max`, `step`, placeholder, readonly, required, list |
| `range` | Imprecise numeric slider | `min`, `max`, `step`, list |
| `date` | Calendar date | `min`, `max`, `step`, readonly, required, list |
| `month` | Year + month | `min`, `max`, `step`, readonly, required, list |
| `week` | Year + week number | `min`, `max`, `step`, readonly, required, list |
| `time` | Time of day | `min`, `max`, `step`, readonly, required, list |
| `datetime-local` | Date + time, no timezone | `min`, `max`, `step`, readonly, required, list |
| `color` | sRGB color | `alpha`, `colorspace` |
| `checkbox` | Independent on/off | `checked`, `required` |
| `radio` | One-of-many (same `name`) | `checked`, `required` |
| `file` | File upload | `accept` (MIME/ext), `multiple`, `required`, `capture` |
| `hidden` | Non-visible value | `autocomplete`, `dirname` |
| `submit` | Submit button | `formaction`, `formenctype`, `formmethod`, `formnovalidate`, `formtarget` |
| `image` | Image submit button | `src`, `alt`, `width`, `height`, + the `form*` overrides |
| `reset` | Reset button | — |
| `button` | Generic button (needs JS) | — |

### Attribute meanings
- `required` — must have a value to submit.
- `min`/`max` — range bounds (numbers, dates, times). `step` — granularity; default is `1` for `number`/`range`/`date` etc., so omitting it already forces integer/whole-unit values. `step="any"` disables snapping (allows decimals).
- `minlength`/`maxlength` — UTF-16 code-unit length limits (text-like).
- `pattern` — full-match JS regex (no anchors needed); pair with `title` to describe the requirement.
- `placeholder` — short hint, **not** a label substitute.
- `readonly` — value submitted but not editable; `disabled` — not editable **and not submitted**, removed from tab order and validation.
- `multiple` — several values (`email`, `file`).
- `list` — id of a `<datalist>` for suggestions.
- `autocomplete` — token(s) like `email`, `name`, `username`, `current-password`, `one-time-code`, `off`.
- `inputmode` (global) — virtual-keyboard hint (`numeric`, `decimal`, `tel`, `email`, `url`, `search`).
- `capture` — camera/mic source for `file`.

## Native constraint validation

The browser validates on submit (unless the form has `novalidate` or the submitter has `formnovalidate`). Validity is driven by: `required`, `type` format (`email`/`url`), `pattern`, `min`/`max`/`step`, `minlength`/`maxlength`.

**Constraint Validation API (per control):**
```js
input.checkValidity();          // false if invalid (fires 'invalid' event), else true
input.reportValidity();         // like above but also shows the browser bubble
input.validity;                 // ValidityState flags:
//   valueMissing, typeMismatch, patternMismatch, tooLong, tooShort,
//   rangeUnderflow, rangeOverflow, stepMismatch, badInput, customError, valid
input.validationMessage;        // localized message string
input.setCustomValidity('...'); // '' clears; any string marks invalid
input.willValidate;             // whether it participates in validation
form.checkValidity();           // validate whole form
form.reportValidity();
```

Style with the `:required`, `:optional`, `:valid`, `:invalid`, `:user-valid`, `:user-invalid`, `:in-range`, `:out-of-range`, `:placeholder-shown` CSS pseudo-classes. Prefer `:user-invalid` for showing errors only after interaction.

```js
form.addEventListener('submit', (e) => {
  if (!form.checkValidity()) {
    e.preventDefault();
    form.reportValidity();
  }
});
```

## Accessibility musts
- Every control has a programmatic label (`<label for>`, wrapping `<label>`, or `aria-label`/`aria-labelledby`). `placeholder` is not a label.
- Group related radios/checkboxes in a `<fieldset>` with a `<legend>`.
- Associate error/help text with `aria-describedby`, and set `aria-invalid="true"` on a control while it fails validation (clear it when fixed). `reportValidity()`'s native bubble focuses the first invalid control but is transient — for a robust accessible form, render persistent inline errors, link them with `aria-describedby`, and move focus to the first invalid field yourself.
- Don't rely on color alone to signal validity.
