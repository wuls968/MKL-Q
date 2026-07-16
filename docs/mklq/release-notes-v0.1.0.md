# MKL-Q v0.1.0 Release Notes

Release tag: `mklq-v0.1.0`.

## Package

- PyPI distribution: `mklq`.
- Supported platform: macOS ARM64 with Python 3.11–3.14.
- Import name: `cudaq`.
- Stable simulator target: `mklq-cpu`.
- Experimental mixed-path target: `mklq-metal`.

Install into a new virtual environment:

```bash
python3 -m venv .venv
source .venv/bin/activate
python -m pip install --upgrade pip
python -m pip install mklq
```

Do not install `mklq` beside NVIDIA `cudaq` or `cuda-quantum*` distributions.
MKL-Q rejects that conflict at import time and explains how to use an isolated
virtual environment.

## Validation

The release workflow first publishes an RC to TestPyPI, installs it in a fresh
environment, and verifies `mklq-cpu`; `mklq-metal` receives an experimental
smoke test only. The final PyPI publication uses the same source commit after
the TestPyPI gate passes.

The release assets include the macOS ARM64 wheel, `SHA256SUMS`, and GitHub
artifact provenance. GitHub provides the matching source archive. The wheel
does not include `nvq++`, a `.pkg`, a `.dmg`, or an installer.

## Known Limitations

`mklq-metal` remains experimental and mixed-path. It is not a statement of
full Metal-native execution, default readiness, or performance certification.
C++ users must continue to build from source to obtain `nvq++`.
