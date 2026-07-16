# MKL-Q Changelog

## mklq-v0.1.0

Status: package-release contract prepared. The tag and release assets are
created only by the protected package-release workflow after TestPyPI,
Apple Silicon, and artifact verification gates pass.

Scope:

- macOS ARM64 Python distribution `mklq` with the CUDA-Q-compatible `cudaq`
  import namespace.
- Python 3.11–3.14; stable `mklq-cpu` and experimental `mklq-metal` targets.
- Wheel assets, `SHA256SUMS`, GitHub provenance, and GitHub source archive.
- No `nvq++`, installer, `.pkg`, `.dmg`, Linux/Windows wheel, or Metal release
  certification.

Release notes: [`docs/mklq/release-notes-v0.1.0.md`](docs/mklq/release-notes-v0.1.0.md).

## Historical source-only candidate

`source-only-rc-v0.1` and `mklq-v0.1.0-source` record the earlier pre-package
validation phase. They are historical evidence, not current release policy.
