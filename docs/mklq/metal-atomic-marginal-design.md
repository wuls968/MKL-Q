# Metal Atomic Marginal Sampling Design

## Goal

Allow the experimental `mklq-metal` target to compute selected large resident
partial-register probability distributions without allocating the legacy
outcome-by-threadgroup partial-sum buffer. The result remains host-visible for
the existing sampling paths.

## Scope and Constraints

- The initial route applies only to a resident Metal state, a valid
  power-of-two marginal output buffer with at least 1024 bins, and q20-q23.
  This bounded Apple M5 policy follows the measured local crossover; widening
  it requires fresh evidence.
- The atomic kernel is compiled from an independent optional Metal library.
  A compiler or pipeline failure must not make `mklq-metal` unavailable.
- The resident state is read-only. A dispatch, allocation, or optional-pipeline
  failure must fall back to the existing resident full-register probability
  fill and host fold path.
- The output uses fp32 atomic accumulation, then converts to the existing fp64
  host output buffer. Correctness tests compare against the CPU oracle using a
  tolerance suitable for the fp32 Metal state contract.
- The target remains experimental and this change does not claim a fully
  Metal-native sampler or a release-performance guarantee.

## Design

The core Metal library retains the current marginal reduction kernel. A second
small library contains `mklq_fill_atomic_marginal_probabilities`; each state
amplitude maps its measured bits to an output outcome and performs one relaxed
`atomic_float` add of its squared magnitude. The host clears the shared output
buffer before dispatch and converts the completed fp32 bins to the caller's
fp64 buffer.

`MetalStateVectorExecutor` exposes the atomic route through a separate method
and tracks successful applications. The simulator tries that method before the
legacy cost-gated marginal kernel and its full-register fallback. This keeps
older Apple GPUs and drivers on the already-tested path when atomic floating
point support is unavailable.

## Validation

1. Add a q20 partial-register sampling regression fixture that is red before
   implementation: when the optional pipeline is usable, it must record one
   atomic marginal application and no full-register probability fill.
2. Compare an unordered measured-qubit distribution with the CPU simulator
   oracle, including a non-uniform state.
3. Keep existing small-state marginal and fallback fixtures green.
4. Run the Metal backend test target, the MKL-Q correctness gates, installed
   Python smoke tests, and public audits.
5. Measure q20, q22, and q24 sampling-phase profiles with 12 measured qubits
   and 65,536 shots. Retain only evidence that improves the current local
   scratch-reuse baseline and label it host-specific.

## Non-Goals

- Replacing the existing small-state marginal reduction kernel.
- Changing CUDA-Q namespaces, target defaults, package publication policy, or
  the CPU oracle fallback contract.
- Treating optional atomic-pipeline availability as a required hardware
  capability.
