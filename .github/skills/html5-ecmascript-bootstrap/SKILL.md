---
name: html5-ecmascript-bootstrap
description: 'Design, refactor, and optimize web UI using HTML5, ECMAScript 2025, and Bootstrap 5.3. Use when: restructuring static pages, reducing concurrent asset requests, improving load behavior on constrained devices, modernizing JavaScript patterns, and keeping Bootstrap-based layouts consistent.'
argument-hint: 'Describe the page(s), requested UI behavior, and performance/load constraints'
---

# HTML5 + ECMAScript 2025 + Bootstrap 5.3 Workflow

## When to Use

Load this skill when asked to:
- Create or update `.html`, `.css`, or `.js` files in this repository.
- Reduce request fan-out and startup load on constrained embedded web servers.
- Modernize frontend logic to ECMAScript 2025 language features where practical.
- Keep Bootstrap 5.3 markup and component behavior consistent.

## Ground Rules

- Prefer semantic HTML5 structure (`header`, `main`, `section`, `footer`, form semantics).
- Use Bootstrap 5.3 classes/components first; avoid custom CSS unless needed.
- Keep JavaScript modular and predictable; avoid hidden global side effects.
- For constrained targets, optimize for fewer connections and fewer round-trips.

## Procedure

1. Audit resource loading
- Inventory CSS, JS, image, and font requests per page.
- Identify common includes that can be bundled into shared assets.
- Separate critical-path assets from optional/lazy-loaded assets.

2. Restructure includes
- Merge repeated CSS into a shared stylesheet where behavior is identical.
- Merge repeated JS helpers into shared bundles while preserving dependency order.
- Keep page-specific code in page-local files or inline blocks only when needed.

3. Apply ECMAScript 2025 practices
- Prefer `const`/`let`, strict equality, and clear promise/async handling.
- Keep APIs explicit and defensive; validate arguments for public helpers.
- Avoid broad fallbacks that hide failures.

4. Validate Bootstrap behavior
- Verify navbar, dropdowns, tooltips, modals, and form components still behave correctly.
- Preserve responsive behavior (`container`, grid, spacing utilities, breakpoints).

5. Verify performance and reliability
- Compare request count before/after on key pages.
- Confirm no missing assets or dependency-order issues.
- Keep compatibility with project-specific template tokens (`~...~`) used in HTML pages.

## Quality Checklist

- [ ] Shared assets are actually reused by multiple pages.
- [ ] Request fan-out is reduced on the main pages.
- [ ] Bootstrap interactions still work (menu, tooltips, dialogs).
- [ ] ECMAScript changes stay readable and maintainable.
- [ ] No page lost required functionality due to bundling.

## References

- [ECMAScript 2025 specification](https://tc39.es/ecma262/2025/)
- [Bootstrap 5.3 introduction](https://getbootstrap.com/docs/5.3/getting-started/introduction/)
- [HTML Living Standard overview](https://html.spec.whatwg.org/)
