#!/usr/bin/env python3
# ============================================================================ #
# Copyright (c) 2026 Linsen Wu.                                                #
#                                                                              #
# This source code and the accompanying materials are made available under     #
# the terms of the Apache License 2.0 which accompanies this distribution.     #
# ============================================================================ #

"""Reject tracked benchmark reports that contain private absolute paths."""

import argparse
import json
import re
import sys
from pathlib import Path
from typing import Any, Iterator


SCHEMA_VERSION = "mklq-public-report-path-check-v1"
PRIVATE_PATH_PATTERNS = (
    ("/Users/", re.compile(r"/Users/")),
    ("/home/", re.compile(r"/home/")),
    ("/private/", re.compile(r"/private/")),
    ("/Volumes/", re.compile(r"/Volumes/")),
    ("<windows-user-path>", re.compile(r"[A-Za-z]:\\Users\\")),
)


def json_strings(value: Any, path: str = "$") -> Iterator[tuple[str, str]]:
    if isinstance(value, dict):
        for key, item in value.items():
            yield from json_strings(item, f"{path}.{key}")
        return
    if isinstance(value, list):
        for index, item in enumerate(value):
            yield from json_strings(item, f"{path}[{index}]")
        return
    if isinstance(value, str):
        yield path, value


def private_path_prefix(value: str) -> str | None:
    for label, pattern in PRIVATE_PATH_PATTERNS:
        if pattern.search(value):
            return label
    if value.startswith("/"):
        return "<unix-absolute-path>"
    return None


def check_reports(report_dir: Path) -> dict[str, Any]:
    reports = sorted(report_dir.glob("*.json"))
    summaries = [path for path in reports if path.name.endswith(".summary.json")]
    violations: list[dict[str, str]] = []
    parse_errors: list[dict[str, str]] = []
    for path in reports:
        relative_path = path.relative_to(report_dir).as_posix()
        try:
            payload = json.loads(path.read_text(encoding="utf-8"))
        except (OSError, json.JSONDecodeError) as error:
            parse_errors.append({"path": relative_path, "error": str(error)})
            continue
        for json_path, value in json_strings(payload):
            prefix = private_path_prefix(value)
            if prefix:
                violations.append({
                    "path": relative_path,
                    "json_path": json_path,
                    "prefix": prefix,
                })

    details = {
        "report_count": len(reports),
        "summary_count": len(summaries),
        "violation_count": len(violations),
        "violations": violations,
        "parse_error_count": len(parse_errors),
        "parse_errors": parse_errors,
    }
    if not reports:
        return {
            "schema_version": SCHEMA_VERSION,
            "status": "failed",
            "message": "no public MKL-Q benchmark reports found",
            "details": details,
        }
    if not summaries:
        return {
            "schema_version": SCHEMA_VERSION,
            "status": "failed",
            "message": "no sanitized MKL-Q benchmark summaries found",
            "details": details,
        }
    if parse_errors:
        return {
            "schema_version": SCHEMA_VERSION,
            "status": "failed",
            "message": "public benchmark report JSON could not be parsed",
            "details": details,
        }
    if violations:
        return {
            "schema_version": SCHEMA_VERSION,
            "status": "failed",
            "message": "public benchmark reports contain private absolute paths",
            "details": details,
        }
    return {
        "schema_version": SCHEMA_VERSION,
        "status": "passed",
        "message": "public benchmark report paths are sanitized",
        "details": details,
    }


def write_json(payload: dict[str, Any], output: Path | None) -> None:
    rendered = json.dumps(payload, indent=2, sort_keys=True) + "\n"
    if output is None:
        print(rendered, end="")
        return
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(rendered, encoding="utf-8")


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Reject public MKL-Q benchmark reports with private paths.")
    parser.add_argument("--reports",
                        type=Path,
                        default=Path("benchmarks/mklq/reports"),
                        help="Directory containing tracked public report JSON.")
    parser.add_argument("--output",
                        type=Path,
                        help="Optional JSON result path.")
    return parser.parse_args(argv)


def main(argv: list[str]) -> int:
    args = parse_args(argv)
    result = check_reports(args.reports)
    write_json(result, args.output)
    return 0 if result["status"] == "passed" else 1


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
