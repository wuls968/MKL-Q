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

Latest local correctness refresh: 2026-07-02. Latest public healthcheck
refresh: 2026-07-02.

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
The full public healthcheck was refreshed again on 2026-07-02 on clean
`main` after adding the self-hosted runner inventory audit, confirming that the
install-prefix build, local signature repair, correctness gate, and public
examples still pass together.
The clean CPU benchmark summary was refreshed separately against
`23d34ab226c3e4d7a47f15af3292bf81ce25987b` after adding the
hardware-efficient ansatz composite row to the clean evidence gate. The older
2026-06-21 clean summary remains tracked as historical evidence for the earlier
QFT-like and seeded Clifford composite gate.

The current refresh includes the earlier Metal counter-evidence work:
resident built-in Rx/Ry/Rz, controlled-Rx/Ry/Rz, phase-family S/T/Sdg/Tdg,
multi-control single-qubit resident, resident three-target gates, and
four-or-more-target unsupported gate fallback/reupload fixtures. It also reran
the full install/build/signature-repair/correctness/example gate on the current
MKL-Q branch after adding the local macOS install-prefix signature repair step.
The current tracked CPU gate counter evidence includes a bounded report for
selected single-qubit, controlled
single-qubit, single-control Rz phase, two-qubit, three-qubit, and composite
fast-path counter ctests. The current tracked CPU sampling/probability counter
evidence includes two bounded reports, each with explicit full-register and
marginal probability-fill counter ctests alongside the existing sampling phase
counter ctests. The current tracked Metal runtime counter evidence likewise
includes two bounded reports. Counter-summary aggregate counts are summed
across tracked reports, so repeated daily probes intentionally count the same
selected tests once per report.

Raw wrapper output was written to ignored local paths
`benchmarks/mklq/results/public-healthcheck-2026-06-24.json`,
`benchmarks/mklq/results/public-healthcheck-2026-07-01.json`,
`benchmarks/mklq/results/public-healthcheck-after-full-doc-refresh-2026-07-02.json`,
`benchmarks/mklq/results/public-healthcheck-full-main-2026-07-02.json`,
`benchmarks/mklq/results/public-healthcheck-full-2026-06-22.json`,
`benchmarks/mklq/results/public-healthcheck-full-2026-06-23.json`,
`benchmarks/mklq/results/public-healthcheck-full-2026-06-24.json`,
`benchmarks/mklq/results/local-correctness-gate-2026-06-22.json`,
`benchmarks/mklq/results/local-correctness-gate-2026-06-23.json`,
`benchmarks/mklq/results/local-correctness-gate-2026-06-24.json`,
`benchmarks/mklq/results/local-correctness-gate-2026-06-28.json`,
`benchmarks/mklq/results/local-correctness-gate-2026-07-02.json`,
`benchmarks/mklq/results/local-metal-runtime-counter-probe-2026-06-22.counter.json`,
`benchmarks/mklq/results/local-metal-runtime-counter-probe-2026-06-23.counter.json`,
`benchmarks/mklq/results/local-metal-runtime-counter-probe-2026-06-24.counter.json`,
`benchmarks/mklq/results/local-metal-runtime-counter-probe-2026-06-28.counter.json`,
`benchmarks/mklq/results/local-metal-runtime-counter-probe-2026-07-02.counter.json`,
`benchmarks/mklq/results/local-sampling-scaling-cpu-q18-q22-2026-06-23.json`,
`benchmarks/mklq/results/example-smoke-2026-06-23.json`,
`benchmarks/mklq/results/example-smoke-2026-07-02.json`, and
`benchmarks/mklq/results/macos-install-signature-repair-2026-07-02.json`;
these raw payloads are not tracked as public evidence.

- Install-prefix build: passed.
- Default public healthcheck: passed on 2026-07-02, with 28 steps passed and
  0 failed.
- Latest full public healthcheck: passed on 2026-07-02, with 32 steps passed
  and 0 failed after adding the dedicated macOS install-prefix signature repair
  step. The signature repair step refreshed and verified 60 local install-prefix
  `.dylib` and `.so` loadables before the correctness and public example smoke
  gates.
- One-command correctness gate: passed with 4 steps passed, 0 failed, and 0
  skipped, including the Metal runtime counter probe.
- Public example smoke gate: passed, with 30 steps passed and 0 failed.
- Current `benchmark_harness_tests`: `177 passed`.
- Current `cpu_gate_counter_probe_parse`: 1 bounded report, 11 expected,
  11 selected, 0 missing, and 0 failures, including the single-control Rz
  direct phase fast-path fixture and hardware-efficient ansatz composite
  fast-path fixture.
- Current `cpu_sampling_counter_probe_parse`: 2 bounded reports, 14 expected,
  14 selected, 0 missing, and 0 failures; each report includes full-register
  and marginal probability-fill counter ctests.
- Standalone install-prefix Python subset: `37 passed`.
- `python_target_smoke`: `61 passed`.
- `nvqpp_smoke`: `2 passed`.
- Current `target_config_ctest`: `89/89 passed`, including the
  `HardwareEfficientAnsatzCompositeUsesDedicatedFastPaths` CPU counter fixture
  that checks the hardware-efficient ansatz gate mix uses the expected
  rotation, CNOT, CRZ, CZ, CRX, and SWAP fast paths.
- Current tracked `metal_runtime_counter_probe`: 2 bounded reports, 40
  expected, 40 selected, 0 missing, and 0 failures; each report runs 20
  counter ctests independently.
- Clean CPU benchmark gate: passed, with 20 q20 `qpp-cpu`/`mklq-cpu` rows and
  20 rows reporting `status == "ok"`, including
  `hardware-efficient-ansatz-state`.
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

Result in the latest standalone correctness-gate refresh: `89/89 passed`.

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

Latest local result: passed on 2026-07-02 as part of the full public
healthcheck wrapper after local install-prefix signature repair. It reported 4
wrapper steps passed, 0 failed, and 0 skipped. The step-level results were:

- `python_target_smoke`: `61 passed`.
- `nvqpp_smoke`: `2 passed`.
- `target_config_ctest`: `89/89 passed`, including the hardware-efficient
  ansatz composite CPU fast-path counter fixture.
- `metal_runtime_counter_probe`: 20 expected, 20 selected, 0 missing, and 20
  independently executed passing counter ctests, including the resident
  direct three-target runtime fixture, built-in Rx/Ry/Rz,
  controlled-Rx/Ry/Rz, and phase-family S/T/Sdg/Tdg fixtures, plus the
  multi-control single-qubit resident fixture, the simulator resident
  three-target gate fixture, and the four-or-more-target unsupported gate
  fallback/reupload boundary fixture.

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

Latest default 2026-07-02 result: `28/28` steps passed. The latest full
2026-07-02 result is `32/32` steps passed. The full gate includes Git
repository hygiene, tracked-artifact checks, public metadata checks, the public
release checklist audit, the upstream sync audit, the self-hosted Apple Silicon
CI audit, sanitized benchmark summary parsing, the clean CPU performance
evidence guards, the Metal evidence boundary guard, bounded CPU gate fast-path
counter evidence parsing, bounded CPU sampling/probability counter evidence
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

Current default clean CPU runs include `hardware-efficient-ansatz-state` for
new evidence refreshes. To regenerate the historical 2026-06-21 sanitized
summary from the already ignored raw JSON, keep the old composite case set
explicit:

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
- `local-multi-control-cpu-q20-2026-06-22.summary.json`
- `local-scaling-cpu-multi-control-q18-q22-2026-06-22.summary.json`
- `local-scaling-cpu-hardware-efficient-ansatz-q18-q22-2026-06-30.summary.json`
- `local-sampling-scaling-cpu-q18-q22-2026-06-23.summary.json`

The latest clean-worktree local benchmark summary was refreshed against
`61e5f099f2d3b87feb6c3e7cf27d37f1e1d77c04` on 2026-06-30. The clean summary
includes `y-state`, `cy-state`, `cz-state`, `qft-like-state`,
`seeded-clifford-state`, `hardware-efficient-ansatz-state`, full-register
sampling, and partial-register sampling rows. All 20 q20 rows completed with
`status == "ok"`, and the local `qpp-cpu` over `mklq-cpu` median elapsed ratio
for `hardware-efficient-ansatz-state` was `100.99x`.
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

The focused sampling scaling summary was generated against
`0cb821897fc158c9755173da70953444099a1e64` on 2026-06-23 after installing the
current local MKL-Q build to `/Users/a0000/.cudaq-mklq`. It compares q18/q20/q22
`qpp-cpu` and `mklq-cpu` on full-register and partial-register sampling at 1024
and 65536 shots with `repeats=2`, `warmups=1`, `layers=8`, and isolated rows.
The public healthcheck now includes a dedicated
`sampling_scaling_evidence_guard` for this summary.

The Metal composite summary is local tuning evidence only. It records q20
`qft-like-state` and `seeded-clifford-state` rows for `qpp-cpu`, `mklq-cpu`, and
experimental `mklq-metal`; all six rows completed with `status == "ok"`. The
summary keeps the Metal scope as mixed-path state-vector updates followed by
host readback, not full Metal-native execution.
