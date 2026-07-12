#!/usr/bin/env python3
# ============================================================================ #
# Copyright (c) 2026 Linsen Wu.                                                #
#                                                                              #
# This source code and the accompanying materials are made available under     #
# the terms of the Apache License 2.0 which accompanies this distribution.     #
# ============================================================================ #

"""Collect native MKL-Q sampling phase timings from opt-in backend tests.

This tool is deliberately separate from the Python benchmark harness. It reads
test-accessor timers emitted by build-tree CPU and Metal simulators, so its
numbers are useful for attributing local sampling cost but are not cross-machine
performance claims or release sign-off.
"""

from __future__ import annotations

import argparse
import json
import math
import os
import platform
import statistics
import subprocess
import sys
import tempfile
import time
import xml.etree.ElementTree as element_tree
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


SCHEMA_VERSION = "mklq-sampling-phase-profile-probe-v1"
EVIDENCE_KIND = "local_native_phase_timing_probe"
PROFILE_TEST_SUFFIX = "SamplingPhaseProfileEmitsNativeTimingForExternalProbe"
TARGETS = {
    "mklq-cpu": {
        "ctest_name": "test_mklq_cpu_backend",
        "gtest_filter": "mklq_cpu_MKLQCpuTester." + PROFILE_TEST_SUFFIX,
    },
    "mklq-metal": {
        "ctest_name": "test_mklq_metal_backend",
        "gtest_filter": "mklq_metal_MKLQMetalTester." + PROFILE_TEST_SUFFIX,
    },
}
PHASE_PROPERTY_NAMES = {
    "probability_fill_seconds":
        "mklq_sampling_phase_profile_probability_fill_seconds",
    "draw_and_count_seconds":
        "mklq_sampling_phase_profile_draw_and_count_seconds",
    "expectation_reduction_seconds":
        "mklq_sampling_phase_profile_expectation_reduction_seconds",
}
OPTIONAL_PROPERTY_NAMES = {
    "metal_probability_fill_applications":
        "mklq_sampling_phase_profile_metal_probability_fill_applications",
    "metal_marginal_probability_applications":
        "mklq_sampling_phase_profile_metal_marginal_probability_applications",
    "metal_generated_count_accumulations":
        "mklq_sampling_phase_profile_metal_generated_count_accumulations",
}
OPTIONAL_PHASE_PROPERTY_NAMES = {
    "host_fold_seconds": "mklq_sampling_phase_profile_host_fold_seconds",
    "metal_probability_buffer_preparation_seconds":
        "mklq_sampling_phase_profile_metal_probability_buffer_preparation_seconds",
    "metal_probability_dispatch_seconds":
        "mklq_sampling_phase_profile_metal_probability_dispatch_seconds",
    "metal_probability_host_conversion_seconds":
        "mklq_sampling_phase_profile_metal_probability_host_conversion_seconds",
}


def repo_root() -> Path:
    return Path(__file__).resolve().parents[2]


def command_path(root: Path, path: Path) -> str:
    try:
        return path.resolve().relative_to(root.resolve()).as_posix()
    except ValueError:
        return "<external-local-path>"


def find_test_command(root: Path, build_dir: Path,
                      ctest_name: str) -> tuple[list[str], Path]:
    command = ["ctest", "--test-dir", str(build_dir), "--show-only=json-v1"]
    result = subprocess.run(command,
                            cwd=root,
                            capture_output=True,
                            text=True,
                            check=False)
    if result.returncode != 0:
        raise RuntimeError("unable to inspect build-tree CTest metadata: " +
                           result.stderr[-600:])
    try:
        tests = json.loads(result.stdout).get("tests", [])
    except json.JSONDecodeError as error:
        raise RuntimeError("CTest metadata is not valid JSON") from error
    for test in tests:
        if test.get("name") != ctest_name:
            continue
        test_command = test.get("command")
        if not isinstance(test_command, list) or not test_command:
            raise RuntimeError(f"CTest entry {ctest_name} has no command")
        working_directory = build_dir
        for property_ in test.get("properties", []):
            if property_.get("name") == "WORKING_DIRECTORY":
                working_directory = Path(property_["value"])
                break
        if not working_directory.is_absolute():
            working_directory = root / working_directory
        return [str(item) for item in test_command], working_directory
    raise RuntimeError(f"CTest entry {ctest_name} was not found")


def properties_from_testcase(testcase: element_tree.Element) -> dict[str, str]:
    properties: dict[str, str] = {}
    for property_ in testcase.findall("./properties/property"):
        name = property_.get("name")
        value = property_.get("value")
        if name is not None and value is not None:
            properties[name] = value
    return properties


def parse_positive_float(properties: dict[str, str], name: str) -> float:
    value = properties.get(name)
    if value is None:
        raise ValueError(f"missing gtest property {name}")
    try:
        parsed = float(value)
    except ValueError as error:
        raise ValueError(f"invalid gtest property {name}={value!r}") from error
    if not math.isfinite(parsed) or parsed <= 0.0:
        raise ValueError(f"gtest property {name} is not a positive duration")
    return parsed


def parse_nonnegative_int(properties: dict[str, str], name: str) -> int:
    value = properties.get(name)
    if value is None:
        raise ValueError(f"missing gtest property {name}")
    try:
        parsed = int(value)
    except ValueError as error:
        raise ValueError(f"invalid gtest property {name}={value!r}") from error
    if parsed < 0:
        raise ValueError(f"gtest property {name} is negative")
    return parsed


def parse_nonnegative_float(properties: dict[str, str], name: str) -> float:
    value = properties.get(name)
    if value is None:
        raise ValueError(f"missing gtest property {name}")
    try:
        parsed = float(value)
    except ValueError as error:
        raise ValueError(f"invalid gtest property {name}={value!r}") from error
    if not math.isfinite(parsed) or parsed < 0.0:
        raise ValueError(f"gtest property {name} is not a duration")
    return parsed


def parse_profile_xml(path: Path, target: str) -> dict[str, Any]:
    try:
        root = element_tree.parse(path).getroot()
    except (OSError, element_tree.ParseError) as error:
        raise ValueError(f"unable to parse gtest XML: {error}") from error

    cases = [case for case in root.iter("testcase")
             if case.get("name") == PROFILE_TEST_SUFFIX]
    if len(cases) != 1:
        raise ValueError("expected exactly one sampling phase profile testcase")
    testcase = cases[0]
    if testcase.find("failure") is not None or testcase.find("error") is not None:
        raise ValueError("sampling phase profile testcase failed")
    if testcase.find("skipped") is not None:
        raise ValueError("sampling phase profile testcase was skipped")

    properties = properties_from_testcase(testcase)
    if properties.get("mklq_sampling_phase_profile") != "true":
        raise ValueError("profile testcase did not mark native phase evidence")
    if properties.get("mklq_sampling_phase_profile_target") != target:
        raise ValueError("profile target property does not match invocation")

    phases = {
        key: parse_positive_float(properties, value)
        for key, value in PHASE_PROPERTY_NAMES.items()
    }
    result: dict[str, Any] = {
        "phase_seconds": phases,
        "qubits": parse_nonnegative_int(
            properties, "mklq_sampling_phase_profile_qubits"),
        "measured_qubits": parse_nonnegative_int(
            properties, "mklq_sampling_phase_profile_measured_qubits"),
        "shots": parse_nonnegative_int(
            properties, "mklq_sampling_phase_profile_shots"),
    }
    for key, name in OPTIONAL_PROPERTY_NAMES.items():
        if name in properties:
            result[key] = parse_nonnegative_int(properties, name)
    subphase_seconds = {
        key: parse_nonnegative_float(properties, name)
        for key, name in OPTIONAL_PHASE_PROPERTY_NAMES.items()
        if name in properties
    }
    if subphase_seconds:
        result["subphase_seconds"] = subphase_seconds
    return result


def validate_profile_config(profile: dict[str, Any], qubits: int,
                            measured_qubits: int, shots: int) -> None:
    expected = {
        "qubits": qubits,
        "measured_qubits": measured_qubits,
        "shots": shots,
    }
    for name, value in expected.items():
        if profile.get(name) != value:
            raise ValueError(
                f"gtest profile {name}={profile.get(name)!r}, expected {value!r}")


def run_sample(root: Path, command: list[str], working_directory: Path,
               gtest_filter: str, target: str, qubits: int,
               measured_qubits: int, shots: int) -> dict[str, Any]:
    with tempfile.TemporaryDirectory(prefix="mklq-sampling-phase-") as directory:
        xml_path = Path(directory) / "profile.xml"
        environment = os.environ.copy()
        environment.update({
            "MKLQ_ENABLE_SAMPLING_PHASE_PROFILE": "1",
            "MKLQ_SAMPLING_PHASE_PROFILE_QUBITS": str(qubits),
            "MKLQ_SAMPLING_PHASE_PROFILE_MEASURED_QUBITS":
                str(measured_qubits),
            "MKLQ_SAMPLING_PHASE_PROFILE_SHOTS": str(shots),
        })
        invocation = [*command, f"--gtest_filter={gtest_filter}",
                      f"--gtest_output=xml:{xml_path}"]
        start = time.perf_counter()
        completed = subprocess.run(invocation,
                                   cwd=working_directory,
                                   env=environment,
                                   capture_output=True,
                                   text=True,
                                   check=False)
        elapsed_seconds = time.perf_counter() - start
        if completed.returncode != 0:
            return {
                "status": "failed",
                "elapsed_seconds": elapsed_seconds,
                "failure_excerpt": {
                    "stdout_tail": completed.stdout[-1200:],
                    "stderr_tail": completed.stderr[-1200:],
                },
            }
        try:
            profile = parse_profile_xml(xml_path, target)
            validate_profile_config(profile, qubits, measured_qubits, shots)
        except ValueError as error:
            return {
                "status": "failed",
                "elapsed_seconds": elapsed_seconds,
                "failure_excerpt": {"parser_error": str(error)},
            }
        return {"status": "passed", "elapsed_seconds": elapsed_seconds,
                **profile}


def build_report(root: Path, build_dir: Path, target: str, qubits: int,
                 measured_qubits: int, shots: int, repeats: int) -> dict[str, Any]:
    if target not in TARGETS:
        raise ValueError(f"unsupported target {target!r}")
    if not 1 <= qubits <= 24:
        raise ValueError("qubits must be in [1, 24]")
    if not 1 <= measured_qubits <= qubits:
        raise ValueError("measured-qubits must be in [1, qubits]")
    if shots < 1:
        raise ValueError("shots must be positive")
    if repeats < 1:
        raise ValueError("repeats must be positive")

    root = root.resolve()
    build_dir = build_dir.resolve()
    target_info = TARGETS[target]
    command, working_directory = find_test_command(root, build_dir,
                                                    target_info["ctest_name"])
    samples = [
        run_sample(root, command, working_directory, target_info["gtest_filter"],
                   target, qubits, measured_qubits, shots)
        for _ in range(repeats)
    ]
    passed = [sample for sample in samples if sample["status"] == "passed"]
    phase_medians: dict[str, float] = {}
    if len(passed) == repeats:
        phase_medians = {
            phase: statistics.median(sample["phase_seconds"][phase]
                                     for sample in passed)
            for phase in PHASE_PROPERTY_NAMES
        }
    status = "passed" if len(passed) == repeats else "failed"
    return {
        "schema_version": SCHEMA_VERSION,
        "evidence_kind": EVIDENCE_KIND,
        "created_at_utc": datetime.now(timezone.utc).isoformat(),
        "machine": {
            "platform": platform.platform(),
            "machine": platform.machine(),
            "processor": platform.processor(),
        },
        "source": {
            "repo_root": ".",
            "build_dir": command_path(root, build_dir),
            "ctest_name": target_info["ctest_name"],
            "test_command": [command_path(root, Path(command[0])),
                             *command[1:]],
            "gtest_filter": target_info["gtest_filter"],
            "working_directory": command_path(root, working_directory),
        },
        "config": {
            "target": target,
            "qubits": qubits,
            "measured_qubits": measured_qubits,
            "shots": shots,
            "repeats": repeats,
        },
        "summary": {
            "status": status,
            "expected": repeats,
            "passed": len(passed),
            "failed": repeats - len(passed),
            "phase_seconds_median": phase_medians,
        },
        "boundary": {
            "native_backend_phase_timing": True,
            "local_machine_evidence": True,
            "performance_benchmark": False,
            "cross_machine_performance_proof": False,
            "release_signoff": False,
            "raw_logs_truncated": True,
        },
        "samples": samples,
        "execution": {"returncode": 0 if status == "passed" else 1},
    }


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Collect MKL-Q native sampling phase timing evidence.")
    parser.add_argument("--repo-root", type=Path, default=repo_root())
    parser.add_argument("--build-dir", type=Path, default=Path("build-python"))
    parser.add_argument("--target", choices=sorted(TARGETS), required=True)
    parser.add_argument("--qubits", type=int, default=20)
    parser.add_argument("--measured-qubits", type=int, default=12)
    parser.add_argument("--shots", type=int, default=65536)
    parser.add_argument("--repeats", type=int, default=3)
    parser.add_argument("--output", type=Path)
    return parser.parse_args(argv)


def main(argv: list[str]) -> int:
    args = parse_args(argv)
    root = args.repo_root.resolve()
    build_dir = args.build_dir if args.build_dir.is_absolute() else root / args.build_dir
    try:
        report = build_report(root, build_dir, args.target, args.qubits,
                              args.measured_qubits, args.shots, args.repeats)
    except (RuntimeError, ValueError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 2
    payload = json.dumps(report, indent=2, sort_keys=True) + "\n"
    if args.output:
        output = args.output if args.output.is_absolute() else root / args.output
        output.parent.mkdir(parents=True, exist_ok=True)
        output.write_text(payload, encoding="utf-8")
    else:
        sys.stdout.write(payload)
    return int(report["execution"]["returncode"])


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
