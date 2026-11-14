---
name: python-coding-guidelines
description: 'Create, modify, refactor, review, or validate Python code using Pixelix PEP 8, typing, docstring, file-layout, and Pylint guidelines. Use for Python scripts, libraries, tests, and Home Assistant integration code.'
---

# Python Coding Guidelines

Apply these rules to new and edited Python files, including tests. Preserve
existing behavior and avoid unrelated repository-wide rewrites. For Home
Assistant integration work, also load the `ha-integration-dev` skill and obey
its framework-specific requirements.

## Style and file layout

- Follow PEP 8 and PEP 257 with the project's line-length exception: use a
  maximum of 140 characters per line, as configured in `.pylintrc`. Do not
  reflow existing lines solely to meet PEP 8's shorter recommended limits.
  Use four spaces and PEP 8 naming and import grouping. Do not silently change
  the repository configuration.
- Start with an optional required shebang or encoding declaration, then a
  module docstring and the complete existing project MIT license header. Keep
  the original copyright notices when editing existing files; do not invent
  ownership or remove license terms.
- Use the section banners `Imports`, `Variables`, `Classes`, and `Functions`
  in that order, including empty sections. Use 79 `#` characters for new
  files; preserve existing banner widths instead of changing them for length.
  Use a `Main` section when the module has executable entry-point code.
  Follow the integration skill's layout for Home Assistant modules.
- Keep imports, constants, classes, functions, and executable code in their
  respective sections, respecting required language semantics such as
  `from __future__` placement. Do not move executable code if doing so
  changes behavior.

## Documentation and typing

- Document every module, class, function, method, and relevant callable,
  including private helpers, tests, and constructors. Use meaningful
  Google-style docstrings describing behavior and side effects.
- Document every parameter, including `self` or `cls` where present, and
  document return values, including `None`; document yielded values and
  relevant exceptions where applicable. Keep descriptions consistent with
  actual behavior and annotations.
- Annotate all explicit function and method parameters and return types;
  implicit `self` and `cls` may use Python's inferred instance/class type.
  Annotate module and class variables and instance attributes where their
  types are not clear. Prefer precise types over `Any` and avoid casts unless
  justified. Respect the target Python version and runtime framework.

## Validation and duplicate code

1. Determine the supported runtime and required dependencies. Run relevant
   tests and a PEP 8 checker such as `pycodestyle --max-line-length=140` on
   changed Python files.
   Report missing tools, dependencies, and any checks that cannot run.
2. Run full Pylint, not `--errors-only`, over the relevant Python files
   together, using the repository configuration and all enabled diagnostics.
   Include related modules in the same invocation so cross-file duplicates
   can be detected. Example:

   ```text
   python -m pylint --rcfile=.pylintrc --min-similarity-lines=9 path/to/first.py path/to/second.py
   ```

   For Pylint 4.1.3, the similarity checker reports more than the configured
   value, so `9` sets the reporting threshold to **10 counted lines**.
   Comments, docstrings, imports, and other non-code lines may be ignored
   according to Pylint's similarity settings; this is not a raw line count.
   If the Pylint version changes, verify the boundary before adjusting the
   option. Do not disable the duplicate-code checker.
3. Aim for zero PEP 8 and Pylint findings. Correct findings caused by the
   change and report pre-existing findings outside the task's scope; do not
   claim a clean check if any remain. Do not use blanket suppressions or
   silently ignore errors.
4. For every reported `duplicate-code` finding, show the affected locations
   and a concrete behavior-preserving refactoring proposal, then **ask the
   user** to choose between that refactoring and a narrowly scoped Pylint
   suppression. Wait for the decision before resolving the finding. If
   suppression is chosen, use the narrowest effective scope, explain its
   reason, and rerun the full check. Never suppress it without approval.
5. Verify the file structure, annotations, and complete docstrings separately:
   Pylint alone does not enforce all of these rules. Report exact commands and
   results, including unresolved findings or unverified requirements.
