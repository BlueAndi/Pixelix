---
name: javascript-coding-guidelines
description: 'Write, refactor, and review JavaScript according to MDN JavaScript code-style guidance and ECMAScript 2025. Use when: modernizing legacy JavaScript, updating inline scripts, replacing var/function-expression-heavy code, improving async flow, choosing safe DOM APIs, reviewing naming/control flow, or adopting appropriate ECMAScript 2025 features.'
argument-hint: 'Describe the JavaScript files, target runtime constraints, and whether you want modernization, review, or a focused rewrite'
---

# JavaScript Coding Guidelines

## When to Use

Load this skill when asked to:
- Create or update `.js` files or inline JavaScript in HTML.
- Modernize legacy JavaScript to current ECMAScript patterns.
- Review JavaScript for readability, maintainability, safety, and standards alignment.
- Replace outdated async, DOM, array, object, string, or control-flow patterns.

## Core Principles

- Prefer standardized ECMAScript and Web APIs over deprecated, prefixed, or ad-hoc patterns.
- Optimize for readability first: short, semantic names and direct control flow.
- Use modern syntax when the target runtime supports it; do not introduce shiny syntax that the runtime cannot execute.
- Keep code compact and focused; add comments only when intent is not obvious.
- Avoid hidden behavior, broad fallbacks, and overly clever coercions.

## Default Rules

### Variables and naming

- Use `const` by default and `let` only when reassignment is required.
- Do not use `var`.
- Declare one variable per line.
- Use `camelCase` for variables, functions, properties, and methods.
- Use `PascalCase` for classes.
- Use short, semantic names; avoid non-obvious abbreviations, Hungarian notation, and type suffixes/prefixes.
- Name collections by content in plural form (`plugins`, `icons`, `users`), not by container type (`pluginList`, `iconArray`).

### Functions and methods

- Prefer function declarations for named functions.
- Use arrow functions for callbacks that do not need their own `this`.
- Do not use arrow functions for object methods.
- Prefer concise arrow callbacks with implicit return when that stays readable.
- Use method shorthand in object literals.

### Collections and objects

- Use array literals (`[]`) and object literals (`{}`), not `new Array()` or `new Object()`.
- Prefer `array.push(value)` over `array[array.length] = value`.
- Use object property shorthand when keys and variable names match.
- Use `class` syntax for classes and `extends` for inheritance.
- Prefer `Object.hasOwn()` over `Object.prototype.hasOwnProperty()`.

### Control flow and operators

- Always use braces with `if`, `for`, `while`, and similar statements.
- If an `if` branch ends with `return`, continue after it instead of adding `else`.
- Prefer the conditional operator for simple value selection or returns.
- Prefer strict equality operators (`===`, `!==`).
- Treat `== null` as the only acceptable loose-equality exception, and only when intentional.
- Prefer boolean shortcuts like `if (value)` and `if (!value)` unless truthy/falsy distinctions matter.

### Loops and iteration

- Prefer `for...of`, `.forEach()`, or other semantic array methods over index-based `for (;;)` loops when iterating collections.
- Prefer semantic methods like `.map()`, `.find()`, `.findIndex()`, `.includes()`, `.every()`, and similar when they express intent more clearly.
- Never use `for...in` with arrays or strings.
- Use `const` in `for...of` loops and `let` for counter-based loops.

### Strings and coercion

- Use template literals for interpolation.
- Do not use template literals when a plain string literal is sufficient.
- Avoid implicit coercion tricks such as `"" + value` or `+value`.
- Use `String(value)` and `Number(value)` for explicit conversion.

### Async code

- Prefer `async`/`await` over long Promise chains when both are practical.
- Keep Promise-based APIs explicit; surface errors instead of swallowing them.
- Use `Promise.all()` for independent concurrent work and `Promise.allSettled()` when partial failure is acceptable.
- Use top-level `await` only when the file is an ECMAScript module and the runtime/toolchain supports it.

### Comments

- Use comments to explain intent, constraints, or non-obvious behavior.
- Prefer single-line `//` comments.
- Put comments on their own lines above the code they describe.
- Do not restate obvious code in prose.
- Use comment-based ellipses for intentionally omitted code in examples.

### DOM and Web API safety

- Prefer `fetch()` over `XMLHttpRequest`.
- Prefer `textContent` for textual DOM updates.
- Avoid `innerHTML` for plain text or where safer DOM construction is practical.
- Avoid browser prefixes and deprecated APIs unless the task is explicitly about them.

## ECMAScript 2025 Guidance

- Consider ECMAScript 2025 features when they improve clarity and the target runtime supports them.
- Especially relevant additions include:
  - `Iterator` helpers for clearer iterator pipelines
  - New `Set` operations for common set math
  - `RegExp.escape()` for safe dynamic regular expressions
  - `Promise.try()` when normalizing sync-or-async execution paths
  - JSON modules and import attributes where module tooling/runtime supports them
- If runtime support is uncertain, prefer broadly supported modern syntax and mention the compatibility constraint explicitly.

## Procedure

1. Identify the target runtime
- Browser, embedded web UI, Node.js, bundler/transpiler, or mixed environment.
- Determine whether ECMAScript 2025-only features are safe to use directly.

2. Audit for legacy patterns
- Look for `var`, constructor-style arrays/objects, loose equality, implicit coercion, callback-heavy async flow, `for...in` on arrays, `innerHTML`, XHR, and prefixed/deprecated APIs.

3. Refactor to modern patterns
- Replace legacy syntax with `const`/`let`, declarations, literals, method shorthand, template literals, `async`/`await`, and safer DOM/Web APIs.
- Keep behavior equivalent unless the request explicitly asks for a behavior change.

4. Validate clarity and compatibility
- Confirm names are semantic and control flow is direct.
- Confirm any ECMAScript 2025 feature fits the runtime support expectations.
- Avoid growing small files with unnecessary wrappers, abstractions, or documentation.

5. Verify output
- Ensure the updated code still works and that errors are surfaced clearly.
- Re-check that no deprecated or unsafe pattern was reintroduced while fixing another issue.

## Review Checklist

- [ ] `const`/`let` used appropriately; no `var`
- [ ] Names are semantic, concise, and convention-aligned
- [ ] Arrays/objects/classes use modern literal or class syntax
- [ ] Control flow is simple, braced, and uses strict equality
- [ ] Async flow is readable and errors are not hidden
- [ ] DOM updates use safe APIs where practical
- [ ] ECMAScript 2025 features are used only when runtime support is acceptable

## References

- [MDN JavaScript code style guide](https://developer.mozilla.org/en-US/docs/MDN/Writing_guidelines/Code_style_guide/JavaScript)
- [ECMAScript 2025 specification](https://tc39.es/ecma262/2025/)
