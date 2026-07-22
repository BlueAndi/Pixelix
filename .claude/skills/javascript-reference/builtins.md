# Standard Built-in Objects

The ECMA-262 standard library. Source: [§19–§28](https://tc39.es/ecma262/2025/#sec-global-object). Edition tags note when a method arrived; unmarked members are long-standing.

## Object

- Create/inspect: `Object.create(proto[,props])`, `Object.getPrototypeOf`/`setPrototypeOf`, `Object.assign(t,...src)` (shallow, own enumerable), `Object.fromEntries` (ES2019), `Object.keys` (ES5), `Object.entries`/`values` (ES2017).
- Descriptors: `Object.defineProperty`/`defineProperties`, `getOwnPropertyDescriptor(s)`, `getOwnPropertyNames`, `getOwnPropertySymbols`.
- Integrity: `Object.freeze` (shallow!), `isFrozen`, `seal`, `isSealed`, `preventExtensions`, `isExtensible`.
- Membership: `Object.hasOwn(o,k)` (ES2022, prefer over `hasOwnProperty`), `Object.groupBy(items, fn)` (ES2024).
- Iteration order: integer-like keys ascending, then string keys in insertion order, then symbols.

## Array

- Create: `Array.of(...)`, `Array.from(iterableOrArrayLike[, mapFn])`, `Array.fromAsync` (ES2024), `Array.isArray`.
- Non-mutating: `map, filter, reduce, reduceRight, slice, concat, flat`/`flatMap` (ES2019), `join, includes` (ES2016, SameValueZero), `indexOf`/`lastIndexOf` (`===`), `find`/`findIndex`, `findLast`/`findLastIndex` (ES2023), `some, every, at` (ES2022, negative index), `entries, keys, values`.
- **Change-by-copy** (ES2023): `toSorted, toReversed, toSpliced, with(i,v)` — return a new array, leaving the original intact.
- Mutating (return-in-place): `push, pop, shift, unshift, splice, sort, reverse, fill, copyWithin`.
- ⚠️ `sort()` defaults to **string** comparison — `[1, 2, 10].sort()` → `[1, 10, 2]`. Use `arr.sort((a,b)=>a-b)` for numbers. `sort` is stable (since ES2019). (Note: `%TypedArray%.prototype.sort` defaults to **numeric** order — the opposite of `Array`.) `length` is writable (assigning truncates). Sparse arrays skip holes in some iterators.

## String

Immutable, UTF-16. `length, charAt, charCodeAt` (code unit), `codePointAt`/`fromCodePoint` (code point), `at` (ES2022), `slice`/`substring` (avoid `substr`), `indexOf`/`includes`/`startsWith`/`endsWith`, `padStart`/`padEnd` (ES2017), `trim`/`trimStart`/`trimEnd` (ES2019), `repeat`, `split`, `replace`/`replaceAll` (ES2021), `match`/`matchAll` (ES2020)/`search`, `normalize` (Unicode), `localeCompare`, `isWellFormed`/`toWellFormed` (ES2024). `String.raw` for tagged templates. Iterate by code point: `for (const ch of str)`.

## Number & Math

- `Number.isNaN`/`isFinite`/`isInteger`/`isSafeInteger` (type-safe; unlike global `isNaN`/`isFinite` which coerce). Constants: `MAX_SAFE_INTEGER`, `MIN_SAFE_INTEGER`, `EPSILON`, `MAX_VALUE`, `MIN_VALUE`, `POSITIVE/NEGATIVE_INFINITY`, `NaN`.
- Formatting: `toFixed`, `toPrecision`, `toString(radix)`. Parsing: `Number(x)` (strict), `parseInt(s, radix)` (**always pass radix**), `parseFloat`.
- `Math`: `abs, ceil, floor, round` (half-up), `trunc, sign, sqrt, cbrt, pow, hypot, log/log2/log10, exp, min/max, random` (in `[0,1)`), `clz32`, `fround`, **`f16round`** (ES2025), trig.
- Precision: money/exact math → integers (cents) or `BigInt`; never binary floats. `0.1 + 0.2 → 0.30000000000000004`.

## BigInt

`10n`, `BigInt(10)`, `BigInt("0x…")`. Integer ops only (`/` truncates). No mixing with Number in arithmetic. Not JSON-serializable (throws). Use for 64-bit IDs, exact large integers.

## Keyed collections

- **`Map`** — any-typed keys, insertion-ordered, `size`, `get/set/has/delete/clear`, iterable of `[k,v]`. Keys compared by SameValueZero (`NaN` works as a key). Prefer over objects for dynamic/non-string keys and frequent add/remove.
- **`Set`** — unique values (SameValueZero). ES2025 set-algebra: `union, intersection, difference, symmetricDifference, isSubsetOf, isSupersetOf, isDisjointFrom` (take any set-like with `size`/`has`/`keys`).
- **`WeakMap`/`WeakSet`** — object-only (or registered-symbol) keys, held weakly (GC-able), not iterable, no `size`. For metadata/caches keyed on objects without leaking.
- **`WeakRef`/`FinalizationRegistry`** (ES2021) — advanced GC hooks; avoid unless necessary (non-deterministic).

## JSON

- `JSON.stringify(value[, replacer[, space]])` — omits `undefined`/functions/symbols in objects (→ nothing) and in arrays (→ `null`); serializes `Date` via `toJSON` → ISO string; **throws** on `BigInt` and circular refs. `replacer` = fn or key allow-list; `space` pretty-prints. Well-formed output for lone surrogates since ES2024.
- `JSON.parse(text[, reviver])` — reviver transforms each pair; ES2024 gives the reviver a `context.source` for exact round-tripping. No comments, no trailing commas, keys must be double-quoted.

## RegExp

Flags: `g` global, `i` ignore-case, `m` multiline, `s` dotAll (ES2018), `u` unicode, `y` sticky, `d` indices (ES2022), `v` unicodeSets (ES2024). Features: named groups `(?<n>…)`/`\k<n>` (ES2018), lookbehind (ES2018), `\p{…}` property escapes. Methods: `test`, `exec` (stateful with `g`/`y` via `lastIndex`), and string-side `match`/`matchAll`/`replace`/`replaceAll`/`split`. **ES2025:** `RegExp.escape(s)`, duplicate named groups across alternatives, inline modifiers `(?ims:…)`/`(?-ims:…)`.

## Symbol

`Symbol([desc])`, `Symbol.for`/`keyFor` (global registry). Well-known symbols hook into language behavior — see [language-core.md](language-core.md).

## Reflect & Proxy

- **`Reflect`** — functional twins of object internal methods: `Reflect.get/set/has/deleteProperty/ownKeys/getPrototypeOf/defineProperty/apply/construct`. Return booleans instead of throwing; ideal inside proxy traps and for `apply`/`construct` forwarding.
- **`Proxy(target, handler)`** — intercept operations via traps (`get, set, has, deleteProperty, ownKeys, getOwnPropertyDescriptor, defineProperty, apply, construct, getPrototypeOf, …`). Uses: validation, reactive state, virtualization, negative-index arrays. Invariants are enforced (can't report a non-existent non-configurable property). Not fully transparent — identity, `WeakMap` keying, and some perf differ.

## Typed arrays & ArrayBuffer

- `ArrayBuffer` — raw bytes; resizable + `transfer()` (ES2024). `SharedArrayBuffer` for cross-thread (with `Atomics`).
- Views: `Int8/Uint8/Uint8Clamped/Int16/Uint16/Int32/Uint32/Float32/Float64/BigInt64/BigUint64Array`, plus **`Float16Array`** (ES2025).
- `DataView` for mixed-type/endian access: `getInt32(offset, littleEndian)`, …, **`getFloat16`/`setFloat16`** (ES2025).
- `Uint8Array` base64/hex conversion helpers are Stage-4/emerging (not ES2025).

## Internationalization — `Intl`

Separate spec (ECMA-402) but standard in engines: `Intl.NumberFormat`, `DateTimeFormat`, `Collator` (locale-aware sort), `RelativeTimeFormat`, `PluralRules`, `ListFormat`, `Segmenter` (grapheme/word/sentence segmentation), `DurationFormat`. Prefer these over manual formatting for locale correctness.

## Dates — `Date` (and `Temporal`)

`Date` is legacy and error-prone: months are **0-indexed**, mutable, parsing is inconsistent (only ISO `YYYY-MM-DDTHH:mm:ss.sssZ` is reliable), and it conflates instants with wall-clock time. Use `Date.now()` (ms epoch), `toISOString()`. For anything nontrivial, prefer **`Temporal`** (Stage-3/emerging, not ES2025 — check availability) or a vetted library. See [syntax-and-features.md](syntax-and-features.md) emerging section.

## Errors

`Error` and subclasses `TypeError, RangeError, ReferenceError, SyntaxError, EvalError, URIError`, plus `AggregateError` (ES2021, from `Promise.any`). `new Error(msg, { cause })` (ES2022) chains an underlying error. Throw `Error` instances (not strings) so stack traces exist. Custom errors: `class AppError extends Error { name = "AppError"; }`.
