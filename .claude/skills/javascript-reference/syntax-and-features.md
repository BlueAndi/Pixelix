# Modern Syntax & Features (ES2015 → ES2025)

"Modern JavaScript" = everything since ES2015 (ES6). Grouped by capability, annotated with the edition that introduced each. All are widely supported in current engines.

## Variables, functions, parameters

- **`let` / `const`** (ES2015) — block scope, TDZ. Default to `const`. See [language-core.md](language-core.md).
- **Arrow functions** (ES2015) — `x => x*2`, `(a,b) => ({a,b})` (wrap object literals in parens). No own `this`/`arguments`/`super`/`new.target`; can't be `new`ed; no `prototype`.
- **Default parameters** (ES2015) — `function f(a, b = 2)`. Evaluated left-to-right at call time; earlier params are in scope.
- **Rest parameters** (ES2015) — `function f(...args)` collects into a real array (unlike `arguments`).
- **Spread** (ES2015 in calls/arrays; ES2018 in objects) — `f(...arr)`, `[...a, ...b]`, `{...o, x:1}`. **Shallow** copy. Later keys win in object spread.

## Destructuring (ES2015)

```js
const [a, , c = 10, ...rest] = arr;          // skip, default, rest
const { x, y: renamed, z = 0, ...others } = obj;
const { data: { items = [] } = {} } = resp;  // nested + defaults for missing
function f({ id, opts = {} } = {}) {}         // params, with default for whole arg
[a, b] = [b, a];                              // swap
```
Defaults apply only when the value is `undefined` (not `null`). Computed keys allowed: `const { [key]: v } = o`.

## Template literals (ES2015)

`` `Hi ${name}, ${1+2}` `` — interpolation + multiline. **Tagged templates**: `` tag`a${x}b` `` calls `tag(strings, ...values)` where `strings.raw` holds un-escaped text (used by `String.raw`, GraphQL/CSS-in-JS DSLs).

## Objects

- Shorthand properties/methods (ES2015): `{ x, greet() {} }`.
- Computed keys (ES2015): `{ [k]: v }`.
- Getters/setters: `{ get p(){}, set p(v){} }`.
- Spread/rest (ES2018): shown above.

## Operators for safety

- **Optional chaining `?.`** (ES2020) — `a?.b`, `a?.[k]`, `a?.()`. Short-circuits to `undefined` if the left is `null`/`undefined` (does not suppress other errors). `a?.b.c` only guards `a`.
- **Nullish coalescing `??`** (ES2020) — `a ?? b` yields `b` only when `a` is `null`/`undefined` (unlike `||`, which also replaces `0`/`""`/`false`). Cannot mix with `&&`/`||` without parens.
- **Logical assignment** (ES2021) — `a ??= b`, `a ||= b`, `a &&= b` (short-circuit + assign).
- **Exponentiation** `**` (ES2016). **Numeric separators** `1_000_000` (ES2021). **`globalThis`** (ES2020).

## Classes

- Core `class`/`extends`/`super`/`static` (ES2015) — see [language-core.md](language-core.md).
- **Public & private fields** (ES2022): `field = 1;`, `#secret = 2;`. Private methods/accessors `#m(){}`. **Static** fields/methods/blocks: `static x = …`, `static { … }` (ES2022). Ergonomic brand check: `#x in obj` (ES2022).

## Modules (ES2015, semantics refined since)

```js
export const a = 1;
export default function () {}
export { x as y } from "./m.js";     // re-export
import def, { a, b as bb } from "./m.js";
import * as ns from "./m.js";
const m = await import("./lazy.js"); // dynamic import → Promise
```
- **Live bindings**: imports reflect the exporter's current value (read-only). Not copies.
- Module scope is strict, deferred, and top-level `this` is `undefined`.
- **Top-level `await`** (ES2022) in modules.
- **Import attributes + JSON modules** (ES2025): `import data from "./d.json" with { type: "json" }`; dynamic form `import(url, { with: { type: "json" } })`.

## Iteration & generators

- `for...of` (ES2015) over any iterable; `for await...of` (ES2018) over async iterables. See [async-and-iteration.md](async-and-iteration.md).
- **Generators** `function*` / `yield` / `yield*` (ES2015). **Async generators** (ES2018).

## Regular expressions (evolving)

- Named groups `(?<year>\d{4})` + backrefs `\k<year>` (ES2018); lookbehind `(?<=…)`/`(?<!…)` (ES2018); dotAll `s` flag (ES2018); Unicode property escapes `\p{…}` with `u` flag (ES2018); `String.matchAll` (ES2020).
- `/d` **indices** flag → match `.indices` (ES2022). `/v` **unicodeSets** flag with set operations `[\p{L}--[a-z]]` (ES2024).
- **ES2025:** `RegExp.escape(str)`, **duplicate named groups** across alternatives, **inline modifiers** `(?ims:…)` / `(?-ims:…)`.

## ES2025 additions (full list)

| Feature | Notes |
|---------|-------|
| **Iterator helpers** | Methods on `Iterator.prototype` (lazy): `map, filter, take, drop, flatMap, reduce, toArray, forEach, some, every, find`; plus `Iterator.from(x)` to wrap any iterator/iterable. See [builtins.md](builtins.md). |
| **Set methods** | `union, intersection, difference, symmetricDifference, isSubsetOf, isSupersetOf, isDisjointFrom`. |
| **`Promise.try(fn, ...args)`** | Invoke `fn` and always get a Promise; synchronous throw → rejection. |
| **`RegExp.escape(s)`** | Escape for literal use in a pattern. |
| **Float16** | `Float16Array`; `DataView` `getFloat16`/`setFloat16`; `Math.f16round`. |
| **Duplicate named capture groups** | `(?<a>x)|(?<a>y)` legal when in different alternatives. |
| **Import attributes / JSON modules** | `with { type: "json" }`. |
| **RegExp modifiers** | Scoped inline flags. |

## Notable pre-2025 library additions (context)

- **ES2024:** `Object.groupBy` / `Map.groupBy`, `Promise.withResolvers()`, `Array.fromAsync`, resizable `ArrayBuffer` + `ArrayBuffer.prototype.transfer`, `String.prototype.isWellFormed`/`toWellFormed`, well-formed `JSON.stringify`.
- **ES2023:** `Array.prototype.{findLast,findLastIndex}`, **change-by-copy** `{toSorted,toReversed,toSpliced,with}`, `Array.prototype.group` was withdrawn (became `Object.groupBy` in 2024), hashbang grammar.
- **ES2022:** `.at()` on arrays/strings/typedarrays (negative indexing), `Object.hasOwn`, `Error.cause`, class fields/private/static blocks, top-level await.
- **ES2021:** `String.prototype.replaceAll`, `Promise.any` + `AggregateError`, `WeakRef`/`FinalizationRegistry`.
- **ES2020:** `Promise.allSettled`, `BigInt`, `globalThis`, `String.matchAll`, dynamic `import()`, `?.`, `??`.
- **ES2019:** `Array.flat`/`flatMap`, `Object.fromEntries`, `String.trimStart`/`trimEnd`, optional `catch` binding.
- **ES2017:** `async`/`await`, `Object.entries`/`values`, `String.padStart`/`padEnd`, `Object.getOwnPropertyDescriptors`.

## Emerging (Stage 3/4, NOT in ES2025 — do not assume availability)

- **`Temporal`** — modern date/time API (replaces `Date`).
- **Explicit Resource Management** — `using`/`await using` declarations + `Symbol.dispose`/`Symbol.asyncDispose` + `DisposableStack`.
- **Decorators** — `@decorator` on classes/members.
- **`Error.isError`**, **`Uint8Array` base64/hex**, **`Math.sumPrecise`**, **`Map.prototype.getOrInsert`**.

Verify support (engine version / caniuse) before using anything in this section.
