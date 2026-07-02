#!/usr/bin/env python3
# ============================================================================ #
# Copyright (c) 2026 Linsen Wu.                                                #
#                                                                              #
# This source code and the accompanying materials are made available under     #
# the terms of the Apache License 2.0 which accompanies this distribution.     #
# ============================================================================ #

"""Audit MKL-Q's self-hosted Apple Silicon CI readiness notes."""

import argparse
import json
import re
import subprocess
import sys
from dataclasses import dataclass
from datetime import date, datetime, timezone
from pathlib import Path
from typing import Any


SCHEMA_VERSION = "mklq-self-hosted-ci-audit-v1"
DOC_PATH = Path("docs/mklq/apple-silicon-ci.md")
LIGHTWEIGHT_WORKFLOW = ".github/workflows/mklq-public-hygiene.yml"
APPLE_SILICON_WORKFLOW = ".github/workflows/mklq-apple-silicon-ci.yml"
EXPECTED_WORKFLOWS = sorted((LIGHTWEIGHT_WORKFLOW, APPLE_SILICON_WORKFLOW))

REQUIRED_SECTIONS = (
    "Scope",
    "Runner Requirements",
    "Workflow Policy",
    "Validation Command",
    "Activation Checklist",
    "Failure Handling",
    "Security Boundary",
)

REQUIRED_TOKENS = (
    "self-hosted",
    "Apple Silicon",
    "macOS ARM64",
    "mklq-apple-silicon",
    "mklq-cpu",
    "mklq-metal",
    "source-only",
    "not release certification",
    "no secrets",
    "read-only",
    "permissions: contents: read",
    "timeout-minutes",
    "concurrency",
    "workflow_dispatch",
    "run_full_gate",
    "default false",
    "manual",
    "run_public_healthcheck.py --full --require-clean",
    "run_correctness_gate.py",
    "benchmarks/mklq/results/",
    "no tags",
    "no GitHub Releases",
    "no wheels",
    "not enable",
)

LIGHTWEIGHT_WORKFLOW_FORBIDDEN_LINES = (
    "runs-on: self-hosted",
    "runs-on: [self-hosted",
    "python3 benchmarks/mklq/run_public_healthcheck.py --full",
    "python3 benchmarks/mklq/run_correctness_gate.py",
    "cmake --build build-python --target install",
)

APPLE_WORKFLOW_REQUIRED_TOKENS = (
    "name: MKL-Q Apple Silicon correctness",
    "workflow_dispatch:",
    "run_full_gate:",
    "type: boolean",
    "default: false",
    "permissions:",
    "contents: read",
    "concurrency:",
    "cancel-in-progress: true",
    "if: ${{ inputs.run_full_gate == true }}",
    "runs-on: [self-hosted, macOS, ARM64, mklq-apple-silicon]",
    "timeout-minutes:",
    "fetch-depth: 0",
    "persist-credentials: false",
    "run_public_healthcheck.py",
    "--full",
    "--require-clean",
    "benchmarks/mklq/results/",
)

APPLE_WORKFLOW_FORBIDDEN_LINE_PATTERNS = (
    (re.compile(r"^\s*pull_request\s*:", re.MULTILINE), "pull_request:"),
    (re.compile(r"^\s*push\s*:", re.MULTILINE), "push:"),
    (re.compile(r"\bsecrets\."), "secrets."),
    (re.compile(r"upload-artifact"), "upload-artifact"),
    (re.compile(r"gh\s+release"), "gh release"),
    (re.compile(r"git\s+tag"), "git tag"),
    (re.compile(r"twine\s+upload"), "twine upload"),
    (re.compile(r"\.whl\b"), ".whl"),
)


@dataclass(frozen=True)
class AuditConfig:
    repo_root: Path
    output: Path


def repo_root() -> Path:
    return Path(__file__).resolve().parents[2]


def output_default(stamp: str) -> Path:
    return Path("benchmarks/mklq/results") / (
        f"self-hosted-ci-audit-{stamp}.json")


def command_path(root: Path, path: Path) -> str:
    try:
        return path.relative_to(root).as_posix()
    except ValueError:
        return path.as_posix()


def command_output(root: Path, args: list[str]) -> str:
    return subprocess.check_output(args, cwd=root, text=True).rstrip("\n")


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


def missing_tokens(text: str, tokens: tuple[str, ...]) -> list[str]:
    return [token for token in tokens if token not in text]


def forbidden_workflow_lines(text: str) -> list[dict[str, Any]]:
    matches: list[dict[str, Any]] = []
    for line_number, line in enumerate(text.splitlines(), start=1):
        for token in LIGHTWEIGHT_WORKFLOW_FORBIDDEN_LINES:
            if token in line:
                matches.append({
                    "line": line_number,
                    "token": token,
                    "text": line.strip(),
                })
    return matches


def forbidden_manual_workflow_lines(text: str) -> list[dict[str, Any]]:
    matches: list[dict[str, Any]] = []
    lines = text.splitlines()
    for pattern, label in APPLE_WORKFLOW_FORBIDDEN_LINE_PATTERNS:
        for match in pattern.finditer(text):
            line_number = text.count("\n", 0, match.start()) + 1
            line_text = lines[line_number - 1].strip() if line_number <= len(
                lines) else ""
            matches.append({
                "line": line_number,
                "token": label,
                "text": line_text,
            })
    return matches


def check_doc_exists(config: AuditConfig) -> dict[str, Any]:
    path = config.repo_root / DOC_PATH
    details = {"path": DOC_PATH.as_posix(), "exists": path.exists()}
    return passed("doc_exists", details) if path.exists() else failed(
        "doc_exists", "self-hosted CI readiness doc is missing", details)


def check_doc_sections(config: AuditConfig, text: str) -> dict[str, Any]:
    missing = [
        section for section in REQUIRED_SECTIONS
        if f"## {section}" not in text
    ]
    details = {
        "path": DOC_PATH.as_posix(),
        "required_sections": list(REQUIRED_SECTIONS),
        "missing": missing,
    }
    return failed("doc_sections", "required self-hosted CI sections missing",
                  details) if missing else passed("doc_sections", details)


def check_doc_tokens(config: AuditConfig, text: str) -> dict[str, Any]:
    missing = missing_tokens(text, REQUIRED_TOKENS)
    details = {
        "path": DOC_PATH.as_posix(),
        "required": list(REQUIRED_TOKENS),
        "missing": missing,
    }
    return failed("doc_tokens", "required self-hosted CI boundary text missing",
                  details) if missing else passed("doc_tokens", details)


def check_workflow_boundary(config: AuditConfig) -> dict[str, Any]:
    workflows = command_output(config.repo_root,
                               ["git", "ls-files",
                                ".github/workflows"]).splitlines()
    unexpected = [path for path in workflows if path not in EXPECTED_WORKFLOWS]
    missing = [path for path in EXPECTED_WORKFLOWS if path not in workflows]

    lightweight_path = config.repo_root / LIGHTWEIGHT_WORKFLOW
    lightweight_forbidden: list[dict[str, Any]] = []
    if lightweight_path.exists():
        lightweight_text = lightweight_path.read_text(encoding="utf-8",
                                                      errors="replace")
        lightweight_forbidden = forbidden_workflow_lines(lightweight_text)

    manual_path = config.repo_root / APPLE_SILICON_WORKFLOW
    manual_missing_tokens: list[str] = []
    manual_forbidden: list[dict[str, Any]] = []
    if manual_path.exists():
        manual_text = manual_path.read_text(encoding="utf-8", errors="replace")
        manual_missing_tokens = missing_tokens(manual_text,
                                               APPLE_WORKFLOW_REQUIRED_TOKENS)
        manual_forbidden = forbidden_manual_workflow_lines(manual_text)

    details = {
        "workflows": workflows,
        "expected_workflows": EXPECTED_WORKFLOWS,
        "unexpected_workflows": unexpected,
        "missing_workflows": missing,
        "lightweight_workflow": LIGHTWEIGHT_WORKFLOW,
        "manual_workflow": APPLE_SILICON_WORKFLOW,
        "lightweight_forbidden_lines": lightweight_forbidden,
        "manual_missing_tokens": manual_missing_tokens,
        "manual_forbidden_lines": manual_forbidden,
    }
    failures = unexpected + missing + lightweight_forbidden + manual_missing_tokens
    failures += manual_forbidden
    return failed(
        "workflow_boundary",
        "Apple Silicon correctness workflow boundary is not manual/source-only",
        details) if failures else passed("workflow_boundary", details)


def build_report(config: AuditConfig) -> dict[str, Any]:
    root = config.repo_root.resolve()
    resolved = AuditConfig(repo_root=root, output=config.output)
    doc = root / DOC_PATH
    text = doc.read_text(encoding="utf-8") if doc.exists() else ""
    checks = [
        check_doc_exists(resolved),
        check_doc_sections(resolved, text),
        check_doc_tokens(resolved, text),
        check_workflow_boundary(resolved),
    ]
    return {
        "schema_version": SCHEMA_VERSION,
        "created_at_utc": datetime.now(timezone.utc).isoformat(),
        "repo_root": root.as_posix(),
        "doc": command_path(root, doc),
        "summary": summarize(checks),
        "checks": checks,
    }


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Audit MKL-Q self-hosted Apple Silicon CI readiness.")
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
