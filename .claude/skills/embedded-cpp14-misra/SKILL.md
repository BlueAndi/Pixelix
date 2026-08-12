---
name: embedded-cpp14-misra
description: Use when writing, reviewing, or refactoring C/C++ firmware code in this repository (src/, lib/, test/) — creating or editing .h/.hpp/.cpp files, applying MISRA-oriented and defensive rules, Yoda conditions, single-exit/pathfinder control flow, mandatory Doxygen headers, and clang-format compliance. Covers the house C++14 style; not for Python build scripts, web assets, or docs.
---

# Embedded C++14 (MISRA-Oriented) House Style

## Overview

Pixelix firmware is C++14 on ESP32 with hard constraints: limited RAM, no exceptions in practice, long uptimes, and dual-core FreeRTOS tasks. The house style optimizes for **deterministic, reviewable code** over brevity — explicit state, bounded loops, single exit, and a documented public API.

Two style layers, both mandatory:

1. **Formatting** — mechanical, owned by `clang-format` (v18.1.3, [.clang-format](../../../.clang-format)). Never hand-tune against it; run it.
2. **Structure & safety** — the rules below. `clang-format` cannot check these; you must.

CI enforces layer 1 on `src/` and `lib/` only:
```bash
find src lib -iname '*.h' -o -iname '*.hpp' -o -iname '*.cpp' | xargs clang-format -i --dry-run -Werror
```

## The Non-Negotiables

Five rules distinguish this codebase from generic modern C++. Violating any of them will fail review.

### 1. Yoda conditions

Constant or literal on the **left** of every equality/boolean comparison. Prevents `=`-for-`==` typos.

```cpp
if (true == isReady)        /* not: if (isReady) */
if (false == m_isRunning)   /* not: if (!m_isRunning) */
if (nullptr == ptr)         /* not: if (!ptr) */
if (0U == count)            /* not: if (count == 0) */
```

Relational comparisons (`<`, `>=`) keep natural order — Yoda applies to equality and explicit bool tests.

### 2. Single point of exit

One `return` per function, at the end. Use a result variable, not early returns.

```cpp
bool Foo::init()
{
    bool isSuccessful = false;

    if (nullptr == m_buffer)
    {
        LOG_ERROR("No buffer.");
    }
    else if (true == m_isInitialized)
    {
        LOG_WARNING("Already initialized.");
    }
    else
    {
        m_isInitialized = true;
        isSuccessful    = true;
    }

    return isSuccessful;
}
```

An empty guard branch with an explanatory comment is idiomatic and preferred over inverting the whole body:

```cpp
if (false == m_isInitialized)
{
    /* Guard path: do not process before initialization. */
}
else
{
    ++m_counter;
}
```

### 3. Pathfinder rule

One clear execution path per function. Guard-check invalid state first, keep nesting shallow (aim ≤ 3 levels), and split dense boolean chains into named helpers or named intermediate `bool` variables. If a reader cannot trace every path in one pass, decompose it.

### 4. Doxygen is mandatory

Every file gets the MIT block plus `@file`, `@brief`, `@author`. Headers add `@addtogroup <GROUP>` / `@{` … `@}` around the content. Existing groups: `PLUGIN`, `GFX`, `UTILITIES`, `SETTINGS`, `SENSORS`, `HTTP_SERVICE`, `HAL`, `TOPIC_HANDLER_SERVICE`, `RTC`, `TEST`. Every public method needs a doc block with `@param[in]`/`@param[out]`/`@return` and any range, unit, ownership, or blocking behavior. Every member gets a trailing `/**< ... */`.

### 5. Section banners in fixed order

Files are divided by 80-column banner comments. Order differs by file type — see the templates. Keep empty sections in place rather than deleting them.

| `.h` | `.hpp` | `.cpp` |
|------|--------|--------|
| Includes | Compile Switches | Includes |
| Compiler Switches | Includes | Compiler Switches |
| Macros | Macros | Macros |
| Types and Classes | Types and Classes | Types and classes |
| Variables | Functions | Prototypes |
| Functions | | Local Variables |
| | | Public / Protected / Private Methods |
| | | External / Local Functions |

## Safety Rules (MISRA-oriented)

- Fixed-width types from `<stdint.h>` (`uint32_t`, `int16_t`); unsigned literals get a `U` suffix.
- No implicit narrowing or signed/unsigned mixing — cast explicitly and say why if non-obvious.
- No dynamic allocation unless justified in a comment; prefer fixed-size buffers and static/member storage. Heap fragmentation is a real failure mode on long-running devices.
- Explicitly initialize every member in the constructor initializer list, in declaration order.
- `const`-correct: `const` parameters, `const` methods, `const`/`constexpr` over macros.
- No side effects inside conditions (no assignment, no `++`, no state-mutating calls in an `if`).
- Bound every loop; bounds-check every index and every buffer write.
- Validate all external input and every pointer parameter before use.
- On failure, leave the object in a valid, consistent state and report status — never half-initialized.
- Guard shared mutable state with `Mutex` / `MutexGuard` wherever a second task can reach it.

Full detail and rationale → [references/coding-rules.md](references/coding-rules.md)

## Naming

| Kind | Convention | Example |
|------|-----------|---------|
| Class / struct | PascalCase | `AudioDrv` |
| Method / function | camelCase | `getSampleRate()` |
| Member variable | `m_` + camelCase | `m_isInitialized` |
| Local variable | camelCase | `isSuccessful` |
| Constant / enum value | UPPER_SNAKE_CASE | `MAX_SLOTS` |
| Macro | UPPER_SNAKE_CASE | `LOG_INFO` |
| Include guard | `<FILE_NAME>_H` / `_HPP` | `AUDIO_DRV_H` |
| Plugin class | must end in `Plugin` | `DateTimePlugin` |

Boolean names read as predicates: `isRunning`, `hasData`, `isSuccessful`.

## Workflow

**Creating a file**

1. Pick the type: `.h` for module and regular class headers, `.hpp` **only** for header-only / template code, `.cpp` for implementation.
2. Copy the matching template and replace the placeholders — `ClassName`, `CLASS_NAME`, `GROUP`, `@brief`, `@author`:
   - [assets/template.h](assets/template.h)
   - [assets/template.hpp](assets/template.hpp)
   - [assets/template.cpp](assets/template.cpp)
3. Place it beside its peers (`lib/<Module>/src/`, `src/<Subsystem>/`), and mirror the neighbors' patterns for anything the template doesn't cover.
4. Write guard checks and error paths before the happy path.
5. Document as you write, not after.

**Editing an existing file**

Match the surrounding file over this document where they disagree — local consistency wins. Keep the diff tight; do not reformat or restyle untouched code.

**Before declaring done**

```bash
clang-format --dry-run -Werror <file>            # must be silent
platformio run --environment esp32doit-devkit-v1-LED-32x8
platformio check --environment <env> --fail-on-defect=medium
platformio test --environment test               # if test/ is touched
```

Then walk the checklist:

- [ ] Yoda conditions on all equality/bool tests
- [ ] Exactly one `return` per function
- [ ] Guard paths first; nesting shallow
- [ ] Fixed-width types; no implicit conversions; `U` suffixes
- [ ] All members initialized in declaration order
- [ ] No unjustified dynamic allocation
- [ ] Shared state mutex-protected
- [ ] Doxygen on file, class, every public method, every member
- [ ] Section banners present and correctly ordered
- [ ] `clang-format` clean

## Common Mistakes

| Mistake | House style |
|---------|-------------|
| `if (!ptr)` | `if (nullptr == ptr)` |
| Early `return false;` in a guard | Set result variable, fall through to the single `return` |
| `int` / `unsigned` for sizes and counts | `uint32_t`, `size_t`, `uint8_t` — fixed width |
| `new` / `std::vector` growth in a hot or long-lived path | Fixed-size buffer, or a justifying comment |
| Member assigned in the constructor body | Constructor initializer list, declaration order |
| Public method with no doc block | `@brief`, `@param`, `@return` — always |
| `#define MAX_LEN 10` | `static const uint32_t MAX_LEN = 10U;` |
| Deleting an empty section banner | Keep it — the layout is the convention |
| Reformatting a whole file while fixing three lines | Touch only what the change needs |
