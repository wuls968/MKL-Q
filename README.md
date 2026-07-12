# MKL-Q

MKL-Q is a CUDA-Q-compatible Apple Silicon fork focused on local simulator
performance for macOS ARM64. It keeps the CUDA-Q C++ and Python public API
surface, including the `cudaq` Python namespace and the `nvq++` compiler, while
adding MKL-Q targets for Apple Silicon development.

This repository is source-only for the first public version. It does not publish
PyPI wheels, binary packages, or GitHub releases yet.

## Targets

- `mklq-cpu`: stable Apple Silicon CPU simulator target. It uses a native fp64
  state-vector backend with focused fast paths for common gates, measurement,
  and sampling.
- `mklq-metal`: experimental Apple GPU target. It uses a mixed Metal/CPU path
  for supported operations and falls back to the MKL-Q CPU oracle when needed.
  It is not the default target.

The upstream CUDA-Q targets, including `qpp-cpu` and density-matrix CPU targets,
remain available so that MKL-Q can keep CUDA-Q compatibility and upstream sync
ability.

## Build

MKL-Q is intended to be built from source on Apple Silicon macOS. The default
development install prefix is `${HOME}/.cudaq-mklq`; override it through CMake
if you need a different prefix.

Install Xcode Command Line Tools before configuring (`xcode-select --install`).
MKL-Q uses the selected compiler runtime when it is complete and falls back to
the active Xcode runtime when a custom LLVM install omits compiler-rt.

```bash
git clone --recursive https://github.com/wuls968/MKL-Q.git
cd MKL-Q

cmake -S . -B build-python -D CUDAQ_ENABLE_MKLQ_BACKEND=ON \
  -D CMAKE_INSTALL_PREFIX="${HOME}/.cudaq-mklq"
cmake --build build-python --target install -j 6
python3 benchmarks/mklq/repair_macos_install_signatures.py \
  --install-prefix "${HOME}/.cudaq-mklq"
```

If you use a fork or a different GitHub owner, replace the clone URL with your
repository URL.

The signature repair command refreshes ad-hoc code signatures for local macOS
install-prefix dylibs, Python extension loadables, and `bin/` Mach-O
executables so that Python imports and `nvq++` smoke executables can load the
installed runtime libraries. It is not release artifact signing.

## Python Smoke Test

```bash
PYTHONPATH="${HOME}/.cudaq-mklq" python3 - <<'PY'
import cudaq

@cudaq.kernel
def bell():
    q = cudaq.qvector(2)
    h(q[0])
    x.ctrl(q[0], q[1])
    mz(q)

for target in ("mklq-cpu", "mklq-metal"):
    cudaq.set_target(target)
    counts = cudaq.sample(bell, shots_count=100)
    print(target, counts)
PY
```

## C++ Smoke Test

```bash
cat > /tmp/mklq_bell.cpp <<'CPP'
#include <cudaq.h>

struct bell {
  void operator()() __qpu__ {
    cudaq::qvector q(2);
    h(q[0]);
    x<cudaq::ctrl>(q[0], q[1]);
    mz(q);
  }
};

int main() {
  auto counts = cudaq::sample(100, bell{});
  counts.dump();
}
CPP

"${HOME}/.cudaq-mklq/bin/nvq++" --target mklq-cpu /tmp/mklq_bell.cpp -o /tmp/mklq_bell_cpu
/tmp/mklq_bell_cpu

"${HOME}/.cudaq-mklq/bin/nvq++" --target mklq-metal /tmp/mklq_bell.cpp -o /tmp/mklq_bell_metal
/tmp/mklq_bell_metal
```

## Examples

Runnable Python and C++ examples are tracked under
[`examples/mklq`](examples/mklq/). They cover Bell/GHZ entanglement,
parameterized rotations, controlled-phase kickback, and a deterministic
Clifford-chain fixture through the same `cudaq` Python namespace and
`nvq++ --target mklq-cpu|mklq-metal` interface as the smoke tests above.
To verify all public examples locally, run:

```bash
python3 examples/mklq/verify_examples.py --install-prefix "${HOME}/.cudaq-mklq"
```

## Validation

The current bootstrap validation record is summarized in
[`docs/mklq/validation.md`](docs/mklq/validation.md). The development roadmap
and known backend limits are summarized in
[`docs/mklq/roadmap.md`](docs/mklq/roadmap.md).
The architecture boundary for the CUDA-Q fork, MKL-Q targets, CPU oracle, and
experimental Metal path is described in
[`docs/mklq/architecture.md`](docs/mklq/architecture.md).
The test and benchmark coverage map is tracked in
[`docs/mklq/testing-matrix.md`](docs/mklq/testing-matrix.md).
The upstream CUDA-Q sync procedure is tracked in
[`docs/mklq/upstream-sync.md`](docs/mklq/upstream-sync.md).
The source-only release policy and future release entry criteria are tracked in
[`docs/mklq/release-policy.md`](docs/mklq/release-policy.md).
The current source-only v0.1 release-candidate entry point is tracked in
[`docs/mklq/source-only-rc-v0.1.md`](docs/mklq/source-only-rc-v0.1.md).
The planned source-only v0.1.0 tag draft is tracked in
[`docs/mklq/release-notes-v0.1.0-source.md`](docs/mklq/release-notes-v0.1.0-source.md),
with milestone history in [`CHANGELOG.md`](CHANGELOG.md). This is tag
preflight material only; no tag, GitHub Release, wheel, PyPI package,
installer, or signed artifact has been created.
The public support boundary and current non-goals are listed in
[`docs/mklq/known-limitations.md`](docs/mklq/known-limitations.md).
The source-only public release checklist is tracked in
[`docs/mklq/public-release-checklist.md`](docs/mklq/public-release-checklist.md).
Contributor and fork maintenance workflow notes are tracked in
[`docs/mklq/developer-workflow.md`](docs/mklq/developer-workflow.md).
Maintainer operations, issue triage, PR gates, recovery steps, and validation
cadence are tracked in
[`docs/mklq/maintainer-runbook.md`](docs/mklq/maintainer-runbook.md).
The issue label taxonomy used for public triage is tracked in
[`docs/mklq/issue-labels.md`](docs/mklq/issue-labels.md).
The intended `main` branch protection policy is tracked in
[`docs/mklq/branch-protection.md`](docs/mklq/branch-protection.md).
The manual self-hosted Apple Silicon correctness CI workflow and activation
boundary are tracked in
[`docs/mklq/apple-silicon-ci.md`](docs/mklq/apple-silicon-ci.md).
The current public repository readiness snapshot is tracked in
[`docs/mklq/public-readiness.md`](docs/mklq/public-readiness.md).
The experimental `mklq-metal` execution and CPU fallback boundary is tracked in
[`docs/mklq/metal-execution-boundary.md`](docs/mklq/metal-execution-boundary.md).

Sanitized local benchmark evidence is kept under `benchmarks/mklq/reports/`.
The public benchmark evidence index is summarized in
[`docs/mklq/benchmark-evidence.md`](docs/mklq/benchmark-evidence.md).
Tracked CPU sampling phase counter coverage is summarized in
[`docs/mklq/cpu-sampling-counters.md`](docs/mklq/cpu-sampling-counters.md).
Tracked CPU gate fast-path counter coverage is summarized in
[`docs/mklq/cpu-gate-counters.md`](docs/mklq/cpu-gate-counters.md).
Tracked Metal runtime counter coverage is summarized in
[`docs/mklq/metal-runtime-counters.md`](docs/mklq/metal-runtime-counters.md).
Raw local benchmark payloads under `benchmarks/mklq/results/` are intentionally
ignored.

To verify that tracked clean CPU benchmark summaries still satisfy the public
evidence floor without running new benchmarks:

```bash
python3 benchmarks/mklq/check_performance_evidence.py
```

To verify that tracked experimental Metal summaries keep the mixed-path,
not-release-ready evidence boundary:

```bash
python3 benchmarks/mklq/check_metal_evidence.py
```

To verify the current tracked experimental Metal stochastic sampling summaries
that document selected full-register and partial-register Metal sample-count
accumulation after host-generated draws at q20/q22/q24. Newer runtime counter
tests track selected counts-only Metal device-generated draw plus sample-count
accumulation, including a uniform-probability generated-count fast path:

```bash
python3 benchmarks/mklq/check_metal_sampling_boundary_evidence.py
```

To verify the focused uniform-probability partial-register Metal sampling
evidence for q20/q22/q24:

```bash
python3 benchmarks/mklq/check_metal_uniform_sampling_evidence.py
```

Current selected Metal runtime counter coverage, including full-register and
partial-register counts-only Metal device-generated draw plus sample-count
accumulation and the uniform-probability generated-count fast path, is checked
through
`docs/mklq/metal-runtime-counters.md`.

For a fast local public-maintenance gate, run:

```bash
python3 benchmarks/mklq/run_preflight_audit.py
python3 benchmarks/mklq/run_public_healthcheck.py
```

For a heavier pre-publication local gate that also builds and runs correctness
checks, use:

```bash
python3 benchmarks/mklq/run_public_healthcheck.py --full --require-clean
```

To audit the public GitHub fork metadata, branch protection, latest hygiene
workflow, and source-only tag/release boundary, run:

```bash
python3 benchmarks/mklq/run_public_readiness_audit.py
```

To audit the planned `mklq-v0.1.0-source` tag draft without creating a tag, run:

```bash
python3 benchmarks/mklq/run_source_release_tag_audit.py
```

## Reporting Issues

Use the public [MKL-Q issue tracker](https://github.com/wuls968/MKL-Q/issues)
for reproducible bugs and focused feature requests. Include the source commit,
macOS and Apple Silicon model, target name, build or run command, and the
smallest relevant output. Security reports should follow [`SECURITY.md`](SECURITY.md)
instead of being filed as public issues.

## Upstream And License

MKL-Q is derived from NVIDIA CUDA-Q and keeps CUDA-Q API compatibility where
possible. The upstream project is available at
<https://github.com/NVIDIA/cuda-quantum>.

This repository is licensed under the Apache License 2.0. See
[`LICENSE`](LICENSE) and [`NOTICE`](NOTICE) for upstream attribution and MKL-Q
modification notices.
