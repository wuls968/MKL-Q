# MKL-Q v0.1.0 Public Release Checklist

## Release identity

- [ ] `git status --short --branch` is clean.
- [ ] Release commit is merged into protected `main`.
- [ ] RC tag is `mklq-v0.1.0rc1`; final tag is `mklq-v0.1.0` on the same
  verified commit.
- [ ] `pyproject.toml` identifies `mklq`, not NVIDIA `cudaq`.
- [ ] PyPI and TestPyPI Trusted Publishing are configured for
  `.github/workflows/mklq-package-release.yml` and protected `testpypi`/`pypi`
  environments.
- [ ] The trusted Apple Silicon runner explicitly provisions native-arm64
  Python 3.11, 3.12, 3.13, and 3.14 interpreters; no Rosetta Python is used.

## Artifact scope

- [ ] Build one macOS ARM64 wheel for each supported Python 3.11–3.14 runtime.
- [ ] Inspect each wheel with `package_release.py`; it contains the CPU and
  experimental Metal target assets, no unsafe path, and no `nvq++` promise.
- [ ] Run `delocate`, verify every wheel dylib/native extension with
  `lipo -archs`, create `SHA256SUMS`, and create GitHub provenance.
- [ ] Do not attach `.pkg`, `.dmg`, unsigned installer, or raw benchmark data.

## Validation

- [ ] `python3 benchmarks/mklq/run_package_release_audit.py --version 0.1.0`
  passes.
- [ ] `python3 benchmarks/mklq/run_public_healthcheck.py --full --require-clean`
  passes for the exact release commit.
- [ ] A manual `run_full_gate=confirm` Apple Silicon workflow succeeded for the
  same commit.
- [ ] A fresh offline wheel install passes `pip check`, `import cudaq`, and a
  `mklq-cpu` Bell smoke test.
- [ ] `mklq-metal` passes only its experimental smoke test; no broader release
  or performance claim is added.

## Publication and rollback

- [ ] Publish `0.1.0rc1` to TestPyPI and repeat fresh-environment smoke tests.
- [ ] For every Python wheel, reinstall `0.1.0rc1` from TestPyPI in a clean
  venv; verify the index-downloaded SHA256, `cudaq.__version__`, target set,
  CPU/experimental-Metal smoke, and tag SHA.
- [ ] Publish `0.1.0` to PyPI only after RC evidence is accepted.
- [ ] For every Python wheel, repeat the clean PyPI-index version/SHA/target
  set/smoke/tag-SHA verification before creating the GitHub Release.
- [ ] Create the GitHub prerelease/release from the same workflow and tag.
- [ ] If a defect is found, yank the PyPI version and GitHub release; publish a
  corrected `.postN` artifact instead of replacing files.
