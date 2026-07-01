#!/usr/bin/env python3
# ============================================================================ #
# Copyright (c) 2026 Linsen Wu.                                                #
#                                                                              #
# This source code and the accompanying materials are made available under     #
# the terms of the Apache License 2.0 which accompanies this distribution.     #
# ============================================================================ #
"""Check that tracked CPU gate counter docs match bounded reports."""

import argparse
import difflib
import importlib.util
import json
import sys
from pathlib import Path
from typing import Any

DEFAULT_REPORTS_DIR = Path("benchmarks/mklq/reports")
DEFAULT_DOC = Path("docs/mklq/cpu-gate-counters.md")
DEFAULT_PATTERN = "*.cpu-gate-counter.json"
REQUIRED_AGGREGATE_NOTE = (
    "Aggregate counts are summed across tracked reports. Repeated daily "
    "probes intentionally count the same selected counter tests once per "
    "report.")


def load_summary_module():
    script = Path(__file__).with_name("summarize_cpu_gate_counters.py")
    spec = importlib.util.spec_from_file_location("summarize_cpu_gate_counters",
                                                  script)
    module = importlib.util.module_from_spec(spec)
    if spec.loader is None:
        raise RuntimeError(f"failed to load {script}")
    spec.loader.exec_module(module)
    return module


def _as_posix(path: Path) -> str:
    return path.as_posix()


def _failed(message: str, details: dict[str, Any]) -> dict[str, Any]:
    return {"status": "failed", "message": message, "details": details}


def check_docs(report_inputs: list[Path] | None = None,
               doc_path: Path = DEFAULT_DOC,
               pattern: str = DEFAULT_PATTERN) -> dict[str, Any]:
    summary_module = load_summary_module()
    report_inputs = report_inputs or [DEFAULT_REPORTS_DIR]
    paths = summary_module.discover_counter_reports(report_inputs, pattern)
    summary = summary_module.build_summary(paths)
    expected = summary_module.render_markdown(summary)
    summary_status = summary.get("summary", {}).get("status", "unknown")
    details = {
        "expected": _as_posix(doc_path),
        "reports": [_as_posix(path) for path in paths],
        "report_count": len(paths),
        "summary_status": summary_status,
    }
    if not paths:
        return _failed("no CPU gate counter reports found", details)
    if not doc_path.is_file():
        return _failed("CPU gate counter docs are missing", details)
    if summary_status != "passed":
        return _failed("CPU gate counter summary is not passing", details)
    if REQUIRED_AGGREGATE_NOTE not in expected:
        details["required_note"] = REQUIRED_AGGREGATE_NOTE
        return _failed(
            "CPU gate counter docs generator is missing the aggregate-count note",
            details)
    actual = doc_path.read_text(encoding="utf-8")
    if actual != expected:
        diff = difflib.unified_diff(actual.splitlines(),
                                    expected.splitlines(),
                                    fromfile=_as_posix(doc_path),
                                    tofile="regenerated-cpu-gate-counters.md",
                                    lineterm="")
        details["diff"] = "\n".join(list(diff)[:160])
        return _failed(
            "CPU gate counter docs output differs from regenerated output",
            details)
    return {
        "status": "passed",
        "message": "CPU gate counter docs are current",
        "details": details,
    }


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=(
        "Check docs/mklq/cpu-gate-counters.md against tracked bounded "
        "CPU gate counter reports."))
    parser.add_argument("--reports",
                        type=Path,
                        nargs="+",
                        default=[DEFAULT_REPORTS_DIR],
                        help="Report files or directories to use.")
    parser.add_argument("--doc",
                        type=Path,
                        default=DEFAULT_DOC,
                        help="Tracked Markdown document to check.")
    parser.add_argument("--pattern",
                        default=DEFAULT_PATTERN,
                        help="Glob used when a --reports path is a directory.")
    parser.add_argument("--json",
                        action="store_true",
                        help="Emit JSON instead of a short status line.")
    return parser.parse_args(argv)


def main(argv: list[str]) -> int:
    args = parse_args(argv)
    result = check_docs(report_inputs=args.reports,
                        doc_path=args.doc,
                        pattern=args.pattern)
    if args.json:
        sys.stdout.write(json.dumps(result, indent=2, sort_keys=True) + "\n")
    elif result["status"] == "passed":
        sys.stdout.write(f"CPU gate counter docs are current: "
                         f"{result['details']['report_count']} report(s).\n")
    else:
        sys.stderr.write(result["message"] + "\n")
    return 0 if result["status"] == "passed" else 1


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
