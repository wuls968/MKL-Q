# MKL-Q mklq-metal execution boundary

This page defines the public execution boundary for the experimental
`mklq-metal` target. It is a maintenance contract for MKL-Q docs, tests, and
benchmark evidence. It is not release sign-off or timing evidence.
This page is not proof that every operation stayed on Metal.

## Purpose

`mklq-metal` is a mixed Metal/CPU simulator target. Supported operations may
run against a resident Metal state, while unsupported, diagnostic, or
not-yet-profitable operations can synchronize to the MKL-Q fp64 CPU oracle.
That CPU-oracle fallback is intentional until resident coverage, correctness,
benchmark evidence, and telemetry justify stronger claims.

The target remains experimental and is not the default MKL-Q backend.

## Resident Metal State

Current counter evidence tracks these resident Metal state routes:

| Route | Evidence | Host boundary |
| --- | --- | --- |
| Single-target gates | `mklq_metal_MKLQMetalTester.*` counter tests and `run_metal_runtime_counter_probe.py` | No user-visible host readback is required for the covered gate update itself. |
| Controlled and multi-control single-target gates | Counter tests grouped under `resident_gate` in `docs/mklq/metal-runtime-counters.md` | Coverage is finite and test-selected. |
| Two-target and three-target updates | Counter tests grouped under `resident_gate` | This is not a broad arbitrary-unitary guarantee. |
| Full-register probability fill | Counter tests grouped under `probability_sampling` | The probability vector is host-visible output by design. |
| Marginal probability fill | Counter tests grouped under `probability_sampling` | Marginal output is host-visible by design. |
| Requested-order partial-register sampling | Counter tests grouped under `probability_sampling` | The selected route proves resident marginal-probability work before counts-only Metal sample-count accumulation or sequential host draw/count accumulation. |
| Deterministic sampling bypass | Counter tests grouped under `probability_sampling` | One-outcome sequential and counts-only distributions can materialize results directly after resident probability work; this is not a general on-device sampler. |
| Full-register and partial-register counts-only sample-count accumulation | Counter tests grouped under `probability_sampling` | Random draws are still host-generated, but selected counts-only paths can accumulate outcome counts with a Metal kernel after resident probability work. |
| Host-side sampling draw telemetry | Counter tests grouped under `probability_sampling` | Sequential draw batches are explicitly counted as host-side work after resident probability work. |
| Native sampling phase timing telemetry | Counter tests grouped under `probability_sampling` | Test-accessor timing accumulators separate probability fill, draw/count, and expectation-reduction phases for selected fixtures; this is not release timing evidence. |
| Measurement, collapse, and reset | Counter tests grouped under `measurement_reset` | Measurement results and sampled counts cross the host boundary. |

The counter summary is rendered by:

```bash
python3 benchmarks/mklq/summarize_metal_runtime_counters.py \
  --reports benchmarks/mklq/reports \
  --output docs/mklq/metal-runtime-counters.md
```

The docs guard is:

```bash
python3 benchmarks/mklq/check_metal_runtime_counter_docs.py
```

## Synchronization And CPU-oracle Fallback

The resident Metal path can synchronize back to the CPU oracle when an
operation is outside the supported Metal route, when a host-visible result is
required, or when an error boundary must restore a known-good state.

Tracked fallback and synchronization boundaries include:

| Boundary | Meaning | Evidence category |
| --- | --- | --- |
| Unsupported custom operations with four or more target qubits | The target synchronizes the resident state before running the CPU oracle path. | `fallback_boundary` |
| Reupload after unsupported fallback | A later supported operation can return to resident execution after the CPU oracle updates state. | `fallback_boundary` |
| Zero-shot expectation or explicit state readback | Host-visible state or expectation paths may force synchronization. | `synchronization_boundary` |
| Device selection and runtime availability | The target records whether a Metal runtime was detected before resident routes are exercised. | `runtime_device` |

A passing `mklq-metal` fixture therefore means the mixed path preserved CUDA-Q
behavior for that fixture within tolerance. It does not mean the whole circuit
was Metal-native.

## Sampling Boundary

Probability fills can be resident Metal work, but public sampling evidence does
not claim an end-to-end on-device sampler. Current counter evidence tracks
deterministic sequential and counts-only shortcuts that materialize a single
non-zero outcome directly after resident probability work. It also tracks a
selected full-register and partial-register counts-only paths where
host-generated random draws are counted by a Metal sample-count accumulation
kernel. Sequential stochastic sampling still records host-side draw/count
telemetry after resident probability work.
The tracked q20 and q22 shot-scaling summaries for stochastic full-register and
partial-register sampling are historical static boundary evidence checked by
`check_metal_sampling_boundary_evidence.py`; that guard still verifies the
summary-era host-side draw/count wording and rejects Metal RNG, GPU sampler, or
general on-device sampler claims. Runtime counter reports are the authoritative
evidence for the newer selected full-register and partial-register counts-only
Metal sample-count accumulation paths.

Selected build-tree fixtures also assert positive native test-accessor timing
accumulators for probability fill, draw/count, and expectation-reduction
phases. Those timers are useful for regression triage and phase attribution,
but they are not benchmark rows, release timing evidence, or cross-machine
performance claims.

## Error Boundary

The runtime counter probe also separates recoverable synchronization from
state-poisoning failures:

| Error boundary | Expected behavior |
| --- | --- |
| Resident gate failure | The simulator marks the resident state invalid and prevents silent reuse. |
| Measurement probability failure | The simulator can synchronize before measurement state mutation when safe. |
| Collapse or reset failure | The simulator marks the resident state invalid because state mutation may be partial. |

These cases are grouped under `error_boundary` in the Metal runtime counter
summary.

## Evidence Commands

Use these commands when changing the Metal runtime boundary:

```bash
python3 benchmarks/mklq/run_metal_runtime_counter_probe.py \
  --build-dir build-python \
  --output benchmarks/mklq/reports/local-metal-runtime-counter-probe-YYYY-MM-DD.counter.json
python3 benchmarks/mklq/summarize_metal_runtime_counters.py \
  --reports benchmarks/mklq/reports \
  --output docs/mklq/metal-runtime-counters.md
python3 benchmarks/mklq/check_metal_runtime_counter_docs.py
python3 benchmarks/mklq/check_metal_sampling_boundary_evidence.py
python3 benchmarks/mklq/run_correctness_gate.py \
  --install-prefix "${HOME}/.cudaq-mklq" \
  --build-dir build-python
```

For public metadata-only changes, also run:

```bash
python3 benchmarks/mklq/run_public_healthcheck.py
```

## Non-Claims

This boundary explicitly does not claim:

- release readiness;
- package, wheel, or installer readiness;
- timing performance or speedup;
- full CUDA-Q backend parity;
- full Metal-native execution;
- proof that all sampling work happens on the device;
- proof that every `mklq-metal` operation stays resident on Metal.

Keep this page and `docs/mklq/metal-runtime-counters.md` aligned whenever the
Metal counter-test surface changes.
