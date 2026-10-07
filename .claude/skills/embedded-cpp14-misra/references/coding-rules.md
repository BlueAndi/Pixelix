# Embedded C++14 Coding Rules (MISRA-Oriented)

Detail and rationale behind the summary in [SKILL.md](../SKILL.md). Read this when a rule's *why* matters — a borderline case, a justified deviation, or a review disagreement.

## 1. Language and Safety Baseline

- **C++14 only.** No C++17/20 constructs (`std::optional`, `if constexpr`, structured bindings, `std::string_view`). The toolchain may accept some; the codebase does not use them.
- **Fixed-width integers** from `<stdint.h>`: `uint8_t`, `int16_t`, `uint32_t`. Use `size_t` for sizes and indices. Plain `int`/`unsigned` only where an external API dictates it.
- **Unsigned literals carry `U`** — `0U`, `1000U`, `MAX_COUNT = 32U`. This makes signed/unsigned comparison warnings meaningful instead of noise.
- **No implicit narrowing.** Cast explicitly (`static_cast`), and comment the cast when the truncation is intentional.
- **No exceptions, no RTTI** in firmware paths. Report failures via return status.
- **Avoid UB-prone constructs:** unsequenced modification, strict-aliasing violations through raw casts, signed overflow, reading uninitialized memory, unchecked `printf`-family format arguments.

## 2. Defensive Programming

Assume every input is wrong until proven otherwise.

- **Validate pointer parameters** before dereference, even from internal callers — internal callers change.
- **Validate ranges and enum values** received from REST, MQTT, settings storage, sensors, or the filesystem. External data is untrusted by definition.
- **Fail safely:** return a status, log at the appropriate level, and leave the object in a valid state. A failed `init()` must leave the object exactly as unusable as it was before — never half-initialized.
- **Explicit lifecycle flags** (`m_isInitialized`, `m_isRunning`) rather than inferring state from a pointer being non-null.
- **Bound every loop.** `while (true == condition)` around hardware polling needs a timeout or an iteration cap; a stuck peripheral must not wedge a task.
- **Bounds-check every index and buffer write.** Use `sizeof(arr) / sizeof(arr[0])` or a named constant, never a duplicated literal.
- **Check the return value** of anything that can fail — filesystem, network, JSON parsing, mutex acquisition with timeout.

## 3. Yoda Conditions

Constant/literal on the left in equality and boolean tests:

```cpp
if (true == isReady)
if (false == m_isRunning)
if (0U == count)
if (nullptr == ptr)
if (SOME_ENUM_VALUE == state)
```

**Rationale:** `if (isReady = true)` compiles silently; `if (true = isReady)` does not.

**Scope:** equality (`==`, `!=`) and explicit boolean tests. Relational operators (`<`, `<=`, `>`, `>=`) keep natural reading order — `if (index < MAX_COUNT)`, not the reverse.

Never rely on implicit truthiness. Compare explicitly against `true`, `false`, `nullptr`, or `0U`.

## 4. Single Point of Exit

One `return` per function, as the last statement.

**Rationale:** predictable cleanup, one place to inspect the result, no path that skips a mutex release or a state update.

**Pattern:** declare a result variable initialized to the failure value, narrow it through the branch chain, return once.

```cpp
bool Parser::parse(const char* data, size_t length)
{
    bool isSuccessful = false;

    if (nullptr == data)
    {
        LOG_ERROR("No data.");
    }
    else if (0U == length)
    {
        LOG_ERROR("Empty payload.");
    }
    else if (MAX_LENGTH < length)
    {
        LOG_ERROR("Payload too long: %u", length);
    }
    else
    {
        isSuccessful = parseInternal(data, length);
    }

    return isSuccessful;
}
```

Empty guard branches with an explanatory comment are idiomatic:

```cpp
if (false == m_isInitialized)
{
    /* Guard path: nothing to process. */
}
else
{
    /* ... */
}
```

`void` functions follow the same shape — one implicit exit at the end, no bare `return;` in the middle.

## 5. Pathfinder Rule

One understandable execution path per function.

- **Guard first.** Invalid state and invalid arguments are handled at the top; the remaining body assumes valid state.
- **Shallow nesting.** Three levels is the working limit. Deeper means the function is doing more than one job.
- **Name your conditions.** Replace `if ((a > b) && (false == c) && (nullptr != d))` with a named `bool` whose name states the intent.
- **Decompose.** Complex logic becomes private helper methods with descriptive names. A helper that is used once and named well is still worth extracting.
- **Parenthesize** sub-expressions in compound conditions rather than relying on precedence.
- **No side effects in conditions.** No assignment, no `++`, no state-mutating call inside an `if`. Compute first, then test.

## 6. MISRA-Oriented Practices

- **Dynamic allocation:** avoid it. Where unavoidable (ArduinoJson documents, framework-imposed APIs), keep the allocation short-lived or one-shot at init, and add a comment justifying it. Long-running devices die of fragmentation, not of leaks.
- **`const` correctness:** `const` on parameters that are not modified, on methods that do not mutate, on data that never changes. Prefer `static const` / `constexpr` typed constants over `#define`.
- **Macros:** minimize. Use typed constants, `enum class`, and inline functions instead. Where a macro is required (logging, compile switches), wrap the body in parentheses and guard multi-statement macros with `do { ... } while (0)`.
- **Explicit member initialization:** every member in the constructor initializer list, in declaration order (mismatched order is a compiler warning and a real bug source).
- **Narrow scope:** declare variables at first use, inside the tightest scope that works.
- **Enums:** prefer `enum class` for new type-safe enumerations; match the surrounding file when extending an existing plain `enum`.
- **`switch`:** always a `default` case; every case terminated by `break` or an explicit `/* fallthrough */` comment.

## 7. Concurrency

Display update, audio processing, and network run as separate FreeRTOS tasks across two cores.

- Any state reachable from more than one task is protected by `Mutex` / `MutexGuard`.
- Prefer `MutexGuard` (RAII) over manual lock/unlock — it survives every exit path.
- Keep critical sections short; never block on I/O, logging, or a network call while holding a mutex.
- Watch for priority inversion and for a lock taken from an ISR context — ISR-safe primitives are different.
- `volatile` is not a synchronization mechanism. Use it for memory-mapped hardware registers only.

## 8. Documentation (Mandatory)

- **File header:** MIT license block, then the `DESCRIPTION` banner with `@file`, `@brief`, `@author`. Headers add `@addtogroup <GROUP>` and `@{`, closed by `/** @} */` at end of file.
- **Doxygen blocks** on classes, interfaces, all public methods, and non-trivial private methods.
- **Every parameter** documented with direction: `@param[in]`, `@param[out]`, `@param[in,out]`.
- **`@return`** on every non-`void` function, stating what each outcome means — not just the type.
- **Document constraints:** valid ranges, physical units, ownership/lifetime of pointers, blocking vs non-blocking, task/ISR context, and side effects.
- **Members** get a trailing `/**< Description. */`.
- Comments explain *why*. The code already says *what*.

## 9. Formatting and Layout

- **`clang-format` (v18.1.3) is the authority.** Config: [.clang-format](../../../../.clang-format). Never hand-format against it; run it and take the output.
- Notable settings, so you know what to expect: 4 spaces, no tabs; no column limit; `SortIncludes: Never` (include order is manual and meaningful); pointer/reference bound left (`char* p`, `const T& v`); braces on their own line (Allman) after class, function, and control statements; space after `if`/`while`/`for`/`switch` but not after a function name; consecutive declarations and assignments aligned; constructor initializers one per line after the colon.
- **Section banners** in the fixed per-file-type order (see [SKILL.md](../SKILL.md)). Keep empty sections rather than removing them.
- **Include order** is manual: the module's own header first in a `.cpp`, then system/framework headers, then project headers.
- **Do not reformat untouched code.** A three-line fix produces a three-line diff.
