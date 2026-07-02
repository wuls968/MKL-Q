# MKL-Q CPU Gate Counter Summary

This file is generated from bounded `.cpu-gate-counter.json` reports under `benchmarks/mklq/reports/`.

Caveat: this is gate fast-path counter evidence from selected build-tree ctest cases. It is not release sign-off, not a benchmark result, and not cross-machine performance proof.

## Aggregate Status

| Field | Value |
| --- | --- |
| `status` | passed |
| `report_count` | 2 |
| `expected` | 22 |
| `selected` | 22 |
| `missing` | 0 |
| `passed` | 22 |
| `failed` | 0 |
| `runtime_counter_evidence` | True |
| `gate_fast_path_counter_evidence` | True |
| `single_control_rz_phase_counter_evidence` | True |
| `release_signoff` | False |
| `performance_benchmark` | False |
| `cross_machine_performance_proof` | False |
| `raw_logs_truncated` | True |

Aggregate counts are summed across tracked reports. Repeated daily probes intentionally count the same selected counter tests once per report.

## Categories

| Category | Passed | Failed | Unknown | Description |
| --- | ---: | ---: | ---: | --- |
| composite_fast_path | 2 | 0 | 0 | Composite circuit fast-path selection counter tests |
| controlled_single_qubit_fast_path | 6 | 0 | 0 | Controlled built-in single-qubit gate fast-path counter tests |
| multi_control_boundary | 2 | 0 | 0 | Multi-control generic specialized-path boundary tests |
| phase_fast_path | 2 | 0 | 0 | Phase-sign gate fast-path counter tests |
| single_control_rz_phase | 2 | 0 | 0 | Single-control Rz direct phase fast-path counter tests |
| single_qubit_fast_path | 4 | 0 | 0 | Uncontrolled built-in single-qubit gate fast-path counter tests |
| three_qubit_fast_path | 2 | 0 | 0 | Three-qubit row-sparse fast-path counter tests |
| two_qubit_fast_path | 2 | 0 | 0 | Two-qubit gate fast-path counter tests |

## Selected Tests

| Category | Test |
| --- | --- |
| composite_fast_path | `mklq_cpu_MKLQCpuTester.HardwareEfficientAnsatzCompositeUsesDedicatedFastPaths` |
| controlled_single_qubit_fast_path | `mklq_cpu_MKLQCpuTester.CnotFastPathAppliesControlledXGate` |
| controlled_single_qubit_fast_path | `mklq_cpu_MKLQCpuTester.ControlledBuiltInSingleQubitFastPathsMatchMatrices` |
| controlled_single_qubit_fast_path | `mklq_cpu_MKLQCpuTester.SingleControlBuiltInSingleQubitGatesUseDedicatedFastPath` |
| multi_control_boundary | `mklq_cpu_MKLQCpuTester.MultiControlBuiltInSingleQubitGatesKeepGenericSpecializedPath` |
| phase_fast_path | `mklq_cpu_MKLQCpuTester.CzFastPathAppliesControlledZGate` |
| single_control_rz_phase | `mklq_cpu_MKLQCpuTester.SingleControlRzUsesDedicatedPhaseFastPath` |
| single_qubit_fast_path | `mklq_cpu_MKLQCpuTester.BuiltInSingleQubitFastPathsMatchMatrices` |
| single_qubit_fast_path | `mklq_cpu_MKLQCpuTester.XFastPathAppliesUncontrolledSingleQubitGate` |
| three_qubit_fast_path | `mklq_cpu_MKLQCpuTester.RowSparseThreeQubitCustomOperationUsesDedicatedFastPath` |
| two_qubit_fast_path | `mklq_cpu_MKLQCpuTester.SwapFastPathAppliesUncontrolledTwoQubitGate` |

## Reports

| Report | Created | Status | Expected | Selected | Missing | Passed | Failed |
| --- | --- | --- | ---: | ---: | ---: | ---: | ---: |
| benchmarks/mklq/reports/local-cpu-gate-counter-probe-2026-07-01.cpu-gate-counter.json | 2026-07-01T12:43:23.665224+00:00 | passed | 11 | 11 | 0 | 11 | 0 |
| benchmarks/mklq/reports/local-cpu-gate-counter-probe-2026-07-02.cpu-gate-counter.json | 2026-07-02T08:01:23.418220+00:00 | passed | 11 | 11 | 0 | 11 | 0 |

## Regenerate

```bash
python3 benchmarks/mklq/summarize_cpu_gate_counters.py \
  --reports benchmarks/mklq/reports \
  --output docs/mklq/cpu-gate-counters.md
```
