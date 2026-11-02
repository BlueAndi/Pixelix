---
name: ha-integration-dev
description: Use when writing, reviewing or refactoring Python code of a Home Assistant custom integration in this repository (doc/homeassistant/integration/) — custom_components modules, manifest.json, config_flow.py, coordinators, entities, actions/services.yaml, translations/ and pytest tests; also when a change must stay PEP8-clean with zero pycodestyle/pylint findings, needs full type annotations, or needs Google-style docstrings documenting every parameter and return value. Covers the house Python file layout (MIT license header, section banners). Not for C++ firmware, build scripts under scripts/, or web assets.
---

# Home Assistant Integration Development

## Overview

The Pixelix Home Assistant integration lives in
[doc/homeassistant/integration/](../../../doc/homeassistant/integration/) and is a
**custom integration** — it is copied into `<HA config>/custom_components/`, so
it must satisfy the rules of
[developers.home-assistant.io](https://developers.home-assistant.io/docs/creating_component_index/)
*and* the Pixelix house style.

Three layers, all mandatory:

1. **Framework** — Home Assistant's integration contract (manifest, config
   entries, config flow, entities, actions, translations). See
   [references/ha-framework.md](references/ha-framework.md).
2. **File layout** — module docstring, MIT license block, section banners,
   exactly like the Python scripts in [scripts/](../../../scripts/). See
   [The File Layout](#the-file-layout) and
   [assets/module_template.py](assets/module_template.py).
3. **Lint cleanliness** — **zero** findings from `pycodestyle` and `pylint`.
   See [Zero-Warning Gate](#zero-warning-gate).

The existing integration already satisfies all three. When in doubt, copy the
pattern from a neighbouring module rather than inventing one.

## The Non-Negotiables

Seven rules. Violating any of them fails review.

### 1. The file layout is fixed

Every `.py` file — source *and* test — has this skeleton, in this order:

```python
"""One short sentence, imperative or nominal, ending with a period."""

# MIT License
# ... full block, see assets/module_template.py ...

###############################################################################
# Imports
###############################################################################

###############################################################################
# Variables
###############################################################################

###############################################################################
# Classes
###############################################################################

###############################################################################
# Functions
###############################################################################
```

- Banners are **79 `#` characters**, never 80. (The `scripts/` banners are 80
  because those files run at a 140-column pylint limit; integration code runs
  at the PEP8 79 limit, so the banner matches it.)
- **All four banners are always present**, even when the section is empty —
  see [const.py](../../../doc/homeassistant/integration/custom_components/pixelix/const.py),
  which has empty `Imports`, `Classes` and `Functions` sections.
- There is **no `Main` section** in integration modules (unlike `scripts/`,
  which are executables).
- The docstring comes **before** the license block, never after.

### 2. 79 columns, hard

PEP8's limit, enforced by `pycodestyle`'s default. The repo's
[.pylintrc](../../../.pylintrc) says `max-line-length=140`, which applies to
`scripts/` — **it does not relax the integration**, because `pycodestyle` is
the stricter of the two gates and both must be clean.

Wrap with parentheses, not backslashes:

```python
# Good
PASSWORD_SELECTOR = TextSelector(
    TextSelectorConfig(type=TextSelectorType.PASSWORD)
)

# Bad - backslash continuation
PASSWORD_SELECTOR = TextSelector( \
    TextSelectorConfig(type=TextSelectorType.PASSWORD))
```

### 3. Everything is async, nothing blocks

The event loop must never block. No `requests`, no `time.sleep()`, no
synchronous file I/O, no blocking library call inside a coroutine.

```python
# Good - HA's shared aiohttp session
session = async_get_clientsession(hass)

# Bad - blocks the event loop, HA will log a warning and may raise
response = requests.get(url, timeout=10)
```

Need a blocking call? `await hass.async_add_executor_job(func, arg)`.

### 4. Never mutate a ConfigEntry

> "A `ConfigEntry` object, including the data and options, must never be
> mutated directly by integrations, instead integrations must call
> `async_update_entry`."

```python
# Good
hass.config_entries.async_update_entry(entry, data={**entry.data, CONF_HOST: host})

# Bad
entry.data[CONF_HOST] = host
```

### 5. `translations/`, never `strings.json`

`strings.json` is a Home Assistant *Core* build-time artefact. A custom
integration that ships one gets raw keys (`username` instead of "Username") in
the UI. Put the complete, flat English text in
`custom_components/<domain>/translations/en.json`, and a translation per
language beside it (`de.json`, …).

Every user-visible string referenced by the code needs a key: each config-flow
`step_id`, every schema field under `data` and `data_description`, every
`errors` key, every `async_abort(reason=…)`.

### 6. Everything is documented — no exceptions

**Every** module, class, method, function, property, generator and nested or
private helper carries a Google-style docstring, and that docstring documents
**every parameter and the return value**.

```python
async def async_send_notification(
    self, text: str, duration: int | None = None
) -> dict[str, Any]:
    """Send a notification to the device.

    Args:
        text: The text to show. At most 256 bytes in UTF-8.
        duration: The display time in ms, or None for the default of
            the firmware.

    Returns:
        The JSON answer of the device.

    Raises:
        PixelixConnectionError: The device is not reachable.
        PixelixAuthError: The web login was rejected.
    """
```

The sections, and when each is mandatory:

| Section | Required when |
| --- | --- |
| Summary line | Always. One sentence, ends with a period, starts on the `"""` line. |
| `Args:` | The callable takes parameters. One entry per parameter, `self`/`cls` excluded. No types — the signature carries them. |
| `Returns:` | **Always**, including `-> None`, where it reads `None.` |
| `Yields:` | Generators and fixtures — instead of `Returns:`. |
| `Raises:` | The callable raises deliberately. One entry per exception type. |

Scope rules:

- A **class** docstring describes what the class represents. The constructor
  parameters are documented on `__init__`, not on the class.
- A **property** documents its `Returns:` like any other callable.
- `del unused_fixture` in a test still leaves that fixture documented under
  `Args:` — it is part of the signature.

Pylint's `missing-*-docstring` catches a *missing* docstring. It does **not**
check that `Args:` and `Returns:` are present or complete. A `10.00/10` score
therefore proves nothing here, so this rule has its own checker:

```bash
python ../../../.claude/skills/ha-integration-dev/assets/check_docs_and_typing.py \
    custom_components tests
```

It reports `path:line: name: reason` and exits non-zero on any finding.

> **The existing code is not yet fully compliant.** At the time of writing the
> checker reports **4** deviations in `custom_components/pixelix` and **129**
> in `tests/` — most test functions document neither `Args:` nor `Returns:`.
> This rule applies to **code you write or touch**. Do not copy the docstring
> style of a neighbouring test; copy the style of
> [assets/test_template.py](assets/test_template.py) instead. Run the checker
> on the files you changed, not on the whole tree, and leave it cleaner than
> you found it.

| Rationalization | Reality |
| --- | --- |
| "It's a one-line private helper" | Private and short still gets `Args:` and `Returns:`. |
| "The name says what it returns" | Write it out. The next reader greps for `Returns:`. |
| "It returns None, that's obvious" | `Returns:` then `None.` — spell it out. |
| "It's just a test function" | Tests follow the same rule, fixtures included. |
| "Pylint is already at 10.00/10" | Pylint cannot see missing `Args:`/`Returns:`. Run the checker. |
| "The type hint already says it" | The hint gives the type, the docstring gives the meaning. |
| "The neighbouring test doesn't do it either" | That is the backlog, not the standard. |

### 7. Full typing on every signature

Every parameter and every return is annotated — including `-> None`, including
private helpers, including test functions and fixtures. Annotate module-level
constants and instance attributes whose type is not obvious from the value.

```python
# Good
def normalize_name(name: str) -> str:
def _handle_coordinator_update(self) -> None:
async def async_step_user(
    self, user_input: dict[str, Any] | None = None
) -> ConfigFlowResult:

# Bad - unannotated parameter, missing return type
def normalize_name(name):
```

Modern syntax only (the integration runs on the Python version HA ships,
currently ≥ 3.12):

| Use | Not |
| --- | --- |
| `dict[str, Any]`, `list[str]`, `tuple[int, ...]` | `Dict`, `List`, `Tuple` |
| `str \| None` | `Optional[str]` |
| `int \| str` | `Union[int, str]` |
| `from collections.abc import Callable` | `from typing import Callable` |
| `type MyEntry = ConfigEntry[MyData]` (PEP 695) | bare `ConfigEntry` |

`from __future__ import annotations` is not needed. Reach for `Any` only when
the shape genuinely is arbitrary, such as a decoded JSON body.

## Zero-Warning Gate

Run both, from `doc/homeassistant/integration/`:

```bash
python -m pip install pycodestyle pylint
python -m pip install -r requirements_test.txt

python -m pycodestyle custom_components/pixelix tests
python -m pylint --rcfile="$(git rev-parse --show-toplevel)/.pylintrc" \
    custom_components/pixelix tests
python -m pytest

# Docstrings and typing - the linters above cannot check these.
# Run it on the files you changed; see Non-Negotiable 6 on the backlog.
python "$(git rev-parse --show-toplevel)/.claude/skills/ha-integration-dev/assets/check_docs_and_typing.py" \
    <the files you changed>
```

**Always pass the workspace-root [.pylintrc](../../../.pylintrc) explicitly.**
Pylint only auto-discovers an rc file in the current directory or upwards
through packages that have an `__init__.py` — it will **not** find the
workspace root from `doc/homeassistant/integration/`, so without `--rcfile`
you silently lint against pylint's own defaults and get different results
than CI. If the file is ever absent, run without the flag and say so.

**Required result:** `pycodestyle` prints nothing and exits 0; `pylint`
reports `10.00/10`; `pytest` passes. Anything else is not done.

Verified on the current integration — both `custom_components/pixelix` and
`tests` are already clean. Note that `pylint` on `tests/` reports
`import-error` for `pytest` and the HA test helpers unless
`requirements_test.txt` is installed first; install it before judging a
finding to be real.

### Suppressing what cannot be fixed

`homeassistant`, `voluptuous` and `pytest_homeassistant_custom_component` are
supplied by the HA runtime, not by this repo, so pylint cannot resolve them.
This is the **only** blanket suppression in the codebase, and it carries a
comment explaining why:

```python
# Home Assistant and voluptuous are provided by the HA runtime only.
# pylint: disable=import-error
import voluptuous as vol
```

Anything else uses the narrowest possible form — `# pylint: disable-next=<id>`
on the single line, with a comment saying why:

```python
# Home Assistant deprecated is_matching(), a flow does not implement it.
# pylint: disable-next=abstract-method
class PixelixConfigFlow(ConfigFlow, domain=DOMAIN):
```

Never add a suppression to silence a finding you could fix instead. Never
disable a whole file for one line.

## Quick Reference

### File set of a custom integration

| File | Required | Purpose |
| --- | --- | --- |
| `manifest.json` | yes | Metadata. Custom integrations **must** set `version`. |
| `__init__.py` | yes | `async_setup_entry` / `async_unload_entry`, action registration. |
| `const.py` | convention | `DOMAIN` and all constants. |
| `config_flow.py` | if `config_flow: true` | UI setup, discovery, reauth, reconfigure. |
| `translations/en.json` | if config flow / actions | All user-visible text. |
| `services.yaml` | if actions | Action fields and selectors for the UI. |
| `api.py` | convention | Device/cloud client, own exception hierarchy. |
| `coordinator.py` | if polling | `DataUpdateCoordinator` subclass. |
| `tests/` | yes | pytest, via `pytest-homeassistant-custom-component`. |

### Config entry lifecycle

| Hook | Signature | Notes |
| --- | --- | --- |
| Setup | `async def async_setup_entry(hass, entry) -> bool` | Store state in `entry.runtime_data`. |
| Unload | `async def async_unload_entry(hass, entry) -> bool` | Must undo everything setup did. |
| Migrate | `async def async_migrate_entry(hass, entry) -> bool` | Bump `ConfigFlow.VERSION` together with it. |
| Remove | `async def async_remove_entry(hass, entry) -> None` | Cleanup on deletion. |

### Raising the right error

| Situation | Raise |
| --- | --- |
| Device temporarily unreachable during setup | `ConfigEntryNotReady` |
| Credentials rejected | `ConfigEntryAuthFailed` (starts the reauth flow) |
| Unrecoverable setup problem | `ConfigEntryError` |
| Bad user input to an action | `ServiceValidationError` |
| Anything else at runtime | `HomeAssistantError` |
| Coordinator refresh failed | `UpdateFailed` |

Never let a bare `Exception` escape to the user.

## Common Mistakes

| Mistake | Fix |
| --- | --- |
| 80-wide `#` banner copied from `scripts/` | 79 wide in integration code. |
| Section banner omitted because the section is empty | Keep all four, always. |
| License block placed above the module docstring | Docstring first, then license. |
| `strings.json` added to the custom component | Use `translations/en.json`. |
| `requests` / `time.sleep()` in a coroutine | `aiohttp` via `async_get_clientsession`, `asyncio.sleep`. |
| Host or IP used as the config entry unique id | Use a stable device id; IPs change. |
| `entry.data[...] = x` | `hass.config_entries.async_update_entry(...)`. |
| Listener registered in setup, not removed on unload | `entry.async_on_unload(unsub)`. |
| `# pylint: disable=` at file scope to hide one finding | `disable-next` on that line, with a reason. |
| Docstring has a summary but no `Args:` / `Returns:` | Both are mandatory; `Returns:` even for `-> None`. |
| Parameter added to a signature, docstring not updated | Every parameter has an `Args:` entry. |
| `Returns:` omitted on a property or a test function | Properties and tests follow the same rule. |
| Unannotated parameter or missing `-> None` | Annotate every parameter and every return. |
| `Optional[str]` / `Dict[str, Any]` / `typing.Callable` | `str \| None`, `dict[str, Any]`, `collections.abc.Callable`. |
| Secrets or notification text written to the log | Log the device, never the payload or credentials. |
| New action field added to code only | Also `services.yaml` **and** every `translations/*.json`. |

## Workflow for a Change

1. Read the neighbouring module — the pattern almost always exists already.
2. Check [references/ha-framework.md](references/ha-framework.md) for the HA
   API you need, and the upstream docs it links for anything deeper.
3. Write the code; start new files from
   [assets/module_template.py](assets/module_template.py) (tests:
   [assets/test_template.py](assets/test_template.py)).
4. Update `translations/en.json` *and* `de.json`, plus `services.yaml`, for
   every user-visible string you touched.
5. Bump `manifest.json` `version` for a released behaviour change.
6. Run the [Zero-Warning Gate](#zero-warning-gate) and `pytest`. Paste the
   actual output; do not claim a pass you did not see.
7. Run `assets/check_docs_and_typing.py` on the files you changed — the
   linters do not see missing `Args:`, `Returns:` or annotations.
8. Update [README.md](../../../doc/homeassistant/integration/README.md) and
   [doc/HOMEASSISTANT.md](../../../doc/HOMEASSISTANT.md) when user-facing
   behaviour changed.
