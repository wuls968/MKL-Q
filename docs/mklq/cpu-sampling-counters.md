# MKL-Q CPU Sampling Counter Summary

This file is generated from bounded `.cpu-counter.json` reports under `benchmarks/mklq/reports/`.

Caveat: this is sampling phase counter evidence from selected build-tree ctest cases. It is not release sign-off, not a benchmark result, and not cross-machine performance proof.

## Aggregate

| Field | Value |
| --- | --- |
| `status` | passed |
| `report_count` | 1 |
| `expected` | 3 |
| `selected` | 3 |
| `missing` | 0 |
| `passed` | 3 |
| `failed` | 0 |

## Evidence Boundary

| Boundary | Value |
| --- | --- |
| `runtime_counter_evidence` | True |
| `sampling_phase_counter_evidence` | True |
| `release_signoff` | False |
| `performance_benchmark` | False |
| `cross_machine_performance_proof` | False |
| `raw_logs_truncated` | True |

## Counter Coverage Categories

| Category | Passed | Failed | Other | Description |
| --- | ---: | ---: | ---: | --- |
| counts_only_full_register | 1 | 0 | 0 | Counts-only full-register sampling phase counter tests |
| counts_only_partial_register | 1 | 0 | 0 | Counts-only partial-register sampling phase counter tests |
| sequential_full_register | 1 | 0 | 0 | Sequential full-register sampling phase counter tests |

## Counter Tests

| Category | Test |
| --- | --- |
| counts_only_full_register | `mklq_cpu_MKLQCpuTester.CountsOnlyFullRegisterSamplingReportsNativePhases` |
| counts_only_partial_register | `mklq_cpu_MKLQCpuTester.CountsOnlyPartialRegisterSamplingReportsNativePhases` |
| sequential_full_register | `mklq_cpu_MKLQCpuTester.SequentialFullRegisterSamplingReportsNativePhases` |

## Reports

| Report | Created | Status | Expected | Selected | Missing | Passed | Failed |
| --- | --- | --- | ---: | ---: | ---: | ---: | ---: |
| benchmarks/mklq/reports/local-cpu-sampling-counter-probe-2026-06-23.cpu-counter.json | 2026-06-23T03:10:32.069719+00:00 | passed | 3 | 3 | 0 | 3 | 0 |

Regenerate with:

```bash
python3 benchmarks/mklq/summarize_cpu_sampling_counters.py \
  --reports benchmarks/mklq/reports \
  --output docs/mklq/cpu-sampling-counters.md
```
