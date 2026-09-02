---
name: embedded-cpp-reference
description: Use when writing, refactoring, or reviewing embedded C/C++14 code in this repository (.h, .hpp, .cpp) — applying MISRA-oriented safety rules, defensive programming, Yoda conditions, the single-exit "pathfinder" rule, fixed-width types, mandatory Doxygen, the repo's file/section-banner structure, and clang-format compliance. Covers firmware, services, plugins, HAL, and graphics code.
---

# Embedded C/C++14 Reference (MISRA-oriented, Pixelix style)

## Overview

This skill is the condensed-but-authoritative reference for writing C++14 the way this
(Pixelix / esp-rgb-led-matrix) firmware is written: **deterministic behavior first,
explicit safety, and a fixed file/documentation layout** that every `.h`, `.hpp`, and
`.cpp` in the tree follows.

**Core principle:** on an ESP32 there is no room for surprises. Prefer explicit checks,
bounded loops, fixed-width types, and one clear control path over clever code. When two
styles both compile, pick the one that a MISRA reviewer and `clang-format` both accept
without comment.

The source of truth is **existing repository files** and **`clang-format` output**
(config in [.clang-format](../../../.clang-format), v18.1.3, enforced in CI). When in
doubt, open a neighbouring file in the same `lib/*` module and match it.

## When to Use

- Creating or updating firmware / service / plugin / HAL / widget code.
- Adding a new `.h`, `.hpp`, or `.cpp` file — start from the [templates](#templates).
- Enforcing MISRA-oriented and defensive style during review or refactor.
- Applying Yoda conditions, the pathfinder (single-exit) rule, and fixed-width types.
- Writing complete Doxygen for a file, class, or API.
- Keeping code `clang-format`-clean and consistent with the section-banner layout.

**Not for:** the web dashboard (see `javascript-reference` / `html5-reference` /
`bootstrap-reference`), Python build scripts, or the generated files under
`src/Generated/` (never hand-edit those).

## The Mental Model (read this first)

Five rules decide almost every line:

1. **Fixed-width & explicit** — `uint32_t`/`int16_t`, not `int`/`unsigned`. No implicit
   narrowing, no hidden conversions, no magic numbers. `const`-correct everywhere.
2. **Defensive** — validate every pointer and external input at the top of the function;
   keep object state valid on every failure path; bounds-check every loop and index.
3. **Yoda comparisons** — constant/literal on the **left** of `==`:
   `if (nullptr == ptr)`, `if (true == isReady)`, `if (0U == count)`.
4. **Pathfinder / single exit** — one `bool`/status result variable, one `return` at the
   end. Guard invalid states early with empty guarded branches; keep nesting shallow.
5. **Documented & formatted** — MIT header + `@file`/`@brief`/`@author`, Doxygen on every
   public API, section banners in fixed order, and `clang-format` clean.

Full details → **[references/coding-rules.md](references/coding-rules.md)**.

## Quick Reference

**Yoda equality** — literal on the left so an accidental `=` is a compile error:

```cpp
if (false == m_isInitialized)   /* not: if (!m_isInitialized) for equality checks */
if (nullptr == pData)
if (0U == length)
```

**Single-exit pathfinder** — one result variable, guard-first, one `return`:

```cpp
bool Foo::doWork(const uint8_t* pData, size_t length)
{
    bool isSuccessful = false;

    if (nullptr == pData)
    {
        /* Guard: invalid buffer. */
    }
    else if (0U == length)
    {
        /* Guard: nothing to do. */
    }
    else
    {
        /* Single happy path here. */
        isSuccessful = true;
    }

    return isSuccessful;
}
```

**Naming & members** — members carry an `m_` prefix, are documented inline, and are
explicitly initialized in the constructor initializer list (in declaration order):

```cpp
private:

    bool        m_isInitialized; /**< Initialization flag. */
    uint32_t    m_counter;       /**< Internal counter.    */
```

**Non-copyable objects** — declare copy ctor / assignment `private` and leave them
undefined (C++14 style used across the repo), unless copying is genuinely intended:

```cpp
    /* An instance shall not be copied. */
    Foo(const Foo& other);
    Foo& operator=(const Foo& other);
```

**Types:** prefer `<stdint.h>` fixed-width types; suffix unsigned literals (`0U`, `1UL`);
avoid dynamic allocation unless justified with a comment; prefer typed `const`/`enum`
over `#define` macros.

## Templates

Copy the matching template and fill in the placeholders — they already carry the MIT
header, Doxygen metadata, `@addtogroup` block, include guard, and section banners in the
correct order.

- **[assets/template.h](assets/template.h)** — regular / module header and ordinary C++
  class headers. Use `.h` by default.
- **[assets/template.hpp](assets/template.hpp)** — header-only / template classes only.
- **[assets/template.cpp](assets/template.cpp)** — implementation files.

## Procedure

1. **Pick the file type** — `.h` for module/class headers, `.hpp` only for header-only
   templates, `.cpp` for implementation. Copy the matching template.
2. **Write the interface** — declare members and public API with full Doxygen; keep the
   section banners; mark non-copyable if applicable.
3. **Implement defensively** — guard-check pointers/inputs first, one result variable,
   single `return`, Yoda equality, bounded loops.
4. **Document while coding** — `@param` (with `[in]`/`[out]`), `@return`, and behavior
   notes (ranges, units, ownership, side effects) on every public method.
5. **Quality gate** — verify against
   [references/coding-rules.md](references/coding-rules.md), confirm the layout matches
   [references/file-conventions.md](references/file-conventions.md), and ensure the file
   is `clang-format` clean (`clang-format --dry-run --Werror <file>`).

## Reference Files

| File | Contents |
|------|----------|
| **[references/coding-rules.md](references/coding-rules.md)** | The full ruleset: language/safety baseline, defensive programming, Yoda conditions, pathfinder / single-exit rule, MISRA-oriented practices, mandatory documentation, and formatting policy. |
| **[references/file-conventions.md](references/file-conventions.md)** | File anatomy: MIT header, `@file` block, `@addtogroup`, include guards, the fixed section-banner order, include grouping, `m_` naming, member alignment, and non-copyable idiom. |
| **[references/clang-format.md](references/clang-format.md)** | The formatting rules that matter in practice, derived from `.clang-format`: brace-on-next-line (Allman), 4-space indent, no column limit, aligned consecutive declarations/assignments, `SortIncludes: Never`, pointer/reference left-alignment, one-arg-per-line packing. |

## Common Mistakes

| Mistake | Fix |
|---------|-----|
| `if (ptr != nullptr)` / `if (isReady)` for equality checks | Yoda form: `if (nullptr != ptr)`, `if (true == isReady)`. |
| Multiple `return` statements scattered through a function | One result variable, guard-first branches, a single `return` at the end (pathfinder rule). |
| `int` / `unsigned` / `long` in interfaces | Fixed-width `<stdint.h>` types: `uint32_t`, `int16_t`, `size_t`. |
| Forgetting to initialize a member | Every member initialized in the constructor initializer list, in declaration order. |
| Editing files in `src/Generated/` | Generated by `scripts/`; change the source or generator, then rebuild. |
| Hand-formatting to look "aligned" | Run `clang-format`; it aligns consecutive declarations/assignments for you. Don't fight it. |
| Adding a public method without Doxygen | Every public API needs `@param`/`@return`/behavior notes; CI and reviewers reject undocumented symbols. |
| `#define` for constants | Prefer typed `const` / `enum`; keep macros minimal. |
| Reordering includes to "tidy" them | `SortIncludes: Never` — order is intentional; keep include groups as-is. |
| Silent failure paths | Log via `LOG_WARNING`/`LOG_ERROR` and keep state valid; never leave a half-updated object. |
| Deep nesting / dense boolean chains | Extract named helper functions; keep one understandable path per function. |
| Dynamic allocation "because it's easy" | Avoid on the heap-constrained ESP32 unless justified with a comment; prefer fixed buffers / static storage. |

## Related Skills

- Build/upload/test commands and architecture → see [CLAUDE.md](../../../CLAUDE.md).
- Web UI counterparts: `javascript-reference`, `html5-reference`, `bootstrap-reference`.
- The Copilot-flavored source of this skill: [.github/skills/embedded-cpp14-misra](../../../.github/skills/embedded-cpp14-misra/SKILL.md).
