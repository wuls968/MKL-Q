#!/usr/bin/env python3
"""Validate release versions and inspect macOS ARM64 MKL-Q wheels."""

from __future__ import annotations

import argparse
import hashlib
import json
import re
import zipfile
from pathlib import Path
from typing import Any


SCHEMA_VERSION = "mklq-wheel-manifest-v1"
RELEASE_VERSION_PATTERN = re.compile(
    r"^\d+\.\d+\.\d+(?:(?:a|b|rc)\d+)?(?:\.post\d+)?$")
REQUIRED_TARGETS = {
    "mklq-cpu": "libnvqir-mklq_cpu",
    "mklq-metal": "libnvqir-mklq_metal",
}
ALLOWED_TARGET_CONFIGS = frozenset(REQUIRED_TARGETS)
ALLOWED_NVQIR_LIBRARY_STEMS = frozenset({
    "libnvqir",
    *REQUIRED_TARGETS.values(),
})
ALLOWED_CUDAQ_RUNTIME_LIBRARY_STEMS = frozenset({
    "libcudaq",
    "libcudaq-builder",
    "libcudaq-common",
    "libcudaq-device-call-runtime",
    "libcudaq-em-default",
    "libcudaq-ensmallen",
    "libcudaq-logger",
    "libcudaq-mlir-runtime",
    "libcudaq-nlopt",
    "libcudaq-operator",
    "libcudaq-platform-default",
    "libcudaq-py-utils",
    "libcudaq-python-interop",
})
# Delocate can bundle the Homebrew OpenMP runtime. No other third-party native
# DSO is permitted without an explicit review and a matching contract test.
ALLOWED_BUNDLED_DEPENDENCY_LIBRARY_STEMS = frozenset({
    "libomp",
})
ALLOWED_MLIR_LIBRARY_STEMS = frozenset({
    "libCUDAQuantumPythonCAPI",
    "libMLIRPythonSupport-cudaq",
    "libnanobind-cudaq",
})
# Exact extension module basenames emitted by the MLIRPythonSources consumed in
# python/extension/CMakeLists.txt. Keep this list explicit: a path below
# cudaq/mlir/_mlir_libs is not by itself an authorization to ship a native DSO.
ALLOWED_PYTHON_EXTENSION_STEMS = frozenset({
    "_mlir",
    "_mlirAsyncPasses",
    "_mlirDialectsAMDGPU",
    "_mlirDialectsGPU",
    "_mlirDialectsIRDL",
    "_mlirDialectsLLVM",
    "_mlirDialectsLinalg",
    "_mlirDialectsNVGPU",
    "_mlirDialectsPDL",
    "_mlirDialectsQuant",
    "_mlirDialectsSMT",
    "_mlirDialectsSparseTensor",
    "_mlirDialectsTransform",
    "_mlirExecutionEngine",
    "_mlirGPUPasses",
    "_mlirLinalgPasses",
    "_mlirRegisterEverything",
    "_mlirSparseTensorPasses",
    "_mlirTransformInterpreter",
    "_quakeDialects",
})
FORBIDDEN_BACKEND_LIBRARY_PREFIXES = (
    "libcudaq-platform-mqpu",
    "libcudaq-rest-qpu",
    "libcudaq-orca-qpu",
    "libcudaq-fermioniq-qpu",
    "libcudaq-pasqal-qpu",
    "libcudaq-quera-qpu",
    "libcudaq-pyscf",
    "libcudaq-serverhelper-",
    "libcudaq-comm-plugin",
)


def validate_release_version(version: str) -> str:
    """Return a release-safe PEP 440 version used by MKL-Q automation."""
    if not RELEASE_VERSION_PATTERN.fullmatch(version):
        raise ValueError(
            "MKLQ_VERSION must be a release PEP 440 version such as "
            "0.1.0, 0.1.0rc1, or 0.1.0.post1")
    return version


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def _unsafe_archive_paths(names: list[str]) -> list[str]:
    unsafe: list[str] = []
    for name in names:
        parts = Path(name).parts
        if name.startswith(("/", "\\")) or ".." in parts or "Users" in parts:
            unsafe.append(name)
    return sorted(unsafe)


def _matches_library_stem(name: str, stem: str) -> bool:
    """Match an exact DSO stem without accepting arbitrary prefixes."""
    return (
        name == f"{stem}.dylib"
        or name == f"{stem}.so"
        or name.startswith(f"{stem}.") and name.endswith((".dylib", ".so"))
    )


def _is_native_dso(name: str) -> bool:
    return name.endswith((".dylib", ".so"))


def _is_allowed_native_library(name: str) -> bool:
    allowed_stems = (
        ALLOWED_NVQIR_LIBRARY_STEMS
        | ALLOWED_CUDAQ_RUNTIME_LIBRARY_STEMS
        | ALLOWED_BUNDLED_DEPENDENCY_LIBRARY_STEMS
        | ALLOWED_MLIR_LIBRARY_STEMS
    )
    return any(_matches_library_stem(name, stem) for stem in allowed_stems)


def _is_allowed_python_extension(path: str) -> bool:
    """Allow only the MLIR extension DSOs installed by this package build."""
    parts = Path(path).parts
    return (
        len(parts) >= 4
        and parts[:3] == ("cudaq", "mlir", "_mlir_libs")
        and any(_matches_library_stem(Path(path).name, stem)
                for stem in ALLOWED_PYTHON_EXTENSION_STEMS)
    )


def _is_forbidden_backend_library(name: str) -> bool:
    if not _is_native_dso(name):
        return False
    if name.startswith("libnvqir"):
        return not any(_matches_library_stem(name, stem)
                       for stem in ALLOWED_NVQIR_LIBRARY_STEMS)
    if name.startswith("libcudaq-em-"):
        return not _matches_library_stem(name, "libcudaq-em-default")
    if name.startswith("libcudaq-platform-"):
        return not _matches_library_stem(name, "libcudaq-platform-default")
    return any(name.startswith(prefix)
               for prefix in FORBIDDEN_BACKEND_LIBRARY_PREFIXES)


def _non_mklq_backend_assets(
        names: list[str]) -> tuple[list[str], list[str], list[str]]:
    """Return unexpected configurations, forbidden backends, and unknown DSOs."""
    target_configs = {
        Path(name).stem for name in names
        if "/targets/" in name and name.endswith(".yml")
    }
    unexpected_configs = sorted(target_configs - ALLOWED_TARGET_CONFIGS)
    forbidden_libraries = sorted(
        name for name in names if _is_forbidden_backend_library(Path(name).name))
    unknown_native_libraries = sorted(
        name for name in names
        if _is_native_dso(Path(name).name)
        and not _is_forbidden_backend_library(Path(name).name)
        and not _is_allowed_native_library(Path(name).name)
        and not _is_allowed_python_extension(name))
    return unexpected_configs, forbidden_libraries, unknown_native_libraries


def inspect_wheel(wheel: Path, *, expected_version: str) -> dict[str, Any]:
    """Check a wheel contains both MKL-Q targets without unsafe path names."""
    expected_version = validate_release_version(expected_version)
    if wheel.suffix != ".whl" or not wheel.is_file():
        raise ValueError(f"wheel does not exist: {wheel}")
    if not wheel.name.startswith(f"mklq-{expected_version}-"):
        raise ValueError(
            f"wheel filename does not match expected mklq {expected_version}: "
            f"{wheel.name}")
    if "macosx" not in wheel.name or "arm64" not in wheel.name:
        raise ValueError(
            "MKL-Q release wheels must target macOS arm64, got " + wheel.name)

    with zipfile.ZipFile(wheel) as archive:
        names = sorted(archive.namelist())

    unsafe_paths = _unsafe_archive_paths(names)
    cli_paths = [name for name in names if Path(name).name == "nvq++"]
    unexpected_configs, forbidden_libraries, unknown_native_libraries = (
        _non_mklq_backend_assets(names))
    missing: list[str] = []
    targets: list[str] = []
    for target, library_stem in REQUIRED_TARGETS.items():
        has_library = any(
            _matches_library_stem(Path(name).name, library_stem)
            for name in names)
        has_config = any(Path(name).name == f"{target}.yml" for name in names)
        if not has_library:
            missing.append(f"{target} library")
        if not has_config:
            missing.append(f"{target} target configuration")
        if has_library and has_config:
            targets.append(target)

    if unsafe_paths:
        raise ValueError(
            "wheel contains unsafe absolute or traversal path(s): " +
            ", ".join(unsafe_paths))
    if cli_paths:
        raise ValueError(
            "MKL-Q Python wheel must not contain nvq++: " + ", ".join(cli_paths))
    if unexpected_configs:
        raise ValueError(
            "MKL-Q wheel contains unsupported target configuration(s): " +
            ", ".join(unexpected_configs))
    if forbidden_libraries:
        raise ValueError(
            "MKL-Q wheel contains forbidden backend library/libraries: " +
            ", ".join(forbidden_libraries))
    if unknown_native_libraries:
        raise ValueError(
            "MKL-Q wheel contains unsupported native library/libraries: " +
            ", ".join(unknown_native_libraries))
    if missing:
        raise ValueError(
            "MKL-Q wheel is missing required assets: " + ", ".join(missing))

    return {
        "schema_version": SCHEMA_VERSION,
        "wheel": {
            "filename": wheel.name,
            "size_bytes": wheel.stat().st_size,
            "sha256": sha256_file(wheel),
        },
        "targets": targets,
        "summary": {"status": "passed", "failed": 0},
    }


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Validate MKL-Q release versions and wheel contents.")
    subparsers = parser.add_subparsers(dest="command", required=True)
    version_parser = subparsers.add_parser("validate-version")
    version_parser.add_argument("version")
    wheel_parser = subparsers.add_parser("inspect-wheel")
    wheel_parser.add_argument("wheel", type=Path)
    wheel_parser.add_argument("--expected-version", required=True)
    wheel_parser.add_argument("--output", type=Path)
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    if args.command == "validate-version":
        print(validate_release_version(args.version))
        return 0

    report = inspect_wheel(args.wheel, expected_version=args.expected_version)
    payload = json.dumps(report, indent=2, sort_keys=True) + "\n"
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(payload, encoding="utf-8")
    else:
        print(payload, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
