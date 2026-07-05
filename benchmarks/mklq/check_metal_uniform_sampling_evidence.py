#!/usr/bin/env python3
# ============================================================================ #
# Copyright (c) 2026 Linsen Wu.                                                #
#                                                                              #
# This source code and the accompanying materials are made available under     #
# the terms of the Apache License 2.0 which accompanies this distribution.     #
# ============================================================================ #

"""Check MKL-Q Metal uniform partial-register sampling evidence."""

import argparse
import json
import math
from pathlib import Path
from typing import Any


CHECK_SCHEMA_VERSION = "mklq-metal-uniform-sampling-evidence-check-v1"
SUMMARY_SCHEMA_VERSION = "mklq-benchmark-summary-v1"
DEFAULT_EVIDENCE_KIND = "local_tuning_evidence"
DEFAULT_REPORT_PATTERN = "*.summary.json"
DEFAULT_SUMMARY_ID = (
    "local-metal-uniform-partial-sampling-q20-q24-2026-07-05")
METAL_TARGET = "mklq-metal"
REQUIRED_CASE = "sample-uniform-partial-register"
DEFAULT_REQUIRED_QUBITS = (20, 22, 24)
DEFAULT_REQUIRED_SHOTS = (256, 1024, 8192, 65536)
DEFAULT_REQUIRED_MEASURED_QUBITS = 12
DEFAULT_REQUIRED_OUTCOMES = 1 << DEFAULT_REQUIRED_MEASURED_QUBITS
DEFAULT_MAX_HIGH_TO_LOW_RATIO = 2.0
REQUIRED_METAL_PATH_LABEL = (
    "mklq_metal_uniform_partial_register_sample_count_accumulation")
REQUIRED_FAST_PATH_PHRASE = "uniform-probability generated-count fast path"


def required_rows() -> tuple[dict[str, Any], ...]:
    return tuple(
        {
            "target": METAL_TARGET,
            "case": REQUIRED_CASE,
            "qubits": qubits,
            "shots": shots,
        }
        for qubits in DEFAULT_REQUIRED_QUBITS
        for shots in DEFAULT_REQUIRED_SHOTS)


DEFAULT_REQUIRED_ROWS = required_rows()
FORBIDDEN_SAMPLING_CLAIMS = (
    "metal rng",
    "gpu rng",
    "on-device sampler",
    "device-side sampler",
    "gpu sampler",
    "gpu-side count accumulation",
    "on-device count accumulation",
    "full metal-native",
    "fully metal-native",
    "release-ready",
    "production-ready",
    "release certification",
)


def repo_root() -> Path:
    return Path(__file__).resolve().parents[2]


def command_path(root: Path, path: Path) -> str:
    try:
        return path.relative_to(root).as_posix()
    except ValueError:
        return path.as_posix()


def load_json(path: Path) -> dict[str, Any]:
    payload = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(payload, dict):
        raise ValueError(f"{path} is not a JSON object")
    return payload


def finite_positive(value: Any) -> bool:
    return (isinstance(value, (int, float)) and not isinstance(value, bool)
            and math.isfinite(float(value)) and float(value) > 0.0)


def all_status_rows_ok(status_rows: Any) -> bool:
    if not isinstance(status_rows, dict) or not status_rows:
        return False
    return all(key == "ok" and isinstance(value, int) and value > 0
               for key, value in status_rows.items())


def row_label(row: dict[str, Any]) -> str:
    return f"q{row['qubits']} shots={row['shots']}"


def find_required_row(rows: list[dict[str, Any]],
                      required: dict[str, Any]) -> dict[str, Any] | None:
    for row in rows:
        if all(row.get(key) == value for key, value in required.items()):
            return row
    return None


def forbidden_claim_failures(values: dict[str, str]) -> list[str]:
    failures: list[str] = []
    for field, value in values.items():
        normalized = value.lower()
        for phrase in FORBIDDEN_SAMPLING_CLAIMS:
            if phrase in normalized:
                failures.append(
                    f"{field} contains forbidden sampling claim {phrase!r}")
    return failures


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


def config_values(summary: dict[str, Any], key: str) -> set[Any]:
    config = summary.get("config")
    if not isinstance(config, dict):
        return set()
    values = config.get(key)
    if not isinstance(values, list):
        return set()
    return set(values)


def check_raw_results(raw_results: Any,
                      required_row_count: int) -> list[str]:
    failures: list[str] = []
    if not isinstance(raw_results, list) or not raw_results:
        return ["missing raw_results"]
    for index, raw in enumerate(raw_results):
        if not isinstance(raw, dict):
            failures.append(f"raw_results[{index}] is not an object")
            continue
        path = str(raw.get("path", ""))
        if not path.startswith("benchmarks/mklq/results/"):
            failures.append(f"raw_results[{index}] path is outside ignored results")
        if raw.get("tracked") is not False:
            failures.append(f"raw_results[{index}] is not marked untracked")
        sha256 = raw.get("sha256")
        if not isinstance(sha256, str) or len(sha256) != 64:
            failures.append(f"raw_results[{index}] has invalid sha256")
        if not all_status_rows_ok(raw.get("status_rows")):
            failures.append(f"raw_results[{index}] has non-ok or missing rows")
            continue
        if raw["status_rows"].get("ok", 0) < required_row_count:
            failures.append(f"raw_results[{index}] has too few ok rows")
    return failures


def check_interpretation(interpretation: Any) -> list[str]:
    failures: list[str] = []
    if not isinstance(interpretation, dict):
        return ["missing interpretation object"]
    if interpretation.get("do_not_treat_as_clean_release_provenance") is not True:
        failures.append(
            "interpretation.do_not_treat_as_clean_release_provenance is not true"
        )
    if interpretation.get("raw_json_files_are_ignored") is not True:
        failures.append("raw JSON files are not marked ignored")
    if interpretation.get("sequential_data_accessor_not_invoked") is not True:
        failures.append("sequential_data accessor is not marked uninvoked")
    if interpretation.get("standard_sample_counts_only_path") is not True:
        failures.append("standard counts-only path is not marked true")
    if interpretation.get("uniform_partial_register_fixture") is not True:
        failures.append("uniform partial-register fixture is not marked true")

    performance_scope = str(interpretation.get("performance_claim_scope", ""))
    if "local" not in performance_scope.lower():
        failures.append("performance scope does not state local-only evidence")
    if "cross-machine" not in performance_scope.lower():
        failures.append("performance scope does not reject cross-machine claims")

    metal_scope = str(interpretation.get("metal_path_scope", ""))
    normalized_scope = metal_scope.lower()
    if "mixed-path" not in normalized_scope:
        failures.append("metal path scope does not state mixed-path execution")
    if REQUIRED_FAST_PATH_PHRASE not in normalized_scope:
        failures.append(
            "metal path scope does not state the "
            f"{REQUIRED_FAST_PATH_PHRASE}")

    failures.extend(
        forbidden_claim_failures({
            "interpretation.performance_claim_scope": performance_scope,
            "interpretation.metal_path_scope": metal_scope,
            "interpretation.scope": str(interpretation.get("scope", "")),
            "interpretation.summary": str(interpretation.get("summary", "")),
        }))
    return failures


def check_summary(summary: dict[str, Any], *,
                  required_rows: tuple[dict[str, Any], ...],
                  max_high_to_low_ratio: float) -> dict[str, Any]:
    failures: list[str] = []

    if summary.get("schema_version") != SUMMARY_SCHEMA_VERSION:
        failures.append("unexpected schema_version")
    if summary.get("evidence_kind") != DEFAULT_EVIDENCE_KIND:
        failures.append("unexpected evidence_kind")
    if METAL_TARGET not in config_values(summary, "targets"):
        failures.append(f"config.targets does not include {METAL_TARGET}")
    if REQUIRED_CASE not in config_values(summary, "cases"):
        failures.append(f"config.cases does not include {REQUIRED_CASE}")
    if not set(DEFAULT_REQUIRED_QUBITS).issubset(
            config_values(summary, "qubits")):
        failures.append("config.qubits does not include required q20/q22/q24")
    if not set(DEFAULT_REQUIRED_SHOTS).issubset(
            config_values(summary, "shot_counts")):
        failures.append("config.shot_counts does not include required shots")

    failures.extend(check_interpretation(summary.get("interpretation")))
    failures.extend(
        check_raw_results(summary.get("raw_results"), len(required_rows)))

    rows = summary.get("rows")
    if not isinstance(rows, list):
        failures.append("missing rows list")
        rows = []
    typed_rows = [row for row in rows if isinstance(row, dict)]

    checked_row_count = 0
    rows_by_qubits_shots: dict[tuple[int, int], dict[str, Any]] = {}
    for required in required_rows:
        label = row_label(required)
        row = find_required_row(typed_rows, required)
        if row is None:
            failures.append(f"missing required Metal uniform sampling row {label}")
            continue
        checked_row_count += 1
        rows_by_qubits_shots[(required["qubits"], required["shots"])] = row
        if row.get("status") != "ok":
            failures.append(f"{label}: status is not ok")
        if not finite_positive(row.get("elapsed_seconds_median")):
            failures.append(f"{label}: elapsed_seconds_median is not positive finite")
        if not finite_positive(row.get("estimated_state_bytes")):
            failures.append(f"{label}: estimated_state_bytes is not positive finite")
        if row.get("measured_qubit_count") != DEFAULT_REQUIRED_MEASURED_QUBITS:
            failures.append(
                f"{label}: measured_qubit_count is not "
                f"{DEFAULT_REQUIRED_MEASURED_QUBITS}")
        if row.get("marginal_outcome_count") != DEFAULT_REQUIRED_OUTCOMES:
            failures.append(
                f"{label}: marginal_outcome_count is not "
                f"{DEFAULT_REQUIRED_OUTCOMES}")
        if row.get("uniform_probability_distribution") is not True:
            failures.append(
                f"{label}: uniform_probability_distribution is not true")
        if row.get("uniform_partial_register_measured_qubit_limit") != (
                DEFAULT_REQUIRED_MEASURED_QUBITS):
            failures.append(
                f"{label}: uniform partial-register measured-qubit limit "
                f"is not {DEFAULT_REQUIRED_MEASURED_QUBITS}")
        if row.get("metal_path_label") != REQUIRED_METAL_PATH_LABEL:
            failures.append(f"{label}: metal_path_label is not expected")
        metal_scope = str(row.get("metal_path_scope", ""))
        if REQUIRED_FAST_PATH_PHRASE not in metal_scope.lower():
            failures.append(
                f"{label}: metal_path_scope does not state the "
                f"{REQUIRED_FAST_PATH_PHRASE}")
        if row.get("metal_full_native") is not False:
            failures.append(f"{label}: row does not mark metal_full_native false")
        if row.get("metal_runtime_counter") is not False:
            failures.append(f"{label}: row does not mark metal_runtime_counter false")
        failures.extend(
            forbidden_claim_failures({
                f"{label} metal_path_scope": metal_scope,
                f"{label} metal_evidence_boundary":
                    str(row.get("metal_evidence_boundary", "")),
            }))

    ratios: dict[str, float] = {}
    low_shot = min(DEFAULT_REQUIRED_SHOTS)
    high_shot = max(DEFAULT_REQUIRED_SHOTS)
    for qubits in DEFAULT_REQUIRED_QUBITS:
        low_row = rows_by_qubits_shots.get((qubits, low_shot))
        high_row = rows_by_qubits_shots.get((qubits, high_shot))
        if not low_row or not high_row:
            continue
        low_elapsed = low_row.get("elapsed_seconds_median")
        high_elapsed = high_row.get("elapsed_seconds_median")
        if not (finite_positive(low_elapsed) and finite_positive(high_elapsed)):
            continue
        ratio = float(high_elapsed) / float(low_elapsed)
        ratios[f"q{qubits}"] = ratio
        if ratio > max_high_to_low_ratio:
            failures.append(
                f"q{qubits}: high/low shot elapsed ratio {ratio:.3f} exceeds "
                f"{max_high_to_low_ratio:.3f}")

    max_ratio = max(ratios.values()) if ratios else None
    return {
        "status": "passed" if not failures else "failed",
        "summary_id": summary.get("summary_id"),
        "checked_row_count": checked_row_count,
        "required_row_count": len(required_rows),
        "max_high_to_low_ratio": max_ratio,
        "high_to_low_ratios": ratios,
        "failures": failures,
    }


def check_reports(reports: Path, pattern: str,
                  summary_ids: set[str],
                  max_high_to_low_ratio: float = (
                      DEFAULT_MAX_HIGH_TO_LOW_RATIO)) -> dict[str, Any]:
    root = repo_root()
    paths = list_summaries(reports, pattern, summary_ids)
    summaries = []
    for path in paths:
        try:
            payload = load_json(path)
            result = check_summary(
                payload,
                required_rows=DEFAULT_REQUIRED_ROWS,
                max_high_to_low_ratio=max_high_to_low_ratio,
            )
        except (OSError, json.JSONDecodeError, ValueError) as exc:
            result = {
                "status": "failed",
                "summary_id": None,
                "checked_row_count": 0,
                "required_row_count": len(DEFAULT_REQUIRED_ROWS),
                "max_high_to_low_ratio": None,
                "high_to_low_ratios": {},
                "failures": [str(exc)],
            }
        summaries.append({
            "path": command_path(root, path),
            "summary_id": result.get("summary_id"),
            "status": result["status"],
            "checked_row_count": result["checked_row_count"],
            "required_row_count": result["required_row_count"],
            "max_high_to_low_ratio": result["max_high_to_low_ratio"],
            "high_to_low_ratios": result["high_to_low_ratios"],
            "failures": result["failures"],
        })

    failed = sum(1 for summary in summaries if summary["status"] != "passed")
    return {
        "schema_version": CHECK_SCHEMA_VERSION,
        "config": {
            "reports": command_path(root, reports),
            "pattern": pattern,
            "summary_ids": sorted(summary_ids),
            "required_qubits": list(DEFAULT_REQUIRED_QUBITS),
            "required_shots": list(DEFAULT_REQUIRED_SHOTS),
            "required_case": REQUIRED_CASE,
            "max_high_to_low_ratio": max_high_to_low_ratio,
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
        description="Check bounded MKL-Q Metal uniform sampling evidence.")
    parser.add_argument("--reports",
                        type=Path,
                        default=repo_root() / "benchmarks" / "mklq" /
                        "reports")
    parser.add_argument("--pattern", default=DEFAULT_REPORT_PATTERN)
    parser.add_argument("--summary-id",
                        action="append",
                        default=None)
    parser.add_argument("--max-high-to-low-ratio",
                        type=float,
                        default=DEFAULT_MAX_HIGH_TO_LOW_RATIO)
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    summary_ids = set(args.summary_id or {DEFAULT_SUMMARY_ID})
    result = check_reports(args.reports, args.pattern, summary_ids,
                           args.max_high_to_low_ratio)
    print(json.dumps(result, indent=2, sort_keys=True))
    return 0 if result["summary"]["status"] == "passed" else 1


if __name__ == "__main__":
    raise SystemExit(main())
