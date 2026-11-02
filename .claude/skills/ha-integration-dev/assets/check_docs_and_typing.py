#!/usr/bin/env python3
"""Check that every definition is documented and fully type annotated."""

# MIT License
#
# Copyright (c) 2019 - 2026 Andreas Merkle (web@blue-andi.de)
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

###############################################################################
# Imports
###############################################################################
import argparse
import ast
import sys
from dataclasses import dataclass
from pathlib import Path

###############################################################################
# Variables
###############################################################################
# Parameters which never appear in an Args: section.
IMPLICIT_PARAMETERS = ("self", "cls")

###############################################################################
# Classes
###############################################################################


@dataclass
class Finding:
    """One deviation from the docstring or typing rules."""

    path: Path
    line: int
    name: str
    reason: str

    def __str__(self) -> str:
        """Format the finding like a compiler diagnostic.

        Returns:
            The finding as ``path:line: name: reason``.
        """
        return f"{self.path}:{self.line}: {self.name}: {self.reason}"


###############################################################################
# Functions
###############################################################################


def _parameters(node: ast.FunctionDef | ast.AsyncFunctionDef) -> list[ast.arg]:
    """Collect every parameter of a callable which must be documented.

    Args:
        node: The function or method definition.

    Returns:
        The parameters without ``self`` and ``cls``.
    """
    candidates = (
        node.args.posonlyargs
        + node.args.args
        + node.args.kwonlyargs
        + [arg for arg in (node.args.vararg, node.args.kwarg) if arg]
    )

    return [arg for arg in candidates if arg.arg not in IMPLICIT_PARAMETERS]


def check_callable(
    path: Path, node: ast.FunctionDef | ast.AsyncFunctionDef
) -> list[Finding]:
    """Check one function or method.

    Args:
        path: The file the callable is defined in.
        node: The function or method definition.

    Returns:
        The findings, empty when the callable is compliant.
    """
    findings: list[Finding] = []
    docstring = ast.get_docstring(node)
    parameters = _parameters(node)

    if docstring is None:
        findings.append(Finding(path, node.lineno, node.name, "no docstring"))
    else:
        if (0 < len(parameters)) and ("Args:" not in docstring):
            findings.append(
                Finding(path, node.lineno, node.name, "no 'Args:' section")
            )
        elif "Args:" in docstring:
            for parameter in parameters:
                if f"{parameter.arg}:" not in docstring:
                    findings.append(
                        Finding(
                            path,
                            node.lineno,
                            node.name,
                            f"parameter '{parameter.arg}' not in 'Args:'",
                        )
                    )

        if ("Returns:" not in docstring) and ("Yields:" not in docstring):
            findings.append(
                Finding(
                    path, node.lineno, node.name, "no 'Returns:'/'Yields:'"
                )
            )

    for parameter in parameters:
        if parameter.annotation is None:
            findings.append(
                Finding(
                    path,
                    node.lineno,
                    node.name,
                    f"parameter '{parameter.arg}' is not annotated",
                )
            )

    if node.returns is None:
        findings.append(
            Finding(path, node.lineno, node.name, "return is not annotated")
        )

    return findings


def check_file(path: Path) -> list[Finding]:
    """Check one Python file.

    Args:
        path: The file to check.

    Returns:
        The findings, empty when the file is compliant.
    """
    findings: list[Finding] = []
    tree = ast.parse(path.read_text(encoding="utf-8"), filename=str(path))

    if ast.get_docstring(tree) is None:
        findings.append(Finding(path, 1, "<module>", "no docstring"))

    for node in ast.walk(tree):
        if isinstance(node, ast.ClassDef):
            if ast.get_docstring(node) is None:
                findings.append(
                    Finding(path, node.lineno, node.name, "no docstring")
                )
        elif isinstance(node, (ast.FunctionDef, ast.AsyncFunctionDef)):
            findings.extend(check_callable(path, node))

    return findings


def collect_files(targets: list[Path]) -> list[Path]:
    """Expand the given files and directories to a list of Python files.

    Args:
        targets: The files and directories given on the command line.

    Returns:
        Every Python file to check, sorted.
    """
    files: set[Path] = set()

    for target in targets:
        if target.is_dir():
            files.update(target.rglob("*.py"))
        else:
            files.add(target)

    return sorted(files)


def main() -> int:
    """Check every given file and report the findings.

    Returns:
        0 if everything is compliant, otherwise 1.
    """
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "targets", nargs="+", type=Path, help="Files or directories to check."
    )
    arguments = parser.parse_args()

    findings: list[Finding] = []
    for path in collect_files(arguments.targets):
        findings.extend(check_file(path))

    for finding in findings:
        print(finding)

    print(f"\n{len(findings)} deviation(s)")

    return 1 if findings else 0


###############################################################################
# Main
###############################################################################
if __name__ == "__main__":
    sys.exit(main())
