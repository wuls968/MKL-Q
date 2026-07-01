#!/usr/bin/env python3
# ============================================================================ #
# Copyright (c) 2026 Linsen Wu.                                                #
#                                                                              #
# This source code and the accompanying materials are made available under     #
# the terms of the Apache License 2.0 which accompanies this distribution.     #
# ============================================================================ #

"""Repair ad-hoc code signatures for a local macOS MKL-Q install prefix."""

import argparse
import json
import platform
import subprocess
import sys
import time
from datetime import date, datetime, timezone
from pathlib import Path
from typing import Any


SCHEMA_VERSION = "mklq-macos-install-signature-repair-v1"
DEFAULT_TAIL_CHARS = 4000
LOADABLE_SUFFIXES = (".dylib", ".so")
MACHO_MAGICS = (
    b"\xca\xfe\xba\xbe",
    b"\xbe\xba\xfe\xca",
    b"\xcf\xfa\xed\xfe",
    b"\xfe\xed\xfa\xcf",
    b"\xce\xfa\xed\xfe",
    b"\xfe\xed\xfa\xce",
)


def repo_root() -> Path:
    return Path(__file__).resolve().parents[2]


def output_default(stamp: str) -> Path:
    return Path("benchmarks/mklq/results") / (
        f"macos-install-signature-repair-{stamp}.json")


def output_tail(value: str | bytes | None, tail_chars: int) -> str:
    if value is None:
        return ""
    if isinstance(value, bytes):
        value = value.decode("utf-8", errors="replace")
    return value[-tail_chars:]


def command_path(root: Path, path: Path) -> str:
    try:
        return path.relative_to(root).as_posix()
    except ValueError:
        return path.as_posix()


def is_macho(path: Path) -> bool:
    try:
        with path.open("rb") as handle:
            return handle.read(4) in MACHO_MAGICS
    except OSError:
        return False


def is_under(path: Path, directory: Path) -> bool:
    try:
        path.relative_to(directory)
    except ValueError:
        return False
    return True


def discover_loadables(install_prefix: Path) -> list[Path]:
    if not install_prefix.is_dir():
        return []
    bin_dir = install_prefix / "bin"
    return sorted(path for path in install_prefix.rglob("*")
                  if path.is_file() and (
                      path.suffix in LOADABLE_SUFFIXES or
                      (is_under(path, bin_dir) and is_macho(path))))


def run_process(command: list[str], *, tail_chars: int) -> dict[str, Any]:
    start = time.perf_counter()
    result = subprocess.run(command, capture_output=True, text=True)
    elapsed = time.perf_counter() - start
    return {
        "command": command,
        "returncode": result.returncode,
        "elapsed_seconds": elapsed,
        "stdout_tail": output_tail(result.stdout, tail_chars),
        "stderr_tail": output_tail(result.stderr, tail_chars),
    }


def planned_loadable(root: Path, loadable: Path, codesign: str,
                     skip_verify: bool) -> dict[str, Any]:
    entry: dict[str, Any] = {
        "path": command_path(root, loadable),
        "status": "planned",
        "sign_command": [codesign, "--force", "--sign", "-", str(loadable)],
    }
    if not skip_verify:
        entry["verify_command"] = [
            codesign,
            "--verify",
            "--verbose=2",
            str(loadable),
        ]
    return entry


def repair_loadable(root: Path, loadable: Path, codesign: str, *,
                    skip_verify: bool, tail_chars: int) -> dict[str, Any]:
    entry: dict[str, Any] = {
        "path": command_path(root, loadable),
    }
    sign_result = run_process([codesign, "--force", "--sign", "-", str(loadable)],
                              tail_chars=tail_chars)
    entry["sign"] = sign_result
    if sign_result["returncode"] != 0:
        entry["status"] = "failed"
        entry["message"] = "codesign --force --sign - failed"
        return entry

    if skip_verify:
        entry["status"] = "passed"
        return entry

    verify_result = run_process(
        [codesign, "--verify", "--verbose=2", str(loadable)],
        tail_chars=tail_chars)
    entry["verify"] = verify_result
    if verify_result["returncode"] != 0:
        entry["status"] = "failed"
        entry["message"] = "codesign --verify failed"
        return entry

    entry["status"] = "passed"
    return entry


def summarize(entries: list[dict[str, Any]], status: str | None = None
              ) -> dict[str, Any]:
    failed_count = sum(1 for entry in entries if entry["status"] == "failed")
    planned_count = sum(1 for entry in entries if entry["status"] == "planned")
    passed_count = sum(1 for entry in entries if entry["status"] == "passed")
    if status is None:
        status = "failed" if failed_count else "planned" if planned_count else "passed"
    return {
        "status": status,
        "loadables": len(entries),
        "passed": passed_count,
        "failed": failed_count,
        "planned": planned_count,
    }


def build_report(install_prefix: Path,
                 *,
                 output: Path,
                 codesign: str = "codesign",
                 dry_run: bool = False,
                 skip_verify: bool = False,
                 tail_chars: int = DEFAULT_TAIL_CHARS) -> dict[str, Any]:
    root = repo_root()
    prefix = install_prefix.expanduser().resolve()
    resolved_output = output if output.is_absolute() else root / output
    config = {
        "install_prefix": prefix.as_posix(),
        "output": resolved_output.as_posix(),
        "codesign": codesign,
        "dry_run": dry_run,
        "skip_verify": skip_verify,
        "tail_chars": tail_chars,
    }

    if platform.system() != "Darwin":
        return {
            "schema_version": SCHEMA_VERSION,
            "created_at_utc": datetime.now(timezone.utc).isoformat(),
            "config": config,
            "summary": {
                "status": "skipped",
                "loadables": 0,
                "passed": 0,
                "failed": 0,
                "planned": 0,
            },
            "message": "macOS install-prefix signature repair is Darwin-only.",
            "loadables": [],
        }

    loadables = discover_loadables(prefix)
    if not loadables:
        return {
            "schema_version": SCHEMA_VERSION,
            "created_at_utc": datetime.now(timezone.utc).isoformat(),
            "config": config,
            "summary": {
                "status": "failed",
                "loadables": 0,
                "passed": 0,
                "failed": 1,
                "planned": 0,
            },
            "message": (
                f"No Mach-O loadables or bin executables found under {prefix}."),
            "loadables": [],
        }

    if dry_run:
        entries = [
            planned_loadable(root, loadable, codesign, skip_verify)
            for loadable in loadables
        ]
    else:
        entries = [
            repair_loadable(root,
                            loadable,
                            codesign,
                            skip_verify=skip_verify,
                            tail_chars=tail_chars) for loadable in loadables
        ]

    return {
        "schema_version": SCHEMA_VERSION,
        "created_at_utc": datetime.now(timezone.utc).isoformat(),
        "config": config,
        "summary": summarize(entries),
        "loadables": entries,
    }


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Repair ad-hoc signatures for local macOS MKL-Q loadables.")
    parser.add_argument("--install-prefix",
                        type=Path,
                        default=Path.home() / ".cudaq-mklq",
                        help="Local MKL-Q install prefix.")
    parser.add_argument("--output",
                        type=Path,
                        help="JSON output path.")
    parser.add_argument("--codesign",
                        default="codesign",
                        help="codesign executable to use.")
    parser.add_argument("--dry-run",
                        action="store_true",
                        help="Plan commands without modifying signatures.")
    parser.add_argument("--skip-verify",
                        action="store_true",
                        help="Skip codesign verification after signing.")
    parser.add_argument("--tail-chars",
                        type=int,
                        default=DEFAULT_TAIL_CHARS,
                        help="Maximum stdout/stderr tail characters to record.")
    return parser.parse_args(argv)


def main(argv: list[str]) -> int:
    args = parse_args(argv)
    stamp = date.today().isoformat()
    output = args.output or output_default(stamp)
    report = build_report(args.install_prefix,
                          output=output,
                          codesign=args.codesign,
                          dry_run=args.dry_run,
                          skip_verify=args.skip_verify,
                          tail_chars=args.tail_chars)
    root = repo_root()
    resolved_output = output if output.is_absolute() else root / output
    resolved_output.parent.mkdir(parents=True, exist_ok=True)
    payload = json.dumps(report, indent=2, sort_keys=True) + "\n"
    resolved_output.write_text(payload, encoding="utf-8")
    sys.stdout.write(payload)
    return 0 if report["summary"]["status"] in {"passed", "planned",
                                                "skipped"} else 1


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
