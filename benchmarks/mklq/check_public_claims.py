#!/usr/bin/env python3
# ============================================================================ #
# Copyright (c) 2026 Linsen Wu.                                                #
#                                                                              #
# This source code and the accompanying materials are made available under     #
# the terms of the Apache License 2.0 which accompanies this distribution.     #
# ============================================================================ #

"""Check public MKL-Q docs for bounded release, Metal, and performance claims."""

import argparse
import json
import re
from dataclasses import dataclass
from pathlib import Path


CHECK_SCHEMA_VERSION = "mklq-public-claim-boundary-check-v1"


@dataclass(frozen=True)
class RequiredClaim:
    path: str
    phrase: str


FORBIDDEN_PUBLIC_CLAIMS = (
    "release-ready",
    "production-ready",
    "default-ready",
    "full Metal-native",
    "full-Metal-native",
    "fully Metal-native",
    "fully-Metal-native",
    "full Metal GPU backend performance",
    "cross-machine performance certification",
    "cross-machine performance proof",
    "release certification",
    "release sign-off",
    "every operation stays on Metal",
    "all-Metal execution",
)

NEGATION_HINTS = (
    "not",
    "does not",
    "do not",
    "no",
    "none",
    "instead of",
    "without",
    "unless",
    "never",
    "nor",
    "cannot",
    "can't",
    "non-goal",
    "unsupported",
    "does not prove",
    "does not certify",
    "do not publish",
    "do not describe",
    "do not do",
    "stop conditions",
    "forbidden now",
)

REQUIRED_PUBLIC_CLAIMS = (
    RequiredClaim("README.md", "source-only"),
    RequiredClaim("README.md", "experimental Apple GPU target"),
    RequiredClaim("docs/mklq/architecture.md", "not a full Metal-native"),
    RequiredClaim("docs/mklq/architecture.md",
                  "not cross-machine performance certification"),
    RequiredClaim("docs/mklq/testing-matrix.md",
                  "Does not prove full GPU residency"),
    RequiredClaim("docs/mklq/testing-matrix.md",
                  "No cross-machine performance certification"),
    RequiredClaim("docs/mklq/validation.md", "not a release certification"),
    RequiredClaim("docs/mklq/validation.md",
                  "cross-machine performance certification"),
    RequiredClaim("docs/mklq/release-policy.md", "source-only"),
    RequiredClaim("docs/mklq/public-readiness.md",
                  "not a release certification"),
    RequiredClaim("docs/mklq/benchmark-evidence.md",
                  "none is a cross-machine performance certification"),
    RequiredClaim("docs/mklq/benchmark-evidence.md",
                  "not native backend internal phase counters"),
    RequiredClaim("docs/mklq/benchmark-evidence.md",
                  "not runtime counters or proof"),
    RequiredClaim("benchmarks/mklq/README.md",
                  "not full Metal GPU backend performance"),
    RequiredClaim("benchmarks/mklq/README.md",
                  "not a native backend profiler"),
)


def repo_root() -> Path:
    return Path(__file__).resolve().parents[2]


def command_path(root: Path, path: Path) -> str:
    try:
        return path.relative_to(root).as_posix()
    except ValueError:
        return path.as_posix()


def normalize_text(text: str) -> str:
    return re.sub(r"\s+", " ", text).strip().lower()


def public_claim_paths(root: Path) -> list[Path]:
    paths = [
        root / "README.md",
        root / ".github" / "pull_request_template.md",
        root / "benchmarks" / "mklq" / "README.md",
        root / "examples" / "mklq" / "README.md",
        root / "runtime" / "nvqir" / "mklq" / "README.md",
        root / "runtime" / "nvqir" / "mklq" / "mklq-metal.yml",
    ]
    paths.extend((root / "docs" / "mklq").glob("*.md"))
    return sorted(path for path in set(paths) if path.exists())


def has_required_phrase(text: str, phrase: str) -> bool:
    return normalize_text(phrase) in normalize_text(text)


def is_negated_claim(line: str, phrase: str) -> bool:
    normalized = normalize_text(line)
    needle = normalize_text(phrase)
    index = normalized.find(needle)
    if index < 0:
        return False
    if any(hint in normalized for hint in (
            "does not prove",
            "does not certify",
            "do not publish",
            "do not do these",
            "stop conditions",
            "forbidden now",
    )):
        return True
    before = normalized[max(0, index - 160):index]
    after = normalized[index:index + len(needle) + 48]
    context = f"{before} {after}"
    return any(hint in context for hint in NEGATION_HINTS)


def check_text(path_label: str, text: str) -> list[str]:
    failures: list[str] = []
    lines = text.splitlines()
    for line_number, line in enumerate(lines, start=1):
        normalized = normalize_text(line)
        if not normalized:
            continue
        for phrase in FORBIDDEN_PUBLIC_CLAIMS:
            if normalize_text(phrase) not in normalized:
                continue
            start = max(0, line_number - 24)
            stop = min(len(lines), line_number + 2)
            context = " ".join(lines[start:stop])
            if is_negated_claim(context, phrase):
                continue
            failures.append(
                f"{path_label}:{line_number}: unbounded public claim {phrase!r}")
    return failures


def check_required_claims(root: Path) -> list[str]:
    failures: list[str] = []
    for requirement in REQUIRED_PUBLIC_CLAIMS:
        path = root / requirement.path
        if not path.exists():
            failures.append(f"{requirement.path}: file is missing")
            continue
        text = path.read_text(encoding="utf-8", errors="replace")
        if not has_required_phrase(text, requirement.phrase):
            failures.append(
                f"{requirement.path}: missing boundary phrase {requirement.phrase!r}"
            )
    return failures


def check_repository(root: Path) -> dict[str, object]:
    scanned = []
    failures = check_required_claims(root)
    for path in public_claim_paths(root):
        text = path.read_text(encoding="utf-8", errors="replace")
        label = command_path(root, path)
        scanned.append(label)
        failures.extend(check_text(label, text))

    return {
        "schema_version": CHECK_SCHEMA_VERSION,
        "scanned_files": scanned,
        "required_claim_count": len(REQUIRED_PUBLIC_CLAIMS),
        "forbidden_claim_count": len(FORBIDDEN_PUBLIC_CLAIMS),
        "summary": {
            "status": "passed" if not failures else "failed",
            "failure_count": len(failures),
        },
        "failures": failures,
    }


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Check public MKL-Q claim-boundary wording.")
    parser.add_argument("--root", type=Path, default=repo_root())
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    result = check_repository(args.root)
    print(json.dumps(result, indent=2, sort_keys=True))
    return 0 if result["summary"]["status"] == "passed" else 1


if __name__ == "__main__":
    raise SystemExit(main())
