# Async & Iteration

Source: ECMA-262 [§27 Control Abstraction Objects](https://tc39.es/ecma262/2025/#sec-control-abstraction-objects) (iterators, generators, promises, async functions).

## Execution model: jobs, microtasks, and (host) tasks

JavaScript runs on a **single thread** with a **run-to-completion** guarantee: a synchronous block runs fully before any queued work.

- **ECMA-262 defines promise *Jobs*** (the reaction/resolve-thenable work items) and the `HostEnqueuePromiseJob` hook — but **deliberately delegates the queue, its ordering, and its draining to the host.** The term **"microtask" and the "drain all microtasks" rule are HTML/host semantics**, not ECMA-262. In practice: once the JS call stack empties, the host runs a *microtask checkpoint* that drains **all** pending microtasks (promise reactions, `queueMicrotask`, `await` continuations) before rendering or the next task.
- **Tasks / "macrotasks"** (`setTimeout`, I/O, events, `setInterval`) are scheduled entirely by the **host** (HTML/Node), *not* by ECMA-262. The initial script runs as a task; then one task runs, the microtask queue is fully drained, then the next task.

**Ordering rule:** sync code → microtasks (until empty) → one macrotask → microtasks → …

```js
console.log("A");
setTimeout(() => console.log("D"), 0);      // macrotask
Promise.resolve().then(() => console.log("C")); // microtask
console.log("B");
// A B C D  — microtask C runs before macrotask D
```

Starvation caveat: an unending stream of microtasks (or recursive `queueMicrotask`) can block tasks/rendering.

## Promises

A `Promise` is a value with state **pending → fulfilled(value) | rejected(reason)**; settling is one-way and permanent.

```js
const p = new Promise((resolve, reject) => { /* executor runs synchronously */ });
p.then(onFulfilled, onRejected)   // returns a NEW promise (chainable)
 .catch(onRejected)               // = .then(undefined, onRejected)
 .finally(onSettled);             // runs regardless; passes value/reason through
```

- `.then` **always** schedules its callback as a microtask, even on an already-settled promise.
- Returning a value from `.then` fulfills the next promise; **returning a promise adopts its state** (flattening); **throwing** rejects the next promise.
- Promises **auto-flatten**: you can't have a `Promise<Promise<T>>`. Adoption applies to any **thenable**, not just native promises.
- `.finally(fn)` normally passes the value/reason through — **but if `fn` throws or returns a rejected promise, that replaces the original outcome.** Keep finally callbacks side-effect-only.

**Static combinators:**
| Method | Resolves when | Rejects when |
|--------|---------------|--------------|
| `Promise.all(iter)` | all fulfill → array of values | any rejects (first reason) |
| `Promise.allSettled(iter)` | all settle → array of `{status, value or reason}` | never |
| `Promise.race(iter)` | first to **settle** (fulfill or reject) | first to settle rejects |
| `Promise.any(iter)` | first fulfillment | all reject → `AggregateError` |
| `Promise.resolve(v)` / `reject(r)` | — | — |

Empty-iterable edge cases: `all([])`/`allSettled([])` fulfill with `[]`; `race([])` stays **pending forever**; `any([])` rejects immediately with an empty `AggregateError`.
| `Promise.withResolvers()` (ES2024) | returns `{promise, resolve, reject}` | — |
| `Promise.try(fn)` (ES2025) | wraps sync-or-async `fn`; sync throw → rejection | — |

## `async` / `await`

- An `async` function **always returns a Promise**. `return x` fulfills with `x`; `throw` rejects; `return aPromise` adopts it.
- `await expr` pauses the function until `expr` settles, then resumes as a **microtask** with the fulfilled value (or throws the rejection reason at the `await` site).
- `await` a non-promise wraps it (`await 5` ≈ resolved 5, resumes next microtask).

```js
async function load(id) {
  try {
    const a = await fetchThing(id);       // sequential
    const [b, c] = await Promise.all([f(a), g(a)]); // concurrent — start both, then await
    return b + c;
  } catch (err) {                          // catches rejections from any awaited op
    throw new Error("load failed", { cause: err }); // ES2022 error chaining
  }
}
```

**Pitfalls:**
- `await` in a loop serializes; to parallelize, start promises first then `await Promise.all`.
- `arr.forEach(async …)` does **not** await — the promises are dropped. Use `for…of` with `await`, or `Promise.all(arr.map(async …))`.
- An unhandled rejection (no `.catch`/`try`) triggers a host `unhandledrejection` warning/crash. Always handle or propagate.
- Errors in a `new Promise` executor's async callbacks that aren't passed to `reject` are lost — reject explicitly.

## Iteration protocols

**Iterable** — has `[Symbol.iterator]()` returning an **Iterator**.
**Iterator** — has `next()` → `{ value, done }`; optionally `return()`/`throw()`.

Consumed by `for...of`, spread `[...x]`, destructuring, `Array.from`, `Map`/`Set` constructors, `yield*`, `Promise.all`, etc. Built-in iterables: Array, String (by code point), Map, Set, TypedArray, `arguments`, and generators.

```js
const range = {
  from: 1, to: 3,
  [Symbol.iterator]() {
    let n = this.from; const last = this.to;
    return { next: () => n <= last ? { value: n++, done: false } : { value: undefined, done: true } };
  }
};
[...range]; // [1,2,3]
```

## Generators

`function*` returns a generator (both iterable and iterator). `yield` suspends and returns a value; `next(v)` resumes, `v` becoming the result of the paused `yield`. `yield*` delegates to another iterable.

```js
function* ids() { let i = 0; while (true) yield i++; }      // infinite, lazy
function* flatten(a) { for (const x of a) Array.isArray(x) ? yield* flatten(x) : yield x; }
```
`gen.return(v)` finishes it; `gen.throw(e)` injects an error at the paused `yield` (lands in the generator's `try`). Great for lazy sequences and custom iteration.

## Async iteration

**Async iterable** — `[Symbol.asyncIterator]()` returning an iterator whose `next()` returns a **Promise** of `{value,done}`. Consumed by **`for await...of`** (ES2018). **Async generators** (`async function*`, may `await` and `yield`) are the easy way to produce them — ideal for streams / paginated APIs.

```js
async function* pages(url) {
  let next = url;
  while (next) {
    const res = await fetch(next);       // host API, illustrative
    const { items, nextUrl } = await res.json();
    yield* items;
    next = nextUrl;
  }
}
for await (const item of pages("/api")) process(item);
```

## ES2025 Iterator helpers

Lazy methods now live on `Iterator.prototype`, so any iterator can be composed without building intermediate arrays:

```js
const first3Evens = Iterator.from(numbersIterable)
  .filter(n => n % 2 === 0)
  .map(n => n * n)
  .take(3)
  .toArray();
```
Available: `map, filter, take, drop, flatMap, reduce, toArray, forEach, some, every, find`, plus `Iterator.from(x)`. They're lazy and short-circuit (`take`/`find`/`some`), so they work on infinite generators.
