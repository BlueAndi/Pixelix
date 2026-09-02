# clang-format Rules That Bite

Formatting is enforced in CI and on save (clang-format **v18.1.3**, config in
[.clang-format](../../../.clang-format)). Never hand-format against the style — run the
formatter. This file lists the settings you actually need to keep in your head so your
code lands clean the first time.

Check a file without changing it:

```bash
clang-format --dry-run --Werror <file>
```

## Braces — Allman (own line)

`BreakBeforeBraces: Custom` with `AfterClass`, `AfterFunction`, `AfterControlStatement`,
`AfterEnum`, `AfterStruct`, `AfterNamespace`, `AfterUnion` all `true`, plus `BeforeElse`,
`BeforeCatch`, `BeforeWhile` `true`. Every brace goes on its **own line**:

```cpp
if (nullptr == pData)
{
    doGuard();
}
else
{
    doWork();
}
```

`AllowShortFunctionsOnASingleLine: None` — even trivial bodies get braces on their own
lines. Empty functions still split (`SplitEmptyFunction: true`):

```cpp
~Foo()
{
}
```

## Indentation & width

- `IndentWidth: 4`, `UseTab: Never` — four spaces, no tabs.
- `ColumnLimit: 0` — **no hard line-length limit**; the formatter will not wrap long lines
  for you. Keep lines reasonable by judgment, not by a column count.
- `AccessModifierOffset: -4`, `IndentAccessModifiers: false` — `public:` etc. sit at the
  class's own indent level; `EmptyLineAfterAccessModifier: Always`.

## Alignment (do NOT hand-align — the formatter does it)

- `AlignConsecutiveDeclarations` (across empty lines) — member/variable declarations line
  up in columns.
- `AlignConsecutiveAssignments` (across empty lines & comments, `AlignCompound: true`,
  `PadOperators: true`) — runs of assignments line up on `=`.

```cpp
m_counter       = 0U;
m_isInitialized = true;
```

Just write them; the formatter pads the columns.

## Includes

`SortIncludes: Never` — include order is **never** changed. Preserve the existing grouping
and order; do not "tidy" includes.

## Pointers, references, angle brackets

- `PointerAlignment: Left`, `ReferenceAlignment: Left` — bind to the type:
  `const char* key`, `const Foo& other`.
- `SpacesInAngles: Leave` — don't add/remove spaces inside `< >`.

## Function parameters & arguments

- `BinPackParameters: false`, `BinPackArguments: false` — when a call/decl wraps, it's
  **one argument per line**, not packed.
- `AlignAfterOpenBracket: DontAlign` — wrapped arguments indent by one level, not to the
  open paren.

## Constructor initializer lists

- `BreakConstructorInitializers: AfterColon`, `PackConstructorInitializers: Never`,
  `ConstructorInitializerIndentWidth: 4` — colon, then one initializer per line:

```cpp
Foo::Foo() :
    m_isInitialized(false),
    m_counter(0U)
{
}
```

## Spacing

- `SpaceBeforeParens: Custom` → `AfterControlStatements: true`,
  `AfterFunctionDefinitionName: false`: space after `if`/`for`/`while`/`switch`
  (`if (cond)`), **no** space before a function definition's `(` (`void foo()`).
- `MaxEmptyLinesToKeep: 2`, `KeepEmptyLinesAtEOF: false`.
- `Cpp11BracedListStyle: false` — braced init lists keep spaces inside the braces.
- `BreakBeforeBinaryOperators: None` — operators stay at line ends when wrapping.

## Note on the templates

The placeholder tokens in the [assets templates](../assets/) use readable, valid names
(`ExampleClass`, `EXAMPLE_CLASS_H`). Replace them with your real names and the file will
already be formatter-clean. If you introduce angle-bracket placeholders yourself, be aware
clang-format may reflow them oddly — rename to real identifiers before formatting.
