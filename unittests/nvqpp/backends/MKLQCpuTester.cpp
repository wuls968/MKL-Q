/*******************************************************************************
 * Copyright (c) 2022 - 2026 NVIDIA Corporation & Affiliates.                  *
 * All rights reserved.                                                        *
 *                                                                             *
 * This source code and the accompanying materials are made available under    *
 * the terms of the Apache License 2.0 which accompanies this distribution.    *
 ******************************************************************************/

#include "MklqCpuCircuitSimulator.cpp"

#include "CUDAQTestUtils.h"
#include "common/ExecutionContext.h"
#include "cudaq/algorithms/sample/policy.h"

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <complex>
#include <functional>
#include <gtest/gtest.h>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

class MklqCpuCircuitSimulatorTester : public nvqir::MklqCpuCircuitSimulator {
public:
  cudaq::ExecutionResult sampleQubits(const std::vector<std::size_t> &qubits,
                                      int shots) {
    return sample(qubits, shots);
  }

  cudaq::ExecutionResult sampleQubitsWithoutSequentialDataForTest(
      const std::vector<std::size_t> &qubits, int shots) {
    return sample(qubits, shots, false);
  }

  cudaq::sample_result sampleFullRegisterViaSamplePolicyForTest(
      int shots, bool explicitMeasurements) {
    cudaq::ExecutionContext context("sample", shots);
    context.explicitMeasurements = explicitMeasurements;

    cudaq::sample_policy policy;
    policy.options.shots = shots;
    policy.options.explicit_measurements = explicitMeasurements;
    configureExecutionContext(policy);

    auto *outerContext = cudaq::getExecutionContext();
    cudaq::detail::setExecutionContext(&context);
    try {
      auto result = finalizeExecutionContext(policy);
      cudaq::detail::resetExecutionContext();
      if (outerContext)
        cudaq::detail::setExecutionContext(outerContext);
      return result;
    } catch (...) {
      cudaq::detail::resetExecutionContext();
      if (outerContext)
        cudaq::detail::setExecutionContext(outerContext);
      throw;
    }
  }

  cudaq::sample_result sampleNamedRegistersViaSamplePolicyForTest(
      int shots, bool explicitMeasurements,
      const std::vector<std::pair<std::size_t, std::string>> &measurements) {
    cudaq::ExecutionContext context("sample", shots);
    context.explicitMeasurements = explicitMeasurements;

    cudaq::sample_policy policy;
    policy.options.shots = shots;
    policy.options.explicit_measurements = explicitMeasurements;
    configureExecutionContext(policy);

    auto *outerContext = cudaq::getExecutionContext();
    cudaq::detail::setExecutionContext(&context);
    try {
      for (const auto &[qubit, registerName] : measurements)
        mz(qubit, registerName);
      auto result = finalizeExecutionContext(policy);
      cudaq::detail::resetExecutionContext();
      if (outerContext)
        cudaq::detail::setExecutionContext(outerContext);
      return result;
    } catch (...) {
      cudaq::detail::resetExecutionContext();
      if (outerContext)
        cudaq::detail::setExecutionContext(outerContext);
      throw;
    }
  }

  void setStateForTest(std::vector<std::complex<double>> data) {
    nQubitsAllocated = std::countr_zero(data.size());
    previousStateDimension = stateDimension;
    stateDimension = data.size();
    state = std::move(data);
  }

  bool sparseFullRegisterWouldHandleForTest() {
    cudaq::ExecutionResult counts;
    return trySampleSparseFullRegister(counts, 1, false);
  }

  std::vector<double> fullRegisterProbabilitiesForTest() {
    std::vector<double> probabilities(state.size(), 0.0);
    fillFullRegisterProbabilities(probabilities);
    return probabilities;
  }

  std::vector<double>
  marginalProbabilitiesForTest(const std::vector<std::size_t> &qubits) {
    std::vector<double> probabilities(1ULL << qubits.size(), 0.0);
    fillMarginalProbabilities(probabilities, qubits);
    return probabilities;
  }

  std::size_t bitStringConversionsForTest() const {
    return bitStringConversions;
  }

  std::size_t denseDrawCountBuffersForTest() const {
    return denseDrawCountBuffers;
  }

  std::size_t sparseDrawCountMapsForTest() const {
    return sparseDrawCountMaps;
  }

  std::size_t sortedSparseDrawCountMapsForTest() const {
    return sortedSparseDrawCountMaps;
  }

  cudaq::ExecutionResult drawSampleOutcomeCountsForTest(
      const std::vector<double> &probabilities, int shots,
      std::size_t bitCount) {
    cudaq::ExecutionResult counts;
    drawAndAppendSampleOutcomeCounts(counts, probabilities, shots, bitCount);
    return counts;
  }

  std::size_t fullRegisterProbabilityFillsForTest() const {
    return fullRegisterProbabilityFills;
  }

  std::size_t marginalProbabilityFillsForTest() const {
    return marginalProbabilityFills;
  }

  std::size_t sparseFullRegisterScansForTest() const {
    return sparseFullRegisterScans;
  }

  std::size_t sparseFullRegisterScanHitsForTest() const {
    return sparseFullRegisterScanHits;
  }

  std::size_t sparseFullRegisterScanMissesForTest() const {
    return sparseFullRegisterScanMisses;
  }

  std::size_t countsOnlySampleDrawBatchesForTest() const {
    return countsOnlySampleDrawBatches;
  }

  std::size_t sequentialSampleDrawBatchesForTest() const {
    return sequentialSampleDrawBatches;
  }

  std::size_t sampleExpectationReductionsForTest() const {
    return sampleExpectationReductions;
  }

  std::size_t countsOnlyNamedRegisterRemapsForTest() const {
    return countsOnlyNamedRegisterRemaps;
  }

  std::size_t sequentialNamedRegisterRemapsForTest() const {
    return sequentialNamedRegisterRemaps;
  }

  std::size_t bitFlipApplicationsForTest() const { return bitFlipApplications; }

  std::size_t phaseApplicationsForTest() const { return phaseApplications; }

  std::size_t specializedSingleQubitApplicationsForTest() const {
    return specializedSingleQubitApplications;
  }

  std::size_t specializedSingleControlQubitApplicationsForTest() const {
    return specializedSingleControlQubitApplications;
  }

  std::size_t accelerateProbabilityFillApplicationsForTest() const {
    return accelerateProbabilityFillApplications;
  }

  std::vector<std::complex<double>> stateVectorForTest() const { return state; }

  void applyTwoQubitGateForTest(
      const std::vector<std::complex<double>> &matrix,
      const std::vector<std::size_t> &controls,
      const std::vector<std::size_t> &targets,
      std::string_view operationName = "test_two_qubit") {
    applyTwoQubitGate(matrix, controls, targets, operationName,
                      /*isBuiltInOperation=*/false);
  }

  std::size_t twoQubitBlockApplicationsForTest() const {
    return twoQubitBlockApplications;
  }

  std::size_t twoQubitRowSparseApplicationsForTest() const {
    return twoQubitRowSparseApplications;
  }

  std::size_t swapApplicationsForTest() const { return swapApplications; }

  std::size_t threeQubitRowSparseApplicationsForTest() const {
    return threeQubitRowSparseApplications;
  }

  static std::size_t indexWithTwoZeroBitsForTest(std::size_t block,
                                                 std::size_t firstBit,
                                                 std::size_t secondBit) {
    return indexWithTwoZeroBits(block, firstBit, secondBit);
  }

  static std::size_t
  controlMaskForTest(const std::vector<std::size_t> &controls) {
    return controlMask(controls);
  }
};

static void expectRuntimeErrorContains(std::function<void()> action,
                                       std::string_view expected) {
  try {
    action();
    FAIL() << "expected std::runtime_error containing '" << expected << "'";
  } catch (const std::runtime_error &error) {
    EXPECT_NE(std::string(error.what()).find(expected), std::string::npos)
        << error.what();
  }
}

static void expectNear(std::complex<double> actual,
                       std::complex<double> expected) {
  EXPECT_NEAR(actual.real(), expected.real(), 1.0e-12);
  EXPECT_NEAR(actual.imag(), expected.imag(), 1.0e-12);
}

static std::vector<std::complex<double>>
applySingleQubitMatrixForTest(const std::vector<std::complex<double>> &input,
                              std::size_t target,
                              const std::array<std::complex<double>, 4> &matrix,
                              const std::vector<std::size_t> &controls = {}) {
  auto expected = input;
  const auto mask = 1ULL << target;
  const auto lowMask = mask - 1;
  const auto pairCount = input.size() >> 1;

  for (std::size_t pair = 0; pair < pairCount; ++pair) {
    const auto zeroIndex = ((pair & ~lowMask) << 1) | (pair & lowMask);
    if (!std::all_of(controls.begin(), controls.end(), [&](auto control) {
          return (zeroIndex & (1ULL << control)) != 0;
        }))
      continue;

    const auto oneIndex = zeroIndex | mask;
    const auto zeroAmplitude = input[zeroIndex];
    const auto oneAmplitude = input[oneIndex];
    expected[zeroIndex] = matrix[0] * zeroAmplitude + matrix[1] * oneAmplitude;
    expected[oneIndex] = matrix[2] * zeroAmplitude + matrix[3] * oneAmplitude;
  }

  return expected;
}

static std::vector<std::complex<double>>
applyTwoQubitMatrixForTest(const std::vector<std::complex<double>> &input,
                           const std::vector<std::size_t> &targets,
                           const std::vector<std::complex<double>> &matrix,
                           const std::vector<std::size_t> &controls = {}) {
  auto expected = input;
  std::vector<std::size_t> targetMasks;
  targetMasks.reserve(targets.size());
  for (auto target : targets)
    targetMasks.push_back(1ULL << target);

  const auto indexWithTargetBits = [&](std::size_t base,
                                       std::size_t targetBits) {
    auto result = base;
    for (std::size_t bit = 0; bit < targetMasks.size(); ++bit)
      if (targetBits & (1ULL << bit))
        result |= targetMasks[bit];
    return result;
  };

  const auto blockCount = input.size() >> targets.size();
  for (std::size_t block = 0; block < blockCount; ++block) {
    const auto base =
        MklqCpuCircuitSimulatorTester::indexWithTwoZeroBitsForTest(
            block, targets[0], targets[1]);

    if (!std::all_of(controls.begin(), controls.end(), [&](auto control) {
          return (base & (1ULL << control)) != 0;
        }))
      continue;

    const auto index0 = base;
    const auto index1 = indexWithTargetBits(base, 1);
    const auto index2 = indexWithTargetBits(base, 2);
    const auto index3 = indexWithTargetBits(base, 3);
    const std::array<std::complex<double>, 4> amplitudes{
        input[index0], input[index1], input[index2], input[index3]};

    expected[index0] = matrix[0] * amplitudes[0] + matrix[1] * amplitudes[1] +
                       matrix[2] * amplitudes[2] + matrix[3] * amplitudes[3];
    expected[index1] = matrix[4] * amplitudes[0] + matrix[5] * amplitudes[1] +
                       matrix[6] * amplitudes[2] + matrix[7] * amplitudes[3];
    expected[index2] = matrix[8] * amplitudes[0] + matrix[9] * amplitudes[1] +
                       matrix[10] * amplitudes[2] +
                       matrix[11] * amplitudes[3];
    expected[index3] = matrix[12] * amplitudes[0] +
                       matrix[13] * amplitudes[1] +
                       matrix[14] * amplitudes[2] +
                       matrix[15] * amplitudes[3];
  }

  return expected;
}

static void expectStateNear(const std::vector<std::complex<double>> &actual,
                            const std::vector<std::complex<double>> &expected) {
  ASSERT_EQ(actual.size(), expected.size());
  for (std::size_t index = 0; index < actual.size(); ++index)
    expectNear(actual[index], expected[index]);
}

static double
stateNormForTest(const std::vector<std::complex<double>> &state) {
  double norm = 0.0;
  for (const auto &amplitude : state)
    norm += std::norm(amplitude);
  return norm;
}

static std::array<std::complex<double>, 4> hMatrixForTest() {
  const auto invSqrt2 = 1.0 / std::sqrt(2.0);
  return {
      {{invSqrt2, 0.0}, {invSqrt2, 0.0}, {invSqrt2, 0.0}, {-invSqrt2, 0.0}}};
}

static std::array<std::complex<double>, 4> yMatrixForTest() {
  return {{{0.0, 0.0}, {0.0, -1.0}, {0.0, 1.0}, {0.0, 0.0}}};
}

static std::array<std::complex<double>, 4> xMatrixForTest() {
  return {{{0.0, 0.0}, {1.0, 0.0}, {1.0, 0.0}, {0.0, 0.0}}};
}

static std::array<std::complex<double>, 4> rxMatrixForTest(double angle) {
  const auto cosine = std::cos(angle / 2.0);
  const auto sine = std::sin(angle / 2.0);
  return {{{cosine, 0.0}, {0.0, -sine}, {0.0, -sine}, {cosine, 0.0}}};
}

static std::array<std::complex<double>, 4> ryMatrixForTest(double angle) {
  const auto cosine = std::cos(angle / 2.0);
  const auto sine = std::sin(angle / 2.0);
  return {{{cosine, 0.0}, {-sine, 0.0}, {sine, 0.0}, {cosine, 0.0}}};
}

static std::array<std::complex<double>, 4> rzMatrixForTest(double angle) {
  return {{{std::cos(-angle / 2.0), std::sin(-angle / 2.0)},
           {0.0, 0.0},
           {0.0, 0.0},
           {std::cos(angle / 2.0), std::sin(angle / 2.0)}}};
}

static std::array<std::complex<double>, 4> zMatrixForTest() {
  return {{{1.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {-1.0, 0.0}}};
}

static std::array<std::complex<double>, 4> sMatrixForTest() {
  return {{{1.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 1.0}}};
}

static std::array<std::complex<double>, 4> sdgMatrixForTest() {
  return {{{1.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, -1.0}}};
}

static std::array<std::complex<double>, 4> tMatrixForTest() {
  const auto invSqrt2 = 1.0 / std::sqrt(2.0);
  return {
      {{1.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {invSqrt2, invSqrt2}}};
}

static std::array<std::complex<double>, 4> tdgMatrixForTest() {
  const auto invSqrt2 = 1.0 / std::sqrt(2.0);
  return {
      {{1.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {invSqrt2, -invSqrt2}}};
}

CUDAQ_TEST(MKLQCpuTester, BackendUnitTestCompilesOpenMpBranchesWhenAvailable) {
#if defined(HAS_OPENMP) && !defined(_OPENMP)
  FAIL() << "HAS_OPENMP is defined, but _OPENMP is not; MKL-Q backend unit "
            "tests are not compiling the simulator OpenMP branches.";
#else
  SUCCEED();
#endif
}

CUDAQ_TEST(MKLQCpuTester, RejectsOutOfRangeMeasureResetAndSampleQubits) {
  MklqCpuCircuitSimulatorTester sim;
  auto q0 = sim.allocateQubit();
  ASSERT_EQ(q0, 0);

  expectRuntimeErrorContains([&] { (void)sim.mz(q0 + 1); },
                             "qubit index 1 out of range");
  expectRuntimeErrorContains([&] { sim.resetQubit(q0 + 1); },
                             "qubit index 1 out of range");
  expectRuntimeErrorContains([&] { (void)sim.sampleQubits({q0 + 1}, 1); },
                             "qubit index 1 out of range");
}

CUDAQ_TEST(MKLQCpuTester, RejectsOutOfRangeQueuedGateTarget) {
  MklqCpuCircuitSimulatorTester sim;
  auto q0 = sim.allocateQubit();
  ASSERT_EQ(q0, 0);

  sim.x({}, q0 + 1);
  expectRuntimeErrorContains([&] { (void)sim.mz(q0); },
                             "gate target qubit index 1 out of range");
}

CUDAQ_TEST(MKLQCpuTester, RejectsDuplicateQueuedGateQubits) {
  MklqCpuCircuitSimulatorTester sim;
  auto q0 = sim.allocateQubit();
  auto q1 = sim.allocateQubit();
  ASSERT_EQ(q0, 0);
  ASSERT_EQ(q1, 1);

  sim.x({q0}, q0);
  expectRuntimeErrorContains([&] { (void)sim.mz(q1); },
                             "duplicate target/control qubit");

  sim.x({q0, q0}, q1);
  expectRuntimeErrorContains([&] { (void)sim.mz(q0); },
                             "duplicate control qubit");
}

CUDAQ_TEST(MKLQCpuTester, RejectsDuplicateSampleQubits) {
  MklqCpuCircuitSimulatorTester sim;
  auto q0 = sim.allocateQubit();
  ASSERT_EQ(q0, 0);

  expectRuntimeErrorContains([&] { (void)sim.sampleQubits({q0, q0}, 1); },
                             "duplicate qubit");
}

CUDAQ_TEST(MKLQCpuTester, StateToHostRejectsNullOutputForNonZeroCopy) {
  nvqir::MklqCpuState state;
  state.state = {{1.0, 0.0}, {0.0, 0.0}};

  expectRuntimeErrorContains([&] { state.toHost(nullptr, 1); },
                             "null output buffer");
}

CUDAQ_TEST(MKLQCpuTester, SparseSamplingDoesNotDropTinyNonzeroOutcomes) {
  constexpr std::size_t qubitCount = 7;
  constexpr std::size_t dimension = 1ULL << qubitCount;
  constexpr double tinyProbability = 1.0e-16;
  constexpr double largeProbability = (1.0 - tinyProbability) / 64.0;

  std::vector<std::complex<double>> state(dimension, {0.0, 0.0});
  for (std::size_t index = 0; index < 64; ++index)
    state[index] = std::sqrt(largeProbability);
  state[64] = std::sqrt(tinyProbability);

  MklqCpuCircuitSimulatorTester sim;
  sim.setStateForTest(std::move(state));
  EXPECT_FALSE(sim.sparseFullRegisterWouldHandleForTest());
}

CUDAQ_TEST(MKLQCpuTester, SparseSamplingHandlesSixtyFourOutcomes) {
  constexpr std::size_t qubitCount = 7;
  constexpr std::size_t dimension = 1ULL << qubitCount;
  constexpr double probability = 1.0 / 64.0;

  std::vector<std::complex<double>> state(dimension, {0.0, 0.0});
  for (std::size_t index = 0; index < 64; ++index)
    state[index] = std::sqrt(probability);

  MklqCpuCircuitSimulatorTester sim;
  sim.setStateForTest(std::move(state));
  EXPECT_TRUE(sim.sparseFullRegisterWouldHandleForTest());
}

CUDAQ_TEST(MKLQCpuTester, DeterministicSparseSamplingConvertsBitStringOnce) {
  std::vector<std::complex<double>> state(8, {0.0, 0.0});
  state[5] = {1.0, 0.0};

  MklqCpuCircuitSimulatorTester sim;
  sim.setStateForTest(std::move(state));

  constexpr int shots = 32;
  const auto counts = sim.sampleQubits({0, 1, 2}, shots);

  ASSERT_EQ(counts.counts.size(), 1);
  ASSERT_TRUE(counts.counts.contains("101"));
  EXPECT_EQ(counts.counts.at("101"), shots);
  EXPECT_EQ(counts.sequentialData.size(), shots);
  EXPECT_EQ(sim.bitStringConversionsForTest(), 1);
}

CUDAQ_TEST(MKLQCpuTester, SparseFullRegisterScanHitReportsNativePhases) {
  std::vector<std::complex<double>> state(8, {0.0, 0.0});
  state[5] = {1.0, 0.0};

  MklqCpuCircuitSimulatorTester sim;
  sim.setStateForTest(std::move(state));

  constexpr int shots = 32;
  const auto counts =
      sim.sampleQubitsWithoutSequentialDataForTest({0, 1, 2}, shots);

  ASSERT_EQ(counts.counts.size(), 1);
  ASSERT_TRUE(counts.counts.contains("101"));
  EXPECT_EQ(counts.counts.at("101"), shots);
  EXPECT_TRUE(counts.sequentialData.empty());
  EXPECT_EQ(sim.sparseFullRegisterScansForTest(), 1);
  EXPECT_EQ(sim.sparseFullRegisterScanHitsForTest(), 1);
  EXPECT_EQ(sim.sparseFullRegisterScanMissesForTest(), 0);
  EXPECT_EQ(sim.fullRegisterProbabilityFillsForTest(), 0);
  EXPECT_EQ(sim.marginalProbabilityFillsForTest(), 0);
  EXPECT_EQ(sim.countsOnlySampleDrawBatchesForTest(), 0);
  EXPECT_EQ(sim.sequentialSampleDrawBatchesForTest(), 0);
  EXPECT_EQ(sim.sampleExpectationReductionsForTest(), 1);
  EXPECT_EQ(sim.denseDrawCountBuffersForTest(), 0);
  EXPECT_EQ(sim.sparseDrawCountMapsForTest(), 0);
  EXPECT_EQ(sim.sortedSparseDrawCountMapsForTest(), 0);
  EXPECT_EQ(sim.bitStringConversionsForTest(), 1);
}

CUDAQ_TEST(MKLQCpuTester,
           CountsOnlyDenseFullRegisterSamplingAggregatesBitStrings) {
  constexpr std::size_t qubitCount = 17;
  constexpr std::size_t dimension = 1ULL << qubitCount;
  constexpr int shots = 4096;
  constexpr std::size_t nonzeroOutcomes = 65;
  const double amplitude =
      1.0 / std::sqrt(static_cast<double>(nonzeroOutcomes));

  std::vector<std::complex<double>> state(dimension, {0.0, 0.0});
  for (std::size_t index = 0; index < nonzeroOutcomes; ++index)
    state[index] = {amplitude, 0.0};

  MklqCpuCircuitSimulatorTester sim;
  sim.setRandomSeed(13);
  sim.setStateForTest(std::move(state));

  std::vector<std::size_t> qubits;
  qubits.reserve(qubitCount);
  for (std::size_t qubit = 0; qubit < qubitCount; ++qubit)
    qubits.push_back(qubit);

  const auto counts =
      sim.sampleQubitsWithoutSequentialDataForTest(qubits, shots);

  std::size_t total = 0;
  for (const auto &[bits, count] : counts.counts)
    total += count;
  EXPECT_EQ(total, shots);
  EXPECT_TRUE(counts.sequentialData.empty());
  EXPECT_LE(sim.bitStringConversionsForTest(), nonzeroOutcomes);
  EXPECT_EQ(sim.denseDrawCountBuffersForTest(), 0);
  EXPECT_EQ(sim.sparseDrawCountMapsForTest(), 1);
  EXPECT_EQ(sim.sortedSparseDrawCountMapsForTest(), 1);
}

CUDAQ_TEST(MKLQCpuTester, SparseFullRegisterScanMissReportsNativePhases) {
  constexpr std::size_t qubitCount = 17;
  constexpr std::size_t dimension = 1ULL << qubitCount;
  constexpr int shots = 4096;
  constexpr std::size_t nonzeroOutcomes = 65;
  const double amplitude =
      1.0 / std::sqrt(static_cast<double>(nonzeroOutcomes));

  std::vector<std::complex<double>> state(dimension, {0.0, 0.0});
  for (std::size_t index = 0; index < nonzeroOutcomes; ++index)
    state[index] = {amplitude, 0.0};

  MklqCpuCircuitSimulatorTester sim;
  sim.setRandomSeed(13);
  sim.setStateForTest(std::move(state));

  std::vector<std::size_t> qubits;
  qubits.reserve(qubitCount);
  for (std::size_t qubit = 0; qubit < qubitCount; ++qubit)
    qubits.push_back(qubit);

  const auto counts =
      sim.sampleQubitsWithoutSequentialDataForTest(qubits, shots);

  std::size_t total = 0;
  for (const auto &[bits, count] : counts.counts)
    total += count;
  EXPECT_EQ(total, shots);
  EXPECT_TRUE(counts.sequentialData.empty());
  EXPECT_EQ(sim.sparseFullRegisterScansForTest(), 1);
  EXPECT_EQ(sim.sparseFullRegisterScanHitsForTest(), 0);
  EXPECT_EQ(sim.sparseFullRegisterScanMissesForTest(), 1);
  EXPECT_EQ(sim.fullRegisterProbabilityFillsForTest(), 1);
  EXPECT_EQ(sim.marginalProbabilityFillsForTest(), 0);
  EXPECT_EQ(sim.countsOnlySampleDrawBatchesForTest(), 1);
  EXPECT_EQ(sim.sequentialSampleDrawBatchesForTest(), 0);
  EXPECT_EQ(sim.sampleExpectationReductionsForTest(), 1);
  EXPECT_EQ(sim.denseDrawCountBuffersForTest(), 0);
  EXPECT_EQ(sim.sparseDrawCountMapsForTest(), 1);
  EXPECT_EQ(sim.sortedSparseDrawCountMapsForTest(), 1);
  EXPECT_LE(sim.bitStringConversionsForTest(), nonzeroOutcomes);
}

CUDAQ_TEST(MKLQCpuTester,
           CountsOnlyLargeSparseDrawUsesSortedSingleScanAggregation) {
  constexpr std::size_t qubitCount = 17;
  constexpr std::size_t outcomeCount = 1ULL << qubitCount;
  constexpr std::size_t hotOutcome = (1ULL << 16) + 7;
  constexpr int shots = 64;

  std::vector<double> probabilities(outcomeCount, 0.0);
  probabilities[hotOutcome] = 1.0;

  MklqCpuCircuitSimulatorTester sim;
  sim.setRandomSeed(23);
  const auto counts =
      sim.drawSampleOutcomeCountsForTest(probabilities, shots, qubitCount);

  std::string hotBits;
  hotBits.reserve(qubitCount);
  for (std::size_t bit = 0; bit < qubitCount; ++bit)
    hotBits.push_back((hotOutcome & (1ULL << bit)) ? '1' : '0');

  ASSERT_EQ(counts.counts.size(), 1);
  ASSERT_TRUE(counts.counts.contains(hotBits));
  EXPECT_EQ(counts.counts.at(hotBits), shots);
  EXPECT_TRUE(counts.sequentialData.empty());
  EXPECT_EQ(sim.denseDrawCountBuffersForTest(), 0);
  EXPECT_EQ(sim.sparseDrawCountMapsForTest(), 1);
  EXPECT_EQ(sim.sortedSparseDrawCountMapsForTest(), 1);
  EXPECT_EQ(sim.bitStringConversionsForTest(), 1);
}

CUDAQ_TEST(MKLQCpuTester,
           CountsOnlyFullRegisterSamplingReportsNativePhases) {
  constexpr std::size_t qubitCount = 17;
  constexpr std::size_t dimension = 1ULL << qubitCount;
  constexpr int shots = 4096;
  constexpr std::size_t nonzeroOutcomes = 65;
  const double amplitude =
      1.0 / std::sqrt(static_cast<double>(nonzeroOutcomes));

  std::vector<std::complex<double>> state(dimension, {0.0, 0.0});
  for (std::size_t index = 0; index < nonzeroOutcomes; ++index)
    state[index] = {amplitude, 0.0};

  MklqCpuCircuitSimulatorTester sim;
  sim.setRandomSeed(13);
  sim.setStateForTest(std::move(state));

  std::vector<std::size_t> qubits;
  qubits.reserve(qubitCount);
  for (std::size_t qubit = 0; qubit < qubitCount; ++qubit)
    qubits.push_back(qubit);

  const auto counts =
      sim.sampleQubitsWithoutSequentialDataForTest(qubits, shots);

  std::size_t total = 0;
  for (const auto &[bits, count] : counts.counts)
    total += count;
  EXPECT_EQ(total, shots);
  EXPECT_TRUE(counts.sequentialData.empty());
  EXPECT_EQ(sim.sparseFullRegisterScansForTest(), 1);
  EXPECT_EQ(sim.fullRegisterProbabilityFillsForTest(), 1);
  EXPECT_EQ(sim.marginalProbabilityFillsForTest(), 0);
  EXPECT_EQ(sim.countsOnlySampleDrawBatchesForTest(), 1);
  EXPECT_EQ(sim.sequentialSampleDrawBatchesForTest(), 0);
  EXPECT_EQ(sim.sampleExpectationReductionsForTest(), 1);
  EXPECT_EQ(sim.denseDrawCountBuffersForTest(), 0);
  EXPECT_EQ(sim.sparseDrawCountMapsForTest(), 1);
  EXPECT_EQ(sim.sortedSparseDrawCountMapsForTest(), 1);
}

CUDAQ_TEST(MKLQCpuTester,
           NonExplicitSamplePolicyUsesCountsOnlyFullRegisterSampling) {
  constexpr std::size_t qubitCount = 17;
  constexpr std::size_t dimension = 1ULL << qubitCount;
  constexpr int shots = 4096;
  constexpr std::size_t nonzeroOutcomes = 65;
  const double amplitude =
      1.0 / std::sqrt(static_cast<double>(nonzeroOutcomes));

  std::vector<std::complex<double>> state(dimension, {0.0, 0.0});
  for (std::size_t index = 0; index < nonzeroOutcomes; ++index)
    state[index] = {amplitude, 0.0};

  MklqCpuCircuitSimulatorTester sim;
  ASSERT_EQ(sim.allocateQubits(qubitCount).size(), qubitCount);
  sim.setRandomSeed(13);
  sim.setStateForTest(std::move(state));

  const auto result =
      sim.sampleFullRegisterViaSamplePolicyForTest(shots, false);

  std::size_t total = 0;
  for (const auto &[bits, count] : result)
    total += count;
  EXPECT_EQ(total, shots);
  EXPECT_EQ(result.sequential_data().size(), shots);
  EXPECT_LE(sim.bitStringConversionsForTest(), nonzeroOutcomes);
  EXPECT_EQ(sim.denseDrawCountBuffersForTest(), 0);
  EXPECT_EQ(sim.sparseDrawCountMapsForTest(), 1);
  EXPECT_EQ(sim.sortedSparseDrawCountMapsForTest(), 1);
}

CUDAQ_TEST(MKLQCpuTester,
           NonExplicitNamedSamplePolicyDoesNotRematerializeSequentialData) {
  constexpr std::size_t qubitCount = 17;
  constexpr std::size_t dimension = 1ULL << qubitCount;
  constexpr int shots = 4096;
  constexpr std::size_t nonzeroOutcomes = 65;
  const double amplitude =
      1.0 / std::sqrt(static_cast<double>(nonzeroOutcomes));

  std::vector<std::complex<double>> state(dimension, {0.0, 0.0});
  for (std::size_t index = 0; index < nonzeroOutcomes; ++index)
    state[index] = {amplitude, 0.0};

  MklqCpuCircuitSimulatorTester sim;
  ASSERT_EQ(sim.allocateQubits(qubitCount).size(), qubitCount);
  sim.setRandomSeed(13);
  sim.setStateForTest(std::move(state));

  std::vector<std::pair<std::size_t, std::string>> measurements;
  measurements.reserve(qubitCount);
  for (std::size_t qubit = 0; qubit < qubitCount; ++qubit)
    measurements.emplace_back(qubit, qubit == 0 ? "left" : "");

  const auto result =
      sim.sampleNamedRegistersViaSamplePolicyForTest(shots, false,
                                                     measurements);

  std::size_t globalTotal = 0;
  for (const auto &[bits, count] : result.to_map())
    globalTotal += count;
  std::size_t leftTotal = 0;
  for (const auto &[bits, count] : result.to_map("left"))
    leftTotal += count;

  EXPECT_EQ(globalTotal, shots);
  EXPECT_EQ(leftTotal, shots);
  EXPECT_EQ(result.sequential_data().size(), shots);
  EXPECT_EQ(result.sequential_data("left").size(), shots);
  EXPECT_LE(sim.bitStringConversionsForTest(), nonzeroOutcomes);
  EXPECT_EQ(sim.denseDrawCountBuffersForTest(), 0);
  EXPECT_EQ(sim.sparseDrawCountMapsForTest(), 1);
  EXPECT_EQ(sim.sortedSparseDrawCountMapsForTest(), 1);
  EXPECT_GT(sim.countsOnlyNamedRegisterRemapsForTest(), 0);
  EXPECT_EQ(sim.sequentialNamedRegisterRemapsForTest(), 0);
}

CUDAQ_TEST(MKLQCpuTester,
           CountsOnlyDensePartialRegisterSamplingAggregatesBitStrings) {
  constexpr std::size_t qubitCount = 7;
  constexpr std::size_t dimension = 1ULL << qubitCount;
  constexpr int shots = 4096;
  const double amplitude =
      1.0 / std::sqrt(static_cast<double>(dimension));

  std::vector<std::complex<double>> state(dimension, {amplitude, 0.0});

  MklqCpuCircuitSimulatorTester sim;
  sim.setRandomSeed(13);
  sim.setStateForTest(std::move(state));

  const auto counts =
      sim.sampleQubitsWithoutSequentialDataForTest({0, 2, 4}, shots);

  std::size_t total = 0;
  for (const auto &[bits, count] : counts.counts)
    total += count;
  EXPECT_EQ(total, shots);
  EXPECT_TRUE(counts.sequentialData.empty());
  EXPECT_LE(sim.bitStringConversionsForTest(), 8);
  EXPECT_EQ(sim.denseDrawCountBuffersForTest(), 1);
  EXPECT_EQ(sim.sparseDrawCountMapsForTest(), 0);
}

CUDAQ_TEST(MKLQCpuTester,
           CountsOnlyPartialRegisterSamplingReportsNativePhases) {
  constexpr std::size_t qubitCount = 7;
  constexpr std::size_t dimension = 1ULL << qubitCount;
  constexpr int shots = 4096;
  const double amplitude =
      1.0 / std::sqrt(static_cast<double>(dimension));

  std::vector<std::complex<double>> state(dimension, {amplitude, 0.0});

  MklqCpuCircuitSimulatorTester sim;
  sim.setRandomSeed(13);
  sim.setStateForTest(std::move(state));

  const auto counts =
      sim.sampleQubitsWithoutSequentialDataForTest({0, 2, 4}, shots);

  std::size_t total = 0;
  for (const auto &[bits, count] : counts.counts)
    total += count;
  EXPECT_EQ(total, shots);
  EXPECT_TRUE(counts.sequentialData.empty());
  EXPECT_EQ(sim.sparseFullRegisterScansForTest(), 0);
  EXPECT_EQ(sim.fullRegisterProbabilityFillsForTest(), 0);
  EXPECT_EQ(sim.marginalProbabilityFillsForTest(), 1);
  EXPECT_EQ(sim.countsOnlySampleDrawBatchesForTest(), 1);
  EXPECT_EQ(sim.sequentialSampleDrawBatchesForTest(), 0);
  EXPECT_EQ(sim.sampleExpectationReductionsForTest(), 1);
  EXPECT_EQ(sim.denseDrawCountBuffersForTest(), 1);
  EXPECT_EQ(sim.sparseDrawCountMapsForTest(), 0);
}

CUDAQ_TEST(MKLQCpuTester,
           SequentialFullRegisterSamplingReportsNativePhases) {
  constexpr std::size_t qubitCount = 7;
  constexpr std::size_t dimension = 1ULL << qubitCount;
  constexpr int shots = 512;
  constexpr std::size_t nonzeroOutcomes = 65;
  const double amplitude =
      1.0 / std::sqrt(static_cast<double>(nonzeroOutcomes));

  std::vector<std::complex<double>> state(dimension, {0.0, 0.0});
  for (std::size_t index = 0; index < nonzeroOutcomes; ++index)
    state[index] = {amplitude, 0.0};

  MklqCpuCircuitSimulatorTester sim;
  sim.setRandomSeed(17);
  sim.setStateForTest(std::move(state));

  std::vector<std::size_t> qubits;
  qubits.reserve(qubitCount);
  for (std::size_t qubit = 0; qubit < qubitCount; ++qubit)
    qubits.push_back(qubit);

  const auto counts = sim.sampleQubits(qubits, shots);

  std::size_t total = 0;
  for (const auto &[bits, count] : counts.counts)
    total += count;
  EXPECT_EQ(total, shots);
  EXPECT_EQ(counts.sequentialData.size(), shots);
  EXPECT_EQ(sim.sparseFullRegisterScansForTest(), 1);
  EXPECT_EQ(sim.fullRegisterProbabilityFillsForTest(), 1);
  EXPECT_EQ(sim.marginalProbabilityFillsForTest(), 0);
  EXPECT_EQ(sim.countsOnlySampleDrawBatchesForTest(), 0);
  EXPECT_EQ(sim.sequentialSampleDrawBatchesForTest(), 1);
  EXPECT_EQ(sim.sampleExpectationReductionsForTest(), 1);
  EXPECT_EQ(sim.denseDrawCountBuffersForTest(), 0);
  EXPECT_EQ(sim.sparseDrawCountMapsForTest(), 0);
}

CUDAQ_TEST(MKLQCpuTester,
           CountsOnlyDenseSamplingMatchesSequentialSamplingWithSameSeed) {
  constexpr std::size_t qubitCount = 7;
  constexpr std::size_t dimension = 1ULL << qubitCount;
  constexpr int shots = 512;
  constexpr int followUpShots = 128;
  constexpr std::size_t nonzeroOutcomes = 65;
  const double amplitude =
      1.0 / std::sqrt(static_cast<double>(nonzeroOutcomes));

  std::vector<std::complex<double>> state(dimension, {0.0, 0.0});
  for (std::size_t index = 0; index < nonzeroOutcomes; ++index)
    state[index] = {amplitude, 0.0};

  std::vector<std::size_t> qubits;
  qubits.reserve(qubitCount);
  for (std::size_t qubit = 0; qubit < qubitCount; ++qubit)
    qubits.push_back(qubit);

  MklqCpuCircuitSimulatorTester sequential;
  sequential.setRandomSeed(17);
  sequential.setStateForTest(state);

  MklqCpuCircuitSimulatorTester countsOnly;
  countsOnly.setRandomSeed(17);
  countsOnly.setStateForTest(std::move(state));

  const auto sequentialResult = sequential.sampleQubits(qubits, shots);
  const auto countsOnlyResult =
      countsOnly.sampleQubitsWithoutSequentialDataForTest(qubits, shots);

  EXPECT_EQ(sequentialResult.counts, countsOnlyResult.counts);
  ASSERT_TRUE(sequentialResult.expectationValue.has_value());
  ASSERT_TRUE(countsOnlyResult.expectationValue.has_value());
  EXPECT_DOUBLE_EQ(*sequentialResult.expectationValue,
                   *countsOnlyResult.expectationValue);
  EXPECT_EQ(sequentialResult.sequentialData.size(), shots);
  EXPECT_TRUE(countsOnlyResult.sequentialData.empty());

  const auto sequentialFollowUp =
      sequential.sampleQubitsWithoutSequentialDataForTest(qubits, followUpShots);
  const auto countsOnlyFollowUp =
      countsOnly.sampleQubitsWithoutSequentialDataForTest(qubits, followUpShots);

  EXPECT_EQ(sequentialFollowUp.counts, countsOnlyFollowUp.counts);
  ASSERT_TRUE(sequentialFollowUp.expectationValue.has_value());
  ASSERT_TRUE(countsOnlyFollowUp.expectationValue.has_value());
  EXPECT_DOUBLE_EQ(*sequentialFollowUp.expectationValue,
                   *countsOnlyFollowUp.expectationValue);
}

CUDAQ_TEST(MKLQCpuTester, DenseFullRegisterProbabilitiesMatchNorms) {
  MklqCpuCircuitSimulatorTester sim;
  sim.setStateForTest({{3.0, 4.0}, {0.5, -0.5}, {0.0, 0.0}, {-2.0, 1.5}});

  const auto probabilities = sim.fullRegisterProbabilitiesForTest();
  ASSERT_EQ(probabilities.size(), 4);
  EXPECT_DOUBLE_EQ(probabilities[0], 25.0);
  EXPECT_DOUBLE_EQ(probabilities[1], 0.5);
  EXPECT_DOUBLE_EQ(probabilities[2], 0.0);
  EXPECT_DOUBLE_EQ(probabilities[3], 6.25);
}

CUDAQ_TEST(MKLQCpuTester, DenseFullRegisterProbabilitiesLargeStateMatchNorms) {
  constexpr std::size_t dimension = 1ULL << 17;
  std::vector<std::complex<double>> state(dimension);
  for (std::size_t index = 0; index < dimension; ++index) {
    const auto real = static_cast<double>(static_cast<int>(index % 7) - 3);
    const auto imag = static_cast<double>(static_cast<int>(index % 5) - 2);
    state[index] = {real, imag};
  }

  MklqCpuCircuitSimulatorTester sim;
  sim.setStateForTest(std::move(state));
  const auto probabilities = sim.fullRegisterProbabilitiesForTest();

  ASSERT_EQ(probabilities.size(), dimension);
  double maxDiff = 0.0;
  for (std::size_t index = 0; index < dimension; ++index) {
    const auto real = static_cast<double>(static_cast<int>(index % 7) - 3);
    const auto imag = static_cast<double>(static_cast<int>(index % 5) - 2);
    const auto expected = real * real + imag * imag;
    maxDiff = std::max(maxDiff, std::abs(probabilities[index] - expected));
  }
  EXPECT_DOUBLE_EQ(maxDiff, 0.0);
}

CUDAQ_TEST(MKLQCpuTester, FullRegisterProbabilityFillReportsNativeCounter) {
  MklqCpuCircuitSimulatorTester sim;
  sim.setStateForTest({{3.0, 4.0}, {0.5, -0.5}, {0.0, 0.0}, {-2.0, 1.5}});

  EXPECT_EQ(sim.fullRegisterProbabilityFillsForTest(), 0);
  EXPECT_EQ(sim.marginalProbabilityFillsForTest(), 0);

  const auto probabilities = sim.fullRegisterProbabilitiesForTest();

  ASSERT_EQ(probabilities.size(), 4);
  EXPECT_DOUBLE_EQ(probabilities[0], 25.0);
  EXPECT_DOUBLE_EQ(probabilities[1], 0.5);
  EXPECT_DOUBLE_EQ(probabilities[2], 0.0);
  EXPECT_DOUBLE_EQ(probabilities[3], 6.25);
  EXPECT_EQ(sim.fullRegisterProbabilityFillsForTest(), 1);
  EXPECT_EQ(sim.marginalProbabilityFillsForTest(), 0);
}

CUDAQ_TEST(MKLQCpuTester,
           MarginalProbabilitiesPreserveRequestedSampleBitOrder) {
  constexpr std::size_t qubitCount = 6;
  constexpr std::size_t dimension = 1ULL << qubitCount;
  std::vector<std::complex<double>> state(dimension);
  std::array<double, 8> expected{};
  for (std::size_t basis = 0; basis < dimension; ++basis) {
    const auto amplitude = static_cast<double>(basis + 1);
    state[basis] = {amplitude, 0.0};
    const auto outcome = ((basis & (1ULL << 5)) ? 1ULL << 0 : 0) |
                         ((basis & (1ULL << 0)) ? 1ULL << 1 : 0) |
                         ((basis & (1ULL << 2)) ? 1ULL << 2 : 0);
    expected[outcome] += amplitude * amplitude;
  }

  MklqCpuCircuitSimulatorTester sim;
  sim.setStateForTest(std::move(state));
  const auto probabilities = sim.marginalProbabilitiesForTest({5, 0, 2});

  ASSERT_EQ(probabilities.size(), expected.size());
  for (std::size_t outcome = 0; outcome < expected.size(); ++outcome)
    EXPECT_DOUBLE_EQ(probabilities[outcome], expected[outcome]);
}

CUDAQ_TEST(MKLQCpuTester, MarginalProbabilityFillReportsNativeCounter) {
  constexpr std::size_t qubitCount = 3;
  constexpr std::size_t dimension = 1ULL << qubitCount;
  std::vector<std::complex<double>> state(dimension);
  std::array<double, 4> expected{};
  for (std::size_t basis = 0; basis < dimension; ++basis) {
    const auto amplitude = static_cast<double>(basis + 1);
    state[basis] = {amplitude, 0.0};
    const auto outcome = ((basis & (1ULL << 2)) ? 1ULL << 0 : 0) |
                         ((basis & (1ULL << 0)) ? 1ULL << 1 : 0);
    expected[outcome] += amplitude * amplitude;
  }

  MklqCpuCircuitSimulatorTester sim;
  sim.setStateForTest(std::move(state));

  EXPECT_EQ(sim.fullRegisterProbabilityFillsForTest(), 0);
  EXPECT_EQ(sim.marginalProbabilityFillsForTest(), 0);

  const auto probabilities = sim.marginalProbabilitiesForTest({2, 0});

  ASSERT_EQ(probabilities.size(), expected.size());
  for (std::size_t outcome = 0; outcome < expected.size(); ++outcome)
    EXPECT_DOUBLE_EQ(probabilities[outcome], expected[outcome]);
  EXPECT_EQ(sim.fullRegisterProbabilityFillsForTest(), 0);
  EXPECT_EQ(sim.marginalProbabilityFillsForTest(), 1);
}

CUDAQ_TEST(MKLQCpuTester,
           DenseFullRegisterProbabilitiesDoNotUseAccelerateByDefaultOnApple) {
#if defined(__APPLE__)
  constexpr std::size_t dimension = 1ULL << 17;
  auto expectedProbability = [](std::size_t index) {
    const auto real = static_cast<double>(static_cast<int>(index % 11) - 5);
    const auto imag = static_cast<double>(static_cast<int>(index % 13) - 6);
    return real * real + imag * imag;
  };

  std::vector<std::complex<double>> state(dimension);
  for (std::size_t index = 0; index < dimension; ++index) {
    const auto real = static_cast<double>(static_cast<int>(index % 11) - 5);
    const auto imag = static_cast<double>(static_cast<int>(index % 13) - 6);
    state[index] = {real, imag};
  }

  MklqCpuCircuitSimulatorTester sim;
  sim.setStateForTest(std::move(state));
  const auto probabilities = sim.fullRegisterProbabilitiesForTest();

  ASSERT_EQ(probabilities.size(), dimension);
  EXPECT_EQ(sim.accelerateProbabilityFillApplicationsForTest(), 0);
  EXPECT_DOUBLE_EQ(probabilities[0], expectedProbability(0));
  EXPECT_DOUBLE_EQ(probabilities[dimension - 1],
                   expectedProbability(dimension - 1));
#else
  GTEST_SKIP() << "Apple Accelerate is available only on Apple platforms.";
#endif
}

CUDAQ_TEST(MKLQCpuTester, XFastPathAppliesUncontrolledSingleQubitGate) {
  MklqCpuCircuitSimulatorTester sim;
  sim.setStateForTest({
      {1.0, 0.0},
      {2.0, -1.0},
      {0.5, 0.25},
      {-3.0, 0.5},
  });

  sim.x(1);
  sim.flushGateQueue();
  const auto state = sim.stateVectorForTest();

  ASSERT_EQ(state.size(), 4);
  expectNear(state[0], {0.5, 0.25});
  expectNear(state[1], {-3.0, 0.5});
  expectNear(state[2], {1.0, 0.0});
  expectNear(state[3], {2.0, -1.0});
  EXPECT_EQ(sim.bitFlipApplicationsForTest(), 1);
}

CUDAQ_TEST(MKLQCpuTester, CnotFastPathAppliesControlledXGate) {
  const std::vector<std::complex<double>> initial{
      {1.0, 0.0},
      {2.0, -1.0},
      {0.5, 0.25},
      {-3.0, 0.5},
      {0.25, -0.5},
      {1.5, 0.5},
      {-2.0, 1.0},
      {0.75, -1.25},
  };

  struct Case {
    std::string_view name;
    std::size_t control;
    std::size_t target;
  };

  for (const auto &testCase : std::vector<Case>{
           {"control below target", 0, 2},
           {"control above target", 2, 0},
       }) {
    SCOPED_TRACE(testCase.name);
    MklqCpuCircuitSimulatorTester sim;
    sim.setStateForTest(initial);

    sim.x({testCase.control}, testCase.target);
    sim.flushGateQueue();

    expectStateNear(
        sim.stateVectorForTest(),
        applySingleQubitMatrixForTest(initial, testCase.target,
                                      xMatrixForTest(), {testCase.control}));
    EXPECT_EQ(sim.bitFlipApplicationsForTest(), 1);
    EXPECT_EQ(sim.specializedSingleQubitApplicationsForTest(), 1);
    EXPECT_EQ(sim.specializedSingleControlQubitApplicationsForTest(), 1);
  }
}

CUDAQ_TEST(MKLQCpuTester, CustomOperationNamedXUsesGenericSingleQubitPath) {
  MklqCpuCircuitSimulatorTester sim;
  sim.setStateForTest({
      {1.0, 0.0},
      {2.0, -1.0},
      {0.5, 0.25},
      {-3.0, 0.5},
  });

  const std::vector<std::complex<double>> identity{
      {1.0, 0.0},
      {0.0, 0.0},
      {0.0, 0.0},
      {1.0, 0.0},
  };
  sim.applyCustomOperation(identity, {}, {1}, "x");
  sim.flushGateQueue();
  const auto state = sim.stateVectorForTest();

  ASSERT_EQ(state.size(), 4);
  expectNear(state[0], {1.0, 0.0});
  expectNear(state[1], {2.0, -1.0});
  expectNear(state[2], {0.5, 0.25});
  expectNear(state[3], {-3.0, 0.5});
  EXPECT_EQ(sim.bitFlipApplicationsForTest(), 0);
}

CUDAQ_TEST(MKLQCpuTester, BuiltInSingleQubitFastPathsMatchMatrices) {
  const std::vector<std::complex<double>> initial{
      {1.0, 0.0},
      {2.0, -1.0},
      {0.5, 0.25},
      {-3.0, 0.5},
  };
  constexpr double positiveAngle = 0.375;
  constexpr double negativeAngle = -0.625;

  struct Case {
    std::string_view name;
    std::size_t target;
    std::function<void(MklqCpuCircuitSimulatorTester &, std::size_t)> apply;
    std::array<std::complex<double>, 4> matrix;
  };

  const std::vector<Case> cases{
      {"h target 1", 1, [](auto &sim, auto target) { sim.h(target); },
       hMatrixForTest()},
      {"h target 0", 0, [](auto &sim, auto target) { sim.h(target); },
       hMatrixForTest()},
      {"y target 1", 1, [](auto &sim, auto target) { sim.y(target); },
       yMatrixForTest()},
      {"y target 0", 0, [](auto &sim, auto target) { sim.y(target); },
       yMatrixForTest()},
      {"rx positive target 1", 1,
       [&](auto &sim, auto target) { sim.rx(positiveAngle, target); },
       rxMatrixForTest(positiveAngle)},
      {"rx negative target 0", 0,
       [&](auto &sim, auto target) { sim.rx(negativeAngle, target); },
       rxMatrixForTest(negativeAngle)},
      {"ry positive target 1", 1,
       [&](auto &sim, auto target) { sim.ry(positiveAngle, target); },
       ryMatrixForTest(positiveAngle)},
      {"ry negative target 0", 0,
       [&](auto &sim, auto target) { sim.ry(negativeAngle, target); },
       ryMatrixForTest(negativeAngle)},
      {"rz positive target 1", 1,
       [&](auto &sim, auto target) { sim.rz(positiveAngle, target); },
       rzMatrixForTest(positiveAngle)},
      {"rz negative target 0", 0,
       [&](auto &sim, auto target) { sim.rz(negativeAngle, target); },
       rzMatrixForTest(negativeAngle)},
  };

  for (const auto &testCase : cases) {
    SCOPED_TRACE(testCase.name);
    MklqCpuCircuitSimulatorTester sim;
    sim.setStateForTest(initial);

    testCase.apply(sim, testCase.target);
    sim.flushGateQueue();

    expectStateNear(sim.stateVectorForTest(),
                    applySingleQubitMatrixForTest(initial, testCase.target,
                                                  testCase.matrix));
    EXPECT_EQ(sim.specializedSingleQubitApplicationsForTest(), 1);
  }
}

CUDAQ_TEST(MKLQCpuTester, ControlledBuiltInSingleQubitFastPathsMatchMatrices) {
  const std::vector<std::complex<double>> twoQubitInitial{
      {1.0, 0.0},
      {2.0, -1.0},
      {0.5, 0.25},
      {-3.0, 0.5},
  };
  const std::vector<std::complex<double>> threeQubitInitial{
      {1.0, 0.0},   {2.0, -1.0}, {0.5, 0.25}, {-3.0, 0.5},
      {0.25, -0.5}, {1.5, 0.5},  {-2.0, 1.0}, {0.75, -1.25},
  };
  constexpr double angle = 0.375;

  struct Case {
    std::string_view name;
    std::vector<std::complex<double>> initial;
    std::vector<std::size_t> controls;
    std::size_t target;
    std::function<void(MklqCpuCircuitSimulatorTester &,
                       const std::vector<std::size_t> &, std::size_t)>
        apply;
    std::array<std::complex<double>, 4> matrix;
  };

  const std::vector<Case> cases{
      {"h control 0 target 1",
       twoQubitInitial,
       {0},
       1,
       [](auto &sim, const auto &controls, auto target) {
         sim.h(controls, target);
       },
       hMatrixForTest()},
      {"h control 1 target 0",
       twoQubitInitial,
       {1},
       0,
       [](auto &sim, const auto &controls, auto target) {
         sim.h(controls, target);
       },
       hMatrixForTest()},
      {"h controls 0,1 target 2",
       threeQubitInitial,
       {0, 1},
       2,
      [](auto &sim, const auto &controls, auto target) {
         sim.h(controls, target);
       },
       hMatrixForTest()},
      {"y control 0 target 1",
       twoQubitInitial,
       {0},
       1,
       [](auto &sim, const auto &controls, auto target) {
         sim.y(controls, target);
       },
       yMatrixForTest()},
      {"y control 1 target 0",
       twoQubitInitial,
       {1},
       0,
       [](auto &sim, const auto &controls, auto target) {
         sim.y(controls, target);
       },
       yMatrixForTest()},
      {"y controls 0,1 target 2",
       threeQubitInitial,
       {0, 1},
       2,
       [](auto &sim, const auto &controls, auto target) {
         sim.y(controls, target);
       },
       yMatrixForTest()},
      {"rx control 0 target 1",
       twoQubitInitial,
       {0},
       1,
       [&](auto &sim, const auto &controls, auto target) {
         sim.rx(angle, controls, target);
       },
       rxMatrixForTest(angle)},
      {"rx control 1 target 0",
       twoQubitInitial,
       {1},
       0,
       [&](auto &sim, const auto &controls, auto target) {
         sim.rx(angle, controls, target);
       },
       rxMatrixForTest(angle)},
      {"rx controls 0,1 target 2",
       threeQubitInitial,
       {0, 1},
       2,
       [&](auto &sim, const auto &controls, auto target) {
         sim.rx(angle, controls, target);
       },
       rxMatrixForTest(angle)},
      {"ry control 0 target 1",
       twoQubitInitial,
       {0},
       1,
       [&](auto &sim, const auto &controls, auto target) {
         sim.ry(angle, controls, target);
       },
       ryMatrixForTest(angle)},
      {"ry control 1 target 0",
       twoQubitInitial,
       {1},
       0,
       [&](auto &sim, const auto &controls, auto target) {
         sim.ry(angle, controls, target);
       },
       ryMatrixForTest(angle)},
      {"ry controls 0,1 target 2",
       threeQubitInitial,
       {0, 1},
       2,
       [&](auto &sim, const auto &controls, auto target) {
         sim.ry(angle, controls, target);
       },
       ryMatrixForTest(angle)},
      {"rz control 0 target 1",
       twoQubitInitial,
       {0},
       1,
       [&](auto &sim, const auto &controls, auto target) {
         sim.rz(angle, controls, target);
       },
       rzMatrixForTest(angle)},
      {"rz control 1 target 0",
       twoQubitInitial,
       {1},
       0,
       [&](auto &sim, const auto &controls, auto target) {
         sim.rz(angle, controls, target);
       },
       rzMatrixForTest(angle)},
      {"rz controls 0,1 target 2",
       threeQubitInitial,
       {0, 1},
       2,
       [&](auto &sim, const auto &controls, auto target) {
         sim.rz(angle, controls, target);
       },
       rzMatrixForTest(angle)},
  };

  for (const auto &testCase : cases) {
    SCOPED_TRACE(testCase.name);
    MklqCpuCircuitSimulatorTester sim;
    sim.setStateForTest(testCase.initial);

    testCase.apply(sim, testCase.controls, testCase.target);
    sim.flushGateQueue();

    expectStateNear(
        sim.stateVectorForTest(),
        applySingleQubitMatrixForTest(testCase.initial, testCase.target,
                                      testCase.matrix, testCase.controls));
    EXPECT_EQ(sim.specializedSingleQubitApplicationsForTest(), 1);
  }
}

static void expectSingleControlBuiltInGateUsesDedicatedFastPath(
    std::string_view gateName,
    std::function<void(MklqCpuCircuitSimulatorTester &, std::size_t,
                       std::size_t)>
        apply,
    const std::array<std::complex<double>, 4> &matrix) {
  const std::vector<std::complex<double>> initial{
      {1.0, 0.0},   {2.0, -1.0}, {0.5, 0.25}, {-3.0, 0.5},
      {0.25, -0.5}, {1.5, 0.5},  {-2.0, 1.0}, {0.75, -1.25},
  };

  struct Case {
    std::string_view name;
    std::size_t control;
    std::size_t target;
  };

  for (const auto &testCase : std::vector<Case>{
           {"control below target", 0, 2},
           {"control above target", 2, 0},
       }) {
    SCOPED_TRACE(gateName);
    SCOPED_TRACE(testCase.name);
    MklqCpuCircuitSimulatorTester sim;
    sim.setStateForTest(initial);

    apply(sim, testCase.control, testCase.target);
    sim.flushGateQueue();

    expectStateNear(
        sim.stateVectorForTest(),
        applySingleQubitMatrixForTest(initial, testCase.target, matrix,
                                      {testCase.control}));
    EXPECT_EQ(sim.specializedSingleQubitApplicationsForTest(), 1);
    EXPECT_EQ(sim.specializedSingleControlQubitApplicationsForTest(), 1);
    EXPECT_EQ(sim.bitFlipApplicationsForTest(), 0);
    EXPECT_EQ(sim.phaseApplicationsForTest(), 0);
  }
}

CUDAQ_TEST(MKLQCpuTester,
           SingleControlBuiltInSingleQubitGatesUseDedicatedFastPath) {
  const std::vector<std::complex<double>> initial{
      {1.0, 0.0},   {2.0, -1.0}, {0.5, 0.25}, {-3.0, 0.5},
      {0.25, -0.5}, {1.5, 0.5},  {-2.0, 1.0}, {0.75, -1.25},
  };
  constexpr double angle = 0.375;

  struct Case {
    std::string_view name;
    std::function<void(MklqCpuCircuitSimulatorTester &)> apply;
    std::array<std::complex<double>, 4> matrix;
  };

  const std::vector<Case> cases{
      {"ch", [](auto &sim) { sim.h({0}, 2); }, hMatrixForTest()},
      {"cy", [](auto &sim) { sim.y({0}, 2); }, yMatrixForTest()},
      {"crx", [&](auto &sim) { sim.rx(angle, {0}, 2); },
       rxMatrixForTest(angle)},
      {"cry", [&](auto &sim) { sim.ry(angle, {0}, 2); },
       ryMatrixForTest(angle)},
      {"crz", [&](auto &sim) { sim.rz(angle, {0}, 2); },
       rzMatrixForTest(angle)},
  };

  for (const auto &testCase : cases) {
    SCOPED_TRACE(testCase.name);
    MklqCpuCircuitSimulatorTester sim;
    sim.setStateForTest(initial);

    testCase.apply(sim);
    sim.flushGateQueue();

    expectStateNear(sim.stateVectorForTest(),
                    applySingleQubitMatrixForTest(initial, 2,
                                                  testCase.matrix, {0}));
    EXPECT_EQ(sim.specializedSingleQubitApplicationsForTest(), 1);
    EXPECT_EQ(sim.specializedSingleControlQubitApplicationsForTest(), 1);
  }
}

CUDAQ_TEST(MKLQCpuTester,
           SingleControlBuiltInHadamardGateUsesDedicatedFastPath) {
  expectSingleControlBuiltInGateUsesDedicatedFastPath(
      "ch",
      [](auto &sim, auto control, auto target) {
        sim.h(std::vector<std::size_t>{control}, target);
      },
      hMatrixForTest());
}

CUDAQ_TEST(MKLQCpuTester, SingleControlBuiltInYGateUsesDedicatedFastPath) {
  expectSingleControlBuiltInGateUsesDedicatedFastPath(
      "cy",
      [](auto &sim, auto control, auto target) {
        sim.y(std::vector<std::size_t>{control}, target);
      },
      yMatrixForTest());
}

CUDAQ_TEST(MKLQCpuTester, SingleControlBuiltInRxGateUsesDedicatedFastPath) {
  constexpr double angle = 0.375;
  expectSingleControlBuiltInGateUsesDedicatedFastPath(
      "crx",
      [&](auto &sim, auto control, auto target) {
        sim.rx(angle, std::vector<std::size_t>{control}, target);
      },
      rxMatrixForTest(angle));
}

CUDAQ_TEST(MKLQCpuTester, SingleControlBuiltInRyGateUsesDedicatedFastPath) {
  constexpr double angle = -0.625;
  expectSingleControlBuiltInGateUsesDedicatedFastPath(
      "cry",
      [&](auto &sim, auto control, auto target) {
        sim.ry(angle, std::vector<std::size_t>{control}, target);
      },
      ryMatrixForTest(angle));
}

CUDAQ_TEST(MKLQCpuTester, SingleControlRzUsesDedicatedPhaseFastPath) {
  const std::vector<std::complex<double>> initial{
      {1.0, 0.0},   {2.0, -1.0}, {0.5, 0.25}, {-3.0, 0.5},
      {0.25, -0.5}, {1.5, 0.5},  {-2.0, 1.0}, {0.75, -1.25},
  };
  constexpr double angle = 0.375;

  struct Case {
    std::string_view name;
    std::size_t control;
    std::size_t target;
  };

  for (const auto &testCase : std::vector<Case>{
           {"control below target", 0, 2},
           {"control above target", 2, 0},
       }) {
    SCOPED_TRACE(testCase.name);
    MklqCpuCircuitSimulatorTester sim;
    sim.setStateForTest(initial);

    sim.rz(angle, {testCase.control}, testCase.target);
    sim.flushGateQueue();

    expectStateNear(
        sim.stateVectorForTest(),
        applySingleQubitMatrixForTest(initial, testCase.target,
                                      rzMatrixForTest(angle),
                                      {testCase.control}));
    EXPECT_EQ(sim.specializedSingleQubitApplicationsForTest(), 1);
    EXPECT_EQ(sim.specializedSingleControlQubitApplicationsForTest(), 1);
    EXPECT_EQ(sim.phaseApplicationsForTest(), 1);
  }
}

CUDAQ_TEST(MKLQCpuTester,
           HardwareEfficientAnsatzCompositeUsesDedicatedFastPaths) {
  constexpr std::size_t qubitCount = 6;
  constexpr std::size_t layers = 3;
  constexpr std::size_t dimension = 1ULL << qubitCount;

  std::vector<std::complex<double>> initial(dimension);
  for (std::size_t index = 0; index < dimension; ++index) {
    const auto real = 0.031 * static_cast<double>(
                                  static_cast<int>(index % 7) - 3);
    const auto imag = 0.027 * static_cast<double>(
                                  static_cast<int>(index % 5) - 2);
    initial[index] = {real, imag};
  }
  const auto norm = std::sqrt(stateNormForTest(initial));
  ASSERT_GT(norm, 0.0);
  for (auto &amplitude : initial)
    amplitude /= norm;

  MklqCpuCircuitSimulatorTester sim;
  sim.setStateForTest(initial);

  std::size_t rotationGateCount = 0;
  std::size_t cxGateCount = 0;
  std::size_t crzGateCount = 0;
  std::size_t czGateCount = 0;
  std::size_t crxGateCount = 0;
  std::size_t swapGateCount = 0;

  for (std::size_t layer = 0; layer < layers; ++layer) {
    for (std::size_t index = 0; index < qubitCount; ++index) {
      const auto theta = 0.037 * static_cast<double>(layer + 1) +
                         0.011 * static_cast<double>(index + 1);
      const auto phi = -0.021 * static_cast<double>(layer + 2) +
                       0.007 * static_cast<double>(index);
      const auto lambda = 0.019 * static_cast<double>(index + 2) -
                          0.005 * static_cast<double>(layer);
      sim.ry(theta, index);
      sim.rz(phi, index);
      sim.rx(lambda, index);
      rotationGateCount += 3;
    }

    for (std::size_t index = 0; index + 1 < qubitCount; index += 2) {
      sim.x({index}, index + 1);
      sim.rz(0.043 * static_cast<double>(layer + 1) +
                 0.003 * static_cast<double>(index),
             {index}, index + 1);
      ++cxGateCount;
      ++crzGateCount;
    }

    for (std::size_t index = 1; index + 1 < qubitCount; index += 2) {
      sim.z({index}, index + 1);
      sim.rx(-0.029 * static_cast<double>(layer + 1) +
                 0.002 * static_cast<double>(index),
             {index}, index + 1);
      ++czGateCount;
      ++crxGateCount;
    }

    sim.swap(layer % qubitCount, (layer + 3) % qubitCount);
    ++swapGateCount;
  }

  sim.flushGateQueue();

  EXPECT_NEAR(stateNormForTest(sim.stateVectorForTest()), 1.0, 1.0e-12);
  EXPECT_EQ(rotationGateCount, 54);
  EXPECT_EQ(cxGateCount, 9);
  EXPECT_EQ(crzGateCount, 9);
  EXPECT_EQ(czGateCount, 6);
  EXPECT_EQ(crxGateCount, 6);
  EXPECT_EQ(swapGateCount, 3);
  EXPECT_EQ(sim.bitFlipApplicationsForTest(), cxGateCount);
  EXPECT_EQ(sim.swapApplicationsForTest(), swapGateCount);
  EXPECT_EQ(sim.phaseApplicationsForTest(), crzGateCount + czGateCount);
  EXPECT_EQ(sim.specializedSingleControlQubitApplicationsForTest(),
            cxGateCount + crzGateCount + crxGateCount);
  EXPECT_EQ(sim.specializedSingleQubitApplicationsForTest(),
            rotationGateCount + cxGateCount + crzGateCount + crxGateCount);
}

CUDAQ_TEST(MKLQCpuTester, TwoZeroBitIndexMappingClearsRequestedBits) {
  auto insertZeroBitReference = [](std::size_t value, std::size_t bit) {
    const auto lowMask = (1ULL << bit) - 1;
    return ((value & ~lowMask) << 1) | (value & lowMask);
  };

  struct Case {
    std::size_t block;
    std::size_t firstBit;
    std::size_t secondBit;
  };

  const std::vector<Case> cases{
      {0b000000, 0, 1},
      {0b111111, 0, 1},
      {0b101011, 1, 4},
      {0b111000, 4, 1},
      {0b1010101, 2, 6},
      {0b1010101, 6, 2},
  };

  for (const auto &testCase : cases) {
    SCOPED_TRACE(testCase.block);
    const auto mapped = MklqCpuCircuitSimulatorTester::
        indexWithTwoZeroBitsForTest(testCase.block, testCase.firstBit,
                                    testCase.secondBit);
    const auto first = std::min(testCase.firstBit, testCase.secondBit);
    const auto second = std::max(testCase.firstBit, testCase.secondBit);
    const auto expected = insertZeroBitReference(
        insertZeroBitReference(testCase.block, first), second);
    EXPECT_EQ(mapped, expected);
    EXPECT_EQ(mapped & (1ULL << testCase.firstBit), 0);
    EXPECT_EQ(mapped & (1ULL << testCase.secondBit), 0);
    EXPECT_EQ(std::popcount(mapped), std::popcount(testCase.block));
  }
}

CUDAQ_TEST(MKLQCpuTester, ControlMaskAggregatesRequestedControls) {
  EXPECT_EQ(MklqCpuCircuitSimulatorTester::controlMaskForTest({}), 0);
  EXPECT_EQ(MklqCpuCircuitSimulatorTester::controlMaskForTest({0}), 0b1);
  EXPECT_EQ(MklqCpuCircuitSimulatorTester::controlMaskForTest({0, 2, 5}),
            0b100101);
  EXPECT_EQ(MklqCpuCircuitSimulatorTester::controlMaskForTest({5, 0, 2}),
            0b100101);
}

CUDAQ_TEST(MKLQCpuTester,
           MultiControlBuiltInSingleQubitGatesKeepGenericSpecializedPath) {
  const std::vector<std::complex<double>> initial{
      {1.0, 0.0},   {2.0, -1.0}, {0.5, 0.25}, {-3.0, 0.5},
      {0.25, -0.5}, {1.5, 0.5},  {-2.0, 1.0}, {0.75, -1.25},
  };

  MklqCpuCircuitSimulatorTester sim;
  sim.setStateForTest(initial);

  sim.h({0, 1}, 2);
  sim.flushGateQueue();

  expectStateNear(sim.stateVectorForTest(),
                  applySingleQubitMatrixForTest(initial, 2, hMatrixForTest(),
                                                {0, 1}));
  EXPECT_EQ(sim.specializedSingleQubitApplicationsForTest(), 1);
  EXPECT_EQ(sim.specializedSingleControlQubitApplicationsForTest(), 0);
}

CUDAQ_TEST(MKLQCpuTester,
           CustomOperationsNamedSingleQubitBuiltInsUseGenericPath) {
  const std::vector<std::complex<double>> initial{
      {1.0, 0.0},
      {2.0, -1.0},
      {0.5, 0.25},
      {-3.0, 0.5},
  };
  const std::vector<std::complex<double>> identity{
      {1.0, 0.0},
      {0.0, 0.0},
      {0.0, 0.0},
      {1.0, 0.0},
  };

  for (std::string_view name :
       {"h", "y", "rx", "ry", "rz", "z", "s", "t", "sdg", "tdg"}) {
    MklqCpuCircuitSimulatorTester sim;
    sim.setStateForTest(initial);

    sim.applyCustomOperation(identity, {}, {1}, name);
    sim.flushGateQueue();

    expectStateNear(sim.stateVectorForTest(), initial);
    EXPECT_EQ(sim.specializedSingleQubitApplicationsForTest(), 0);
    EXPECT_EQ(sim.phaseApplicationsForTest(), 0);
  }

  for (std::string_view name :
       {"h", "y", "rx", "ry", "rz", "z", "s", "t", "sdg", "tdg"}) {
    MklqCpuCircuitSimulatorTester sim;
    sim.setStateForTest(initial);

    sim.applyCustomOperation(identity, {0}, {1}, name);
    sim.flushGateQueue();

    expectStateNear(sim.stateVectorForTest(), initial);
    EXPECT_EQ(sim.specializedSingleQubitApplicationsForTest(), 0);
    EXPECT_EQ(sim.phaseApplicationsForTest(), 0);
  }
}

CUDAQ_TEST(MKLQCpuTester, CzFastPathAppliesControlledZGate) {
  MklqCpuCircuitSimulatorTester sim;
  sim.setStateForTest({
      {1.0, 0.0},
      {2.0, -1.0},
      {0.5, 0.25},
      {-3.0, 0.5},
  });

  sim.z({0}, 1);
  sim.flushGateQueue();
  const auto state = sim.stateVectorForTest();

  ASSERT_EQ(state.size(), 4);
  expectNear(state[0], {1.0, 0.0});
  expectNear(state[1], {2.0, -1.0});
  expectNear(state[2], {0.5, 0.25});
  expectNear(state[3], {3.0, -0.5});
  EXPECT_EQ(sim.phaseApplicationsForTest(), 1);
}

CUDAQ_TEST(MKLQCpuTester, CustomControlledOperationNamedZUsesGenericPath) {
  MklqCpuCircuitSimulatorTester sim;
  sim.setStateForTest({
      {1.0, 0.0},
      {2.0, -1.0},
      {0.5, 0.25},
      {-3.0, 0.5},
  });

  const std::vector<std::complex<double>> identity{
      {1.0, 0.0},
      {0.0, 0.0},
      {0.0, 0.0},
      {1.0, 0.0},
  };
  sim.applyCustomOperation(identity, {0}, {1}, "z");
  sim.flushGateQueue();
  const auto state = sim.stateVectorForTest();

  ASSERT_EQ(state.size(), 4);
  expectNear(state[0], {1.0, 0.0});
  expectNear(state[1], {2.0, -1.0});
  expectNear(state[2], {0.5, 0.25});
  expectNear(state[3], {-3.0, 0.5});
  EXPECT_EQ(sim.phaseApplicationsForTest(), 0);
}

CUDAQ_TEST(MKLQCpuTester,
           UncontrolledDiagonalPhaseGatesUsePhaseFastPath) {
  const std::vector<std::complex<double>> initial{
      {1.0, 0.0},
      {2.0, -1.0},
      {0.5, 0.25},
      {-3.0, 0.5},
  };

  struct Case {
    std::string_view name;
    std::size_t target;
    std::function<void(MklqCpuCircuitSimulatorTester &, std::size_t)> apply;
    std::array<std::complex<double>, 4> matrix;
  };

  const std::vector<Case> cases{
      {"z target 1", 1, [](auto &sim, auto target) { sim.z(target); },
       zMatrixForTest()},
      {"s target 0", 0, [](auto &sim, auto target) { sim.s(target); },
       sMatrixForTest()},
      {"t target 1", 1, [](auto &sim, auto target) { sim.t(target); },
       tMatrixForTest()},
      {"sdg target 0", 0, [](auto &sim, auto target) { sim.sdg(target); },
       sdgMatrixForTest()},
      {"tdg target 1", 1, [](auto &sim, auto target) { sim.tdg(target); },
       tdgMatrixForTest()},
  };

  for (const auto &testCase : cases) {
    SCOPED_TRACE(testCase.name);
    MklqCpuCircuitSimulatorTester sim;
    sim.setStateForTest(initial);

    testCase.apply(sim, testCase.target);
    sim.flushGateQueue();

    expectStateNear(sim.stateVectorForTest(),
                    applySingleQubitMatrixForTest(initial, testCase.target,
                                                  testCase.matrix));
    EXPECT_EQ(sim.phaseApplicationsForTest(), 1);
    EXPECT_EQ(sim.specializedSingleQubitApplicationsForTest(), 0);
    EXPECT_EQ(sim.specializedSingleControlQubitApplicationsForTest(), 0);
  }
}

CUDAQ_TEST(MKLQCpuTester,
           SingleControlDiagonalPhaseGatesUsePhaseFastPath) {
  const std::vector<std::complex<double>> initial{
      {1.0, 0.0},   {2.0, -1.0}, {0.5, 0.25}, {-3.0, 0.5},
      {0.25, -0.5}, {1.5, 0.5},  {-2.0, 1.0}, {0.75, -1.25},
  };

  struct Case {
    std::string_view name;
    std::vector<std::size_t> controls;
    std::size_t target;
    std::function<void(MklqCpuCircuitSimulatorTester &,
                       const std::vector<std::size_t> &, std::size_t)>
        apply;
    std::array<std::complex<double>, 4> matrix;
  };

  const std::vector<Case> cases{
      {"cs control 0 target 2",
       {0},
       2,
       [](auto &sim, const auto &controls, auto target) {
         sim.s(controls, target);
       },
       sMatrixForTest()},
      {"ct control 2 target 0",
       {2},
       0,
       [](auto &sim, const auto &controls, auto target) {
         sim.t(controls, target);
       },
       tMatrixForTest()},
      {"csdg control 0 target 1",
       {0},
       1,
       [](auto &sim, const auto &controls, auto target) {
         sim.sdg(controls, target);
       },
       sdgMatrixForTest()},
      {"ctdg control 1 target 2",
       {1},
       2,
       [](auto &sim, const auto &controls, auto target) {
         sim.tdg(controls, target);
       },
       tdgMatrixForTest()},
  };

  for (const auto &testCase : cases) {
    SCOPED_TRACE(testCase.name);
    MklqCpuCircuitSimulatorTester sim;
    sim.setStateForTest(initial);

    testCase.apply(sim, testCase.controls, testCase.target);
    sim.flushGateQueue();

    expectStateNear(sim.stateVectorForTest(),
                    applySingleQubitMatrixForTest(
                        initial, testCase.target, testCase.matrix,
                        testCase.controls));
    EXPECT_EQ(sim.phaseApplicationsForTest(), 1);
    EXPECT_EQ(sim.specializedSingleQubitApplicationsForTest(), 1);
    EXPECT_EQ(sim.specializedSingleControlQubitApplicationsForTest(), 1);
  }
}

CUDAQ_TEST(MKLQCpuTester, SwapFastPathAppliesUncontrolledTwoQubitGate) {
  MklqCpuCircuitSimulatorTester sim;
  sim.setStateForTest({
      {1.0, 0.0},
      {2.0, -1.0},
      {0.5, 0.25},
      {-3.0, 0.5},
  });

  sim.swap(0, 1);
  sim.flushGateQueue();
  const auto state = sim.stateVectorForTest();

  ASSERT_EQ(state.size(), 4);
  expectNear(state[0], {1.0, 0.0});
  expectNear(state[1], {0.5, 0.25});
  expectNear(state[2], {2.0, -1.0});
  expectNear(state[3], {-3.0, 0.5});
  EXPECT_EQ(sim.twoQubitBlockApplicationsForTest(), 0);
  EXPECT_EQ(sim.twoQubitRowSparseApplicationsForTest(), 0);
  EXPECT_EQ(sim.swapApplicationsForTest(), 1);
}

CUDAQ_TEST(MKLQCpuTester, GenericTwoQubitBlockPathAppliesCustomGate) {
  const std::vector<std::complex<double>> initial{
      {1.0, 0.0},
      {2.0, -1.0},
      {0.5, 0.25},
      {-3.0, 0.5},
  };
  const auto invSqrt2 = 1.0 / std::sqrt(2.0);
  const std::vector<std::complex<double>> denseMix{
      {invSqrt2, 0.0}, {0.0, 0.0},       {invSqrt2, 0.0},  {0.0, 0.0},
      {0.0, 0.0},      {invSqrt2, 0.0},  {0.0, 0.0},       {invSqrt2, 0.0},
      {invSqrt2, 0.0}, {0.0, 0.0},       {-invSqrt2, 0.0}, {0.0, 0.0},
      {0.0, 0.0},      {invSqrt2, 0.0},  {0.0, 0.0},       {-invSqrt2, 0.0},
  };

  MklqCpuCircuitSimulatorTester sim;
  sim.setStateForTest(initial);

  sim.applyTwoQubitGateForTest(denseMix, {}, {0, 1});

  expectStateNear(sim.stateVectorForTest(),
                  applyTwoQubitMatrixForTest(initial, {0, 1}, denseMix, {}));
  EXPECT_EQ(sim.twoQubitBlockApplicationsForTest(), 1);
  EXPECT_EQ(sim.twoQubitRowSparseApplicationsForTest(), 0);
  EXPECT_EQ(sim.swapApplicationsForTest(), 0);
  EXPECT_EQ(sim.specializedSingleQubitApplicationsForTest(), 0);
}

CUDAQ_TEST(MKLQCpuTester, ControlledDenseTwoQubitBlockPathAppliesCustomGate) {
  const std::vector<std::complex<double>> initial{
      {1.0, 0.0},   {2.0, -1.0}, {0.5, 0.25}, {-3.0, 0.5},
      {0.25, -0.5}, {1.5, 0.5},  {-2.0, 1.0}, {0.75, -1.25},
  };
  const auto invSqrt2 = 1.0 / std::sqrt(2.0);
  const std::vector<std::complex<double>> denseMix{
      {invSqrt2, 0.0}, {0.0, 0.0},       {invSqrt2, 0.0},  {0.0, 0.0},
      {0.0, 0.0},      {invSqrt2, 0.0},  {0.0, 0.0},       {invSqrt2, 0.0},
      {invSqrt2, 0.0}, {0.0, 0.0},       {-invSqrt2, 0.0}, {0.0, 0.0},
      {0.0, 0.0},      {invSqrt2, 0.0},  {0.0, 0.0},       {-invSqrt2, 0.0},
  };

  MklqCpuCircuitSimulatorTester sim;
  sim.setStateForTest(initial);

  sim.applyTwoQubitGateForTest(denseMix, {0}, {1, 2});

  expectStateNear(
      sim.stateVectorForTest(),
      applyTwoQubitMatrixForTest(initial, {1, 2}, denseMix, {0}));
  EXPECT_EQ(sim.twoQubitBlockApplicationsForTest(), 1);
  EXPECT_EQ(sim.twoQubitRowSparseApplicationsForTest(), 0);
  EXPECT_EQ(sim.swapApplicationsForTest(), 0);
  EXPECT_EQ(sim.specializedSingleQubitApplicationsForTest(), 0);
}

CUDAQ_TEST(MKLQCpuTester,
           RowSparseTwoQubitCustomOperationUsesDedicatedFastPath) {
  const std::vector<std::complex<double>> initial{
      {1.0, 0.0},   {2.0, -1.0}, {0.5, 0.25}, {-3.0, 0.5},
      {0.25, -0.5}, {1.5, 0.5},  {-2.0, 1.0}, {0.75, -1.25},
  };
  const std::vector<std::complex<double>> phasedISwap{
      {1.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0},
      {0.0, 0.0}, {0.0, 0.0}, {0.0, 1.0}, {0.0, 0.0},
      {0.0, 0.0}, {0.0, 1.0}, {0.0, 0.0}, {0.0, 0.0},
      {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {-1.0, 0.0},
  };

  MklqCpuCircuitSimulatorTester sim;
  sim.setStateForTest(initial);

  sim.applyTwoQubitGateForTest(phasedISwap, {0}, {1, 2});

  expectStateNear(sim.stateVectorForTest(),
                  applyTwoQubitMatrixForTest(initial, {1, 2}, phasedISwap,
                                             {0}));
  EXPECT_EQ(sim.twoQubitBlockApplicationsForTest(), 0);
  EXPECT_EQ(sim.twoQubitRowSparseApplicationsForTest(), 1);
  EXPECT_EQ(sim.swapApplicationsForTest(), 0);
  EXPECT_EQ(sim.specializedSingleQubitApplicationsForTest(), 0);
}

CUDAQ_TEST(MKLQCpuTester, ControlledSwapFastPathAppliesBuiltInGate) {
  const std::vector<std::complex<double>> initial{
      {0.0, 0.0},   {1.0, -0.25}, {2.0, -0.5},  {3.0, -0.75},
      {4.0, -1.0},  {5.0, -1.25}, {6.0, -1.5},  {7.0, -1.75},
      {8.0, -2.0},  {9.0, -2.25}, {10.0, -2.5}, {11.0, -2.75},
      {12.0, -3.0}, {13.0, -3.25}, {14.0, -3.5}, {15.0, -3.75},
  };
  const std::vector<std::complex<double>> swapMatrix{
      {1.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0},
      {0.0, 0.0}, {0.0, 0.0}, {1.0, 0.0}, {0.0, 0.0},
      {0.0, 0.0}, {1.0, 0.0}, {0.0, 0.0}, {0.0, 0.0},
      {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {1.0, 0.0},
  };

  struct Case {
    std::string_view name;
    std::vector<std::size_t> controls;
    std::array<std::size_t, 2> targets;
  };

  for (const auto &testCase : std::vector<Case>{
           {"single control", {0}, {1, 2}},
           {"two controls", {0, 3}, {1, 2}},
       }) {
    SCOPED_TRACE(testCase.name);
    MklqCpuCircuitSimulatorTester sim;
    sim.setStateForTest(initial);

    sim.swap(testCase.controls, testCase.targets[0], testCase.targets[1]);
    sim.flushGateQueue();

    expectStateNear(sim.stateVectorForTest(),
                    applyTwoQubitMatrixForTest(
                        initial,
                        {testCase.targets[0], testCase.targets[1]},
                        swapMatrix, testCase.controls));
    EXPECT_EQ(sim.twoQubitBlockApplicationsForTest(), 0);
    EXPECT_EQ(sim.twoQubitRowSparseApplicationsForTest(), 0);
    EXPECT_EQ(sim.swapApplicationsForTest(), 1);
  }
}

CUDAQ_TEST(MKLQCpuTester, CustomOperationNamedSwapUsesRowSparseTwoQubitPath) {
  MklqCpuCircuitSimulatorTester sim;
  sim.setStateForTest({
      {1.0, 0.0},
      {2.0, -1.0},
      {0.5, 0.25},
      {-3.0, 0.5},
  });

  const std::vector<std::complex<double>> identity{
      {1.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {1.0, 0.0},
      {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {1.0, 0.0}, {0.0, 0.0},
      {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {1.0, 0.0},
  };
  sim.applyCustomOperation(identity, {}, {0, 1}, "swap");
  sim.flushGateQueue();
  const auto state = sim.stateVectorForTest();

  ASSERT_EQ(state.size(), 4);
  expectNear(state[0], {1.0, 0.0});
  expectNear(state[1], {2.0, -1.0});
  expectNear(state[2], {0.5, 0.25});
  expectNear(state[3], {-3.0, 0.5});
  EXPECT_EQ(sim.twoQubitBlockApplicationsForTest(), 0);
  EXPECT_EQ(sim.twoQubitRowSparseApplicationsForTest(), 1);
  EXPECT_EQ(sim.swapApplicationsForTest(), 0);
}

CUDAQ_TEST(MKLQCpuTester,
           RowSparseThreeQubitCustomOperationUsesDedicatedFastPath) {
  MklqCpuCircuitSimulatorTester sim;
  sim.setStateForTest({
      {0.0, 0.0},   {1.0, -0.25}, {2.0, -0.5},  {3.0, -0.75},
      {4.0, -1.0},  {5.0, -1.25}, {6.0, -1.5},  {7.0, -1.75},
      {8.0, -2.0},  {9.0, -2.25}, {10.0, -2.5}, {11.0, -2.75},
      {12.0, -3.0}, {13.0, -3.25}, {14.0, -3.5}, {15.0, -3.75},
  });

  std::vector<std::complex<double>> flipAll(64, {0.0, 0.0});
  for (std::size_t row = 0; row < 8; ++row)
    flipAll[row * 8 + (7 - row)] = {1.0, 0.0};

  sim.applyCustomOperation(flipAll, {3}, {2, 0, 1}, "row_sparse_flip_all");
  sim.flushGateQueue();
  const auto state = sim.stateVectorForTest();

  ASSERT_EQ(state.size(), 16);
  for (std::size_t index = 0; index < 8; ++index)
    expectNear(state[index],
               {static_cast<double>(index), -0.25 * static_cast<double>(index)});
  for (std::size_t index = 8; index < 16; ++index) {
    const auto source = index ^ 7;
    expectNear(state[index],
               {static_cast<double>(source),
                -0.25 * static_cast<double>(source)});
  }
  EXPECT_EQ(sim.threeQubitRowSparseApplicationsForTest(), 1);
}
