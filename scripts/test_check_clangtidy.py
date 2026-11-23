"""Tests of the Windows friendly clang-tidy runner."""

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
import json
from pathlib import Path

import pytest

import check_clangtidy as cct

################################################################################
# Helpers
################################################################################


def create_tree(root: Path) -> None:
    """Create a small project tree with sources, headers and excluded folders."""
    for relative in (
        "src/main.cpp",
        "src/Hal/Board.c",
        "include/Config.h",
        "lib/FooPlugin/src/FooPlugin.cpp",
        "lib/FooPlugin/src/FooPlugin.h",
        "lib/libwebp/src/dec.c",
        "lib/HalLedMatrixNative/src/imgui/imgui.cpp",
        "doc/notes.txt",
        "src/readme.md",
    ):
        path = root / relative
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text("", encoding="utf-8")


################################################################################
# Tests: configuration
################################################################################


def test_extract_check_config_from_pio_json() -> None:
    """The checker options of one environment are taken from the resolved PlatformIO config."""
    pio_config = [
        ["platformio", [["default_envs", ["envA"]], ["packages_dir", "C:/pkgs"]]],
        ["env:envA", [
            ["check_tool", ["clangtidy"]],
            ["check_severity", ["high", "medium"]],
            ["check_src_filters", ["+<src/>", "-<lib/libwebp/>"]],
            ["check_flags", ["clangtidy: --checks=-*,performance-*"]],
            ["check_skip_packages", True],
        ]],
    ]
    config = cct.extract_check_config(pio_config, "envA")
    assert ["clangtidy"] == config.tools
    assert ["high", "medium"] == config.severities
    assert ["+<src/>", "-<lib/libwebp/>"] == config.src_filters
    assert ["--checks=-*,performance-*"] == config.clangtidy_flags
    assert config.skip_packages is True
    assert Path("C:/pkgs") == config.packages_dir


def test_extract_check_config_falls_back_to_check_patterns() -> None:
    """The deprecated check_patterns option is used when check_src_filters is missing."""
    pio_config = [
        ["env:envA", [["check_patterns", ["src", "lib"]]]],
    ]
    config = cct.extract_check_config(pio_config, "envA")
    assert ["+<src>", "+<lib>"] == config.src_filters
    assert ["clangtidy"] == config.tools
    assert ["low", "medium", "high"] == config.severities
    assert config.packages_dir is None


def test_extract_check_config_default_src_filters() -> None:
    """Without filters the project source and include directories are checked like PlatformIO does."""
    config = cct.extract_check_config([["env:envA", []]], "envA")
    assert ["+<src>", "+<include>"] == config.src_filters
    pio_config = [["platformio", [["src_dir", "code"], ["include_dir", "inc"]]], ["env:envA", []]]
    assert ["+<code>", "+<inc>"] == cct.extract_check_config(pio_config, "envA").src_filters


def test_extract_check_config_unknown_environment() -> None:
    """An unknown environment is rejected."""
    with pytest.raises(ValueError):
        cct.extract_check_config([["env:envA", []]], "envB")


def test_default_environment() -> None:
    """The first default environment of the project is used."""
    assert "envA" == cct.default_environment([["platformio", [["default_envs", ["envA", "envB"]]]]])
    assert cct.default_environment([["platformio", []]]) is None


@pytest.mark.parametrize("flags,expected", [
    (["clangtidy: --checks=-*,a-* --warnings-as-errors=a-*"], ["--checks=-*,a-*", "--warnings-as-errors=a-*"]),
    (["cppcheck: --std=c++17", "clangtidy: --header-filter=''"], ["--header-filter="]),
    (["--quiet"], ["--quiet"]),
    (["-extra-arg=-Wall"], ["-extra-arg=-Wall"]),
    (["cppcheck: --inline-suppr"], []),
    ([], []),
])
def test_tool_flags(flags: list[str], expected: list[str]) -> None:
    """Only the clang-tidy specific and the tool independent flags are selected."""
    assert expected == cct.tool_flags(flags, "clangtidy")


def test_checks_default_added_only_without_explicit_checks() -> None:
    """Without --checks or --config all checks are enabled like PlatformIO does."""
    assert ["--quiet", "--checks=*"] == cct.clangtidy_options([])
    assert ["--quiet", "--checks=-*,a"] == cct.clangtidy_options(["--checks=-*,a"])
    assert ["--quiet", "--config-file=x"] == cct.clangtidy_options(["--config-file=x"])


################################################################################
# Tests: source selection
################################################################################


def test_match_src_files(tmp_path: Path) -> None:
    """Sources and headers are collected by the PlatformIO like +<>/-<> filters."""
    create_tree(tmp_path)
    filters = ["+<include/>", "+<src/>", "+<lib/>", "-<lib/HalLedMatrixNative/src/imgui/>", "-<lib/libwebp/>"]
    files = cct.match_src_files(tmp_path, filters)
    expected = sorted([
        tmp_path / "include/Config.h",
        tmp_path / "src/main.cpp",
        tmp_path / "src/Hal/Board.c",
        tmp_path / "lib/FooPlugin/src/FooPlugin.cpp",
        tmp_path / "lib/FooPlugin/src/FooPlugin.h",
    ])
    assert expected == files


def test_match_src_files_with_glob_and_plain_pattern(tmp_path: Path) -> None:
    """Glob patterns are supported and a pattern without +<> is treated as include."""
    create_tree(tmp_path)
    assert [tmp_path / "src/main.cpp"] == cct.match_src_files(tmp_path, ["+<src/*.cpp>"])
    assert [tmp_path / "include/Config.h"] == cct.match_src_files(tmp_path, "include")


@pytest.mark.parametrize("name,language", [
    ("a.c", "c"), ("a.cpp", "c++"), ("a.cc", "c++"), ("a.cxx", "c++"), ("a.ino", "c++"),
    ("a.h", "header"), ("a.hpp", "header"), ("a.hh", "header"), ("a.hxx", "header"),
])
def test_language_of(name: str, language: str) -> None:
    """The language is derived from the file extension."""
    assert language == cct.language_of(Path(name))


################################################################################
# Tests: compile database
################################################################################


def test_write_compile_database(tmp_path: Path) -> None:
    """One compile command per file with language specific defines and standard."""
    files = [tmp_path / "src/a.c", tmp_path / "src/b.cpp", tmp_path / "include/c.h"]
    build = cct.BuildInfo(
        target="xtensa-esp32-elf",
        includes=[str(tmp_path / "include"), "C:/pkgs/framework/cores"],
        defines=["COMMON=1", 'STR="text with space"'],
        c_defines=["__C_ONLY__"],
        cxx_defines=["__cplusplus=201703L"],
        c_std="-std=gnu99",
        cxx_std="-std=gnu++17",
    )
    db_dir = tmp_path / ".pio/check"
    db_path = cct.write_compile_database(db_dir, tmp_path, files, build)
    assert db_dir / "compile_commands.json" == db_path
    entries = json.loads(db_path.read_text(encoding="utf-8"))
    assert 3 == len(entries)
    by_file = {Path(entry["file"]): entry for entry in entries}

    c_entry = by_file[tmp_path / "src/a.c"]
    assert str(tmp_path) == c_entry["directory"]
    args = c_entry["arguments"]
    assert "clang" == args[0]
    assert ["-x", "c", "--target=xtensa-esp32-elf", "-std=gnu99"] == args[1:5]
    assert "-DCOMMON=1" in args
    assert '-DSTR="text with space"' in args
    assert "-D__C_ONLY__" in args
    assert "-D__cplusplus=201703L" not in args
    assert "-IC:/pkgs/framework/cores" in args
    assert ["-c", str(tmp_path / "src/a.c")] == args[-2:]

    cpp_args = by_file[tmp_path / "src/b.cpp"]["arguments"]
    assert "clang++" == cpp_args[0]
    assert ["-x", "c++", "--target=xtensa-esp32-elf", "-std=gnu++17"] == cpp_args[1:5]
    assert "-D__cplusplus=201703L" in cpp_args
    assert "-D__C_ONLY__" not in cpp_args

    header_args = by_file[tmp_path / "include/c.h"]["arguments"]
    assert ["clang++", "-x", "c++", "--target=xtensa-esp32-elf", "-std=gnu++17"] == header_args[0:5]


def test_compile_arguments_without_target(tmp_path: Path) -> None:
    """Without a target triple clang uses the host target."""
    args = cct.compile_arguments(tmp_path / "a.cpp", cct.BuildInfo(cxx_std="-std=gnu++17"))
    assert ["clang++", "-x", "c++", "-std=gnu++17", "-c", str(tmp_path / "a.cpp")] == args


def test_filter_package_includes() -> None:
    """Includes below the packages directory are dropped on request."""
    includes = ["C:/pkgs/framework/cores", "C:/project/include", "c:/PKGS/toolchain/include"]
    assert ["C:/project/include"] == cct.filter_includes(includes, Path("C:/pkgs"), True)
    assert includes == cct.filter_includes(includes, Path("C:/pkgs"), False)
    assert includes == cct.filter_includes(includes, None, True)


def test_parse_toolchain_defines() -> None:
    """The preprocessor dump of the toolchain is converted into -D compatible defines."""
    dump = "#define __XTENSA__ 1\n#define __GNUC__ 8\n#define EMPTY\n#define __VERSION__ \"8.4.0\"\ngarbage\n\n"
    assert ["__XTENSA__=1", "__GNUC__=8", "EMPTY", '__VERSION__="8.4.0"'] == cct.parse_toolchain_defines(dump)


def test_parse_toolchain_defines_drops_gcc_has_include() -> None:
    """gcc's __has_include macros would shadow clang's built-in ones."""
    dump = "#define __has_include(STR) __has_include__(STR)\n#define __has_include_next(STR) __has_include_next__(STR)\n#define A 1\n"
    assert ["A=1"] == cct.parse_toolchain_defines(dump)


@pytest.mark.parametrize("cc_path,triple", [
    (r"C:\pkgs\toolchain-xtensa-esp32\bin\xtensa-esp32-elf-gcc.exe", "xtensa-esp32-elf"),
    ("/home/u/.platformio/packages/toolchain-xtensa-esp32s3/bin/xtensa-esp32s3-elf-gcc", "xtensa-esp32s3-elf"),
    ("/x/riscv32-esp-elf-gcc", "riscv32-esp-elf"),
    (r"C:\mingw\bin\gcc.exe", None),
    ("/usr/bin/cc", None),
    ("/usr/bin/clang", None),
    (None, None),
])
def test_target_triple_of(cc_path: str, triple: str) -> None:
    """The clang target triple is derived from a cross compiler name only."""
    assert triple == cct.target_triple_of(cc_path)


def test_toolchain_probe_flags() -> None:
    """Only machine, feature and standard flags are passed to the toolchain probe."""
    flags = ["-std=gnu++17", "-Wno-frame-address", "-fexceptions", "-Os", "-mlongcalls", "-MMD", "-ggdb"]
    assert ["-std=gnu++17", "-fexceptions", "-mlongcalls"] == cct.toolchain_probe_flags(flags)
    assert "-std=gnu++17" == cct.standard_flag(flags)
    assert cct.standard_flag(["-Os"]) is None


################################################################################
# Tests: defects
################################################################################


def test_parse_defect() -> None:
    """A clang-tidy diagnostic line is parsed into a defect."""
    line = r"C:\p\src\Main.cpp:12:5: warning: do not use 'else' after 'return' [readability-else-after-return]"
    defect = cct.parse_defect(line)
    assert defect is not None
    assert r"C:\p\src\Main.cpp" == defect.file
    assert 12 == defect.line
    assert 5 == defect.column
    assert "warning" == defect.category
    assert "medium" == defect.severity
    assert "do not use 'else' after 'return'" == defect.message
    assert "readability-else-after-return" == defect.check


@pytest.mark.parametrize("category,severity", [("error", "high"), ("warning", "medium"), ("note", "low")])
def test_defect_severity(category: str, severity: str) -> None:
    """The severity mapping is the one of PlatformIO."""
    line = f"/a/b.cpp:1:1: {category}: text [check-id]"
    defect = cct.parse_defect(line)
    assert defect is not None
    assert severity == defect.severity


@pytest.mark.parametrize("line", [
    "1 warning generated.",
    "Error while processing C:\\p\\src\\Main.cpp.",
    "C:\\p\\src\\Main.cpp:12:5: warning: message without check id",
    "",
])
def test_parse_defect_ignores_other_lines(line: str) -> None:
    """Non diagnostic lines are ignored."""
    assert cct.parse_defect(line) is None


def test_collect_defects_filters_and_deduplicates() -> None:
    """Severity filtering and duplicate removal of the tool output."""
    output = "\n".join([
        "/a.cpp:1:1: error: e [c1]",
        "/a.cpp:2:1: warning: w [c2]",
        "/a.cpp:2:1: warning: w [c2]",
        "/a.cpp:3:1: note: n [c3]",
        "/a.cpp:4:1: error: compile [clang-diagnostic-error]",
    ])
    defects = cct.collect_defects(output.splitlines(), ["high", "medium"])
    assert [("high", 1), ("medium", 2)] == [(d.severity, d.line) for d in defects]


def test_collect_defects_keeps_compile_errors_in_verbose_mode() -> None:
    """Compilation errors are reported only when requested like in PlatformIO."""
    lines = ["/a.cpp:4:1: error: compile [clang-diagnostic-error]"]
    assert not cct.collect_defects(lines, ["high"])
    assert 1 == len(cct.collect_defects(lines, ["high"], keep_compile_errors=True))


def test_defect_format() -> None:
    """Defects are printed PlatformIO like, relative to the project directory."""
    defect = cct.Defect(str(Path("C:/p/src/Main.cpp")), 12, 5, "warning", "medium", "msg", "chk")
    assert "src/Main.cpp:12:5: [medium:warning] msg [chk]" == cct.format_defect(defect, Path("C:/p"))
    other = cct.Defect(str(Path("D:/x/y.cpp")), 1, 2, "error", "high", "m", "c")
    assert str(Path("D:/x/y.cpp")) + ":1:2: [high:error] m [c]" == cct.format_defect(other, Path("C:/p"))


def test_has_failing_defects() -> None:
    """The exit code depends on the requested fail-on-defect severities."""
    defects = [cct.Defect("f", 1, 1, "warning", "medium", "m", "c")]
    assert cct.has_failing_defects(defects, ["medium", "high"]) is True
    assert cct.has_failing_defects(defects, ["high"]) is False
    assert cct.has_failing_defects(defects, []) is False


def test_summary_counts() -> None:
    """The summary counts the defects per severity."""
    defects = [
        cct.Defect("f", 1, 1, "error", "high", "m", "c"),
        cct.Defect("f", 2, 1, "warning", "medium", "m", "c"),
        cct.Defect("f", 3, 1, "warning", "medium", "m", "c"),
    ]
    assert {"high": 1, "medium": 2, "low": 0} == cct.summary_counts(defects)
