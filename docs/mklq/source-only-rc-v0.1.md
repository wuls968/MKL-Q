# MKL-Q Source-Only RC v0.1

This page is the source-only release-candidate entry point for MKL-Q v0.1. It
is a documentation label for the current public source posture, not a Git tag,
GitHub Release, wheel, PyPI package, installer, or signed artifact.

## Status

- Candidate label: `source-only-rc-v0.1`
- Release type: source-only public fork readiness candidate
- Public repository: `wuls968/MKL-Q`
- Default branch: `main`
- Upstream parent: `NVIDIA/cuda-quantum`
- Public API namespace: CUDA-Q-compatible `cudaq`
- C++ compiler entry point: `nvq++`
- Stable MKL-Q target: `mklq-cpu`
- Experimental MKL-Q target: `mklq-metal`

The candidate is suitable for public source inspection, local Apple Silicon
source builds, issue triage, and source-only development. It is not a binary
release and must not be described as release certification or performance
certification.

## Current Verified Baseline

The documented verified public baseline was collected on 2026-07-06:

- Verified branch head:
  `7902659bce562702147a4ae2862818861f8992c8`
- Public hygiene workflow:
  <https://github.com/wuls968/MKL-Q/actions/runs/28800917068>
- Manual Apple Silicon full gate:
  <https://github.com/wuls968/MKL-Q/actions/runs/28800993186>
- Public readiness audit output:
  `/tmp/mklq-public-readiness-audit-final-rc-v0.1-2026-07-06.json`
- Public readiness audit result: 13/13 checks passed
- Source tag preflight audit output:
  `/tmp/mklq-source-release-tag-post-apple-gate-2026-07-06.json`
- Source tag preflight audit result: 8/8 checks passed
- Full public healthcheck result: 36/36 steps passed
- Correctness gate ctest subset: 104/104 tests passed
- Metal runtime counter probe: 50/50 selected counter tests passed
- Public example smoke: 30/30 checks passed
- Benchmark harness tests: 228 passed

This tracked page is an evidence index, not a live status endpoint. The live
audits below are authoritative for the exact current commit. Before using a
newer commit as the documented source-only RC baseline, rerun the commands
below and update the evidence references if the commit, workflow run IDs, or
gate totals changed.

## Acceptance Gates

Run these checks before describing a commit as the current source-only RC
baseline:

```bash
git status --short --branch
git rev-parse HEAD
git ls-remote origin refs/heads/main
python3 benchmarks/mklq/run_preflight_audit.py --require-clean
python3 benchmarks/mklq/run_public_release_checklist_audit.py
python3 benchmarks/mklq/run_public_healthcheck.py
python3 benchmarks/mklq/run_public_readiness_audit.py
```

For the Apple Silicon full gate, dispatch the manual workflow only from a
trusted maintainer context:

```bash
python3 benchmarks/mklq/run_self_hosted_ci_audit.py --check-runners \
  --repo wuls968/MKL-Q
gh workflow run "MKL-Q Apple Silicon correctness" \
  --repo wuls968/MKL-Q --ref main -f run_full_gate=confirm
gh run watch <run-id> --repo wuls968/MKL-Q --exit-status
python3 benchmarks/mklq/run_public_readiness_audit.py
```

The RC baseline is acceptable only when `origin/main` equals local `HEAD`, the
public hygiene workflow succeeds for that `HEAD`, the manual Apple Silicon full
gate succeeds for that `HEAD`, and the public readiness audit passes.

## Claim Boundary

Allowed wording:

- "MKL-Q has a source-only v0.1 release-candidate baseline."
- "The source tree can be built locally on Apple Silicon from the documented
  commands."
- "`mklq-cpu` is the stable local simulator target."
- "`mklq-metal` is experimental and mixed-path."

Forbidden wording:

- "MKL-Q v0.1 is released."
- "MKL-Q publishes wheels, PyPI packages, installers, or signed artifacts."
- "`mklq-metal` is complete on Apple GPU, selected by default, or certified
  for release artifacts."
- "The local benchmark summaries certify cross-machine performance."
- "The self-hosted Apple Silicon full gate is a public hosted CI replacement."

## No Artifact Policy

Do not create or publish these for `source-only-rc-v0.1`:

- Git tags
- GitHub Releases
- wheels
- PyPI packages
- binary installers
- Homebrew formulae
- checksums for local build products
- notarized or signed release artifacts

The only public artifacts for this candidate are the source tree, tracked
documentation, tracked workflow configuration, and sanitized benchmark
summaries under `benchmarks/mklq/reports/`.

## Promotion Requirements

Before MKL-Q can move from `source-only-rc-v0.1` to a real tagged release, at
minimum:

- `docs/mklq/release-policy.md` must be updated with an accepted release plan.
- `docs/mklq/public-release-checklist.md` must be completed for the exact
  release commit.
- `docs/mklq/validation.md` and `docs/mklq/public-readiness.md` must cite the
  exact release commit and successful gates.
- A packaging/signing policy must exist before any binary artifact is attached.
- `mklq-metal` must remain experimental unless a separate Metal
  release-readiness plan passes.
- The public claim-boundary guard must pass without weakening the current
  source-only, non-certification language.
