---
name: html5-bootstrap
description: 'Create, refactor, and review HTML using the HTML Living Standard and Bootstrap 5.3. Use when: building semantic HTML5 pages, restructuring markup, improving responsive layout, standardizing forms/navigation, adding Bootstrap 5.3 components, or cleaning up non-semantic legacy HTML.'
argument-hint: 'Describe the HTML file(s), page structure, Bootstrap usage, and any responsive or accessibility constraints'
---

# HTML5 + Bootstrap 5.3 Guidelines

## When to Use

Load this skill when asked to:
- Create or update `.html` files.
- Refactor legacy markup to semantic HTML5 structure.
- Apply or review Bootstrap 5.3 layout and component usage.
- Improve responsive behavior, form markup, navigation, or page structure.
- Clean up pages with overly generic `<div>` wrappers or inconsistent document structure.

## Core Principles

- Prefer semantic HTML elements over generic containers when they express meaning clearly.
- Start with valid, minimal HTML structure and add Bootstrap classes on top of that structure.
- Preserve document meaning first; use Bootstrap for layout and styling, not to replace semantics.
- Keep markup predictable, readable, and easy to maintain.
- Favor native HTML behavior and accessibility attributes before adding custom workarounds.

## Default Rules

### Document structure

- Use the HTML5 doctype: `<!doctype html>`.
- Always include `<html lang="...">`.
- Include `<meta charset="utf-8">`.
- Include the responsive viewport meta tag:
  - `<meta name="viewport" content="width=device-width, initial-scale=1">`
- Keep document structure complete and ordered: `html`, `head`, `body`.
- Put CSS `<link>` elements in `<head>`.
- Put Bootstrap JavaScript near the end of `<body>`.

### Semantic HTML5

- Prefer `header`, `main`, `section`, `article`, `aside`, `nav`, and `footer` where they match the content.
- Use heading levels in order and avoid skipping levels without reason.
- Use `button` for actions and `a` for navigation.
- Use proper form semantics: `form`, `label`, `input`, `select`, `textarea`, and helpful attributes like `for`, `id`, `name`, `type`, `required`, `min`, `max`, and `placeholder` when appropriate.
- Use lists (`ul`, `ol`) for grouped items rather than freeform line breaks.
- Use `figure` and `figcaption` when media has a caption.
- Keep `alt` text meaningful for informative images and empty only for decorative images.

### Content model and validity

- Respect allowed nesting and content-model rules from the HTML Living Standard.
- Do not place interactive controls inside incompatible interactive elements unless explicitly valid.
- Avoid obsolete or purely presentational markup when semantic alternatives exist.
- Prefer attributes and elements with standard meaning over custom data encoding in generic markup.

### Bootstrap 5.3 usage

- Use Bootstrap’s layout primitives first: `container`, `container-fluid`, grid rows/columns, spacing utilities, and display helpers.
- Use Bootstrap components as designed instead of recreating them with custom markup.
- Keep component structure consistent with Bootstrap expectations so JS behavior continues to work.
- Include `bootstrap.bundle.min.js` when components require Bootstrap JavaScript behavior.
- If dropdowns, popovers, or tooltips are not used, separate JS loading can be considered to reduce payload.
- Use Bootstrap utility classes for spacing, alignment, sizing, and visibility before adding custom CSS.

### Accessibility and interaction

- Ensure landmarks are meaningful and not duplicated without purpose.
- Use `aria-*` attributes only when needed and consistent with the native element behavior.
- Keep labels associated with controls.
- Preserve keyboard accessibility for toggles, dialogs, menus, and navigation.
- Use descriptive link and button text.

### Maintainability

- Avoid deeply nested wrapper `<div>` elements when semantic grouping is available.
- Keep class lists purposeful; remove unused or contradictory classes.
- Preserve project-specific HTML tokens and placeholders exactly as required.
- Avoid inline event handlers when a separate script can attach behavior cleanly.

## Bootstrap 5.3 Guidance

- Bootstrap is mobile-first: design base structure for small screens first, then layer breakpoint-specific classes.
- Use the viewport meta tag so responsive CSS behaves correctly on mobile devices.
- Use the Bootstrap bundle when the page needs component JavaScript behavior.
- Follow documented component structure for navbars, modals, dropdowns, forms, tabs, accordions, and offcanvas elements.
- Prefer Bootstrap’s normalization and utility system over ad-hoc structural hacks.

## HTML Living Standard Guidance

- Use the document outline intentionally with clear sectioning and heading structure.
- Prefer native HTML semantics and built-in behavior over generic containers plus script.
- Keep resource metadata (`charset`, `viewport`, `title`, links, scripts) in the correct document locations.
- Respect standard attribute meanings and value types.
- Avoid obsolete features and invalid nesting.

## Procedure

1. Audit the page structure
- Check doctype, `lang`, `charset`, `viewport`, `title`, and head/body ordering.
- Identify where generic wrappers should become semantic elements.

2. Review layout and components
- Map page regions to semantic landmarks.
- Check whether layout should use Bootstrap containers, grid, spacing utilities, or built-in components.

3. Refactor markup
- Replace non-semantic structure with semantic HTML5 where it preserves behavior.
- Normalize Bootstrap classes and component markup to documented structure.

4. Validate accessibility and responsiveness
- Confirm labels, landmarks, heading order, button/link semantics, and responsive behavior.
- Ensure required Bootstrap JS dependencies match the components present.

5. Verify output
- Confirm the page remains valid, readable, and behaviorally consistent.
- Re-check that template tokens, IDs, ARIA references, and Bootstrap targets still match.

## Review Checklist

- [ ] HTML5 document structure is complete and correctly ordered
- [ ] Semantic landmarks and headings fit the content
- [ ] Forms and controls use proper native HTML elements
- [ ] Bootstrap layout/components follow Bootstrap 5.3 patterns
- [ ] Responsive viewport and mobile-first structure are in place
- [ ] Accessibility semantics remain intact
- [ ] Obsolete, invalid, or purely presentational markup has been reduced

## References

- [Bootstrap 5.3 introduction](https://getbootstrap.com/docs/5.3/getting-started/introduction/)
- [HTML Living Standard overview](https://html.spec.whatwg.org/)
