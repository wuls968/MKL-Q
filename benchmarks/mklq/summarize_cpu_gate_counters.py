#!/usr/bin/env python3
# ============================================================================ #
# Copyright (c) 2026 Linsen Wu.                                                #
#                                                                              #
# This source code and the accompanying materials are made available under     #
# the terms of the Apache License 2.0 which accompanies this distribution.     #
# ============================================================================ #
"""Render bounded MKL-Q CPU gate fast-path counter evidence."""

import argparse
import json
import sys
from collections import Counter, defaultdict
from pathlib import Path
from typing import Any

SCHEMA_VERSION = "mklq-cpu-gate-counter-summary-v1"
COUNTER_REPORT_SCHEMA_VERSION = "mklq-cpu-gate-counter-probe-v1"
DEFAULT_REPORTS_DIR = Path(__file__).resolve().parent / "reports"
DEFAULT_PATTERN = "*.cpu-gate-counter.json"

CATEGORY_DESCRIPTIONS = {
    "single_qubit_fast_path":
        "Uncontrolled built-in single-qubit gate fast-path counter tests",
    "controlled_single_qubit_fast_path":
        "Controlled built-in single-qubit gate fast-path counter tests",
    "single_control_rz_phase":
        "Single-control Rz direct phase fast-path counter tests",
    "composite_fast_path":
        "Composite circuit fast-path selection counter tests",
    "multi_control_boundary":
        "Multi-control generic specialized-path boundary tests",
    "phase_fast_path":
        "Phase-sign gate fast-path counter tests",
    "two_qubit_fast_path":
        "Two-qubit gate fast-path counter tests",
    "three_qubit_fast_path":
        "Three-qubit row-sparse fast-path counter tests",
    "other":
        "Unclassified CPU gate counter tests",
}

CATEGORY_RULES = (
    ("single_control_rz_phase", ("SingleControlRz",)),
    ("composite_fast_path", ("HardwareEfficientAnsatzComposite",)),
    ("multi_control_boundary", ("MultiControl",)),
    ("controlled_single_qubit_fast_path", (
        "CnotFastPath",
        "ControlledBuiltInSingleQubit",
        "SingleControlBuiltIn",
        "SingleControlBuiltInSingleQubit",
    )),
    ("phase_fast_path", ("CzFastPath",)),
    ("two_qubit_fast_path", ("SwapFastPath",)),
    ("three_qubit_fast_path", ("RowSparseThreeQubit",)),
    ("single_qubit_fast_path", (
        "XFastPath",
        "BuiltInSingleQubit",
    )),
)


def markdown_escape(value: object) -> str:
    return str(value).replace("|", "\\|").replace("\n", " ")


def load_counter_report(path: Path) -> dict[str, Any]:
    report = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(report, dict):
        raise ValueError(f"{path} is not a JSON object")
    schema_version = report.get("schema_version")
    if schema_version != COUNTER_REPORT_SCHEMA_VERSION:
        raise ValueError(
            f"{path} has schema_version={schema_version!r}, expected "
            f"{COUNTER_REPORT_SCHEMA_VERSION!r}")
    return report


def discover_counter_reports(reports: list[Path],
                             pattern: str = DEFAULT_PATTERN) -> list[Path]:
    paths: list[Path] = []
    for report_path in reports:
        if report_path.is_dir():
            paths.extend(sorted(report_path.glob(pattern)))
            continue
        paths.append(report_path)

    unique: dict[str, Path] = {}
    for path in paths:
        unique[path.resolve().as_posix()] = path
    return [unique[key] for key in sorted(unique)]


def test_suffix(test_name: str) -> str:
    return test_name.rsplit(".", 1)[-1]


def categorize_test(test_name: str) -> str:
    suffix = test_suffix(test_name)
    for category, tokens in CATEGORY_RULES:
        if any(token in suffix for token in tokens):
            return category
    return "other"


def boundary_summary(reports: list[dict[str, Any]]) -> dict[str, Any]:
    if not reports:
        return {
            "runtime_counter_evidence": False,
            "gate_fast_path_counter_evidence": False,
            "single_control_rz_phase_counter_evidence": False,
            "release_signoff": False,
            "performance_benchmark": False,
            "cross_machine_performance_proof": False,
            "raw_logs_truncated": False,
        }

    boundaries = [
        report.get("boundary", {})
        for report in reports
        if isinstance(report.get("boundary"), dict)
    ]
    if len(boundaries) != len(reports):
        return {
            "runtime_counter_evidence": False,
            "gate_fast_path_counter_evidence": False,
            "single_control_rz_phase_counter_evidence": False,
            "release_signoff": False,
            "performance_benchmark": False,
            "cross_machine_performance_proof": False,
            "raw_logs_truncated": False,
        }
    return {
        "runtime_counter_evidence":
            all(
                bool(boundary.get("runtime_counter_evidence"))
                for boundary in boundaries),
        "gate_fast_path_counter_evidence":
            all(
                bool(boundary.get("gate_fast_path_counter_evidence"))
                for boundary in boundaries),
        "single_control_rz_phase_counter_evidence":
            all(
                bool(boundary.get("single_control_rz_phase_counter_evidence"))
                for boundary in boundaries),
        "release_signoff":
            any(
                bool(boundary.get("release_signoff")) for boundary in boundaries
            ),
        "performance_benchmark":
            any(
                bool(boundary.get("performance_benchmark"))
                for boundary in boundaries),
        "cross_machine_performance_proof":
            any(
                bool(boundary.get("cross_machine_performance_proof"))
                for boundary in boundaries),
        "raw_logs_truncated":
            all(
                bool(boundary.get("raw_logs_truncated", True))
                for boundary in boundaries),
    }


def build_report_digest(path: Path, report: dict[str, Any]) -> dict[str, Any]:
    summary = report.get("summary", {})
    if not isinstance(summary, dict):
        summary = {}
    boundary = report.get("boundary", {})
    if not isinstance(boundary, dict):
        boundary = {}

    return {
        "path":
            path.as_posix(),
        "created_at_utc":
            report.get("created_at_utc", "-"),
        "status":
            summary.get("status", "unknown"),
        "expected":
            int(summary.get("expected", 0)),
        "selected":
            int(summary.get("selected", 0)),
        "missing":
            int(summary.get("missing", 0)),
        "passed":
            int(summary.get("passed", 0)),
        "failed":
            int(summary.get("failed", 0)),
        "runtime_counter_evidence":
            bool(boundary.get("runtime_counter_evidence")),
        "gate_fast_path_counter_evidence":
            bool(boundary.get("gate_fast_path_counter_evidence")),
        "single_control_rz_phase_counter_evidence":
            bool(boundary.get("single_control_rz_phase_counter_evidence")),
        "release_signoff":
            bool(boundary.get("release_signoff")),
        "performance_benchmark":
            bool(boundary.get("performance_benchmark")),
        "cross_machine_performance_proof":
            bool(boundary.get("cross_machine_performance_proof")),
    }


def build_summary(paths: list[Path]) -> dict[str, Any]:
    loaded = [(path, load_counter_report(path)) for path in sorted(paths)]
    reports = [report for _, report in loaded]

    totals = Counter()
    category_counts: dict[str, Counter[str]] = defaultdict(Counter)
    category_tests: dict[str, list[str]] = defaultdict(list)

    for _, report in loaded:
        report_summary = report.get("summary", {})
        if isinstance(report_summary, dict):
            for key in ("expected", "selected", "missing", "passed", "failed"):
                totals[key] += int(report_summary.get(key, 0))
        for test in report.get("tests", []):
            if not isinstance(test, dict):
                continue
            name = str(test.get("name", "<unknown>"))
            status = str(test.get("status", "unknown"))
            category = categorize_test(name)
            category_counts[category][status] += 1
            category_tests[category].append(name)

    categories = []
    for category in sorted(CATEGORY_DESCRIPTIONS):
        counts = category_counts.get(category, Counter())
        if not counts:
            continue
        tests = sorted(set(category_tests.get(category, [])))
        categories.append({
            "category":
                category,
            "description":
                CATEGORY_DESCRIPTIONS[category],
            "passed":
                counts.get("passed", 0),
            "failed":
                counts.get("failed", 0),
            "unknown":
                sum(value for key, value in counts.items()
                    if key not in {"passed", "failed"}),
            "tests":
                tests,
        })

    boundary = boundary_summary(reports)
    boundary_ok = (bool(boundary["runtime_counter_evidence"]) and
                   bool(boundary["gate_fast_path_counter_evidence"]) and bool(
                       boundary["single_control_rz_phase_counter_evidence"]) and
                   not bool(boundary["release_signoff"]) and
                   not bool(boundary["performance_benchmark"]) and
                   not bool(boundary["cross_machine_performance_proof"]) and
                   bool(boundary["raw_logs_truncated"]))
    status = ("passed" if loaded and totals["failed"] == 0 and
              totals["missing"] == 0 and boundary_ok else "failed")

    return {
        "schema_version": SCHEMA_VERSION,
        "source_schema_version": COUNTER_REPORT_SCHEMA_VERSION,
        "summary": {
            "status": status,
            "report_count": len(loaded),
            "expected": totals["expected"],
            "selected": totals["selected"],
            "missing": totals["missing"],
            "passed": totals["passed"],
            "failed": totals["failed"],
        },
        "boundary": boundary,
        "reports": [
            build_report_digest(path, report) for path, report in loaded
        ],
        "categories": categories,
    }


def render_markdown(summary: dict[str, Any]) -> str:
    totals = summary.get("summary", {})
    boundary = summary.get("boundary", {})
    lines = [
        "# MKL-Q CPU Gate Counter Summary",
        "",
        "This file is generated from bounded `.cpu-gate-counter.json` "
        "reports under `benchmarks/mklq/reports/`.",
        "",
        "Caveat: this is gate fast-path counter evidence from selected "
        "build-tree ctest cases. It is not release sign-off, not a benchmark "
        "result, and not cross-machine performance proof.",
        "",
        "## Aggregate Status",
        "",
        "| Field | Value |",
        "| --- | --- |",
    ]
    for key in ("status", "report_count", "expected", "selected", "missing",
                "passed", "failed"):
        lines.append(f"| `{key}` | {markdown_escape(totals.get(key, '-'))} |")
    for key in ("runtime_counter_evidence", "gate_fast_path_counter_evidence",
                "single_control_rz_phase_counter_evidence", "release_signoff",
                "performance_benchmark", "cross_machine_performance_proof",
                "raw_logs_truncated"):
        lines.append(f"| `{key}` | {markdown_escape(boundary.get(key, '-'))} |")
    lines.extend([
        "",
        "Aggregate counts are summed across tracked reports. Repeated daily "
        "probes intentionally count the same selected counter tests once per "
        "report.",
        "",
        "## Categories",
        "",
        "| Category | Passed | Failed | Unknown | Description |",
        "| --- | ---: | ---: | ---: | --- |",
    ])
    for category in summary.get("categories", []):
        lines.append(f"| {markdown_escape(category['category'])} | "
                     f"{category['passed']} | {category['failed']} | "
                     f"{category['unknown']} | "
                     f"{markdown_escape(category['description'])} |")
    lines.extend([
        "",
        "## Selected Tests",
        "",
        "| Category | Test |",
        "| --- | --- |",
    ])
    for category in summary.get("categories", []):
        for test in category.get("tests", []):
            lines.append(f"| {markdown_escape(category['category'])} | "
                         f"`{markdown_escape(test)}` |")
    lines.extend([
        "",
        "## Reports",
        "",
        "| Report | Created | Status | Expected | Selected | Missing | Passed | Failed |",
        "| --- | --- | --- | ---: | ---: | ---: | ---: | ---: |",
    ])
    for report in summary.get("reports", []):
        lines.append(f"| {markdown_escape(report['path'])} | "
                     f"{markdown_escape(report['created_at_utc'])} | "
                     f"{markdown_escape(report['status'])} | "
                     f"{report['expected']} | {report['selected']} | "
                     f"{report['missing']} | {report['passed']} | "
                     f"{report['failed']} |")
    lines.extend([
        "",
        "## Regenerate",
        "",
        "```bash",
        "python3 benchmarks/mklq/summarize_cpu_gate_counters.py \\",
        "  --reports benchmarks/mklq/reports \\",
        "  --output docs/mklq/cpu-gate-counters.md",
        "```",
        "",
    ])
    return "\n".join(lines)


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=(
        "Summarize bounded MKL-Q CPU gate fast-path counter reports."))
    parser.add_argument("--reports",
                        type=Path,
                        nargs="+",
                        default=[DEFAULT_REPORTS_DIR],
                        help="Report files or directories to summarize.")
    parser.add_argument("--pattern",
                        default=DEFAULT_PATTERN,
                        help="Glob used when a report input is a directory.")
    parser.add_argument("--format",
                        choices=("json", "markdown"),
                        default="markdown",
                        help="Output format.")
    parser.add_argument("--output", type=Path, help="Optional output path.")
    return parser.parse_args(argv)


def main(argv: list[str]) -> int:
    args = parse_args(argv)
    paths = discover_counter_reports(args.reports, args.pattern)
    summary = build_summary(paths)
    if args.format == "json":
        payload = json.dumps(summary, indent=2, sort_keys=True) + "\n"
    else:
        payload = render_markdown(summary)
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(payload, encoding="utf-8")
    else:
        sys.stdout.write(payload)
    return 0 if summary["summary"]["status"] == "passed" else 1


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
