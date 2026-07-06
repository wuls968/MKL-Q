#!/usr/bin/env python3
# ============================================================================ #
# Copyright (c) 2026 Linsen Wu.                                                #
#                                                                              #
# This source code and the accompanying materials are made available under     #
# the terms of the Apache License 2.0 which accompanies this distribution.     #
# ============================================================================ #

"""Audit MKL-Q source-only tag preflight without creating tags."""

import argparse
import json
import re
import subprocess
import sys
import time
from dataclasses import dataclass
from datetime import date, datetime, timezone
from pathlib import Path
from typing import Any


SCHEMA_VERSION = "mklq-source-release-tag-audit-v1"
DEFAULT_REPO = "wuls968/MKL-Q"
DEFAULT_TAG = "mklq-v0.1.0-source"
DEFAULT_WORKFLOW = "MKL-Q public hygiene"
APPLE_WORKFLOW = "MKL-Q Apple Silicon correctness"
LIVE_COMMAND_ATTEMPTS = 3
LIVE_COMMAND_RETRY_DELAY_SECONDS = 1.0
TAG_PATTERN = re.compile(r"^mklq-v\d+\.\d+\.\d+-source$")

RELEASE_NOTES_PATH = Path("docs/mklq/release-notes-v0.1.0-source.md")
CHANGELOG_PATH = Path("CHANGELOG.md")
RELEASE_POLICY_PATH = Path("docs/mklq/release-policy.md")
PUBLIC_CHECKLIST_PATH = Path("docs/mklq/public-release-checklist.md")
SOURCE_RC_PATH = Path("docs/mklq/source-only-rc-v0.1.md")
PUBLIC_READINESS_PATH = Path("docs/mklq/public-readiness.md")
README_PATH = Path("README.md")

RELEASE_NOTE_REQUIRED_TOKENS = (
    "mklq-v0.1.0-source",
    "source-only tag candidate",
    "not a GitHub Release",
    "No wheels or PyPI package",
    "No binary installer",
    "mklq-cpu",
    "mklq-metal",
    "experimental",
    "run_source_release_tag_audit.py",
    "run_public_healthcheck.py --full --require-clean",
    "run_public_readiness_audit.py",
    "28784584186",
    "206d392fc30019f6934965ec88ae18d30c87324d",
)

CHANGELOG_REQUIRED_TOKENS = (
    "mklq-v0.1.0-source",
    "Planned Source-Only Tag",
    "tag has not been created",
    "No GitHub Release",
    "No wheel, PyPI package, installer",
    "release-notes-v0.1.0-source.md",
)

POLICY_REQUIRED_TOKENS = (
    "mklq-vX.Y.Z",
    "Create or push release tags",
    "reviewed release plan",
    "source-only-rc-v0.1",
)

CHECKLIST_REQUIRED_TOKENS = (
    "run_source_release_tag_audit.py",
    "mklq-v0.1.0-source",
    "release-notes-v0.1.0-source.md",
    "CHANGELOG.md",
    "Do not create tags",
)

README_REQUIRED_TOKENS = (
    "CHANGELOG.md",
    "release-notes-v0.1.0-source.md",
    "run_source_release_tag_audit.py",
)


@dataclass(frozen=True)
class AuditConfig:
    repo_root: Path
    repo: str
    tag: str
    workflow: str
    output: Path
    docs_only: bool = False


def repo_root() -> Path:
    return Path(__file__).resolve().parents[2]


def output_default(stamp: str) -> Path:
    return Path("benchmarks/mklq/results") / (
        f"source-release-tag-audit-{stamp}.json")


def is_retryable_live_command(command: list[str]) -> bool:
    if not command:
        return False
    if command[0] == "gh":
        return True
    return command[:2] == ["git", "ls-remote"]


def command_output(cwd: Path, command: list[str]) -> str:
    attempts = LIVE_COMMAND_ATTEMPTS if is_retryable_live_command(command) else 1
    for attempt in range(1, attempts + 1):
        try:
            return subprocess.check_output(command,
                                           cwd=cwd,
                                           text=True,
                                           stderr=subprocess.STDOUT).rstrip("\n")
        except subprocess.CalledProcessError:
            if attempt == attempts:
                raise
            time.sleep(LIVE_COMMAND_RETRY_DELAY_SECONDS)
    raise RuntimeError("unreachable command retry state")


def normalize_whitespace(text: str) -> str:
    return " ".join(text.split())


def missing_tokens(text: str, tokens: tuple[str, ...]) -> list[str]:
    normalized = normalize_whitespace(text)
    return [
        token for token in tokens
        if normalize_whitespace(token) not in normalized
    ]


def passed(name: str, details: dict[str, Any] | None = None) -> dict[str, Any]:
    return {"name": name, "status": "passed", "details": details or {}}


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


def remote_sha(output: str) -> str:
    if not output.strip():
        return ""
    return output.split()[0]


def check_candidate_tag(config: AuditConfig) -> dict[str, Any]:
    failures: list[str] = []
    if not TAG_PATTERN.fullmatch(config.tag):
        failures.append("tag does not match mklq-vX.Y.Z-source")

    remote = ""
    if not config.docs_only:
        remote = command_output(
            config.repo_root,
            ["git", "ls-remote", "--tags", "origin", f"refs/tags/{config.tag}"],
        )
    local = command_output(config.repo_root, ["git", "tag", "-l", config.tag])
    if not config.docs_only and remote.strip():
        failures.append("candidate tag already exists on origin")
    if local.strip():
        failures.append("candidate tag already exists locally")

    details = {
        "tag": config.tag,
        "pattern": TAG_PATTERN.pattern,
        "remote_tag": remote.splitlines(),
        "local_tag": local.splitlines(),
    }
    return failed("candidate_tag", "; ".join(failures),
                  details) if failures else passed("candidate_tag", details)


def check_local_git(config: AuditConfig) -> dict[str, Any]:
    status = command_output(config.repo_root,
                            ["git", "status", "--short", "--branch"])
    shallow = command_output(config.repo_root,
                             ["git", "rev-parse", "--is-shallow-repository"])
    head = command_output(config.repo_root, ["git", "rev-parse", "HEAD"])
    origin = command_output(config.repo_root,
                            ["git", "ls-remote", "origin", "refs/heads/main"])
    origin_main = remote_sha(origin)
    failures: list[str] = []
    dirty = [line for line in status.splitlines() if not line.startswith("##")]
    if dirty:
        failures.append("working tree is dirty")
    if shallow.strip() != "false":
        failures.append("repository is shallow")
    if head != origin_main:
        failures.append("local HEAD does not match origin/main")
    details = {
        "status_short_branch": status.splitlines(),
        "is_shallow": shallow.strip(),
        "head": head,
        "origin_main": origin_main,
    }
    return failed("local_git", "; ".join(failures),
                  details) if failures else passed("local_git", details)


def check_release_notes(config: AuditConfig) -> dict[str, Any]:
    path = config.repo_root / RELEASE_NOTES_PATH
    if not path.exists():
        return failed("release_notes", "release notes are missing",
                      {"path": RELEASE_NOTES_PATH.as_posix()})
    text = path.read_text(encoding="utf-8")
    missing = missing_tokens(text, RELEASE_NOTE_REQUIRED_TOKENS)
    details = {
        "path": RELEASE_NOTES_PATH.as_posix(),
        "required": list(RELEASE_NOTE_REQUIRED_TOKENS),
        "missing": missing,
    }
    return failed("release_notes", "release notes are missing required tokens",
                  details) if missing else passed("release_notes", details)


def check_changelog(config: AuditConfig) -> dict[str, Any]:
    path = config.repo_root / CHANGELOG_PATH
    if not path.exists():
        return failed("changelog", "CHANGELOG.md is missing",
                      {"path": CHANGELOG_PATH.as_posix()})
    text = path.read_text(encoding="utf-8")
    missing = missing_tokens(text, CHANGELOG_REQUIRED_TOKENS)
    details = {
        "path": CHANGELOG_PATH.as_posix(),
        "required": list(CHANGELOG_REQUIRED_TOKENS),
        "missing": missing,
    }
    return failed("changelog", "CHANGELOG.md is missing required tokens",
                  details) if missing else passed("changelog", details)


def check_policy_and_checklist(config: AuditConfig) -> dict[str, Any]:
    required = {
        RELEASE_POLICY_PATH: POLICY_REQUIRED_TOKENS,
        PUBLIC_CHECKLIST_PATH: CHECKLIST_REQUIRED_TOKENS,
        README_PATH: README_REQUIRED_TOKENS,
        SOURCE_RC_PATH: ("206d392fc30019f6934965ec88ae18d30c87324d",
                         "28784584186"),
        PUBLIC_READINESS_PATH: ("206d392fc30019f6934965ec88ae18d30c87324d",
                                "28784584186"),
    }
    failures: list[str] = []
    details: dict[str, Any] = {"files": {}}
    for relative, tokens in required.items():
        path = config.repo_root / relative
        file_details: dict[str, Any] = {"required": list(tokens)}
        if not path.exists():
            file_details["missing"] = list(tokens)
            failures.append(f"{relative.as_posix()}: file is missing")
            details["files"][relative.as_posix()] = file_details
            continue
        missing = missing_tokens(path.read_text(encoding="utf-8"), tokens)
        file_details["missing"] = missing
        if missing:
            failures.append(f"{relative.as_posix()}: missing required text")
        details["files"][relative.as_posix()] = file_details
    return failed("policy_and_checklist",
                  "; ".join(failures),
                  details) if failures else passed("policy_and_checklist",
                                                   details)


def check_no_release_artifacts(config: AuditConfig) -> dict[str, Any]:
    tracked = command_output(config.repo_root, ["git", "ls-files"]).splitlines()
    forbidden_patterns = (
        re.compile(r"^(dist|wheelhouse)/"),
        re.compile(r"\.(whl|dmg|pkg|zip)$"),
        re.compile(r"\.tar\.gz$"),
        re.compile(r"^benchmarks/mklq/results/"),
    )
    bad = [
        path for path in tracked
        if any(pattern.search(path) for pattern in forbidden_patterns)
    ]
    releases = ""
    if not config.docs_only:
        releases = command_output(
            config.repo_root,
            ["gh", "release", "list", "--repo", config.repo, "--limit", "100"],
        )
    details = {
        "bad_tracked_paths": bad,
        "github_releases": releases.splitlines(),
    }
    failures: list[str] = []
    if bad:
        failures.append("release or local result artifacts are tracked")
    if releases.strip():
        failures.append("GitHub Releases exist")
    return failed("no_release_artifacts", "; ".join(failures),
                  details) if failures else passed("no_release_artifacts",
                                                   details)


def latest_workflow(config: AuditConfig, workflow: str) -> dict[str, Any]:
    head = command_output(config.repo_root, ["git", "rev-parse", "HEAD"])
    payload = command_output(config.repo_root, [
        "gh",
        "run",
        "list",
        "--repo",
        config.repo,
        "--branch",
        "main",
        "--workflow",
        workflow,
        "--limit",
        "1",
        "--json",
        "status,conclusion,headSha,url,createdAt,event,databaseId,name",
    ])
    runs = json.loads(payload or "[]")
    details = runs[0] if runs else {}
    failures: list[str] = []
    if not runs:
        failures.append(f"no {workflow} run found")
    else:
        if details.get("status") != "completed":
            failures.append(f"{workflow} run is not completed")
        if details.get("conclusion") != "success":
            failures.append(f"{workflow} run did not succeed")
        if details.get("headSha") != head:
            failures.append(f"{workflow} run does not match local HEAD")
    return failed(f"latest_{workflow_key(workflow)}",
                  "; ".join(failures),
                  details) if failures else passed(
                      f"latest_{workflow_key(workflow)}", details)


def workflow_key(workflow: str) -> str:
    return workflow.lower().replace("mkl-q ", "").replace(" ", "_")


def build_report(config: AuditConfig) -> dict[str, Any]:
    root = config.repo_root.resolve()
    resolved = AuditConfig(repo_root=root,
                           repo=config.repo,
                           tag=config.tag,
                           workflow=config.workflow,
                           output=config.output,
                           docs_only=config.docs_only)
    checks = [
        check_candidate_tag(resolved),
        check_release_notes(resolved),
        check_changelog(resolved),
        check_policy_and_checklist(resolved),
        check_no_release_artifacts(resolved),
    ]
    if not resolved.docs_only:
        checks.insert(1, check_local_git(resolved))
        checks.extend([
            latest_workflow(resolved, resolved.workflow),
            latest_workflow(resolved, APPLE_WORKFLOW),
        ])
    return {
        "schema_version": SCHEMA_VERSION,
        "created_at_utc": datetime.now(timezone.utc).isoformat(),
        "repo_root": root.as_posix(),
        "repo": resolved.repo,
        "candidate_tag": resolved.tag,
        "docs_only": resolved.docs_only,
        "summary": summarize(checks),
        "checks": checks,
    }


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Audit MKL-Q source-only tag preflight without creating tags.")
    parser.add_argument("--repo-root",
                        type=Path,
                        default=repo_root(),
                        help="Repository root. Defaults to this checkout.")
    parser.add_argument("--repo",
                        default=DEFAULT_REPO,
                        help="GitHub repo in owner/name form.")
    parser.add_argument("--tag",
                        default=DEFAULT_TAG,
                        help="Candidate source-only tag to audit.")
    parser.add_argument("--workflow",
                        default=DEFAULT_WORKFLOW,
                        help="Public hygiene workflow name.")
    parser.add_argument("--output",
                        type=Path,
                        help="Optional JSON output path.")
    parser.add_argument("--docs-only",
                        action="store_true",
                        help=("Skip live GitHub and origin/main checks. Use this "
                              "for PR/public-hygiene docs-boundary checks."))
    return parser.parse_args(argv)


def main(argv: list[str]) -> int:
    args = parse_args(argv)
    root = args.repo_root.resolve()
    stamp = date.today().isoformat()
    output = args.output or output_default(stamp)
    output = output if output.is_absolute() else root / output
    config = AuditConfig(repo_root=root,
                         repo=args.repo,
                         tag=args.tag,
                         workflow=args.workflow,
                         output=output,
                         docs_only=args.docs_only)
    report = build_report(config)
    output.parent.mkdir(parents=True, exist_ok=True)
    payload = json.dumps(report, indent=2, sort_keys=True) + "\n"
    output.write_text(payload, encoding="utf-8")
    sys.stdout.write(payload)
    return 0 if report["summary"]["status"] == "passed" else 1


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
