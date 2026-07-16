# MKL-Q Public Release Readiness

This page defines the release-ready repository contract. It is not evidence
that `mklq-v0.1.0` has already been tagged or published.

Passing the repository gates is not a release certification: a named tag,
trusted publication, wheel verification, and the required fresh-environment
smoke tests remain mandatory for each release.

## Supported package surface

- Distribution: `mklq`; import namespace: `cudaq`.
- Platform: macOS ARM64 with Python 3.11–3.14.
- Stable target: `mklq-cpu`.
- Experimental target: `mklq-metal`, which is not default-ready or fully
  Metal-native.
- The wheel excludes `nvq++`, installers, Linux/Windows support, and native
  CUDA GPU support.

## Required repository gates

- `MKL-Q repository checks` on the exact pushed commit.
- Manual `MKL-Q Apple Silicon correctness` with `run_full_gate=confirm` on the
  exact release commit.
- `run_package_release_audit.py`, clean public healthcheck, wheel inspection,
  fresh virtual-environment smoke tests, checksums, and provenance.
- GitHub branch protection must match
  [`.github/branch-protection-main.json`](../../.github/branch-protection-main.json).

## Publication controls

The package workflow is manual-only and uses protected `testpypi` and `pypi`
environments with Trusted Publishing. It verifies that an existing
`mklq-vX.Y.Z` tag points to `origin/main`, builds only on the trusted Apple
Silicon runner, then publishes TestPyPI RCs or final PyPI wheels.

The workflow does not create an unsigned `.pkg` or `.dmg`. A signed native
installer requires a separate Developer ID and notarization release plan.

## Historical evidence

The source-only RC documents and historical Apple Silicon gate records remain
useful provenance for earlier commits. They are not a live release status line;
use current audits and workflow runs for any tag or publication decision.
