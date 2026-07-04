# MKL-Q Metal Runtime Counter Summary

This file is generated from bounded `.counter.json` reports under `benchmarks/mklq/reports/`.

Caveat: this is runtime counter evidence from selected build-tree ctest cases. It is not release sign-off, not a benchmark result, and not proof that every operation stayed on Metal.

Aggregate counts are summed across tracked reports; repeated daily probes intentionally count the same selected tests once per report.

## Aggregate

| Field | Value |
| --- | --- |
| `status` | passed |
| `report_count` | 4 |
| `expected` | 119 |
| `selected` | 119 |
| `missing` | 0 |
| `passed` | 119 |
| `failed` | 0 |

## Evidence Boundary

| Boundary | Value |
| --- | --- |
| `runtime_counter_evidence` | True |
| `release_signoff` | False |
| `all_metal_execution_proof` | False |
| `raw_logs_truncated` | True |

## Counter Coverage Categories

| Category | Passed | Failed | Other | Description |
| --- | ---: | ---: | ---: | --- |
| error_boundary | 12 | 0 | 0 | Resident Metal error and poisoned-state boundary tests |
| fallback_boundary | 4 | 0 | 0 | Unsupported-gate fallback and reupload boundary tests |
| measurement_reset | 12 | 0 | 0 | Measurement, collapse, and reset counter tests |
| probability_sampling | 31 | 0 | 0 | Resident probability fill and sampling counter tests |
| resident_gate | 54 | 0 | 0 | Resident Metal gate/update counter tests |
| runtime_device | 2 | 0 | 0 | Runtime/device boundary counter tests |
| synchronization_boundary | 4 | 0 | 0 | Resident state synchronization boundary tests |

## Counter Tests

| Category | Test |
| --- | --- |
| error_boundary | `mklq_metal_MKLQMetalTester.SimulatorPoisonsResidentStateWhenSingleGateFails` |
| error_boundary | `mklq_metal_MKLQMetalTester.SimulatorPoisonsResidentStateWhenThreeGateFails` |
| error_boundary | `mklq_metal_MKLQMetalTester.SimulatorPoisonsResidentStateWhenTwoGateFails` |
| error_boundary | `mklq_metal_MKLQMetalTester.SimulatorThrowsWhenResidentMeasurementCollapseFails` |
| error_boundary | `mklq_metal_MKLQMetalTester.SimulatorThrowsWhenResidentMeasurementProbabilityFails` |
| error_boundary | `mklq_metal_MKLQMetalTester.SimulatorThrowsWhenResidentResetGateFails` |
| fallback_boundary | `mklq_metal_MKLQMetalTester.SimulatorReuploadsResidentStateAfterUnsupportedGateFallback` |
| measurement_reset | `mklq_metal_MKLQMetalTester.MetalRuntimeComputesAndCollapsesResidentQubitProbability` |
| measurement_reset | `mklq_metal_MKLQMetalTester.SimulatorMeasuresAndResetsResidentStateWithoutReadback` |
| measurement_reset | `mklq_metal_MKLQMetalTester.SimulatorResetsResidentNonzeroTargetWithoutReadback` |
| probability_sampling | `mklq_metal_MKLQMetalTester.MetalRuntimeFillsFullRegisterProbabilities` |
| probability_sampling | `mklq_metal_MKLQMetalTester.MetalRuntimeFillsResidentMarginalProbabilities` |
| probability_sampling | `mklq_metal_MKLQMetalTester.MetalRuntimeFillsResidentProbabilitiesWithoutStateReadback` |
| probability_sampling | `mklq_metal_MKLQMetalTester.MetalRuntimeProbabilityFillMatchesCpuNorms` |
| probability_sampling | `mklq_metal_MKLQMetalTester.SimulatorSamplesDenseFullRegisterThroughMetalProbabilityFill` |
| probability_sampling | `mklq_metal_MKLQMetalTester.SimulatorSamplesDeterministicSparseStateWithOneBitStringConversion` |
| probability_sampling | `mklq_metal_MKLQMetalTester.SimulatorSamplesLargeResidentPartialRegisterThroughFullProbability` |
| probability_sampling | `mklq_metal_MKLQMetalTester.SimulatorSamplesRequestedOrderPartialRegisterThroughMarginalProbability` |
| probability_sampling | `mklq_metal_MKLQMetalTester.SimulatorSamplesResidentDenseStateWithoutReadback` |
| probability_sampling | `mklq_metal_MKLQMetalTester.SimulatorSamplesSmallResidentPartialRegisterThroughMarginalProbability` |
| probability_sampling | `mklq_metal_MKLQMetalTester.SimulatorUsesMetalFullRegisterProbabilityFill` |
| resident_gate | `mklq_metal_MKLQMetalTester.MetalRuntimeAppliesControlledSingleQubitGate` |
| resident_gate | `mklq_metal_MKLQMetalTester.MetalRuntimeAppliesControlledTwoQubitGate` |
| resident_gate | `mklq_metal_MKLQMetalTester.MetalRuntimeAppliesResidentThreeQubitGate` |
| resident_gate | `mklq_metal_MKLQMetalTester.MetalRuntimeAppliesSingleQubitGate` |
| resident_gate | `mklq_metal_MKLQMetalTester.MetalRuntimeAppliesTwoQubitGate` |
| resident_gate | `mklq_metal_MKLQMetalTester.MetalRuntimeKeepsResidentStateAcrossGateSequence` |
| resident_gate | `mklq_metal_MKLQMetalTester.MetalRuntimeKeepsResidentYAndControlledYSequence` |
| resident_gate | `mklq_metal_MKLQMetalTester.SimulatorKeepsBuiltInPhaseFamilyResidentUntilReadback` |
| resident_gate | `mklq_metal_MKLQMetalTester.SimulatorKeepsBuiltInRxAndControlledRxResidentUntilReadback` |
| resident_gate | `mklq_metal_MKLQMetalTester.SimulatorKeepsBuiltInRyAndControlledRyResidentUntilReadback` |
| resident_gate | `mklq_metal_MKLQMetalTester.SimulatorKeepsBuiltInRzAndControlledRzResidentUntilReadback` |
| resident_gate | `mklq_metal_MKLQMetalTester.SimulatorKeepsBuiltInYAndControlledYResidentUntilReadback` |
| resident_gate | `mklq_metal_MKLQMetalTester.SimulatorKeepsMultiControlSingleQubitResidentUntilReadback` |
| resident_gate | `mklq_metal_MKLQMetalTester.SimulatorKeepsSupportedGateSequenceResidentUntilReadback` |
| resident_gate | `mklq_metal_MKLQMetalTester.SimulatorKeepsThreeQubitGateResidentUntilReadback` |
| resident_gate | `mklq_metal_MKLQMetalTester.SimulatorKeepsYAndControlledYResidentUntilReadback` |
| runtime_device | `mklq_metal_MKLQMetalTester.MetalRuntimeRejectsTargetsOutsideStateRange` |
| synchronization_boundary | `mklq_metal_MKLQMetalTester.SimulatorSynchronizesResidentStateBeforeUnsupportedGate` |
| synchronization_boundary | `mklq_metal_MKLQMetalTester.SimulatorSynchronizesResidentStateBeforeZeroShotExpectation` |

## Reports

| Report | Created | Status | Expected | Selected | Missing | Passed | Failed |
| --- | --- | --- | ---: | ---: | ---: | ---: | ---: |
| benchmarks/mklq/reports/local-metal-runtime-counter-probe-2026-06-23.counter.json | 2026-06-23T07:53:35.315481+00:00 | passed | 20 | 20 | 0 | 20 | 0 |
| benchmarks/mklq/reports/local-metal-runtime-counter-probe-2026-06-24.counter.json | 2026-06-24T04:49:20.683974+00:00 | passed | 20 | 20 | 0 | 20 | 0 |
| benchmarks/mklq/reports/local-metal-runtime-counter-probe-2026-07-03.counter.json | 2026-07-03T12:17:01.069186+00:00 | passed | 39 | 39 | 0 | 39 | 0 |
| benchmarks/mklq/reports/local-metal-runtime-counter-probe-2026-07-04.counter.json | 2026-07-04T07:22:11.941840+00:00 | passed | 40 | 40 | 0 | 40 | 0 |

Regenerate with:

```bash
python3 benchmarks/mklq/summarize_metal_runtime_counters.py \
  --reports benchmarks/mklq/reports \
  --output docs/mklq/metal-runtime-counters.md
```
