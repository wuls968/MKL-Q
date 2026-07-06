# MKL-Q Validation

This page records the current local validation gate for the MKL-Q public
bootstrap. It is not a release certification and does not replace clean CI.
See [`known-limitations.md`](known-limitations.md) for the current support
boundary and evidence limits.

## Machine

- Host: Apple Silicon local development machine
- CPU: Apple M5, 10 logical cores
- Memory: 16 GB RAM
- OS: macOS 26.5.1
- Install prefix used for the public bootstrap gate: `/Users/a0000/.cudaq-mklq`

## Current Evidence Snapshot

Latest local correctness refresh: 2026-07-05. Latest public healthcheck
refresh: 2026-07-05.

The install-prefix build, one-command correctness gate, public example smoke
gate, default public healthcheck, and standalone install-prefix Python subset
were refreshed on the local MKL-Q branch after enforcing administrator branch
protection and switching maintainer flow to PR-first. The full public
healthcheck was refreshed after adding CPU probability-fill counter evidence.
The default public healthcheck and one-command correctness gate were refreshed
again on 2026-06-24 after syncing CUDA-Q upstream and before refreshing the
tracked bounded CPU/Metal counter reports.
The one-command correctness gate was refreshed again on 2026-06-28 after adding
the CPU hardware-efficient ansatz state and observable oracle fixture.
The one-command correctness gate was refreshed again on 2026-07-04 after
adding the CPU phase-family, controlled phase, partial-register bit-order, and
qpp-cpu marginal sampling oracle fixtures.
It was refreshed again on 2026-07-04 after adding the experimental
`mklq-metal` partial-register bit-order and qpp-cpu marginal sampling oracle
fixtures.
The full public healthcheck was refreshed again on 2026-07-05 on clean
`main` after adding the Metal uniform sampling evidence guard, confirming that
the install-prefix build, local signature repair, correctness gate, benchmark
harness tests, and public examples still pass together.
A source-only release-candidate dry run was also executed on 2026-07-05 on
clean `main` without creating tags, releases, wheels, installers, or signed
artifacts. The local full public healthcheck passed with 35/35 steps, the
public release checklist audit passed with 7/7 checks, the preflight audit
passed, and the public readiness audit passed with 13/13 checks. The live
self-hosted runner inventory check was refreshed on 2026-07-06 and reported one
online runner named `mklq-apple-silicon-a0000` with the required `self-hosted`,
`macOS`, `ARM64`, and `mklq-apple-silicon` labels. The latest manual
self-hosted Apple Silicon full gate passed on 2026-07-05 for
`d92e4c1494f5aa43afb01d469fd76ad37e7fe800` in
<https://github.com/wuls968/MKL-Q/actions/runs/28742962472>, with 35/35 public
healthcheck steps passed and the Metal runtime counter probe reporting 50
expected, 50 selected, 0 missing, and 50 passed.
The clean CPU benchmark summary was refreshed separately against
`dbebe3744f826ba4cbeed2b99708a2bdab03b11e` after promoting the two-qubit/SWAP
and three-qubit custom gate rows into the clean local CPU benchmark evidence
gate. Older clean summaries remain tracked as historical evidence for the
earlier single-control, QFT-like, seeded Clifford, and hardware-efficient ansatz
gates.
The focused two-qubit/SWAP and three-qubit CPU scaling summary was refreshed
separately against `cb688b20c825a970965ffe41ca84757287abf847`, covering
q18/q20/q22 `two-qubit-state` and `three-qubit-state` rows with a dedicated
public healthcheck guard.

The current refresh includes the earlier Metal counter-evidence work:
resident built-in Rx/Ry/Rz, controlled-Rx/Ry/Rz, phase-family S/T/Sdg/Tdg,
multi-control single-qubit resident, resident three-target gates, and
four-or-more-target unsupported gate fallback/reupload fixtures. The
2026-07-04 Metal counter refresh adds a requested-order partial-register
sampling fixture that asserts the resident marginal probability route, no
full-register probability fill, and no resident download for that selected
path. The later 2026-07-04 Metal counter refresh adds explicit host-side
sequential and counts-only draw telemetry for selected resident
partial-register sampling paths. A follow-up 2026-07-04 Metal counter refresh
adds native test-accessor timing accumulators for selected probability-fill,
host draw/count, and expectation-reduction sampling phases. The next
2026-07-04 Metal counter refresh adds a deterministic counts-only sampling
bypass fixture that materializes a one-outcome partial-register distribution
without entering the host draw loop after resident marginal probability work.
The next 2026-07-04 Metal counter refresh extends the same deterministic
one-outcome bypass to explicit sequential sampling and tightens deterministic
sparse full-register telemetry so only real stochastic draw loops increment
draw-batch counters.
The next 2026-07-04 Metal counter refresh adds resident full-register
stochastic sampling telemetry for explicit sequential and counts-only paths,
confirming resident full-register probability fill without host state download
while still counting host-side draw/count batches.
The next 2026-07-04 Metal runtime refresh moves the selected full-register
counts-only draw/count aggregation boundary forward: host-generated draws are
accumulated into outcome counts by a Metal sample-count kernel, while
sequential sampling and partial-register stochastic counts-only sampling still
used the host path in that report.
The next 2026-07-04 Metal runtime refresh extends that selected counts-only
sample-count accumulation path to partial-register sampling after resident
marginal probability work; random draws are still host-generated, and
sequential stochastic sampling remains host-side.
The follow-up 2026-07-04 public-healthcheck refresh adds a static Metal
stochastic sampling boundary guard for the tracked q20 counts-only
shot-scaling summary. The next 2026-07-04 evidence refresh adds tracked q22
`mklq-metal` counts-only shot-scaling rows for the same full-register and
partial-register sampling boundary; it remains local tuning evidence and does
not claim an on-device sampler.
The next 2026-07-04 Metal sampling evidence refresh replaces the default
sampling-boundary guard inputs with clean-worktree q20 and q22
`mklq-metal` summaries for the selected full-register Metal sample-count
accumulation path after host-generated draws. The same guard now permits
newer selected partial-register Metal sample-count accumulation wording after
host-generated draws while preserving the older partial-register host-side
summary boundary.
The 2026-07-05 Metal sampling evidence refresh adds clean-worktree q20/q22/q24
`mklq-metal` shot-scaling summaries for selected full-register and
partial-register Metal sample-count accumulation after host-generated draws,
and makes those summaries the default sampling-boundary guard inputs.
The next 2026-07-05 Metal runtime refresh adds selected counts-only
Metal-generated draw plus sample-count accumulation for full-register and
partial-register sampling, while sequential stochastic sampling remains
host-side.
The follow-up 2026-07-05 benchmark evidence refresh adds a focused
`sample-uniform-partial-register` fixture for q20/q22/q24 `mklq-metal`
counts-only sampling. It is local tuning evidence for the
uniform-probability generated-count fast path, not clean release provenance or
cross-machine performance certification.
A previous 2026-07-04 validation pass also reran the full
install/build/signature-repair/correctness/example gate on the current MKL-Q
branch after adding the local macOS install-prefix signature repair step.
The current tracked CPU gate counter evidence includes three bounded reports for
selected single-qubit, controlled single-qubit, single-control X/CNOT,
single-control H/Y/Rx/Ry, single-control Rz phase, two-qubit, three-qubit, and
composite fast-path counter ctests. The 2026-07-03 CPU gate counter refresh
covers per-gate single-control H/Y/Rx/Ry direct target/control pair fixtures,
on top of the 2026-07-02 single-control X/CNOT direct pair refresh and the
earlier single-control Rz direct phase path. The current tracked CPU
sampling/probability counter evidence includes three bounded reports, each with
explicit full-register and
marginal probability-fill counter ctests alongside the existing sampling phase
counter ctests. The current tracked Metal runtime counter evidence includes
thirteen bounded reports. Counter-summary aggregate counts are summed
across tracked reports, so repeated daily probes intentionally count the same
selected tests once per report.

Raw wrapper output was written to ignored local paths
`benchmarks/mklq/results/public-healthcheck-2026-06-24.json`,
`benchmarks/mklq/results/public-healthcheck-2026-07-01.json`,
`benchmarks/mklq/results/public-healthcheck-after-full-doc-refresh-2026-07-02.json`,
`benchmarks/mklq/results/public-healthcheck-full-main-2026-07-02.json`,
`benchmarks/mklq/results/public-healthcheck-two-three-scaling-evidence-2026-07-03.json`,
`benchmarks/mklq/results/public-healthcheck-full-two-three-scaling-evidence-2026-07-03.json`,
`benchmarks/mklq/results/public-healthcheck-cpu-phase-fixture-docs-2026-07-04.json`,
`benchmarks/mklq/results/public-healthcheck-cpu-sampling-oracle-2026-07-04.json`,
`benchmarks/mklq/results/public-healthcheck-metal-sampling-oracle-2026-07-04.json`,
`benchmarks/mklq/results/public-healthcheck-metal-sampling-counter-oracle-2026-07-04.json`,
`benchmarks/mklq/results/public-healthcheck-metal-host-sampling-telemetry-2026-07-04.json`,
`benchmarks/mklq/results/public-healthcheck-metal-sampling-phase-timing-2026-07-04.json`,
`benchmarks/mklq/results/public-healthcheck-deterministic-sampling-bypass-2026-07-04.json`,
`benchmarks/mklq/results/public-healthcheck-deterministic-sampling-bypass-final-2026-07-04.json`,
`benchmarks/mklq/results/public-healthcheck-deterministic-sequential-bypass-2026-07-04.json`,
`benchmarks/mklq/results/public-healthcheck-2026-07-05.json`,
`benchmarks/mklq/results/public-healthcheck-full-2026-07-05.json`,
`benchmarks/mklq/results/public-healthcheck-full-2026-06-22.json`,
`benchmarks/mklq/results/public-healthcheck-full-2026-06-23.json`,
`benchmarks/mklq/results/public-healthcheck-full-2026-06-24.json`,
`benchmarks/mklq/results/local-correctness-gate-2026-06-22.json`,
`benchmarks/mklq/results/local-correctness-gate-2026-06-23.json`,
`benchmarks/mklq/results/local-correctness-gate-2026-06-24.json`,
`benchmarks/mklq/results/local-correctness-gate-2026-06-28.json`,
`benchmarks/mklq/results/local-correctness-gate-2026-07-02.json`,
`benchmarks/mklq/results/local-correctness-gate-cpu-phase-fixture-2026-07-04.json`,
`benchmarks/mklq/results/local-correctness-gate-cpu-sampling-oracle-2026-07-04.json`,
`benchmarks/mklq/results/local-correctness-gate-metal-sampling-oracle-2026-07-04.json`,
`benchmarks/mklq/results/local-correctness-gate-metal-sampling-counter-oracle-2026-07-04.json`,
`benchmarks/mklq/results/local-correctness-gate-metal-host-sampling-telemetry-2026-07-04.json`,
`benchmarks/mklq/results/local-correctness-gate-metal-sampling-phase-timing-2026-07-04.json`,
`benchmarks/mklq/results/local-correctness-gate-deterministic-sampling-bypass-2026-07-04.json`,
`benchmarks/mklq/results/local-correctness-gate-deterministic-sequential-bypass-2026-07-04.json`,
`benchmarks/mklq/results/local-correctness-gate-2026-07-05.json`,
`benchmarks/mklq/results/local-metal-runtime-counter-probe-2026-06-22.counter.json`,
`benchmarks/mklq/results/local-metal-runtime-counter-probe-2026-06-23.counter.json`,
`benchmarks/mklq/results/local-metal-runtime-counter-probe-2026-06-24.counter.json`,
`benchmarks/mklq/results/local-metal-runtime-counter-probe-2026-06-28.counter.json`,
`benchmarks/mklq/results/local-metal-runtime-counter-probe-2026-07-02.counter.json`,
`benchmarks/mklq/results/local-metal-runtime-counter-probe-2026-07-04.counter.json`,
`benchmarks/mklq/results/local-metal-runtime-counter-probe-metal-sampling-counter-oracle-2026-07-04.counter.json`,
`benchmarks/mklq/results/local-metal-runtime-counter-probe-metal-host-sampling-telemetry-2026-07-04.counter.json`,
`benchmarks/mklq/results/local-metal-runtime-counter-probe-metal-sampling-phase-timing-2026-07-04.counter.json`,
`benchmarks/mklq/results/local-metal-runtime-counter-probe-deterministic-sampling-bypass-2026-07-04.counter.json`,
`benchmarks/mklq/results/local-metal-runtime-counter-probe-2026-07-05.counter.json`,
`benchmarks/mklq/results/local-sampling-scaling-cpu-q18-q22-2026-06-23.json`,
`benchmarks/mklq/results/local-scaling-cpu-two-qubit-three-qubit-q18-q22-2026-07-03-two-three-scaling.json`,
`benchmarks/mklq/results/local-metal-sampling-boundary-q22-2026-07-04.json`,
`benchmarks/mklq/results/local-metal-count-accumulation-sampling-q20-2026-07-04.json`,
`benchmarks/mklq/results/local-metal-count-accumulation-sampling-q22-2026-07-04.json`,
`benchmarks/mklq/results/local-metal-partial-count-accumulation-sampling-q20-2026-07-05.json`,
`benchmarks/mklq/results/local-metal-partial-count-accumulation-sampling-q22-2026-07-05.json`,
`benchmarks/mklq/results/local-metal-partial-count-accumulation-sampling-q24-2026-07-05.json`,
`benchmarks/mklq/results/local-metal-uniform-partial-sampling-q20-q24-2026-07-05.json`,
`benchmarks/mklq/results/example-smoke-2026-06-23.json`,
`benchmarks/mklq/results/example-smoke-2026-07-02.json`,
`benchmarks/mklq/results/example-smoke-2026-07-05.json`,
`benchmarks/mklq/results/macos-install-signature-repair-2026-07-02.json`,
`benchmarks/mklq/results/macos-install-signature-repair-2026-07-04.json`, and
`benchmarks/mklq/results/macos-install-signature-repair-2026-07-05.json`;
these raw payloads are not tracked as public evidence.

Latest default 2026-07-05 result: `31/31` steps passed.
latest full 2026-07-05 result is `35/35` steps passed.

- Install-prefix build: passed.
- Default public healthcheck: passed on 2026-07-05, with 31 steps passed and
  0 failed.
- Latest full public healthcheck: passed on 2026-07-05, with 35 steps passed
  and 0 failed, including install-prefix build, local macOS signature repair,
  correctness gate, benchmark harness tests, and public example smoke.
- One-command correctness gate: passed with 4 steps passed, 0 failed, and 0
  skipped, including the Metal runtime counter probe.
- Public example smoke gate: passed, with 30 steps passed and 0 failed.
- Current `benchmark_harness_tests`: `218 passed`.
- Current `cpu_gate_counter_probe_parse`: 3 bounded reports, 37 expected,
  37 selected, 0 missing, and 0 failures, including single-control X/CNOT,
  per-gate single-control H/Y/Rx/Ry direct pair fixtures, single-control Rz
  direct phase fast-path fixture, and hardware-efficient ansatz composite
  fast-path fixture.
- Current `cpu_sampling_counter_probe_parse`: 3 bounded reports, 21 expected,
  21 selected, 0 missing, and 0 failures; each report includes full-register
  and marginal probability-fill counter ctests.
- Standalone install-prefix Python subset: `37 passed`.
- `python_target_smoke`: `70 passed`.
- `nvqpp_smoke`: `2 passed`.
- Current build-tree `ctest` subset: `104/104 passed`, including the
  `HardwareEfficientAnsatzCompositeUsesDedicatedFastPaths` CPU counter fixture
  that checks the hardware-efficient ansatz gate mix uses the expected
  rotation, CNOT, CRZ, CZ, CRX, and SWAP fast paths, plus the resident
  full-register Metal sampling telemetry and selected sample-count accumulation
  fixtures.
- Current tracked `metal_runtime_counter_probe`: 13 bounded reports, 535
  expected, 535 selected, 0 missing, and 0 failures. The latest tracked report
  runs 50 counter ctests independently, including the direct Metal
  sample-count accumulation kernel fixture, the direct Metal generated-draw
  sample-count fixture, selected full-register and partial-register
  counts-only Metal generated-draw/count accumulation, the uniform-probability
  generated-count fast path, resident sequential host draw telemetry,
  requested-order partial-register sampling marginal-probability coverage,
  deterministic sequential/counts-only draw-loop bypasses, and native sampling
  phase timing accumulators; the older reports remain historical 20-, 20-,
  39-, 40-, 42-, 43-, 44-, 45-, 47-, 48-, 49-, and 50-test evidence.
- Clean CPU benchmark gate: passed, with 32 q20 `qpp-cpu`/`mklq-cpu` rows and
  32 rows reporting `status == "ok"`, including `two-qubit-state`,
  `three-qubit-state`, and `hardware-efficient-ansatz-state`.
- Focused multi-control CPU benchmark evidence: passed, with 2 q20
  `multi-control-state` rows reporting `status == "ok"` and a tracked
  `qpp-cpu` over `mklq-cpu` median elapsed ratio of `45.09x`.
- Focused CPU scaling evidence: passed, with 6 q18/q20/q22
  `multi-control-state` rows reporting `status == "ok"` and tracked
  `qpp-cpu` over `mklq-cpu` median elapsed ratios of `11.66x`, `28.00x`, and
  `72.93x`.
- Focused hardware-efficient ansatz CPU scaling evidence: passed, with 6
  q18/q20/q22 rows reporting `status == "ok"` and tracked `qpp-cpu` over
  `mklq-cpu` median elapsed ratios of `26.84x`, `52.94x`, and `81.37x`.
- Focused two/three-qubit CPU scaling evidence: passed, with 12 q18/q20/q22
  `two-qubit-state` and `three-qubit-state` rows reporting `status == "ok"`.
  The tracked `qpp-cpu` over `mklq-cpu` median elapsed ratios are `47.20x`,
  `131.99x`, and `163.42x` for `two-qubit-state`, and `24.54x`, `87.34x`,
  and `90.91x` for `three-qubit-state`.
- Focused sampling scaling evidence: passed, with 24 q18/q20/q22
  full-register and partial-register sampling rows reporting `status == "ok"`.
  The tracked `qpp-cpu` over `mklq-cpu` median elapsed ratios range from
  `45.24x` to `109.56x` across 1024-shot and 65536-shot rows.

## Install-prefix Gate

```bash
cmake --build build-python --target install -j 6
```

Result: passed in the latest local refresh, installing to
`/Users/a0000/.cudaq-mklq`.

```bash
PYTHONPATH=/Users/a0000/.cudaq-mklq \
python3 -m pytest \
  python/tests/backends/test_mklq_python_api.py \
  python/tests/builder/test_mklq_targets.py \
  -q
```

Result: `37 passed in 4.46s` in the latest local refresh.

```bash
CUDAQ_NVQPP=/Users/a0000/.cudaq-mklq/bin/nvq++ \
PYTHONPATH=/Users/a0000/.cudaq-mklq \
python3 -m pytest python/tests/backends/test_mklq_nvqpp_smoke.py -q
```

Result: `2 passed`.

## Build-tree Gate

```bash
ctest --test-dir build-python \
  -R "(mklq_(cpu|metal)_MKLQ|backend_target_setter_check|TargetConfigTester)" \
  --output-on-failure
```

Result in the latest standalone correctness-gate refresh: `104/104 passed`.

```bash
PYTHONPATH=/Users/a0000/Documents/MKL-Q/build-python/python \
python3 -m pytest \
  python/tests/backends/test_mklq_nvqpp_smoke.py \
  python/tests/backends/test_mklq_benchmark_harness.py \
  python/tests/backends/test_mklq_python_api.py \
  python/tests/builder/test_mklq_targets.py \
  -q
```

Historical bootstrap result: `63 passed`. This build-tree Python bundle is not
part of the latest full public healthcheck; the install-prefix correctness
wrapper is the current public readiness gate.

```bash
PYTHONPATH=/Users/a0000/Documents/MKL-Q/tpls/llvm/llvm/utils/lit \
/opt/anaconda3/bin/python3 /Users/a0000/.local/llvm/bin/llvm-lit \
  -j 1 -sv \
  --filter 'mklq_(targets|runtime_smoke)' \
  --param cudaq_site_config=/Users/a0000/Documents/MKL-Q/build-python/targettests/lit.site.cfg.py \
  /Users/a0000/Documents/MKL-Q/build-python/targettests/TargetConfig
```

Historical bootstrap result: 2 selected MKL-Q TargetConfig tests passed. The
latest correctness refresh uses the broader TargetConfig `ctest` selection
above.

## One-command Correctness Gate

Use the local correctness gate wrapper to run the install-prefix Python smoke
tests, the `nvq++` smoke tests, the build-tree TargetConfig `ctest` gate, and
the Metal runtime counter probe in one command:

```bash
python3 benchmarks/mklq/run_correctness_gate.py \
  --install-prefix "${HOME}/.cudaq-mklq" \
  --build-dir build-python
```

Latest local/self-hosted result: passed on 2026-07-05 for
`d92e4c1494f5aa43afb01d469fd76ad37e7fe800` in the manual Apple Silicon
correctness workflow after the source submodule bootstrap, focused install
build, macOS install-prefix signature repair, correctness gate, benchmark
harness tests, and public example smoke gate all completed. It reported 4
correctness-wrapper steps passed, 0 failed, and 0 skipped.
The step-level results were:

- `python_target_smoke`: `70 passed`.
- `nvqpp_smoke`: `2 passed`.
- `target_config_ctest`: `104/104 passed`, including the hardware-efficient
  ansatz composite CPU fast-path counter fixture and the resident full-register
  Metal sampling telemetry plus selected sample-count accumulation fixtures.
- `metal_runtime_counter_probe`: 50 expected, 50 selected, 0 missing, and 50
  independently executed passing counter ctests, including direct runtime
  single-, two-, and three-qubit gate fixtures, full-register and marginal
  probability fixtures, built-in Rx/Ry/Rz and phase-family fixtures, sampling
  fixtures including requested-order partial-register sampling through the
  resident marginal probability route, deterministic sequential/counts-only
  draw-loop bypasses, selected full-register and partial-register counts-only
  Metal generated-draw/count accumulation, the uniform-probability
  generated-count fast path, sequential host draw telemetry, and native
  probability-fill/draw/expectation phase timing,
  measurement/collapse/reset fixtures, unsupported-gate fallback/reupload
  fixtures, and resident error-boundary fixtures.

The Python smoke step includes the MKL-Q API smoke tests, the CPU correctness
fixture suite, the limited experimental Metal correctness fixture suite, and
the builder-level MKL-Q target tests.

The default JSON output path is ignored by Git:
`benchmarks/mklq/results/local-correctness-gate-<date>.json`. The default
Metal runtime counter probe output path is also ignored by Git:
`benchmarks/mklq/results/local-metal-runtime-counter-probe-<date>.counter.json`.
Use `--skip-metal-counter-probe` only when that build-tree counter evidence is
intentionally out of scope. Use `--plan-only` to inspect the exact commands and
environment without running the gate:

```bash
python3 benchmarks/mklq/run_correctness_gate.py \
  --install-prefix "${HOME}/.cudaq-mklq" \
  --build-dir build-python \
  --plan-only
```

For build-tree-only experiments, override the runtime paths explicitly:

```bash
python3 benchmarks/mklq/run_correctness_gate.py \
  --pythonpath /Users/a0000/Documents/MKL-Q/build-python/python \
  --nvqpp /Users/a0000/Documents/MKL-Q/build-python/bin/nvq++ \
  --build-dir build-python
```

## Repository Hygiene Gate

```bash
python3 benchmarks/mklq/run_public_healthcheck.py --full --require-clean
```

Latest default 2026-07-05 result: `31/31` steps passed. The latest full
2026-07-05 result is `35/35` steps passed. The full gate includes Git
repository hygiene, tracked-artifact checks, public metadata checks, the public
release checklist audit, the upstream sync audit, the self-hosted Apple Silicon
CI audit, sanitized benchmark summary parsing, the clean CPU performance
evidence guards, the Metal evidence boundary guard, the Metal stochastic
sampling boundary guard, the Metal uniform sampling evidence guard, bounded CPU
gate fast-path counter evidence parsing, bounded CPU sampling/probability counter evidence
parsing, bounded Metal runtime counter evidence parsing, CPU gate,
CPU sampling/probability, and Metal counter docs drift detection, concrete
public docs/workflows report-reference checks, helper `py_compile`, markdown links,
benchmark evidence regeneration, healthcheck snapshot docs drift detection,
benchmark harness tests, install-prefix build, local macOS install-prefix
signature repair, the one-command correctness gate, and the public example
smoke gate.

The ignored raw healthcheck JSON records the exact Git state for these local
runs. The latest default healthcheck benchmark harness step reported
`177 passed`. The latest full wrapper run reported `32 passed`, including
install-prefix build, local signature repair, correctness, and example-smoke
gates in the `--full --require-clean` wrapper. An earlier wrapper run before
the signature-repair step was added failed when installed `nvq++` smoke
executables could not load a locally installed loadable with an invalid ad-hoc
signature.

## Benchmark Evidence

Sanitized local benchmark summaries are tracked under
`benchmarks/mklq/reports/`. Raw local benchmark JSON under
`benchmarks/mklq/results/` is intentionally ignored.

The compact public index for the tracked summaries is
[`benchmark-evidence.md`](benchmark-evidence.md). Rerun the clean CPU benchmark
gate, regenerate the sanitized summary, and refresh the public index with:

```bash
python3 benchmarks/mklq/run_clean_cpu_benchmark.py \
  --pythonpath "${HOME}/.cudaq-mklq" \
  --stamp YYYY-MM-DD
```

Current default clean CPU runs include `ch-state`, `crx-state`, `cry-state`,
`crz-state`, `two-qubit-state`, `three-qubit-state`, and
`hardware-efficient-ansatz-state` for new evidence refreshes. To regenerate the
historical 2026-06-21 sanitized summary from the already ignored raw JSON, keep
the old composite case set explicit:

```bash
python3 benchmarks/mklq/run_clean_cpu_benchmark.py \
  --pythonpath "${HOME}/.cudaq-mklq" \
  --stamp 2026-06-21 \
  --composite-cases qft-like-state,seeded-clifford-state \
  --skip-benchmark
```

Run the focused CPU qubit-scaling evidence gate for the multi-control hot path
with:

```bash
python3 benchmarks/mklq/run_cpu_scaling_benchmark.py \
  --pythonpath "${HOME}/.cudaq-mklq" \
  --stamp 2026-06-22
```

If the ignored raw JSON already exists, regenerate only the sanitized summary
and public index with:

```bash
python3 benchmarks/mklq/run_cpu_scaling_benchmark.py \
  --pythonpath "${HOME}/.cudaq-mklq" \
  --stamp 2026-06-22 \
  --skip-benchmark
```

Run the focused CPU qubit-scaling evidence gate for the hardware-efficient
ansatz composite path with:

```bash
python3 benchmarks/mklq/run_cpu_scaling_benchmark.py \
  --pythonpath "${HOME}/.cudaq-mklq" \
  --stamp YYYY-MM-DD \
  --cases hardware-efficient-ansatz-state \
  --summary-text "Clean-worktree local scaling run comparing qpp-cpu and mklq-cpu for q18/q20/q22 hardware-efficient ansatz composite state-vector updates." \
  --performance-scope "local Apple M5 q18-q22 hardware-efficient ansatz CPU target scaling comparison only; not a cross-machine release benchmark" \
  --runtime-note "The CUDA-Q Python runtime and source provenance are recorded from the raw benchmark report generated by run_cpu_scaling_benchmark.py for hardware-efficient ansatz rows."
```

Run the focused CPU qubit-scaling evidence gate for the two-qubit/SWAP and
three-qubit custom state-vector update paths with:

```bash
python3 benchmarks/mklq/run_cpu_scaling_benchmark.py \
  --pythonpath "${HOME}/.cudaq-mklq" \
  --stamp 2026-07-03-two-three-scaling \
  --cases two-qubit-state,three-qubit-state \
  --summary-text "Clean-worktree local scaling run comparing qpp-cpu and mklq-cpu for q18/q20/q22 SWAP/two-qubit and three-qubit custom state-vector updates." \
  --performance-scope "local Apple M5 q18-q22 two-qubit and three-qubit CPU target scaling comparison only; not a cross-machine release benchmark" \
  --runtime-note "The CUDA-Q Python runtime and source provenance are recorded from the raw benchmark report generated by run_cpu_scaling_benchmark.py for two-qubit and three-qubit rows."
```

Run the focused CPU sampling-scaling evidence gate for full-register and
partial-register sampling with:

```bash
python3 benchmarks/mklq/run_sampling_scaling_benchmark.py \
  --pythonpath "${HOME}/.cudaq-mklq" \
  --stamp 2026-06-23
```

If the ignored raw JSON already exists, regenerate only the sanitized summary
and public index with:

```bash
python3 benchmarks/mklq/run_sampling_scaling_benchmark.py \
  --pythonpath "${HOME}/.cudaq-mklq" \
  --stamp 2026-06-23 \
  --skip-benchmark
```

Current tracked summaries include:

- `local-clean-cpu-q20-2026-07-03-two-three.summary.json`
- `local-clean-cpu-q20-2026-07-03.summary.json`
- `local-clean-cpu-q20-2026-06-30.summary.json`
- `local-crz-distance-sweep-cpu-q20-2026-07-01.summary.json`
- `local-clean-cpu-q20-2026-06-28.summary.json`
- `local-clean-cpu-q20-2026-06-21.summary.json`
- `local-current-sampling-fullprob-gated-q20-2026-06-19.summary.json`
- `local-y-cy-fastpath-isolated-q20-2026-06-19.summary.json`
- `local-metal-composite-mixed-path-q20-2026-06-21.summary.json`
- `local-metal-path-labels-q20-2026-06-22.summary.json`
- `local-metal-y-cy-resident-isolated-q20-2026-06-19.summary.json`
- `local-counts-only-sampling-shot-scaling-q20-2026-06-19.summary.json`
- `local-metal-sampling-boundary-q22-2026-07-04.summary.json`
- `local-metal-count-accumulation-sampling-q20-2026-07-04.summary.json`
- `local-metal-count-accumulation-sampling-q22-2026-07-04.summary.json`
- `local-metal-partial-count-accumulation-sampling-q20-2026-07-05.summary.json`
- `local-metal-partial-count-accumulation-sampling-q22-2026-07-05.summary.json`
- `local-metal-partial-count-accumulation-sampling-q24-2026-07-05.summary.json`
- `local-metal-uniform-partial-sampling-q20-q24-2026-07-05.summary.json`
- `local-multi-control-cpu-q20-2026-06-22.summary.json`
- `local-scaling-cpu-multi-control-q18-q22-2026-06-22.summary.json`
- `local-scaling-cpu-hardware-efficient-ansatz-q18-q22-2026-06-30.summary.json`
- `local-scaling-cpu-two-qubit-three-qubit-q18-q22-2026-07-03-two-three-scaling.summary.json`
- `local-sampling-scaling-cpu-q18-q22-2026-06-23.summary.json`

The latest clean-worktree local benchmark summary was refreshed against
`dbebe3744f826ba4cbeed2b99708a2bdab03b11e` on 2026-07-03. The clean summary
includes `y-state`, `ch-state`, `cy-state`, `crx-state`, `cry-state`,
`crz-state`, `cz-state`, `two-qubit-state`, `three-qubit-state`,
`qft-like-state`, `seeded-clifford-state`, `hardware-efficient-ansatz-state`,
full-register sampling, and partial-register sampling rows. All 32 q20 rows
completed with `status == "ok"`, and the local `qpp-cpu` over `mklq-cpu` median
elapsed ratios include `56.82x` for `two-qubit-state`, `41.92x` for
`three-qubit-state`, and `80.59x` for `hardware-efficient-ansatz-state`.
These files include clean-worktree local benchmark summaries plus older
dirty-worktree tuning summaries. Interpret each file through its
`evidence_kind` and `interpretation` fields. Do not treat any local summary as
cross-machine performance certification.

The focused CRZ distance-sweep summary was generated against
`a311c8749bbf5edfa553f64eb71a79faeafdd803` on 2026-07-01 after installing the
current local MKL-Q build to `/Users/a0000/.cudaq-mklq`. It compares q20
`qpp-cpu` and `mklq-cpu` on `crz-distance-sweep-state` for distances 1 through
19 with `repeats=2`, `warmups=1`, `layers=8`, and isolated rows. All 38 rows
completed with `status == "ok"`, and the minimum local `qpp-cpu` over
`mklq-cpu` median elapsed ratio across the distance sweep was `68.56x`. The
public healthcheck includes a dedicated `crz_distance_evidence_guard` for this
summary.

The focused multi-control summary was generated against
`4ece8d2396e8feee1c59a04e58324c529564f487` on 2026-06-22 after installing the
current local MKL-Q build to `/Users/a0000/.cudaq-mklq`. It compares q20
`qpp-cpu` and `mklq-cpu` on `multi-control-state` with `repeats=5`,
`warmups=2`, `layers=8`, and isolated rows. The public healthcheck now includes
a dedicated `multi_control_evidence_guard` for this summary.

The focused CPU scaling summary was generated against
`e632e65f45645c9648523d86cb7612ab96d31023` on 2026-06-22 after installing the
current local MKL-Q build to `/Users/a0000/.cudaq-mklq`. It compares q18/q20/q22
`qpp-cpu` and `mklq-cpu` on `multi-control-state` with `repeats=3`,
`warmups=1`, `layers=8`, and isolated rows. The public healthcheck now includes
a dedicated `cpu_scaling_evidence_guard` for this summary.

The focused hardware-efficient ansatz scaling summary was generated against
`f2d87a4bf1e0d0163481a560df868292715a660a` on 2026-06-30 after installing the
current local MKL-Q build to `/Users/a0000/.cudaq-mklq`. It compares q18/q20/q22
`qpp-cpu` and `mklq-cpu` on `hardware-efficient-ansatz-state` with
`repeats=3`, `warmups=1`, `layers=8`, and isolated rows. The public
healthcheck now includes a dedicated `ansatz_scaling_evidence_guard` for this
summary.

The focused two/three-qubit CPU scaling summary was generated against
`cb688b20c825a970965ffe41ca84757287abf847` on 2026-07-03 after installing the
current local MKL-Q build to `/Users/a0000/.cudaq-mklq`. It compares q18/q20/q22
`qpp-cpu` and `mklq-cpu` on `two-qubit-state` and `three-qubit-state` with
`repeats=3`, `warmups=1`, `layers=8`, and isolated rows. All 12 rows completed
with `status == "ok"`. The public healthcheck now includes a dedicated
`two_three_scaling_evidence_guard` for this summary.

The focused sampling scaling summary was generated against
`0cb821897fc158c9755173da70953444099a1e64` on 2026-06-23 after installing the
current local MKL-Q build to `/Users/a0000/.cudaq-mklq`. It compares q18/q20/q22
`qpp-cpu` and `mklq-cpu` on full-register and partial-register sampling at 1024
and 65536 shots with `repeats=2`, `warmups=1`, `layers=8`, and isolated rows.
The public healthcheck now includes a dedicated
`sampling_scaling_evidence_guard` for this summary.

The focused Metal uniform partial-register sampling summary is local tuning
evidence only. It records q20/q22/q24 `mklq-metal`
`sample-uniform-partial-register` rows at 256, 1024, 8192, and 65536 shots,
requires the 12-measured-qubit/4096-outcome uniform-probability metadata, and
keeps the static path label boundary separate from runtime counter or release
certification claims. The public healthcheck now includes a dedicated
`metal_uniform_sampling_evidence_guard` for this summary.

The Metal composite summary is local tuning evidence only. It records q20
`qft-like-state` and `seeded-clifford-state` rows for `qpp-cpu`, `mklq-cpu`, and
experimental `mklq-metal`; all six rows completed with `status == "ok"`. The
summary keeps the Metal scope as mixed-path state-vector updates followed by
host readback, not full Metal-native execution.
