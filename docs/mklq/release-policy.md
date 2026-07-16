# MKL-Q Package Release Policy

MKL-Q publishes a Python runtime distribution named `mklq` for macOS ARM64.
The public Python import remains `cudaq` for CUDA-Q compatibility, so `mklq`
must be installed in an isolated virtual environment and must never be
presented as an NVIDIA CUDA-Q distribution.

The source-only phase is historical evidence for earlier MKL-Q commits; it is
not the release policy for `mklq-v0.1.0`.

## v0.1.0 Scope

- Release tag format: `mklq-vX.Y.Z`; the first final tag is `mklq-v0.1.0`.
- Python distribution: `mklq`, supporting Python `>=3.11,<3.15` on macOS
  ARM64 only.
- Stable target: `mklq-cpu`.
- Experimental target: `mklq-metal`; it is not default-ready, fully
  Metal-native, or release-certified.
- Wheel contents: Python runtime plus the `mklq-cpu` and `mklq-metal` target
  libraries/configurations. The wheel does not provide `nvq++`.
- Release assets: wheels, `SHA256SUMS`, GitHub artifact provenance, and
  GitHub's source archive for the release tag.

## Publication Sequence

1. Build `0.1.0rc1` from the release commit for Python 3.11–3.14 on trusted
   macOS ARM64 runners. Each interpreter is provisioned by the runner image,
   verified as native arm64 (not Rosetta), and builds its own wheel, manifest,
   `lipo -archs` check, and fresh-environment smoke test.
2. Publish all RC wheels through the protected `testpypi` environment. For
   each Python version, download and reinstall the exact version from the
   TestPyPI index in a clean virtual environment; verify the downloaded SHA256
   against the build manifest, `cudaq.__version__`, the exact two-target set,
   the CPU/experimental-Metal smoke tests, and the release tag commit SHA.
3. Create `mklq-v0.1.0` on the same verified commit, build `0.1.0`, and
   publish through the protected `pypi` environment. Repeat the clean PyPI
   index installation, checksum, target-set, smoke, and tag-SHA checks.
4. Create the GitHub prerelease/release only after the applicable index
   verification, checksum generation, and provenance attestation succeed.

PyPI and TestPyPI must use Trusted Publishing. No API token, Developer ID,
notarization credential, or release signing secret may be committed to this
repository or injected into local scripts.

## Required Gates

Before an RC or final release, all of the following must pass for the exact
tagged commit:

- `python3 benchmarks/mklq/run_package_release_audit.py --version <version>`;
- `python3 benchmarks/mklq/run_public_healthcheck.py --full --require-clean`;
- the manual Apple Silicon correctness workflow with `run_full_gate=confirm`;
- a fresh virtual-environment installation and `mklq-cpu` Bell smoke test;
- an experimental `mklq-metal` smoke test with no claim beyond that scope;
- wheel inspection, `delocate` dependency repair, `SHA256SUMS`, native-arm64
  `lipo -archs` verification, and artifact provenance verification;
- a clean index-specific post-publication reinstall that verifies the exact
  distribution version, downloaded wheel SHA256, MKL-Q target set, and tag
  commit SHA.

## Explicit Non-goals

- No Linux or Windows wheel.
- No `nvq++` or C++ toolchain in the Python wheel.
- No `.pkg`, `.dmg`, Homebrew formula, or unsigned native installer.
- No native installer until a Developer ID signing identity and notarization
  profile are available and independently validated.
- No cross-machine performance certification or broad Metal release claim.

## Incident Handling

Use GitHub private vulnerability reporting for security issues. If a published
wheel must be withdrawn, mark the GitHub Release as draft or yanked and yank
the PyPI version; publish the repair as a new `.postN` version. Never delete
or overwrite a published PyPI file.
