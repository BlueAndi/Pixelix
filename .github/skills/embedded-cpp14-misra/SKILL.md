---
name: embedded-cpp14-misra
description: 'Write and refactor embedded C/C++14 code with MISRA-oriented rules, defensive programming, Yoda conditions, pathfinder rule, mandatory Doxygen, and clang-format compliance. Use for .h, .hpp, .cpp files in firmware and services.'
argument-hint: 'Describe the module, file type (.h/.hpp/.cpp), and expected behavior'
---

# Embedded C/C++14 MISRA Workflow

## When to Use

Load this skill when asked to:
- Create or update embedded C/C++14 firmware code
- Add new `.h`, `.hpp`, or `.cpp` files in this repository style
- Enforce MISRA-oriented and defensive coding style
- Apply Yoda conditions and pathfinder rule
- Add complete Doxygen documentation
- Keep code clang-format clean

## Ground Rules

- Target language: C++14 (embedded context, deterministic behavior first)
- Style source of truth: existing repository files and clang-format output
- Safety first: prefer explicit checks, bounded loops, and clear state transitions
- Documentation is mandatory for public API and non-trivial internals

## Coding Policy

Use [coding-rules.md](./references/coding-rules.md) for details. Mandatory points:

- MISRA-oriented patterns:
  - Avoid implicit conversions and hidden side effects
  - Prefer fixed width integer types (`uint32_t`, `int16_t`, ...)
  - Avoid dynamic allocation unless strictly required and justified
  - Keep scopes narrow and initialization explicit
  - A function should have a single point of exit.
- Defensive programming:
  - Validate all external inputs and pointers
  - Handle failure paths explicitly
  - Keep service/object state consistent on errors
- Yoda style:
  - Place constant/literal on the left in comparisons
  - Example: `if (false == isRunning)`
- Pathfinder rule:
  - Keep one clear control path through each function
  - Use guard checks and explicit branches to avoid ambiguous flow
  - Avoid deeply nested branching where practical
- Formatting:
  - Always run `clang-format` (or ensure output is clang-format clean)
- Doxygen:
  - File header blocks + `@file`, `@brief`, `@author`
  - Group tags (`@addtogroup`, `@{`, `@}`) where applicable
  - `@param`, `@return`, and behavior notes for APIs

## Procedure

1. Determine file type and role
- Pick `.h` for C-style/module headers and regular C++ headers.
- Pick `.hpp` only for header-only and template-style C++ headers.
- Pick `.cpp` for implementation.

2. Start from repository template
- Use one of:
  - [Regular/module header template (.h)](./assets/template.h)
  - [Template/header-only header template (.hpp)](./assets/template.hpp)
  - [C++ source template](./assets/template.cpp)

3. Implement behavior with safety constraints
- Add explicit boundary/error checks first.
- Keep data flow readable and branch logic deterministic.
- Apply Yoda conditions in equality/boolean checks.

4. Document while coding
- Add Doxygen for every public API.
- Document assumptions, units, ranges, and side effects.

5. Final quality gate
- Check against [coding-rules.md](./references/coding-rules.md).
- Ensure file formatting is clang-format compliant.
- Ensure no undocumented public symbols remain.

## Completion Checklist

- [ ] File structure matches repo pattern for `.h/.hpp/.cpp`
- [ ] MISRA-oriented constraints respected
- [ ] Defensive checks implemented
- [ ] Yoda condition style applied where relevant
- [ ] Pathfinder rule respected (clear flow, bounded complexity)
- [ ] Doxygen complete for file and APIs
- [ ] clang-format compliance preserved

## References

- [Coding rules](./references/coding-rules.md)
- [Regular/module header template (.h)](./assets/template.h)
- [Template/header-only header template (.hpp)](./assets/template.hpp)
- [C++ source template](./assets/template.cpp)
