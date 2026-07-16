"""Prevent the MKL-Q wheel from silently sharing ``cudaq`` with CUDA-Q."""

from __future__ import annotations

from collections.abc import Iterable, Mapping
from importlib import metadata


CUDAQ_PACKAGE = "cudaq"
CUDAQ_DISTRIBUTION_FAMILY = ("cudaq", "cuda-quantum")


def _normalize_distribution_name(name: str) -> str:
    return name.lower().replace("_", "-")


def conflicting_distribution_names(
        package_owners: Mapping[str, Iterable[str]],
        *,
        current_distribution: str = "mklq") -> list[str]:
    """Return distributions other than MKL-Q that own the ``cudaq`` package."""
    current = _normalize_distribution_name(current_distribution)
    owners = package_owners.get(CUDAQ_PACKAGE, ())
    return sorted({
        _normalize_distribution_name(owner)
        for owner in owners
        if _normalize_distribution_name(owner) != current
    })


def _installed_distribution_names() -> set[str]:
    names: set[str] = set()
    for distribution in metadata.distributions():
        name = distribution.metadata.get("Name")
        if name:
            names.add(_normalize_distribution_name(name))
    return names


def conflicting_cudaq_distribution_family(
        installed_distributions: Iterable[str],
        *,
        current_distribution: str = "mklq") -> list[str]:
    """Return installed CUDA-Q distributions that cannot share MKL-Q's venv."""
    current = _normalize_distribution_name(current_distribution)
    conflicts: set[str] = set()
    for distribution in installed_distributions:
        normalized = _normalize_distribution_name(distribution)
        if normalized == current:
            continue
        if (normalized == CUDAQ_DISTRIBUTION_FAMILY[0] or
                normalized == CUDAQ_DISTRIBUTION_FAMILY[1] or
                normalized.startswith(CUDAQ_DISTRIBUTION_FAMILY[1] + "-")):
            conflicts.add(normalized)
    return sorted(conflicts)


def ensure_mklq_distribution_ownership(
        package_owners: Mapping[str, Iterable[str]] | None = None,
        *,
        installed_distributions: set[str] | None = None) -> None:
    """Raise a direct error when ``mklq`` is installed beside CUDA-Q."""
    installed = (installed_distributions if installed_distributions is not None
                 else _installed_distribution_names())
    normalized_installed = {_normalize_distribution_name(name) for name in installed}
    if "mklq" not in normalized_installed:
        return

    owners = (package_owners if package_owners is not None else
              metadata.packages_distributions())
    conflicts = sorted(set(
        conflicting_cudaq_distribution_family(normalized_installed) +
        conflicting_distribution_names(owners)))
    if conflicts:
        rendered = ", ".join(conflicts)
        raise ImportError(
            "MKL-Q (`mklq`) cannot share the `cudaq` Python package with "
            f"{rendered}. Create an isolated virtual environment and install "
            "only `mklq` there.")
