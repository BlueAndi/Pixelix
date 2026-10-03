#!/usr/bin/env python3
"""Collect and compare firmware size metrics for GitHub Actions."""

from __future__ import annotations

import argparse
import json
import os
import re
import subprocess
import sys
from pathlib import Path
from typing import Any


MEMORY_RE = re.compile(
    r"^\s*(RAM|Flash):.*?\(used\s+"
    r"([\d,]+)\s+bytes\s+from\s+([\d,]+)\s+bytes\)",
    re.MULTILINE,
)
IMAGE_NAMES = ("littlefs.bin", "spiffs.bin")


def parse_bytes(value: str) -> int:
    """Convert a comma-separated byte count to an integer."""
    return int(value.replace(",", ""))


def collect_metrics(
    environment: str,
    build_log: Path,
    build_dir: Path,
) -> dict[str, Any]:
    """Collect firmware and filesystem size metrics for one environment."""
    log_text = build_log.read_text(encoding="utf-8", errors="replace")
    memory = {
        name.lower(): {"used": parse_bytes(used), "total": parse_bytes(total)}
        for name, used, total in MEMORY_RE.findall(log_text)
    }
    if "ram" not in memory or "flash" not in memory:
        raise ValueError(
            f"PlatformIO RAM/Flash summary not found in {build_log}"
        )

    images = {
        name: (build_dir / name).stat().st_size
        for name in IMAGE_NAMES
        if (build_dir / name).is_file()
    }
    if not images:
        raise ValueError(f"No LittleFS or SPIFFS image found in {build_dir}")

    return {
        "environment": environment,
        "firmware_flash": memory["flash"],
        "ram": memory["ram"],
        "filesystem_images": images,
    }


def read_metrics(directory: Path) -> dict[str, dict[str, Any]]:
    """Load environment metrics from JSON files in a directory."""
    result: dict[str, dict[str, Any]] = {}
    if not directory.is_dir():
        return result
    for path in directory.glob("*.json"):
        data = json.loads(path.read_text(encoding="utf-8"))
        result[data["environment"]] = data
    return result


def format_bytes(value: int, signed: bool = False) -> str:
    """Format a byte count using a readable binary unit."""
    sign = ""
    if signed:
        sign = "+" if value > 0 else "-" if value < 0 else ""
    magnitude = abs(value)
    if magnitude >= 1024 * 1024:
        formatted = f"{magnitude / (1024 * 1024):.2f} MiB"
    elif magnitude >= 1024:
        formatted = f"{magnitude / 1024:.1f} KiB"
    else:
        formatted = f"{magnitude} B"
    return f"{sign}{formatted}"


def format_usage(metric: dict[str, int]) -> str:
    """Format a used/available size and its percentage."""
    used = metric["used"]
    total = metric["total"]
    percent = (used / total * 100.0) if total else 0.0
    return f"{format_bytes(used)} / {format_bytes(total)} ({percent:.1f}%)"


def format_delta(current: int, previous: int | None) -> str:
    """Format an absolute and relative change from a previous value."""
    if previous is None:
        return "n/a"
    delta = current - previous
    percent = (
        "n/a" if previous == 0 else f"{delta / previous * 100:+.1f}%"
    )
    return f"{format_bytes(delta, signed=True)} ({percent})"


def filesystem_total(metrics: dict[str, Any]) -> int:
    """Return the combined size of filesystem images."""
    return sum(metrics["filesystem_images"].values())


def format_filesystem(metrics: dict[str, Any]) -> str:
    """Format each generated filesystem image and its size."""
    return "<br>".join(
        f"{name}: {format_bytes(size)}"
        for name, size in sorted(metrics["filesystem_images"].items())
    )


def metric_value(metrics: dict[str, Any], metric_name: str) -> int:
    """Return the byte count for one metric."""
    if metric_name == "filesystem_images":
        return filesystem_total(metrics)
    return metrics[metric_name]["used"]


def metric_display(metrics: dict[str, Any], metric_name: str) -> str:
    """Format one metric for display in a report table."""
    if metric_name == "filesystem_images":
        return format_filesystem(metrics)
    return format_usage(metrics[metric_name])


def render_table(
    title: str,
    current: dict[str, dict[str, Any]],
    branch: dict[str, dict[str, Any]],
    target: dict[str, dict[str, Any]],
    metric_name: str,
) -> str:
    """Render one table with current values and both baseline deltas."""
    lines = [
        f"### {title}",
        "| Environment | Current | vs previous branch | vs target branch |",
        "| --- | ---: | ---: | ---: |",
    ]
    for environment, metrics in sorted(current.items()):
        current_value = metric_value(metrics, metric_name)
        branch_metrics = branch.get(environment)
        target_metrics = target.get(environment)
        branch_value = (
            metric_value(branch_metrics, metric_name)
            if branch_metrics
            else None
        )
        target_value = (
            metric_value(target_metrics, metric_name)
            if target_metrics
            else None
        )
        lines.append(
            f"| {environment} | {metric_display(metrics, metric_name)} | "
            f"{format_delta(current_value, branch_value)} | "
            f"{format_delta(current_value, target_value)} |"
        )
    return "\n".join(lines)


def write_report(args: argparse.Namespace) -> None:
    """Write firmware, filesystem, and RAM tables to the Actions summary."""
    current = read_metrics(args.current_dir)
    branch = read_metrics(args.branch_dir) if args.branch_dir else {}
    target = read_metrics(args.target_dir) if args.target_dir else {}
    if not current:
        raise ValueError(f"No current metrics found in {args.current_dir}")

    report = [
        "## Firmware size trend",
        f"Branch comparison: {args.branch_label}. "
        f"Target comparison: {args.target_label}.",
        "Positive deltas mean increased usage; filesystem image sizes are "
        "shown separately from firmware flash.",
        "",
        render_table(
            "Firmware Flash", current, branch, target, "firmware_flash"
        ),
        "",
        render_table(
            "Filesystem Images", current, branch, target, "filesystem_images"
        ),
        "",
        render_table("RAM", current, branch, target, "ram"),
        "",
    ]
    content = "\n".join(report)
    summary_path = os.environ.get("GITHUB_STEP_SUMMARY")
    if summary_path:
        with open(summary_path, "a", encoding="utf-8") as summary_file:
            summary_file.write(content)
    else:
        print(content)


def find_successful_run(
    workflow_file: str,
    branch_name: str | None,
    event_name: str,
    excluded_sha: str,
) -> str:
    """Find the latest successful matching workflow run ID."""
    command = [
        "gh",
        "run",
        "list",
        "--workflow",
        workflow_file,
        "--event",
        event_name,
        "--status",
        "success",
        "--limit",
        "50",
        "--json",
        "databaseId,headSha",
    ]
    if branch_name:
        command.extend(("--branch", branch_name))
    result = subprocess.run(
        command, check=True, capture_output=True, text=True
    )
    for run in json.loads(result.stdout):
        if run["headSha"] != excluded_sha:
            return str(run["databaseId"])
    return ""


def write_run_outputs(args: argparse.Namespace) -> None:
    """Resolve and write the branch and target baseline run IDs."""
    workflow_file = Path(args.workflow_file).name
    event_name = args.event
    head_branch = args.head_branch or args.ref_name
    branch_workflow = workflow_file
    branch_name: str | None = head_branch
    branch_event = "push"
    target_workflow = ""
    target_name = ""
    target_event = "push"

    if workflow_file == "featureBranches.yml":
        if event_name == "pull_request":
            target_name = args.base_branch
            target_workflow = (
                "main.yml"
                if target_name in {"main", "master"}
                else "featureBranches.yml"
            )
    elif workflow_file == "main.yml":
        if event_name == "pull_request":
            branch_workflow = "featureBranches.yml"
            branch_name = args.head_branch
            target_workflow = "main.yml"
            target_name = args.base_branch
        elif event_name == "release":
            branch_name = None
            branch_event = "release"
        else:
            branch_name = args.ref_name
    elif workflow_file == "release.yml":
        branch_event = "workflow_dispatch"
    else:
        raise ValueError(f"Unsupported workflow file: {workflow_file}")

    branch_run = find_successful_run(
        branch_workflow, branch_name, branch_event, args.current_sha
    )
    target_run = ""
    if target_workflow and target_name:
        target_run = find_successful_run(
            target_workflow, target_name, target_event, args.current_sha
        )

    with open(args.output, "a", encoding="utf-8") as output_file:
        output_file.write(f"branch_run_id={branch_run}\n")
        output_file.write(f"target_run_id={target_run}\n")


def build_parser() -> argparse.ArgumentParser:
    """Build the command-line argument parser."""
    parser = argparse.ArgumentParser(description=__doc__)
    subparsers = parser.add_subparsers(dest="command", required=True)

    collect_parser = subparsers.add_parser("collect")
    collect_parser.add_argument("--environment", required=True)
    collect_parser.add_argument("--build-log", type=Path, required=True)
    collect_parser.add_argument("--build-dir", type=Path, required=True)
    collect_parser.add_argument("--output-dir", type=Path, required=True)

    report_parser = subparsers.add_parser("report")
    report_parser.add_argument("--current-dir", type=Path, required=True)
    report_parser.add_argument("--branch-dir", type=Path)
    report_parser.add_argument("--target-dir", type=Path)
    report_parser.add_argument(
        "--branch-label", default="previous successful run"
    )
    report_parser.add_argument("--target-label", default="target branch")

    runs_parser = subparsers.add_parser("find-runs")
    runs_parser.add_argument("--workflow-file", required=True)
    runs_parser.add_argument("--event", required=True)
    runs_parser.add_argument("--ref-name", default="")
    runs_parser.add_argument("--head-branch", default="")
    runs_parser.add_argument("--base-branch", default="")
    runs_parser.add_argument("--current-sha", required=True)
    runs_parser.add_argument("--output", required=True)
    return parser


def main() -> int:
    """Run the requested metrics collection or reporting command."""
    args = build_parser().parse_args()
    try:
        if args.command == "collect":
            metrics = collect_metrics(
                args.environment,
                args.build_log,
                args.build_dir,
            )
            args.output_dir.mkdir(parents=True, exist_ok=True)
            output_path = args.output_dir / f"{args.environment}.json"
            output_path.write_text(
                json.dumps(metrics, indent=2) + "\n", encoding="utf-8"
            )
        elif args.command == "report":
            write_report(args)
        else:
            write_run_outputs(args)
    except (
        OSError,
        ValueError,
        subprocess.CalledProcessError,
        json.JSONDecodeError,
    ) as error:
        print(f"Size report error: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
