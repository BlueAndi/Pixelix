# Embedded C/C++14 Coding Rules (MISRA-Oriented)

## 1. Language and Safety Baseline

- Use C++14 only.
- Prefer fixed width integer types from `<stdint.h>`.
- Avoid UB-prone constructs and implicit narrowing conversions.
- Keep interfaces explicit and predictable.

## 2. Defensive Programming

- Validate all input arguments and external data.
- Fail safely: return error/status and keep object state valid.
- Use explicit state flags for lifecycle (`m_isRunning`, initialized flags).
- Protect shared mutable state (`Mutex`, `MutexGuard`) where concurrency exists.
- Bounds-check all loops and array indexing.

## 3. Yoda Conditions

- Equality checks use constants/literals on left side.
- Examples:
  - `if (true == isReady)`
  - `if (0U == count)`
  - `if (nullptr == ptr)`

## 4. Pathfinder Rule

- Keep one understandable execution path per function.
- Apply guard checks early for invalid states.
- Keep nesting shallow; use explicit branches instead of dense boolean chains.
- Complex logic should be split into named helper functions.

## 5. MISRA-Oriented Practices

- Avoid dynamic allocation unless absolutely required and justified.
- Prefer `const` correctness and immutable data where possible.
- Avoid hidden side effects in conditions.
- Keep macros minimal; prefer typed constants and enums.
- Explicitly initialize all members.

## 6. Documentation (Mandatory)

- File header with license and Doxygen metadata.
- Use Doxygen blocks for:
  - classes/interfaces
  - public methods
  - non-trivial private methods
- Include `@param`, `@return`, and behavior constraints (ranges, units, ownership).

## 7. Formatting and Layout

- Formatting is always checked by `clang-format`.
- Do not hand-format against project style; follow formatter output.
- Keep include groups and section banners aligned with existing repository pattern.
