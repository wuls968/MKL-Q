#!/usr/bin/env python3
# ============================================================================ #
# Copyright (c) 2026 Linsen Wu.                                                #
#                                                                              #
# This source code and the accompanying materials are made available under     #
# the terms of the Apache License 2.0 which accompanies this distribution.     #
# ============================================================================ #

"""Check sanitized MKL-Q sampling profile summaries for public evidence bounds."""

import argparse
import json
import math
from pathlib import Path
from typing import Any


CHECK_SCHEMA_VERSION = "mklq-sampling-profile-evidence-check-v1"
SUMMARY_SCHEMA_VERSION = "mklq-benchmark-summary-v1"
DEFAULT_EVIDENCE_KIND = "local_tuning_evidence"
DEFAULT_REPORT_PATTERN = "*.summary.json"
DEFAULT_SUMMARY_ID = "local-sampling-profile-breakdown-cpu-q20-q22-2026-06-23"
SAMPLING_PROFILE_SCOPE = (
    "benchmark_harness_diagnostic_timing_not_native_backend_counters")
REQUIRED_POSITIVE_FIELDS = (
    "elapsed_seconds_median",
    "sample_latency_seconds_per_shot",
    "sample_throughput_shots_per_second",
    "sampling_kernel_build_seconds_median",
    "sampling_result_counts_materialization_seconds_median",
)
DEFAULT_REQUIRED_ROWS = tuple(
    {
        "target": "mklq-cpu",
        "case": case,
        "qubits": qubits,
        "shots": shots,
    }
    for case in ("sample-full-register", "sample-partial-register")
    for qubits in (20, 22)
    for shots in (1024, 65536))


def repo_root() -> Path:
    return Path(__file__).resolve().parents[2]


def command_path(root: Path, path: Path) -> str:
    try:
        return path.relative_to(root).as_posix()
    except ValueError:
        return path.as_posix()


def finite_positive(value: Any) -> bool:
    return (isinstance(value, (int, float)) and not isinstance(value, bool)
            and math.isfinite(float(value)) and float(value) > 0.0)


def int_at_least(value: Any, minimum: int) -> bool:
    try:
        parsed = int(value)
    except (TypeError, ValueError):
        return False
    return parsed >= minimum


def row_label(row: dict[str, Any]) -> str:
    return f"{row['case']} q{row['qubits']} shots={row['shots']}"


def load_json(path: Path) -> dict[str, Any]:
    payload = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(payload, dict):
        raise ValueError(f"{path} is not a JSON object")
    return payload


def list_summaries(reports: Path, pattern: str,
                   summary_ids: set[str]) -> list[Path]:
    paths = sorted(reports.glob(pattern))
    if not summary_ids:
        return paths
    selected: list[Path] = []
    for path in paths:
        try:
            summary = load_json(path)
        except (OSError, json.JSONDecodeError, ValueError):
            selected.append(path)
            continue
        if str(summary.get("summary_id")) in summary_ids:
            selected.append(path)
    return selected


def find_required_row(rows: list[dict[str, Any]],
                      required: dict[str, Any]) -> dict[str, Any] | None:
    for row in rows:
        if all(row.get(key) == value for key, value in required.items()):
            return row
    return None


def check_summary(summary: dict[str, Any], *,
                  required_rows: tuple[dict[str, Any], ...]) -> dict[str, Any]:
    failures: list[str] = []

    if summary.get("schema_version") != SUMMARY_SCHEMA_VERSION:
        failures.append("unexpected schema_version")
    if summary.get("evidence_kind") != DEFAULT_EVIDENCE_KIND:
        failures.append("unexpected evidence_kind")

    interpretation = summary.get("interpretation")
    if not isinstance(interpretation, dict):
        failures.append("missing interpretation object")
        interpretation = {}
    if interpretation.get("clean_worktree") is not True:
        failures.append("interpretation.clean_worktree is not true")
    if interpretation.get("raw_json_files_are_ignored") is not True:
        failures.append("raw JSON files are not marked ignored")
    scope = str(interpretation.get("performance_claim_scope", ""))
    if "not" not in scope or "cross-machine" not in scope:
        failures.append("performance scope does not reject cross-machine claims")
    if "native backend internal phase counters" not in scope:
        failures.append("performance scope does not separate native counters")

    rows = summary.get("rows")
    if not isinstance(rows, list):
        failures.append("missing rows list")
        rows = []
    typed_rows = [row for row in rows if isinstance(row, dict)]

    checked_row_count = 0
    for required in required_rows:
        label = row_label(required)
        row = find_required_row(typed_rows, required)
        if row is None:
            failures.append(f"missing required sampling profile row {label}")
            continue
        checked_row_count += 1
        if row.get("status") != "ok":
            failures.append(f"{label}: status is not ok")
        if row.get("sampling_profile_enabled") is not True:
            failures.append(f"{label}: sampling profile is not enabled")
        if row.get("sampling_profile_scope") != SAMPLING_PROFILE_SCOPE:
            failures.append(f"{label}: unexpected sampling profile scope")
        boundary = str(row.get("sampling_profile_boundary", ""))
        if "not native backend internal phase counters" not in boundary:
            failures.append(f"{label}: boundary does not reject native counters")
        if not int_at_least(row.get("sampling_profile_extra_sample_calls"), 1):
            failures.append(f"{label}: missing extra profile sample call")
        for field in REQUIRED_POSITIVE_FIELDS:
            if not finite_positive(row.get(field)):
                failures.append(f"{label}: {field} is not positive finite")

    return {
        "status": "passed" if not failures else "failed",
        "summary_id": summary.get("summary_id"),
        "checked_row_count": checked_row_count,
        "required_row_count": len(required_rows),
        "failures": failures,
    }


def check_reports(reports: Path, pattern: str,
                  summary_ids: set[str]) -> dict[str, Any]:
    root = repo_root()
    paths = list_summaries(reports, pattern, summary_ids)
    summaries = []
    for path in paths:
        failures: list[str] = []
        try:
            payload = load_json(path)
            result = check_summary(payload, required_rows=DEFAULT_REQUIRED_ROWS)
        except (OSError, json.JSONDecodeError, ValueError) as exc:
            result = {
                "status": "failed",
                "checked_row_count": 0,
                "required_row_count": len(DEFAULT_REQUIRED_ROWS),
                "failures": [str(exc)],
            }
        if result["status"] != "passed":
            failures.extend(result["failures"])
        summaries.append({
            "path": command_path(root, path),
            "summary_id": result.get("summary_id"),
            "status": result["status"],
            "checked_row_count": result["checked_row_count"],
            "required_row_count": result["required_row_count"],
            "failures": failures,
        })

    failed = sum(1 for summary in summaries if summary["status"] != "passed")
    return {
        "schema_version": CHECK_SCHEMA_VERSION,
        "config": {
            "reports": command_path(root, reports),
            "pattern": pattern,
            "summary_ids": sorted(summary_ids),
            "required_rows": list(DEFAULT_REQUIRED_ROWS),
        },
        "summaries": summaries,
        "summary": {
            "status": "passed" if summaries and failed == 0 else "failed",
            "checked": len(summaries),
            "passed": len(summaries) - failed,
            "failed": failed,
        },
    }


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Check bounded MKL-Q sampling profile evidence summaries.")
    parser.add_argument("--reports",
                        type=Path,
                        default=repo_root() / "benchmarks" / "mklq" /
                        "reports")
    parser.add_argument("--pattern", default=DEFAULT_REPORT_PATTERN)
    parser.add_argument("--summary-id",
                        action="append",
                        default=None)
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    summary_ids = set(args.summary_id or [DEFAULT_SUMMARY_ID])
    result = check_reports(args.reports, args.pattern, summary_ids)
    print(json.dumps(result, indent=2, sort_keys=True))
    return 0 if result["summary"]["status"] == "passed" else 1


if __name__ == "__main__":
    raise SystemExit(main())
