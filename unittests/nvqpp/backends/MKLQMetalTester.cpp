/*******************************************************************************
 * Copyright (c) 2022 - 2026 NVIDIA Corporation & Affiliates.                  *
 * All rights reserved.                                                        *
 *                                                                             *
 * This source code and the accompanying materials are made available under    *
 * the terms of the Apache License 2.0 which accompanies this distribution.    *
 ******************************************************************************/

#define MKLQ_SIMULATOR_BACKEND_NAME "mklq_metal"
#define MKLQ_SIMULATOR_CLASS MklqMetalCircuitSimulator
#define MKLQ_SIMULATOR_PRINTED_NAME mklq_metal
#define MKLQ_SIMULATOR_DIAGNOSTIC_PREFIX "[mklq-metal]"
#define MKLQ_SIMULATOR_STATE_DIAGNOSTIC_PREFIX "[mklq-metal-state]"
#include "MklqCpuCircuitSimulator.cpp"
#include "MklqMetalRuntime.h"
#include "MKLQSamplingPhaseProfile.h"

#include "CUDAQTestUtils.h"

#include <array>
#include <bit>
#include <cmath>
#include <complex>
#include <cstdint>
#include <numeric>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

enum class ResidentFailureMode {
  None,
  SingleGate,
  TwoGate,
  ThreeGate,
  FourGate,
  Probability,
  Collapse,
  Reset,
  ExpectationFlush,
  ExpectationRead
};

class MklqMetalCircuitSimulatorTester
    : public nvqir::MklqMetalCircuitSimulator {
public:
  void setResidentFailureModeForTest(ResidentFailureMode mode) {
    residentFailureMode = mode;
  }

  void setStateForTest(std::vector<std::complex<double>> data) {
#if defined(MKLQ_ENABLE_METAL_RUNTIME)
    invalidateMetalResidentState();
#endif
    nQubitsAllocated = std::countr_zero(data.size());
    previousStateDimension = stateDimension;
    stateDimension = data.size();
    state = std::move(data);
  }

  std::vector<double> fullRegisterProbabilitiesForTest() {
    std::vector<double> probabilities(state.size(), 0.0);
    fillFullRegisterProbabilities(probabilities);
    return probabilities;
  }

  cudaq::ExecutionResult sampleQubitsForTest(
      const std::vector<std::size_t> &qubits, int shots) {
    return sample(qubits, shots);
  }

  cudaq::ExecutionResult sampleQubitsWithoutSequentialDataForTest(
      const std::vector<std::size_t> &qubits, int shots) {
    return sample(qubits, shots, false);
  }

  void applySingleQubitGateForTest(
      const std::vector<std::complex<double>> &matrix,
      const std::vector<std::size_t> &controls, std::size_t target) {
    applySingleQubitGate(matrix, controls, target, "", false);
  }

  void applyGateTaskForTest(const std::string &name,
                            const std::vector<std::complex<double>> &matrix,
                            const std::vector<std::size_t> &controls,
                            const std::vector<std::size_t> &targets,
                            bool isBuiltInOperation = false) {
    nvqir::CircuitSimulatorBase<double>::GateApplicationTask task(
        name, matrix, controls, targets, {}, isBuiltInOperation);
    applyGate(task);
  }

  bool measureQubitForTest(std::size_t qubit) { return measureQubit(qubit); }

  std::vector<std::complex<double>> stateVectorForTest() {
    auto simulationState = getSimulationState();
    std::vector<std::complex<double>> output(state.size());
    simulationState->toHost(output.data(), output.size());
    return output;
  }

  bool metalRuntimeAvailableForTest() const {
#if defined(MKLQ_ENABLE_METAL_RUNTIME)
    return metalExecutor.available();
#else
    return false;
#endif
  }

  std::size_t probabilityFillApplicationsForTest() const {
#if defined(MKLQ_ENABLE_METAL_RUNTIME)
    return metalExecutor.probabilityFillApplications();
#else
    return 0;
#endif
  }

  double residentProbabilityFillDispatchSecondsForTest() const {
#if defined(MKLQ_ENABLE_METAL_RUNTIME)
    return metalExecutor.residentProbabilityFillDispatchSeconds();
#else
    return 0.0;
#endif
  }

  double residentProbabilityFillBufferPreparationSecondsForTest() const {
#if defined(MKLQ_ENABLE_METAL_RUNTIME)
    return metalExecutor.residentProbabilityFillBufferPreparationSeconds();
#else
    return 0.0;
#endif
  }

  double residentProbabilityFillGateFlushSecondsForTest() const {
#if defined(MKLQ_ENABLE_METAL_RUNTIME)
    return metalExecutor.residentProbabilityFillGateFlushSeconds();
#else
    return 0.0;
#endif
  }

  double residentProbabilityFillHostConversionSecondsForTest() const {
#if defined(MKLQ_ENABLE_METAL_RUNTIME)
    return metalExecutor.residentProbabilityFillHostConversionSeconds();
#else
    return 0.0;
#endif
  }

  std::size_t measurementProbabilityApplicationsForTest() const {
#if defined(MKLQ_ENABLE_METAL_RUNTIME)
    return metalExecutor.measurementProbabilityApplications();
#else
    return 0;
#endif
  }

  std::size_t measurementProbabilityReductionApplicationsForTest() const {
#if defined(MKLQ_ENABLE_METAL_RUNTIME)
    return metalExecutor.measurementProbabilityReductionApplications();
#else
    return 0;
#endif
  }

  std::size_t measurementCollapseApplicationsForTest() const {
#if defined(MKLQ_ENABLE_METAL_RUNTIME)
    return metalExecutor.measurementCollapseApplications();
#else
    return 0;
#endif
  }

  std::size_t sampleCountAccumulationsForTest() const {
#if defined(MKLQ_ENABLE_METAL_RUNTIME)
    return metalExecutor.sampleCountAccumulations();
#else
    return 0;
#endif
  }

  std::size_t generatedSampleCountAccumulationsForTest() const {
#if defined(MKLQ_ENABLE_METAL_RUNTIME)
    return metalExecutor.generatedSampleCountAccumulations();
#else
    return 0;
#endif
  }

  std::size_t uniformGeneratedSampleCountAccumulationsForTest() const {
#if defined(MKLQ_ENABLE_METAL_RUNTIME)
    return metalExecutor.uniformGeneratedSampleCountAccumulations();
#else
    return 0;
#endif
  }

  std::size_t marginalProbabilityApplicationsForTest() const {
#if defined(MKLQ_ENABLE_METAL_RUNTIME)
    return metalExecutor.marginalProbabilityApplications();
#else
    return 0;
#endif
  }

  bool shouldUseResidentMarginalProbabilitiesForTest(
      std::size_t probabilityCount) const {
#if defined(MKLQ_ENABLE_METAL_RUNTIME)
    return shouldUseMetalResidentMarginalProbabilities(probabilityCount);
#else
    return false;
#endif
  }

  std::size_t singleQubitApplicationsForTest() const {
#if defined(MKLQ_ENABLE_METAL_RUNTIME)
    return metalExecutor.singleQubitGateApplications();
#else
    return 0;
#endif
  }

  std::size_t twoQubitApplicationsForTest() const {
#if defined(MKLQ_ENABLE_METAL_RUNTIME)
    return metalExecutor.twoQubitGateApplications();
#else
    return 0;
#endif
  }

  std::size_t threeQubitApplicationsForTest() const {
#if defined(MKLQ_ENABLE_METAL_RUNTIME)
    return metalExecutor.threeQubitGateApplications();
#else
    return 0;
#endif
  }

  std::size_t fourQubitApplicationsForTest() const {
#if defined(MKLQ_ENABLE_METAL_RUNTIME)
    return metalExecutor.fourQubitGateApplications();
#else
    return 0;
#endif
  }

  std::size_t residentStateUploadsForTest() const {
#if defined(MKLQ_ENABLE_METAL_RUNTIME)
    return metalExecutor.residentStateUploads();
#else
    return 0;
#endif
  }

  std::size_t residentStateDownloadsForTest() const {
#if defined(MKLQ_ENABLE_METAL_RUNTIME)
    return metalExecutor.residentStateDownloads();
#else
    return 0;
#endif
  }

  std::size_t residentGateCommandBufferSubmissionsForTest() const {
#if defined(MKLQ_ENABLE_METAL_RUNTIME)
    return metalExecutor.residentGateCommandBufferSubmissions();
#else
    return 0;
#endif
  }

  std::size_t metalCpuFallbackApplicationsForTest() const {
#if defined(MKLQ_ENABLE_TEST_ACCESSORS)
    return metalCpuFallbackApplications;
#else
    return 0;
#endif
  }

  std::size_t bitStringConversionsForTest() const {
    return bitStringConversions;
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

  std::size_t denseDrawCountBuffersForTest() const {
    return denseDrawCountBuffers;
  }

  std::size_t sparseDrawCountMapsForTest() const {
    return sparseDrawCountMaps;
  }

  double sampleProbabilityFillSecondsForTest() const {
    return sampleProbabilityFillSeconds;
  }

  double sampleProbabilityHostFoldSecondsForTest() const {
    return sampleProbabilityHostFoldSeconds;
  }

  double sampleFullRegisterProbabilityBufferPreparationSecondsForTest() const {
    return sampleFullRegisterProbabilityBufferPreparationSeconds;
  }

  double sampleDrawAndCountSecondsForTest() const {
    return sampleDrawAndCountSeconds;
  }

  double sampleExpectationReductionSecondsForTest() const {
    return sampleExpectationReductionSeconds;
  }

protected:
#if defined(MKLQ_ENABLE_METAL_RUNTIME)
  bool applyMetalResidentSingleQubitGate(
      const std::complex<double> *matrix, const std::size_t *controlQubits,
      std::size_t controlCount, std::size_t target) override {
    if (residentFailureMode == ResidentFailureMode::SingleGate)
      return false;
    return nvqir::MklqMetalCircuitSimulator::applyMetalResidentSingleQubitGate(
        matrix, controlQubits, controlCount, target);
  }

  bool applyMetalResidentTwoQubitGate(
      const std::complex<double> *matrix, const std::size_t *controlQubits,
      std::size_t controlCount, const std::size_t *targets) override {
    if (residentFailureMode == ResidentFailureMode::TwoGate)
      return false;
    return nvqir::MklqMetalCircuitSimulator::applyMetalResidentTwoQubitGate(
        matrix, controlQubits, controlCount, targets);
  }

  bool applyMetalResidentThreeQubitGate(
      const std::complex<double> *matrix, const std::size_t *controlQubits,
      std::size_t controlCount, const std::size_t *targets) override {
    if (residentFailureMode == ResidentFailureMode::ThreeGate)
      return false;
    return nvqir::MklqMetalCircuitSimulator::applyMetalResidentThreeQubitGate(
        matrix, controlQubits, controlCount, targets);
  }

  bool applyMetalResidentFourQubitGate(
      const std::complex<double> *matrix, const std::size_t *controlQubits,
      std::size_t controlCount, const std::size_t *targets) override {
    if (residentFailureMode == ResidentFailureMode::FourGate)
      return false;
    return nvqir::MklqMetalCircuitSimulator::applyMetalResidentFourQubitGate(
        matrix, controlQubits, controlCount, targets);
  }

  bool computeMetalResidentMeasurementProbability(
      std::size_t qubit, double &probabilityOne) override {
    if (residentFailureMode == ResidentFailureMode::Probability)
      return false;
    return nvqir::MklqMetalCircuitSimulator::
        computeMetalResidentMeasurementProbability(qubit, probabilityOne);
  }

  bool collapseMetalResidentMeasurement(std::size_t qubit, bool result,
                                        double branchProbability) override {
    if (residentFailureMode == ResidentFailureMode::Collapse)
      return false;
    return nvqir::MklqMetalCircuitSimulator::collapseMetalResidentMeasurement(
        qubit, result, branchProbability);
  }

  bool applyMetalResidentResetGate(
      std::size_t qubit,
      const std::complex<double> *matrix) override {
    if (residentFailureMode == ResidentFailureMode::Reset)
      return false;
    return nvqir::MklqMetalCircuitSimulator::applyMetalResidentResetGate(
        qubit, matrix);
  }

  bool flushMetalResidentGateCommands() override {
    if (residentFailureMode == ResidentFailureMode::ExpectationFlush)
      return false;
    return nvqir::MklqMetalCircuitSimulator::flushMetalResidentGateCommands();
  }

  bool computeMetalResidentZParityExpectation(
      const std::size_t *qubits, std::size_t qubitCount,
      double &expectation) override {
    if (residentFailureMode == ResidentFailureMode::ExpectationRead)
      return false;
    return nvqir::MklqMetalCircuitSimulator::
        computeMetalResidentZParityExpectation(qubits, qubitCount, expectation);
  }
#endif

private:
  ResidentFailureMode residentFailureMode = ResidentFailureMode::None;
};

static void recordSamplingPhaseProfile(
    const mklq_test::SamplingPhaseProfileConfig &config,
    const MklqMetalCircuitSimulatorTester &sim) {
  ::testing::Test::RecordProperty("mklq_sampling_phase_profile", "true");
  ::testing::Test::RecordProperty("mklq_sampling_phase_profile_target",
                                  "mklq-metal");
  ::testing::Test::RecordProperty("mklq_sampling_phase_profile_qubits",
                                  std::to_string(config.qubitCount));
  ::testing::Test::RecordProperty(
      "mklq_sampling_phase_profile_measured_qubits",
      std::to_string(config.measuredQubitCount));
  ::testing::Test::RecordProperty("mklq_sampling_phase_profile_shots",
                                  std::to_string(config.shots));
  ::testing::Test::RecordProperty(
      "mklq_sampling_phase_profile_probability_fill_seconds",
      mklq_test::formatPhaseSeconds(sim.sampleProbabilityFillSecondsForTest()));
  ::testing::Test::RecordProperty(
      "mklq_sampling_phase_profile_host_fold_seconds",
      mklq_test::formatPhaseSeconds(sim.sampleProbabilityHostFoldSecondsForTest()));
  ::testing::Test::RecordProperty(
      "mklq_sampling_phase_profile_full_register_probability_buffer_preparation_seconds",
      mklq_test::formatPhaseSeconds(
          sim.sampleFullRegisterProbabilityBufferPreparationSecondsForTest()));
  ::testing::Test::RecordProperty(
      "mklq_sampling_phase_profile_metal_probability_buffer_preparation_seconds",
      mklq_test::formatPhaseSeconds(
          sim.residentProbabilityFillBufferPreparationSecondsForTest()));
  ::testing::Test::RecordProperty(
      "mklq_sampling_phase_profile_metal_probability_gate_flush_seconds",
      mklq_test::formatPhaseSeconds(
          sim.residentProbabilityFillGateFlushSecondsForTest()));
  ::testing::Test::RecordProperty(
      "mklq_sampling_phase_profile_metal_probability_dispatch_seconds",
      mklq_test::formatPhaseSeconds(
          sim.residentProbabilityFillDispatchSecondsForTest()));
  ::testing::Test::RecordProperty(
      "mklq_sampling_phase_profile_metal_probability_host_conversion_seconds",
      mklq_test::formatPhaseSeconds(
          sim.residentProbabilityFillHostConversionSecondsForTest()));
  ::testing::Test::RecordProperty(
      "mklq_sampling_phase_profile_draw_and_count_seconds",
      mklq_test::formatPhaseSeconds(sim.sampleDrawAndCountSecondsForTest()));
  ::testing::Test::RecordProperty(
      "mklq_sampling_phase_profile_expectation_reduction_seconds",
      mklq_test::formatPhaseSeconds(
          sim.sampleExpectationReductionSecondsForTest()));
  ::testing::Test::RecordProperty(
      "mklq_sampling_phase_profile_metal_probability_fill_applications",
      std::to_string(sim.probabilityFillApplicationsForTest()));
  ::testing::Test::RecordProperty(
      "mklq_sampling_phase_profile_metal_marginal_probability_applications",
      std::to_string(sim.marginalProbabilityApplicationsForTest()));
  ::testing::Test::RecordProperty(
      "mklq_sampling_phase_profile_metal_generated_count_accumulations",
      std::to_string(sim.generatedSampleCountAccumulationsForTest()));
}

void expectNear(std::complex<double> actual, std::complex<double> expected) {
  EXPECT_NEAR(actual.real(), expected.real(), 1.0e-6);
  EXPECT_NEAR(actual.imag(), expected.imag(), 1.0e-6);
}

std::vector<std::complex<double>> identityGateForTargets(
    std::size_t targetCount) {
  const auto dimension = 1ULL << targetCount;
  std::vector<std::complex<double>> matrix(dimension * dimension, {0.0, 0.0});
  for (std::size_t index = 0; index < dimension; ++index)
    matrix[index * dimension + index] = {1.0, 0.0};
  return matrix;
}

double zParityExpectationCpuOracle(
    const std::vector<std::complex<double>> &state,
    const std::vector<std::size_t> &qubits) {
  std::size_t qubitMask = 0;
  for (const auto qubit : qubits)
    qubitMask |= std::size_t{1} << qubit;

  double expectation = 0.0;
  for (std::size_t basis = 0; basis < state.size(); ++basis) {
    const auto sign = std::popcount(basis & qubitMask) % 2 == 0 ? 1.0 : -1.0;
    expectation += sign * std::norm(state[basis]);
  }
  return expectation;
}

bool controlsSatisfiedForBasis(std::size_t basis,
                               const std::vector<std::size_t> &controls) {
  for (auto control : controls)
    if ((basis & (1ULL << control)) == 0)
      return false;
  return true;
}

void applyExpectedSingleQubitGate(
    std::vector<std::complex<double>> &state,
    const std::array<std::complex<double>, 4> &matrix,
    const std::vector<std::size_t> &controls, std::size_t target) {
  const auto mask = 1ULL << target;
  for (std::size_t zeroIndex = 0; zeroIndex < state.size(); ++zeroIndex) {
    if ((zeroIndex & mask) != 0 ||
        !controlsSatisfiedForBasis(zeroIndex, controls))
      continue;

    const auto oneIndex = zeroIndex | mask;
    const auto zeroAmplitude = state[zeroIndex];
    const auto oneAmplitude = state[oneIndex];
    state[zeroIndex] = matrix[0] * zeroAmplitude + matrix[1] * oneAmplitude;
    state[oneIndex] = matrix[2] * zeroAmplitude + matrix[3] * oneAmplitude;
  }
}

bool expectMetalRuntimeReadyOrUnavailable(
    const nvqir::mklq::MetalStateVectorExecutor &executor) {
  if (executor.available())
    return true;

  const auto device = nvqir::mklq::queryMetalDevice();
  EXPECT_FALSE(device.available) << executor.lastError();
  return false;
}

} // namespace

CUDAQ_TEST(MKLQMetalTester, RegistersSeparateBackendName) {
  nvqir::MklqMetalCircuitSimulator sim;

  EXPECT_EQ(sim.name(), "mklq_metal");
  EXPECT_EQ(std::string(sim.clone()->name()), "mklq_metal");
}

CUDAQ_TEST(MKLQMetalTester, DiagnosticsUseMetalPrefix) {
  nvqir::MklqMetalCircuitSimulator sim;

  try {
    std::vector<std::complex<double>> amplitudes{{1.0, 0.0}, {0.0, 0.0},
                                                 {0.0, 0.0}};
    cudaq::state_data invalidState{amplitudes};
    (void)sim.createStateFromData(invalidState);
  } catch (const std::runtime_error &error) {
    EXPECT_NE(std::string(error.what()).find("[mklq-metal]"),
              std::string::npos)
        << error.what();
    return;
  }

  FAIL() << "expected invalid metal state construction to raise";
}

CUDAQ_TEST(MKLQMetalTester, DetectsMetalRuntimeDevice) {
  const auto device = nvqir::mklq::queryMetalDevice();

  if (device.available)
    EXPECT_FALSE(device.name.empty());
  else
    EXPECT_TRUE(device.name.empty());
}

CUDAQ_TEST(MKLQMetalTester, MetalRuntimeAppliesSingleQubitGate) {
  nvqir::mklq::MetalStateVectorExecutor executor;

  if (!expectMetalRuntimeReadyOrUnavailable(executor))
    return;

  constexpr double invSqrt2 = 0.70710678118654752440;
  std::vector<std::complex<double>> state{{1.0, 0.0}, {0.0, 0.0}};
  const std::array<std::complex<double>, 4> hGate{
      std::complex<double>{invSqrt2, 0.0},
      std::complex<double>{invSqrt2, 0.0},
      std::complex<double>{invSqrt2, 0.0},
      std::complex<double>{-invSqrt2, 0.0}};

  ASSERT_TRUE(executor.applySingleQubitGate(state.data(), state.size(),
                                            hGate.data(), nullptr, 0, 0))
      << executor.lastError();

  expectNear(state[0], {invSqrt2, 0.0});
  expectNear(state[1], {invSqrt2, 0.0});
  EXPECT_EQ(executor.singleQubitGateApplications(), 1);
}

CUDAQ_TEST(MKLQMetalTester, MetalRuntimeAppliesControlledSingleQubitGate) {
  nvqir::mklq::MetalStateVectorExecutor executor;

  if (!expectMetalRuntimeReadyOrUnavailable(executor))
    return;

  std::vector<std::complex<double>> state{{0.0, 0.0},
                                          {0.0, 0.0},
                                          {1.0, 0.0},
                                          {0.0, 0.0}};
  const std::array<std::complex<double>, 4> xGate{
      std::complex<double>{0.0, 0.0}, std::complex<double>{1.0, 0.0},
      std::complex<double>{1.0, 0.0}, std::complex<double>{0.0, 0.0}};
  const std::array<std::size_t, 1> controls{1};

  ASSERT_TRUE(executor.applySingleQubitGate(state.data(), state.size(),
                                            xGate.data(), controls.data(),
                                            controls.size(), 0))
      << executor.lastError();

  expectNear(state[0], {0.0, 0.0});
  expectNear(state[1], {0.0, 0.0});
  expectNear(state[2], {0.0, 0.0});
  expectNear(state[3], {1.0, 0.0});
  EXPECT_EQ(executor.singleQubitGateApplications(), 1);
}

CUDAQ_TEST(MKLQMetalTester, MetalRuntimeAppliesTwoQubitGate) {
  nvqir::mklq::MetalStateVectorExecutor executor;

  if (!expectMetalRuntimeReadyOrUnavailable(executor))
    return;

  std::vector<std::complex<double>> state{{0.0, 0.0},
                                          {1.0, 0.0},
                                          {0.0, 0.0},
                                          {0.0, 0.0}};
  const std::array<std::complex<double>, 16> swapGate{
      std::complex<double>{1.0, 0.0}, std::complex<double>{0.0, 0.0},
      std::complex<double>{0.0, 0.0}, std::complex<double>{0.0, 0.0},
      std::complex<double>{0.0, 0.0}, std::complex<double>{0.0, 0.0},
      std::complex<double>{1.0, 0.0}, std::complex<double>{0.0, 0.0},
      std::complex<double>{0.0, 0.0}, std::complex<double>{1.0, 0.0},
      std::complex<double>{0.0, 0.0}, std::complex<double>{0.0, 0.0},
      std::complex<double>{0.0, 0.0}, std::complex<double>{0.0, 0.0},
      std::complex<double>{0.0, 0.0}, std::complex<double>{1.0, 0.0}};
  const std::array<std::size_t, 2> targets{0, 1};

  ASSERT_TRUE(executor.applyTwoQubitGate(state.data(), state.size(),
                                         swapGate.data(), nullptr, 0,
                                         targets.data()))
      << executor.lastError();

  expectNear(state[0], {0.0, 0.0});
  expectNear(state[1], {0.0, 0.0});
  expectNear(state[2], {1.0, 0.0});
  expectNear(state[3], {0.0, 0.0});
  EXPECT_EQ(executor.twoQubitGateApplications(), 1);
}

CUDAQ_TEST(MKLQMetalTester, MetalRuntimeAppliesControlledTwoQubitGate) {
  nvqir::mklq::MetalStateVectorExecutor executor;

  if (!expectMetalRuntimeReadyOrUnavailable(executor))
    return;

  std::vector<std::complex<double>> state{{0.0, 0.0}, {0.0, 0.0},
                                          {0.0, 0.0}, {0.0, 0.0},
                                          {0.0, 0.0}, {0.0, 0.0},
                                          {0.0, 0.0}, {1.0, 0.0}};
  const std::array<std::complex<double>, 16> czGate{
      std::complex<double>{1.0, 0.0},  std::complex<double>{0.0, 0.0},
      std::complex<double>{0.0, 0.0},  std::complex<double>{0.0, 0.0},
      std::complex<double>{0.0, 0.0},  std::complex<double>{1.0, 0.0},
      std::complex<double>{0.0, 0.0},  std::complex<double>{0.0, 0.0},
      std::complex<double>{0.0, 0.0},  std::complex<double>{0.0, 0.0},
      std::complex<double>{1.0, 0.0},  std::complex<double>{0.0, 0.0},
      std::complex<double>{0.0, 0.0},  std::complex<double>{0.0, 0.0},
      std::complex<double>{0.0, 0.0},  std::complex<double>{-1.0, 0.0}};
  const std::array<std::size_t, 1> controls{2};
  const std::array<std::size_t, 2> targets{0, 1};

  ASSERT_TRUE(executor.applyTwoQubitGate(state.data(), state.size(),
                                         czGate.data(), controls.data(),
                                         controls.size(), targets.data()))
      << executor.lastError();

  expectNear(state[7], {-1.0, 0.0});
  EXPECT_EQ(executor.twoQubitGateApplications(), 1);
}

CUDAQ_TEST(MKLQMetalTester, MetalRuntimeAppliesResidentThreeQubitGate) {
  nvqir::mklq::MetalStateVectorExecutor executor;

  if (!expectMetalRuntimeReadyOrUnavailable(executor))
    return;

  std::vector<std::complex<double>> state(8, {0.0, 0.0});
  state[0] = {1.0, 0.0};

  std::array<std::complex<double>, 64> flipAllGate{};
  for (std::size_t row = 0; row < 8; ++row)
    flipAllGate[row * 8 + (7 - row)] = {1.0, 0.0};
  const std::array<std::size_t, 3> targets{0, 1, 2};

  ASSERT_TRUE(executor.uploadState(state.data(), state.size()))
      << executor.lastError();
  ASSERT_TRUE(executor.applyResidentThreeQubitGate(
      flipAllGate.data(), nullptr, 0, targets.data()))
      << executor.lastError();
  ASSERT_TRUE(executor.downloadState(state.data(), state.size()))
      << executor.lastError();

  for (std::size_t index = 0; index + 1 < state.size(); ++index)
    expectNear(state[index], {0.0, 0.0});
  expectNear(state[7], {1.0, 0.0});
  EXPECT_EQ(executor.residentStateUploads(), 1);
  EXPECT_EQ(executor.residentStateDownloads(), 1);
  EXPECT_EQ(executor.threeQubitGateApplications(), 1);
}

CUDAQ_TEST(MKLQMetalTester, MetalRuntimeFillsFullRegisterProbabilities) {
  nvqir::mklq::MetalStateVectorExecutor executor;

  if (!expectMetalRuntimeReadyOrUnavailable(executor))
    return;

  std::vector<std::complex<double>> state{
      {3.0, 4.0}, {0.5, -0.5}, {0.0, 0.0}, {-2.0, 1.5}};
  std::vector<double> probabilities(state.size(), 0.0);

  ASSERT_TRUE(executor.fillFullRegisterProbabilities(
      state.data(), state.size(), probabilities.data(), probabilities.size()))
      << executor.lastError();

  EXPECT_DOUBLE_EQ(probabilities[0], 25.0);
  EXPECT_DOUBLE_EQ(probabilities[1], 0.5);
  EXPECT_DOUBLE_EQ(probabilities[2], 0.0);
  EXPECT_DOUBLE_EQ(probabilities[3], 6.25);
  EXPECT_EQ(executor.probabilityFillApplications(), 1);
}

CUDAQ_TEST(MKLQMetalTester, MetalRuntimeProbabilityFillMatchesCpuNorms) {
  nvqir::mklq::MetalStateVectorExecutor executor;

  if (!expectMetalRuntimeReadyOrUnavailable(executor))
    return;

  std::vector<std::complex<double>> state{
      {0.1, -0.2}, {-0.3, 0.125}, {1.0 / 3.0, -2.0 / 7.0},
      {-0.875, 0.0625}};
  std::vector<double> probabilities(state.size(), 0.0);

  ASSERT_TRUE(executor.fillFullRegisterProbabilities(
      state.data(), state.size(), probabilities.data(), probabilities.size()))
      << executor.lastError();

  for (std::size_t index = 0; index < state.size(); ++index)
    EXPECT_NEAR(probabilities[index], std::norm(state[index]), 1.0e-6)
        << "index " << index;
  EXPECT_EQ(executor.probabilityFillApplications(), 1);
}

CUDAQ_TEST(MKLQMetalTester,
           MetalRuntimeFillsResidentMarginalProbabilities) {
  nvqir::mklq::MetalStateVectorExecutor executor;

  if (!expectMetalRuntimeReadyOrUnavailable(executor))
    return;

  std::vector<std::complex<double>> state(8, {0.0, 0.0});
  state[0] = {std::sqrt(0.1), 0.0};
  state[1] = {std::sqrt(0.1), 0.0};
  state[3] = {std::sqrt(0.2), 0.0};
  state[4] = {std::sqrt(0.2), 0.0};
  state[5] = {std::sqrt(0.4), 0.0};

  const std::array<std::size_t, 2> qubits{2, 0};
  std::vector<double> probabilities(1ULL << qubits.size(), 0.0);

  ASSERT_TRUE(executor.uploadState(state.data(), state.size()))
      << executor.lastError();
  ASSERT_TRUE(executor.fillResidentMarginalProbabilities(
      qubits.data(), qubits.size(), probabilities.data(),
      probabilities.size()))
      << executor.lastError();

  ASSERT_EQ(probabilities.size(), 4);
  EXPECT_NEAR(probabilities[0], 0.1, 1.0e-6);
  EXPECT_NEAR(probabilities[1], 0.2, 1.0e-6);
  EXPECT_NEAR(probabilities[2], 0.3, 1.0e-6);
  EXPECT_NEAR(probabilities[3], 0.4, 1.0e-6);
  EXPECT_EQ(executor.marginalProbabilityApplications(), 1);
  EXPECT_EQ(executor.residentStateDownloads(), 0);
}

CUDAQ_TEST(MKLQMetalTester,
           MetalRuntimeComputesAndCollapsesResidentQubitProbability) {
  nvqir::mklq::MetalStateVectorExecutor executor;

  if (!expectMetalRuntimeReadyOrUnavailable(executor))
    return;

  std::vector<std::complex<double>> state{{0.5, 0.0},
                                          {0.0, 0.0},
                                          {std::sqrt(0.75), 0.0},
                                          {0.0, 0.0}};
  double probabilityOne = 0.0;

  ASSERT_TRUE(executor.uploadState(state.data(), state.size()))
      << executor.lastError();
  ASSERT_TRUE(executor.computeResidentQubitProbability(1, &probabilityOne))
      << executor.lastError();
  EXPECT_NEAR(probabilityOne, 0.75, 1.0e-6);

  ASSERT_TRUE(executor.collapseResidentQubit(1, true, probabilityOne))
      << executor.lastError();
  ASSERT_TRUE(executor.downloadState(state.data(), state.size()))
      << executor.lastError();

  expectNear(state[0], {0.0, 0.0});
  expectNear(state[1], {0.0, 0.0});
  expectNear(state[2], {1.0, 0.0});
  expectNear(state[3], {0.0, 0.0});
  EXPECT_EQ(executor.measurementProbabilityApplications(), 1);
  EXPECT_EQ(executor.measurementProbabilityReductionApplications(), 1);
  EXPECT_EQ(executor.probabilityFillApplications(), 0);
  EXPECT_EQ(executor.measurementCollapseApplications(), 1);
  EXPECT_EQ(executor.residentStateDownloads(), 1);
}

CUDAQ_TEST(MKLQMetalTester, MetalRuntimeKeepsResidentStateAcrossGateSequence) {
  nvqir::mklq::MetalStateVectorExecutor executor;

  if (!expectMetalRuntimeReadyOrUnavailable(executor))
    return;

  constexpr double invSqrt2 = 0.70710678118654752440;
  std::vector<std::complex<double>> state{{1.0, 0.0},
                                          {0.0, 0.0},
                                          {0.0, 0.0},
                                          {0.0, 0.0}};
  const std::array<std::complex<double>, 4> hGate{
      std::complex<double>{invSqrt2, 0.0},
      std::complex<double>{invSqrt2, 0.0},
      std::complex<double>{invSqrt2, 0.0},
      std::complex<double>{-invSqrt2, 0.0}};
  const std::array<std::complex<double>, 4> xGate{
      std::complex<double>{0.0, 0.0}, std::complex<double>{1.0, 0.0},
      std::complex<double>{1.0, 0.0}, std::complex<double>{0.0, 0.0}};
  const std::array<std::size_t, 1> controls{0};

  ASSERT_TRUE(executor.uploadState(state.data(), state.size()))
      << executor.lastError();
  ASSERT_TRUE(executor.applyResidentSingleQubitGate(hGate.data(), nullptr, 0,
                                                    0))
      << executor.lastError();
  ASSERT_TRUE(executor.applyResidentSingleQubitGate(xGate.data(),
                                                    controls.data(),
                                                    controls.size(), 1))
      << executor.lastError();
  ASSERT_TRUE(executor.downloadState(state.data(), state.size()))
      << executor.lastError();

  expectNear(state[0], {invSqrt2, 0.0});
  expectNear(state[1], {0.0, 0.0});
  expectNear(state[2], {0.0, 0.0});
  expectNear(state[3], {invSqrt2, 0.0});
  EXPECT_EQ(executor.residentStateUploads(), 1);
  EXPECT_EQ(executor.residentStateDownloads(), 1);
  EXPECT_EQ(executor.singleQubitGateApplications(), 2);
}

CUDAQ_TEST(MKLQMetalTester,
           MetalRuntimeKeepsResidentYAndControlledYSequence) {
  nvqir::mklq::MetalStateVectorExecutor executor;

  if (!expectMetalRuntimeReadyOrUnavailable(executor))
    return;

  std::vector<std::complex<double>> state{{1.0, 0.0},
                                          {0.0, 0.0},
                                          {0.0, 0.0},
                                          {0.0, 0.0}};
  const std::array<std::complex<double>, 4> yGate{
      std::complex<double>{0.0, 0.0}, std::complex<double>{0.0, -1.0},
      std::complex<double>{0.0, 1.0}, std::complex<double>{0.0, 0.0}};
  const std::array<std::size_t, 1> controls{0};

  ASSERT_TRUE(executor.uploadState(state.data(), state.size()))
      << executor.lastError();
  ASSERT_TRUE(executor.applyResidentSingleQubitGate(yGate.data(), nullptr, 0,
                                                    0))
      << executor.lastError();
  ASSERT_TRUE(executor.applyResidentSingleQubitGate(yGate.data(),
                                                    controls.data(),
                                                    controls.size(), 1))
      << executor.lastError();
  ASSERT_TRUE(executor.downloadState(state.data(), state.size()))
      << executor.lastError();

  expectNear(state[0], {0.0, 0.0});
  expectNear(state[1], {0.0, 0.0});
  expectNear(state[2], {0.0, 0.0});
  expectNear(state[3], {-1.0, 0.0});
  EXPECT_EQ(executor.residentStateUploads(), 1);
  EXPECT_EQ(executor.residentStateDownloads(), 1);
  EXPECT_EQ(executor.singleQubitGateApplications(), 2);
}

CUDAQ_TEST(MKLQMetalTester,
           MetalRuntimeFillsResidentProbabilitiesWithoutStateReadback) {
  nvqir::mklq::MetalStateVectorExecutor executor;

  if (!expectMetalRuntimeReadyOrUnavailable(executor))
    return;

  constexpr double invSqrt2 = 0.70710678118654752440;
  std::vector<std::complex<double>> state{{invSqrt2, 0.0},
                                          {0.0, 0.0},
                                          {0.0, 0.0},
                                          {invSqrt2, 0.0}};
  std::vector<double> probabilities(state.size(), 0.0);

  ASSERT_TRUE(executor.uploadState(state.data(), state.size()))
      << executor.lastError();
  ASSERT_TRUE(executor.fillResidentFullRegisterProbabilities(
      probabilities.data(), probabilities.size()))
      << executor.lastError();

  EXPECT_NEAR(probabilities[0], 0.5, 1.0e-6);
  EXPECT_NEAR(probabilities[1], 0.0, 1.0e-6);
  EXPECT_NEAR(probabilities[2], 0.0, 1.0e-6);
  EXPECT_NEAR(probabilities[3], 0.5, 1.0e-6);
  EXPECT_EQ(executor.residentStateUploads(), 1);
  EXPECT_EQ(executor.residentStateDownloads(), 0);
  EXPECT_EQ(executor.probabilityFillApplications(), 1);
}

CUDAQ_TEST(MKLQMetalTester, MetalRuntimeAccumulatesSampleCounts) {
  nvqir::mklq::MetalStateVectorExecutor executor;

  if (!expectMetalRuntimeReadyOrUnavailable(executor))
    return;

  const std::array<double, 3> probabilities{0.25, 0.25, 0.5};
  const std::array<double, 6> draws{0.0, 0.24, 0.25, 0.49, 0.5, 0.99};
  std::array<std::uint32_t, 3> counts{0, 0, 0};

  ASSERT_TRUE(executor.accumulateSampleCounts(
      probabilities.data(), probabilities.size(), draws.data(), draws.size(),
      counts.data(), counts.size()))
      << executor.lastError();

  EXPECT_EQ(counts[0], 2u);
  EXPECT_EQ(counts[1], 2u);
  EXPECT_EQ(counts[2], 2u);
  EXPECT_EQ(executor.sampleCountAccumulations(), 1);
}

CUDAQ_TEST(MKLQMetalTester, MetalRuntimeGeneratesSampleCountsOnDevice) {
  nvqir::mklq::MetalStateVectorExecutor executor;

  if (!expectMetalRuntimeReadyOrUnavailable(executor))
    return;

  const std::array<double, 3> probabilities{0.25, 0.25, 0.5};
  constexpr std::uint64_t seed = 0x4d4b4c512d73616dULL;
  constexpr std::size_t shots = 256;
  std::array<std::uint32_t, 3> counts{0, 0, 0};

  ASSERT_TRUE(executor.accumulateGeneratedSampleCounts(
      probabilities.data(), probabilities.size(), seed, shots, counts.data(),
      counts.size()))
      << executor.lastError();

  const auto total = counts[0] + counts[1] + counts[2];
  EXPECT_EQ(total, shots);
  EXPECT_GT(counts[0], 0u);
  EXPECT_GT(counts[1], 0u);
  EXPECT_GT(counts[2], 0u);
  EXPECT_EQ(executor.sampleCountAccumulations(), 0);
  EXPECT_EQ(executor.generatedSampleCountAccumulations(), 1);
  EXPECT_EQ(executor.uniformGeneratedSampleCountAccumulations(), 0);
}

CUDAQ_TEST(MKLQMetalTester, MetalRuntimeGeneratesUniformSampleCountsOnDevice) {
  nvqir::mklq::MetalStateVectorExecutor executor;

  if (!expectMetalRuntimeReadyOrUnavailable(executor))
    return;

  const std::array<double, 4> probabilities{0.25, 0.25, 0.25, 0.25};
  constexpr std::uint64_t seed = 0x4d4b4c512d756e69ULL;
  constexpr std::size_t shots = 512;
  std::array<std::uint32_t, 4> counts{0, 0, 0, 0};

  ASSERT_TRUE(executor.accumulateGeneratedSampleCounts(
      probabilities.data(), probabilities.size(), seed, shots, counts.data(),
      counts.size()))
      << executor.lastError();

  const auto total = counts[0] + counts[1] + counts[2] + counts[3];
  EXPECT_EQ(total, shots);
  for (const auto count : counts)
    EXPECT_GT(count, 0u);
  EXPECT_EQ(executor.sampleCountAccumulations(), 0);
  EXPECT_EQ(executor.generatedSampleCountAccumulations(), 1);
  EXPECT_EQ(executor.uniformGeneratedSampleCountAccumulations(), 1);
}

CUDAQ_TEST(MKLQMetalTester, MetalRuntimeRejectsTargetsOutsideStateRange) {
  nvqir::mklq::MetalStateVectorExecutor executor;

  if (!expectMetalRuntimeReadyOrUnavailable(executor))
    return;

  std::vector<std::complex<double>> state{{1.0, 0.0}, {0.0, 0.0}};
  const std::array<std::complex<double>, 4> xGate{
      std::complex<double>{0.0, 0.0}, std::complex<double>{1.0, 0.0},
      std::complex<double>{1.0, 0.0}, std::complex<double>{0.0, 0.0}};
  const std::array<std::size_t, 1> invalidControls{2};
  const std::array<std::size_t, 1> overlappingControl{0};
  const std::array<std::size_t, 2> duplicateControls{1, 1};
  const std::array<std::complex<double>, 16> identityTwoQubitGate{
      std::complex<double>{1.0, 0.0}, std::complex<double>{0.0, 0.0},
      std::complex<double>{0.0, 0.0}, std::complex<double>{0.0, 0.0},
      std::complex<double>{0.0, 0.0}, std::complex<double>{1.0, 0.0},
      std::complex<double>{0.0, 0.0}, std::complex<double>{0.0, 0.0},
      std::complex<double>{0.0, 0.0}, std::complex<double>{0.0, 0.0},
      std::complex<double>{1.0, 0.0}, std::complex<double>{0.0, 0.0},
      std::complex<double>{0.0, 0.0}, std::complex<double>{0.0, 0.0},
      std::complex<double>{0.0, 0.0}, std::complex<double>{1.0, 0.0}};
  const std::array<std::size_t, 2> invalidTwoQubitTargets{0, 2};
  const std::array<std::size_t, 2> validTwoQubitTargets{0, 1};

  EXPECT_FALSE(executor.applySingleQubitGate(state.data(), state.size(),
                                             xGate.data(), nullptr, 0, 2));
  EXPECT_FALSE(executor.applySingleQubitGate(
      state.data(), state.size(), xGate.data(), invalidControls.data(),
      invalidControls.size(), 0));
  EXPECT_FALSE(executor.applySingleQubitGate(
      state.data(), state.size(), xGate.data(), overlappingControl.data(),
      overlappingControl.size(), 0));
  EXPECT_FALSE(executor.applySingleQubitGate(
      state.data(), state.size(), xGate.data(), duplicateControls.data(),
      duplicateControls.size(), 0));
  std::vector<std::complex<double>> twoQubitState{
      {1.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}};
  EXPECT_FALSE(executor.applyTwoQubitGate(
      twoQubitState.data(), twoQubitState.size(), identityTwoQubitGate.data(),
      nullptr, 0, invalidTwoQubitTargets.data()));
  EXPECT_FALSE(executor.applyTwoQubitGate(
      twoQubitState.data(), twoQubitState.size(), identityTwoQubitGate.data(),
      invalidControls.data(), invalidControls.size(),
      validTwoQubitTargets.data()));
  EXPECT_FALSE(executor.applyTwoQubitGate(
      twoQubitState.data(), twoQubitState.size(), identityTwoQubitGate.data(),
      overlappingControl.data(), overlappingControl.size(),
      validTwoQubitTargets.data()));
  EXPECT_FALSE(executor.applyTwoQubitGate(
      twoQubitState.data(), twoQubitState.size(), identityTwoQubitGate.data(),
      duplicateControls.data(), duplicateControls.size(),
      validTwoQubitTargets.data()));

  ASSERT_TRUE(executor.uploadState(state.data(), state.size()))
      << executor.lastError();
  EXPECT_FALSE(
      executor.applyResidentSingleQubitGate(xGate.data(), nullptr, 0, 2));
  EXPECT_FALSE(executor.applyResidentSingleQubitGate(
      xGate.data(), invalidControls.data(), invalidControls.size(), 0));
  EXPECT_FALSE(executor.applyResidentSingleQubitGate(
      xGate.data(), overlappingControl.data(), overlappingControl.size(), 0));
  EXPECT_FALSE(executor.applyResidentSingleQubitGate(
      xGate.data(), duplicateControls.data(), duplicateControls.size(), 0));

  ASSERT_TRUE(executor.uploadState(twoQubitState.data(), twoQubitState.size()))
      << executor.lastError();
  EXPECT_FALSE(executor.applyResidentTwoQubitGate(
      identityTwoQubitGate.data(), nullptr, 0, invalidTwoQubitTargets.data()));
  EXPECT_FALSE(executor.applyResidentTwoQubitGate(
      identityTwoQubitGate.data(), invalidControls.data(),
      invalidControls.size(), validTwoQubitTargets.data()));
  EXPECT_FALSE(executor.applyResidentTwoQubitGate(
      identityTwoQubitGate.data(), overlappingControl.data(),
      overlappingControl.size(), validTwoQubitTargets.data()));
  EXPECT_FALSE(executor.applyResidentTwoQubitGate(
      identityTwoQubitGate.data(), duplicateControls.data(),
      duplicateControls.size(), validTwoQubitTargets.data()));
}

CUDAQ_TEST(MKLQMetalTester, SimulatorUsesMetalFullRegisterProbabilityFill) {
  MklqMetalCircuitSimulatorTester sim;
  sim.setStateForTest({{3.0, 4.0}, {0.5, -0.5}, {0.0, 0.0}, {-2.0, 1.5}});

  const auto probabilities = sim.fullRegisterProbabilitiesForTest();

  ASSERT_EQ(probabilities.size(), 4);
  EXPECT_DOUBLE_EQ(probabilities[0], 25.0);
  EXPECT_DOUBLE_EQ(probabilities[1], 0.5);
  EXPECT_DOUBLE_EQ(probabilities[2], 0.0);
  EXPECT_DOUBLE_EQ(probabilities[3], 6.25);
  EXPECT_EQ(sim.probabilityFillApplicationsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);
}

CUDAQ_TEST(MKLQMetalTester,
           SimulatorKeepsSupportedGateSequenceResidentUntilReadback) {
  constexpr double invSqrt2 = 0.70710678118654752440;
  const std::vector<std::complex<double>> hGate{
      {invSqrt2, 0.0}, {invSqrt2, 0.0}, {invSqrt2, 0.0}, {-invSqrt2, 0.0}};
  const std::vector<std::complex<double>> xGate{{0.0, 0.0},
                                                {1.0, 0.0},
                                                {1.0, 0.0},
                                                {0.0, 0.0}};

  MklqMetalCircuitSimulatorTester sim;
  sim.setStateForTest(
      {{1.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}});
  sim.applySingleQubitGateForTest(hGate, {}, 0);
  sim.applySingleQubitGateForTest(xGate, {0}, 1);

  EXPECT_EQ(sim.residentGateCommandBufferSubmissionsForTest(), 0);
  const auto state = sim.stateVectorForTest();

  ASSERT_EQ(state.size(), 4);
  expectNear(state[0], {invSqrt2, 0.0});
  expectNear(state[1], {0.0, 0.0});
  expectNear(state[2], {0.0, 0.0});
  expectNear(state[3], {invSqrt2, 0.0});
  EXPECT_EQ(sim.singleQubitApplicationsForTest(),
            sim.metalRuntimeAvailableForTest() ? 2 : 0);
  EXPECT_EQ(sim.residentStateUploadsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);
  EXPECT_EQ(sim.residentStateDownloadsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);
  EXPECT_EQ(sim.residentGateCommandBufferSubmissionsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);
}

CUDAQ_TEST(MKLQMetalTester,
           SimulatorKeepsYAndControlledYResidentUntilReadback) {
  const std::vector<std::complex<double>> yGate{{0.0, 0.0},
                                                {0.0, -1.0},
                                                {0.0, 1.0},
                                                {0.0, 0.0}};

  MklqMetalCircuitSimulatorTester sim;
  sim.setStateForTest(
      {{1.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}});
  sim.applySingleQubitGateForTest(yGate, {}, 0);
  sim.applySingleQubitGateForTest(yGate, {0}, 1);

  EXPECT_EQ(sim.residentStateDownloadsForTest(), 0);
  const auto state = sim.stateVectorForTest();

  ASSERT_EQ(state.size(), 4);
  expectNear(state[0], {0.0, 0.0});
  expectNear(state[1], {0.0, 0.0});
  expectNear(state[2], {0.0, 0.0});
  expectNear(state[3], {-1.0, 0.0});
  EXPECT_EQ(sim.singleQubitApplicationsForTest(),
            sim.metalRuntimeAvailableForTest() ? 2 : 0);
  EXPECT_EQ(sim.residentStateUploadsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);
  EXPECT_EQ(sim.residentStateDownloadsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);
}

CUDAQ_TEST(MKLQMetalTester,
           SimulatorKeepsBuiltInYAndControlledYResidentUntilReadback) {
  MklqMetalCircuitSimulatorTester sim;
  sim.setStateForTest(
      {{1.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}});
  sim.y(0);
  sim.y({0}, 1);
  sim.flushGateQueue();

  EXPECT_EQ(sim.residentStateDownloadsForTest(), 0);
  const auto state = sim.stateVectorForTest();

  ASSERT_EQ(state.size(), 4);
  expectNear(state[0], {0.0, 0.0});
  expectNear(state[1], {0.0, 0.0});
  expectNear(state[2], {0.0, 0.0});
  expectNear(state[3], {-1.0, 0.0});
  EXPECT_EQ(sim.singleQubitApplicationsForTest(),
            sim.metalRuntimeAvailableForTest() ? 2 : 0);
  EXPECT_EQ(sim.residentStateUploadsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);
  EXPECT_EQ(sim.residentStateDownloadsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);
}

CUDAQ_TEST(MKLQMetalTester,
           SimulatorKeepsBuiltInRxAndControlledRxResidentUntilReadback) {
  constexpr double theta = 0.4375;
  constexpr double phi = -0.6875;
  const std::vector<std::complex<double>> initial{
      {0.125, -0.25}, {0.5, 0.375}, {-0.375, 0.25}, {0.75, -0.125}};

  auto rxMatrix = [](double angle) {
    const auto cosine = std::cos(angle / 2.0);
    const auto sine = std::sin(angle / 2.0);
    return std::array<std::complex<double>, 4>{
        std::complex<double>{cosine, 0.0},
        std::complex<double>{0.0, -sine},
        std::complex<double>{0.0, -sine},
        std::complex<double>{cosine, 0.0}};
  };

  auto expected = initial;
  applyExpectedSingleQubitGate(expected, rxMatrix(theta), {}, 0);
  applyExpectedSingleQubitGate(expected, rxMatrix(phi), {0}, 1);

  MklqMetalCircuitSimulatorTester sim;
  sim.setStateForTest(initial);
  sim.rx(theta, 0);
  sim.rx(phi, std::vector<std::size_t>{0}, 1);
  sim.flushGateQueue();

  EXPECT_EQ(sim.residentStateDownloadsForTest(), 0);
  const auto state = sim.stateVectorForTest();

  ASSERT_EQ(state.size(), expected.size());
  for (std::size_t index = 0; index < state.size(); ++index)
    expectNear(state[index], expected[index]);
  EXPECT_EQ(sim.singleQubitApplicationsForTest(),
            sim.metalRuntimeAvailableForTest() ? 2 : 0);
  EXPECT_EQ(sim.residentStateUploadsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);
  EXPECT_EQ(sim.residentStateDownloadsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);
}

CUDAQ_TEST(MKLQMetalTester,
           SimulatorKeepsBuiltInRyAndControlledRyResidentUntilReadback) {
  constexpr double theta = -0.3125;
  constexpr double phi = 0.59375;
  const std::vector<std::complex<double>> initial{
      {-0.25, 0.125}, {0.375, -0.5}, {0.625, 0.25}, {-0.125, -0.75}};

  auto ryMatrix = [](double angle) {
    const auto cosine = std::cos(angle / 2.0);
    const auto sine = std::sin(angle / 2.0);
    return std::array<std::complex<double>, 4>{
        std::complex<double>{cosine, 0.0},
        std::complex<double>{-sine, 0.0},
        std::complex<double>{sine, 0.0},
        std::complex<double>{cosine, 0.0}};
  };

  auto expected = initial;
  applyExpectedSingleQubitGate(expected, ryMatrix(theta), {}, 0);
  applyExpectedSingleQubitGate(expected, ryMatrix(phi), {0}, 1);

  MklqMetalCircuitSimulatorTester sim;
  sim.setStateForTest(initial);
  sim.ry(theta, 0);
  sim.ry(phi, std::vector<std::size_t>{0}, 1);
  sim.flushGateQueue();

  EXPECT_EQ(sim.residentStateDownloadsForTest(), 0);
  const auto state = sim.stateVectorForTest();

  ASSERT_EQ(state.size(), expected.size());
  for (std::size_t index = 0; index < state.size(); ++index)
    expectNear(state[index], expected[index]);
  EXPECT_EQ(sim.singleQubitApplicationsForTest(),
            sim.metalRuntimeAvailableForTest() ? 2 : 0);
  EXPECT_EQ(sim.residentStateUploadsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);
  EXPECT_EQ(sim.residentStateDownloadsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);
}

CUDAQ_TEST(MKLQMetalTester,
           SimulatorKeepsBuiltInRzAndControlledRzResidentUntilReadback) {
  constexpr double theta = 0.375;
  constexpr double phi = -0.8125;
  const std::vector<std::complex<double>> initial{
      {0.25, 0.5}, {-0.125, 0.375}, {0.5, -0.25}, {-0.75, 0.125}};

  auto rzPhase = [](double angle, bool oneBranch) {
    const auto phase = oneBranch ? angle / 2.0 : -angle / 2.0;
    return std::complex<double>{std::cos(phase), std::sin(phase)};
  };

  auto expected = initial;
  for (std::size_t index = 0; index < expected.size(); ++index) {
    const bool q0IsOne = (index & 1ULL) != 0;
    const bool q1IsOne = (index & 2ULL) != 0;
    expected[index] *= rzPhase(theta, q0IsOne);
    if (q0IsOne)
      expected[index] *= rzPhase(phi, q1IsOne);
  }

  MklqMetalCircuitSimulatorTester sim;
  sim.setStateForTest(initial);
  sim.rz(theta, 0);
  sim.rz(phi, std::vector<std::size_t>{0}, 1);
  sim.flushGateQueue();

  EXPECT_EQ(sim.residentStateDownloadsForTest(), 0);
  const auto state = sim.stateVectorForTest();

  ASSERT_EQ(state.size(), expected.size());
  for (std::size_t index = 0; index < state.size(); ++index)
    expectNear(state[index], expected[index]);
  EXPECT_EQ(sim.singleQubitApplicationsForTest(),
            sim.metalRuntimeAvailableForTest() ? 2 : 0);
  EXPECT_EQ(sim.residentStateUploadsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);
  EXPECT_EQ(sim.residentStateDownloadsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);
}

CUDAQ_TEST(MKLQMetalTester,
           SimulatorKeepsBuiltInPhaseFamilyResidentUntilReadback) {
  const auto pi = std::acos(-1.0);
  const std::vector<std::complex<double>> initial{
      {0.3125, -0.125}, {-0.25, 0.4375}, {0.625, 0.25}, {-0.5, -0.375}};

  auto phaseMatrix = [](double angle) {
    return std::array<std::complex<double>, 4>{
        std::complex<double>{1.0, 0.0},
        std::complex<double>{0.0, 0.0},
        std::complex<double>{0.0, 0.0},
        std::complex<double>{std::cos(angle), std::sin(angle)}};
  };

  auto expected = initial;
  applyExpectedSingleQubitGate(expected, phaseMatrix(pi / 2.0), {}, 0);
  applyExpectedSingleQubitGate(expected, phaseMatrix(pi / 4.0), {0}, 1);
  applyExpectedSingleQubitGate(expected, phaseMatrix(-pi / 2.0), {}, 1);
  applyExpectedSingleQubitGate(expected, phaseMatrix(-pi / 4.0), {1}, 0);

  MklqMetalCircuitSimulatorTester sim;
  sim.setStateForTest(initial);
  sim.s(0);
  sim.t(std::vector<std::size_t>{0}, 1);
  sim.sdg(1);
  sim.tdg(std::vector<std::size_t>{1}, 0);
  sim.flushGateQueue();

  EXPECT_EQ(sim.residentStateDownloadsForTest(), 0);
  const auto state = sim.stateVectorForTest();

  ASSERT_EQ(state.size(), expected.size());
  for (std::size_t index = 0; index < state.size(); ++index)
    expectNear(state[index], expected[index]);
  EXPECT_EQ(sim.singleQubitApplicationsForTest(),
            sim.metalRuntimeAvailableForTest() ? 4 : 0);
  EXPECT_EQ(sim.residentStateUploadsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);
  EXPECT_EQ(sim.residentStateDownloadsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);
}

CUDAQ_TEST(MKLQMetalTester,
           SimulatorKeepsMultiControlSingleQubitResidentUntilReadback) {
  constexpr double theta = 0.53125;
  const std::vector<std::complex<double>> initial{
      {0.25, -0.125}, {-0.375, 0.5},  {0.625, -0.25}, {-0.125, -0.75},
      {0.5, 0.375},   {-0.25, 0.125}, {0.75, -0.5},   {-0.625, 0.25}};
  const std::array<std::complex<double>, 4> xMatrix{
      std::complex<double>{0.0, 0.0}, std::complex<double>{1.0, 0.0},
      std::complex<double>{1.0, 0.0}, std::complex<double>{0.0, 0.0}};

  auto rzMatrix = [](double angle) {
    const auto zeroPhase = -angle / 2.0;
    const auto onePhase = angle / 2.0;
    return std::array<std::complex<double>, 4>{
        std::complex<double>{std::cos(zeroPhase), std::sin(zeroPhase)},
        std::complex<double>{0.0, 0.0},
        std::complex<double>{0.0, 0.0},
        std::complex<double>{std::cos(onePhase), std::sin(onePhase)}};
  };

  auto expected = initial;
  applyExpectedSingleQubitGate(expected, xMatrix, {0, 1}, 2);
  applyExpectedSingleQubitGate(expected, rzMatrix(theta), {0, 2}, 1);

  MklqMetalCircuitSimulatorTester sim;
  sim.setStateForTest(initial);
  sim.x(std::vector<std::size_t>{0, 1}, 2);
  sim.rz(theta, std::vector<std::size_t>{0, 2}, 1);
  sim.flushGateQueue();

  EXPECT_EQ(sim.residentStateDownloadsForTest(), 0);
  const auto state = sim.stateVectorForTest();

  ASSERT_EQ(state.size(), expected.size());
  for (std::size_t index = 0; index < state.size(); ++index)
    expectNear(state[index], expected[index]);
  EXPECT_EQ(sim.singleQubitApplicationsForTest(),
            sim.metalRuntimeAvailableForTest() ? 2 : 0);
  EXPECT_EQ(sim.residentStateUploadsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);
  EXPECT_EQ(sim.residentStateDownloadsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);
}

CUDAQ_TEST(MKLQMetalTester,
           SimulatorSamplesResidentDenseStateWithoutReadback) {
  constexpr std::size_t qubitCount = 7;
  constexpr std::size_t dimension = 1ULL << qubitCount;
  constexpr double invSqrt2 = 0.70710678118654752440;
  const std::vector<std::complex<double>> hGate{
      {invSqrt2, 0.0}, {invSqrt2, 0.0}, {invSqrt2, 0.0}, {-invSqrt2, 0.0}};

  MklqMetalCircuitSimulatorTester sim;
  std::vector<std::complex<double>> state(dimension, {0.0, 0.0});
  state[0] = {1.0, 0.0};
  sim.setStateForTest(std::move(state));

  std::vector<std::size_t> qubits;
  qubits.reserve(qubitCount);
  for (std::size_t qubit = 0; qubit < qubitCount; ++qubit) {
    qubits.push_back(qubit);
    sim.applySingleQubitGateForTest(hGate, {}, qubit);
  }

  const auto counts = sim.sampleQubitsForTest(qubits, 8);
  std::size_t totalShots = 0;
  for (const auto &[bits, count] : counts.counts)
    totalShots += count;

  EXPECT_EQ(totalShots, 8);
  EXPECT_EQ(sim.singleQubitApplicationsForTest(),
            sim.metalRuntimeAvailableForTest() ? qubitCount : 0);
  EXPECT_EQ(sim.residentStateUploadsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);
  EXPECT_EQ(sim.residentStateDownloadsForTest(), 0);
  EXPECT_EQ(sim.probabilityFillApplicationsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);
}

CUDAQ_TEST(MKLQMetalTester,
           SimulatorSamplesResidentFullRegisterWithHostSequentialDrawTelemetry) {
  constexpr std::size_t qubitCount = 7;
  constexpr std::size_t dimension = 1ULL << qubitCount;
  constexpr int shots = 32;
  constexpr double invSqrt2 = 0.70710678118654752440;
  const std::vector<std::complex<double>> hGate{
      {invSqrt2, 0.0}, {invSqrt2, 0.0}, {invSqrt2, 0.0}, {-invSqrt2, 0.0}};

  MklqMetalCircuitSimulatorTester sim;
  std::vector<std::complex<double>> state(dimension, {0.0, 0.0});
  state[0] = {1.0, 0.0};
  sim.setStateForTest(std::move(state));

  std::vector<std::size_t> qubits;
  qubits.reserve(qubitCount);
  for (std::size_t qubit = 0; qubit < qubitCount; ++qubit) {
    qubits.push_back(qubit);
    sim.applySingleQubitGateForTest(hGate, {}, qubit);
  }

  const auto counts = sim.sampleQubitsForTest(qubits, shots);

  std::size_t totalShots = 0;
  for (const auto &[bits, count] : counts.counts)
    totalShots += count;
  EXPECT_EQ(totalShots, shots);
  EXPECT_EQ(counts.sequentialData.size(), shots);
  EXPECT_EQ(sim.singleQubitApplicationsForTest(),
            sim.metalRuntimeAvailableForTest() ? qubitCount : 0);
  EXPECT_EQ(sim.residentStateUploadsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);
  EXPECT_EQ(sim.residentStateDownloadsForTest(), 0);
  EXPECT_EQ(sim.marginalProbabilityApplicationsForTest(), 0);
  EXPECT_EQ(sim.probabilityFillApplicationsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);
  EXPECT_EQ(sim.sequentialSampleDrawBatchesForTest(), 1);
  EXPECT_EQ(sim.countsOnlySampleDrawBatchesForTest(), 0);
  EXPECT_EQ(sim.sampleExpectationReductionsForTest(), 1);
  EXPECT_GT(sim.sampleProbabilityFillSecondsForTest(), 0.0);
  EXPECT_GT(sim.sampleDrawAndCountSecondsForTest(), 0.0);
  EXPECT_GT(sim.sampleExpectationReductionSecondsForTest(), 0.0);
}

CUDAQ_TEST(MKLQMetalTester,
           SimulatorSamplesResidentFullRegisterCountsOnlyWithMetalAccumulation) {
  constexpr std::size_t qubitCount = 7;
  constexpr std::size_t dimension = 1ULL << qubitCount;
  constexpr int shots = 32;
  constexpr double invSqrt2 = 0.70710678118654752440;
  const std::vector<std::complex<double>> hGate{
      {invSqrt2, 0.0}, {invSqrt2, 0.0}, {invSqrt2, 0.0}, {-invSqrt2, 0.0}};

  MklqMetalCircuitSimulatorTester sim;
  std::vector<std::complex<double>> state(dimension, {0.0, 0.0});
  state[0] = {1.0, 0.0};
  sim.setStateForTest(std::move(state));

  std::vector<std::size_t> qubits;
  qubits.reserve(qubitCount);
  for (std::size_t qubit = 0; qubit < qubitCount; ++qubit) {
    qubits.push_back(qubit);
    sim.applySingleQubitGateForTest(hGate, {}, qubit);
  }

  const auto counts =
      sim.sampleQubitsWithoutSequentialDataForTest(qubits, shots);

  std::size_t totalShots = 0;
  for (const auto &[bits, count] : counts.counts)
    totalShots += count;
  EXPECT_EQ(totalShots, shots);
  EXPECT_TRUE(counts.sequentialData.empty());
  EXPECT_EQ(sim.residentStateDownloadsForTest(), 0);
  EXPECT_EQ(sim.probabilityFillApplicationsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);
  EXPECT_EQ(sim.sampleCountAccumulationsForTest(),
            0);
  EXPECT_EQ(sim.generatedSampleCountAccumulationsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);
  EXPECT_EQ(sim.uniformGeneratedSampleCountAccumulationsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);
  EXPECT_EQ(sim.countsOnlySampleDrawBatchesForTest(),
            sim.metalRuntimeAvailableForTest() ? 0 : 1);
  EXPECT_EQ(sim.sequentialSampleDrawBatchesForTest(), 0);
  EXPECT_EQ(sim.denseDrawCountBuffersForTest(),
            sim.metalRuntimeAvailableForTest() ? 0 : 1);
  EXPECT_EQ(sim.sparseDrawCountMapsForTest(), 0);
  EXPECT_EQ(sim.sampleExpectationReductionsForTest(), 1);
  EXPECT_GT(sim.sampleProbabilityFillSecondsForTest(), 0.0);
  EXPECT_GT(sim.sampleDrawAndCountSecondsForTest(), 0.0);
  EXPECT_GT(sim.sampleExpectationReductionSecondsForTest(), 0.0);
}

CUDAQ_TEST(MKLQMetalTester,
           SimulatorSamplesLargeResidentPartialRegisterThroughFullProbability) {
  constexpr std::size_t qubitCount = 16;
  constexpr std::size_t dimension = 1ULL << qubitCount;
  const std::vector<std::complex<double>> xGate{{0.0, 0.0},
                                                {1.0, 0.0},
                                                {1.0, 0.0},
                                                {0.0, 0.0}};

  MklqMetalCircuitSimulatorTester sim;
  std::vector<std::complex<double>> state(dimension, {0.0, 0.0});
  state[0] = {1.0, 0.0};
  sim.setStateForTest(std::move(state));
  sim.applySingleQubitGateForTest(xGate, {}, 0);
  sim.applySingleQubitGateForTest(xGate, {}, 14);

  const std::vector<std::size_t> measuredQubits{0, 2, 4, 6, 8, 10, 12, 14};
  const auto counts = sim.sampleQubitsForTest(measuredQubits, 8);

  ASSERT_EQ(counts.counts.size(), 1);
  ASSERT_TRUE(counts.counts.contains("10000001"));
  EXPECT_EQ(counts.counts.at("10000001"), 8);
  EXPECT_EQ(sim.singleQubitApplicationsForTest(),
            sim.metalRuntimeAvailableForTest() ? 2 : 0);
  EXPECT_EQ(sim.residentStateUploadsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);
  EXPECT_EQ(sim.residentStateDownloadsForTest(), 0);
  EXPECT_EQ(sim.marginalProbabilityApplicationsForTest(), 0);
  EXPECT_EQ(sim.probabilityFillApplicationsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);
}

CUDAQ_TEST(MKLQMetalTester,
           SimulatorSamplesSmallResidentPartialRegisterThroughMarginalProbability) {
  constexpr std::size_t qubitCount = 7;
  constexpr std::size_t dimension = 1ULL << qubitCount;
  const std::vector<std::complex<double>> xGate{{0.0, 0.0},
                                                {1.0, 0.0},
                                                {1.0, 0.0},
                                                {0.0, 0.0}};

  MklqMetalCircuitSimulatorTester sim;
  std::vector<std::complex<double>> state(dimension, {0.0, 0.0});
  state[0] = {1.0, 0.0};
  sim.setStateForTest(std::move(state));
  sim.applySingleQubitGateForTest(xGate, {}, 0);
  sim.applySingleQubitGateForTest(xGate, {}, 4);

  const std::vector<std::size_t> measuredQubits{0, 2, 4};
  const auto counts = sim.sampleQubitsForTest(measuredQubits, 8);

  ASSERT_EQ(counts.counts.size(), 1);
  ASSERT_TRUE(counts.counts.contains("101"));
  EXPECT_EQ(counts.counts.at("101"), 8);
  EXPECT_EQ(sim.singleQubitApplicationsForTest(),
            sim.metalRuntimeAvailableForTest() ? 2 : 0);
  EXPECT_EQ(sim.residentStateUploadsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);
  EXPECT_EQ(sim.residentStateDownloadsForTest(), 0);
  EXPECT_EQ(sim.marginalProbabilityApplicationsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);
  EXPECT_EQ(sim.probabilityFillApplicationsForTest(), 0);
}

CUDAQ_TEST(MKLQMetalTester,
           SimulatorSamplesRequestedOrderPartialRegisterThroughMarginalProbability) {
  constexpr std::size_t qubitCount = 7;
  constexpr std::size_t dimension = 1ULL << qubitCount;
  const std::vector<std::complex<double>> xGate{{0.0, 0.0},
                                                {1.0, 0.0},
                                                {1.0, 0.0},
                                                {0.0, 0.0}};

  MklqMetalCircuitSimulatorTester sim;
  std::vector<std::complex<double>> state(dimension, {0.0, 0.0});
  state[0] = {1.0, 0.0};
  sim.setStateForTest(std::move(state));
  sim.applySingleQubitGateForTest(xGate, {}, 0);
  sim.applySingleQubitGateForTest(xGate, {}, 4);

  const std::vector<std::size_t> measuredQubits{4, 0, 2};
  const auto counts = sim.sampleQubitsForTest(measuredQubits, 16);

  ASSERT_EQ(counts.counts.size(), 1);
  ASSERT_TRUE(counts.counts.contains("110"));
  EXPECT_EQ(counts.counts.at("110"), 16);
  EXPECT_EQ(counts.sequentialData.size(), 16);
  EXPECT_EQ(sim.singleQubitApplicationsForTest(),
            sim.metalRuntimeAvailableForTest() ? 2 : 0);
  EXPECT_EQ(sim.residentStateUploadsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);
  EXPECT_EQ(sim.residentStateDownloadsForTest(), 0);
  EXPECT_EQ(sim.marginalProbabilityApplicationsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);
  EXPECT_EQ(sim.probabilityFillApplicationsForTest(), 0);
}

CUDAQ_TEST(MKLQMetalTester,
           SimulatorSamplesResidentPartialRegisterWithHostSequentialDrawTelemetry) {
  constexpr std::size_t qubitCount = 7;
  constexpr std::size_t dimension = 1ULL << qubitCount;
  constexpr int shots = 16;
  constexpr double invSqrt2 = 0.70710678118654752440;
  const std::vector<std::complex<double>> xGate{{0.0, 0.0},
                                                {1.0, 0.0},
                                                {1.0, 0.0},
                                                {0.0, 0.0}};
  const std::vector<std::complex<double>> hGate{
      {invSqrt2, 0.0}, {invSqrt2, 0.0}, {invSqrt2, 0.0}, {-invSqrt2, 0.0}};

  MklqMetalCircuitSimulatorTester sim;
  std::vector<std::complex<double>> state(dimension, {0.0, 0.0});
  state[0] = {1.0, 0.0};
  sim.setStateForTest(std::move(state));
  sim.applySingleQubitGateForTest(xGate, {}, 0);
  sim.applySingleQubitGateForTest(xGate, {}, 4);
  sim.applySingleQubitGateForTest(hGate, {}, 2);

  const auto counts = sim.sampleQubitsForTest({4, 0, 2}, shots);

  std::size_t totalShots = 0;
  for (const auto &[bits, count] : counts.counts)
    totalShots += count;
  EXPECT_EQ(totalShots, shots);
  EXPECT_FALSE(counts.counts.empty());
  EXPECT_EQ(counts.sequentialData.size(), shots);
  EXPECT_EQ(sim.singleQubitApplicationsForTest(),
            sim.metalRuntimeAvailableForTest() ? 3 : 0);
  EXPECT_EQ(sim.residentStateUploadsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);
  EXPECT_EQ(sim.residentStateDownloadsForTest(), 0);
  EXPECT_EQ(sim.marginalProbabilityApplicationsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);
  EXPECT_EQ(sim.probabilityFillApplicationsForTest(), 0);
  EXPECT_EQ(sim.sequentialSampleDrawBatchesForTest(), 1);
  EXPECT_EQ(sim.countsOnlySampleDrawBatchesForTest(), 0);
  EXPECT_EQ(sim.sampleExpectationReductionsForTest(), 1);
}

CUDAQ_TEST(
    MKLQMetalTester,
    SimulatorSamplesResidentDeterministicPartialRegisterSequentialWithoutDrawLoop) {
  constexpr std::size_t qubitCount = 7;
  constexpr std::size_t dimension = 1ULL << qubitCount;
  constexpr int shots = 4096;
  const std::vector<std::complex<double>> xGate{{0.0, 0.0},
                                                {1.0, 0.0},
                                                {1.0, 0.0},
                                                {0.0, 0.0}};

  MklqMetalCircuitSimulatorTester sim;
  std::vector<std::complex<double>> state(dimension, {0.0, 0.0});
  state[0] = {1.0, 0.0};
  sim.setStateForTest(std::move(state));
  sim.applySingleQubitGateForTest(xGate, {}, 0);
  sim.applySingleQubitGateForTest(xGate, {}, 4);

  const auto counts = sim.sampleQubitsForTest({4, 0, 2}, shots);

  ASSERT_EQ(counts.counts.size(), 1);
  ASSERT_TRUE(counts.counts.contains("110"));
  EXPECT_EQ(counts.counts.at("110"), shots);
  ASSERT_EQ(counts.sequentialData.size(), shots);
  for (const auto &bits : counts.sequentialData)
    EXPECT_EQ(bits, "110");
  EXPECT_EQ(sim.singleQubitApplicationsForTest(),
            sim.metalRuntimeAvailableForTest() ? 2 : 0);
  EXPECT_EQ(sim.residentStateUploadsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);
  EXPECT_EQ(sim.residentStateDownloadsForTest(), 0);
  EXPECT_EQ(sim.marginalProbabilityApplicationsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);
  EXPECT_EQ(sim.probabilityFillApplicationsForTest(), 0);
  EXPECT_EQ(sim.sequentialSampleDrawBatchesForTest(), 0);
  EXPECT_EQ(sim.countsOnlySampleDrawBatchesForTest(), 0);
  EXPECT_EQ(sim.sampleExpectationReductionsForTest(), 1);
  EXPECT_EQ(sim.bitStringConversionsForTest(), 1);
}

CUDAQ_TEST(
    MKLQMetalTester,
    SimulatorSamplesResidentPartialRegisterCountsOnlyWithMetalAccumulation) {
  constexpr std::size_t qubitCount = 7;
  constexpr std::size_t dimension = 1ULL << qubitCount;
  constexpr int shots = 16;
  constexpr double invSqrt2 = 0.70710678118654752440;
  const std::vector<std::complex<double>> xGate{{0.0, 0.0},
                                                {1.0, 0.0},
                                                {1.0, 0.0},
                                                {0.0, 0.0}};
  const std::vector<std::complex<double>> hGate{
      {invSqrt2, 0.0}, {invSqrt2, 0.0}, {invSqrt2, 0.0}, {-invSqrt2, 0.0}};

  MklqMetalCircuitSimulatorTester sim;
  std::vector<std::complex<double>> state(dimension, {0.0, 0.0});
  state[0] = {1.0, 0.0};
  sim.setStateForTest(std::move(state));
  sim.applySingleQubitGateForTest(xGate, {}, 0);
  sim.applySingleQubitGateForTest(xGate, {}, 4);
  sim.applySingleQubitGateForTest(hGate, {}, 2);

  const auto counts =
      sim.sampleQubitsWithoutSequentialDataForTest({4, 0, 2}, shots);

  std::size_t totalShots = 0;
  for (const auto &[bits, count] : counts.counts)
    totalShots += count;
  EXPECT_EQ(totalShots, shots);
  EXPECT_FALSE(counts.counts.empty());
  EXPECT_TRUE(counts.sequentialData.empty());
  EXPECT_EQ(sim.singleQubitApplicationsForTest(),
            sim.metalRuntimeAvailableForTest() ? 3 : 0);
  EXPECT_EQ(sim.residentStateUploadsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);
  EXPECT_EQ(sim.residentStateDownloadsForTest(), 0);
  EXPECT_EQ(sim.marginalProbabilityApplicationsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);
  EXPECT_EQ(sim.probabilityFillApplicationsForTest(), 0);
  EXPECT_EQ(sim.sampleCountAccumulationsForTest(),
            0);
  EXPECT_EQ(sim.generatedSampleCountAccumulationsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);
  EXPECT_EQ(sim.countsOnlySampleDrawBatchesForTest(),
            sim.metalRuntimeAvailableForTest() ? 0 : 1);
  EXPECT_EQ(sim.sequentialSampleDrawBatchesForTest(), 0);
  EXPECT_EQ(sim.sampleExpectationReductionsForTest(), 1);
  EXPECT_EQ(sim.denseDrawCountBuffersForTest(),
            sim.metalRuntimeAvailableForTest() ? 0 : 1);
  EXPECT_EQ(sim.sparseDrawCountMapsForTest(), 0);
}

CUDAQ_TEST(
    MKLQMetalTester,
    SimulatorSamplesResidentDeterministicPartialRegisterCountsOnlyWithoutDrawLoop) {
  constexpr std::size_t qubitCount = 7;
  constexpr std::size_t dimension = 1ULL << qubitCount;
  constexpr int shots = 4096;
  const std::vector<std::complex<double>> xGate{{0.0, 0.0},
                                                {1.0, 0.0},
                                                {1.0, 0.0},
                                                {0.0, 0.0}};

  MklqMetalCircuitSimulatorTester sim;
  std::vector<std::complex<double>> state(dimension, {0.0, 0.0});
  state[0] = {1.0, 0.0};
  sim.setStateForTest(std::move(state));
  sim.applySingleQubitGateForTest(xGate, {}, 0);
  sim.applySingleQubitGateForTest(xGate, {}, 4);

  const auto counts =
      sim.sampleQubitsWithoutSequentialDataForTest({4, 0, 2}, shots);

  ASSERT_EQ(counts.counts.size(), 1);
  ASSERT_TRUE(counts.counts.contains("110"));
  EXPECT_EQ(counts.counts.at("110"), shots);
  EXPECT_TRUE(counts.sequentialData.empty());
  EXPECT_EQ(sim.singleQubitApplicationsForTest(),
            sim.metalRuntimeAvailableForTest() ? 2 : 0);
  EXPECT_EQ(sim.residentStateUploadsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);
  EXPECT_EQ(sim.residentStateDownloadsForTest(), 0);
  EXPECT_EQ(sim.marginalProbabilityApplicationsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);
  EXPECT_EQ(sim.probabilityFillApplicationsForTest(), 0);
  EXPECT_EQ(sim.countsOnlySampleDrawBatchesForTest(), 0);
  EXPECT_EQ(sim.sequentialSampleDrawBatchesForTest(), 0);
  EXPECT_EQ(sim.denseDrawCountBuffersForTest(), 0);
  EXPECT_EQ(sim.sparseDrawCountMapsForTest(), 0);
  EXPECT_EQ(sim.sampleExpectationReductionsForTest(), 1);
}

CUDAQ_TEST(MKLQMetalTester,
           SimulatorSamplesResidentPartialRegisterReportsNativePhaseTiming) {
  constexpr std::size_t qubitCount = 7;
  constexpr std::size_t dimension = 1ULL << qubitCount;
  constexpr int shots = 2048;
  constexpr double invSqrt2 = 0.70710678118654752440;
  const std::vector<std::complex<double>> xGate{{0.0, 0.0},
                                                {1.0, 0.0},
                                                {1.0, 0.0},
                                                {0.0, 0.0}};
  const std::vector<std::complex<double>> hGate{
      {invSqrt2, 0.0}, {invSqrt2, 0.0}, {invSqrt2, 0.0}, {-invSqrt2, 0.0}};

  MklqMetalCircuitSimulatorTester sim;
  std::vector<std::complex<double>> state(dimension, {0.0, 0.0});
  state[0] = {1.0, 0.0};
  sim.setStateForTest(std::move(state));
  sim.applySingleQubitGateForTest(xGate, {}, 0);
  sim.applySingleQubitGateForTest(xGate, {}, 4);
  sim.applySingleQubitGateForTest(hGate, {}, 2);

  const auto counts =
      sim.sampleQubitsWithoutSequentialDataForTest({4, 0, 2}, shots);

  std::size_t totalShots = 0;
  for (const auto &[bits, count] : counts.counts)
    totalShots += count;
  EXPECT_EQ(totalShots, shots);
  EXPECT_FALSE(counts.counts.empty());
  EXPECT_TRUE(counts.sequentialData.empty());
  EXPECT_EQ(sim.marginalProbabilityApplicationsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);
  EXPECT_EQ(sim.probabilityFillApplicationsForTest(), 0);
  EXPECT_EQ(sim.sampleCountAccumulationsForTest(),
            0);
  EXPECT_EQ(sim.generatedSampleCountAccumulationsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);
  EXPECT_EQ(sim.countsOnlySampleDrawBatchesForTest(),
            sim.metalRuntimeAvailableForTest() ? 0 : 1);
  EXPECT_EQ(sim.sequentialSampleDrawBatchesForTest(), 0);
  EXPECT_EQ(sim.sampleExpectationReductionsForTest(), 1);
  EXPECT_GT(sim.sampleProbabilityFillSecondsForTest(), 0.0);
  EXPECT_GT(sim.sampleDrawAndCountSecondsForTest(), 0.0);
  EXPECT_GT(sim.sampleExpectationReductionSecondsForTest(), 0.0);
}

CUDAQ_TEST(MKLQMetalTester,
           SamplingPhaseProfileEmitsNativeTimingForExternalProbe) {
  const auto config = mklq_test::samplingPhaseProfileConfigFromEnvironment();
  if (!config)
    GTEST_SKIP() << "set MKLQ_ENABLE_SAMPLING_PHASE_PROFILE=1 to profile";

  const auto dimension = 1ULL << config->qubitCount;
  const auto amplitude = 1.0 / std::sqrt(static_cast<double>(dimension));
  std::vector<std::complex<double>> state(
      dimension, std::complex<double>{amplitude, 0.0});
  std::vector<std::size_t> measuredQubits(config->measuredQubitCount);
  std::iota(measuredQubits.begin(), measuredQubits.end(), 0);
  const std::vector<std::complex<double>> zGate{
      {1.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {-1.0, 0.0}};

  MklqMetalCircuitSimulatorTester sim;
  sim.setStateForTest(std::move(state));
  sim.applySingleQubitGateForTest(zGate, {}, 0);
  if (!sim.metalRuntimeAvailableForTest())
    GTEST_SKIP() << "Metal runtime is unavailable on this machine";

  const auto counts = sim.sampleQubitsWithoutSequentialDataForTest(
      measuredQubits, config->shots);

  std::size_t totalShots = 0;
  for (const auto &[bits, count] : counts.counts)
    totalShots += count;
  EXPECT_EQ(totalShots, static_cast<std::size_t>(config->shots));
  EXPECT_GT(sim.sampleProbabilityFillSecondsForTest(), 0.0);
  EXPECT_GT(sim.sampleFullRegisterProbabilityBufferPreparationSecondsForTest(),
            0.0);
  EXPECT_GT(sim.sampleDrawAndCountSecondsForTest(), 0.0);
  EXPECT_GT(sim.sampleExpectationReductionSecondsForTest(), 0.0);
  EXPECT_EQ(sim.probabilityFillApplicationsForTest() +
                sim.marginalProbabilityApplicationsForTest(),
            1);
  recordSamplingPhaseProfile(*config, sim);
}

CUDAQ_TEST(MKLQMetalTester,
           LargeStateCostGateAvoidsResidentMarginalProbabilityScratch) {
  MklqMetalCircuitSimulatorTester sim;
  sim.setStateForTest(std::vector<std::complex<double>>(1ULL << 19));
  EXPECT_FALSE(sim.shouldUseResidentMarginalProbabilitiesForTest(16));

  sim.setStateForTest(std::vector<std::complex<double>>(1ULL << 18));
  EXPECT_TRUE(sim.shouldUseResidentMarginalProbabilitiesForTest(16));
}

CUDAQ_TEST(MKLQMetalTester,
           SimulatorSamplesDeterministicSparseStateWithOneBitStringConversion) {
  std::vector<std::complex<double>> state(8, {0.0, 0.0});
  state[5] = {1.0, 0.0};

  MklqMetalCircuitSimulatorTester sim;
  sim.setStateForTest(std::move(state));

  constexpr int shots = 32;
  const auto counts = sim.sampleQubitsForTest({0, 1, 2}, shots);

  ASSERT_EQ(counts.counts.size(), 1);
  ASSERT_TRUE(counts.counts.contains("101"));
  EXPECT_EQ(counts.counts.at("101"), shots);
  EXPECT_EQ(counts.sequentialData.size(), shots);
  EXPECT_EQ(sim.bitStringConversionsForTest(), 1);
  EXPECT_EQ(sim.sequentialSampleDrawBatchesForTest(), 0);
  EXPECT_EQ(sim.countsOnlySampleDrawBatchesForTest(), 0);
  EXPECT_EQ(sim.probabilityFillApplicationsForTest(), 0);
  EXPECT_EQ(sim.residentStateDownloadsForTest(), 0);
}

CUDAQ_TEST(MKLQMetalTester,
           SimulatorKeepsFourQubitGateResidentUntilReadback) {
  constexpr double invSqrt2 = 0.70710678118654752440;
  const std::vector<std::complex<double>> hGate{
      {invSqrt2, 0.0}, {invSqrt2, 0.0}, {invSqrt2, 0.0}, {-invSqrt2, 0.0}};
  std::vector<std::complex<double>> flipFirstTargetGate(256, {0.0, 0.0});
  for (std::size_t column = 0; column < 16; ++column)
    flipFirstTargetGate[(column ^ 1) * 16 + column] = {1.0, 0.0};

  MklqMetalCircuitSimulatorTester sim;
  std::vector<std::complex<double>> state(16, {0.0, 0.0});
  state[0] = {1.0, 0.0};
  sim.setStateForTest(std::move(state));

  sim.applySingleQubitGateForTest(hGate, {}, 0);
  sim.applyGateTaskForTest("flip-first-target4", flipFirstTargetGate, {},
                           {2, 0, 3, 1});
  EXPECT_EQ(sim.residentStateDownloadsForTest(), 0);
  EXPECT_EQ(sim.metalCpuFallbackApplicationsForTest(), 0);
  EXPECT_EQ(sim.fourQubitApplicationsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);
  const auto output = sim.stateVectorForTest();

  ASSERT_EQ(output.size(), 16);
  for (std::size_t index = 0; index < output.size(); ++index)
    expectNear(output[index],
               index == 4 || index == 5
                   ? std::complex<double>{invSqrt2, 0.0}
                   : std::complex<double>{0.0, 0.0});
  EXPECT_EQ(sim.residentStateDownloadsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);
}

CUDAQ_TEST(MKLQMetalTester, SimulatorKeepsControlledFourQubitGateResident) {
  std::vector<std::complex<double>> flipAllGate(256, {0.0, 0.0});
  for (std::size_t row = 0; row < 16; ++row)
    flipAllGate[row * 16 + (15 - row)] = {1.0, 0.0};

  MklqMetalCircuitSimulatorTester sim;
  std::vector<std::complex<double>> state(32, {0.0, 0.0});
  state[16] = {1.0, 0.0};
  sim.setStateForTest(std::move(state));

  sim.applyGateTaskForTest("flip4", flipAllGate, {4}, {0, 1, 2, 3});

  EXPECT_EQ(sim.residentStateDownloadsForTest(), 0);
  EXPECT_EQ(sim.metalCpuFallbackApplicationsForTest(), 0);
  EXPECT_EQ(sim.fourQubitApplicationsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);

  const auto output = sim.stateVectorForTest();
  ASSERT_EQ(output.size(), 32);
  for (std::size_t index = 0; index < output.size(); ++index)
    expectNear(output[index], index == 31 ? std::complex<double>{1.0, 0.0}
                                          : std::complex<double>{0.0, 0.0});
  EXPECT_EQ(sim.residentStateDownloadsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);
}

CUDAQ_TEST(MKLQMetalTester, SimulatorAppliesDenseFourQubitGateResident) {
  std::vector<std::complex<double>> hadamardTensor(256, {0.0, 0.0});
  for (std::size_t row = 0; row < 16; ++row)
    for (std::size_t column = 0; column < 16; ++column)
      hadamardTensor[row * 16 + column] = std::polar(
          0.25, 0.17 * static_cast<double>(std::popcount(row)) +
                    ((std::popcount(row & column) % 2) == 0
                         ? 0.0
                         : 3.14159265358979323846));

  MklqMetalCircuitSimulatorTester sim;
  std::vector<std::complex<double>> state(16, {0.0, 0.0});
  for (std::size_t column = 0; column < state.size(); ++column)
    state[column] = std::polar(1.0 + 0.1 * static_cast<double>(column),
                               0.11 * static_cast<double>(column));
  double norm = 0.0;
  for (const auto amplitude : state)
    norm += std::norm(amplitude);
  for (auto &amplitude : state)
    amplitude /= std::sqrt(norm);

  std::vector<std::complex<double>> expected(16, {0.0, 0.0});
  for (std::size_t row = 0; row < 16; ++row)
    for (std::size_t column = 0; column < 16; ++column)
      expected[row] += hadamardTensor[row * 16 + column] * state[column];
  sim.setStateForTest(std::move(state));

  sim.applyGateTaskForTest("dense-h4", hadamardTensor, {}, {0, 1, 2, 3});
  EXPECT_EQ(sim.residentStateDownloadsForTest(), 0);
  EXPECT_EQ(sim.metalCpuFallbackApplicationsForTest(), 0);
  EXPECT_EQ(sim.fourQubitApplicationsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);

  const auto output = sim.stateVectorForTest();
  ASSERT_EQ(output.size(), 16);
  for (std::size_t row = 0; row < output.size(); ++row)
    expectNear(output[row], expected[row]);
  EXPECT_EQ(sim.residentStateDownloadsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);
}

CUDAQ_TEST(MKLQMetalTester,
           SimulatorKeepsThreeQubitGateResidentUntilReadback) {
  constexpr double invSqrt2 = 0.70710678118654752440;
  const std::vector<std::complex<double>> hGate{
      {invSqrt2, 0.0}, {invSqrt2, 0.0}, {invSqrt2, 0.0}, {-invSqrt2, 0.0}};
  const std::vector<std::complex<double>> xGate{{0.0, 0.0},
                                                {1.0, 0.0},
                                                {1.0, 0.0},
                                                {0.0, 0.0}};
  const auto identityThreeQubit = identityGateForTargets(3);

  MklqMetalCircuitSimulatorTester sim;
  std::vector<std::complex<double>> state(8, {0.0, 0.0});
  state[0] = {1.0, 0.0};
  sim.setStateForTest(std::move(state));

  sim.applySingleQubitGateForTest(hGate, {}, 0);
  ASSERT_EQ(sim.residentStateUploadsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);
  ASSERT_EQ(sim.residentStateDownloadsForTest(), 0);
  ASSERT_EQ(sim.metalCpuFallbackApplicationsForTest(), 0);

  sim.applyGateTaskForTest("identity3", identityThreeQubit, {}, {0, 1, 2});
  EXPECT_EQ(sim.residentStateDownloadsForTest(), 0);
  EXPECT_EQ(sim.metalCpuFallbackApplicationsForTest(), 0);
  EXPECT_EQ(sim.threeQubitApplicationsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);

  sim.applySingleQubitGateForTest(xGate, {}, 1);
  EXPECT_EQ(sim.residentStateUploadsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);
  EXPECT_EQ(sim.singleQubitApplicationsForTest(),
            sim.metalRuntimeAvailableForTest() ? 2 : 0);
  EXPECT_EQ(sim.residentStateDownloadsForTest(), 0);

  const auto output = sim.stateVectorForTest();

  ASSERT_EQ(output.size(), 8);
  expectNear(output[0], {0.0, 0.0});
  expectNear(output[1], {0.0, 0.0});
  expectNear(output[2], {invSqrt2, 0.0});
  expectNear(output[3], {invSqrt2, 0.0});
  for (std::size_t index = 4; index < output.size(); ++index)
    expectNear(output[index], {0.0, 0.0});
  EXPECT_EQ(sim.residentStateDownloadsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);
}

CUDAQ_TEST(MKLQMetalTester,
           SimulatorKeepsBuiltInControlledSwapResidentUntilReadback) {
  const std::vector<std::complex<double>> swapGate{
      {1.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0},
      {0.0, 0.0}, {0.0, 0.0}, {1.0, 0.0}, {0.0, 0.0},
      {0.0, 0.0}, {1.0, 0.0}, {0.0, 0.0}, {0.0, 0.0},
      {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {1.0, 0.0},
  };

  MklqMetalCircuitSimulatorTester sim;
  std::vector<std::complex<double>> state(8, {0.0, 0.0});
  state[3] = {1.0, 0.0};
  sim.setStateForTest(std::move(state));

  sim.applyGateTaskForTest("swap", swapGate, {0}, {1, 2}, true);

  EXPECT_EQ(sim.twoQubitApplicationsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);
  EXPECT_EQ(sim.residentStateUploadsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);
  EXPECT_EQ(sim.residentStateDownloadsForTest(), 0);
  EXPECT_EQ(sim.metalCpuFallbackApplicationsForTest(), 0);

  const auto output = sim.stateVectorForTest();

  ASSERT_EQ(output.size(), 8);
  for (std::size_t index = 0; index < output.size(); ++index)
    expectNear(output[index], index == 5 ? std::complex<double>{1.0, 0.0}
                                         : std::complex<double>{0.0, 0.0});
  EXPECT_EQ(sim.residentStateDownloadsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);
}

CUDAQ_TEST(MKLQMetalTester,
           SimulatorReuploadsResidentStateAfterFiveQubitGateFallback) {
  constexpr double invSqrt2 = 0.70710678118654752440;
  const std::vector<std::complex<double>> hGate{
      {invSqrt2, 0.0}, {invSqrt2, 0.0}, {invSqrt2, 0.0}, {-invSqrt2, 0.0}};
  const std::vector<std::complex<double>> xGate{{0.0, 0.0},
                                                {1.0, 0.0},
                                                {1.0, 0.0},
                                                {0.0, 0.0}};
  const auto identityFiveQubit = identityGateForTargets(5);

  MklqMetalCircuitSimulatorTester sim;
  std::vector<std::complex<double>> state(32, {0.0, 0.0});
  state[0] = {1.0, 0.0};
  sim.setStateForTest(std::move(state));

  sim.applySingleQubitGateForTest(hGate, {}, 0);
  ASSERT_EQ(sim.residentStateUploadsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);
  ASSERT_EQ(sim.residentStateDownloadsForTest(), 0);
  ASSERT_EQ(sim.metalCpuFallbackApplicationsForTest(), 0);

  sim.applyGateTaskForTest("identity5", identityFiveQubit, {},
                           {0, 1, 2, 3, 4});
  EXPECT_EQ(sim.residentStateDownloadsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);
  EXPECT_EQ(sim.metalCpuFallbackApplicationsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);

  sim.applySingleQubitGateForTest(xGate, {}, 1);
  EXPECT_EQ(sim.residentStateUploadsForTest(),
            sim.metalRuntimeAvailableForTest() ? 2 : 0);
  EXPECT_EQ(sim.singleQubitApplicationsForTest(),
            sim.metalRuntimeAvailableForTest() ? 2 : 0);
  EXPECT_EQ(sim.residentStateDownloadsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);

  const auto output = sim.stateVectorForTest();

  ASSERT_EQ(output.size(), 32);
  expectNear(output[0], {0.0, 0.0});
  expectNear(output[1], {0.0, 0.0});
  expectNear(output[2], {invSqrt2, 0.0});
  expectNear(output[3], {invSqrt2, 0.0});
  for (std::size_t index = 4; index < output.size(); ++index)
    expectNear(output[index], {0.0, 0.0});
  EXPECT_EQ(sim.residentStateDownloadsForTest(),
            sim.metalRuntimeAvailableForTest() ? 2 : 0);
}

CUDAQ_TEST(MKLQMetalTester,
           SimulatorMeasuresAndResetsResidentStateWithoutReadback) {
  const std::vector<std::complex<double>> xGate{{0.0, 0.0},
                                                {1.0, 0.0},
                                                {1.0, 0.0},
                                                {0.0, 0.0}};

  MklqMetalCircuitSimulatorTester measured;
  measured.setStateForTest({{1.0, 0.0}, {0.0, 0.0}});
  measured.applySingleQubitGateForTest(xGate, {}, 0);
  EXPECT_TRUE(measured.measureQubitForTest(0));
  EXPECT_EQ(measured.residentStateDownloadsForTest(),
            0);
  EXPECT_EQ(measured.measurementProbabilityApplicationsForTest(),
            measured.metalRuntimeAvailableForTest() ? 1 : 0);
  EXPECT_EQ(measured.measurementProbabilityReductionApplicationsForTest(),
            measured.metalRuntimeAvailableForTest() ? 1 : 0);
  EXPECT_EQ(measured.measurementCollapseApplicationsForTest(),
            measured.metalRuntimeAvailableForTest() ? 1 : 0);

  MklqMetalCircuitSimulatorTester reset;
  reset.setStateForTest({{1.0, 0.0}, {0.0, 0.0}});
  reset.applySingleQubitGateForTest(xGate, {}, 0);
  reset.resetQubit(0);
  EXPECT_EQ(reset.residentStateDownloadsForTest(), 0);
  EXPECT_EQ(reset.measurementProbabilityApplicationsForTest(),
            reset.metalRuntimeAvailableForTest() ? 1 : 0);
  EXPECT_EQ(reset.measurementProbabilityReductionApplicationsForTest(),
            reset.metalRuntimeAvailableForTest() ? 1 : 0);
  EXPECT_EQ(reset.measurementCollapseApplicationsForTest(),
            reset.metalRuntimeAvailableForTest() ? 1 : 0);
  const auto output = reset.stateVectorForTest();

  ASSERT_EQ(output.size(), 2);
  expectNear(output[0], {1.0, 0.0});
  expectNear(output[1], {0.0, 0.0});
  EXPECT_EQ(reset.residentStateDownloadsForTest(),
            reset.metalRuntimeAvailableForTest() ? 1 : 0);
}

CUDAQ_TEST(MKLQMetalTester,
           SimulatorResetsResidentNonzeroTargetWithoutReadback) {
  constexpr double invSqrt2 = 0.70710678118654752440;
  const std::vector<std::complex<double>> hGate{
      {invSqrt2, 0.0}, {invSqrt2, 0.0}, {invSqrt2, 0.0}, {-invSqrt2, 0.0}};

  MklqMetalCircuitSimulatorTester reset;
  reset.setStateForTest(
      {{0.0, 0.0}, {0.0, 0.0}, {1.0, 0.0}, {0.0, 0.0}});
  reset.applySingleQubitGateForTest(hGate, {}, 0);
  reset.resetQubit(1);

  EXPECT_EQ(reset.residentStateDownloadsForTest(), 0);
  EXPECT_EQ(reset.measurementProbabilityApplicationsForTest(),
            reset.metalRuntimeAvailableForTest() ? 1 : 0);
  EXPECT_EQ(reset.measurementCollapseApplicationsForTest(),
            reset.metalRuntimeAvailableForTest() ? 1 : 0);

  const auto output = reset.stateVectorForTest();
  ASSERT_EQ(output.size(), 4);
  expectNear(output[0], {invSqrt2, 0.0});
  expectNear(output[1], {invSqrt2, 0.0});
  expectNear(output[2], {0.0, 0.0});
  expectNear(output[3], {0.0, 0.0});
  EXPECT_EQ(reset.residentStateDownloadsForTest(),
            reset.metalRuntimeAvailableForTest() ? 1 : 0);
}

CUDAQ_TEST(MKLQMetalTester,
           SimulatorPoisonsResidentStateWhenSingleGateFails) {
  const std::vector<std::complex<double>> xGate{{0.0, 0.0},
                                                {1.0, 0.0},
                                                {1.0, 0.0},
                                                {0.0, 0.0}};

  MklqMetalCircuitSimulatorTester sim;
  if (!sim.metalRuntimeAvailableForTest())
    return;

  sim.setStateForTest({{1.0, 0.0}, {0.0, 0.0}});
  sim.applySingleQubitGateForTest(xGate, {}, 0);
  ASSERT_EQ(sim.residentStateUploadsForTest(), 1);
  ASSERT_EQ(sim.singleQubitApplicationsForTest(), 1);
  ASSERT_EQ(sim.residentStateDownloadsForTest(), 0);
  sim.setResidentFailureModeForTest(ResidentFailureMode::SingleGate);

  try {
    sim.applySingleQubitGateForTest(xGate, {}, 0);
  } catch (const std::runtime_error &error) {
    EXPECT_NE(std::string(error.what()).find(
                  "failed to apply resident Metal single-qubit gate"),
              std::string::npos)
        << error.what();
    EXPECT_EQ(sim.residentStateDownloadsForTest(), 0);
  } catch (...) {
    FAIL() << "expected runtime_error from resident single-gate failure";
  }

  try {
    (void)sim.stateVectorForTest();
  } catch (const std::runtime_error &error) {
    EXPECT_NE(std::string(error.what()).find(
                  "unrecoverable Metal resident state"),
              std::string::npos)
        << error.what();
    EXPECT_EQ(sim.residentStateDownloadsForTest(), 0);
    return;
  }

  FAIL() << "expected resident single-gate failure to poison readback";
}

CUDAQ_TEST(MKLQMetalTester,
           SimulatorPoisonsResidentStateWhenTwoGateFails) {
  const std::vector<std::complex<double>> xGate{{0.0, 0.0},
                                                {1.0, 0.0},
                                                {1.0, 0.0},
                                                {0.0, 0.0}};
  const std::vector<std::complex<double>> swapGate{
      {1.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0},
      {0.0, 0.0}, {0.0, 0.0}, {1.0, 0.0}, {0.0, 0.0},
      {0.0, 0.0}, {1.0, 0.0}, {0.0, 0.0}, {0.0, 0.0},
      {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {1.0, 0.0}};

  MklqMetalCircuitSimulatorTester sim;
  if (!sim.metalRuntimeAvailableForTest())
    return;

  sim.setStateForTest(
      {{1.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}});
  sim.applySingleQubitGateForTest(xGate, {}, 0);
  ASSERT_EQ(sim.residentStateUploadsForTest(), 1);
  ASSERT_EQ(sim.singleQubitApplicationsForTest(), 1);
  ASSERT_EQ(sim.residentStateDownloadsForTest(), 0);
  sim.setResidentFailureModeForTest(ResidentFailureMode::TwoGate);

  try {
    sim.applyGateTaskForTest("swap", swapGate, {}, {0, 1});
  } catch (const std::runtime_error &error) {
    EXPECT_NE(std::string(error.what()).find(
                  "failed to apply resident Metal two-qubit gate"),
              std::string::npos)
        << error.what();
    EXPECT_EQ(sim.residentStateDownloadsForTest(), 0);
  } catch (...) {
    FAIL() << "expected runtime_error from resident two-gate failure";
  }

  try {
    (void)sim.stateVectorForTest();
  } catch (const std::runtime_error &error) {
    EXPECT_NE(std::string(error.what()).find(
                  "unrecoverable Metal resident state"),
              std::string::npos)
        << error.what();
    EXPECT_EQ(sim.residentStateDownloadsForTest(), 0);
    return;
  }

  FAIL() << "expected resident two-gate failure to poison readback";
}

CUDAQ_TEST(MKLQMetalTester,
           SimulatorPoisonsResidentStateWhenThreeGateFails) {
  const std::vector<std::complex<double>> xGate{{0.0, 0.0},
                                                {1.0, 0.0},
                                                {1.0, 0.0},
                                                {0.0, 0.0}};
  const auto identityThreeQubit = identityGateForTargets(3);

  MklqMetalCircuitSimulatorTester sim;
  if (!sim.metalRuntimeAvailableForTest())
    return;

  std::vector<std::complex<double>> state(8, {0.0, 0.0});
  state[0] = {1.0, 0.0};
  sim.setStateForTest(std::move(state));
  sim.applySingleQubitGateForTest(xGate, {}, 0);
  ASSERT_EQ(sim.residentStateUploadsForTest(), 1);
  ASSERT_EQ(sim.singleQubitApplicationsForTest(), 1);
  ASSERT_EQ(sim.residentStateDownloadsForTest(), 0);
  sim.setResidentFailureModeForTest(ResidentFailureMode::ThreeGate);

  try {
    sim.applyGateTaskForTest("identity3", identityThreeQubit, {}, {0, 1, 2});
  } catch (const std::runtime_error &error) {
    EXPECT_NE(std::string(error.what()).find(
                  "failed to apply resident Metal three-qubit gate"),
              std::string::npos)
        << error.what();
    EXPECT_EQ(sim.residentStateDownloadsForTest(), 0);
  } catch (...) {
    FAIL() << "expected runtime_error from resident three-gate failure";
  }

  try {
    (void)sim.stateVectorForTest();
  } catch (const std::runtime_error &error) {
    EXPECT_NE(std::string(error.what()).find(
                  "unrecoverable Metal resident state"),
              std::string::npos)
        << error.what();
    EXPECT_EQ(sim.residentStateDownloadsForTest(), 0);
    return;
  }

  FAIL() << "expected resident three-gate failure to poison readback";
}

CUDAQ_TEST(MKLQMetalTester,
           SimulatorPoisonsResidentStateWhenFourGateFails) {
  const std::vector<std::complex<double>> xGate{{0.0, 0.0},
                                                {1.0, 0.0},
                                                {1.0, 0.0},
                                                {0.0, 0.0}};
  const auto identityFourQubit = identityGateForTargets(4);

  MklqMetalCircuitSimulatorTester sim;
  if (!sim.metalRuntimeAvailableForTest())
    return;

  std::vector<std::complex<double>> state(16, {0.0, 0.0});
  state[0] = {1.0, 0.0};
  sim.setStateForTest(std::move(state));
  sim.applySingleQubitGateForTest(xGate, {}, 0);
  ASSERT_EQ(sim.residentStateUploadsForTest(), 1);
  ASSERT_EQ(sim.singleQubitApplicationsForTest(), 1);
  ASSERT_EQ(sim.residentStateDownloadsForTest(), 0);
  sim.setResidentFailureModeForTest(ResidentFailureMode::FourGate);

  try {
    sim.applyGateTaskForTest("identity4", identityFourQubit, {},
                             {0, 1, 2, 3});
  } catch (const std::runtime_error &error) {
    EXPECT_NE(std::string(error.what()).find(
                  "failed to apply resident Metal four-qubit gate"),
              std::string::npos)
        << error.what();
    EXPECT_EQ(sim.residentStateDownloadsForTest(), 0);
  } catch (...) {
    FAIL() << "expected runtime_error from resident four-gate failure";
  }

  try {
    (void)sim.stateVectorForTest();
  } catch (const std::runtime_error &error) {
    EXPECT_NE(std::string(error.what()).find(
                  "unrecoverable Metal resident state"),
              std::string::npos)
        << error.what();
    EXPECT_EQ(sim.residentStateDownloadsForTest(), 0);
    return;
  }

  FAIL() << "expected resident four-gate failure to poison readback";
}

CUDAQ_TEST(MKLQMetalTester,
           SimulatorThrowsWhenResidentMeasurementProbabilityFails) {
  const std::vector<std::complex<double>> xGate{{0.0, 0.0},
                                                {1.0, 0.0},
                                                {1.0, 0.0},
                                                {0.0, 0.0}};

  MklqMetalCircuitSimulatorTester sim;
  if (!sim.metalRuntimeAvailableForTest())
    return;

  sim.setStateForTest({{1.0, 0.0}, {0.0, 0.0}});
  sim.applySingleQubitGateForTest(xGate, {}, 0);
  ASSERT_EQ(sim.residentStateUploadsForTest(), 1);
  ASSERT_EQ(sim.singleQubitApplicationsForTest(), 1);
  ASSERT_EQ(sim.residentStateDownloadsForTest(), 0);
  sim.setResidentFailureModeForTest(ResidentFailureMode::Probability);

  bool threwProbabilityFailure = false;
  try {
    (void)sim.measureQubitForTest(0);
  } catch (const std::runtime_error &error) {
    threwProbabilityFailure = true;
    EXPECT_NE(std::string(error.what()).find(
                  "failed to compute Metal resident measurement probability"),
              std::string::npos)
        << error.what();
    EXPECT_EQ(sim.residentStateDownloadsForTest(), 0);
  }

  ASSERT_TRUE(threwProbabilityFailure)
      << "expected resident measurement probability failure to throw";
  const auto state = sim.stateVectorForTest();
  ASSERT_EQ(state.size(), 2);
  expectNear(state[0], {0.0, 0.0});
  expectNear(state[1], {1.0, 0.0});
  EXPECT_EQ(sim.residentStateDownloadsForTest(), 1);
}

CUDAQ_TEST(MKLQMetalTester,
           SimulatorThrowsWhenResidentMeasurementCollapseFails) {
  const std::vector<std::complex<double>> xGate{{0.0, 0.0},
                                                {1.0, 0.0},
                                                {1.0, 0.0},
                                                {0.0, 0.0}};

  MklqMetalCircuitSimulatorTester sim;
  if (!sim.metalRuntimeAvailableForTest())
    return;

  sim.setStateForTest({{1.0, 0.0}, {0.0, 0.0}});
  sim.applySingleQubitGateForTest(xGate, {}, 0);
  ASSERT_EQ(sim.residentStateUploadsForTest(), 1);
  ASSERT_EQ(sim.singleQubitApplicationsForTest(), 1);
  ASSERT_EQ(sim.residentStateDownloadsForTest(), 0);
  sim.setResidentFailureModeForTest(ResidentFailureMode::Collapse);

  try {
    (void)sim.measureQubitForTest(0);
  } catch (const std::runtime_error &error) {
    EXPECT_NE(std::string(error.what()).find(
                  "failed to collapse Metal resident measurement branch"),
              std::string::npos)
        << error.what();
    EXPECT_EQ(sim.residentStateDownloadsForTest(), 0);
  } catch (...) {
    FAIL() << "expected runtime_error from resident collapse failure";
  }

  try {
    (void)sim.stateVectorForTest();
  } catch (const std::runtime_error &error) {
    EXPECT_NE(std::string(error.what()).find(
                  "unrecoverable Metal resident state"),
              std::string::npos)
        << error.what();
    EXPECT_EQ(sim.residentStateDownloadsForTest(), 0);
    return;
  }

  FAIL() << "expected resident collapse failure to poison readback";
}

CUDAQ_TEST(MKLQMetalTester, SimulatorThrowsWhenResidentResetGateFails) {
  const std::vector<std::complex<double>> xGate{{0.0, 0.0},
                                                {1.0, 0.0},
                                                {1.0, 0.0},
                                                {0.0, 0.0}};

  MklqMetalCircuitSimulatorTester sim;
  if (!sim.metalRuntimeAvailableForTest())
    return;

  sim.setStateForTest({{1.0, 0.0}, {0.0, 0.0}});
  sim.applySingleQubitGateForTest(xGate, {}, 0);
  ASSERT_EQ(sim.residentStateUploadsForTest(), 1);
  ASSERT_EQ(sim.singleQubitApplicationsForTest(), 1);
  ASSERT_EQ(sim.residentStateDownloadsForTest(), 0);
  sim.setResidentFailureModeForTest(ResidentFailureMode::Reset);

  try {
    sim.resetQubit(0);
  } catch (const std::runtime_error &error) {
    EXPECT_NE(std::string(error.what()).find(
                  "failed to reset Metal resident qubit"),
              std::string::npos)
        << error.what();
    EXPECT_EQ(sim.residentStateDownloadsForTest(), 0);
  } catch (...) {
    FAIL() << "expected runtime_error from resident reset failure";
  }

  try {
    (void)sim.stateVectorForTest();
  } catch (const std::runtime_error &error) {
    EXPECT_NE(std::string(error.what()).find(
                  "unrecoverable Metal resident state"),
              std::string::npos)
        << error.what();
    EXPECT_EQ(sim.residentStateDownloadsForTest(), 0);
    return;
  }

  FAIL() << "expected resident reset failure to poison readback";
}

CUDAQ_TEST(MKLQMetalTester,
           SimulatorComputesZeroShotZParityExpectationFromResidentState) {
  const std::vector<std::complex<double>> xGate{{0.0, 0.0},
                                                {1.0, 0.0},
                                                {1.0, 0.0},
                                                {0.0, 0.0}};

  MklqMetalCircuitSimulatorTester sim;
  sim.setStateForTest({{1.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0},
                       {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}});
  sim.applySingleQubitGateForTest(xGate, {}, 0);
  sim.applySingleQubitGateForTest(xGate, {}, 2);

  if (!sim.metalRuntimeAvailableForTest())
    return;

  const auto fullRegister = sim.sampleQubitsForTest({0, 1, 2}, 0);
  const auto partialRegister = sim.sampleQubitsForTest({2, 0}, 0);
  const auto singleQubit = sim.sampleQubitsForTest({0}, 0);

  ASSERT_TRUE(fullRegister.expectationValue.has_value());
  ASSERT_TRUE(partialRegister.expectationValue.has_value());
  ASSERT_TRUE(singleQubit.expectationValue.has_value());
  EXPECT_NEAR(*fullRegister.expectationValue, 1.0, 1.0e-12);
  EXPECT_NEAR(*partialRegister.expectationValue, 1.0, 1.0e-12);
  EXPECT_NEAR(*singleQubit.expectationValue, -1.0, 1.0e-12);
  EXPECT_EQ(sim.residentStateDownloadsForTest(), 0);
  EXPECT_EQ(sim.probabilityFillApplicationsForTest(), 0);
  EXPECT_EQ(sim.marginalProbabilityApplicationsForTest(), 0);
}

CUDAQ_TEST(MKLQMetalTester,
           SimulatorPoisonsResidentStateWhenExpectationFlushFails) {
  const std::vector<std::complex<double>> xGate{{0.0, 0.0},
                                                {1.0, 0.0},
                                                {1.0, 0.0},
                                                {0.0, 0.0}};

  MklqMetalCircuitSimulatorTester sim;
  if (!sim.metalRuntimeAvailableForTest())
    return;

  sim.setStateForTest({{1.0, 0.0}, {0.0, 0.0}});
  sim.applySingleQubitGateForTest(xGate, {}, 0);
  ASSERT_EQ(sim.residentStateUploadsForTest(), 1);
  ASSERT_EQ(sim.residentStateDownloadsForTest(), 0);
  sim.setResidentFailureModeForTest(ResidentFailureMode::ExpectationFlush);

  try {
    (void)sim.sampleQubitsForTest({0}, 0);
  } catch (const std::runtime_error &error) {
    EXPECT_NE(std::string(error.what()).find(
                  "failed to flush Metal resident gates before computing "
                  "expectation"),
              std::string::npos)
        << error.what();
    EXPECT_EQ(sim.residentStateDownloadsForTest(), 0);
  } catch (...) {
    FAIL() << "expected runtime_error from resident expectation flush failure";
  }

  try {
    (void)sim.stateVectorForTest();
  } catch (const std::runtime_error &error) {
    EXPECT_NE(std::string(error.what()).find(
                  "unrecoverable Metal resident state"),
              std::string::npos)
        << error.what();
    EXPECT_EQ(sim.residentStateDownloadsForTest(), 0);
    return;
  }

  FAIL() << "expected resident expectation flush failure to poison readback";
}

CUDAQ_TEST(MKLQMetalTester,
           SimulatorFallsBackToCpuWhenResidentExpectationReadFails) {
  const std::vector<std::complex<double>> xGate{{0.0, 0.0},
                                                {1.0, 0.0},
                                                {1.0, 0.0},
                                                {0.0, 0.0}};

  MklqMetalCircuitSimulatorTester sim;
  if (!sim.metalRuntimeAvailableForTest())
    return;

  sim.setStateForTest({{1.0, 0.0}, {0.0, 0.0}});
  sim.applySingleQubitGateForTest(xGate, {}, 0);
  sim.setResidentFailureModeForTest(ResidentFailureMode::ExpectationRead);

  const auto counts = sim.sampleQubitsForTest({0}, 0);
  ASSERT_TRUE(counts.expectationValue.has_value());
  EXPECT_NEAR(*counts.expectationValue, -1.0, 1.0e-12);
  EXPECT_EQ(sim.residentStateDownloadsForTest(), 1);
  EXPECT_EQ(sim.probabilityFillApplicationsForTest(), 0);
  EXPECT_EQ(sim.marginalProbabilityApplicationsForTest(), 0);
}

CUDAQ_TEST(MKLQMetalTester,
           SimulatorReducesLargeNonuniformZeroShotExpectationOnMetalWithoutProbabilityFill) {
  constexpr std::size_t qubitCount = 17;
  constexpr std::size_t dimension = 1ULL << qubitCount;
  constexpr std::size_t measuredQubit = qubitCount - 1;

  std::vector<std::complex<double>> state(dimension, {0.0, 0.0});
  state[0] = {std::sqrt(0.30), 0.0};
  state[2] = {std::sqrt(0.20), 0.0};
  state[4] = {std::sqrt(0.125), 0.0};
  state[dimension / 2] = {std::sqrt(0.10), 0.0};
  state[dimension / 2 + 2] = {std::sqrt(0.15), 0.0};
  state[dimension / 2 + 4] = {std::sqrt(0.125), 0.0};
  const auto expected = zParityExpectationCpuOracle(state, {measuredQubit});
  EXPECT_NEAR(expected, 0.25, 1.0e-12);

  const std::vector<std::complex<double>> xGate{{0.0, 0.0},
                                                {1.0, 0.0},
                                                {1.0, 0.0},
                                                {0.0, 0.0}};
  MklqMetalCircuitSimulatorTester sim;
  sim.setStateForTest(std::move(state));
  sim.applySingleQubitGateForTest(xGate, {}, 0);
  sim.applySingleQubitGateForTest(xGate, {}, 0);

  if (!sim.metalRuntimeAvailableForTest())
    return;

  const auto counts = sim.sampleQubitsForTest({measuredQubit}, 0);

  ASSERT_TRUE(counts.expectationValue.has_value());
  EXPECT_NEAR(*counts.expectationValue, expected, 1.0e-5);
  EXPECT_EQ(sim.residentStateUploadsForTest(), 1);
  EXPECT_EQ(sim.residentStateDownloadsForTest(), 0);
  EXPECT_EQ(sim.probabilityFillApplicationsForTest(), 0);
  EXPECT_EQ(sim.marginalProbabilityApplicationsForTest(), 0);
}

CUDAQ_TEST(MKLQMetalTester,
           SimulatorSamplesDenseFullRegisterThroughMetalProbabilityFill) {
  constexpr std::size_t qubitCount = 7;
  constexpr std::size_t dimension = 1ULL << qubitCount;
  const double amplitude = 1.0 / std::sqrt(static_cast<double>(dimension));

  std::vector<std::complex<double>> state(dimension, {amplitude, 0.0});

  MklqMetalCircuitSimulatorTester sim;
  sim.setStateForTest(std::move(state));

  std::vector<std::size_t> qubits;
  qubits.reserve(qubitCount);
  for (std::size_t qubit = 0; qubit < qubitCount; ++qubit)
    qubits.push_back(qubit);

  const auto counts = sim.sampleQubitsForTest(qubits, 8);
  std::size_t totalShots = 0;
  for (const auto &[bits, count] : counts.counts)
    totalShots += count;

  EXPECT_EQ(totalShots, 8);
  EXPECT_EQ(sim.probabilityFillApplicationsForTest(),
            sim.metalRuntimeAvailableForTest() ? 1 : 0);
}
