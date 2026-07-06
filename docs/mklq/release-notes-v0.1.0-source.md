# MKL-Q v0.1.0 Source-Only Release Notes

These notes describe the planned `mklq-v0.1.0-source` source-only tag. The tag
has not been created. This page is not a GitHub Release, wheel, PyPI package,
installer, signed artifact, checksum manifest, or binary distribution.

## Status

- Planned tag: `mklq-v0.1.0-source`
- Release shape: source-only tag candidate
- Public repository: `wuls968/MKL-Q`
- Upstream parent: `NVIDIA/cuda-quantum`
- Default branch before tagging: `main`
- Public Python namespace: CUDA-Q-compatible `cudaq`
- C++ compiler entry point: `nvq++`
- Stable MKL-Q target: `mklq-cpu`
- Experimental MKL-Q target: `mklq-metal`

Use these notes as the human-readable release note draft for a future
source-only tag. Do not create the tag until the tag preflight audit, public
readiness audit, latest public hygiene workflow, and manual Apple Silicon full
gate all pass for the exact commit.

## Included In Scope

- Preserves CUDA-Q upstream history and sync ability.
- Adds MKL-Q Apple Silicon simulator targets while keeping upstream CUDA-Q
  target compatibility.
- Documents local source build and install-prefix signature repair for macOS
  ARM64 development.
- Provides Python and C++ smoke paths for `mklq-cpu` and `mklq-metal`.
- Publishes sanitized benchmark summaries under `benchmarks/mklq/reports/`.
- Tracks source-only public hygiene, readiness, release policy, branch
  protection, issue labels, Apple Silicon CI activation boundaries, and
  benchmark evidence boundaries under `docs/mklq/`.
- Keeps full Apple Silicon correctness validation manual-only through
  `workflow_dispatch` and `run_full_gate=confirm`.

## Explicitly Out Of Scope

- No GitHub Release.
- No wheels or PyPI package.
- No binary installer, Homebrew formula, or signed release artifact.
- No checksums for local build products.
- No claim that `mklq-metal` is complete, default, full Metal-native, or ready
  as a release-artifact backend.
- No cross-machine performance certification.
- No replacement of the CUDA-Q public API namespace.

## Validation Required Before Tagging

The exact candidate commit must satisfy all of the following:

```bash
git status --short --branch
git rev-parse HEAD
git ls-remote origin refs/heads/main
python3 benchmarks/mklq/run_preflight_audit.py --require-clean
python3 benchmarks/mklq/run_public_release_checklist_audit.py
python3 benchmarks/mklq/run_source_release_tag_audit.py
python3 benchmarks/mklq/run_public_healthcheck.py --full --require-clean
python3 benchmarks/mklq/run_public_readiness_audit.py
```

The manual Apple Silicon full gate must also pass on the pushed `main` commit:

```bash
python3 benchmarks/mklq/run_self_hosted_ci_audit.py --check-runners \
  --repo wuls968/MKL-Q
gh workflow run "MKL-Q Apple Silicon correctness" \
  --repo wuls968/MKL-Q --ref main -f run_full_gate=confirm
gh run watch <run-id> --repo wuls968/MKL-Q --exit-status
```

## Current Evidence Snapshot

As of 2026-07-06, the latest verified source-only baseline on `main` is:

- Commit: `206d392fc30019f6934965ec88ae18d30c87324d`
- Public hygiene workflow:
  <https://github.com/wuls968/MKL-Q/actions/runs/28784573258>
- Manual Apple Silicon full gate:
  <https://github.com/wuls968/MKL-Q/actions/runs/28784584186>
- Full public healthcheck: 35/35 steps passed
- Correctness gate: 4/4 steps passed
- Metal runtime counter probe: 50/50 selected counter tests passed
- Public readiness audit: 13/13 checks passed

This snapshot is evidence for the named commit only. Rerun the tag preflight
and public readiness audits if any commit, workflow run, branch-protection
state, or release policy changes.

## Promotion Boundary

Creating `mklq-v0.1.0-source` later would mark a source snapshot only. It must
not create a GitHub Release, upload artifacts, publish packages, weaken branch
protection, change `mklq-metal` from experimental to default, or imply binary
support.

Any future wheel, PyPI, installer, or signed artifact needs a separate reviewed
packaging plan and a release-policy update before publication.
