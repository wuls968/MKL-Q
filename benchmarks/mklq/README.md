# MKL-Q Benchmark Harness

This directory contains local benchmark tooling for comparing CUDA-Q `qpp-cpu`
against MKL-Q Apple Silicon targets. It records measurements and machine
metadata; it does not encode performance claims.

The script's default target list includes `mklq-cpu` and `mklq-metal` only on
Apple Silicon (`Darwin arm64/aarch64`). On other platforms, pass MKL-Q targets
explicitly only after building them intentionally.

`mklq-metal` is included as an experimental target name while the Metal backend
is being built. It currently loads the MKL-Q `mklq_metal` mixed-path simulator:
supported single-target and two-target updates, including controlled forms, can
stay in a resident fp32 Metal state buffer across supported gate sequences.
Generic three-target custom operations also have a resident fp32 Metal path for
state-vector updates followed by host readback. Dense full-register probability
fills, cost-gated resident marginal probability fills, and measure/reset
collapse paths can read or update that resident buffer directly.
Measurement probability uses a
dedicated measured-qubit Metal reduction kernel with a small host partial-sum
finish; branch collapse uses a Metal kernel. Unsupported paths fall back to the
MKL-Q fp64 CPU oracle after synchronizing host state. Selected full-register
and partial-register counts-only sampling can generate stochastic draws and
accumulate outcome counts with Metal kernels after resident probability work;
sequential sampling still accumulates draw/count results on the host.
Deterministic one-outcome sequential and counts-only distributions can bypass
the draw loop after probability work.
Treat `mklq-metal` benchmark rows as mixed-path evidence, not full Metal GPU
backend performance.
New benchmark rows for `mklq-metal` include conservative `metal_path_label`,
`metal_path_scope`, `metal_path_label_source`, `metal_runtime_counter`, and
`metal_full_native` metrics. These labels come from the benchmark harness static
case map; they describe the benchmark case boundary and are not runtime
counters or release sign-off.

## Dry Run

```bash
python3 benchmarks/mklq/bench_mklq_targets.py \
  --dry-run \
  --targets qpp-cpu,mklq-cpu,mklq-metal \
  --cases gate-state,sample-basis,sample-ghz,sample-full-register,sample-partial-register,sample-uniform-partial-register,single-qubit-state,h-state,y-state,rx-state,ry-state,rz-state,diagonal-phase-state,controlled-state,multi-control-state,ch-state,cy-state,crx-state,cry-state,crz-state,cz-state,two-qubit-state,custom-two-qubit-state,dense-two-qubit-state,controlled-dense-two-qubit-state,three-qubit-state,qft-like-state,crz-distance-state,crz-distance-sweep-state,seeded-clifford-state \
  --qubits 4,8,12 \
  --shot-counts 256,1024,8192 \
  --output /tmp/mklq-benchmark-plan.json
```

## Smoke Benchmark

Use the built Python tree when running from the repository:

```bash
PYTHONPATH="$(pwd)/build-python/python" \
python3 benchmarks/mklq/bench_mklq_targets.py \
  --targets qpp-cpu,mklq-cpu,mklq-metal \
  --cases gate-state,sample-basis,sample-ghz,sample-full-register,sample-partial-register,sample-uniform-partial-register,single-qubit-state,h-state,y-state,rx-state,ry-state,rz-state,diagonal-phase-state,controlled-state,multi-control-state,ch-state,cy-state,crx-state,cry-state,crz-state,cz-state,two-qubit-state,custom-two-qubit-state,dense-two-qubit-state,controlled-dense-two-qubit-state,three-qubit-state,qft-like-state,crz-distance-state,crz-distance-sweep-state,seeded-clifford-state \
  --qubits 4 \
  --shots 32 \
  --repeats 1 \
  --warmups 1 \
  --layers 2 \
  --output /tmp/mklq-benchmark-smoke.json
```

Add `--profile-sampling-breakdown` when a sampling row needs harness-level
diagnostic timing for kernel construction, the measured `cudaq.sample` call,
and sample-result count-map materialization.

The `--qubits 4` smoke command is only a quick wiring check. Use larger qubit
counts, such as q15-q20 isolated rows, when collecting dense sampling evidence.
For CPU-backed `sample-full-register`, q4 can still fit the sparse
full-register sampling fast path; use q7 or larger with more than 64 nonzero
outcomes to exercise the dense probability-fill path. For `mklq-metal`, a dirty
resident Metal state skips the sparse host probe and can exercise resident
dense probability-fill even at small smoke sizes, but q7+ remains the clearer
path-level check.

For performance comparisons where row ordering, allocator history, or
`ru_maxrss` inheritance matter, run each row in a fresh Python process:

```bash
OMP_NUM_THREADS=10 \
PYTHONPATH="$(pwd)/build-python/python" \
python3 benchmarks/mklq/bench_mklq_targets.py \
  --isolate-rows \
  --targets mklq-cpu \
  --cases gate-state,sample-basis,sample-ghz,sample-full-register,sample-partial-register,sample-uniform-partial-register,single-qubit-state,h-state,y-state,rx-state,ry-state,rz-state,diagonal-phase-state,controlled-state,multi-control-state,ch-state,cy-state,crx-state,cry-state,crz-state,cz-state,two-qubit-state,custom-two-qubit-state,dense-two-qubit-state,controlled-dense-two-qubit-state,three-qubit-state,qft-like-state,crz-distance-state,crz-distance-sweep-state,seeded-clifford-state \
  --qubits 15,16,17,18,19,20 \
  --shots 1024 \
  --repeats 2 \
  --warmups 1 \
  --layers 8 \
  --output /tmp/mklq-benchmark-isolated.json
```

## Output

The JSON report includes:

- machine metadata: platform, macOS version, CPU brand, core count, memory
- provenance metadata: cwd, git branch/commit/dirty status, and selected
  OpenMP/vector-library environment variables
- runtime metadata: CUDA-Q module path/version and Python path context for
  non-isolated runs, or per-row child runtime metadata with `--isolate-rows`
- command/config metadata: targets, cases, qubits, shots, shot counts, repeats,
  warmups, and whether optional sampling profiling is enabled
- per-row measurements: elapsed time, throughput/latency, estimated state bytes,
  and cumulative process max RSS
- for sampling rows with `--profile-sampling-breakdown`, diagnostic
  harness-level `sampling_kernel_build_seconds_*`, `sampling_call_seconds_*`,
  and `sampling_result_counts_materialization_seconds_*` fields
- CPU gate fast-path counter evidence lives outside benchmark rows in bounded
  `.cpu-gate-counter.json` reports and `docs/mklq/cpu-gate-counters.md`
- CPU sampling phase counter evidence lives outside benchmark rows in bounded
  `.cpu-counter.json` reports and `docs/mklq/cpu-sampling-counters.md`
- for `mklq-metal` rows, conservative static path labels that state the
  mixed-path/resident/host boundary without implying all-Metal execution

`single-qubit-state`, `h-state`, `y-state`, `rx-state`, `ry-state`,
`rz-state`, `diagonal-phase-state`,
`controlled-state`, `multi-control-state`, `ch-state`, `cy-state`,
`crx-state`, `cry-state`, `crz-state`, `cz-state`, and
`two-qubit-state` are focused state-vector update microbenchmarks. The dedicated
H/Y/Rx/Ry/Rz cases initialize a non-uniform state, then apply layers of one
built-in single-qubit gate; their elapsed times include the state-preparation
gates, while the gate-specific throughput fields use only the repeated
target-gate count. Use those rows to compare built-in uncontrolled
single-qubit hot paths, not custom or controlled gate behavior. The dedicated
`diagonal-phase-state` case initializes a non-uniform state, then applies
Z/S/T/Sdg/Tdg layers; use it as timing evidence for the built-in diagonal
phase-family fast path, not custom diagonal operations or controlled phase
gates. The dedicated
CH/CY/CRX/CRY/CRZ cases initialize a non-uniform state, then apply layers of one
built-in controlled single-qubit gate; their elapsed times include the
state-preparation gates, while the controlled-gate throughput fields use only
the repeated target-gate count. Use those rows as evidence for built-in
controlled-gate hot paths, not custom controlled operations.
The `multi-control-state` case initializes a non-uniform state, then applies
two-control CRX/CX/CZ layers; use it as evidence for multi-control gate-loop
cost and control-mask handling, not as a claim about arbitrary controlled
custom operations. The `cz-state` case
initializes a
non-uniform state, then applies CZ-only layers; use it as evidence for the CZ
phase fast path, not as a general claim about every controlled single-qubit
gate. The `two-qubit-state` case initializes a non-uniform state, then applies
SWAP layers; use it as evidence for this hot path, not as a general claim about
every custom 4x4 gate. The `custom-two-qubit-state` case initializes a
non-uniform state, then applies a registered custom phased-iSWAP-like 4x4
unitary over adjacent two-qubit pairs; use it as evidence for the benchmark's
row-sparse custom two-target path, not as a claim about every possible 4x4
unitary. The `dense-two-qubit-state` case initializes a non-uniform state, then
applies a registered dense H-tensor-H-style 4x4 unitary over adjacent two-qubit
pairs; use it as evidence for the generic dense 4x4 block path, not as a claim
about every possible 4x4 unitary. The `controlled-dense-two-qubit-state` case
applies that dense 4x4 unitary through a one-control sub-kernel over adjacent
three-qubit windows; use it as evidence for the controlled generic dense 4x4
block path, not as a claim about every controlled custom unitary. The
`three-qubit-state` case initializes a
non-uniform state, then
applies a registered custom 8x8 flip-all unitary over adjacent three-qubit
windows; use it as evidence for the benchmark's custom three-target path, not
as a claim about every possible 8x8 unitary. The
`qft-like-state` case prepares a nonzero basis state and applies layered
H/CRZ/SWAP blocks shaped like the QFT fixtures; its
gate-specific throughput excludes the two state-preparation X gates. The
`crz-distance-state` case prepares a non-uniform state, then applies only the
same increasing-distance CRZ pattern used inside the QFT-like fixture. It
records a `crz_distance_histogram` so long-range controlled-RZ behavior can be
separated from H/SWAP work; use it as diagnostic evidence for CRZ distance
distribution, not as a broad QFT performance claim.
`crz-distance-sweep-state` expands into one benchmark row per CRZ distance for
each qubit count, records the top-level `distance` plus matching
`crz_distance` metric, and keeps each row to a single fixed control-target
separation. Use it to compare distance-specific CRZ behavior across targets;
it is still local benchmark evidence, not a QFT performance certification. The
`seeded-clifford-state` case applies a deterministic seed-17 mixture of
single-qubit Clifford gates and CX/CY/CZ/SWAP operations; use it as an
end-to-end mixed-gate stress row, not as evidence for one isolated primitive.
`sample-basis` targets deterministic sparse full-register sampling from the
allocated `|0...0>` basis state. `sample-ghz` targets sparse full-register
sampling with two nonzero outcomes, while
`sample-full-register` targets dense full-register sampling after non-uniform
single-qubit rotations. `sample-partial-register` applies non-uniform
single-qubit rotations across the whole state but measures only every other
qubit; use it to exercise cost-gated partial-register sampling paths. Standard
non-explicit `cudaq.sample` rows use counts-only aggregation rather than
retaining per-shot sequential data, so dense sampling rows exercise the
backend's aggregate draw-count path. The benchmark does not call the public
sequential-data accessor, which may expand counts on demand for API
compatibility. For
`mklq-metal`, small marginal buffers use the resident marginal probability
kernel, while q15-q20 every-other-qubit rows currently route to resident
full-register probability fill plus host marginal folding.
`sample-uniform-partial-register` applies H gates to up to the first 12 measured
qubits, measures only those qubits, and leaves the remaining state in a basis
configuration. Use it as a dedicated uniform marginal fixture for the
counts-only uniform-probability generated-count fast path; it is not a claim
about arbitrary non-uniform partial-register sampling.

`--profile-sampling-breakdown` is a diagnostic benchmark-harness feature, not a
native backend profiler. It records additional timings around Python-visible
work and explicitly labels rows with
`sampling_profile_boundary`; do not interpret those fields as internal
probability-fill, draw, or count-aggregation counters.

## CPU Gate Counter Probe

Use the CPU gate counter probe when changing `mklq-cpu` gate fast paths,
including single-qubit, controlled single-qubit, single-control Rz phase,
diagonal phase gates, generic dense two-qubit, row-sparse custom two-qubit,
three-qubit, and composite fast-path selection tests:

```bash
python3 benchmarks/mklq/run_cpu_gate_counter_probe.py \
  --build-dir build-python \
  --output benchmarks/mklq/reports/local-cpu-gate-counter-probe-YYYY-MM-DD.cpu-gate-counter.json
```

The report is bounded evidence from selected build-tree ctest cases. It records
whether the complete expected CPU gate fast-path counter test set is present
and passing; it is not a benchmark result, release sign-off, or cross-machine
performance proof.

## CPU Gate Counter Summary

Regenerate the public CPU gate counter Markdown summary from tracked bounded
reports:

```bash
python3 benchmarks/mklq/summarize_cpu_gate_counters.py \
  --reports benchmarks/mklq/reports \
  --output docs/mklq/cpu-gate-counters.md
```

When multiple bounded reports are tracked, aggregate counts in the generated
summary are summed across reports. Repeated daily probes intentionally count
the same selected counter tests once per report.

## CPU Gate Counter Docs Guard

Check that the tracked public Markdown matches the tracked bounded reports:

```bash
python3 benchmarks/mklq/check_cpu_gate_counter_docs.py
```

## CPU Sampling Counter Probe

Use the CPU sampling counter probe when changing `mklq-cpu` sampling or
probability-fill internals, or the native sampling/probability counter tests:

```bash
python3 benchmarks/mklq/run_cpu_sampling_counter_probe.py \
  --build-dir build-python \
  --output benchmarks/mklq/reports/local-cpu-sampling-counter-probe-YYYY-MM-DD.cpu-counter.json
```

The report is bounded evidence from selected build-tree ctest cases. It records
whether the complete expected CPU sampling phase and probability-fill counter
test set is present and passing; it is not a benchmark result, release
sign-off, or cross-machine performance proof.

## CPU Sampling Counter Summary

Regenerate the public CPU sampling counter Markdown summary from tracked
bounded reports:

```bash
python3 benchmarks/mklq/summarize_cpu_sampling_counters.py \
  --reports benchmarks/mklq/reports \
  --output docs/mklq/cpu-sampling-counters.md
```

When multiple bounded reports are tracked, aggregate counts in the generated
summary are summed across reports. Repeated daily probes intentionally count
the same selected counter tests once per report.

## CPU Sampling Counter Docs Guard

Check that the tracked public Markdown matches the tracked bounded reports:

```bash
python3 benchmarks/mklq/check_cpu_sampling_counter_docs.py
```

Measured timings are post-warmup execution calls; target setup and kernel
construction are outside the timed region. `process_max_rss_bytes` is the
maximum RSS for the benchmark Python process. The JSON field is named
`process_max_rss_bytes_cumulative` because later rows inherit earlier rows'
memory history. Use one fresh process per row if you need strict per-target
peak memory isolation; `--isolate-rows` automates this for benchmark rows.

By default, the script exits nonzero if any benchmark row has `status != "ok"`.
Use `--allow-errors` only when collecting partial data from experimental
targets.

Use larger qubit counts and repeats only after correctness gates are green. To
run the local aggregate correctness gate before collecting benchmark evidence:

```bash
python3 benchmarks/mklq/run_correctness_gate.py \
  --install-prefix "${HOME}/.cudaq-mklq" \
  --build-dir build-python
```

The gate also runs the Metal runtime counter probe by default and writes both
ignored local JSON outputs under `benchmarks/mklq/results/`. Use
`--skip-metal-counter-probe` only when you intentionally want a correctness
gate without build-tree Metal counter evidence. When preserving rejected tuning
runs, label them clearly and keep them separate from the local baseline so they
are not read as performance evidence.

## Install-prefix Signature Repair

Use the local macOS signature repair helper after installing to a development
prefix and before running installed `nvq++` smoke executables:

```bash
python3 benchmarks/mklq/repair_macos_install_signatures.py \
  --install-prefix "${HOME}/.cudaq-mklq"
```

The helper refreshes ad-hoc signatures for local install-prefix dylibs,
Python extension loadables, and `bin/` Mach-O executables, then writes ignored
JSON under `benchmarks/mklq/results/`. It is not release artifact signing, does
not publish binary artifacts, and is skipped on non-Darwin platforms.

## Preflight Audit

Use the preflight audit before opening or updating a public pull request:

```bash
python3 benchmarks/mklq/run_preflight_audit.py
```

The audit checks the local Git worktree state, stale Git lock files, expected
`origin` and `upstream` remotes, shallow-clone state, tracked generated or raw
local artifacts, public report references, ignored local artifact summaries,
and the public `main` branch protection settings. The
`public_report_references` check fails when public docs or workflows reference
missing or untracked report files under `benchmarks/mklq/reports/*.json`.
Template paths such as `YYYY-MM-DD` or glob examples do not count as concrete
evidence. Add
`--require-clean` when the branch should have no uncommitted changes, or
`--skip-github` for offline local-only checks. A short-lived Git lock is
rechecked once before the audit reports it as a stale lock failure.

For pre-commit planning only, add `--preview-report-reference-adds` to treat
existing untracked report files referenced by public docs or workflows as
planned Git additions for the `public_report_references` check. This does not
modify the Git index and is not a replacement for the normal clean preflight
before publishing.

## Public Release Checklist Audit

Use the release checklist audit when public release instructions, source-only
boundaries, or maintenance gates change:

```bash
python3 benchmarks/mklq/run_public_release_checklist_audit.py
```

The audit checks that `docs/mklq/public-release-checklist.md` still contains
the required source-only sections, the public hygiene and full local gate
commands, referenced docs/scripts, preflight public report-reference boundary,
and stop conditions. It also checks that `docs/mklq/developer-workflow.md`
keeps the current public hygiene command set, counter aggregate-count boundary,
and `public_report_references` warning. It writes ignored JSON under
`benchmarks/mklq/results/`. It does not run builds, benchmarks, GitHub Actions,
or backend correctness tests.

## Source Release Tag Audit

Use the source release tag audit before changing planned source-only tag notes
or checking the planned `mklq-v0.1.0-source` boundary:

```bash
python3 benchmarks/mklq/run_source_release_tag_audit.py --docs-only
```

The docs-only mode is safe for PR branches and public hygiene jobs. It checks
the candidate tag naming convention, `CHANGELOG.md`,
`docs/mklq/release-notes-v0.1.0-source.md`, release policy, public checklist,
README links, and tracked artifact hygiene without querying live GitHub run
state.

Run the full mode only from clean `main` after pushing:

```bash
python3 benchmarks/mklq/run_source_release_tag_audit.py
```

Full mode additionally verifies `HEAD == origin/main`, confirms the candidate
tag does not exist locally or on `origin`, checks that no GitHub Releases exist,
requires the latest public hygiene workflow to succeed for the exact commit,
and requires a successful manual `workflow_dispatch` run of
`MKL-Q Apple Silicon correctness` for the exact commit with the
`Manual Apple Silicon correctness gate` job present and successful. The
automatic `main` push `Dispatch guard` does not satisfy this source tag
preflight. The audit never creates tags, GitHub Releases, packages, or
artifacts.

## Upstream Sync Audit

Use the upstream sync audit before interpreting or merging changes from
`NVIDIA/cuda-quantum`:

```bash
python3 benchmarks/mklq/run_upstream_sync_audit.py
```

The audit checks the expected `origin` and `upstream` remotes, shallow-clone
state, local `main`/`origin/main`/`upstream/main` refs, upstream delta counts,
manual-review risk categories for changed files, and
`docs/mklq/upstream-sync.md` guard coverage. The default mode is read-only and
does not fetch or merge. Use `--check-remote` when you want to confirm that the
local `upstream/main` ref matches the live upstream remote before making a sync
decision.

## Self-hosted Apple Silicon CI Audit

Use the self-hosted Apple Silicon CI audit when changing
`docs/mklq/apple-silicon-ci.md`, GitHub workflow boundaries, or source-only
public release wording:

```bash
python3 benchmarks/mklq/run_self_hosted_ci_audit.py
```

The audit checks that the Apple Silicon CI readiness plan names the expected
self-hosted macOS ARM64 runner labels, validation commands, security boundary,
activation checklist, and source-only no-release behavior. It also confirms the
tracked Apple Silicon workflow keeps only the lightweight `main` push
`Dispatch guard` automatic while the full self-hosted job remains manual,
read-only, default-off, free of pull-request, secret, release, or upload paths.
It writes ignored JSON under `benchmarks/mklq/results/`.

Before dispatching `run_full_gate=confirm`, use the optional live runner
inventory check:

```bash
python3 benchmarks/mklq/run_self_hosted_ci_audit.py \
  --check-runners \
  --repo wuls968/MKL-Q
```

That mode queries the GitHub `actions/runners` API and fails unless an online
runner has `self-hosted`, `macOS`, `ARM64`, and `mklq-apple-silicon` labels.

## Public Health Check

Use the public health check as the default local pre-push maintenance command:

```bash
python3 benchmarks/mklq/run_preflight_audit.py
python3 benchmarks/mklq/run_public_healthcheck.py
```

The default mode checks Git remotes and shallow state, tracked artifact hygiene,
public metadata and banned tokens, the public release checklist audit, the
upstream sync audit, the self-hosted Apple Silicon CI audit, sanitized
benchmark summary JSON, the static clean CPU performance evidence guards,
including focused CRZ distance, multi-control, q18-q22 CPU scaling, and
q18-q22 sampling scaling evidence, the static experimental Metal evidence
boundary guard, the Metal stochastic sampling host-boundary evidence guard,
the focused Metal uniform partial-register sampling evidence guard,
bounded CPU sampling phase counter evidence, bounded Metal runtime counter
probe JSON, helper syntax, local markdown links, regenerated benchmark-evidence
plus CPU and Metal counter docs consistency, and the benchmark harness tests.
It writes an ignored JSON report under
`benchmarks/mklq/results/`.

Before describing a commit as public-ready, run the heavier local gate:

```bash
python3 benchmarks/mklq/run_public_healthcheck.py --full --require-clean
```

Then audit the pushed public fork state:

```bash
python3 benchmarks/mklq/run_public_readiness_audit.py
```

The readiness audit is intended for a clean local `main` that matches
`origin/main`. It checks the public fork identity, tracked workflow set, issue
template set, issue-template labels, live GitHub label metadata, live branch
protection drift against `.github/branch-protection-main.json`, public claim
boundaries, latest public hygiene workflow success, latest Apple Silicon
workflow success for the pushed commit, and the source-only no-tags/no-releases
boundary. It does not run backend correctness tests or refresh benchmark
evidence.

`--full` adds the install-prefix build, local macOS install-prefix signature
repair, one-command correctness gate, and public example smoke gate. It does
not refresh benchmark evidence. To intentionally refresh clean CPU benchmark
evidence, run from a clean worktree:

```bash
python3 benchmarks/mklq/run_public_healthcheck.py \
  --full \
  --require-clean \
  --refresh-clean-cpu-benchmark
```

## Public Claim Boundary Guard

Use the static claim-boundary guard when a change touches public README or
`docs/mklq` wording about source-only status, Metal readiness, release status,
or performance evidence:

```bash
python3 benchmarks/mklq/check_public_claims.py
```

The guard does not run benchmarks. It scans public Markdown for required
source-only, experimental Metal, non-release, and non-cross-machine wording,
and rejects non-negated claims such as release-ready Metal support, full
Metal-native execution, release certification, or cross-machine performance
certification.

## Performance Evidence Guard

Use the static guard when a change touches tracked sanitized benchmark summaries
or public performance wording:

```bash
python3 benchmarks/mklq/check_performance_evidence.py
```

The guard does not run benchmarks. It verifies that the accepted clean q20 CPU
summary is clean-worktree evidence, points raw payloads at ignored
`benchmarks/mklq/results/` files, rejects cross-machine claims, and keeps the
tracked `qpp-cpu` over `mklq-cpu` ratios above the local evidence floor.

## Metal Evidence Guard

Use the static Metal guard when a change touches tracked `mklq-metal` summaries
or public wording about the experimental Metal path:

```bash
python3 benchmarks/mklq/check_metal_evidence.py
```

The guard does not run benchmarks. It verifies that tracked Metal summaries are
local tuning evidence, keep raw payloads under ignored
`benchmarks/mklq/results/` paths, include successful `mklq-metal` rows, and
state the mixed-path/resident/host-readback boundary instead of implying
default status, release status, or all-Metal execution.

## Metal Sampling Boundary Evidence Guard

Use the static Metal sampling boundary guard when a change touches tracked
`mklq-metal` stochastic sampling summaries or public wording about sample
draw/count placement:

```bash
python3 benchmarks/mklq/check_metal_sampling_boundary_evidence.py
```

The guard does not run benchmarks. It checks the tracked q20, q22, and q24
counts-only shot-scaling summaries for full-register and partial-register
`mklq-metal` sampling rows at 256, 1024, 8192, and 65536 shots, verifies
ignored raw payload boundaries, requires explicit selected Metal sample-count
accumulation with host-generated or device-generated draws or historical
host-side draw/count wording for partial-register rows, and rejects claims of
broad GPU sampler coverage, release readiness, or all-Metal execution.

## Metal Uniform Sampling Evidence Guard

Use the static Metal uniform sampling guard when a change touches the tracked
uniform-probability partial-register sampling summary or public wording about
the generated-count fast path:

```bash
python3 benchmarks/mklq/check_metal_uniform_sampling_evidence.py
```

The guard does not run benchmarks. It checks the tracked q20/q22/q24
`sample-uniform-partial-register` summary for 256, 1024, 8192, and 65536-shot
rows, requires the 12-measured-qubit/4096-outcome uniform-probability metadata,
verifies the static `mklq_metal_uniform_partial_register_sample_count_accumulation`
path label, and rejects broad GPU sampler, release-readiness, or all-Metal
execution claims.

## Metal Runtime Counter Probe

Use the runtime counter probe when you need local build-tree evidence that the
experimental Metal path actually exercised `MetalStateVectorExecutor` counters
and MKL-Q simulator test-accessor counters:

```bash
python3 benchmarks/mklq/run_metal_runtime_counter_probe.py \
  --build-dir build-python \
  --output benchmarks/mklq/reports/local-metal-runtime-counter-probe-YYYY-MM-DD.counter.json
```

The probe requires the complete expected
`mklq_metal_MKLQMetalTester.*` counter-test set to be present, runs each test
through an exact ctest regex, and fails if any expected counter test is missing
or failing. The tracked `.counter.json` report is bounded evidence: it records
`expected`, `selected`, and `missing` counts, per-test pass/fail status, and
keeps `release_signoff` and `all_metal_execution_proof` false. It is not a
benchmark result and it does not prove every `mklq-metal` operation stays on
Metal.
Current sampling counter coverage includes selected full-register and
partial-register counts-only Metal generated-draw/count accumulation,
the uniform-probability generated-count fast path,
full-register and partial-register sequential host draw telemetry after
resident Metal probability work, plus deterministic one-outcome draw-loop
bypasses.

## Metal Runtime Counter Summary

Use the summary renderer when the tracked `.counter.json` evidence changes and
you need to refresh the public coverage table:

```bash
python3 benchmarks/mklq/summarize_metal_runtime_counters.py \
  --reports benchmarks/mklq/reports \
  --output docs/mklq/metal-runtime-counters.md
```

The generated summary groups counter tests into resident gate,
probability/sampling, measurement/reset, unsupported-gate fallback,
synchronization, error-boundary, and runtime/device categories. It preserves
the same boundary as the raw probe report: runtime counter evidence only, not
release sign-off, not timing evidence, and not proof that every operation
stayed on Metal.
When multiple bounded reports are tracked, aggregate counts are summed across
reports. Repeated daily probes intentionally count the same selected counter
tests once per report.

## Metal Execution Boundary

Use [`docs/mklq/metal-execution-boundary.md`](../../docs/mklq/metal-execution-boundary.md)
when public wording needs to explain what `mklq-metal` resident Metal state
does and does not prove. That page is the human-readable boundary for
CPU-oracle fallback, synchronization, host-side sampling, error handling, and
the explicit non-claim that the current counter evidence is not proof that
every operation stayed on Metal.

## Metal Runtime Counter Docs Guard

Check that the tracked public counter summary still matches the tracked
bounded counter reports:

```bash
python3 benchmarks/mklq/check_metal_runtime_counter_docs.py
```

The guard does not run ctest or collect new Metal evidence. It regenerates the
Markdown summary from `benchmarks/mklq/reports/*.counter.json` in memory and
fails if `docs/mklq/metal-runtime-counters.md` is stale.

## Tracked Accepted Local Benchmark Evidence

To rerun the clean CPU benchmark gate, regenerate the sanitized summary, and
refresh the public evidence index, run:

```bash
python3 benchmarks/mklq/run_clean_cpu_benchmark.py \
  --pythonpath "${HOME}/.cudaq-mklq" \
  --stamp YYYY-MM-DD
```

Current default clean CPU runs include the `y-state`, `diagonal-phase-state`,
`ch-state`, `cy-state`, `crx-state`, `cry-state`, `crz-state`, `cz-state`,
`two-qubit-state`, `custom-two-qubit-state`, `dense-two-qubit-state`,
`controlled-dense-two-qubit-state`, `three-qubit-state`, `qft-like-state`,
`seeded-clifford-state`, `hardware-efficient-ansatz-state`, full-register
sampling, and partial-register sampling cases.

The gate writes ignored raw JSON under `benchmarks/mklq/results/`, writes the
sanitized summary under `benchmarks/mklq/reports/`, and refreshes
`docs/mklq/benchmark-evidence.md`. It refuses to collect clean evidence from a
dirty worktree unless `--allow-dirty` is passed explicitly.

If the raw JSON already exists and you only need to regenerate the sanitized
summary and public evidence index, run:

```bash
python3 benchmarks/mklq/run_clean_cpu_benchmark.py \
  --pythonpath "${HOME}/.cudaq-mklq" \
  --stamp 2026-06-21 \
  --composite-cases qft-like-state,seeded-clifford-state \
  --skip-benchmark
```

To rerun the focused CPU qubit-scaling evidence gate for the
hardware-efficient ansatz composite path, run:

```bash
python3 benchmarks/mklq/run_cpu_scaling_benchmark.py \
  --pythonpath "${HOME}/.cudaq-mklq" \
  --stamp YYYY-MM-DD \
  --cases hardware-efficient-ansatz-state \
  --summary-text "Clean-worktree local scaling run comparing qpp-cpu and mklq-cpu for q18/q20/q22 hardware-efficient ansatz composite state-vector updates." \
  --performance-scope "local Apple M5 q18-q22 hardware-efficient ansatz CPU target scaling comparison only; not a cross-machine release benchmark" \
  --runtime-note "The CUDA-Q Python runtime and source provenance are recorded from the raw benchmark report generated by run_cpu_scaling_benchmark.py for hardware-efficient ansatz rows."
```

To rerun the focused CPU qubit-scaling evidence gate for the built-in
two-qubit/SWAP, row-sparse custom 4x4 two-qubit, dense generic 4x4
two-qubit, controlled dense generic 4x4 two-qubit, and three-qubit custom
state-vector update paths, run:

```bash
python3 benchmarks/mklq/run_cpu_scaling_benchmark.py \
  --pythonpath "${HOME}/.cudaq-mklq" \
  --stamp YYYY-MM-DD-custom-two-qubit-scaling \
  --cases two-qubit-state,custom-two-qubit-state,dense-two-qubit-state,controlled-dense-two-qubit-state,three-qubit-state \
  --summary-text "Clean-worktree local scaling run comparing qpp-cpu and mklq-cpu for q18/q20/q22 SWAP/two-qubit, row-sparse custom 4x4 two-qubit, dense generic 4x4 two-qubit, controlled dense generic 4x4 two-qubit, and three-qubit custom state-vector updates." \
  --performance-scope "local Apple M5 q18-q22 built-in two-qubit, row-sparse custom 4x4 two-qubit, dense generic 4x4 two-qubit, controlled dense generic 4x4 two-qubit, and three-qubit CPU target scaling comparison only; not a cross-machine release benchmark" \
  --runtime-note "The CUDA-Q Python runtime and source provenance are recorded from the raw benchmark report generated by run_cpu_scaling_benchmark.py for built-in two-qubit, row-sparse custom 4x4 two-qubit, dense generic 4x4 two-qubit, controlled dense generic 4x4 two-qubit, and three-qubit rows."
```

For a compact table across all tracked sanitized summaries, run:

```bash
python3 benchmarks/mklq/summarize_reports.py \
  --reports benchmarks/mklq/reports \
  --format markdown \
  --output docs/mklq/benchmark-evidence.md
```

The generated public index is tracked at
`docs/mklq/benchmark-evidence.md`.

- `reports/local-clean-cpu-q20-2026-06-28.summary.json`: tracked sanitized
  summary for ignored raw results
  `results/local-clean-cpu-gate-y-cy-cz-q20-2026-06-28.json`
  (`sha256: fd46266986bd026c5db724194e5d66cdc092fae84fa74a4206de59f5038b355f`),
  `results/local-clean-cpu-composite-qft-like-seeded-clifford-hardware-efficient-ansatz-q20-2026-06-28.json`
  (`sha256: 3fa15408e1495cce37efaef455b0420674352ae30ba07df302a1eb8cdc754982`),
  and `results/local-clean-cpu-sampling-q20-2026-06-28.json`
  (`sha256: c17703381afc2ad21152836e591ba4c6362aee7cddebcc8d9d6f414d5799cdb0`).
  This run was collected from a clean worktree at
  `23d34ab226c3e4d7a47f15af3292bf81ce25987b` with `qpp-cpu` and
  `mklq-cpu` rows for `y-state`, `cy-state`, `cz-state`, `qft-like-state`,
  `seeded-clifford-state`, `hardware-efficient-ansatz-state`,
  `sample-full-register`, and `sample-partial-register` at q20 with
  `OMP_NUM_THREADS=10`, `OMP_PROC_BIND=close`, `OMP_DYNAMIC=false`,
  `VECLIB_MAXIMUM_THREADS=1`, `repeats=2`, `warmups=1`, and `layers=8` on
  Apple M5, 10 logical cores, 16 GB RAM, macOS 26.5.1. All 20 rows completed
  with `status == "ok"`. In this local run, q20 median elapsed ratios for
  `qpp-cpu` over `mklq-cpu` were 86.24x for `y-state`, 110.99x for
  `cy-state`, 96.82x for `cz-state`, 55.74x for `qft-like-state`, 107.64x for
  `seeded-clifford-state`, 87.95x for `hardware-efficient-ansatz-state`,
  81.41x for `sample-full-register` at 1024 shots, 84.25x for
  `sample-full-register` at 65536 shots, 102.36x for `sample-partial-register`
  at 1024 shots, and 90.39x for `sample-partial-register` at 65536 shots.
  Treat this as local clean-worktree CPU evidence, not as cross-machine
  performance certification.
- `reports/local-clean-cpu-q20-2026-07-03-two-three.summary.json`: tracked
  sanitized summary for ignored raw results
  `results/local-clean-cpu-gate-y-ch-cy-crx-cry-crz-cz-two-qubit-three-qubit-q20-2026-07-03-two-three.json`
  (`sha256: e45243bbdabaf2c79cb598e1592eddcdd7baa51fbbd7b6cc777d21c29243bbcc`),
  `results/local-clean-cpu-composite-qft-like-seeded-clifford-hardware-efficient-ansatz-q20-2026-07-03-two-three.json`
  (`sha256: d57ba2e9a520e8d6be69f4fe24cd27499534b4c81bda4e76c380490834546eeb`),
  and `results/local-clean-cpu-sampling-q20-2026-07-03-two-three.json`
  (`sha256: 7a66431362fd606c4ed96cab1546bfd9acbf6423f294cc86a7455cc738e9ec91`).
  This run was collected from a clean worktree at
  `dbebe3744f826ba4cbeed2b99708a2bdab03b11e` with `qpp-cpu` and `mklq-cpu`
  rows for `y-state`, `ch-state`, `cy-state`, `crx-state`, `cry-state`,
  `crz-state`, `cz-state`, `two-qubit-state`, `three-qubit-state`,
  `qft-like-state`, `seeded-clifford-state`,
  `hardware-efficient-ansatz-state`, `sample-full-register`, and
  `sample-partial-register` at q20 with `OMP_NUM_THREADS=10`,
  `OMP_PROC_BIND=close`, `OMP_DYNAMIC=false`, `VECLIB_MAXIMUM_THREADS=1`,
  `repeats=2`, `warmups=1`, and `layers=8` on Apple M5, 10 logical cores,
  16 GB RAM, macOS 26.5.1. All 32 rows completed with `status == "ok"`. In
  this local run, q20 median elapsed ratios for `qpp-cpu` over `mklq-cpu`
  included 56.82x for `two-qubit-state`, 41.92x for `three-qubit-state`,
  78.77x for `qft-like-state`, 98.46x for `seeded-clifford-state`, and 80.59x
  for `hardware-efficient-ansatz-state`. Treat this as local clean-worktree CPU
  evidence, not as cross-machine performance certification.
- `reports/local-clean-cpu-q20-2026-07-03.summary.json`: historical tracked
  sanitized summary for ignored raw results
  `results/local-clean-cpu-gate-y-ch-cy-crx-cry-crz-cz-q20-2026-07-03.json`
  (`sha256: d116aec76b8fcc5da0445e58ffe9596411cdba57665ce347f724d4ace24a9288`),
  `results/local-clean-cpu-composite-qft-like-seeded-clifford-hardware-efficient-ansatz-q20-2026-07-03.json`
  (`sha256: 88d0a2b69681c9fc958c936ba0205cd8b7b85dba12629f420fe73269efe9faea`),
  and `results/local-clean-cpu-sampling-q20-2026-07-03.json`
  (`sha256: 9e5f0b0a94ad4326f3059ac18f8f8ae92fe63baf84a7e14155f8f8fc9e81bf98`).
  This run was collected from a clean worktree at
  `e6843424cec9c636a76b71bb1c12a035401c2d00` with `qpp-cpu` and
  `mklq-cpu` rows for `y-state`, `ch-state`, `cy-state`, `crx-state`,
  `cry-state`, `crz-state`, `cz-state`, `qft-like-state`,
  `seeded-clifford-state`, `hardware-efficient-ansatz-state`,
  `sample-full-register`, and `sample-partial-register` at q20 with
  `OMP_NUM_THREADS=10`, `OMP_PROC_BIND=close`, `OMP_DYNAMIC=false`,
  `VECLIB_MAXIMUM_THREADS=1`, `repeats=2`, `warmups=1`, and `layers=8` on
  Apple M5, 10 logical cores, 16 GB RAM, macOS 26.5.1. All 28 rows completed
  with `status == "ok"`. In this local run, q20 median elapsed ratios for
  `qpp-cpu` over `mklq-cpu` were 44.90x for `y-state`, 36.07x for `ch-state`,
  122.20x for `cy-state`, 76.43x for `crx-state`, 78.64x for `cry-state`,
  96.49x for `crz-state`, 183.33x for `cz-state`, 54.08x for
  `qft-like-state`, 116.73x for `seeded-clifford-state`, 77.80x for
  `hardware-efficient-ansatz-state`, 81.62x for `sample-full-register` at
  1024 shots, 97.38x for `sample-full-register` at 65536 shots, 85.98x for
  `sample-partial-register` at 1024 shots, and 124.96x for
  `sample-partial-register` at 65536 shots. Treat this as local clean-worktree
  CPU evidence, not as cross-machine performance certification.
- `reports/local-clean-cpu-q20-2026-06-30.summary.json`: historical tracked
  sanitized summary for ignored raw results
  `results/local-clean-cpu-gate-y-cy-cz-q20-2026-06-30.json`
  (`sha256: 2cacb592d4e37d1c32877fc6303a0d347cfce48768cd2d4cdfce0d1531d83b2b`),
  `results/local-clean-cpu-composite-qft-like-seeded-clifford-hardware-efficient-ansatz-q20-2026-06-30.json`
  (`sha256: b79f9b2ef836f5b715595efda231e5560cc5f408e60a07391eba54493ff47653`),
  and `results/local-clean-cpu-sampling-q20-2026-06-30.json`
  (`sha256: 219bb960f0e079501f97fd26f564a4590d54a70d260202d62e639f4cb42b9a83`).
  This run was collected from a clean worktree at
  `61e5f099f2d3b87feb6c3e7cf27d37f1e1d77c04` with `qpp-cpu` and
  `mklq-cpu` rows for `y-state`, `cy-state`, `cz-state`, `qft-like-state`,
  `seeded-clifford-state`, `hardware-efficient-ansatz-state`,
  `sample-full-register`, and `sample-partial-register` at q20 with
  `OMP_NUM_THREADS=10`, `OMP_PROC_BIND=close`, `OMP_DYNAMIC=false`,
  `VECLIB_MAXIMUM_THREADS=1`, `repeats=2`, `warmups=1`, and `layers=8` on
  Apple M5, 10 logical cores, 16 GB RAM, macOS 26.5.1. All 20 rows completed
  with `status == "ok"`. In this local run, q20 median elapsed ratios for
  `qpp-cpu` over `mklq-cpu` were 123.52x for `y-state`, 116.93x for
  `cy-state`, 136.95x for `cz-state`, 80.09x for `qft-like-state`, 155.55x for
  `seeded-clifford-state`, 100.99x for `hardware-efficient-ansatz-state`,
  93.85x for `sample-full-register` at 1024 shots, 122.15x for
  `sample-full-register` at 65536 shots, 138.15x for `sample-partial-register`
  at 1024 shots, and 126.38x for `sample-partial-register` at 65536 shots.
  Treat this as local clean-worktree CPU evidence, not as cross-machine
  performance certification.
- `reports/local-crz-distance-sweep-cpu-q20-2026-07-01.summary.json`: tracked
  sanitized summary for the ignored raw result
  `results/local-clean-cpu-crz-distance-sweep-q20-2026-07-01.json`
  (`sha256: e502854a8ca2af9b5beef5840ccabc127dd9bf131e78371f2430cd451f57e8ad`).
  This run was collected from a clean worktree at
  `a311c8749bbf5edfa553f64eb71a79faeafdd803` with `qpp-cpu` and `mklq-cpu`
  rows for `crz-distance-sweep-state` at q20, distances 1 through 19, with
  `OMP_NUM_THREADS=10`, `OMP_PROC_BIND=close`, `OMP_DYNAMIC=false`,
  `VECLIB_MAXIMUM_THREADS=1`, `repeats=2`, `warmups=1`, and `layers=8` on
  Apple M5, 10 logical cores, 16 GB RAM, macOS 26.5.1. All 38 rows completed
  with `status == "ok"`. In this local run, the minimum median elapsed ratio
  for `qpp-cpu` over `mklq-cpu` across the distance sweep was 68.56x. Treat
  this as local clean-worktree CPU evidence, not as cross-machine performance
  certification.
- `reports/local-scaling-cpu-hardware-efficient-ansatz-q18-q22-2026-06-30.summary.json`:
  tracked sanitized summary for the ignored raw result
  `results/local-scaling-cpu-hardware-efficient-ansatz-q18-q22-2026-06-30.json`
  (`sha256: 26721c3b56f9c08234b4fcbe5e96b72a781edc77addea1702e8aa4047c45b859`).
  This run was collected from a clean worktree at
  `f2d87a4bf1e0d0163481a560df868292715a660a` with `qpp-cpu` and `mklq-cpu`
  rows for `hardware-efficient-ansatz-state` at q18, q20, and q22 with
  `OMP_NUM_THREADS=10`, `OMP_PROC_BIND=close`, `OMP_DYNAMIC=false`,
  `VECLIB_MAXIMUM_THREADS=1`, `repeats=3`, `warmups=1`, and `layers=8` on
  Apple M5, 10 logical cores, 16 GB RAM, macOS 26.5.1. All six rows completed
  with `status == "ok"`. In this local run, q18/q20/q22 median elapsed ratios
  for `qpp-cpu` over `mklq-cpu` were 26.84x, 52.94x, and 81.37x. Treat this as
  local clean-worktree CPU scaling evidence, not as cross-machine performance
  certification.
- `reports/local-scaling-cpu-two-qubit-three-qubit-q18-q22-2026-07-03-two-three-scaling.summary.json`:
  tracked sanitized summary for the ignored raw result
  `results/local-scaling-cpu-two-qubit-three-qubit-q18-q22-2026-07-03-two-three-scaling.json`
  (`sha256: 95dacd993ab733dff776683e4ca6ac06fbd414ae3005f8f855d23a4f59858ee2`).
  This run was collected from a clean worktree at
  `cb688b20c825a970965ffe41ca84757287abf847` with `qpp-cpu` and `mklq-cpu`
  rows for `two-qubit-state` and `three-qubit-state` at q18, q20, and q22 with
  `OMP_NUM_THREADS=10`, `OMP_PROC_BIND=close`, `OMP_DYNAMIC=false`,
  `VECLIB_MAXIMUM_THREADS=1`, `repeats=3`, `warmups=1`, and `layers=8` on
  Apple M5, 10 logical cores, 16 GB RAM, macOS 26.5.1. All 12 rows completed
  with `status == "ok"`. In this local run, q18/q20/q22 median elapsed ratios
  for `qpp-cpu` over `mklq-cpu` were 47.20x, 131.99x, and 163.42x for
  `two-qubit-state`, and 24.54x, 87.34x, and 90.91x for `three-qubit-state`.
  Treat this as local clean-worktree CPU scaling evidence, not as cross-machine
  performance certification.
- `reports/local-clean-cpu-q20-2026-06-21.summary.json`: tracked sanitized
  summary for ignored raw results
  `results/local-clean-cpu-gate-y-cy-cz-q20-2026-06-21.json`
  (`sha256: 2b438094b63bf0dda2a06be2785b75ca54fdb2c8b2fa74d6ba212b6fea832ef0`),
  `results/local-clean-cpu-composite-qft-like-seeded-clifford-q20-2026-06-21.json`
  (`sha256: b07b3ba92b83c0db12ad560ab650e3be035f543fb690dcb5ff946852e6eb423f`),
  and `results/local-clean-cpu-sampling-q20-2026-06-21.json`
  (`sha256: 167b5c4adef8fa0da682e05c841f0475da0570bc50483739b71b8d6fcab2716a`).
  This run was collected from a clean worktree at
  `34f4b260d1c657ad626c526eed4e6b9d3a441be4` with `qpp-cpu` and
  `mklq-cpu` rows for `y-state`, `cy-state`, `cz-state`, `qft-like-state`,
  `seeded-clifford-state`, `sample-full-register`, and
  `sample-partial-register` at q20 with `OMP_NUM_THREADS=10`,
  `OMP_PROC_BIND=close`, `OMP_DYNAMIC=false`, `VECLIB_MAXIMUM_THREADS=1`,
  `repeats=2`, `warmups=1`, and `layers=8` on Apple M5, 10 logical cores,
  16 GB RAM, macOS 26.5.1. All 18 rows completed with `status == "ok"`.
  In this local run, q20 median elapsed ratios for `qpp-cpu` over `mklq-cpu`
  were 120.44x for `y-state`, 99.08x for `cy-state`, 121.47x for `cz-state`,
  54.63x for `qft-like-state`, 97.56x for `seeded-clifford-state`,
  139.93x for `sample-full-register` at 1024 shots, 106.66x for
  `sample-full-register` at 65536 shots, 166.86x for
  `sample-partial-register` at 1024 shots, and 120.60x for
  `sample-partial-register` at 65536 shots. Treat this as local
  clean-worktree CPU evidence, not as cross-machine performance certification.
- `reports/local-current-sampling-fullprob-gated-q20-2026-06-19.summary.json`:
  tracked sanitized summary for the ignored raw result
  `results/local-current-sampling-fullprob-gated-q20-2026-06-19.json`
  (`sha256: 8ca6a4f7a7aea1670aa572ea6897a125ea4ff0a9e0d1d93502c1158e81ba33b3`).
  isolated `qpp-cpu`, `mklq-cpu`, and `mklq-metal` rows for
  `sample-full-register` and `sample-partial-register` at q20 with
  `OMP_NUM_THREADS=10`, `OMP_PROC_BIND=close`, `OMP_DYNAMIC=false`,
  `shots=1024`, `repeats=2`, `warmups=1`, and `layers=4` on Apple M5,
  10 logical cores, 16 GB RAM, macOS 26.5.1. All six rows completed with
  `status == "ok"`. Treat this as local tuning evidence for the Metal
  partial-register sampling cost gate, not as clean-release provenance. In this
  run, q20 `mklq-metal` median elapsed time was 0.0370576665 s for
  `sample-full-register` and 0.022011521 s for `sample-partial-register`.
  The same-day pre-gate probe
  `results/local-current-sampling-shot-scaling-q20-2026-06-19.json` measured
  0.255696875 s for the q20 `mklq-metal` `sample-partial-register` row at
  1024 shots with `repeats=1`, so use the comparison as a tuning signal rather
  than a formal release benchmark.
- `reports/local-counts-only-sampling-shot-scaling-q20-2026-06-19.summary.json`:
  tracked sanitized summary for the ignored raw result
  `results/local-counts-only-sampling-shot-scaling-q20-2026-06-19.json`
  (`sha256: ef9846673b461e3abc6d359933408be58e1f745d8b68738b757a76339f9b5092`).
  Isolated `qpp-cpu`, `mklq-cpu`, and `mklq-metal` rows for
  `sample-full-register` and `sample-partial-register` at q20 with
  `OMP_NUM_THREADS=10`, `OMP_PROC_BIND=close`, `OMP_DYNAMIC=false`, shot counts
  `256,1024,8192,65536`, `repeats=2`, `warmups=1`, and `layers=8` on Apple M5,
  10 logical cores, 16 GB RAM, macOS 26.5.1. All 24 rows completed with
  `status == "ok"`. Treat this as local dirty-worktree tuning evidence for the
  standard non-explicit `cudaq.sample` counts-only backend path, not as
  clean-release provenance. The benchmark does not call
  `sample_result::sequential_data()`, so it measures backend sample/count
  aggregation rather than the public accessor's lazy counts expansion. In this
  run at q20 and 65536 shots, `mklq-cpu` median elapsed time was
  0.01916737499414012 s for `sample-full-register` and 0.016119854502903763 s
  for `sample-partial-register`; `mklq-metal` median elapsed time was
  0.04015256251295796 s and 0.03547552099917084 s for the same cases.
  This remains historical static evidence for the earlier stochastic
  `mklq-metal` host-side sample draw/count boundary.
- `reports/local-metal-sampling-boundary-q22-2026-07-04.summary.json`:
  tracked sanitized summary for the ignored raw result
  `results/local-metal-sampling-boundary-q22-2026-07-04.json`
  (`sha256: c351ec6c2b3e2b32344d63a6386f222a83270a36cfa332b3ece2ef4d12d828c2`).
  Isolated `mklq-metal` rows for `sample-full-register` and
  `sample-partial-register` at q22 with `OMP_NUM_THREADS=10`,
  `OMP_PROC_BIND=close`, `OMP_DYNAMIC=false`, `VECLIB_MAXIMUM_THREADS=1`,
  shot counts `256,1024,8192,65536`, `repeats=2`, `warmups=1`, and
  `layers=8` on Apple M5, 10 logical cores, 16 GB RAM, macOS 26.5.1.
  All 8 rows completed with `status == "ok"`. Treat this as local tuning
  evidence for the standard non-explicit `cudaq.sample` counts-only backend
  path, not as clean-release provenance, cross-machine certification, or proof
  of an on-device sampler. The q22 high-shot versus low-shot median elapsed
  ratios were 1.588x for full-register sampling and 1.113x for
  partial-register sampling, under the static guard threshold of 2.0 used for
  that historical boundary.
- `reports/local-metal-partial-count-accumulation-sampling-q20-2026-07-05.summary.json`:
  tracked sanitized summary for the ignored raw result
  `results/local-metal-partial-count-accumulation-sampling-q20-2026-07-05.json`
  (`sha256: e96c244a6ce3e7d40cca717659452df845ed1f24400b3cf8b2ce82fa43245e0e`).
  Isolated `mklq-metal` rows for `sample-full-register` and
  `sample-partial-register` at q20 with shot counts
  `256,1024,8192,65536`, `repeats=2`, `warmups=1`, and `layers=8` on Apple
  M5, 10 logical cores, 16 GB RAM, macOS 26.5.1. All 8 rows completed with
  `status == "ok"` from a clean worktree. Treat this as local tuning evidence
  for selected full-register and partial-register Metal sample-count
  accumulation after host-generated draws, not as release readiness,
  cross-machine certification, Metal RNG evidence, or a full on-device sampler.
  The q20 high-shot versus low-shot median elapsed ratios were 1.284x for
  full-register sampling and 0.836x for partial-register sampling.
- `reports/local-metal-partial-count-accumulation-sampling-q22-2026-07-05.summary.json`:
  tracked sanitized summary for the ignored raw result
  `results/local-metal-partial-count-accumulation-sampling-q22-2026-07-05.json`
  (`sha256: ad5e8ca8091146ce4f3bff82d3b0d755da912b8f69a1c5dd9aa3b02819eb9016`).
  Isolated `mklq-metal` rows for `sample-full-register` and
  `sample-partial-register` at q22 with shot counts
  `256,1024,8192,65536`, `repeats=2`, `warmups=1`, and `layers=8` on Apple
  M5, 10 logical cores, 16 GB RAM, macOS 26.5.1. All 8 rows completed with
  `status == "ok"` from a clean worktree. Treat this as local tuning evidence
  for selected full-register and partial-register Metal sample-count
  accumulation after host-generated draws. The q22 high-shot versus low-shot
  median elapsed ratios were 0.955x for full-register sampling and 1.152x for
  partial-register sampling.
- `reports/local-metal-partial-count-accumulation-sampling-q24-2026-07-05.summary.json`:
  tracked sanitized summary for the ignored raw result
  `results/local-metal-partial-count-accumulation-sampling-q24-2026-07-05.json`
  (`sha256: be9062049a05c2bfcfa49e4071a58604d4cad3846588a3dcec42d248fe4e37dd`).
  Isolated `mklq-metal` rows for `sample-full-register` and
  `sample-partial-register` at q24 with shot counts
  `256,1024,8192,65536`, `repeats=2`, `warmups=1`, and `layers=8` on Apple
  M5, 10 logical cores, 16 GB RAM, macOS 26.5.1. All 8 rows completed with
  `status == "ok"` from a clean worktree. Treat this as the current largest
  tracked local Metal counts-only sampling boundary run, not as release or
  cross-machine performance certification. The q24 high-shot versus low-shot
  median elapsed ratios were 0.984x for full-register sampling and 0.941x for
  partial-register sampling.
- `reports/local-metal-uniform-partial-sampling-q20-q24-2026-07-05.summary.json`:
  tracked sanitized summary for the ignored raw result
  `results/local-metal-uniform-partial-sampling-q20-q24-2026-07-05.json`
  (`sha256: a9505bdef5818c9055944ac0d2702edbf000428702355779299cc84310e4c7ec`).
  Isolated `mklq-metal` rows for `sample-uniform-partial-register` at
  q20/q22/q24 with shot counts `256,1024,8192,65536`, `repeats=2`,
  `warmups=1`, and `layers=8` on Apple M5, 10 logical cores, 16 GB RAM,
  macOS 26.5.1. All 12 rows completed with `status == "ok"` from a dirty
  worktree because this benchmark fixture and public docs were being added in
  the same change. Treat this as local tuning evidence for the
  uniform-probability generated-count fast path, not as clean release
  provenance or cross-machine certification. The high-shot versus low-shot
  median elapsed ratios were 1.285x at q20, 0.892x at q22, and 1.110x at q24.
- `reports/local-metal-count-accumulation-sampling-q20-2026-07-04.summary.json`:
  tracked sanitized summary for the ignored raw result
  `results/local-metal-count-accumulation-sampling-q20-2026-07-04.json`
  (`sha256: 751c59993fc4590b9729cc321ae1f328c0655b421b79ff596ddbaaf590b1bf8e`).
  Isolated `mklq-metal` rows for `sample-full-register` and
  `sample-partial-register` at q20 with shot counts
  `256,1024,8192,65536`, `repeats=2`, `warmups=1`, and `layers=8` on Apple
  M5, 10 logical cores, 16 GB RAM, macOS 26.5.1. All 8 rows completed with
  `status == "ok"` from a clean worktree. Treat this as local tuning evidence
  for the selected full-register Metal sample-count accumulation path after
  host-generated draws; the partial-register rows remain historical
  host-side-boundary evidence collected before selected partial-register Metal
  count accumulation was enabled. The q20 high-shot versus low-shot median elapsed ratios were
  0.988x for full-register sampling and 0.560x for partial-register sampling.
- `reports/local-metal-count-accumulation-sampling-q22-2026-07-04.summary.json`:
  tracked sanitized summary for the ignored raw result
  `results/local-metal-count-accumulation-sampling-q22-2026-07-04.json`
  (`sha256: 0a270eda1a9725b5cddf27a292363fdb495770494c77b746b950eba17a4c0d33`).
  Isolated `mklq-metal` rows for `sample-full-register` and
  `sample-partial-register` at q22 with shot counts
  `256,1024,8192,65536`, `repeats=2`, `warmups=1`, and `layers=8` on Apple
  M5, 10 logical cores, 16 GB RAM, macOS 26.5.1. All 8 rows completed with
  `status == "ok"` from a clean worktree. Treat this as local tuning evidence
  for the same selected full-register Metal sample-count accumulation boundary;
  the partial-register rows remain historical host-side-boundary evidence
  collected before selected partial-register Metal count accumulation was
  enabled. Treat it as local tuning evidence, not as release readiness,
  cross-machine certification, or proof of a Metal RNG/device-side sampler. The
  q22 high-shot versus low-shot median elapsed ratios were 0.914x for
  full-register sampling and 0.943x for partial-register sampling.
- `reports/local-y-cy-fastpath-isolated-q20-2026-06-19.summary.json`:
  tracked sanitized summary for the ignored raw result
  `results/local-y-cy-fastpath-isolated-q20-2026-06-19.json`
  (`sha256: 93bce3b77fccce0ce48611fbccc2a88d81e31b8a34f4885ff9235750178701fa`).
  Isolated `qpp-cpu` and `mklq-cpu` rows for `y-state` and `cy-state` at q20
  with `OMP_NUM_THREADS=10`, `OMP_PROC_BIND=close`, `OMP_DYNAMIC=false`,
  `shots=1024`, `repeats=2`, `warmups=1`, and `layers=8` on Apple M5,
  10 logical cores, 16 GB RAM, macOS 26.5.1. All four rows completed with
  `status == "ok"`. Treat this as local dirty-worktree tuning evidence for the
  CPU built-in Y/CY structured fast path, not as clean-release provenance. In
  this run, q20 `mklq-cpu` median elapsed time was 0.04815118750411784 s for
  `y-state` and 0.08607120799570112 s for `cy-state`.
- `reports/local-metal-y-cy-resident-isolated-q20-2026-06-19.summary.json`:
  tracked sanitized summary for the ignored raw result
  `results/local-metal-y-cy-resident-isolated-q20-2026-06-19.json`
  (`sha256: 84891e8f907c38295a4975b1d0b0c493c2658b9b36b29975c539b93fcdfff9bb`).
  Isolated `qpp-cpu`, `mklq-cpu`, and `mklq-metal` rows for `y-state` and
  `cy-state` at q20 with `OMP_NUM_THREADS=10`, `OMP_PROC_BIND=close`,
  `OMP_DYNAMIC=false`, `shots=1024`, `repeats=2`, `warmups=1`, and `layers=8`
  on Apple M5, 10 logical cores, 16 GB RAM, macOS 26.5.1. All six rows
  completed with `status == "ok"`. Treat this as local dirty-worktree tuning
  evidence for resident fp32 Metal Y/CY gate updates followed by host readback
  for `cudaq.get_state`, not as clean-release provenance. In this run, q20
  `mklq-metal` median elapsed time was 0.09025897899846314 s for `y-state` and
  0.09229137500369688 s for `cy-state`. The summary's path labels are curated
  labels inferred from runtime tests and code inspection; the raw benchmark
  JSON does not currently emit resident-path counters.
- `reports/local-metal-composite-mixed-path-q20-2026-06-21.summary.json`:
  tracked sanitized summary for the ignored raw result
  `results/local-metal-composite-mixed-path-q20-2026-06-21.json`
  (`sha256: ef58b59222218df43d39212fb1d0fb824d7228516305f89af7a2f043101a70a9`).
  Isolated `qpp-cpu`, `mklq-cpu`, and `mklq-metal` rows for
  `qft-like-state` and `seeded-clifford-state` at q20 with
  `OMP_NUM_THREADS=10`, `OMP_PROC_BIND=close`, `OMP_DYNAMIC=false`,
  `VECLIB_MAXIMUM_THREADS=1`, `shots=1024`, `repeats=2`, `warmups=1`, and
  `layers=8` on Apple M5, 10 logical cores, 16 GB RAM, macOS 26.5.1. All six
  rows completed with `status == "ok"`. Treat this as local dirty-worktree
  tuning evidence for experimental `mklq-metal` mixed-path composite
  state-vector updates followed by host readback, not as clean-release
  provenance. In this run, q20 `mklq-metal` median elapsed time was
  1.2168069164908957 s for `qft-like-state` and 0.1892540624976391 s for
  `seeded-clifford-state`; the same-day `qpp-cpu` over `mklq-metal` ratios were
  56.00x and 70.50x. The `mklq-metal` row was slightly faster than `mklq-cpu`
  for `qft-like-state` and slower for `seeded-clifford-state`, so do not read
  this as a general Metal-is-always-faster claim.
- `reports/local-metal-three-qubit-resident-q20-2026-06-22.summary.json`:
  tracked sanitized summary for the ignored raw result
  `results/local-metal-three-qubit-resident-q20-2026-06-22.json`
  (`sha256: daed4c1deb2d2cc470428e6000cc15267776b132f37356335078b3e0ab39ebbe`).
  Isolated `qpp-cpu`, `mklq-cpu`, and `mklq-metal` rows for
  `three-qubit-state` at q20 with `OMP_NUM_THREADS=10`,
  `OMP_PROC_BIND=close`, `OMP_DYNAMIC=false`, `VECLIB_MAXIMUM_THREADS=1`,
  `shots=1024`, `repeats=2`, `warmups=1`, and `layers=8` on Apple M5,
  10 logical cores, 16 GB RAM, macOS 26.5.1. All three rows completed with
  `status == "ok"`. Treat this as local dirty-worktree tuning evidence for the
  resident fp32 Metal three-target custom gate update followed by host readback,
  not as clean-release provenance. In this run, q20 `mklq-metal` median elapsed
  time was 0.15560556251148228 s for `three-qubit-state`; the same-day
  `qpp-cpu` over `mklq-metal` ratio was 54.67x.

## Untracked Local Benchmark Notes

The following ignored `results/*.json` files are local development notes only.
They are useful for understanding why a path was tuned, but they are not
accepted commit evidence unless a tracked sanitized summary under `reports/`
records their hashes and bounded metrics.

- `results/local-controlled-h-fastpath-isolated-q15-q20-2026-06-18.json`:
  isolated `qpp-cpu` and `mklq-cpu` rows for `ch-state` at q15-q20 with
  `OMP_NUM_THREADS=10`, `shots=1024`, `repeats=2`, `warmups=1`, and `layers=8`
  on Apple M5, 10 logical cores, 16 GB RAM, macOS 26.5.1. All 12 rows completed
  with `status == "ok"`. The benchmark JSON records a dirty worktree, so treat
  this as local development evidence for the CPU built-in controlled H
  structured fast path, not as clean-release provenance. At q20, median elapsed
  time was 9.98382543751 s for `qpp-cpu` and 0.103235583993 s for `mklq-cpu`, a
  96.71x local cross-target ratio. This run was collected after the controlled H
  fast path was already implemented, so it is not a before/after speedup claim.
- `results/local-controlled-rotation-fastpath-isolated-q15-q20-2026-06-18.json`:
  isolated `qpp-cpu` and `mklq-cpu` rows for `crx-state`, `cry-state`, and
  `crz-state` at q15-q20 with `OMP_NUM_THREADS=10`, `shots=1024`,
  `repeats=2`, `warmups=1`, and `layers=8` on Apple M5, 10 logical cores,
  16 GB RAM, macOS 26.5.1. All 36 rows completed with `status == "ok"`. The
  benchmark JSON records a dirty worktree, so treat this as local
  development evidence for the CPU built-in controlled Rx/Ry/Rz structured fast
  paths, not as clean-release provenance. Compared with
  `results/local-controlled-rotation-breakdown-isolated-q15-q20-2026-06-18.json`,
  q20 `mklq-cpu` median elapsed time changed from 0.128347500002 s to
  0.0893733750054 s for `crx-state`, from 0.127916166493 s to
  0.0786046455032 s for `cry-state`, and from 0.123464874996 s to
  0.115874896001 s for `crz-state`. That is 1.44x, 1.63x, and 1.07x local q20
  speedup respectively. Across q15-q20 this run improved all three dedicated
  controlled-rotation cases, but CRZ's q20 improvement was small, so rerun with
  higher repeats before making fine-grained CRZ tuning claims.
- `results/local-controlled-rotation-breakdown-isolated-q15-q20-2026-06-18.json`:
  isolated `qpp-cpu` and `mklq-cpu` rows for `crx-state`, `cry-state`, and
  `crz-state` at q15-q20 with the same command shape as the fast-path run
  above. All 36 rows completed with `status == "ok"`, and this JSON also records
  a dirty worktree. Treat this as the local pre-fast-path breakdown that
  identified built-in controlled rotations as a remaining `controlled-state`
  hot path, not as current performance evidence or clean-release provenance.
- `results/local-single-gate-fastpath-isolated-q15-q20-2026-06-18.json`:
  isolated `qpp-cpu` and `mklq-cpu` rows for `h-state`, `rx-state`,
  `ry-state`, and `rz-state` at q15-q20 with `OMP_NUM_THREADS=10`,
  `shots=1024`, `repeats=2`, `warmups=1`, and `layers=8` on Apple M5,
  10 logical cores, 16 GB RAM, macOS 26.5.1. All 48 rows completed with
  `status == "ok"`. The benchmark JSON records a dirty worktree, so treat this
  as local development evidence for the CPU built-in uncontrolled
  H/Rx/Ry/Rz structured fast paths, not as clean-release provenance. Compared with
  `results/local-single-gate-breakdown-isolated-q15-q20-2026-06-18.json`,
  q20 `mklq-cpu` median elapsed time changed from 0.110392645998 s to
  0.0517783540017 s for `h-state`, from 0.128242312501 s to
  0.0645899794981 s for `rx-state`, from 0.104353729501 s to
  0.0679914585016 s for `ry-state`, and from 0.107659312496 s to
  0.0797631040004 s for `rz-state`. That is 2.13x, 1.99x, 1.53x, and 1.35x
  local q20 speedup respectively. Across q15-q20, H/Rx/Ry improved in this
  run; Rz improved from q16-q20 but the q15 row was slower, 0.02130858349846676 s
  after versus 0.018260312001075363 s before, so do not cite this as an
  all-size Rz win without a higher-repeat follow-up.
- `results/local-single-gate-breakdown-isolated-q15-q20-2026-06-18.json`:
  isolated `qpp-cpu` and `mklq-cpu` rows for `h-state`, `rx-state`,
  `ry-state`, and `rz-state` at q15-q20 with the same command shape as the
  fast-path run above. All 48 rows completed with `status == "ok"`, and this
  JSON also records a dirty worktree. Treat this as the local pre-fast-path
  breakdown that identified `rx-state` as the slowest q20 `mklq-cpu` dedicated
  single-gate case before the structured H/Rx/Ry/Rz CPU fast paths landed, not
  as current performance evidence or clean-release provenance.
- `results/local-cz-fastpath-isolated-q15-q20-2026-06-18.json`:
  isolated `qpp-cpu` and `mklq-cpu` rows for `cz-state` at q15-q20 with
  `OMP_NUM_THREADS=10`, `shots=1024`, `repeats=2`, `warmups=1`, and `layers=8`
  on Apple M5, 10 logical cores, 16 GB RAM, macOS 26.5.1. All 12 rows completed
  with `status == "ok"`. Treat this as the local cross-target evidence
  for the CPU built-in CZ phase fast path. At q20, median elapsed time was
  7.334453104002023 s for `qpp-cpu` and 0.10550895799678983 s for `mklq-cpu`, a
  69.51x local cross-target ratio. This benchmark is not a before/after
  comparison against an older MKL-Q implementation, because `cz-state` was added
  with this change.
- `results/local-bitflip-fastpath-isolated-q15-q20-2026-06-18.json`:
  isolated `mklq-cpu` rows for `gate-state` and `controlled-state` at q15-q20
  with `OMP_NUM_THREADS=10`, `shots=1024`, `repeats=2`, `warmups=1`, and
  `layers=8` on Apple M5, 10 logical cores, 16 GB RAM, macOS 26.5.1. All 12
  rows completed with `status == "ok"`. Treat this as the local
  evidence for the CPU built-in CNOT/controlled-X bit-flip permutation fast
  path. Compared with
  `results/local-focused-allcases-isolated-q15-q20-2026-06-18.json`, median q20
  `gate-state` elapsed time changed from 0.683557729000313 s to
  0.25968133350033895 s, a 2.63x local speedup, and q20 `controlled-state`
  elapsed time changed from 0.4340257920011936 s to 0.2211792920024891 s, a
  1.96x local speedup. Built-in X correctness is covered by unit tests, but this
  benchmark does not claim standalone X performance improvements.
- `results/local-swap-fastpath-isolated-q15-q20-2026-06-18.json`:
  isolated `mklq-cpu` rows for `two-qubit-state` at q15-q20 with
  `OMP_NUM_THREADS=10`, `shots=1024`, `repeats=2`, `warmups=1`, and `layers=8`
  on Apple M5, 10 logical cores, 16 GB RAM, macOS 26.5.1. All six rows
  completed with `status == "ok"`. Treat this as the local evidence
  for the CPU SWAP permutation fast path. Compared with
  `results/local-focused-allcases-isolated-q15-q20-2026-06-18.json`, median
  `two-qubit-state` elapsed time at q20 changed from 0.3724326045012276 s to
  0.04745125000044936 s, a 7.85x local speedup. q17-q20 improved by
  3.53x-7.85x, and all q15-q20 rows remained `status == "ok"` in this rerun.
- `results/local-focused-allcases-isolated-q15-q20-2026-06-18.json`:
  isolated `qpp-cpu`, `mklq-cpu`, and `mklq-metal` rows for
  `gate-state`, `sample-basis`, `sample-ghz`, `sample-full-register`,
  `sample-partial-register`, `single-qubit-state`, `controlled-state`, and
  `two-qubit-state` at q15-q20 with `OMP_NUM_THREADS=10`, `shots=1024`,
  `repeats=2`, `warmups=1`, and `layers=8` on Apple M5, 10 logical cores,
  16 GB RAM, macOS 26.5.1. All 144 rows completed with `status == "ok"`.
  Treat this as the current local focused benchmark baseline for choosing the
  next hot path, not as portable performance evidence. In its q20 rows,
  `mklq-cpu` median elapsed time was 0.6906239789932442 s for
  `single-qubit-state`, 0.683557729000313 s for `gate-state`,
  0.4340257920011936 s for `controlled-state`, and
  0.3724326045012276 s for `two-qubit-state`; these state-update rows are the
  largest remaining `mklq-cpu` local latencies in this benchmark. In the same
  q20 rows, `mklq-metal` was faster than `mklq-cpu` for dense state-update and
  dense sampling cases, but slower for `sample-basis` and `sample-ghz`, so do
  not move sparse sampling work to Metal without a separate benchmark gate.
- `results/local-metal-sampling-shot-scaling-q15-q20-2026-06-18.json`:
  isolated `qpp-cpu`, `mklq-cpu`, and `mklq-metal` rows for
  `sample-full-register` and `sample-partial-register` at q15-q20 with
  `OMP_NUM_THREADS=10`, `shot_counts=256,1024,8192,65536`, `repeats=2`, and
  `warmups=1` on Apple M5, 10 logical cores, 16 GB RAM, macOS 26.5.1. All
  144 rows completed with `status == "ok"`. Treat this as the local
  shot-scaling gate for sample draw/count decisions, not as portable
  performance evidence. In its q20 `mklq-metal` rows, median elapsed time
  changed from 0.02171218749936088 s at 256 shots to 0.027931499997066567 s at
  65536 shots for `sample-full-register`, and from 0.024053000001003966 s to
  0.02549454149630037 s for `sample-partial-register`; at the time, this
  measured range did not justify moving sample count accumulation onto the GPU.
  The next accepted low-risk step was host-side counts-only aggregation for
  `includeSequentialData=false`, using bounded dense counters for small outcome
  spaces and sparse maps for larger outcome spaces. A later bounded counter
  fixtures add deterministic one-outcome sequential and counts-only draw-loop
  bypasses after probability work; this is still not Metal RNG or general GPU
  count accumulation.
- `results/local-sample-basis-isolated-q15-q20-2026-06-18.json`:
  isolated `qpp-cpu`, `mklq-cpu`, and `mklq-metal` rows for deterministic
  `sample-basis` at q15-q20 with `OMP_NUM_THREADS=10`, `shots=1024`,
  `repeats=2`, and `warmups=1`. This run covers sparse full-register sampling
  from the allocated `|0...0>` state with one deterministic outcome. Treat it
  as local Apple M5 evidence for the deterministic sparse sampling path only.
- `results/local-sampling-full-partial-fullprob-isolated-q15-q20-2026-06-18.json`:
  current isolated `qpp-cpu`, `mklq-cpu`, and `mklq-metal` rows for
  `sample-full-register` and `sample-partial-register` at q15-q20 with
  `OMP_NUM_THREADS=10`, `shots=1024`, `repeats=2`, and `warmups=1`.
  This run covers the current `mklq-metal` partial-register path, which fills
  resident full-register probabilities once and folds them to marginal
  probabilities on the host. Treat this as local Apple M5 evidence for sampling
  latency and memory rows, not as a portable performance claim.
- `results/local-sampling-full-partial-isolated-q15-q20-2026-06-18.json`:
  historical before/after comparison point for the earlier isolated
  `sample-full-register` and `sample-partial-register` run. Its Metal
  partial-register rows used the earlier marginal scan path, so do not treat it
  as evidence for the current implementation.

## Probability Kernel Microbenchmark

Use this standalone C++ microbenchmark before changing the dense
full-register probability helper in `runtime/nvqir/mklq`. It compares the
probability-vector fill kernels without Python or CUDA-Q target overhead:

```bash
OMP_NUM_THREADS=4 \
python3 benchmarks/mklq/bench_probability_kernels.py \
  --variants scalar-norm,scalar-split,accelerate-interleaved,accelerate-vdsp,openmp-split \
  --qubits 15,16,17,18,19,20 \
  --repeats 5 \
  --warmups 2 \
  --output benchmarks/mklq/results/local-probability-kernels-interleaved-vdsp-omp4-q15-q20-2026-06-19.json
```

The local Apple M5 run in
`results/local-probability-kernels-interleaved-vdsp-omp4-q15-q20-2026-06-19.json`
with 10 logical cores, 16 GB RAM, macOS 26.5.1, and `OMP_NUM_THREADS=4`
produced `ok` rows for all variants. In that run, `scalar-split` was fastest at
q15 and `openmp-split` was fastest at q16-q20. The runtime-shaped
`accelerate-interleaved` variant was slower than `openmp-split` across q16-q20;
at q20 its median elapsed time was 0.000581792 s versus 0.00016575 s for
`openmp-split`. The older `accelerate-vdsp` split-complex variant was also
slower than `openmp-split` at q16-q20. Treat these rows as evidence for keeping
the default `mklq-cpu` dense full-register probability fill on the OpenMP/scalar
path for now, not as a whole-backend performance claim.
