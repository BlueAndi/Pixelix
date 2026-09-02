# Embedded C/C++14 Coding Rules (MISRA-oriented)

The complete ruleset for firmware, service, plugin, HAL, and graphics code in this
repository. When a rule and an existing neighbouring file disagree, match the file and
flag the discrepancy.

## 1. Language and Safety Baseline

- Use **C++14 only**. No later-standard features.
- Prefer **fixed-width integer types** from `<stdint.h>` (`uint8_t`, `int16_t`,
  `uint32_t`, `size_t`) over `int`/`unsigned`/`long`.
- Suffix unsigned literals: `0U`, `1UL`. Avoid magic numbers — name them with typed
  `const`/`enum`.
- Avoid undefined-behavior-prone constructs and **implicit narrowing conversions**. Make
  every conversion explicit.
- Keep interfaces explicit and predictable; no hidden global state.

## 2. Defensive Programming

- **Validate every input argument, pointer, and external data item** before use — at the
  top of the function.
- **Fail safely:** return an error/status value and leave the object in a valid state.
  Never leave a half-updated object after a failure.
- Use explicit **state flags** for lifecycle (`m_isInitialized`, `m_isRunning`, ...) and
  guard against operating before init or after teardown.
- Protect shared mutable state with `Mutex` / `MutexGuard` wherever concurrency exists
  (display, audio, and WiFi run as separate FreeRTOS tasks).
- **Bounds-check every loop and array index.** Keep loops bounded (no unbounded `while`).
- Log meaningful failures via `LOG_WARNING` / `LOG_ERROR` (from `Logging.h`); log
  significant lifecycle events via `LOG_INFO`.

## 3. Yoda Conditions

Place the constant / literal on the **left** side of an equality comparison, so an
accidental assignment (`=` instead of `==`) becomes a compile error.

```cpp
if (true == isReady)   { ... }
if (false == isBusy)   { ... }
if (0U == count)       { ... }
if (nullptr == pData)  { ... }
```

This applies to equality/inequality checks. For plain boolean flags used as a condition
(not compared), follow the surrounding file's style; equality comparisons are always Yoda.

## 4. Pathfinder Rule (single exit, clear flow)

- Keep **one understandable execution path** per function.
- Use a **single result variable** initialized to the failure/default value, and **one
  `return` statement** at the end.
- Apply **guard checks early** for invalid states using empty guarded branches with an
  explanatory comment; put the real work in the final `else`.
- Keep nesting **shallow**; replace dense boolean chains with named helper functions.
- Split complex logic into small, named private helpers.

```cpp
bool Service::start()
{
    bool isSuccessful = false;

    if (true == m_isRunning)
    {
        /* Guard: already running. */
    }
    else if (false == m_isInitialized)
    {
        LOG_ERROR("Start before init.");
    }
    else
    {
        m_isRunning = true;
        isSuccessful = true;
    }

    return isSuccessful;
}
```

## 5. MISRA-oriented Practices

- **Avoid dynamic allocation** unless strictly required and justified with a comment;
  prefer fixed buffers / static storage on the heap-constrained ESP32.
- Prefer **`const` correctness** and immutable data wherever possible.
- **No hidden side effects** inside conditions or expressions.
- Keep **macros minimal** — prefer typed constants and `enum`s over `#define`.
- **Explicitly initialize all members** in the constructor initializer list, in
  declaration order.
- Prefer scoped enums / typed constants; avoid implicit enum-to-int reliance.
- Keep scopes narrow; declare variables close to first use with explicit initialization.
- Mark overrides with `override` / `final`; mark single-argument constructors `explicit`
  where implicit conversion is undesirable.

## 6. Documentation (mandatory)

- Every file begins with the **MIT license block**, then a Doxygen `@file` block with
  `@file`, `@brief`, `@author`, and (for headers) `@addtogroup <GROUP>` + `@{ ... @}`.
- Doxygen blocks are required for:
  - classes / interfaces / structs
  - **all public methods**
  - non-trivial private methods
- Document `@param` (with direction `[in]` / `[out]` / `[inout]`), `@return`, and behavior
  constraints: ranges, units, ownership, side effects, threading assumptions.
- Data members are documented inline with `/**< ... */`.
- No undocumented public symbol may remain.

See [file-conventions.md](file-conventions.md) for the exact layout.

## 7. Formatting and Layout

- Formatting is **always** enforced by `clang-format` (v18.1.3, config in
  `.clang-format`). Check with `clang-format --dry-run --Werror <file>`.
- **Do not hand-format** against the project style — follow the formatter's output.
- Keep include groups and section banners aligned with the existing repository pattern
  (`SortIncludes: Never` — order is intentional).

See [clang-format.md](clang-format.md) for the rules that bite most often.

## 8. Completion Checklist

- [ ] File structure matches the repo pattern for `.h` / `.hpp` / `.cpp`.
- [ ] Fixed-width types; no implicit narrowing; no magic numbers.
- [ ] Defensive checks on every input/pointer; state valid on all failure paths.
- [ ] Yoda equality style applied.
- [ ] Pathfinder rule: single result variable, single `return`, shallow nesting.
- [ ] All members explicitly initialized in declaration order.
- [ ] Non-copyable idiom applied where copying is not intended.
- [ ] Doxygen complete for file, classes, and every public API.
- [ ] `clang-format --dry-run --Werror` passes.
- [ ] `platformio check` (static analysis) raises no new medium+ defects.
