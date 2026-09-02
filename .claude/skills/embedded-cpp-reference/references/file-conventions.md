# File Conventions (anatomy of a Pixelix source file)

Every `.h`, `.hpp`, and `.cpp` follows the same skeleton. Copy a
[template](../assets/template.h) and fill the placeholders; this file explains each part.

## 1. MIT license header

The exact block that opens every file (do not reword):

```cpp
/* MIT License
 *
 * Copyright (c) 2019 - 2026 Andreas Merkle <web@blue-andi.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * ... (full MIT text) ...
 * SOFTWARE.
 */
```

## 2. Description / `@file` block

A `DESCRIPTION` banner followed by the Doxygen file block. Headers also open a Doxygen
group with `@addtogroup <GROUP>` and `@{`, closed by `@}` at the very end of the file.

```cpp
/*******************************************************************************
    DESCRIPTION
*******************************************************************************/
/**
 * @file   KeyValueBool.h
 * @brief  Key value pair with bool type
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup SETTINGS
 *
 * @{
 */
```

`.cpp` files use the same `@file`/`@brief`/`@author` block but **without** the
`@addtogroup`/`@{`/`@}` group markers.

## 3. Include guard (headers)

Uppercase, derived from the file name, no leading underscore. `.hpp` uses the `_HPP`
suffix, `.h` uses `_H`.

```cpp
#ifndef KEY_VALUE_BOOL_H
#define KEY_VALUE_BOOL_H
...
#endif /* KEY_VALUE_BOOL_H */
```

`#pragma once` is **not** used — match the `#ifndef` guard style.

## 4. Section banners (fixed order)

Sections appear in this order, each introduced by a banner comment, even when empty
(empty sections are kept as placeholders so every file reads the same way).

**Header (`.h` / `.hpp`):**

```
Compile Switches
Includes
Macros
Types and Classes
Variables
Functions
```

**Source (`.cpp`):**

```
Includes
Compiler Switches
Macros
Types and classes
Prototypes
Local Variables
Public Methods
Protected Methods
Private Methods
External Functions
Local Functions
```

Banner format (78-char rule of asterisks):

```cpp
/******************************************************************************
 * Includes
 *****************************************************************************/
```

## 5. Include grouping

`SortIncludes: Never` — order is intentional, never auto-sort. Typical order: the file's
own header first (in the `.cpp`), then project headers (`"..."`), then framework/3rd-party
and standard headers (`<...>`), grouped with blank lines. Match the neighbouring file.

```cpp
#include "MyClass.h"

#include <Logging.h>
#include <stdint.h>
```

## 6. Class and member conventions

- Access sections are `public:` → `protected:` → `private:`, each followed by a blank line
  (`EmptyLineAfterAccessModifier: Always`) and offset to column 0
  (`AccessModifierOffset: -4`).
- Data members use the **`m_` prefix**, are documented inline with `/**< ... */`, and are
  aligned into columns by `clang-format` (`AlignConsecutiveDeclarations`).
- Initialize every member in the constructor initializer list, in declaration order.
  Initializers break after the colon, one per line
  (`BreakConstructorInitializers: AfterColon`, `PackConstructorInitializers: Never`).

```cpp
KeyValueBool(const char* key, const char* name, bool defValue) :
    KeyValue(),
    m_key(key),
    m_name(name),
    m_defValue(defValue)
{
}

private:

    const char* m_key;      /**< Key */
    const char* m_name;     /**< Name */
    bool        m_defValue; /**< Default value */
```

- Mark overrides `override`, sealed overrides `final`.

## 7. Non-copyable idiom

When an instance must not be copied, declare the copy constructor and assignment operator
`private` and leave them **undefined** (the C++14 style used throughout the repo — not
`= delete`):

```cpp
private:
    ...
    /* An instance shall not be copied. */
    KeyValueBool(const KeyValueBool& kv);
    KeyValueBool& operator=(const KeyValueBool& kv);
```

## 8. End of file

- Headers close the Doxygen group after the include guard's `#endif`:

```cpp
#endif /* KEY_VALUE_BOOL_H */

/** @} */
```

- `KeepEmptyLinesAtEOF: false` — no trailing blank lines beyond the single newline the
  formatter keeps.
