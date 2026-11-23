#!/usr/bin/env python3
"""Run clang-tidy with the PlatformIO checker configuration without hitting the Windows command line limit.

PlatformIO's ``check`` command passes every source file and every include path
of the environment to clang-tidy in a single command line. For an ESP32 Arduino
project the include list alone is far beyond the 32767 characters Windows
allows, therefore the check fails on Windows.

This script reads the same configuration PlatformIO would use (``check_*``
options of the environment, resolved via ``pio project config``, including
``config/checker.ini``), collects the build metadata of the environment
(``pio project metadata``) and writes a ``compile_commands.json`` with one
entry per source file. clang-tidy is then started once per file with
``-p <database>`` so that the command line stays short, independent of the
number of include paths and defines.

Example:
    python scripts/check_clangtidy.py --environment esp32doit-devkit-v1-LED-32x8 --fail-on-defect=medium
"""

# MIT License
#
# Copyright (c) 2019 - 2026 Andreas Merkle <web@blue-andi.de>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

################################################################################
# Imports
################################################################################
import argparse
import glob
import json
import os
import re
import shlex
import shutil
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any, Optional

################################################################################
# Variables
################################################################################
SEVERITIES = ("low", "medium", "high")

CATEGORY_TO_SEVERITY = {
    "error": "high",
    "warning": "medium",
    "note": "low",
}

C_EXTENSIONS = (".c",)
CPP_EXTENSIONS = (".cc", ".cpp", ".cxx", ".ino")
HEADER_EXTENSIONS = (".h", ".hh", ".hpp", ".hxx")

# Same format PlatformIO parses: <file>:<line>:<column>: <category>: <message> [<check>]
DEFECT_PATTERN = re.compile(r"^(?P<file>.*):(?P<line>\d+):(?P<column>\d+):\s+(?P<category>[^:]+):\s(?P<message>.+)\[(?P<check>[^]]+)\]$")

SRC_FILTER_PATTERN = re.compile(r"(\+|-)<([^>]+)>")

COMPILE_ERROR_CHECK = "clang-diagnostic-error"

# gcc maps these to internal macros clang doesn't know, clang has its own built-in versions.
IGNORED_TOOLCHAIN_DEFINES = ("__has_include", "__has_include_next")

# clang-tidy returns 1 for non critical compilation errors, like PlatformIO only >= 2 is a tool failure.
CLANG_TIDY_MAX_OK_RETURNCODE = 1

EXIT_OK = 0
EXIT_DEFECTS = 1
EXIT_ERROR = 2

################################################################################
# Classes
################################################################################


@dataclass
class CheckConfig:
    """Checker configuration of one PlatformIO environment."""

    tools: list[str] = field(default_factory=lambda: ["clangtidy"])
    severities: list[str] = field(default_factory=lambda: list(SEVERITIES))
    src_filters: list[str] = field(default_factory=list)
    clangtidy_flags: list[str] = field(default_factory=list)
    skip_packages: bool = False
    packages_dir: Optional[Path] = None


@dataclass
class BuildInfo:
    """Compiler related information needed to create the compile database."""

    target: Optional[str] = None
    includes: list[str] = field(default_factory=list)
    defines: list[str] = field(default_factory=list)
    c_defines: list[str] = field(default_factory=list)
    cxx_defines: list[str] = field(default_factory=list)
    c_std: Optional[str] = None
    cxx_std: Optional[str] = None


@dataclass(frozen=True)
class Defect:
    """One clang-tidy diagnostic."""

    file: str
    line: int
    column: int
    category: str
    severity: str
    message: str
    check: str


@dataclass
class ToolRun:
    """Result of one clang-tidy invocation."""

    file: Path
    returncode: int
    lines: list[str]


################################################################################
# Functions
################################################################################


def warn(message: str) -> None:
    """Print a warning to stderr.

    Args:
        message: Warning text.
    """
    print(f"Warning: {message}", file=sys.stderr)


def error(message: str) -> None:
    """Print an error to stderr.

    Args:
        message: Error text.
    """
    print(f"Error: {message}", file=sys.stderr)


def find_pio() -> list[str]:
    """Determine how the PlatformIO core CLI can be started.

    Returns:
        Command prefix for the PlatformIO CLI.
    """
    for name in ("platformio", "pio"):
        path = shutil.which(name)
        if path:
            return [path]
    return [sys.executable, "-m", "platformio"]


def run_json(cmd: list[str], cwd: Path) -> Any:
    """Run a command and parse its stdout as JSON.

    Args:
        cmd: Command and arguments.
        cwd: Working directory.

    Returns:
        Parsed JSON.

    Raises:
        RuntimeError: If the command fails or doesn't return JSON.
    """
    result = subprocess.run(cmd, cwd=cwd, capture_output=True, text=True, encoding="utf-8", errors="replace", check=False)
    if 0 != result.returncode:
        raise RuntimeError(f"'{' '.join(cmd)}' failed with exit code {result.returncode}:\n{result.stderr}")
    try:
        return json.loads(result.stdout)
    except json.JSONDecodeError as exc:
        raise RuntimeError(f"'{' '.join(cmd)}' returned no valid JSON: {exc}") from exc


def as_list(value: Any) -> list[str]:
    """Convert a PlatformIO option value to a list of strings.

    Args:
        value: Option value, either a scalar or a list.

    Returns:
        List of strings.
    """
    if value is None:
        return []
    if isinstance(value, (list, tuple)):
        return [str(item) for item in value]
    return [str(value)]


def as_bool(value: Any) -> bool:
    """Convert a PlatformIO option value to a boolean.

    Args:
        value: Option value.

    Returns:
        Boolean.
    """
    if isinstance(value, bool):
        return value
    return str(value).strip().lower() in ("1", "true", "yes", "on")


def section_items(pio_config: list[Any], section: str) -> Optional[dict[str, Any]]:
    """Get the options of one section of the resolved PlatformIO configuration.

    Args:
        pio_config: Output of ``pio project config --json-output``.
        section: Section name, e.g. ``env:native``.

    Returns:
        Options of the section or None if the section doesn't exist.
    """
    for name, items in pio_config:
        if name == section:
            return dict(items)
    return None


def default_environment(pio_config: list[Any]) -> Optional[str]:
    """Get the first default environment of the project.

    Args:
        pio_config: Output of ``pio project config --json-output``.

    Returns:
        Environment name or None.
    """
    platformio = section_items(pio_config, "platformio") or {}
    default_envs = as_list(platformio.get("default_envs"))
    return default_envs[0] if default_envs else None


def normalize_src_filter(src_filter: str) -> str:
    """Normalize one source filter like PlatformIO does.

    Args:
        src_filter: Pattern with or without ``+<>``/``-<>``.

    Returns:
        Pattern with ``+<>``/``-<>``.
    """
    src_filter = src_filter.strip()
    return src_filter if src_filter.startswith(("+<", "-<")) else f"+<{src_filter}>"


def extract_check_config(pio_config: list[Any], environment: str) -> CheckConfig:
    """Extract the checker options of one environment.

    Args:
        pio_config: Output of ``pio project config --json-output``.
        environment: Environment name.

    Returns:
        Checker configuration.

    Raises:
        ValueError: If the environment doesn't exist.
    """
    env = section_items(pio_config, f"env:{environment}")
    if env is None:
        raise ValueError(f"Environment '{environment}' not found in the PlatformIO configuration.")

    platformio = section_items(pio_config, "platformio") or {}
    packages_dir = platformio.get("packages_dir")

    config = CheckConfig()
    config.tools = as_list(env.get("check_tool")) or config.tools
    config.severities = [sev.lower() for sev in as_list(env.get("check_severity"))] or config.severities
    src_filters = as_list(env.get("check_src_filters")) or as_list(env.get("check_patterns"))
    if not src_filters:
        # Same default as PlatformIO: the source and include directory of the project.
        src_filters = [str(platformio.get("src_dir") or "src"), str(platformio.get("include_dir") or "include")]
    config.src_filters = [normalize_src_filter(item) for item in src_filters]
    config.clangtidy_flags = tool_flags(as_list(env.get("check_flags")), "clangtidy")
    config.skip_packages = as_bool(env.get("check_skip_packages", False))
    config.packages_dir = Path(packages_dir) if packages_dir else None

    return config


def tool_flags(check_flags: list[str], tool: str) -> list[str]:
    """Select the flags of one tool from the ``check_flags`` option.

    A flag entry without a tool prefix applies to every tool. Shell like quoting
    is resolved, so ``--header-filter=''`` becomes ``--header-filter=``.

    Args:
        check_flags: Entries of the ``check_flags`` option.
        tool: Tool name, e.g. ``clangtidy``.

    Returns:
        Flags of the tool.
    """
    result: list[str] = []
    for entry in check_flags:
        entry = entry.strip()
        if not entry:
            continue
        if ":" not in entry or entry.startswith("-"):
            result.extend(shlex.split(entry, posix=True))
        elif entry.startswith(f"{tool}:"):
            result.extend(shlex.split(entry.split(":", 1)[1], posix=True))
    return result


def clangtidy_options(flags: list[str]) -> list[str]:
    """Build the clang-tidy options from the configured flags.

    Args:
        flags: Configured clang-tidy flags.

    Returns:
        clang-tidy options.
    """
    options = ["--quiet"]
    has_checks = any(("--checks" in flag) or ("--config" in flag) for flag in flags)
    if not has_checks:
        options.append("--checks=*")
    options.extend(flags)
    return options


def language_of(path: Path) -> Optional[str]:
    """Determine the language of a file by its extension.

    Args:
        path: File path.

    Returns:
        ``c``, ``c++``, ``header`` or None for other files.
    """
    suffix = path.suffix.lower()
    if suffix in C_EXTENSIONS:
        return "c"
    if suffix in CPP_EXTENSIONS:
        return "c++"
    if suffix in HEADER_EXTENSIONS:
        return "header"
    return None


def checkable_files(item: Path) -> set[Path]:
    """Collect the checkable files of a path, recursively for directories.

    Args:
        item: File or directory.

    Returns:
        Absolute paths of source and header files.
    """
    result: set[Path] = set()
    if item.is_dir():
        for root, _, files in os.walk(item):
            for name in files:
                candidate = Path(root) / name
                if language_of(candidate) is not None:
                    result.add(candidate.resolve())
    elif language_of(item) is not None:
        result.add(item.resolve())
    return result


def match_src_files(project_dir: Path, src_filters: Any) -> list[Path]:
    """Collect source and header files by PlatformIO like source filters.

    Args:
        project_dir: Project root directory.
        src_filters: Source filters as list or single string.

    Returns:
        Sorted list of absolute file paths.
    """
    if isinstance(src_filters, (list, tuple)):
        src_filters = " ".join(normalize_src_filter(item) for item in src_filters)
    else:
        src_filters = normalize_src_filter(str(src_filters))

    matches: set[Path] = set()
    for action, pattern in SRC_FILTER_PATTERN.findall(src_filters):
        pattern = pattern.replace("\\", "/").rstrip("/")
        items: set[Path] = set()
        for item in glob.glob(os.path.join(glob.escape(str(project_dir)), pattern), recursive=True):
            items |= checkable_files(Path(item))

        if "+" == action:
            matches |= items
        else:
            matches -= items

    return sorted(matches)


def filter_includes(includes: list[str], packages_dir: Optional[Path], skip_packages: bool) -> list[str]:
    """Remove include paths below the PlatformIO packages directory on request.

    Args:
        includes: Include paths.
        packages_dir: PlatformIO packages directory.
        skip_packages: True to drop package includes.

    Returns:
        Filtered include paths.
    """
    if (not skip_packages) or (packages_dir is None):
        return list(includes)
    prefix = str(packages_dir).replace("\\", "/").lower()
    return [inc for inc in includes if not inc.replace("\\", "/").lower().startswith(prefix)]


def unique_includes(includes_map: dict[str, list[str]]) -> list[str]:
    """Flatten the include map of the build metadata, keeping the order.

    Args:
        includes_map: Include paths grouped by origin.

    Returns:
        Unique include paths.
    """
    result: list[str] = []
    for group in includes_map.values():
        for include in group:
            if include not in result:
                result.append(include)
    return result


def toolchain_probe_flags(flags: list[str]) -> list[str]:
    """Select the flags which influence the built-in defines of the toolchain.

    Args:
        flags: Compiler flags of the build.

    Returns:
        Machine, feature and standard flags.
    """
    return [flag for flag in flags if flag.startswith(("-m", "-f", "-std"))]


def standard_flag(flags: list[str]) -> Optional[str]:
    """Get the language standard flag.

    Args:
        flags: Compiler flags of the build.

    Returns:
        The ``-std=`` flag or None.
    """
    for flag in flags:
        if flag.startswith("-std="):
            return flag
    return None


def target_triple_of(cc_path: Optional[str]) -> Optional[str]:
    """Derive the clang target triple from the toolchain compiler name.

    Without a target, clang assumes the host (64 bit) and the 32 bit toolchain
    headers produce bogus errors. ``xtensa-esp32-elf-gcc.exe`` becomes
    ``xtensa-esp32-elf``, a plain ``gcc`` yields None (host target).

    Args:
        cc_path: Path to the toolchain C compiler.

    Returns:
        Target triple or None.
    """
    if not cc_path:
        return None
    name = Path(cc_path).name
    if name.lower().endswith(".exe"):
        name = name[:-4]
    for suffix in ("-gcc", "-cc", "-clang"):
        if name.endswith(suffix):
            triple = name[: -len(suffix)]
            return triple if "-" in triple else None
    return None


def parse_toolchain_defines(dump: str) -> list[str]:
    """Convert a ``-dM -E`` preprocessor dump into ``NAME=VALUE`` entries.

    Args:
        dump: Preprocessor output.

    Returns:
        Defines in ``-D`` compatible form.
    """
    defines: list[str] = []
    for line in dump.splitlines():
        tokens = line.strip().split(" ", 2)
        if (not tokens) or ("#define" != tokens[0]) or (len(tokens) < 2):
            continue
        if tokens[1].split("(", 1)[0] in IGNORED_TOOLCHAIN_DEFINES:
            continue
        if 2 < len(tokens):
            defines.append(f"{tokens[1]}={tokens[2]}")
        else:
            defines.append(tokens[1])
    return defines


def collect_toolchain_defines(cc_path: str, language: str, flags: list[str], verbose: bool) -> list[str]:
    """Ask the toolchain compiler for its built-in defines.

    The include paths are intentionally not passed: they don't influence the
    built-in defines and gcc would forward them to cc1 on the command line,
    which exceeds the Windows limit again.

    Args:
        cc_path: Path to the toolchain C compiler.
        language: ``c`` or ``c++``.
        flags: Compiler flags of the build for this language.
        verbose: True to print the compiler output on failure.

    Returns:
        Built-in defines.
    """
    cmd = [cc_path, "-x", language] + toolchain_probe_flags(flags) + ["-dM", "-E", "-"]
    result = subprocess.run(cmd, input="", capture_output=True, text=True, encoding="utf-8", errors="replace", check=False)
    if 0 != result.returncode:
        warn(f"Failed to extract toolchain defines for {language}.")
        if verbose:
            print(result.stdout)
            print(result.stderr)
    return parse_toolchain_defines(result.stdout)


def load_build_info(metadata: dict[str, Any], config: CheckConfig, target: Optional[str], verbose: bool) -> BuildInfo:
    """Derive the compile database inputs from the PlatformIO build metadata.

    Args:
        metadata: Build metadata of the environment (``pio project metadata``).
        config: Checker configuration.
        target: Target triple override, None to derive it from the toolchain, "" for the host.
        verbose: True for verbose output.

    Returns:
        Build information.
    """
    includes = unique_includes(metadata.get("includes", {}))
    cc_flags = as_list(metadata.get("cc_flags"))
    cxx_flags = as_list(metadata.get("cxx_flags"))
    cc_path = metadata.get("cc_path")

    build = BuildInfo()
    build.target = target_triple_of(cc_path) if target is None else (target or None)
    build.includes = filter_includes(includes, config.packages_dir, config.skip_packages)
    build.defines = as_list(metadata.get("defines"))
    build.c_std = standard_flag(cc_flags)
    build.cxx_std = standard_flag(cxx_flags)

    if cc_path and Path(cc_path).is_file():
        build.c_defines = collect_toolchain_defines(cc_path, "c", cc_flags, verbose)
        build.cxx_defines = collect_toolchain_defines(cc_path, "c++", cxx_flags, verbose)
    else:
        warn("Toolchain compiler not found, built-in toolchain defines are not available.")

    return build


def compile_arguments(path: Path, build: BuildInfo) -> list[str]:
    """Build the compiler arguments of one file for the compile database.

    Args:
        path: Source or header file.
        build: Build information.

    Returns:
        Compiler command as argument list.
    """
    language = language_of(path)
    if "c" == language:
        args = ["clang", "-x", "c"]
        std = build.c_std
        lang_defines = build.c_defines
    else:
        args = ["clang++", "-x", "c++"]
        std = build.cxx_std
        lang_defines = build.cxx_defines

    if build.target:
        args.append(f"--target={build.target}")
    if std:
        args.append(std)
    args.extend(f"-D{define}" for define in build.defines + lang_defines)
    args.extend(f"-I{include}" for include in build.includes)
    args.extend(["-c", str(path)])
    return args


def write_compile_database(db_dir: Path, project_dir: Path, files: list[Path], build: BuildInfo) -> Path:
    """Write a ``compile_commands.json`` with one entry per file.

    Args:
        db_dir: Directory the database is written to.
        project_dir: Project root directory, used as working directory of the commands.
        files: Files to check.
        build: Build information.

    Returns:
        Path of the written database.
    """
    entries = [
        {
            "directory": str(project_dir),
            "file": str(path),
            "arguments": compile_arguments(path, build),
        }
        for path in files
    ]
    db_dir.mkdir(parents=True, exist_ok=True)
    db_path = db_dir / "compile_commands.json"
    db_path.write_text(json.dumps(entries, indent=1), encoding="utf-8")
    return db_path


def parse_defect(line: str) -> Optional[Defect]:
    """Parse one clang-tidy output line.

    Args:
        line: Output line.

    Returns:
        Defect or None if the line is no diagnostic.
    """
    match = DEFECT_PATTERN.match(line.strip())
    if not match:
        return None

    category = match.group("category").strip()
    return Defect(
        file=match.group("file"),
        line=int(match.group("line")),
        column=int(match.group("column")),
        category=category,
        severity=CATEGORY_TO_SEVERITY.get(category, "low"),
        message=match.group("message").strip(),
        check=match.group("check"),
    )


def collect_defects(lines: list[str], severities: list[str], keep_compile_errors: bool = False) -> list[Defect]:
    """Parse the tool output into unique defects of the requested severities.

    Args:
        lines: clang-tidy output lines.
        severities: Severities to keep.
        keep_compile_errors: True to keep ``clang-diagnostic-error`` entries.

    Returns:
        Defects in order of appearance.
    """
    defects: list[Defect] = []
    seen: set[Defect] = set()
    for line in lines:
        defect = parse_defect(line)
        if defect is None:
            continue
        if (COMPILE_ERROR_CHECK == defect.check) and (not keep_compile_errors):
            continue
        if defect.severity not in severities:
            continue
        if defect in seen:
            continue
        seen.add(defect)
        defects.append(defect)
    return defects


def format_defect(defect: Defect, project_dir: Path) -> str:
    """Format a defect PlatformIO like.

    Args:
        defect: Defect.
        project_dir: Project root directory, used to shorten the path.

    Returns:
        Formatted line.
    """
    path = Path(defect.file)
    try:
        location = path.resolve().relative_to(project_dir.resolve()).as_posix()
    except ValueError:
        location = str(path)
    return f"{location}:{defect.line}:{defect.column}: [{defect.severity}:{defect.category}] {defect.message} [{defect.check}]"


def has_failing_defects(defects: list[Defect], fail_on: list[str]) -> bool:
    """Check whether any defect has a severity that fails the run.

    Args:
        defects: Defects.
        fail_on: Severities that fail the run.

    Returns:
        True if the run shall fail.
    """
    return any(defect.severity in fail_on for defect in defects)


def summary_counts(defects: list[Defect]) -> dict[str, int]:
    """Count the defects per severity.

    Args:
        defects: Defects.

    Returns:
        Count per severity.
    """
    counts = {severity: 0 for severity in SEVERITIES}
    for defect in defects:
        counts[defect.severity] += 1
    return counts


def find_clang_tidy(explicit: Optional[str], packages_dir: Optional[Path], pio: list[str], project_dir: Path) -> Optional[Path]:
    """Locate the clang-tidy executable.

    The PlatformIO package ``tool-clangtidy`` is preferred and installed on
    demand, afterwards the PATH is searched.

    Args:
        explicit: Path given on the command line.
        packages_dir: PlatformIO packages directory.
        pio: PlatformIO CLI command prefix.
        project_dir: Project root directory.

    Returns:
        Path to clang-tidy or None.
    """
    if explicit:
        return Path(explicit) if Path(explicit).is_file() else None

    exe = "clang-tidy.exe" if "win32" == sys.platform else "clang-tidy"
    if packages_dir is None:
        packages_dir = Path.home() / ".platformio" / "packages"

    candidate = packages_dir / "tool-clangtidy" / exe
    if not candidate.is_file():
        print("clang-tidy package not found, installing tool-clangtidy via PlatformIO ...")
        subprocess.run(pio + ["pkg", "install", "--global", "--tool", "tool-clangtidy"], cwd=project_dir, check=False)
    if candidate.is_file():
        return candidate

    found = shutil.which("clang-tidy")
    return Path(found) if found else None


def run_clang_tidy(clang_tidy: Path, db_dir: Path, options: list[str], path: Path, project_dir: Path) -> ToolRun:
    """Run clang-tidy for one file.

    Args:
        clang_tidy: clang-tidy executable.
        db_dir: Directory containing ``compile_commands.json``.
        options: clang-tidy options.
        path: File to check.
        project_dir: Working directory.

    Returns:
        Tool run result.
    """
    cmd = [str(clang_tidy)] + options + ["-p", str(db_dir), str(path)]
    result = subprocess.run(cmd, cwd=project_dir, capture_output=True, text=True, encoding="utf-8", errors="replace", check=False)
    lines = result.stdout.splitlines() + result.stderr.splitlines()
    return ToolRun(file=path, returncode=result.returncode, lines=lines)


def parse_arguments(argv: Optional[list[str]] = None) -> argparse.Namespace:
    """Parse the command line.

    Args:
        argv: Arguments without the program name.

    Returns:
        Parsed arguments.
    """
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("-e", "--environment", help="PlatformIO environment (default: first default_envs entry).")
    parser.add_argument("-d", "--project-dir", default=".", help="Project directory (default: current directory).")
    parser.add_argument("--fail-on-defect", action="append", choices=SEVERITIES, default=[],
                        help="Exit with 1 if a defect of this severity is found, may be repeated.")
    parser.add_argument("--severity", action="append", choices=SEVERITIES, default=None,
                        help="Override the check_severity option, may be repeated.")
    parser.add_argument("--src-filter", action="append", default=None,
                        help="Override the check_src_filters option, e.g. '+<src/main.cpp>', may be repeated.")
    parser.add_argument("--skip-packages", action="store_true", help="Drop include paths of PlatformIO packages.")
    parser.add_argument("--target", default=None,
                        help="clang target triple (default: derived from the toolchain compiler, e.g. xtensa-esp32-elf; "
                             "pass an empty string for the host target).")
    parser.add_argument("--clang-tidy", help="Path to the clang-tidy executable.")
    parser.add_argument("-j", "--jobs", type=int, default=max(1, os.cpu_count() or 1), help="Parallel clang-tidy processes.")
    parser.add_argument("-v", "--verbose", action="store_true", help="Show commands and the raw tool output.")
    parser.add_argument("--dry-run", action="store_true", help="Write the compile database, but don't run clang-tidy.")
    return parser.parse_args(argv)


# pylint: disable-next=too-many-locals,too-many-return-statements,too-many-branches,too-many-statements
def main(argv: Optional[list[str]] = None) -> int:
    """Run the checker.

    Args:
        argv: Arguments without the program name.

    Returns:
        Exit code.
    """
    args = parse_arguments(argv)
    project_dir = Path(args.project_dir).resolve()
    pio = find_pio()

    if not (project_dir / "platformio.ini").is_file():
        error(f"No platformio.ini found in {project_dir}")
        return EXIT_ERROR

    try:
        pio_config = run_json(pio + ["project", "config", "--json-output"], project_dir)
    except RuntimeError as exc:
        error(str(exc))
        return EXIT_ERROR

    environment = args.environment or default_environment(pio_config)
    if not environment:
        error("No environment given and no default_envs configured.")
        return EXIT_ERROR

    try:
        config = extract_check_config(pio_config, environment)
    except ValueError as exc:
        error(str(exc))
        return EXIT_ERROR

    if "clangtidy" not in config.tools:
        warn(f"Environment '{environment}' doesn't configure clangtidy as check_tool, running it anyway.")

    if args.severity:
        config.severities = args.severity
    if args.src_filter:
        config.src_filters = [normalize_src_filter(item) for item in args.src_filter]
    if args.skip_packages:
        config.skip_packages = True

    files = match_src_files(project_dir, config.src_filters)
    if not files:
        error("No source files matched the source filters.")
        return EXIT_ERROR

    clang_tidy = find_clang_tidy(args.clang_tidy, config.packages_dir, pio, project_dir)
    if clang_tidy is None:
        error("clang-tidy not found. Install it via 'pio pkg install --global --tool tool-clangtidy' "
              "or use --clang-tidy.")
        return EXIT_ERROR

    print(f"Environment: {environment}")
    print(f"Tool:        {clang_tidy}")
    print(f"Files:       {len(files)}")
    print(f"Severities:  {', '.join(config.severities)}")
    print("Collecting build metadata ...")

    try:
        metadata = run_json(pio + ["project", "metadata", "-e", environment, "--json-output"], project_dir)
    except RuntimeError as exc:
        error(str(exc))
        return EXIT_ERROR
    if isinstance(metadata, dict) and (environment in metadata):
        metadata = metadata[environment]

    build = load_build_info(metadata, config, args.target, args.verbose)
    db_dir = project_dir / ".pio" / "check" / environment / "clangtidy"
    db_path = write_compile_database(db_dir, project_dir, files, build)
    options = clangtidy_options(config.clangtidy_flags)

    if args.verbose:
        print(f"Compile database: {db_path}")
        print(f"Target: {build.target or 'host'}")
        print(f"clang-tidy options: {' '.join(options)}")

    if args.dry_run:
        return EXIT_OK

    print(f"Running clang-tidy with {args.jobs} job(s) ...")
    all_lines: list[str] = []
    tool_failed = False
    with ThreadPoolExecutor(max_workers=args.jobs) as executor:
        runs = executor.map(lambda path: run_clang_tidy(clang_tidy, db_dir, options, path, project_dir), files)
        for index, run in enumerate(runs, start=1):
            if args.verbose:
                print(f"[{index}/{len(files)}] {run.file}")
                for line in run.lines:
                    print(line)
            if CLANG_TIDY_MAX_OK_RETURNCODE < run.returncode:
                tool_failed = True
                error(f"clang-tidy failed with exit code {run.returncode} for {run.file}")
                if not args.verbose:
                    for line in run.lines:
                        print(line)
            all_lines.extend(run.lines)

    defects = collect_defects(all_lines, config.severities, keep_compile_errors=args.verbose)
    for defect in defects:
        print(format_defect(defect, project_dir))

    counts = summary_counts(defects)
    print()
    print(f"Defects: high={counts['high']} medium={counts['medium']} low={counts['low']}")

    if tool_failed:
        print("[FAILED] clang-tidy could not be executed for all files.")
        return EXIT_ERROR
    if has_failing_defects(defects, args.fail_on_defect):
        print("[FAILED]")
        return EXIT_DEFECTS

    print("[PASSED]")
    return EXIT_OK


################################################################################
# Main
################################################################################
if __name__ == "__main__":
    sys.exit(main())
