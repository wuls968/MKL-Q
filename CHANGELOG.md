# MKL-Q Changelog

This changelog records MKL-Q source milestones only. It does not announce
GitHub Releases, wheels, PyPI packages, installers, signed artifacts, or binary
distribution.

## mklq-v0.1.0-source - Planned Source-Only Tag

Status: tag preflight candidate; the tag has not been created.

Reference notes:
[`docs/mklq/release-notes-v0.1.0-source.md`](docs/mklq/release-notes-v0.1.0-source.md).

Scope:

- Public source fork of NVIDIA CUDA-Q with upstream history preserved.
- CUDA-Q-compatible public API surface: Python keeps `cudaq`, and C++ keeps
  `nvq++`.
- Stable local simulator target: `mklq-cpu`.
- Experimental mixed-path simulator target: `mklq-metal`.
- Source-only validation and audit tooling for public repository hygiene,
  Apple Silicon local correctness, benchmark evidence boundaries, branch
  protection, and issue triage.

Non-goals for this planned tag:

- No GitHub Release.
- No wheel, PyPI package, installer, Homebrew formula, or signed artifact.
- No release certification for `mklq-metal`.
- No cross-machine performance certification from local benchmark summaries.

## source-only-rc-v0.1 - Documentation Candidate

Status: tracked documentation label, not a tag or package version.

Reference notes:
[`docs/mklq/source-only-rc-v0.1.md`](docs/mklq/source-only-rc-v0.1.md).

This candidate records the first public source-only readiness posture for
MKL-Q. It is useful for source inspection, local Apple Silicon builds, issue
triage, and continued development.
