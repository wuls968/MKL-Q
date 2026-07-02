#!/usr/bin/env python3
# ============================================================================ #
# Copyright (c) 2026 Linsen Wu.                                                #
#                                                                              #
# This source code and the accompanying materials are made available under     #
# the terms of the Apache License 2.0 which accompanies this distribution.     #
# ============================================================================ #

"""Audit MKL-Q public GitHub readiness and write bounded JSON evidence."""

import argparse
import json
import re
import subprocess
import sys
from dataclasses import dataclass
from datetime import date, datetime, timezone
from pathlib import Path
from typing import Any


SCHEMA_VERSION = "mklq-public-readiness-audit-v1"
DEFAULT_REPO = "wuls968/MKL-Q"
DEFAULT_WORKFLOW = "MKL-Q public hygiene"
REQUIRED_STATUS_CHECK = "Source-only repository checks"
EXPECTED_DESCRIPTION = (
    "CUDA-Q-compatible Apple Silicon simulator fork with MKL-Q targets")
EXPECTED_TOPICS = {
    "accelerate",
    "apple-silicon",
    "cuda-quantum",
    "metal",
    "mklq",
    "quantum-computing",
}
EXPECTED_ISSUE_TEMPLATES = [
    ".github/ISSUE_TEMPLATE/bug_report.yaml",
    ".github/ISSUE_TEMPLATE/feature_request.yaml",
]
TRACKED_ARTIFACT_PATTERN = re.compile(
    r"(^|/)(__pycache__|\.pytest_cache)(/|$)|"
    r"\.pyc$|\.DS_Store$|^build(-python)?/|"
    r"^benchmarks/mklq/results/|^docs/superpowers/|"
    r"^(dist|wheelhouse)/|\.(whl|dmg|pkg|zip)$|\.tar\.gz$")
PUBLIC_READINESS_DOC = Path("docs/mklq/public-readiness.md")
PUBLIC_READINESS_REQUIRED_PHRASES = [
    ("source_only_audit", "source-only repository audit"),
    ("does_not_certify", "It does not certify:"),
    ("audit_script", "run_public_readiness_audit.py"),
    (
        "latest_hygiene",
        "latest pushed commit has a successful `MKL-Q public hygiene` run",
    ),
    (
        "branch_protection_reference",
        "live branch protection matches `.github/branch-protection-main.json`",
    ),
    (
        "no_tags_or_releases",
        "no release tags or GitHub Releases exist in the current source-only phase",
    ),
    (
        "metal_experimental",
        "`mklq-metal` is experimental and must not be described as default-ready",
    ),
]


@dataclass(frozen=True)
class AuditConfig:
    repo_root: Path
    repo: str
    workflow: str
    output: Path


def repo_root() -> Path:
    return Path(__file__).resolve().parents[2]


def output_default(stamp: str) -> Path:
    return Path("benchmarks/mklq/results") / (
        f"public-readiness-audit-{stamp}.json")


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


def load_json(text: str, fallback: Any) -> Any:
    if not text.strip():
        return fallback
    return json.loads(text)


def unquote_yaml_scalar(value: str) -> str:
    value = value.strip()
    if len(value) >= 2 and value[0] == value[-1] and value[0] in {'"', "'"}:
        return value[1:-1]
    return value


def parse_inline_yaml_list(value: str) -> list[str]:
    value = value.strip()
    if not value.startswith("[") or not value.endswith("]"):
        return []
    items = value[1:-1].split(",")
    return [unquote_yaml_scalar(item) for item in items if item.strip()]


def parse_labels_yaml(path: Path) -> dict[str, dict[str, str]]:
    labels: dict[str, dict[str, str]] = {}
    current: dict[str, str] | None = None
    for raw_line in path.read_text(encoding="utf-8").splitlines():
        line = raw_line.strip()
        if not line or line.startswith("#"):
            continue
        if line.startswith("- name:"):
            name = unquote_yaml_scalar(line.split(":", 1)[1])
            current = {"name": name}
            labels[name] = current
            continue
        if current is None or ":" not in line:
            continue
        key, value = line.split(":", 1)
        if key in {"color", "description"}:
            current[key] = unquote_yaml_scalar(value)
    return labels


def parse_issue_template_labels(path: Path) -> list[str]:
    for raw_line in path.read_text(encoding="utf-8").splitlines():
        line = raw_line.strip()
        if not line.startswith("labels:"):
            continue
        value = line.split(":", 1)[1].strip()
        return parse_inline_yaml_list(value)
    return []


def bool_from_protection_field(value: Any) -> bool | None:
    if isinstance(value, bool):
        return value
    if isinstance(value, dict) and isinstance(value.get("enabled"), bool):
        return value["enabled"]
    return None


def protection_core(payload: dict[str, Any]) -> dict[str, Any]:
    required = payload.get("required_status_checks") or {}
    checks = required.get("checks") or []
    check_contexts = [
        str(check.get("context")) for check in checks
        if isinstance(check, dict) and check.get("context")
    ]
    contexts = sorted({str(item) for item in required.get("contexts") or []}
                      | set(check_contexts))
    return {
        "required_status_checks": {
            "strict": required.get("strict"),
            "contexts": contexts,
        },
        "enforce_admins": bool_from_protection_field(
            payload.get("enforce_admins")),
        "allow_force_pushes": bool_from_protection_field(
            payload.get("allow_force_pushes")),
        "allow_deletions": bool_from_protection_field(
            payload.get("allow_deletions")),
        "required_linear_history": bool_from_protection_field(
            payload.get("required_linear_history")),
        "block_creations": bool_from_protection_field(
            payload.get("block_creations")),
        "required_conversation_resolution": bool_from_protection_field(
            payload.get("required_conversation_resolution")),
        "lock_branch": bool_from_protection_field(payload.get("lock_branch")),
        "allow_fork_syncing": bool_from_protection_field(
            payload.get("allow_fork_syncing")),
    }


def protection_reference_core(payload: dict[str, Any]) -> dict[str, Any]:
    required = payload.get("required_status_checks") or {}
    return {
        "required_status_checks": {
            "strict": required.get("strict"),
            "contexts": sorted(str(item) for item in required.get("contexts") or []),
        },
        "enforce_admins": payload.get("enforce_admins"),
        "allow_force_pushes": payload.get("allow_force_pushes"),
        "allow_deletions": payload.get("allow_deletions"),
        "required_linear_history": payload.get("required_linear_history"),
        "block_creations": payload.get("block_creations"),
        "required_conversation_resolution":
            payload.get("required_conversation_resolution"),
        "lock_branch": payload.get("lock_branch"),
        "allow_fork_syncing": payload.get("allow_fork_syncing"),
    }


def remote_head_sha(output: str) -> str:
    if not output.strip():
        return ""
    return output.split()[0]


def check_local_git(config: AuditConfig) -> dict[str, Any]:
    status = command_output(config.repo_root,
                            ["git", "status", "--short", "--branch"])
    shallow = command_output(config.repo_root,
                             ["git", "rev-parse", "--is-shallow-repository"])
    head = command_output(config.repo_root, ["git", "rev-parse", "HEAD"])
    remote = command_output(config.repo_root,
                            ["git", "ls-remote", "origin", "refs/heads/main"])
    remote_sha = remote_head_sha(remote)
    failures: list[str] = []
    dirty = [line for line in status.splitlines() if not line.startswith("##")]
    if dirty:
        failures.append("working tree is dirty")
    if shallow.strip() != "false":
        failures.append("repository is shallow")
    if head != remote_sha:
        failures.append("local HEAD does not match origin/main")
    details = {
        "status_short_branch": status.splitlines(),
        "is_shallow": shallow.strip(),
        "head": head,
        "origin_main": remote_sha,
    }
    return failed("local_git", "; ".join(failures),
                  details) if failures else passed("local_git", details)


def check_tracked_artifacts(config: AuditConfig) -> dict[str, Any]:
    tracked = command_output(config.repo_root, ["git", "ls-files"]).splitlines()
    bad = [path for path in tracked if TRACKED_ARTIFACT_PATTERN.search(path)]
    details = {"tracked_file_count": len(tracked), "bad_paths": bad}
    return failed("tracked_artifacts", "generated or local artifacts are tracked",
                  details) if bad else passed("tracked_artifacts", details)


def check_workflows(config: AuditConfig) -> dict[str, Any]:
    workflows = command_output(config.repo_root,
                               ["git", "ls-files",
                                ".github/workflows"]).splitlines()
    expected = [".github/workflows/mklq-public-hygiene.yml"]
    details = {"workflows": workflows}
    return failed("github_workflows", "unexpected workflow set",
                  details) if workflows != expected else passed(
                      "github_workflows", details)


def check_issue_templates(config: AuditConfig) -> dict[str, Any]:
    tracked = command_output(config.repo_root, [
        "git",
        "ls-files",
        ".github/ISSUE_TEMPLATE",
    ]).splitlines()
    labels = parse_labels_yaml(config.repo_root / ".github" / "labels.yml")
    failures: list[str] = []
    if tracked != EXPECTED_ISSUE_TEMPLATES:
        failures.append("unexpected issue template set")
    referenced_labels: set[str] = set()
    for relative_path in EXPECTED_ISSUE_TEMPLATES:
        path = config.repo_root / relative_path
        if not path.exists():
            failures.append(f"{relative_path} is missing")
            continue
        text = path.read_text(encoding="utf-8", errors="replace")
        if "name:" not in text or "description:" not in text:
            failures.append(f"{relative_path} is missing required metadata")
        referenced_labels.update(parse_issue_template_labels(path))
    missing_declared_labels = sorted(referenced_labels - set(labels))
    if missing_declared_labels:
        failures.append("issue template labels are missing from .github/labels.yml")
    details = {
        "templates": tracked,
        "expected_templates": EXPECTED_ISSUE_TEMPLATES,
        "referenced_labels": sorted(referenced_labels),
        "missing_declared_labels": missing_declared_labels,
    }
    return failed("issue_templates", "; ".join(failures),
                  details) if failures else passed("issue_templates", details)


def check_repository(config: AuditConfig) -> dict[str, Any]:
    payload = load_json(
        command_output(config.repo_root, [
            "gh",
            "repo",
            "view",
            config.repo,
            "--json",
            "nameWithOwner,isFork,parent,defaultBranchRef,url,description,"
            "repositoryTopics,licenseInfo,visibility",
        ]), {})
    failures: list[str] = []
    parent = payload.get("parent") or {}
    parent_owner = parent.get("owner") or {}
    topics = {item.get("name") for item in payload.get("repositoryTopics", [])}
    if payload.get("nameWithOwner") != config.repo:
        failures.append("unexpected repository")
    if payload.get("isFork") is not True:
        failures.append("repository is not a fork")
    if parent.get("name") != "cuda-quantum" or parent_owner.get("login") != "NVIDIA":
        failures.append("parent is not NVIDIA/cuda-quantum")
    if (payload.get("defaultBranchRef") or {}).get("name") != "main":
        failures.append("default branch is not main")
    if payload.get("visibility") != "PUBLIC":
        failures.append("repository is not public")
    if (payload.get("licenseInfo") or {}).get("key") != "apache-2.0":
        failures.append("license is not Apache-2.0")
    if payload.get("description") != EXPECTED_DESCRIPTION:
        failures.append("description does not match MKL-Q public metadata")
    missing_topics = sorted(EXPECTED_TOPICS - topics)
    if missing_topics:
        failures.append("expected topics are missing")
    details = dict(payload)
    details["missing_topics"] = missing_topics
    return failed("github_repository", "; ".join(failures),
                  details) if failures else passed("github_repository", details)


def check_github_labels(config: AuditConfig) -> dict[str, Any]:
    local = parse_labels_yaml(config.repo_root / ".github" / "labels.yml")
    live_payload = load_json(
        command_output(config.repo_root, [
            "gh",
            "label",
            "list",
            "--repo",
            config.repo,
            "--limit",
            "100",
            "--json",
            "name,color,description",
        ]), [])
    live = {
        str(label.get("name")): {
            "name": str(label.get("name")),
            "color": str(label.get("color", "")).lower(),
            "description": str(label.get("description", "")),
        }
        for label in live_payload
        if isinstance(label, dict) and label.get("name")
    }
    failures: list[str] = []
    missing = sorted(set(local) - set(live))
    differing = []
    for name, expected in local.items():
        actual = live.get(name)
        if not actual:
            continue
        if actual.get("color") != expected.get("color", "").lower():
            differing.append(name)
            continue
        if actual.get("description") != expected.get("description", ""):
            differing.append(name)
    if missing or differing:
        failures.append("live label metadata differs from .github/labels.yml")
    details = {
        "expected_labels": sorted(local),
        "live_labels": sorted(live),
        "missing": missing,
        "differing": sorted(differing),
    }
    return failed("github_labels", "; ".join(failures),
                  details) if failures else passed("github_labels", details)


def check_branch_protection(config: AuditConfig) -> dict[str, Any]:
    branch = load_json(
        command_output(config.repo_root, [
            "gh",
            "api",
            f"repos/{config.repo}/branches/main",
        ]), {})
    protection = load_json(
        command_output(config.repo_root, [
            "gh",
            "api",
            f"repos/{config.repo}/branches/main/protection",
        ]), {})
    required = protection.get("required_status_checks") or {}
    contexts = set(required.get("contexts") or [])
    checks = {check.get("context") for check in required.get("checks") or []}
    failures: list[str] = []
    if branch.get("protected") is not True:
        failures.append("branch is not protected")
    if REQUIRED_STATUS_CHECK not in contexts and REQUIRED_STATUS_CHECK not in checks:
        failures.append("required status check is missing")
    if required.get("strict") is not True:
        failures.append("required status checks are not strict")
    if (protection.get("allow_force_pushes") or {}).get("enabled") is not False:
        failures.append("force pushes are not disabled")
    if (protection.get("allow_deletions") or {}).get("enabled") is not False:
        failures.append("branch deletion is not disabled")
    if (protection.get("enforce_admins") or {}).get("enabled") is not True:
        failures.append("administrator enforcement is not enabled")
    details = {"branch": branch, "protection": protection}
    return failed("branch_protection", "; ".join(failures),
                  details) if failures else passed("branch_protection", details)


def check_branch_protection_reference(config: AuditConfig) -> dict[str, Any]:
    reference_path = config.repo_root / ".github" / "branch-protection-main.json"
    reference = load_json(reference_path.read_text(encoding="utf-8"), {})
    protection = load_json(
        command_output(config.repo_root, [
            "gh",
            "api",
            f"repos/{config.repo}/branches/main/protection",
        ]), {})
    expected = protection_reference_core(reference)
    actual = protection_core(protection)
    failures: list[str] = []
    if actual != expected:
        failures.append(
            "branch protection differs from .github/branch-protection-main.json"
        )
    details = {"expected": expected, "actual": actual}
    return failed("branch_protection_reference", "; ".join(failures),
                  details) if failures else passed(
                      "branch_protection_reference", details)


def check_public_claim_boundaries(config: AuditConfig) -> dict[str, Any]:
    script = config.repo_root / "benchmarks" / "mklq" / "check_public_claims.py"
    result = load_json(
        command_output(config.repo_root, [
            sys.executable,
            str(script),
            "--root",
            str(config.repo_root),
        ]), {})
    status = (result.get("summary") or {}).get("status")
    if status != "passed":
        return failed("public_claim_boundaries",
                      "public claim-boundary check failed", result)
    return passed("public_claim_boundaries", result)


def check_public_readiness_doc(config: AuditConfig) -> dict[str, Any]:
    path = config.repo_root / PUBLIC_READINESS_DOC
    if not path.exists():
        return failed("public_readiness_doc",
                      f"{PUBLIC_READINESS_DOC.as_posix()} is missing",
                      {"path": PUBLIC_READINESS_DOC.as_posix()})
    text = path.read_text(encoding="utf-8", errors="replace")
    missing = [
        key for key, phrase in PUBLIC_READINESS_REQUIRED_PHRASES
        if phrase not in text
    ]
    details = {
        "path": PUBLIC_READINESS_DOC.as_posix(),
        "required_phrase_keys": [
            key for key, _ in PUBLIC_READINESS_REQUIRED_PHRASES
        ],
        "missing_phrase_keys": missing,
    }
    if missing:
        return failed("public_readiness_doc",
                      "public readiness document is missing boundary phrases",
                      details)
    return passed("public_readiness_doc", details)


def check_latest_public_hygiene(config: AuditConfig) -> dict[str, Any]:
    runs = load_json(
        command_output(config.repo_root, [
            "gh",
            "run",
            "list",
            "--repo",
            config.repo,
            "--branch",
            "main",
            "--workflow",
            config.workflow,
            "--limit",
            "1",
            "--json",
            "status,conclusion,headSha,url,name,event,createdAt",
        ]), [])
    head = command_output(config.repo_root, ["git", "rev-parse", "HEAD"])
    run = runs[0] if runs else {}
    failures: list[str] = []
    if not run:
        failures.append("no public hygiene run found")
    if run.get("status") != "completed":
        failures.append("latest public hygiene run is not completed")
    if run.get("conclusion") != "success":
        failures.append("latest public hygiene run did not succeed")
    if run.get("headSha") != head:
        failures.append("latest public hygiene run is not for local HEAD")
    return failed("latest_public_hygiene", "; ".join(failures),
                  run) if failures else passed("latest_public_hygiene", run)


def check_no_releases(config: AuditConfig) -> dict[str, Any]:
    tags = command_output(config.repo_root,
                          ["git", "ls-remote", "--tags", "origin", "refs/tags/*"])
    releases = command_output(config.repo_root, [
        "gh",
        "release",
        "list",
        "--repo",
        config.repo,
        "--limit",
        "20",
    ])
    failures: list[str] = []
    if tags.strip():
        failures.append("release tags exist")
    if releases.strip():
        failures.append("GitHub releases exist")
    details = {
        "remote_tags": tags.splitlines(),
        "releases": releases.splitlines(),
    }
    return failed("no_tags_or_releases", "; ".join(failures),
                  details) if failures else passed("no_tags_or_releases", details)


def summarize(checks: list[dict[str, Any]]) -> dict[str, Any]:
    passed_count = sum(1 for check in checks if check["status"] == "passed")
    failed_count = sum(1 for check in checks if check["status"] == "failed")
    return {
        "status": "passed" if failed_count == 0 else "failed",
        "passed": passed_count,
        "failed": failed_count,
    }


def build_report(config: AuditConfig) -> dict[str, Any]:
    checks = [
        check_local_git(config),
        check_tracked_artifacts(config),
        check_workflows(config),
        check_issue_templates(config),
        check_repository(config),
        check_github_labels(config),
        check_branch_protection(config),
        check_branch_protection_reference(config),
        check_public_claim_boundaries(config),
        check_public_readiness_doc(config),
        check_latest_public_hygiene(config),
        check_no_releases(config),
    ]
    return {
        "schema_version": SCHEMA_VERSION,
        "created_at_utc": datetime.now(timezone.utc).isoformat(),
        "config": {
            "repo_root": config.repo_root.as_posix(),
            "repo": config.repo,
            "workflow": config.workflow,
            "output": config.output.as_posix(),
        },
        "checks": checks,
        "summary": summarize(checks),
    }


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Audit MKL-Q public GitHub readiness.")
    parser.add_argument("--repo",
                        default=DEFAULT_REPO,
                        help="GitHub repository, owner/name.")
    parser.add_argument("--workflow",
                        default=DEFAULT_WORKFLOW,
                        help="GitHub Actions workflow name to check.")
    parser.add_argument("--output",
                        type=Path,
                        help="JSON output path. Defaults under ignored results/.")
    parser.add_argument("--stamp",
                        default=date.today().isoformat(),
                        help="Date or label for the default output filename.")
    return parser.parse_args(argv)


def make_config(args: argparse.Namespace) -> AuditConfig:
    root = repo_root()
    output = args.output or output_default(args.stamp)
    if not output.is_absolute():
        output = root / output
    return AuditConfig(repo_root=root,
                       repo=args.repo,
                       workflow=args.workflow,
                       output=output)


def main(argv: list[str]) -> int:
    config = make_config(parse_args(argv))
    report = build_report(config)
    config.output.parent.mkdir(parents=True, exist_ok=True)
    config.output.write_text(json.dumps(report, indent=2, sort_keys=True) +
                             "\n",
                             encoding="utf-8")
    print(json.dumps(report, indent=2, sort_keys=True))
    return 0 if report["summary"]["status"] == "passed" else 1


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
