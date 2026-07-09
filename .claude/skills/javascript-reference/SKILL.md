---
name: javascript-reference
description: Use when writing, reviewing, or debugging JavaScript — reasoning about type coercion, equality, `this`, closures, scope/hoisting, prototypes, async/promises, iterators, modules, or the standard built-in objects, and when checking behavior against the ECMA-262 (ECMAScript 2025) language standard. Covers the language itself; host/runtime APIs (DOM, Node, browser) are out of scope.
---

# JavaScript Reference (ECMA-262 / ECMAScript 2025)

## Overview

[ECMA-262](https://tc39.es/ecma262/2025/) defines the **JavaScript language**: its syntax, type system, evaluation semantics, and standard built-in objects. It does **not** define the DOM, `fetch`, `console`, Node's `fs`, timers, or modules resolution — those are *host* environments layered on top. This skill is the condensed-but-authoritative reference for the language itself.

**Core principle:** Most JS bugs come from a handful of semantics that are precise but unintuitive — coercion, `==` vs `===`, `this` binding, closures over mutable bindings, floating-point, and reference vs value semantics. Reason from the spec's rules, not from what "looks right."

ECMAScript is versioned yearly; **ES2025 is the 16th edition**. Features here are annotated with the edition that introduced them where it matters. When behavior is contested, the linked spec is the source of truth.

## When to Use

- Deciding **why** a coercion/comparison/`this` value is what it is, or predicting output of a tricky snippet.
- Choosing the right **language construct**: `let`/`const`/`var`, `==`/`===`, `for...of`/`for...in`/`forEach`, class fields vs prototype, `Map` vs object.
- Writing correct **async** code: promises, `async`/`await`, the microtask/task ordering, error propagation, cancellation.
- Using the **standard library** correctly (Array/Object/String/Map/Set/Reflect/Proxy/typed arrays/Intl) including ES2025 additions.
- Reviewing code for subtle correctness bugs (mutation aliasing, `NaN` checks, integer precision, prototype pollution).

**Not for:** DOM/BOM, Node/Deno/Bun APIs, `fetch`/network, bundlers, TypeScript types, or framework patterns. Those are host/tooling concerns, not ECMA-262.

## The Mental Model (read this first)

**Everything is one of 8 language types.** 7 primitives + object:

| Type | `typeof` | Notes |
|------|----------|-------|
| Undefined | `"undefined"` | sole value `undefined` |
| Null | `"object"` ⚠️ | sole value `null`; the `typeof null` bug is spec-mandated |
| Boolean | `"boolean"` | `true` / `false` |
| Number | `"number"` | IEEE-754 double; `NaN`, `Infinity`, `-0` all exist |
| BigInt | `"bigint"` | arbitrary precision, literal `10n`; never mixes with Number in arithmetic |
| String | `"string"` | immutable sequence of UTF-16 **code units** (not code points) |
| Symbol | `"symbol"` | unique immutable identifier |
| Object | `"object"` / `"function"` | callable objects report `"function"` |

**Primitives are immutable and compared by value; objects are compared by reference.** Assignment and argument passing copy the *reference* for objects — mutations are shared (aliasing).

Full details → **[language-core.md](language-core.md)** (types, coercion, equality, scope, `this`, prototypes).

## Quick Reference

**Falsy values (exactly 8)** — everything else is truthy (including `"0"`, `"false"`, `[]`, `{}`):
```
false   0   -0   0n   ""   null   undefined   NaN
```

**`===` (strict)** — no coercion; different types ⇒ not equal. `NaN === NaN` is `false`. `+0 === -0` is `true`.
**`==` (loose)** — coerces; avoid it. Only defensible use: `x == null` (true for `null` *and* `undefined`).
**`Object.is`** — like `===` but `Object.is(NaN, NaN)` is `true` and `Object.is(+0, -0)` is `false`.

**Declarations:**

| | Scope | Hoist | Reassign | Redeclare | TDZ |
|--|-------|-------|----------|-----------|-----|
| `var` | function | yes (→`undefined`) | yes | yes | no |
| `let` | block | yes (uninit) | yes | no | **yes** |
| `const` | block | yes (uninit) | no* | no | **yes** |

*`const` binds the binding, not the value — a `const` object is still mutable.

**Number checks:** `Number.isNaN(x)` (not global `isNaN`), `Number.isInteger`, `Number.isSafeInteger`, `Number.MAX_SAFE_INTEGER` (2⁵³−1). `0.1 + 0.2 !== 0.3`.

## ES2025 Highlights (16th edition)

| Feature | What it gives you |
|---------|-------------------|
| **Iterator helpers** | Lazy `Iterator.prototype.{map,filter,take,drop,flatMap,reduce,toArray,forEach,some,every,find}`, `Iterator.from()` — compose over any iterator without materializing arrays. |
| **Set methods** | `Set.prototype.{union,intersection,difference,symmetricDifference,isSubsetOf,isSupersetOf,isDisjointFrom}`. |
| **`Promise.try(fn)`** | Run a possibly-sync-throwing fn and get a promise (sync throws become rejections). |
| **`RegExp.escape(str)`** | Escape a string for safe literal use inside a `RegExp`. |
| **Float16** | `Float16Array`, `DataView.prototype.{getFloat16,setFloat16}`, `Math.f16round`. |
| **Duplicate named groups** | Same `(?<name>…)` in different regex alternatives. |
| **Import attributes + JSON modules** | `import cfg from "./c.json" with { type: "json" }`. |
| **RegExp modifiers** | Inline flags: `(?i:…)` / `(?-i:…)` scope `i`/`m`/`s` to part of a pattern. |

Details and pre-2025 modern features (ES2015→2024) → **[syntax-and-features.md](syntax-and-features.md)**.

## Reference Files

| File | Contents |
|------|----------|
| **[language-core.md](language-core.md)** | Types & values, coercion rules (`ToPrimitive`/`ToNumber`/`ToString`), equality algorithms, truthiness, scope/hoisting/TDZ, closures, `this` binding, prototypes & inheritance, strict mode, `class` internals. |
| **[syntax-and-features.md](syntax-and-features.md)** | Modern syntax edition-by-edition (ES2015→ES2025): destructuring, spread/rest, arrow functions, classes & private fields, modules, optional chaining, nullish coalescing, logical assignment, generators, and the full ES2025 additions. |
| **[async-and-iteration.md](async-and-iteration.md)** | Job queue (microtask vs macrotask), Promises + combinators, `async`/`await`, error propagation, iteration protocols, generators, async iterators, `for await…of`. |
| **[builtins.md](builtins.md)** | Standard library: `Object`, `Array`, `String`, `Number`/`Math`, `Map`/`Set`/`WeakMap`/`WeakSet`/`WeakRef`, `JSON`, `RegExp`, `Symbol`, `Reflect`, `Proxy`, typed arrays/`ArrayBuffer`, `Intl`, `Date` (and the emerging `Temporal`). |

## Common Mistakes

| Mistake | Reality / fix |
|---------|---------------|
| `if (x == null)` conflated with all falsy checks | `== null` matches only `null`/`undefined`; `0`/`""`/`NaN` are not null. Use it deliberately. |
| `arr.indexOf(NaN)` / `=== NaN` | `NaN` is never equal to anything. Use `Number.isNaN`, `arr.includes(NaN)`, or `Array.prototype.findIndex`. |
| Expecting `0.1 + 0.2 === 0.3` | Floating point. Compare with an epsilon, or use integers/`BigInt`/decimal libs. |
| `this` "lost" in a callback | `this` is set by *call site*, not definition (except arrow fns, which capture lexically). Use arrow, `.bind`, or class fields. |
| `var`/`let` in a loop with async callbacks | `var` shares one binding; `let`/`const` create a fresh binding per iteration. Prefer `let`/`const`. |
| Mutating a shared object/array thinking it's copied | Objects pass by reference. Copy explicitly (spread, `structuredClone`, `Array.from`). Spread is **shallow**. |
| `for...in` to iterate arrays | `for...in` enumerates string keys incl. inherited; order not guaranteed for integer-like intent. Use `for...of`, `forEach`, or `entries()`. |
| `sort()` without a comparator on numbers | Default sort is by UTF-16 string order: `[1,2,10].sort()` → `[1,10,2]`. Pass `(a,b)=>a-b`. (TypedArray `sort` is numeric by default — the opposite.) |
| `typeof x === "object"` to detect objects | `null` is also `"object"`. Use `x !== null && typeof x === "object"`, or `Array.isArray`, etc. |
| Adding a property to `Object.prototype` | Prototype pollution; breaks `for...in`/`in` everywhere. Never. |
| `async` function "returns a value" | It returns a Promise. Callers must `await`/`.then`. Unhandled rejections are silent-ish. |
| `await` inside a `forEach` | `forEach` ignores the returned promise. Use `for…of` with `await`, or `Promise.all(map(...))`. |

## Spec Navigation

- Types & values: [§6](https://tc39.es/ecma262/2025/#sec-ecmascript-data-types-and-values) · Abstract ops (coercion): [§7](https://tc39.es/ecma262/2025/#sec-abstract-operations)
- Expressions: [§13](https://tc39.es/ecma262/2025/#sec-ecmascript-language-expressions) · Statements/declarations: [§14](https://tc39.es/ecma262/2025/#sec-ecmascript-language-statements-and-declarations) · Functions & classes: [§15](https://tc39.es/ecma262/2025/#sec-ecmascript-language-functions-and-classes)
- Scripts & modules: [§16](https://tc39.es/ecma262/2025/#sec-ecmascript-language-scripts-and-modules)
- Built-ins: [Global §19](https://tc39.es/ecma262/2025/#sec-global-object) · [Fundamental §20](https://tc39.es/ecma262/2025/#sec-fundamental-objects) · [Numbers/Dates §21](https://tc39.es/ecma262/2025/#sec-numbers-and-dates) · [Text/RegExp §22](https://tc39.es/ecma262/2025/#sec-text-processing) · [Collections §24](https://tc39.es/ecma262/2025/#sec-keyed-collections) · [Control abstractions (iterators/promises) §27](https://tc39.es/ecma262/2025/#sec-control-abstraction-objects)
