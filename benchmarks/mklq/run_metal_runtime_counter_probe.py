#!/usr/bin/env python3
# ============================================================================ #
# Copyright (c) 2026 Linsen Wu.                                                #
#                                                                              #
# This source code and the accompanying materials are made available under     #
# the terms of the Apache License 2.0 which accompanies this distribution.     #
# ============================================================================ #

"""Run build-tree Metal runtime counter tests and emit bounded evidence JSON."""

import argparse
import json
import platform
import re
import subprocess
import sys
import time
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


SCHEMA_VERSION = "mklq-metal-runtime-counter-probe-v1"
EVIDENCE_KIND = "local_runtime_counter_probe"
COUNTER_SOURCE = (
    "MetalStateVectorExecutor and MKL-Q simulator test-accessor runtime "
    "counters")

COUNTER_TEST_SUFFIXES = (
    "MetalRuntimeAppliesSingleQubitGate",
    "MetalRuntimeAppliesControlledSingleQubitGate",
    "MetalRuntimeAppliesTwoQubitGate",
    "MetalRuntimeAppliesControlledTwoQubitGate",
    "MetalRuntimeAppliesResidentThreeQubitGate",
    "MetalRuntimeFillsFullRegisterProbabilities",
    "MetalRuntimeProbabilityFillMatchesCpuNorms",
    "MetalRuntimeFillsResidentMarginalProbabilities",
    "MetalRuntimeComputesAndCollapsesResidentQubitProbability",
    "MetalRuntimeKeepsResidentStateAcrossGateSequence",
    "MetalRuntimeKeepsResidentYAndControlledYSequence",
    "MetalRuntimeFillsResidentProbabilitiesWithoutStateReadback",
    "MetalRuntimeAccumulatesSampleCounts",
    "MetalRuntimeGeneratesSampleCountsOnDevice",
    "MetalRuntimeGeneratesUniformSampleCountsOnDevice",
    "MetalRuntimeRejectsTargetsOutsideStateRange",
    "SimulatorUsesMetalFullRegisterProbabilityFill",
    "SimulatorKeepsSupportedGateSequenceResidentUntilReadback",
    "SimulatorKeepsYAndControlledYResidentUntilReadback",
    "SimulatorKeepsBuiltInYAndControlledYResidentUntilReadback",
    "SimulatorKeepsBuiltInRxAndControlledRxResidentUntilReadback",
    "SimulatorKeepsBuiltInRyAndControlledRyResidentUntilReadback",
    "SimulatorKeepsBuiltInRzAndControlledRzResidentUntilReadback",
    "SimulatorKeepsBuiltInPhaseFamilyResidentUntilReadback",
    "SimulatorKeepsMultiControlSingleQubitResidentUntilReadback",
    "SimulatorSamplesResidentDenseStateWithoutReadback",
    "SimulatorSamplesLargeResidentPartialRegisterThroughFullProbability",
    "SimulatorUsesAtomicMarginalForLargeUnorderedPartialRegister",
    "SimulatorFallsBackWhenAtomicMarginalPipelineIsDisabled",
    "SimulatorKeepsReorderedFullRegisterOnFullProbabilityRoute",
    "SimulatorSamplesSmallResidentPartialRegisterThroughMarginalProbability",
    "SimulatorSamplesRequestedOrderPartialRegisterThroughMarginalProbability",
    "SimulatorSamplesResidentPartialRegisterWithHostSequentialDrawTelemetry",
    "SimulatorSamplesResidentPartialRegisterCountsOnlyWithMetalAccumulation",
    "SimulatorSamplesResidentFullRegisterWithHostSequentialDrawTelemetry",
    "SimulatorSamplesResidentFullRegisterCountsOnlyWithMetalAccumulation",
    "SimulatorSamplesResidentDeterministicPartialRegisterCountsOnlyWithoutDrawLoop",
    "SimulatorSamplesResidentDeterministicPartialRegisterSequentialWithoutDrawLoop",
    "SimulatorSamplesResidentPartialRegisterReportsNativePhaseTiming",
    "SimulatorSamplesDeterministicSparseStateWithOneBitStringConversion",
    "SimulatorKeepsFourQubitGateResidentUntilReadback",
    "SimulatorKeepsControlledFourQubitGateResident",
    "SimulatorAppliesDenseFourQubitGateResident",
    "SimulatorKeepsThreeQubitGateResidentUntilReadback",
    "SimulatorKeepsBuiltInControlledSwapResidentUntilReadback",
    "SimulatorReuploadsResidentStateAfterFiveQubitGateFallback",
    "SimulatorMeasuresAndResetsResidentStateWithoutReadback",
    "SimulatorResetsResidentNonzeroTargetWithoutReadback",
    "SimulatorPoisonsResidentStateWhenSingleGateFails",
    "SimulatorPoisonsResidentStateWhenTwoGateFails",
    "SimulatorPoisonsResidentStateWhenThreeGateFails",
    "SimulatorPoisonsResidentStateWhenFourGateFails",
    "SimulatorThrowsWhenResidentMeasurementProbabilityFails",
    "SimulatorThrowsWhenResidentMeasurementCollapseFails",
    "SimulatorThrowsWhenResidentResetGateFails",
    "SimulatorComputesZeroShotZParityExpectationFromResidentState",
    "SimulatorPoisonsResidentStateWhenExpectationFlushFails",
    "SimulatorFallsBackToCpuWhenResidentExpectationReadFails",
    "SimulatorReducesLargeNonuniformZeroShotExpectationOnMetalWithoutProbabilityFill",
    "SimulatorSamplesDenseFullRegisterThroughMetalProbabilityFill",
)

TEST_PREFIX = "mklq_metal_MKLQMetalTester."
SINGLE_EXECUTABLE_TEST_NAME = "test_mklq_metal_backend"
TEST_LINE_RE = re.compile(r"Test\s+#\d+:\s+(\S+)")
TAIL_CHARS = 1200


def repo_root() -> Path:
    return Path(__file__).resolve().parents[2]


def command_output(cwd: Path, command: list[str]) -> str:
    return subprocess.check_output(command,
                                   cwd=cwd,
                                   text=True,
                                   stderr=subprocess.STDOUT)


def run_command(cwd: Path, command: list[str]) -> dict[str, Any]:
    start = time.perf_counter()
    result = subprocess.run(command,
                            cwd=cwd,
                            capture_output=True,
                            text=True)
    return {
        "returncode": result.returncode,
        "elapsed_seconds": time.perf_counter() - start,
        "stdout": result.stdout,
        "stderr": result.stderr,
    }


def output_tail(text: str | None, limit: int = TAIL_CHARS) -> str:
    return (text or "")[-limit:]


def command_path(root: Path, path: Path) -> str:
    try:
        return path.relative_to(root).as_posix()
    except ValueError:
        return "<external-local-path>"


def public_ctest_command(command: list[str], build_dir: str) -> list[str]:
    public = list(command)
    public[public.index("--test-dir") + 1] = build_dir
    return public


def select_counter_tests(ctest_listing: str) -> list[str]:
    found: set[str] = set()
    suffixes = set(COUNTER_TEST_SUFFIXES)
    for line in ctest_listing.splitlines():
        match = TEST_LINE_RE.search(line)
        if not match:
            continue
        test_name = match.group(1)
        if not test_name.startswith(TEST_PREFIX):
            continue
        suffix = test_name.removeprefix(TEST_PREFIX)
        if suffix in suffixes:
            found.add(test_name)
    return [test_name for test_name in expected_counter_tests() if test_name in found]


def expected_counter_tests() -> list[str]:
    return [TEST_PREFIX + suffix for suffix in COUNTER_TEST_SUFFIXES]


def missing_counter_tests(selected: list[str]) -> list[str]:
    selected_set = set(selected)
    return [
        test_name for test_name in expected_counter_tests()
        if test_name not in selected_set
    ]


def exact_ctest_regex(test_name: str) -> str:
    return f"^{re.escape(test_name)}$"


def single_executable_test_command(
        ctest_json: str) -> tuple[list[str], Path] | None:
    try:
        tests = json.loads(ctest_json).get("tests", [])
    except json.JSONDecodeError:
        return None
    for test in tests:
        if test.get("name") != SINGLE_EXECUTABLE_TEST_NAME:
            continue
        command = test.get("command")
        if not isinstance(command, list) or not command:
            return None
        working_directory = None
        for prop in test.get("properties", []):
            if prop.get("name") == "WORKING_DIRECTORY":
                working_directory = prop.get("value")
                break
        return command, Path(working_directory or ".")
    return None


def build_report(repo_root: Path, build_dir: Path) -> dict[str, Any]:
    repo_root = repo_root.resolve()
    build_dir = build_dir.resolve()
    listing_command = [
        "ctest",
        "--test-dir",
        str(build_dir),
        "-N",
        "-R",
        "mklq_metal_MKLQMetalTester",
    ]
    listing = command_output(repo_root, listing_command)
    public_build_dir = command_path(repo_root, build_dir)
    public_listing_command = public_ctest_command(listing_command,
                                                  public_build_dir)
    selected = select_counter_tests(listing)
    missing = missing_counter_tests(selected)
    execution_mode = "ctest-case"
    ctest_json_command = None
    executable_command = None
    executable_working_directory = repo_root
    if not selected:
        ctest_json_command = [
            "ctest",
            "--test-dir",
            str(build_dir),
            "--show-only=json-v1",
        ]
        standalone = single_executable_test_command(
            command_output(repo_root, ctest_json_command))
        if standalone:
            executable_command, executable_working_directory = standalone
            if not executable_working_directory.is_absolute():
                executable_working_directory = repo_root / executable_working_directory
            selected = expected_counter_tests()
            missing = []
            execution_mode = "ctest-executable-gtest-filter"

    tests: list[dict[str, Any]] = []
    probe_commands: list[list[str]] = []
    for test_name in selected:
        if execution_mode == "ctest-case":
            run_command_args = [
                "ctest",
                "--test-dir",
                str(build_dir),
                "-R",
                exact_ctest_regex(test_name),
                "--output-on-failure",
            ]
            probe_commands.append(public_ctest_command(run_command_args,
                                                       public_build_dir))
            run_result = run_command(repo_root, run_command_args)
            invocation = {"ctest_regex": exact_ctest_regex(test_name)}
        else:
            assert executable_command is not None
            run_command_args = [
                *executable_command,
                f"--gtest_filter={test_name}",
            ]
            probe_commands.append([
                command_path(repo_root, Path(executable_command[0])),
                *executable_command[1:],
                f"--gtest_filter={test_name}",
            ])
            run_result = run_command(executable_working_directory,
                                     run_command_args)
            invocation = {"gtest_filter": test_name}
        passed = run_result["returncode"] == 0
        item: dict[str, Any] = {
            "name": test_name,
            "status": "passed" if passed else "failed",
            "counter_source": COUNTER_SOURCE,
            "returncode": run_result["returncode"],
            **invocation,
        }
        if "elapsed_seconds" in run_result:
            item["elapsed_seconds"] = run_result["elapsed_seconds"]
        if not passed:
            item["failure_excerpt"] = {
                "stdout_tail": output_tail(run_result.get("stdout")),
                "stderr_tail": output_tail(run_result.get("stderr")),
            }
        tests.append(item)

    passed_count = sum(1 for item in tests if item["status"] == "passed")
    failed_count = sum(1 for item in tests if item["status"] == "failed")
    status = (
        "passed" if selected and not missing and failed_count == 0 else
        "failed")

    report: dict[str, Any] = {
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
            "build_dir": public_build_dir,
            "listing_command": public_listing_command,
            "ctest_json_command":
                (public_ctest_command(ctest_json_command, public_build_dir)
                 if ctest_json_command else None),
            "execution_mode": execution_mode,
            "probe_commands": probe_commands,
        },
        "summary": {
            "status": status,
            "expected": len(COUNTER_TEST_SUFFIXES),
            "selected": len(selected),
            "missing": len(missing),
            "passed": passed_count,
            "failed": failed_count,
        },
        "boundary": {
            "runtime_counter_evidence": True,
            "runtime_counter_source":
                f"build-tree counter cases that assert {COUNTER_SOURCE}",
            "release_signoff": False,
            "all_metal_execution_proof": False,
            "raw_logs_truncated": True,
        },
        "expected_counter_tests": expected_counter_tests(),
        "missing_counter_tests": missing,
        "tests": tests,
    }
    report["execution"] = {
        "returncode": 0 if status == "passed" else 1,
        "test_command_count": len(probe_commands),
    }
    return report


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Run MKL-Q Metal runtime counter ctests and write bounded JSON "
            "evidence."))
    parser.add_argument("--repo-root",
                        type=Path,
                        default=repo_root(),
                        help="Repository root. Defaults to this checkout.")
    parser.add_argument("--build-dir",
                        type=Path,
                        default=Path("build-python"),
                        help="CMake build directory containing ctest metadata.")
    parser.add_argument("--output",
                        type=Path,
                        help="Optional JSON output path.")
    return parser.parse_args(argv)


def main(argv: list[str]) -> int:
    args = parse_args(argv)
    root = args.repo_root.resolve()
    build_dir = args.build_dir
    if not build_dir.is_absolute():
        build_dir = root / build_dir
    report = build_report(repo_root=root, build_dir=build_dir)
    payload = json.dumps(report, indent=2, sort_keys=True) + "\n"
    if args.output:
        output = args.output if args.output.is_absolute() else root / args.output
        output.parent.mkdir(parents=True, exist_ok=True)
        output.write_text(payload, encoding="utf-8")
    else:
        sys.stdout.write(payload)
    return 0 if report["summary"]["status"] == "passed" else 1


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
