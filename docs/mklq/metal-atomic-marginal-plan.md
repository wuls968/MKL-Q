# Metal Atomic Marginal Sampling Implementation Plan

> **For maintainers:** implement each task with a red-green test cycle and
> review the diff before merge.

**Goal:** Add an optional Metal atomic marginal-probability route for large
resident partial-register sampling while preserving the current fallback.

**Architecture:** The existing core Metal library and reduction kernel remain
required. `MklqMetalRuntime.mm` compiles a second, independent atomic-kernel
library; if this pipeline is unavailable, `fillMarginalProbabilities` retains
its current marginal/full-register fallback behavior.

**Tech stack:** C++20, Objective-C++, Metal, CUDA-Q gtest, Python benchmarks.

## Global Constraints

- Keep `mklq-metal` experimental and preserve the fp64 CPU oracle.
- Do not make atomic floating-point support part of backend availability.
- Do not commit raw local benchmark payloads under `benchmarks/mklq/results/`.
- Treat new timings as local Apple Silicon evidence only.

### Task 1: Route-selection regression tests

**Files:**
- Modify: `unittests/nvqpp/backends/MKLQMetalTester.cpp`
- Modify: `runtime/nvqir/mklq/MklqCpuCircuitSimulator.cpp`

- [x] Add a q20 unordered partial-register fixture with a non-uniform state.
  The Metal runtime path must report one atomic marginal application and zero
  full-register probability fills when the optional pipeline is available.
- [x] Run the focused test before implementation. It must fail to compile
  because the atomic counter accessor does not exist.
- [x] Add simulator forwarding for
  `atomicMarginalProbabilityApplicationsForTest()` and use it in the fixture.

### Task 2: Optional runtime pipeline

**Files:**
- Modify: `runtime/nvqir/mklq/MklqMetalRuntime.h`
- Modify: `runtime/nvqir/mklq/MklqMetalRuntime.mm`
- Modify: `runtime/nvqir/mklq/MklqMetalRuntimeFallback.cpp`

- [x] Add `fillResidentAtomicMarginalProbabilities(...)` and
  `atomicMarginalProbabilityApplications()` to the public runtime interface.
- [x] Compile `mklq_fill_atomic_marginal_probabilities` from a separate Metal
  source string. Store compilation failure as an optional diagnostic and leave
  `available()` unchanged.
- [x] Validate inputs, flush pending resident gates, clear a shared fp32 output
  buffer, dispatch one thread per amplitude, wait, and convert bins to fp64.
- [x] Try this route before the current cost-gated marginal route. Return to
  the existing full-register and host-fold path if it returns false.
- [x] Run the focused test again. It must pass on an atomic-capable device and
  still pass through the existing fallback on other platforms.

### Task 3: Correctness, evidence, and documentation

**Files:**
- Modify: `unittests/nvqpp/backends/MKLQMetalTester.cpp`
- Modify: `docs/mklq/metal-execution-boundary.md`
- Modify: `runtime/nvqir/mklq/README.md`

- [x] Compare the q20 unordered marginal distribution with the CPU oracle
  within the existing Metal fp32 tolerance.
- [x] Document optional-pipeline behavior and the unchanged sampling boundary.
- [x] Run the final Metal backend test target, MKL-Q Python smoke tests,
  q20/q22/q23/q24 route/profile evidence, and public health audits.
- [x] Request code review and resolve blocker and important findings.
- [ ] Commit and publish through a protected-branch PR.
