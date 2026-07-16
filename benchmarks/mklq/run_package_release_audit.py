#!/usr/bin/env python3
"""Audit the tracked contract for an MKL-Q Python package release."""

from __future__ import annotations

import argparse
import json
import re
import tomllib
from pathlib import Path
from typing import Any


SCHEMA_VERSION = "mklq-package-release-audit-v1"
TAG_PATTERN = re.compile(
    r"^mklq-v(\d+\.\d+\.\d+(?:(?:a|b|rc)\d+)?(?:\.post\d+)?)$")
BASE_VERSION_PATTERN = re.compile(r"^(\d+\.\d+\.\d+)")


def passed(name: str, details: dict[str, Any] | None = None) -> dict[str, Any]:
    return {"name": name, "status": "passed", "details": details or {}}


def failed(name: str, message: str,
           details: dict[str, Any] | None = None) -> dict[str, Any]:
    payload: dict[str, Any] = {"name": name, "status": "failed", "message": message}
    if details:
        payload["details"] = details
    return payload


def validate_release_tag(tag: str, version: str) -> None:
    match = TAG_PATTERN.fullmatch(tag)
    if match is None or match.group(1) != version:
        raise ValueError(f"release tag must be mklq-v{version}, got {tag}")


def release_notes_version(version: str) -> str:
    """Map RC/post release versions to the final release-notes document."""
    match = BASE_VERSION_PATTERN.fullmatch(version) or BASE_VERSION_PATTERN.match(
        version)
    if match is None:
        raise ValueError(f"cannot derive release notes version from {version}")
    return match.group(1)


def check_package_metadata(root: Path) -> dict[str, Any]:
    path = root / "pyproject.toml"
    try:
        with path.open("rb") as handle:
            project = tomllib.load(handle)["project"]
    except (FileNotFoundError, KeyError, tomllib.TOMLDecodeError) as exc:
        return failed("package_metadata", f"cannot read package metadata: {exc}")

    expected = {"name": "mklq", "requires-python": ">=3.11,<3.15"}
    mismatches = {
        key: {"expected": value, "actual": project.get(key)}
        for key, value in expected.items() if project.get(key) != value
    }
    return (failed("package_metadata", "MKL-Q package metadata mismatch", mismatches)
            if mismatches else passed("package_metadata", expected))


def check_document_tokens(root: Path, relative_path: str,
                          required: tuple[str, ...]) -> dict[str, Any]:
    path = root / relative_path
    try:
        text = path.read_text(encoding="utf-8")
    except FileNotFoundError:
        return failed(relative_path, "required release document is missing")
    missing = [token for token in required if token not in text]
    return (failed(relative_path, "required release text is missing",
                   {"missing": missing}) if missing else
            passed(relative_path, {"required": list(required)}))


def build_report(root: Path, *, version: str, docs_only: bool) -> dict[str, Any]:
    notes_version = release_notes_version(version)
    checks = [
        check_package_metadata(root),
        check_document_tokens(
            root,
            "docs/mklq/release-policy.md",
            ("mklq-vX.Y.Z", "TestPyPI", "PyPI", "mklq-metal", "experimental",
             ".pkg"),
        ),
        check_document_tokens(
            root,
            f"docs/mklq/release-notes-v{notes_version}.md",
            (f"mklq-v{notes_version}", "mklq-cpu", "mklq-metal", "experimental",
             "TestPyPI", "PyPI"),
        ),
        check_document_tokens(
            root,
            "SECURITY.md",
            ("private vulnerability reporting",),
        ),
        check_document_tokens(
            root,
            ".github/workflows/mklq-package-release.yml",
            ("workflow_dispatch", "id-token: write", "testpypi", "pypi",
             "attest-build-provenance", 'python: "3.11"', 'python: "3.12"',
             'python: "3.13"', 'python: "3.14"',
             "Verify trusted runner Python provisioning", "lipo -archs",
             "Reinstall and validate the published package",
             "https://test.pypi.org/simple", "https://pypi.org/simple",
             "mklq-v${{ inputs.version }}rc1",
             "MKLQ_ASSET_ROOT: ${{ runner.temp }}",
             "git status --porcelain --untracked-files=all"),
        ),
    ]
    if not docs_only:
        try:
            validate_release_tag(f"mklq-v{version}", version)
        except ValueError as exc:
            checks.append(failed("release_tag", str(exc)))

    passed_count = sum(check["status"] == "passed" for check in checks)
    failed_count = len(checks) - passed_count
    return {
        "schema_version": SCHEMA_VERSION,
        "version": version,
        "docs_only": docs_only,
        "checks": checks,
        "summary": {
            "status": "passed" if failed_count == 0 else "failed",
            "passed": passed_count,
            "failed": failed_count,
        },
    }


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo-root", type=Path, default=Path.cwd())
    parser.add_argument("--version", required=True)
    parser.add_argument("--docs-only", action="store_true")
    parser.add_argument("--output", type=Path)
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    report = build_report(args.repo_root.resolve(), version=args.version,
                          docs_only=args.docs_only)
    payload = json.dumps(report, indent=2, sort_keys=True) + "\n"
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(payload, encoding="utf-8")
    else:
        print(payload, end="")
    return 0 if report["summary"]["status"] == "passed" else 1


if __name__ == "__main__":
    raise SystemExit(main())
