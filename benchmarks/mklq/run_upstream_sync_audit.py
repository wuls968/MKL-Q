#!/usr/bin/env python3
# ============================================================================ #
# Copyright (c) 2026 Linsen Wu.                                                #
#                                                                              #
# This source code and the accompanying materials are made available under     #
# the terms of the Apache License 2.0 which accompanies this distribution.     #
# ============================================================================ #

"""Dry-run audit for future MKL-Q syncs from NVIDIA CUDA-Q upstream."""

import argparse
import json
import subprocess
import sys
from dataclasses import dataclass
from datetime import date, datetime, timezone
from pathlib import Path
from typing import Any


SCHEMA_VERSION = "mklq-upstream-sync-audit-v1"
EXPECTED_ORIGIN = "https://github.com/wuls968/MKL-Q.git"
EXPECTED_UPSTREAM = "https://github.com/NVIDIA/cuda-quantum.git"
UPSTREAM_SYNC_DOC = Path("docs/mklq/upstream-sync.md")

REQUIRED_DOC_TOKENS = (
    "Preflight",
    "Inspect Upstream Delta",
    "Sync Branch",
    "Conflict Rules",
    "Post-merge Gates",
    "Stop Conditions",
    "git fetch upstream main",
    "git merge --no-ff upstream/main",
    "git merge --abort",
    "run_upstream_sync_audit.py",
    "run_public_healthcheck.py",
    "run_correctness_gate.py",
    "run_public_release_checklist_audit.py",
    ".github/workflows/mklq-public-hygiene.yml",
    "mklq-metal",
)

RISK_CATEGORIES = (
    ("github_automation", (
        ".github/",
    )),
    ("public_metadata", (
        "README.md",
        "CITATION.cff",
        "Contributing.md",
        "SECURITY.md",
        "NOTICE",
        "LICENSE",
    )),
    ("runtime_and_targets", (
        "runtime/nvqir/",
        "targettests/TargetConfig/",
        "python/tests/backends/test_mklq",
        "python/tests/builder/test_mklq",
        "python/tests/mklq_test_utils.py",
        "unittests/nvqpp/backends/",
    )),
    ("build_system", (
        "CMakeLists.txt",
        "cmake/",
        "scripts/",
    )),
    ("mklq_docs_and_evidence", (
        "benchmarks/mklq/",
        "docs/mklq/",
        "examples/mklq/",
    )),
)


@dataclass(frozen=True)
class AuditConfig:
    repo_root: Path
    output: Path
    require_clean: bool
    check_remote: bool
    log_limit: int = 80


def repo_root() -> Path:
    return Path(__file__).resolve().parents[2]


def output_default(stamp: str) -> Path:
    return Path("benchmarks/mklq/results") / (
        f"upstream-sync-audit-{stamp}.json")


def command_output(cwd: Path, command: list[str]) -> str:
    return subprocess.check_output(command,
                                   cwd=cwd,
                                   text=True,
                                   stderr=subprocess.STDOUT).rstrip("\n")


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


def has_fetch_remote(remotes: list[str], name: str, url: str) -> bool:
    return any(line.startswith(f"{name}\t{url} (fetch)")
               for line in remotes)


def relative_path(root: Path, path: Path) -> str:
    try:
        return path.relative_to(root).as_posix()
    except ValueError:
        return path.as_posix()


def check_git_topology(config: AuditConfig) -> dict[str, Any]:
    try:
        status = command_output(config.repo_root,
                                ["git", "status", "--short", "--branch"])
        remotes = command_output(config.repo_root, ["git", "remote", "-v"])
        shallow = command_output(config.repo_root, [
            "git", "rev-parse", "--is-shallow-repository"
        ])
    except (FileNotFoundError, subprocess.CalledProcessError) as exc:
        return failed("git_topology", "git topology commands failed",
                      {"error": str(exc)})

    status_lines = status.splitlines()
    remote_lines = remotes.splitlines()
    dirty = [line for line in status_lines if not line.startswith("##")]
    failures: list[str] = []
    if config.require_clean and dirty:
        failures.append("working tree is dirty")
    if shallow.strip() != "false":
        failures.append("repository is shallow")
    if not has_fetch_remote(remote_lines, "origin", EXPECTED_ORIGIN):
        failures.append("origin remote is missing or unexpected")
    if not has_fetch_remote(remote_lines, "upstream", EXPECTED_UPSTREAM):
        failures.append("upstream remote is missing or unexpected")

    details = {
        "status_short_branch": status_lines,
        "is_shallow": shallow.strip(),
        "remotes": remote_lines,
        "expected_origin": EXPECTED_ORIGIN,
        "expected_upstream": EXPECTED_UPSTREAM,
        "require_clean": config.require_clean,
    }
    return failed("git_topology", "; ".join(failures),
                  details) if failures else passed("git_topology", details)


def check_local_refs(config: AuditConfig) -> dict[str, Any]:
    commands = {
        "main": ["git", "rev-parse", "main"],
        "origin_main": ["git", "rev-parse", "origin/main"],
        "upstream_main": ["git", "rev-parse", "upstream/main"],
        "merge_base": ["git", "merge-base", "main", "upstream/main"],
    }
    refs: dict[str, str] = {}
    failures: list[str] = []
    for name, command in commands.items():
        try:
            refs[name] = command_output(config.repo_root, command).strip()
        except (FileNotFoundError, subprocess.CalledProcessError) as exc:
            refs[name] = ""
            failures.append(f"{name}: {exc}")

    return failed("local_refs", "required local refs are missing",
                  refs | {"failures": failures}) if failures else passed(
                      "local_refs", refs)


def remote_sha_from_ls_remote(output: str) -> str:
    if not output.strip():
        return ""
    return output.split()[0]


def check_remote_freshness(config: AuditConfig) -> dict[str, Any]:
    try:
        local = command_output(config.repo_root,
                               ["git", "rev-parse", "upstream/main"]).strip()
        remote_output = command_output(config.repo_root, [
            "git", "ls-remote", "upstream", "refs/heads/main"
        ])
    except (FileNotFoundError, subprocess.CalledProcessError) as exc:
        return failed("remote_freshness", "remote upstream check failed",
                      {"error": str(exc)})

    remote = remote_sha_from_ls_remote(remote_output)
    details = {
        "local_upstream_main": local,
        "remote_upstream_main": remote,
    }
    if not remote:
        return failed("remote_freshness", "remote upstream/main was not found",
                      details)
    if local != remote:
        return failed("remote_freshness",
                      "local upstream/main is stale; run git fetch upstream main",
                      details)
    return passed("remote_freshness", details)


def parse_left_right_counts(text: str) -> tuple[int, int]:
    parts = text.strip().split()
    if len(parts) != 2:
        raise ValueError(f"expected two rev-list counts, got {text!r}")
    return int(parts[0]), int(parts[1])


def parse_name_status(text: str) -> list[dict[str, str]]:
    rows: list[dict[str, str]] = []
    for line in text.splitlines():
        if not line.strip():
            continue
        parts = line.split("\t")
        status = parts[0]
        if status.startswith("R") or status.startswith("C"):
            old_path = parts[1] if len(parts) > 1 else ""
            path = parts[2] if len(parts) > 2 else old_path
            rows.append({"status": status, "path": path, "old_path": old_path})
            continue
        path = parts[1] if len(parts) > 1 else ""
        rows.append({"status": status, "path": path})
    return rows


def check_sync_delta(config: AuditConfig) -> tuple[dict[str, Any],
                                                  list[dict[str, str]]]:
    try:
        counts_text = command_output(config.repo_root, [
            "git", "rev-list", "--left-right", "--count",
            "main...upstream/main"
        ])
        local_only, upstream_only = parse_left_right_counts(counts_text)
        diff_text = command_output(config.repo_root, [
            "git", "diff", "--name-status", "main...upstream/main"
        ])
        log_text = command_output(config.repo_root, [
            "git", "log", "--oneline", "--left-right",
            "main...upstream/main"
        ])
    except (FileNotFoundError, subprocess.CalledProcessError, ValueError) as exc:
        return failed("sync_delta", "could not compute upstream delta",
                      {"error": str(exc)}), []

    changed = parse_name_status(diff_text)
    details = {
        "local_only_commits": local_only,
        "upstream_only_commits": upstream_only,
        "changed_file_count": len(changed),
        "changed_files": changed[:config.log_limit],
        "changed_files_truncated": len(changed) > config.log_limit,
        "log_excerpt": log_text.splitlines()[:config.log_limit],
    }
    return passed("sync_delta", details), changed


def category_for_path(path: str) -> str:
    for category, prefixes in RISK_CATEGORIES:
        if any(path == prefix.rstrip("/") or path.startswith(prefix)
               for prefix in prefixes):
            return category
    return "unclassified"


def check_risk_classification(changed: list[dict[str, str]]) -> dict[str, Any]:
    categories: dict[str, dict[str, Any]] = {
        category: {
            "count": 0,
            "paths": [],
            "manual_review_required": category in {
                "github_automation",
                "public_metadata",
                "runtime_and_targets",
                "build_system",
                "mklq_docs_and_evidence",
            },
        } for category, _ in RISK_CATEGORIES
    }
    categories["unclassified"] = {
        "count": 0,
        "paths": [],
        "manual_review_required": False,
    }

    for row in changed:
        path = row.get("path", "")
        category = category_for_path(path)
        categories[category]["count"] += 1
        categories[category]["paths"].append(path)

    for payload in categories.values():
        payload["paths"] = payload["paths"][:80]

    return passed("risk_classification", {
        "categories": categories,
        "changed_file_count": len(changed),
        "note": "Risk categories are advisory; nonzero counts require review but do not fail the audit.",
    })


def check_sync_docs(config: AuditConfig) -> dict[str, Any]:
    path = config.repo_root / UPSTREAM_SYNC_DOC
    if not path.exists():
        return failed("sync_docs", "upstream sync documentation is missing",
                      {"path": UPSTREAM_SYNC_DOC.as_posix()})
    text = path.read_text(encoding="utf-8")
    missing = [token for token in REQUIRED_DOC_TOKENS if token not in text]
    details = {
        "path": UPSTREAM_SYNC_DOC.as_posix(),
        "required": list(REQUIRED_DOC_TOKENS),
        "missing": missing,
    }
    return failed("sync_docs", "upstream sync documentation is incomplete",
                  details) if missing else passed("sync_docs", details)


def build_report(config: AuditConfig) -> dict[str, Any]:
    root = config.repo_root.resolve()
    resolved = AuditConfig(repo_root=root,
                           output=config.output,
                           require_clean=config.require_clean,
                           check_remote=config.check_remote,
                           log_limit=config.log_limit)
    delta_check, changed = check_sync_delta(resolved)
    checks = [
        check_git_topology(resolved),
        check_local_refs(resolved),
    ]
    if resolved.check_remote:
        checks.append(check_remote_freshness(resolved))
    checks.extend([
        delta_check,
        check_risk_classification(changed),
        check_sync_docs(resolved),
    ])
    return {
        "schema_version": SCHEMA_VERSION,
        "created_at_utc": datetime.now(timezone.utc).isoformat(),
        "repo_root": root.as_posix(),
        "config": {
            "require_clean": resolved.require_clean,
            "check_remote": resolved.check_remote,
            "log_limit": resolved.log_limit,
            "output": relative_path(root, resolved.output),
        },
        "summary": summarize(checks),
        "checks": checks,
    }


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Dry-run audit MKL-Q upstream sync readiness.")
    parser.add_argument("--repo-root",
                        type=Path,
                        default=repo_root(),
                        help="Repository root. Defaults to this checkout.")
    parser.add_argument("--output",
                        type=Path,
                        help="Optional JSON output path.")
    parser.add_argument("--require-clean",
                        action="store_true",
                        help="Fail if the worktree has uncommitted changes.")
    parser.add_argument("--check-remote",
                        action="store_true",
                        help="Compare local upstream/main with the live upstream remote.")
    parser.add_argument("--log-limit",
                        type=int,
                        default=80,
                        help="Maximum changed-file and log rows to keep in JSON.")
    return parser.parse_args(argv)


def main(argv: list[str]) -> int:
    args = parse_args(argv)
    root = args.repo_root.resolve()
    stamp = date.today().isoformat()
    output = args.output or output_default(stamp)
    output = output if output.is_absolute() else root / output
    config = AuditConfig(repo_root=root,
                         output=output,
                         require_clean=args.require_clean,
                         check_remote=args.check_remote,
                         log_limit=args.log_limit)
    report = build_report(config)
    payload = json.dumps(report, indent=2, sort_keys=True) + "\n"
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(payload, encoding="utf-8")
    sys.stdout.write(payload)
    return 0 if report["summary"]["status"] == "passed" else 1


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
