# MKL-Q CPU Sampling Counter Summary

This file is generated from bounded `.cpu-counter.json` reports under `benchmarks/mklq/reports/`.

Caveat: this is sampling phase counter evidence and probability-fill counter evidence from selected build-tree ctest cases. It is not release sign-off, not a benchmark result, and not cross-machine performance proof.

Aggregate counts are summed across tracked reports; repeated daily probes intentionally count the same selected tests once per report.

## Aggregate

| Field | Value |
| --- | --- |
| `status` | passed |
| `report_count` | 2 |
| `expected` | 14 |
| `selected` | 14 |
| `missing` | 0 |
| `passed` | 14 |
| `failed` | 0 |

## Evidence Boundary

| Boundary | Value |
| --- | --- |
| `runtime_counter_evidence` | True |
| `sampling_phase_counter_evidence` | True |
| `probability_fill_counter_evidence` | True |
| `release_signoff` | False |
| `performance_benchmark` | False |
| `cross_machine_performance_proof` | False |
| `raw_logs_truncated` | True |

## Counter Coverage Categories

| Category | Passed | Failed | Other | Description |
| --- | ---: | ---: | ---: | --- |
| counts_only_full_register | 2 | 0 | 0 | Counts-only full-register sampling phase counter tests |
| counts_only_partial_register | 2 | 0 | 0 | Counts-only partial-register sampling phase counter tests |
| probability_fill | 4 | 0 | 0 | Full-register and marginal probability-fill counter tests |
| sequential_full_register | 2 | 0 | 0 | Sequential full-register sampling phase counter tests |
| sparse_full_register_scan_hit | 2 | 0 | 0 | Sparse full-register scan hit phase counter tests |
| sparse_full_register_scan_miss | 2 | 0 | 0 | Sparse full-register scan miss and fallback phase counter tests |

## Counter Tests

| Category | Test |
| --- | --- |
| counts_only_full_register | `mklq_cpu_MKLQCpuTester.CountsOnlyFullRegisterSamplingReportsNativePhases` |
| counts_only_partial_register | `mklq_cpu_MKLQCpuTester.CountsOnlyPartialRegisterSamplingReportsNativePhases` |
| probability_fill | `mklq_cpu_MKLQCpuTester.FullRegisterProbabilityFillReportsNativeCounter` |
| probability_fill | `mklq_cpu_MKLQCpuTester.MarginalProbabilityFillReportsNativeCounter` |
| sequential_full_register | `mklq_cpu_MKLQCpuTester.SequentialFullRegisterSamplingReportsNativePhases` |
| sparse_full_register_scan_hit | `mklq_cpu_MKLQCpuTester.SparseFullRegisterScanHitReportsNativePhases` |
| sparse_full_register_scan_miss | `mklq_cpu_MKLQCpuTester.SparseFullRegisterScanMissReportsNativePhases` |

## Reports

| Report | Created | Status | Expected | Selected | Missing | Passed | Failed |
| --- | --- | --- | ---: | ---: | ---: | ---: | ---: |
| benchmarks/mklq/reports/local-cpu-sampling-counter-probe-2026-06-23.cpu-counter.json | 2026-06-23T07:38:00.016818+00:00 | passed | 7 | 7 | 0 | 7 | 0 |
| benchmarks/mklq/reports/local-cpu-sampling-counter-probe-2026-06-24.cpu-counter.json | 2026-06-24T04:48:29.028396+00:00 | passed | 7 | 7 | 0 | 7 | 0 |

Regenerate with:

```bash
python3 benchmarks/mklq/summarize_cpu_sampling_counters.py \
  --reports benchmarks/mklq/reports \
  --output docs/mklq/cpu-sampling-counters.md
```
