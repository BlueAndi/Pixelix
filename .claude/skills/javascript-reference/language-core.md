# Language Core: Types, Coercion, Equality, Scope, `this`, Prototypes

Source: ECMA-262 [§6 Data Types](https://tc39.es/ecma262/2025/#sec-ecmascript-data-types-and-values), [§7 Abstract Operations](https://tc39.es/ecma262/2025/#sec-abstract-operations).

## Types & values

8 language types: **7 primitives** (Undefined, Null, Boolean, String, Symbol, Number, BigInt) + **Object**.

- **Number** — IEEE-754 binary64 (double). Only ~15–17 significant decimal digits; integers exact only up to `Number.MAX_SAFE_INTEGER` = 2⁵³−1. Special values: `NaN` (unequal to everything, incl. itself), `Infinity`/`-Infinity`, and `-0` (distinct from `+0`, but `-0 === +0`). Bitwise ops coerce to 32-bit ints.
- **BigInt** — arbitrary-precision integers, literal suffix `n` (`9007199254740993n`). Cannot mix with Number in arithmetic (`1n + 1` throws `TypeError`); compare across types with `==`/`<` works (`1n == 1` is `true`), `===` does not (`1n === 1` is `false`).
- **String** — immutable, indexed by UTF-16 **code units**. `"😀".length === 2` (surrogate pair). Iterate code *points* with `for...of` or spread; `[..."😀"].length === 1`.
- **Symbol** — unique. `Symbol("x") !== Symbol("x")`. `Symbol.for(k)` uses a global registry (shared). Well-known symbols customize behavior: `Symbol.iterator`, `Symbol.asyncIterator`, `Symbol.hasInstance` (`instanceof`), `Symbol.toPrimitive` (coercion), `Symbol.toStringTag` (`Object.prototype.toString`), `Symbol.hasInstance`, `Symbol.match/replace/search/split`, `Symbol.species`, `Symbol.isConcatSpreadable`, `Symbol.unscopables`.
- **Object** — a collection of properties. Properties are **data** (`[[Value]]`, `[[Writable]]`, `[[Enumerable]]`, `[[Configurable]]`) or **accessor** (`[[Get]]`, `[[Set]]`, `[[Enumerable]]`, `[[Configurable]]`). Inspect with `Object.getOwnPropertyDescriptor`. Literal/assignment properties default all boolean attributes to `true`. `Object.defineProperty` defaults omitted attributes to `false` **when creating a new property**; when redefining an existing property it leaves unspecified attributes **unchanged**.

Property **keys** are strings or symbols only. `obj[1]` coerces the key to `"1"`. `Map` allows any value as a key.

## Coercion (type conversion)

Coercion runs through abstract operations. The three that matter:

**ToPrimitive(input, hint)** — object → primitive. Calls `input[Symbol.toPrimitive](hint)` if present; else tries `valueOf`/`toString` in an order set by `hint` (`"number"`: valueOf first; `"string"`: toString first; default ≈ number, except `Date` defaults to string).

**ToNumber:**
| From | Result |
|------|--------|
| `undefined` | `NaN` |
| `null` | `+0` |
| `true`/`false` | `1`/`0` |
| `""` / whitespace | `0` |
| `"42"`, `" 42 "`, `"0x1F"`, `"1e3"` | parsed number |
| non-numeric string | `NaN` |
| BigInt/Symbol | throws `TypeError` |
| object | `ToNumber(ToPrimitive(x,"number"))` |

**ToString:** `null`→`"null"`, `undefined`→`"undefined"`, `-0`→`"0"`, arrays→join with `,` (`[1,[2,3]] → "1,2,3"`), plain object→`"[object Object]"`, Symbol→throws.

**`+` operator:** if either operand becomes a string after ToPrimitive, it *concatenates*; otherwise numeric add. `1 + "2" === "12"`, `1 + 2 + "3" === "33"`, `"1" + 2 + 3 === "123"`. All other arithmetic (`-`,`*`,`/`,`%`,`**`) coerces to Number (or stays BigInt).

**Notorious results (all correct):** `[] + [] === ""`, `[] + {} === "[object Object]"`, `+[] === 0`, `+{}` is `NaN`, `"" == 0` is `true`, `[] == ![]` is `true`, `null == undefined` is `true` but `null == 0` is `false`.

**Equality vs relational asymmetry:** `==` has a special case that makes `null`/`undefined` equal only to each other, but **relational** operators (`<`, `<=`, `>`, `>=`) don't — they coerce via `ToNumber`. So `null == 0` is `false` yet `null >= 0` is `true` (because `null` → `0`). Likewise `undefined >= 0` is `false` (`undefined` → `NaN`). Range checks against possibly-null values are a common trap.

## Equality algorithms

- **`===` (Strict Equality):** different types → `false`. Same type → same value, except `NaN !== NaN` and `+0 === -0`. Objects: same reference.
- **`==` (Loose Equality):** if same type → strict. Else: `null == undefined` (only these two). Number↔String coerces string ToNumber. Boolean coerced ToNumber first. Object↔primitive coerces object ToPrimitive. BigInt↔Number/String compares mathematical value. **Never** produces true across `null`/`undefined` and anything else.
- **`Object.is`:** SameValue — like `===` but `Object.is(NaN,NaN)===true` and `Object.is(-0,+0)===false`.
- **SameValueZero** (used by `Array.includes`, `Map`/`Set` keys): like `Object.is` but `-0` and `+0` are equal; `NaN` equals `NaN`. This is why `[NaN].includes(NaN)` is `true` but `[NaN].indexOf(NaN)` (uses `===`) is `-1`.

Rule of thumb: **always `===`**; use `== null` only as the deliberate "null or undefined" idiom.

## Truthiness

Exactly 8 falsy values: `false`, `0`, `-0`, `0n`, `""`, `null`, `undefined`, `NaN`. Everything else is truthy — including `"0"`, `"false"`, `" "`, `[]`, `{}`, and all functions. `ToBoolean` never throws and never calls user code.

## Scope, hoisting, TDZ

- **Lexical (static) scope:** inner functions see variables where they were *defined*, not called.
- **`var`** is function-scoped and hoisted, initialized to `undefined` at function entry (so reads before the declaration yield `undefined`, not an error).
- **`let`/`const`/`class`** are block-scoped and hoisted but **uninitialized** until their declaration executes — the **Temporal Dead Zone**. Reading in the TDZ throws `ReferenceError`.
- **Function declarations** are fully hoisted (callable before their line). **Function expressions**/arrow assigned to `let`/`const`/`var` are not (the binding hoists per its keyword's rules).
- Per-iteration bindings: a `let`/`const` in a `for` head gets a **fresh binding each iteration**, which is why closures in loops capture distinct values; `var` shares one binding.

```js
for (let i = 0; i < 3; i++) setTimeout(() => console.log(i)); // 0 1 2
for (var i = 0; i < 3; i++) setTimeout(() => console.log(i)); // 3 3 3
```

## Closures

A closure is a function plus the lexical environment it captures. Captured **bindings** are live references, not snapshots — mutating a captured `let` after closure creation is visible to the closure. Common uses: data privacy, partial application, memoization, stable per-iteration values.

## `this` binding

`this` is determined by **how a function is called**, resolved at call time (except arrows). Precedence:

1. **`new fn()`** → `this` is the freshly created object — this even **overrides** a bound `this` (`new (fn.bind(x))()` ignores `x`).
2. **`fn.bind(x)`** permanently fixes `this` to `x`; a later `.call`/`.apply` on the bound function **cannot** change it (bind wins). **`fn.call(x)` / `fn.apply(x)`** set `this` for that single call.
3. **`obj.fn()`** → `this` is `obj` (the receiver, lost if you extract `const f = obj.fn`).
4. **Plain `fn()`** → `this` is `undefined` in strict mode / modules / class bodies. In sloppy mode it's the global object, and a primitive `this` passed via `call`/`apply` is **boxed** into a wrapper object (`null`/`undefined` become the global object).
5. **Arrow functions** have **no own `this`** — they capture `this` lexically from the enclosing scope; `call`/`bind` cannot change it, and they **cannot be `new`ed** (`TypeError`). Same lexical capture for `arguments`, `super`, `new.target`.

Practical: use arrow functions or class fields (`handler = () => {…}`) for callbacks that need the instance; don't use arrows for object methods that rely on the receiver, or for prototype methods/constructors.

## Prototypes & inheritance

Every object has an internal `[[Prototype]]` (accessed via `Object.getPrototypeOf` / the legacy `__proto__`). Property lookup walks the prototype chain until found or `null`. A plain assignment `obj.x = v` **usually** creates/overwrites an **own writable data** property (shadowing) without touching the prototype — **but the `[[Set]]` algorithm consults the chain first**:
- a **setter** for `x` anywhere up-chain runs instead (no own property is created);
- a **getter-only accessor** or a **non-writable data property** up-chain makes the assignment **fail** — `TypeError` in strict mode, silently ignored in sloppy mode — with no shadowing.

To force an own property in those cases, use `Object.defineProperty`.

- `Object.create(proto)` makes an object with a given prototype; `Object.create(null)` makes a prototype-less "dict".
- Constructor functions: `new F()` creates an object whose `[[Prototype]]` is `F.prototype`, runs `F` with `this` = that object, returns it (unless `F` returns an object).
- `instanceof` walks the chain checking against `Constructor.prototype` (customizable via `Symbol.hasInstance`).
- `obj.hasOwnProperty(k)` / `Object.hasOwn(obj, k)` (prefer the latter) test own vs inherited. `in` checks the whole chain.

### `class` is prototype sugar (with real differences)

```js
class Animal {
  static kingdom = "Animalia";      // static field
  #energy = 100;                    // private field (truly private, not #-string)
  constructor(name) { this.name = name; }
  speak() { return `${this.name} noises`; }   // on Animal.prototype
  get tired() { return this.#energy < 20; }   // accessor
  static make(n) { return new this(n); }
}
class Dog extends Animal {
  speak() { return `${super.speak()} (woof)`; } // super → prototype chain
}
```

Class specifics vs functions: class bodies are always **strict mode**; class declarations are **hoisted but sit in the TDZ** (the binding exists from the top of the block but is uninitialized — referencing it before the declaration throws `ReferenceError`, unlike function declarations which are callable early); methods are non-enumerable; the constructor cannot be called without `new`; `#private` members are enforced by the engine (accessing a missing one throws); `extends` sets up both instance and static prototype chains; subclass `constructor` must call `super()` before using `this`.

## Strict mode

`"use strict"` (implicit in modules and class bodies): assignments to undeclared vars throw; `this` is `undefined` in plain calls; duplicate params and `with` are errors; writes to non-writable/getter-only properties throw; `eval` gets its own scope. Prefer modules — strict everywhere, top-level `this` is `undefined`, and top-level `let`/`const` don't become global properties.
