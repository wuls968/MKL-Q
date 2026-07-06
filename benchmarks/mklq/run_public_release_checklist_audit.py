#!/usr/bin/env python3
# ============================================================================ #
# Copyright (c) 2026 Linsen Wu.                                                #
#                                                                              #
# This source code and the accompanying materials are made available under     #
# the terms of the Apache License 2.0 which accompanies this distribution.     #
# ============================================================================ #

"""Audit the public release checklist against source-only MKL-Q gates."""

import argparse
import json
import sys
from dataclasses import dataclass
from datetime import date, datetime, timezone
from pathlib import Path
from typing import Any


SCHEMA_VERSION = "mklq-public-release-checklist-audit-v1"
CHECKLIST_PATH = Path("docs/mklq/public-release-checklist.md")
DEVELOPER_WORKFLOW_PATH = Path("docs/mklq/developer-workflow.md")

REQUIRED_SECTIONS = (
    "Scope",
    "Git And Remotes",
    "Public Metadata",
    "Tree Hygiene",
    "Local Build Gate",
    "Correctness Gate",
    "Benchmark Evidence",
    "Public Hygiene Gate",
    "Push And GitHub Verification",
    "Stop Conditions",
)

REQUIRED_COMMAND_TOKENS = (
    "git status --short --branch",
    "git remote -v",
    "git rev-parse --is-shallow-repository",
    "python3 benchmarks/mklq/run_upstream_sync_audit.py",
    "python3 benchmarks/mklq/run_preflight_audit.py --require-clean",
    "git status --ignored --short",
    "git ls-files .github | sort",
    "git diff --check",
    "cmake --build build-python --target install -j 6",
    "python3 benchmarks/mklq/repair_macos_install_signatures.py",
    "python3 benchmarks/mklq/run_correctness_gate.py",
    "python3 benchmarks/mklq/run_clean_cpu_benchmark.py",
    "python3 benchmarks/mklq/run_public_release_checklist_audit.py",
    "python3 benchmarks/mklq/run_source_release_tag_audit.py",
    "python3 benchmarks/mklq/run_source_release_tag_audit.py --docs-only",
    "python3 benchmarks/mklq/run_public_healthcheck.py",
    "python3 benchmarks/mklq/run_public_healthcheck.py --full --require-clean",
    "python3 benchmarks/mklq/run_self_hosted_ci_audit.py",
    "python3 benchmarks/mklq/run_self_hosted_ci_audit.py --check-runners --repo wuls968/MKL-Q",
    "python3 benchmarks/mklq/check_performance_evidence.py",
    "python3 benchmarks/mklq/check_metal_evidence.py",
    "python3 benchmarks/mklq/check_metal_sampling_boundary_evidence.py",
    "python3 benchmarks/mklq/check_metal_uniform_sampling_evidence.py",
    "python3 benchmarks/mklq/check_public_claims.py",
    "python3 benchmarks/mklq/check_sampling_profile_evidence.py",
    "python3 benchmarks/mklq/check_cpu_gate_counter_docs.py",
    "python3 benchmarks/mklq/check_cpu_sampling_counter_docs.py",
    "python3 benchmarks/mklq/check_metal_runtime_counter_docs.py",
    "gh pr checks --repo wuls968/MKL-Q --watch",
    "git ls-remote origin refs/heads/main",
    "gh repo view wuls968/MKL-Q",
    "gh run list --repo wuls968/MKL-Q --branch main",
    "python3 benchmarks/mklq/run_public_readiness_audit.py",
)

REQUIRED_REFERENCED_FILES = (
    "README.md",
    "docs/mklq/release-policy.md",
    "docs/mklq/source-only-rc-v0.1.md",
    "docs/mklq/release-notes-v0.1.0-source.md",
    "CHANGELOG.md",
    "docs/mklq/public-readiness.md",
    "docs/mklq/upstream-sync.md",
    "docs/mklq/validation.md",
    "docs/mklq/known-limitations.md",
    "docs/mklq/testing-matrix.md",
    "docs/mklq/apple-silicon-ci.md",
    "docs/mklq/developer-workflow.md",
    "docs/mklq/maintainer-runbook.md",
    "docs/mklq/branch-protection.md",
    "docs/mklq/issue-labels.md",
    ".github/workflows/mklq-apple-silicon-ci.yml",
    ".github/workflows/mklq-public-hygiene.yml",
    "benchmarks/mklq/run_preflight_audit.py",
    "benchmarks/mklq/run_upstream_sync_audit.py",
    "benchmarks/mklq/run_public_release_checklist_audit.py",
    "benchmarks/mklq/run_source_release_tag_audit.py",
    "benchmarks/mklq/run_public_healthcheck.py",
    "benchmarks/mklq/run_public_readiness_audit.py",
    "benchmarks/mklq/repair_macos_install_signatures.py",
    "benchmarks/mklq/run_self_hosted_ci_audit.py",
    "benchmarks/mklq/run_correctness_gate.py",
    "benchmarks/mklq/run_clean_cpu_benchmark.py",
    "benchmarks/mklq/run_cpu_gate_counter_probe.py",
    "benchmarks/mklq/summarize_cpu_gate_counters.py",
    "benchmarks/mklq/check_performance_evidence.py",
    "benchmarks/mklq/check_metal_evidence.py",
    "benchmarks/mklq/check_metal_sampling_boundary_evidence.py",
    "benchmarks/mklq/check_metal_uniform_sampling_evidence.py",
    "benchmarks/mklq/check_public_claims.py",
    "benchmarks/mklq/check_sampling_profile_evidence.py",
    "benchmarks/mklq/check_cpu_gate_counter_docs.py",
    "benchmarks/mklq/check_cpu_sampling_counter_docs.py",
    "benchmarks/mklq/check_metal_runtime_counter_docs.py",
)

SOURCE_ONLY_REQUIRED_TOKENS = (
    "source-only",
    "not a wheel",
    "PyPI",
    "GitHub Release",
    "release certification",
    "performance certification",
    "mklq-metal",
    "experimental",
    "full Metal-native",
    "default-ready",
)

PREFLIGHT_REFERENCE_REQUIRED_TOKENS = (
    "public_report_references",
    "workflows",
    "untracked report files",
    "benchmarks/mklq/reports/*.json",
)

HEALTHCHECK_INTEGRATION_FILES = (
    "benchmarks/mklq/run_public_healthcheck.py",
    ".github/workflows/mklq-apple-silicon-ci.yml",
    ".github/workflows/mklq-public-hygiene.yml",
)

DEVELOPER_WORKFLOW_REQUIRED_TOKENS = (
    "python3 benchmarks/mklq/run_preflight_audit.py",
    "python3 benchmarks/mklq/run_public_release_checklist_audit.py",
    "python3 benchmarks/mklq/run_source_release_tag_audit.py --docs-only",
    "python3 benchmarks/mklq/run_public_healthcheck.py",
    "python3 benchmarks/mklq/run_self_hosted_ci_audit.py",
    "python3 benchmarks/mklq/repair_macos_install_signatures.py",
    "git diff --check",
    "git ls-files .github/workflows | sort",
    "python3 benchmarks/mklq/check_public_claims.py",
    "python3 benchmarks/mklq/check_performance_evidence.py",
    "python3 benchmarks/mklq/check_metal_evidence.py",
    "python3 benchmarks/mklq/check_metal_sampling_boundary_evidence.py",
    "python3 benchmarks/mklq/check_metal_uniform_sampling_evidence.py",
    "python3 benchmarks/mklq/check_sampling_profile_evidence.py",
    "python3 benchmarks/mklq/check_cpu_gate_counter_docs.py",
    "python3 benchmarks/mklq/check_cpu_sampling_counter_docs.py",
    "python3 benchmarks/mklq/check_metal_runtime_counter_docs.py",
    "benchmarks/mklq/run_cpu_scaling_benchmark.py",
    "benchmarks/mklq/run_sampling_scaling_benchmark.py",
    "benchmarks/mklq/repair_macos_install_signatures.py",
    "benchmarks/mklq/run_upstream_sync_audit.py",
    "benchmarks/mklq/run_source_release_tag_audit.py",
    "benchmarks/mklq/summarize_cpu_gate_counters.py",
    "benchmarks/mklq/summarize_cpu_sampling_counters.py",
    "benchmarks/mklq/summarize_metal_runtime_counters.py",
    "multiple bounded reports are tracked",
    "selected counter tests once per report",
    "public_report_references",
    "workflows",
    "untracked report files",
    "self-hosted Apple Silicon CI",
    "mklq-apple-silicon-ci.yml",
)


@dataclass(frozen=True)
class AuditConfig:
    repo_root: Path
    output: Path


def repo_root() -> Path:
    return Path(__file__).resolve().parents[2]


def output_default(stamp: str) -> Path:
    return Path("benchmarks/mklq/results") / (
        f"public-release-checklist-audit-{stamp}.json")


def passed(name: str, details: dict[str, Any] | None = None) -> dict[str, Any]:
    return {
        "name": name,
        "status": "passed",
        "details": details or {},
    }


def failed(name: str, message: str,
           details: dict[str, Any] | None = None) -> dict[str, Any]:
    payload: dict[str, Any] = {
        "name": name,
        "status": "failed",
        "message": message,
    }
    if details:
        payload["details"] = details
    return payload


def summarize(checks: list[dict[str, Any]]) -> dict[str, Any]:
    failed_count = sum(1 for check in checks if check["status"] == "failed")
    return {
        "status": "failed" if failed_count else "passed",
        "passed": len(checks) - failed_count,
        "failed": failed_count,
    }


def command_path(root: Path, path: Path) -> str:
    try:
        return path.relative_to(root).as_posix()
    except ValueError:
        return path.as_posix()


def checklist_path(root: Path) -> Path:
    return root / CHECKLIST_PATH


def read_checklist(root: Path) -> str:
    return checklist_path(root).read_text(encoding="utf-8")


def normalize_whitespace(text: str) -> str:
    return " ".join(text.split())


def missing_tokens(text: str, tokens: tuple[str, ...]) -> list[str]:
    normalized_text = normalize_whitespace(text)
    return [
        token for token in tokens
        if normalize_whitespace(token) not in normalized_text
    ]


def check_checklist_structure(config: AuditConfig, text: str) -> dict[str, Any]:
    missing = [
        section for section in REQUIRED_SECTIONS
        if f"## {section}" not in text
    ]
    details = {
        "checklist": CHECKLIST_PATH.as_posix(),
        "required_sections": list(REQUIRED_SECTIONS),
        "missing": missing,
    }
    return failed("checklist_structure", "required checklist sections missing",
                  details) if missing else passed("checklist_structure", details)


def check_checklist_commands(config: AuditConfig, text: str) -> dict[str, Any]:
    missing = missing_tokens(text, REQUIRED_COMMAND_TOKENS)
    details = {
        "required": list(REQUIRED_COMMAND_TOKENS),
        "missing": missing,
    }
    return failed("checklist_commands", "required release commands missing",
                  details) if missing else passed("checklist_commands", details)


def check_referenced_files(config: AuditConfig, text: str) -> dict[str, Any]:
    missing_from_text = missing_tokens(text, REQUIRED_REFERENCED_FILES)
    missing_from_tree = [
        path for path in REQUIRED_REFERENCED_FILES
        if not (config.repo_root / path).exists()
    ]
    details = {
        "required": list(REQUIRED_REFERENCED_FILES),
        "missing_from_text": missing_from_text,
        "missing_from_tree": missing_from_tree,
    }
    failures = missing_from_text + missing_from_tree
    return failed("referenced_files", "required files are missing from checklist or tree",
                  details) if failures else passed("referenced_files", details)


def check_source_only_boundaries(config: AuditConfig,
                                 text: str) -> dict[str, Any]:
    missing = missing_tokens(text, SOURCE_ONLY_REQUIRED_TOKENS)
    details = {
        "required": list(SOURCE_ONLY_REQUIRED_TOKENS),
        "missing": missing,
    }
    return failed("source_only_boundaries",
                  "source-only and experimental-target boundary text missing",
                  details) if missing else passed("source_only_boundaries",
                                                 details)


def check_preflight_reference_boundaries(config: AuditConfig,
                                         text: str) -> dict[str, Any]:
    missing = missing_tokens(text, PREFLIGHT_REFERENCE_REQUIRED_TOKENS)
    details = {
        "required": list(PREFLIGHT_REFERENCE_REQUIRED_TOKENS),
        "missing": missing,
    }
    return failed(
        "preflight_reference_boundaries",
        "preflight public report-reference boundary text missing",
        details) if missing else passed("preflight_reference_boundaries",
                                        details)


def check_healthcheck_integration(config: AuditConfig,
                                  text: str) -> dict[str, Any]:
    missing_files = [
        path for path in HEALTHCHECK_INTEGRATION_FILES
        if not (config.repo_root / path).exists()
    ]
    missing_tokens_in_checklist = missing_tokens(
        text,
        (
            "run_public_healthcheck.py",
            "run_public_readiness_audit.py",
            "run_self_hosted_ci_audit.py",
            "mklq-apple-silicon-ci.yml",
            "Dispatch guard",
            "mklq-public-hygiene.yml",
        ),
    )
    details = {
        "required_files": list(HEALTHCHECK_INTEGRATION_FILES),
        "missing_files": missing_files,
        "missing_tokens_in_checklist": missing_tokens_in_checklist,
    }
    failures = missing_files + missing_tokens_in_checklist
    return failed("healthcheck_integration",
                  "checklist is not tied to healthcheck/readiness gates",
                  details) if failures else passed("healthcheck_integration",
                                                   details)


def check_developer_workflow_commands(config: AuditConfig) -> dict[str, Any]:
    path = config.repo_root / DEVELOPER_WORKFLOW_PATH
    details: dict[str, Any] = {
        "path": DEVELOPER_WORKFLOW_PATH.as_posix(),
        "required": list(DEVELOPER_WORKFLOW_REQUIRED_TOKENS),
    }
    if not path.exists():
        details["missing"] = list(DEVELOPER_WORKFLOW_REQUIRED_TOKENS)
        return failed("developer_workflow_commands",
                      "developer workflow document is missing", details)
    text = path.read_text(encoding="utf-8")
    missing = missing_tokens(text, DEVELOPER_WORKFLOW_REQUIRED_TOKENS)
    details["missing"] = missing
    return failed("developer_workflow_commands",
                  "developer workflow public hygiene commands are missing",
                  details) if missing else passed("developer_workflow_commands",
                                                 details)


def build_report(config: AuditConfig) -> dict[str, Any]:
    root = config.repo_root.resolve()
    checklist = checklist_path(root)
    if not checklist.exists():
        checks = [
            failed("checklist_structure", "public release checklist is missing",
                   {"checklist": CHECKLIST_PATH.as_posix()}),
            failed("checklist_commands", "public release checklist is missing"),
            failed("referenced_files", "public release checklist is missing"),
            failed("source_only_boundaries", "public release checklist is missing"),
            failed("preflight_reference_boundaries",
                   "public release checklist is missing"),
            failed("healthcheck_integration", "public release checklist is missing"),
            check_developer_workflow_commands(config),
        ]
    else:
        text = read_checklist(root)
        resolved_config = AuditConfig(repo_root=root, output=config.output)
        checks = [
            check_checklist_structure(resolved_config, text),
            check_checklist_commands(resolved_config, text),
            check_referenced_files(resolved_config, text),
            check_source_only_boundaries(resolved_config, text),
            check_preflight_reference_boundaries(resolved_config, text),
            check_healthcheck_integration(resolved_config, text),
            check_developer_workflow_commands(resolved_config),
        ]

    return {
        "schema_version": SCHEMA_VERSION,
        "created_at_utc": datetime.now(timezone.utc).isoformat(),
        "repo_root": root.as_posix(),
        "checklist": command_path(root, checklist),
        "summary": summarize(checks),
        "checks": checks,
    }


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Audit MKL-Q public release checklist coverage.")
    parser.add_argument("--repo-root",
                        type=Path,
                        default=repo_root(),
                        help="Repository root. Defaults to this checkout.")
    parser.add_argument("--output",
                        type=Path,
                        help="Optional JSON output path.")
    return parser.parse_args(argv)


def main(argv: list[str]) -> int:
    args = parse_args(argv)
    root = args.repo_root.resolve()
    stamp = date.today().isoformat()
    output = args.output or output_default(stamp)
    output = output if output.is_absolute() else root / output
    config = AuditConfig(repo_root=root, output=output)
    report = build_report(config)
    payload = json.dumps(report, indent=2, sort_keys=True) + "\n"
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(payload, encoding="utf-8")
    sys.stdout.write(payload)
    return 0 if report["summary"]["status"] == "passed" else 1


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
