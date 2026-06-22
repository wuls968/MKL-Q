#!/usr/bin/env python3
# ============================================================================ #
# Copyright (c) 2026 Linsen Wu.                                                #
#                                                                              #
# This source code and the accompanying materials are made available under     #
# the terms of the Apache License 2.0 which accompanies this distribution.     #
# ============================================================================ #

"""Check that public Metal runtime counter docs match tracked reports."""

import argparse
import difflib
import importlib.util
import json
import sys
from pathlib import Path
from typing import Any


DEFAULT_REPORTS_DIR = Path("benchmarks/mklq/reports")
DEFAULT_DOC = Path("docs/mklq/metal-runtime-counters.md")


def _load_summary_module():
    script = Path(__file__).resolve().with_name(
        "summarize_metal_runtime_counters.py")
    spec = importlib.util.spec_from_file_location(
        "summarize_metal_runtime_counters", script)
    module = importlib.util.module_from_spec(spec)
    if spec.loader is None:
        raise RuntimeError(f"cannot load {script}")
    spec.loader.exec_module(module)
    return module


def _as_posix(path: Path) -> str:
    return path.as_posix()


def _failed(message: str, details: dict[str, Any]) -> dict[str, Any]:
    return {"status": "failed", "message": message, "details": details}


def check_docs(report_inputs: list[Path] | None = None,
               doc_path: Path = DEFAULT_DOC,
               pattern: str = "*.counter.json") -> dict[str, Any]:
    summary_module = _load_summary_module()
    report_inputs = report_inputs or [DEFAULT_REPORTS_DIR]
    paths = summary_module.discover_counter_reports(report_inputs, pattern)
    summary = summary_module.build_summary(paths)
    generated = summary_module.render_markdown(summary)
    current = doc_path.read_text(encoding="utf-8") if doc_path.exists() else None
    summary_status = summary.get("summary", {}).get("status", "unknown")
    details = {
        "expected": _as_posix(doc_path),
        "reports": [_as_posix(path) for path in paths],
        "report_count": len(paths),
        "summary_status": summary_status,
    }

    if current is None:
        return _failed("Metal runtime counter docs are missing", details)
    if summary_status != "passed":
        return _failed("Metal runtime counter summary is not passing", details)
    if current != generated:
        diff = difflib.unified_diff(current.splitlines(),
                                    generated.splitlines(),
                                    fromfile=_as_posix(doc_path),
                                    tofile="regenerated-metal-runtime-counters.md",
                                    lineterm="")
        details["diff"] = "\n".join(list(diff)[:160])
        return _failed(
            "tracked Metal runtime counter docs output differs from regenerated output",
            details)

    return {"status": "passed", "details": details}


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Check docs/mklq/metal-runtime-counters.md against tracked bounded "
            "Metal runtime counter reports."))
    parser.add_argument("--reports",
                        nargs="+",
                        type=Path,
                        default=[DEFAULT_REPORTS_DIR],
                        help=(
                            "Report file(s) or directory/directories to read. "
                            "Directories are searched for *.counter.json."))
    parser.add_argument("--doc",
                        type=Path,
                        default=DEFAULT_DOC,
                        help="Tracked public Markdown file to compare.")
    parser.add_argument("--pattern",
                        default="*.counter.json",
                        help="Glob used when a --reports path is a directory.")
    parser.add_argument("--json",
                        action="store_true",
                        help="Print the full check result as JSON.")
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(sys.argv[1:] if argv is None else argv)
    result = check_docs(report_inputs=args.reports,
                        doc_path=args.doc,
                        pattern=args.pattern)
    if args.json:
        print(json.dumps(result, indent=2, sort_keys=True))
    elif result["status"] == "passed":
        details = result["details"]
        print("Metal runtime counter docs are current: "
              f"{details['report_count']} report(s).")
    else:
        print(result["message"], file=sys.stderr)
        diff = result.get("details", {}).get("diff")
        if diff:
            print(diff, file=sys.stderr)
    return 0 if result["status"] == "passed" else 1


if __name__ == "__main__":
    raise SystemExit(main())
