# Preprocessing

PlantUML's preprocessor runs before a diagram is drawn. It mirrors the C
preprocessor, with `!` in place of the leading `#`. It works in every diagram
type.

## Variables

Define with `!$name = value`; names should start with `$`. Values may be
integers, quoted strings or JSON. Variables defined outside a function are
global. Reference a variable by its name.

```plantuml
!$service = "Auth"
!$retries = 3
Alice -> Bob : connect to $service
```

`?=` assigns only if the variable is not already set — useful for defaults a
caller can override earlier in the file.

```plantuml
!$name ?= "Charlie"
```

## Conditions

`!if`, `!elseif`, `!else`, `!endif`. Expressions support `&&`, `||` and
parentheses. `0` is false; any other number or string is true.

```plantuml
!$level = 10
!if ($level >= 5)
Alice -> Bob : high priority
!else
Alice -> Bob : low priority
!endif
```

## Loops

`!while` / `!endwhile`. Update the controlling variable inside the body.

```plantuml
!$i = 1
!while $i <= 3
Alice -> Bob : message $i
!$i = $i + 1
!endwhile
```

## Procedures

A procedure emits diagram text when called. Names and arguments start with `$`;
procedures may call other procedures, and variables defined inside are local to
the call.

```plantuml
!procedure $msg($from, $to)
$from --> $to : hello
!endprocedure

$msg("Alice", "Bob")
```

## Functions

A function emits nothing and instead returns a value usable in expressions or
inline in diagram text. `!return` produces the result.

```plantuml
!function $double($a)
!return $a + $a
!endfunction

Alice -> Bob : the double of 3 is $double(3)
```

Both procedures and functions may declare defaults for trailing arguments, which
callers can then omit.

```plantuml
!function $inc($value, $step=1)
!return $value + $step
!endfunction
```

## Including files

`!include` pulls in another file so shared definitions live in one place. It also
accepts URLs. A file may be included more than once; `!include_once` forbids
that.

```text
!include List.iuml
!include <archimate/Archimate>
```

The include target must be reachable by the renderer, so this snippet only works
where those files exist.

Keep shared skinparams, sprites and macros in an included `.iuml` file rather
than repeating them in every diagram.

## Built-in functions

Helper functions whose names begin with `%` cover strings, maths, colours and
dates, and can be used inline in diagram text.

```plantuml
Alice -> Bob : %upper("hello")
Bob --> Alice : 10 mod 3 is %mod(10, 3)
```
