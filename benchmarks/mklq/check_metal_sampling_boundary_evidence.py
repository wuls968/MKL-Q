#!/usr/bin/env python3
# ============================================================================ #
# Copyright (c) 2026 Linsen Wu.                                                #
#                                                                              #
# This source code and the accompanying materials are made available under     #
# the terms of the Apache License 2.0 which accompanies this distribution.     #
# ============================================================================ #

"""Check MKL-Q Metal stochastic sampling boundary evidence."""

import argparse
import json
import math
from pathlib import Path
from typing import Any


CHECK_SCHEMA_VERSION = "mklq-metal-sampling-boundary-evidence-check-v1"
SUMMARY_SCHEMA_VERSION = "mklq-benchmark-summary-v1"
DEFAULT_EVIDENCE_KIND = "local_tuning_evidence"
DEFAULT_REPORT_PATTERN = "*.summary.json"
DEFAULT_SUMMARY_ID = (
    "local-metal-partial-count-accumulation-sampling-q20-2026-07-05")
Q22_SUMMARY_ID = (
    "local-metal-partial-count-accumulation-sampling-q22-2026-07-05")
Q24_SUMMARY_ID = (
    "local-metal-partial-count-accumulation-sampling-q24-2026-07-05")
DEFAULT_SUMMARY_IDS = (DEFAULT_SUMMARY_ID, Q22_SUMMARY_ID, Q24_SUMMARY_ID)
METAL_TARGET = "mklq-metal"
DEFAULT_REQUIRED_QUBITS = (20, 22, 24)
DEFAULT_REQUIRED_SHOTS = (256, 1024, 8192, 65536)
DEFAULT_MAX_HIGH_TO_LOW_RATIO = 2.0


def required_rows_for_qubits(qubits: int) -> tuple[dict[str, Any], ...]:
    return tuple(
        {
            "target": METAL_TARGET,
            "case": case,
            "qubits": qubits,
            "shots": shots,
        }
        for case in ("sample-full-register", "sample-partial-register")
        for shots in DEFAULT_REQUIRED_SHOTS)


DEFAULT_REQUIRED_ROWS = required_rows_for_qubits(20)
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
    return f"{row['case']} q{row['qubits']} shots={row['shots']}"


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


def states_host_draw_count_boundary(value: str) -> bool:
    normalized = value.lower()
    return ("host-side" in normalized and
            ("draw/count" in normalized or
             ("draw" in normalized and "count" in normalized)))


def states_selected_metal_count_accumulation(value: str) -> bool:
    normalized = value.lower()
    has_metal_sample_counts = (
        "metal" in normalized and "sample-count" in normalized)
    has_historical_host_draws = "host-generated draw" in normalized
    has_current_device_draws = (
        "device-generated draw" in normalized or
        ("generated draw" in normalized and "host-generated" not in normalized))
    return has_metal_sample_counts and (has_historical_host_draws or
                                        has_current_device_draws)


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


def target_values(summary: dict[str, Any]) -> set[str]:
    config = summary.get("config")
    if not isinstance(config, dict):
        return set()
    targets = config.get("targets")
    if not isinstance(targets, list):
        return set()
    return {str(target) for target in targets}


def single_config_qubits(summary: dict[str, Any]) -> int | None:
    config = summary.get("config")
    if not isinstance(config, dict):
        return None
    qubits = config.get("qubits")
    if not isinstance(qubits, list) or len(qubits) != 1:
        return None
    value = qubits[0]
    if isinstance(value, bool) or not isinstance(value, int):
        return None
    return value


def check_raw_results(raw_results: Any) -> list[str]:
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

    performance_scope = str(interpretation.get("performance_claim_scope", ""))
    if "local" not in performance_scope.lower():
        failures.append("performance scope does not state local-only evidence")
    if "cross-machine" not in performance_scope.lower():
        failures.append("performance scope does not reject cross-machine claims")

    metal_scope = str(interpretation.get("metal_path_scope", ""))
    normalized_scope = metal_scope.lower()
    if "mixed-path" not in normalized_scope:
        failures.append("metal path scope does not state mixed-path execution")

    full_register_counts = str(
        interpretation.get("full_register_counts_accumulation", ""))
    partial_register_counts = str(
        interpretation.get("partial_register_counts_accumulation", ""))
    has_selected_metal_counts = (
        states_selected_metal_count_accumulation(metal_scope) or
        states_selected_metal_count_accumulation(full_register_counts))
    has_host_counts_boundary = states_host_draw_count_boundary(metal_scope)

    if has_selected_metal_counts:
        if not (states_host_draw_count_boundary(partial_register_counts) or
                states_selected_metal_count_accumulation(
                    partial_register_counts)):
            failures.append(
                "partial-register counts accumulation boundary must state "
                "host-side draw/count accumulation or selected Metal "
                "sample-count accumulation with host-generated or "
                "device-generated draws")
        if not states_selected_metal_count_accumulation(full_register_counts):
            failures.append(
                "full-register counts accumulation boundary must state "
                "selected Metal sample-count accumulation with "
                "host-generated or device-generated draws")
    elif not has_host_counts_boundary:
        failures.append(
            "metal path scope does not state host-side draw/count accumulation")

    failures.extend(
        forbidden_claim_failures({
            "interpretation.performance_claim_scope": performance_scope,
            "interpretation.metal_path_scope": metal_scope,
            "interpretation.full_register_counts_accumulation":
                full_register_counts,
            "interpretation.partial_register_counts_accumulation":
                partial_register_counts,
            "interpretation.scope": str(interpretation.get("scope", "")),
            "interpretation.summary": str(interpretation.get("summary", "")),
        }))
    return failures


def check_summary(summary: dict[str, Any], *,
                  required_rows: tuple[dict[str, Any], ...],
                  max_high_to_low_ratio: float) -> dict[str, Any]:
    failures: list[str] = []
    required_qubits = (required_rows[0].get("qubits")
                       if required_rows else None)

    if summary.get("schema_version") != SUMMARY_SCHEMA_VERSION:
        failures.append("unexpected schema_version")
    if summary.get("evidence_kind") != DEFAULT_EVIDENCE_KIND:
        failures.append("unexpected evidence_kind")
    if METAL_TARGET not in target_values(summary):
        failures.append(f"config.targets does not include {METAL_TARGET}")

    failures.extend(check_interpretation(summary.get("interpretation")))
    failures.extend(check_raw_results(summary.get("raw_results")))

    rows = summary.get("rows")
    if not isinstance(rows, list):
        failures.append("missing rows list")
        rows = []
    typed_rows = [row for row in rows if isinstance(row, dict)]

    checked_row_count = 0
    rows_by_case_shots: dict[tuple[str, int], dict[str, Any]] = {}
    for required in required_rows:
        label = row_label(required)
        row = find_required_row(typed_rows, required)
        if row is None:
            failures.append(f"missing required Metal sampling row {label}")
            continue
        checked_row_count += 1
        rows_by_case_shots[(required["case"], required["shots"])] = row
        if row.get("status") != "ok":
            failures.append(f"{label}: status is not ok")
        if not finite_positive(row.get("elapsed_seconds_median")):
            failures.append(f"{label}: elapsed_seconds_median is not positive finite")
        if not finite_positive(row.get("estimated_state_bytes")):
            failures.append(f"{label}: estimated_state_bytes is not positive finite")
        if row.get("metal_full_native") is True:
            failures.append(f"{label}: row claims full Metal-native sampling")
        failures.extend(
            forbidden_claim_failures({
                f"{label} metal_path_scope":
                    str(row.get("metal_path_scope", "")),
                f"{label} metal_evidence_boundary":
                    str(row.get("metal_evidence_boundary", "")),
            }))

    ratios: dict[str, float] = {}
    low_shot = min(DEFAULT_REQUIRED_SHOTS)
    high_shot = max(DEFAULT_REQUIRED_SHOTS)
    for case in sorted({row["case"] for row in required_rows}):
        low_row = rows_by_case_shots.get((case, low_shot))
        high_row = rows_by_case_shots.get((case, high_shot))
        if not low_row or not high_row:
            continue
        low_elapsed = low_row.get("elapsed_seconds_median")
        high_elapsed = high_row.get("elapsed_seconds_median")
        if not (finite_positive(low_elapsed) and finite_positive(high_elapsed)):
            continue
        ratio = float(high_elapsed) / float(low_elapsed)
        ratios[case] = ratio
        if ratio > max_high_to_low_ratio:
            failures.append(
                f"{case}: high/low shot elapsed ratio {ratio:.3f} exceeds "
                f"{max_high_to_low_ratio:.3f}")

    max_ratio = max(ratios.values()) if ratios else None
    return {
        "status": "passed" if not failures else "failed",
        "summary_id": summary.get("summary_id"),
        "required_qubits": required_qubits,
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
            qubits = single_config_qubits(payload)
            if qubits not in DEFAULT_REQUIRED_QUBITS:
                result = {
                    "status": "failed",
                    "summary_id": payload.get("summary_id"),
                    "required_qubits": qubits,
                    "checked_row_count": 0,
                    "required_row_count": 0,
                    "max_high_to_low_ratio": None,
                    "high_to_low_ratios": {},
                    "failures": [
                        "summary config.qubits must contain exactly one "
                        f"supported value from {DEFAULT_REQUIRED_QUBITS}"
                    ],
                }
            else:
                required_rows = required_rows_for_qubits(qubits)
                result = check_summary(
                    payload,
                    required_rows=required_rows,
                    max_high_to_low_ratio=max_high_to_low_ratio,
                )
        except (OSError, json.JSONDecodeError, ValueError) as exc:
            result = {
                "status": "failed",
                "summary_id": None,
                "required_qubits": None,
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
            "required_qubits": result["required_qubits"],
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
        description="Check bounded MKL-Q Metal sampling boundary evidence.")
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
    summary_ids = set(args.summary_id or DEFAULT_SUMMARY_IDS)
    result = check_reports(args.reports, args.pattern, summary_ids,
                           args.max_high_to_low_ratio)
    print(json.dumps(result, indent=2, sort_keys=True))
    return 0 if result["summary"]["status"] == "passed" else 1


if __name__ == "__main__":
    raise SystemExit(main())
