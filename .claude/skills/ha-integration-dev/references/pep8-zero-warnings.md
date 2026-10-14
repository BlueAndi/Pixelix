# PEP8 With Zero Warnings

The goal is not "mostly clean". It is `pycodestyle` silent and `pylint`
`10.00/10`, every time, before the change is called done.

```bash
cd doc/homeassistant/integration
python -m pycodestyle custom_components/pixelix tests
python -m pylint --rcfile="$(git rev-parse --show-toplevel)/.pylintrc" \
    custom_components/pixelix tests
```

## Always point pylint at the workspace-root .pylintrc

The repo keeps one [.pylintrc](../../../../.pylintrc) at the workspace root. Pass
it with `--rcfile` on every run.

Pylint's auto-discovery does **not** reach it from here: it looks at a
`pylintrc`/`.pylintrc` in the current directory, then walks up only as long as
each parent is a package (has `__init__.py`), then falls back to
`~/.pylintrc` and the system file. `doc/homeassistant/integration/` is not a
package chain to the root, so an unqualified `python -m pylint …` lints
against pylint's built-in defaults — different line limit, different enabled
checks, different score than CI. Pass the flag, or your `10.00/10` means
nothing.

From a subdirectory, `$(git rev-parse --show-toplevel)/.pylintrc` resolves it
regardless of where you are. A plain relative `../../../.pylintrc` works too
when you are exactly in `doc/homeassistant/integration/`. If the file is
genuinely missing, run without the flag and state that in the result.

`pycodestyle` meanwhile runs with its defaults — **79 columns**, no
configuration file for the integration. The root `.pylintrc` only raises
*pylint's* line limit to 140 (which is what `scripts/` needs); pycodestyle
still enforces 79, so 79 wins for integration code.

---

## The error codes you will actually hit

| Code | Meaning | Fix |
| --- | --- | --- |
| `E501` | line too long | Wrap in parentheses; extract a local variable or constant. |
| `E302` / `E303` | expected 2 blank lines / too many | Two blank lines between top-level defs, one between methods. |
| `E128` / `E127` | continuation line misaligned | Use a hanging indent of 4, closing bracket on its own line. |
| `E226` | missing whitespace around arithmetic operator | `a * b`, not `a*b`. |
| `W291` / `W293` | trailing whitespace (also on blank lines) | Strip it. Editors do this on save. |
| `W391` | blank line at end of file | Exactly one newline, no blank line. |
| `E402` | import not at top of file | Imports go in the `Imports` section only. |
| `E731` | lambda assigned to a name | Use `def`. |
| `E722` | bare `except:` | Catch the specific exception. |

Pylint additions that bite in HA code:

| Code | Meaning | Fix |
| --- | --- | --- |
| `import-error` | `homeassistant` / `voluptuous` not installed here | Documented blanket disable in the import block — see below. |
| `missing-function-docstring` | no docstring | Add one. Google style with `Args:` / `Returns:`. |
| `invalid-name` | module constant not `UPPER_CASE` | Rename, or `disable-next=invalid-name` where HA dictates the name (e.g. `CONFIG_SCHEMA`). |
| `too-many-arguments` | long signature | Group into a dataclass, or justify with `disable-next`. |
| `unused-argument` | fixture injected only for its side effect | `del fixture_name` at the top of the body — the repo's pattern in `conftest.py`. |
| `abstract-method` | `ConfigFlow.is_matching()` not implemented | HA deprecated it; `disable-next` with that comment. |
| `duplicate-code` | similar test bodies | Pull the shared part into `tests/helpers.py`. |

---

## Wrapping at 79 columns

Parentheses, never backslashes.

```python
# Imports
from homeassistant.const import (
    CONF_HOST,
    CONF_PASSWORD,
    CONF_PORT,
    CONF_USERNAME,
)

# Calls
vol.Optional(ATTR_MELODY): vol.All(
    cv.string, max_bytes(MAX_MELODY_BYTES)
),

# A value that still does not fit
vol.Required(CONF_PASSWORD, default=DEFAULT_PASSWORD): (
    PASSWORD_SELECTOR
),

# Signatures
async def async_step_user(
    self, user_input: dict[str, Any] | None = None
) -> ConfigFlowResult:
```

Long literal text — a log message, a docstring sentence — is wrapped as
adjacent string literals inside parentheses, never cut mid-word with a
backslash:

```python
_LOGGER.debug(
    "Device %s did not answer within %d s, falling back to REST.",
    host,
    timeout,
)
```

Prefer `%s` placeholders over f-strings in logging calls: pylint's
`logging-fstring-interpolation` flags the latter, and lazy formatting skips
the work when the level is disabled.

---

## Import order

Three groups, separated by one blank line, alphabetical within each:

```python
###############################################################################
# Imports
###############################################################################
import asyncio
import logging
from collections.abc import Callable
from dataclasses import dataclass
from typing import Any

# Home Assistant and voluptuous are provided by the HA runtime only.
# pylint: disable=import-error
import voluptuous as vol

from homeassistant.config_entries import ConfigEntry
from homeassistant.core import HomeAssistant
from homeassistant.helpers import config_validation as cv

from .api import PixelixClient, PixelixError
from .const import DOMAIN
```

1. Standard library.
2. Third party and `homeassistant.*`, preceded by the `import-error` disable
   with its explanatory comment.
3. Local, relative (`.api`, `.const`). Tests import absolutely
   (`custom_components.pixelix.const`) plus relative `.helpers`.

---

## Docstrings

Google style. Module, class, method, function, property, generator — all of
them, including private helpers, fixtures and test functions. Every parameter
and the return value are documented. See Non-Negotiable 6 in
[SKILL.md](../SKILL.md) for the full rule and its rationalization table.

```python
def normalize_name(name: str) -> str:
    """Normalize the host name of a device to get its unique id.

    The name which a device announces by mDNS and the name which it reports
    by its REST API differ at most in the case and a ``.local`` suffix.

    Args:
        name: The host name of the device.

    Returns:
        The name in lower case without a ``.local`` suffix.
    """
    return name.strip().lower().removesuffix(".local")
```

- Summary line: one sentence, ends with a period, fits on the line with the
  opening `"""`.
- `Args:` lists **every** parameter, no types (the signature has them).
  `self` and `cls` are excluded. A parameter the body discards with
  `del name` is still documented — it is part of the signature.
- `Returns:` is **always** present. For `-> None`, write `None.`.
- Generators and fixtures use `Yields:` instead of `Returns:`.
- `Raises:` when the function raises deliberately, one entry per type.
- A class docstring describes the class; its constructor parameters are
  documented on `__init__`, not on the class.
- Double backticks for inline code, as above.

Pylint's `missing-module-docstring` / `missing-class-docstring` /
`missing-function-docstring` only detect an *absent* docstring. They do not
verify that `Args:` and `Returns:` exist or are complete, so `10.00/10` is no
evidence for this rule. Check it by reading the diff.

---

## Type hints

Mandatory on every signature — every parameter and every return, including
`-> None`, private helpers, fixtures and test functions. Annotate module-level
constants and instance attributes whose type is not obvious from the value.
Modern syntax only (the integration targets the Python version HA ships,
currently ≥ 3.12):

```python
dict[str, Any]                       # not Dict[str, Any]
str | None                           # not Optional[str]
list[str]                            # not List[str]
Callable[[], None]
type PixelixConfigEntry = ConfigEntry[PixelixRuntimeData]   # PEP 695
```

`from __future__ import annotations` is not needed.

---

## Formatters

The integration is **hand-formatted to 79 columns**, matching the file layout
with its 79-wide banners. Do not run `black` or `ruff format` over it — their
default is 88 and they would reflow the whole tree and break the banner
alignment. (`ruff format` is the Home Assistant *Core* standard; this is a
custom integration in a non-HA repository, which follows this repo's style.)

---

## Before you claim it is done

```bash
cd doc/homeassistant/integration
python -m pip install -r requirements_test.txt          # pylint needs these
python -m pycodestyle custom_components/pixelix tests   # expect: no output
python -m pylint --rcfile="$(git rev-parse --show-toplevel)/.pylintrc" \
    custom_components/pixelix tests                     # expect: 10.00/10
python -m pytest                                        # expect: all passed
```

Read the output. "Should be fine" is not a result.
