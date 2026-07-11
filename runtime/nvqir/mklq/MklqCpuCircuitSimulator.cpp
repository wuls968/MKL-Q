/*******************************************************************************
 * Copyright (c) 2022 - 2026 NVIDIA Corporation & Affiliates.                  *
 * All rights reserved.                                                        *
 *                                                                             *
 * This source code and the accompanying materials are made available under    *
 * the terms of the Apache License 2.0 which accompanies this distribution.    *
 ******************************************************************************/

#include "common/FmtCore.h"
#include "nvqir/CircuitSimulator.h"

#if defined(MKLQ_ENABLE_METAL_RUNTIME)
#include "MklqMetalRuntime.h"
#endif

#include <algorithm>
#include <array>
#include <bit>
#include <chrono>
#include <cmath>
#include <complex>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <optional>
#include <ostream>
#include <random>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#if defined(_OPENMP)
#include <omp.h>
#endif

using namespace cudaq;

#ifndef MKLQ_SIMULATOR_BACKEND_NAME
#define MKLQ_SIMULATOR_BACKEND_NAME "mklq_cpu"
#endif

#ifndef MKLQ_SIMULATOR_CLASS
#define MKLQ_SIMULATOR_CLASS MklqCpuCircuitSimulator
#endif

#ifndef MKLQ_SIMULATOR_PRINTED_NAME
#define MKLQ_SIMULATOR_PRINTED_NAME mklq_cpu
#endif

#ifndef MKLQ_SIMULATOR_DIAGNOSTIC_PREFIX
#define MKLQ_SIMULATOR_DIAGNOSTIC_PREFIX "[mklq-cpu]"
#endif

#ifndef MKLQ_SIMULATOR_STATE_DIAGNOSTIC_PREFIX
#define MKLQ_SIMULATOR_STATE_DIAGNOSTIC_PREFIX "[mklq-cpu-state]"
#endif

namespace nvqir {
namespace {

using complexd = std::complex<double>;

std::size_t checkedNumQubits(std::size_t dimension, std::string_view context) {
  if (dimension == 0 || !std::has_single_bit(dimension))
    throw std::runtime_error(
        fmt::format(MKLQ_SIMULATOR_DIAGNOSTIC_PREFIX
                    " {} requires a non-zero power-of-two state dimension.",
                    context));

  return std::countr_zero(dimension);
}

std::size_t basisStateToIndex(const std::vector<int> &basisState) {
  return std::accumulate(
      std::make_reverse_iterator(basisState.end()),
      std::make_reverse_iterator(basisState.begin()), 0ull,
      [](std::size_t acc, int bit) { return (acc << 1) + bit; });
}

double validateProbabilityWeights(std::span<const double> probabilities,
                                  std::string_view context) {
  double sum = 0.0;
  for (const auto probability : probabilities) {
    if (!std::isfinite(probability) || probability < 0.0)
      throw std::runtime_error(fmt::format(
          MKLQ_SIMULATOR_DIAGNOSTIC_PREFIX
          " {} encountered a non-finite probability weight.",
          context));
    sum += probability;
  }

  if (!(sum > 0.0) || !std::isfinite(sum))
    throw std::runtime_error(fmt::format(
        MKLQ_SIMULATOR_DIAGNOSTIC_PREFIX
        " {} cannot sample a zero-norm probability distribution.",
        context));

  return sum;
}

struct MklqCpuState : public cudaq::SimulationState {
  std::vector<complexd> state;

  MklqCpuState() = default;
  explicit MklqCpuState(std::vector<complexd> data) : state(std::move(data)) {}

  MklqCpuState(const std::vector<std::size_t> &shape,
               const std::vector<complexd> &data) {
    if (shape.size() != 1)
      throw std::runtime_error(MKLQ_SIMULATOR_STATE_DIAGNOSTIC_PREFIX
                               " expected a one-dimensional state vector.");
    if (shape[0] != data.size())
      throw std::runtime_error(MKLQ_SIMULATOR_STATE_DIAGNOSTIC_PREFIX
                               " shape does not match state data size.");
    checkedNumQubits(data.size(), "state construction");
    state = data;
  }

  std::size_t getNumQubits() const override {
    return checkedNumQubits(state.size(), "getNumQubits");
  }

  complexd overlap(const cudaq::SimulationState &other) override {
    if (other.getNumTensors() != 1 ||
        other.getTensor().extents != getTensor().extents ||
        other.getTensor().fp_precision != getPrecision())
      throw std::runtime_error(MKLQ_SIMULATOR_STATE_DIAGNOSTIC_PREFIX
                               " overlap dimension or "
                               "precision mismatch.");

    std::span<complexd> otherState(
        reinterpret_cast<complexd *>(other.getTensor().data),
        other.getTensor().extents[0]);
    return std::abs(std::inner_product(
        state.begin(), state.end(), otherState.begin(), complexd{0.0, 0.0},
        [](auto a, auto b) { return a + b; },
        [](auto a, auto b) { return a * std::conj(b); }));
  }

  complexd getAmplitude(const std::vector<int> &basisState) override {
    if (getNumQubits() != basisState.size())
      throw std::runtime_error(
          fmt::format(MKLQ_SIMULATOR_STATE_DIAGNOSTIC_PREFIX
                      " invalid basis state width: expected {}, got {}.",
                      getNumQubits(), basisState.size()));
    if (std::any_of(basisState.begin(), basisState.end(),
                    [](int bit) { return bit != 0 && bit != 1; }))
      throw std::runtime_error(MKLQ_SIMULATOR_STATE_DIAGNOSTIC_PREFIX
                               " basis states must contain only 0 or 1 bits.");

    return state[basisStateToIndex(basisState)];
  }

  Tensor getTensor(std::size_t tensorIdx = 0) const override {
    if (tensorIdx != 0)
      throw std::runtime_error(MKLQ_SIMULATOR_STATE_DIAGNOSTIC_PREFIX
                               " invalid tensor index.");
    return Tensor{
        reinterpret_cast<void *>(const_cast<complexd *>(state.data())),
        std::vector<std::size_t>{state.size()}, getPrecision()};
  }

  std::vector<Tensor> getTensors() const override { return {getTensor()}; }

  std::size_t getNumTensors() const override { return 1; }

  complexd operator()(std::size_t tensorIdx,
                      const std::vector<std::size_t> &indices) override {
    if (tensorIdx != 0 || indices.size() != 1)
      throw std::runtime_error(MKLQ_SIMULATOR_STATE_DIAGNOSTIC_PREFIX
                               " invalid element access.");
    if (indices[0] >= state.size())
      throw std::runtime_error(MKLQ_SIMULATOR_STATE_DIAGNOSTIC_PREFIX
                               " state index out of range.");
    return state[indices[0]];
  }

  std::unique_ptr<SimulationState>
  createFromSizeAndPtr(std::size_t size, void *ptr, std::size_t) override {
    if (!ptr || size == 0)
      throw std::runtime_error(MKLQ_SIMULATOR_STATE_DIAGNOSTIC_PREFIX
                               " invalid null pointer or zero size.");
    checkedNumQubits(size, "createFromData");
    auto *data = reinterpret_cast<complexd *>(ptr);
    return std::make_unique<MklqCpuState>(
        std::vector<complexd>(data, data + size));
  }

  void dump(std::ostream &os) const override {
    for (const auto &amplitude : state)
      os << amplitude << '\n';
  }

  precision getPrecision() const override {
    return cudaq::SimulationState::precision::fp64;
  }

  void destroyState() override { state.clear(); }

  void toHost(std::complex<double> *clientAllocatedData,
              std::size_t numElements) const override {
    if (!clientAllocatedData && numElements != 0)
      throw std::runtime_error(MKLQ_SIMULATOR_STATE_DIAGNOSTIC_PREFIX
                               " null output buffer for non-zero copy.");
    if (numElements > state.size())
      throw std::runtime_error(MKLQ_SIMULATOR_STATE_DIAGNOSTIC_PREFIX
                               " requested more elements than available.");
    std::copy_n(state.data(), numElements, clientAllocatedData);
  }
};

} // namespace

class MKLQ_SIMULATOR_CLASS : public nvqir::CircuitSimulatorBase<double> {
protected:
  std::vector<complexd> state;
  std::mt19937_64 randomEngine{std::random_device{}()};
#if defined(MKLQ_ENABLE_TEST_ACCESSORS)
  mutable std::size_t bitStringConversions = 0;
  mutable std::size_t bitFlipApplications = 0;
  mutable std::size_t phaseApplications = 0;
  mutable std::size_t specializedSingleQubitApplications = 0;
  mutable std::size_t specializedSingleControlQubitApplications = 0;
  mutable std::size_t accelerateProbabilityFillApplications = 0;
  mutable std::size_t twoQubitBlockApplications = 0;
  mutable std::size_t twoQubitRowSparseApplications = 0;
  mutable std::size_t swapApplications = 0;
  mutable std::size_t denseDrawCountBuffers = 0;
  mutable std::size_t sparseDrawCountMaps = 0;
  mutable std::size_t sortedSparseDrawCountMaps = 0;
  mutable std::size_t fullRegisterProbabilityFills = 0;
  mutable std::size_t marginalProbabilityFills = 0;
  mutable std::size_t sparseFullRegisterScans = 0;
  mutable std::size_t sparseFullRegisterScanHits = 0;
  mutable std::size_t sparseFullRegisterScanMisses = 0;
  mutable std::size_t countsOnlySampleDrawBatches = 0;
  mutable std::size_t sequentialSampleDrawBatches = 0;
  mutable std::size_t sampleExpectationReductions = 0;
  mutable double sampleProbabilityFillSeconds = 0.0;
  mutable double sampleDrawAndCountSeconds = 0.0;
  mutable double sampleExpectationReductionSeconds = 0.0;
  mutable std::size_t metalCpuFallbackApplications = 0;
  mutable std::size_t threeQubitRowSparseApplications = 0;
  mutable std::size_t threeQubitUnitPermutationApplications = 0;

  struct ScopedTestTimer {
    double &accumulator;
    std::chrono::steady_clock::time_point start =
        std::chrono::steady_clock::now();

    explicit ScopedTestTimer(double &accumulator) : accumulator(accumulator) {}

    ~ScopedTestTimer() {
      const auto elapsed = std::chrono::duration<double>(
                               std::chrono::steady_clock::now() - start)
                               .count();
      // Counter probes use a positive value to distinguish an executed phase
      // from one that was not reached. A sub-clock-tick phase is not a timing
      // measurement, but it is still an executed phase.
      accumulator += elapsed > 0.0 ? elapsed :
                                   std::numeric_limits<double>::min();
    }
  };
#endif

#if defined(MKLQ_ENABLE_METAL_RUNTIME)
  mutable mklq::MetalStateVectorExecutor metalExecutor;
  bool metalStateHostDirty = false;
  bool metalResidentStatePoisoned = false;
  std::string metalResidentStatePoisonReason;
#endif

  static constexpr double zeroTolerance = 1.0e-15;
  static constexpr std::size_t parallelStateThreshold = 1ULL << 15;
  static constexpr std::size_t parallelProbabilityThreshold = 1ULL << 17;
  static constexpr std::size_t sparseSamplingOutcomeLimit = 64;
  static constexpr std::size_t denseDrawCountOutcomeLimit = 1ULL << 16;
  static constexpr int maxOpenMpThreads = 4;

#if defined(_OPENMP)
  int parallelThreadCount() const {
    return std::max(1, std::min(omp_get_max_threads(), maxOpenMpThreads));
  }
#endif

  void validateQubitIndex(std::size_t qubit, std::string_view context) const {
    if (qubit >= nQubitsAllocated)
      throw std::runtime_error(fmt::format(
          MKLQ_SIMULATOR_DIAGNOSTIC_PREFIX
          " {} qubit index {} out of range for {} allocated qubits.",
          context, qubit, nQubitsAllocated));
    if (qubit >= std::numeric_limits<unsigned long long>::digits)
      throw std::runtime_error(
          fmt::format(MKLQ_SIMULATOR_DIAGNOSTIC_PREFIX
                      " {} qubit index {} exceeds the bit-mask range.",
                      context, qubit));
  }

  void validateQubitsInRange(const std::vector<std::size_t> &qubits,
                             std::string_view context) const {
    for (auto qubit : qubits)
      validateQubitIndex(qubit, context);
  }

  static void validateUniqueQubits(const std::vector<std::size_t> &controls,
                                   const std::vector<std::size_t> &targets) {
    std::unordered_set<std::size_t> seen;
    for (auto control : controls)
      if (!seen.insert(control).second)
        throw std::runtime_error(
            MKLQ_SIMULATOR_DIAGNOSTIC_PREFIX
            " duplicate control qubit in gate application.");
    for (auto target : targets)
      if (!seen.insert(target).second)
        throw std::runtime_error(
            MKLQ_SIMULATOR_DIAGNOSTIC_PREFIX
            " duplicate target/control qubit in gate application.");
  }

  void validateGateQubits(const GateApplicationTask &task) const {
    validateQubitsInRange(task.controls, "gate control");
    validateQubitsInRange(task.targets, "gate target");
    validateUniqueQubits(task.controls, task.targets);
  }

  void applyNoiseChannel(const std::string_view gateName,
                         const std::vector<std::size_t> &controls,
                         const std::vector<std::size_t> &targets,
                         const std::vector<double> &params) override {
    if (!cudaq::getExecutionContext())
      return;

    auto noiseModel = getNoiseModel();
    if (!noiseModel)
      return;

    auto krausChannels = noiseModel->get_channels(std::string(gateName),
                                                  targets, controls, params);
    if (krausChannels.empty())
      return;

    deallocateState();
    throw std::runtime_error(fmt::format(
        MKLQ_SIMULATOR_DIAGNOSTIC_PREFIX
        " noise_model is not supported yet for gate '{}'. Use a "
        "noise-capable CUDA-Q target such as density-matrix-cpu for noisy "
        "simulation.",
        gateName));
  }

  void validateSampleQubits(const std::vector<std::size_t> &qubits) const {
    std::unordered_set<std::size_t> seen;
    for (auto qubit : qubits) {
      validateQubitIndex(qubit, "sample");
      if (!seen.insert(qubit).second)
        throw std::runtime_error(
            fmt::format(MKLQ_SIMULATOR_DIAGNOSTIC_PREFIX
                        " duplicate qubit {} in sample request.",
                        qubit));
    }
  }

  static std::size_t qubitBit(std::size_t qubit) { return 1ULL << qubit; }

  std::size_t qubitMask(std::size_t qubit) const { return qubitBit(qubit); }

  static std::size_t controlMask(const std::vector<std::size_t> &controls) {
    std::size_t mask = 0;
    for (auto control : controls)
      mask |= qubitBit(control);
    return mask;
  }

#if defined(MKLQ_ENABLE_METAL_RUNTIME)
  bool ensureMetalResidentState() {
    if (state.empty())
      return false;
    throwIfMetalResidentStatePoisoned("cannot use");
    if (metalExecutor.hasResidentState(state.size()))
      return true;
    if (!metalExecutor.uploadState(state.data(), state.size()))
      return false;
    metalStateHostDirty = false;
    return true;
  }

  bool synchronizeHostStateFromMetal() {
    if (!metalStateHostDirty)
      return true;
    throwIfMetalResidentStatePoisoned("cannot synchronize");
    if (!metalExecutor.hasResidentState(state.size()) ||
        !metalExecutor.downloadState(state.data(), state.size()))
      throw std::runtime_error(
          fmt::format(MKLQ_SIMULATOR_DIAGNOSTIC_PREFIX
                      " failed to synchronize Metal resident state: {}",
                      metalExecutor.lastError()));
    metalStateHostDirty = false;
    return true;
  }

  void invalidateMetalResidentState() {
    metalExecutor.releaseResidentState();
    metalStateHostDirty = false;
    metalResidentStatePoisoned = false;
    metalResidentStatePoisonReason.clear();
  }

  void markMetalResidentStatePoisoned(std::string_view reason) {
    metalResidentStatePoisoned = true;
    metalResidentStatePoisonReason = std::string(reason);
  }

  void throwIfMetalResidentStatePoisoned(std::string_view context) const {
    if (!metalResidentStatePoisoned)
      return;
    throw std::runtime_error(
        fmt::format(MKLQ_SIMULATOR_DIAGNOSTIC_PREFIX
                    " {} unrecoverable Metal resident state after a failed "
                    "mutating command: {}",
                    context, metalResidentStatePoisonReason));
  }

  virtual bool applyMetalResidentSingleQubitGate(
      const complexd *matrix, const std::size_t *controlQubits,
      std::size_t controlCount, std::size_t target) {
    return metalExecutor.applyResidentSingleQubitGate(matrix, controlQubits,
                                                      controlCount, target);
  }

  virtual bool applyMetalResidentTwoQubitGate(const complexd *matrix,
                                              const std::size_t *controlQubits,
                                              std::size_t controlCount,
                                              const std::size_t *targets) {
    return metalExecutor.applyResidentTwoQubitGate(matrix, controlQubits,
                                                   controlCount, targets);
  }

  virtual bool
  applyMetalResidentThreeQubitGate(const complexd *matrix,
                                   const std::size_t *controlQubits,
                                   std::size_t controlCount,
                                   const std::size_t *targets) {
    return metalExecutor.applyResidentThreeQubitGate(matrix, controlQubits,
                                                     controlCount, targets);
  }

  virtual bool
  computeMetalResidentMeasurementProbability(std::size_t index,
                                             double &probabilityOne) {
    return metalExecutor.computeResidentQubitProbability(index,
                                                         &probabilityOne);
  }

  virtual bool collapseMetalResidentMeasurement(std::size_t index, bool result,
                                                double branchProbability) {
    return metalExecutor.collapseResidentQubit(index, result,
                                               branchProbability);
  }

  virtual bool applyMetalResidentResetGate(std::size_t index,
                                           const complexd *matrix) {
    return metalExecutor.applyResidentSingleQubitGate(matrix, nullptr, 0,
                                                      index);
  }

  virtual bool flushMetalResidentGateCommands() {
    return metalExecutor.flushResidentGateCommands();
  }

  virtual bool computeMetalResidentZParityExpectation(
      const std::size_t *qubits, std::size_t qubitCount, double &expectation) {
    return metalExecutor.computeResidentZParityExpectation(
        qubits, qubitCount, &expectation);
  }

  virtual bool fillMetalResidentMarginalProbabilities(
      const std::size_t *qubits, std::size_t qubitCount, double *probabilities,
      std::size_t probabilityCount) {
    return metalExecutor.fillResidentMarginalProbabilities(
        qubits, qubitCount, probabilities, probabilityCount);
  }

  bool tryMeasureResidentQubit(std::size_t index, bool &result) {
    if (!metalStateHostDirty || !metalExecutor.hasResidentState(state.size()))
      return false;
    throwIfMetalResidentStatePoisoned("cannot measure");

    double probabilityOne = 0.0;
    if (!computeMetalResidentMeasurementProbability(index, probabilityOne))
      throw std::runtime_error(fmt::format(
          MKLQ_SIMULATOR_DIAGNOSTIC_PREFIX
          " failed to compute Metal resident measurement probability: {}",
          metalExecutor.lastError()));

    probabilityOne = std::clamp(probabilityOne, 0.0, 1.0);
    std::bernoulli_distribution distribution(probabilityOne);
    result = distribution(randomEngine);
    const auto branchProbability =
        result ? probabilityOne : 1.0 - probabilityOne;
    const auto norm = std::sqrt(branchProbability);
    if (norm <= zeroTolerance)
      throw std::runtime_error(
          MKLQ_SIMULATOR_DIAGNOSTIC_PREFIX
          " sampled a zero-probability measurement branch.");

    if (!collapseMetalResidentMeasurement(index, result, branchProbability)) {
      const auto error = fmt::format(
          MKLQ_SIMULATOR_DIAGNOSTIC_PREFIX
          " failed to collapse Metal resident measurement branch: {}",
          metalExecutor.lastError());
      markMetalResidentStatePoisoned(error);
      throw std::runtime_error(error);
    }

    metalStateHostDirty = true;
    return true;
  }
#endif

  static bool controlsSatisfied(std::size_t basis, std::size_t controls) {
    return (basis & controls) == controls;
  }

  static bool
  isDiagonalSingleQubitMatrix(const std::vector<complexd> &matrix) {
    return matrix[1] == complexd{0.0, 0.0} &&
           matrix[2] == complexd{0.0, 0.0};
  }

  template <typename PairUpdater>
  void applySingleControlSingleQubitPairs(std::size_t control,
                                          std::size_t target,
                                          PairUpdater updatePair) {
    const auto targetMask = qubitMask(target);
    const auto controlMask = qubitMask(control);
    const auto indexMasks = twoZeroBitIndexMasks(target, control);
    const auto blockCount = stateDimension >> 2;

#if defined(_OPENMP)
    const auto threadCount = parallelThreadCount();
#pragma omp parallel for num_threads(                                          \
        threadCount) if (threadCount > 1 &&                                    \
                             stateDimension >= parallelStateThreshold)
#endif
    for (std::size_t block = 0; block < blockCount; ++block) {
      const auto zeroIndex =
          indexWithTwoZeroBits(block, indexMasks) | controlMask;
      const auto oneIndex = zeroIndex | targetMask;
      updatePair(zeroIndex, oneIndex);
    }

#if defined(MKLQ_ENABLE_TEST_ACCESSORS)
    ++specializedSingleQubitApplications;
    ++specializedSingleControlQubitApplications;
#endif
  }

  void applySingleControlRzPhaseGate(complexd zeroPhase, complexd onePhase,
                                     std::size_t control,
                                     std::size_t target) {
    applySingleControlSingleQubitPairs(
        control, target, [&](std::size_t zeroIndex, std::size_t oneIndex) {
          state[zeroIndex] *= zeroPhase;
          state[oneIndex] *= onePhase;
        });

#if defined(MKLQ_ENABLE_TEST_ACCESSORS)
    ++phaseApplications;
#endif
  }

  void applyDiagonalSingleQubitGate(complexd zeroPhase, complexd onePhase,
                                    const std::vector<std::size_t> &controls,
                                    std::size_t target) {
    if (controls.size() == 1) {
      applySingleControlSingleQubitPairs(
          controls[0], target, [&](std::size_t zeroIndex,
                                   std::size_t oneIndex) {
            state[zeroIndex] *= zeroPhase;
            state[oneIndex] *= onePhase;
          });

#if defined(MKLQ_ENABLE_TEST_ACCESSORS)
      ++phaseApplications;
#endif
      return;
    }

    const auto mask = qubitMask(target);
    const auto controlBits = controlMask(controls);
    const auto zeroPhaseIsIdentity = zeroPhase == complexd{1.0, 0.0};
    const auto onePhaseIsIdentity = onePhase == complexd{1.0, 0.0};

#if defined(_OPENMP)
    const auto threadCount = parallelThreadCount();
#pragma omp parallel for num_threads(                                          \
        threadCount) if (threadCount > 1 &&                                    \
                             stateDimension >= parallelStateThreshold)
#endif
    for (std::size_t basis = 0; basis < stateDimension; ++basis) {
      if (!controlsSatisfied(basis, controlBits))
        continue;

      if ((basis & mask) == 0) {
        if (!zeroPhaseIsIdentity)
          state[basis] *= zeroPhase;
      } else if (!onePhaseIsIdentity) {
        state[basis] *= onePhase;
      }
    }

#if defined(MKLQ_ENABLE_TEST_ACCESSORS)
    ++phaseApplications;
#endif
  }

  void addQubitToState() override { addQubitsToState(1); }

  void addQubitsToState(std::size_t qubitCount,
                        const void *stateDataIn = nullptr) override {
    if (qubitCount == 0)
      return;

#if defined(MKLQ_ENABLE_METAL_RUNTIME)
    synchronizeHostStateFromMetal();
    invalidateMetalResidentState();
#endif

    const auto newQubitDim = 1ULL << qubitCount;
    const auto *stateData = reinterpret_cast<const complexd *>(stateDataIn);

    if (state.empty()) {
      state.assign(stateDimension, complexd{0.0, 0.0});
      if (stateData)
        std::copy_n(stateData, stateDimension, state.begin());
      else
        state[0] = 1.0;
      return;
    }

    std::vector<complexd> next(stateDimension, complexd{0.0, 0.0});
    if (!stateData) {
      std::copy(state.begin(), state.end(), next.begin());
    } else {
      for (std::size_t newBasis = 0; newBasis < newQubitDim; ++newBasis)
        for (std::size_t oldBasis = 0; oldBasis < previousStateDimension;
             ++oldBasis)
          next[(newBasis << checkedNumQubits(previousStateDimension,
                                             "state growth")) |
               oldBasis] = stateData[newBasis] * state[oldBasis];
    }
    state = std::move(next);
  }

  void addQubitsToState(const cudaq::SimulationState &inState) override {
#if defined(MKLQ_ENABLE_METAL_RUNTIME)
    synchronizeHostStateFromMetal();
    invalidateMetalResidentState();
#endif

    if (inState.getNumTensors() != 1)
      throw std::invalid_argument(MKLQ_SIMULATOR_DIAGNOSTIC_PREFIX
                                  " incompatible simulation state input.");

    auto tensor = inState.getTensor();
    if (tensor.extents.size() != 1 ||
        tensor.fp_precision != cudaq::SimulationState::precision::fp64)
      throw std::invalid_argument(
          MKLQ_SIMULATOR_DIAGNOSTIC_PREFIX
          " expected a one-dimensional fp64 state vector input.");
    if (!tensor.data)
      throw std::invalid_argument(MKLQ_SIMULATOR_DIAGNOSTIC_PREFIX
                                  " null state vector input.");
    checkedNumQubits(tensor.extents[0], "state input");

    auto *data = reinterpret_cast<complexd *>(tensor.data);
    std::vector<complexd> incoming(data, data + tensor.extents[0]);

    if (state.empty()) {
      state = std::move(incoming);
      return;
    }

    const auto oldQubits =
        checkedNumQubits(previousStateDimension, "state append");
    std::vector<complexd> next(stateDimension, complexd{0.0, 0.0});
    for (std::size_t newBasis = 0; newBasis < incoming.size(); ++newBasis)
      for (std::size_t oldBasis = 0; oldBasis < previousStateDimension;
           ++oldBasis)
        next[(newBasis << oldQubits) | oldBasis] =
            incoming[newBasis] * state[oldBasis];
    state = std::move(next);
  }

  void deallocateStateImpl() override {
#if defined(MKLQ_ENABLE_METAL_RUNTIME)
    invalidateMetalResidentState();
#endif
    state.clear();
  }

  std::size_t
  indexWithTargetBits(std::size_t base, std::size_t targetBits,
                      const std::vector<std::size_t> &targetMasks) const {
    auto result = base;
    for (std::size_t bit = 0; bit < targetMasks.size(); ++bit)
      if (targetBits & (1ULL << bit))
        result |= targetMasks[bit];
    return result;
  }

  static std::size_t insertZeroBit(std::size_t value, std::size_t bit) {
    const auto lowMask = (1ULL << bit) - 1;
    return ((value & ~lowMask) << 1) | (value & lowMask);
  }

  struct TwoZeroBitIndexMasks {
    std::size_t lowMask;
    std::size_t middleMask;
    std::size_t highMask;
  };

  static TwoZeroBitIndexMasks twoZeroBitIndexMasks(std::size_t firstBit,
                                                   std::size_t secondBit) {
    const auto first = std::min(firstBit, secondBit);
    const auto second = std::max(firstBit, secondBit);
    const auto lowMask = (1ULL << first) - 1;
    const auto middleMask = ((1ULL << (second - 1)) - 1) & ~lowMask;
    const auto highMask = ~((1ULL << (second - 1)) - 1);
    return {lowMask, middleMask, highMask};
  }

  static std::size_t indexWithTwoZeroBits(
      std::size_t block, const TwoZeroBitIndexMasks &masks) {
    return (block & masks.lowMask) | ((block & masks.middleMask) << 1) |
           ((block & masks.highMask) << 2);
  }

  static std::size_t indexWithTwoZeroBits(std::size_t block,
                                          std::size_t firstBit,
                                          std::size_t secondBit) {
    return indexWithTwoZeroBits(block,
                                twoZeroBitIndexMasks(firstBit, secondBit));
  }

  static std::size_t
  indexWithTwoTargetBits(std::size_t base, std::size_t targetBits,
                         const std::array<std::size_t, 2> &targetMasks) {
    auto result = base;
    if (targetBits & 1)
      result |= targetMasks[0];
    if (targetBits & 2)
      result |= targetMasks[1];
    return result;
  }

  static std::size_t
  indexWithThreeZeroBits(std::size_t block,
                         const std::array<std::size_t, 3> &sortedTargets) {
    auto result = block;
    for (auto target : sortedTargets)
      result = insertZeroBit(result, target);
    return result;
  }

  static std::size_t
  indexWithThreeTargetBits(std::size_t base, std::size_t targetBits,
                           const std::array<std::size_t, 3> &targetMasks) {
    auto result = base;
    if (targetBits & 1)
      result |= targetMasks[0];
    if (targetBits & 2)
      result |= targetMasks[1];
    if (targetBits & 4)
      result |= targetMasks[2];
    return result;
  }

  void applySingleQubitGate(const std::vector<complexd> &matrix,
                            const std::vector<std::size_t> &controls,
                            std::size_t target, std::string_view operationName,
                            bool isBuiltInOperation) {
    const auto mask = qubitMask(target);
    const auto lowMask = mask - 1;
    const auto pairCount = stateDimension >> 1;
    const auto m00 = matrix[0];
    const auto m01 = matrix[1];
    const auto m10 = matrix[2];
    const auto m11 = matrix[3];

#if defined(MKLQ_ENABLE_METAL_RUNTIME)
    const bool hadDirtyMetalState = metalStateHostDirty;
    if (ensureMetalResidentState()) {
      if (applyMetalResidentSingleQubitGate(matrix.data(), controls.data(),
                                            controls.size(), target)) {
        metalStateHostDirty = true;
        return;
      }
      if (hadDirtyMetalState) {
        const auto error =
            fmt::format(MKLQ_SIMULATOR_DIAGNOSTIC_PREFIX
                        " failed to apply resident Metal single-qubit gate: {}",
                        metalExecutor.lastError());
        markMetalResidentStatePoisoned(error);
        throw std::runtime_error(error);
      }
    }
    synchronizeHostStateFromMetal();
    invalidateMetalResidentState();
#endif

    if (isBuiltInOperation && operationName == "x") {
      applyBitFlipGate(controls, target);
      return;
    }

    if (isBuiltInOperation && operationName == "z" && !controls.empty()) {
      applyZPhaseGate(controls, target);
      return;
    }

    if (isBuiltInOperation && applySpecializedSingleQubitGate(
                                  operationName, matrix, controls, target))
      return;

    if (isBuiltInOperation && isDiagonalSingleQubitMatrix(matrix)) {
      applyDiagonalSingleQubitGate(matrix[0], matrix[3], controls, target);
      return;
    }

    const auto controlBits = controlMask(controls);
#if defined(_OPENMP)
    const auto threadCount = parallelThreadCount();
#pragma omp parallel for num_threads(                                          \
        threadCount) if (threadCount > 1 &&                                    \
                             stateDimension >= parallelStateThreshold)
#endif
    for (std::size_t pair = 0; pair < pairCount; ++pair) {
      const auto zeroIndex = ((pair & ~lowMask) << 1) | (pair & lowMask);
      if (!controlsSatisfied(zeroIndex, controlBits))
        continue;

      const auto oneIndex = zeroIndex | mask;
      const auto zeroAmplitude = state[zeroIndex];
      const auto oneAmplitude = state[oneIndex];
      state[zeroIndex] = m00 * zeroAmplitude + m01 * oneAmplitude;
      state[oneIndex] = m10 * zeroAmplitude + m11 * oneAmplitude;
    }
  }

  bool applySpecializedSingleQubitGate(std::string_view operationName,
                                       const std::vector<complexd> &matrix,
                                       const std::vector<std::size_t> &controls,
                                       std::size_t target) {
    if (operationName == "h") {
      applyHadamardGate(matrix, controls, target);
      return true;
    }
    if (operationName == "y") {
      applyYGate(controls, target);
      return true;
    }
    if (operationName == "rx") {
      applyRxGate(matrix, controls, target);
      return true;
    }
    if (operationName == "ry") {
      applyRyGate(matrix, controls, target);
      return true;
    }
    if (operationName == "rz") {
      applyRzGate(matrix, controls, target);
      return true;
    }
    return false;
  }

  void applyYGate(const std::vector<std::size_t> &controls,
                  std::size_t target) {
    if (controls.size() == 1) {
      applySingleControlSingleQubitPairs(
          controls[0], target, [&](std::size_t zeroIndex,
                                   std::size_t oneIndex) {
            const auto zeroAmplitude = state[zeroIndex];
            const auto oneAmplitude = state[oneIndex];
            state[zeroIndex] = {oneAmplitude.imag(), -oneAmplitude.real()};
            state[oneIndex] = {-zeroAmplitude.imag(), zeroAmplitude.real()};
          });
      return;
    }

    const auto mask = qubitMask(target);
    const auto lowMask = mask - 1;
    const auto pairCount = stateDimension >> 1;
    const auto controlBits = controlMask(controls);

#if defined(_OPENMP)
    const auto threadCount = parallelThreadCount();
#pragma omp parallel for num_threads(                                          \
        threadCount) if (threadCount > 1 &&                                    \
                             stateDimension >= parallelStateThreshold)
#endif
    for (std::size_t pair = 0; pair < pairCount; ++pair) {
      const auto zeroIndex = ((pair & ~lowMask) << 1) | (pair & lowMask);
      if (!controlsSatisfied(zeroIndex, controlBits))
        continue;

      const auto oneIndex = zeroIndex | mask;
      const auto zeroAmplitude = state[zeroIndex];
      const auto oneAmplitude = state[oneIndex];
      state[zeroIndex] = {oneAmplitude.imag(), -oneAmplitude.real()};
      state[oneIndex] = {-zeroAmplitude.imag(), zeroAmplitude.real()};
    }

#if defined(MKLQ_ENABLE_TEST_ACCESSORS)
    ++specializedSingleQubitApplications;
#endif
  }

  void applyHadamardGate(const std::vector<complexd> &matrix,
                         const std::vector<std::size_t> &controls,
                         std::size_t target) {
    const auto scale = matrix[0].real();
    if (controls.size() == 1) {
      applySingleControlSingleQubitPairs(
          controls[0], target, [&](std::size_t zeroIndex,
                                   std::size_t oneIndex) {
            const auto zeroAmplitude = state[zeroIndex];
            const auto oneAmplitude = state[oneIndex];
            state[zeroIndex] = scale * (zeroAmplitude + oneAmplitude);
            state[oneIndex] = scale * (zeroAmplitude - oneAmplitude);
          });
      return;
    }

    const auto mask = qubitMask(target);
    const auto lowMask = mask - 1;
    const auto pairCount = stateDimension >> 1;
    const auto controlBits = controlMask(controls);

#if defined(_OPENMP)
    const auto threadCount = parallelThreadCount();
#pragma omp parallel for num_threads(                                          \
        threadCount) if (threadCount > 1 &&                                    \
                             stateDimension >= parallelStateThreshold)
#endif
    for (std::size_t pair = 0; pair < pairCount; ++pair) {
      const auto zeroIndex = ((pair & ~lowMask) << 1) | (pair & lowMask);
      if (!controlsSatisfied(zeroIndex, controlBits))
        continue;

      const auto oneIndex = zeroIndex | mask;
      const auto zeroAmplitude = state[zeroIndex];
      const auto oneAmplitude = state[oneIndex];
      state[zeroIndex] = scale * (zeroAmplitude + oneAmplitude);
      state[oneIndex] = scale * (zeroAmplitude - oneAmplitude);
    }

#if defined(MKLQ_ENABLE_TEST_ACCESSORS)
    ++specializedSingleQubitApplications;
#endif
  }

  void applyRxGate(const std::vector<complexd> &matrix,
                   const std::vector<std::size_t> &controls,
                   std::size_t target) {
    const auto cosine = matrix[0].real();
    const auto imaginarySine = matrix[1].imag();

    auto multiplyByPureImaginary = [](double imaginary,
                                      complexd amplitude) -> complexd {
      return {-imaginary * amplitude.imag(), imaginary * amplitude.real()};
    };

    if (controls.size() == 1) {
      applySingleControlSingleQubitPairs(
          controls[0], target, [&](std::size_t zeroIndex,
                                   std::size_t oneIndex) {
            const auto zeroAmplitude = state[zeroIndex];
            const auto oneAmplitude = state[oneIndex];
            state[zeroIndex] =
                cosine * zeroAmplitude +
                multiplyByPureImaginary(imaginarySine, oneAmplitude);
            state[oneIndex] =
                multiplyByPureImaginary(imaginarySine, zeroAmplitude) +
                cosine * oneAmplitude;
          });
      return;
    }

    const auto mask = qubitMask(target);
    const auto lowMask = mask - 1;
    const auto pairCount = stateDimension >> 1;
    const auto controlBits = controlMask(controls);

#if defined(_OPENMP)
    const auto threadCount = parallelThreadCount();
#pragma omp parallel for num_threads(                                          \
        threadCount) if (threadCount > 1 &&                                    \
                             stateDimension >= parallelStateThreshold)
#endif
    for (std::size_t pair = 0; pair < pairCount; ++pair) {
      const auto zeroIndex = ((pair & ~lowMask) << 1) | (pair & lowMask);
      if (!controlsSatisfied(zeroIndex, controlBits))
        continue;

      const auto oneIndex = zeroIndex | mask;
      const auto zeroAmplitude = state[zeroIndex];
      const auto oneAmplitude = state[oneIndex];
      state[zeroIndex] = cosine * zeroAmplitude +
                         multiplyByPureImaginary(imaginarySine, oneAmplitude);
      state[oneIndex] = multiplyByPureImaginary(imaginarySine, zeroAmplitude) +
                        cosine * oneAmplitude;
    }

#if defined(MKLQ_ENABLE_TEST_ACCESSORS)
    ++specializedSingleQubitApplications;
#endif
  }

  void applyRyGate(const std::vector<complexd> &matrix,
                   const std::vector<std::size_t> &controls,
                   std::size_t target) {
    const auto cosine = matrix[0].real();
    const auto sine = matrix[2].real();

    if (controls.size() == 1) {
      applySingleControlSingleQubitPairs(
          controls[0], target, [&](std::size_t zeroIndex,
                                   std::size_t oneIndex) {
            const auto zeroAmplitude = state[zeroIndex];
            const auto oneAmplitude = state[oneIndex];
            state[zeroIndex] = cosine * zeroAmplitude - sine * oneAmplitude;
            state[oneIndex] = sine * zeroAmplitude + cosine * oneAmplitude;
          });
      return;
    }

    const auto mask = qubitMask(target);
    const auto lowMask = mask - 1;
    const auto pairCount = stateDimension >> 1;
    const auto controlBits = controlMask(controls);

#if defined(_OPENMP)
    const auto threadCount = parallelThreadCount();
#pragma omp parallel for num_threads(                                          \
        threadCount) if (threadCount > 1 &&                                    \
                             stateDimension >= parallelStateThreshold)
#endif
    for (std::size_t pair = 0; pair < pairCount; ++pair) {
      const auto zeroIndex = ((pair & ~lowMask) << 1) | (pair & lowMask);
      if (!controlsSatisfied(zeroIndex, controlBits))
        continue;

      const auto oneIndex = zeroIndex | mask;
      const auto zeroAmplitude = state[zeroIndex];
      const auto oneAmplitude = state[oneIndex];
      state[zeroIndex] = cosine * zeroAmplitude - sine * oneAmplitude;
      state[oneIndex] = sine * zeroAmplitude + cosine * oneAmplitude;
    }

#if defined(MKLQ_ENABLE_TEST_ACCESSORS)
    ++specializedSingleQubitApplications;
#endif
  }

  void applyRzGate(const std::vector<complexd> &matrix,
                   const std::vector<std::size_t> &controls,
                   std::size_t target) {
    const auto zeroPhase = matrix[0];
    const auto onePhase = matrix[3];

    if (controls.size() == 1) {
      applySingleControlRzPhaseGate(zeroPhase, onePhase, controls[0], target);
      return;
    }

    const auto mask = qubitMask(target);
    const auto lowMask = mask - 1;
    const auto pairCount = stateDimension >> 1;
    const auto controlBits = controlMask(controls);

#if defined(_OPENMP)
    const auto threadCount = parallelThreadCount();
#pragma omp parallel for num_threads(                                          \
        threadCount) if (threadCount > 1 &&                                    \
                             stateDimension >= parallelStateThreshold)
#endif
    for (std::size_t pair = 0; pair < pairCount; ++pair) {
      const auto zeroIndex = ((pair & ~lowMask) << 1) | (pair & lowMask);
      if (!controlsSatisfied(zeroIndex, controlBits))
        continue;

      const auto oneIndex = zeroIndex | mask;
      state[zeroIndex] *= zeroPhase;
      state[oneIndex] *= onePhase;
    }

#if defined(MKLQ_ENABLE_TEST_ACCESSORS)
    ++specializedSingleQubitApplications;
#endif
  }

  void applyTwoQubitGate(const std::vector<complexd> &matrix,
                         const std::vector<std::size_t> &controls,
                         const std::vector<std::size_t> &targets,
                         std::string_view operationName,
                         bool isBuiltInOperation) {
#if defined(MKLQ_ENABLE_METAL_RUNTIME)
    const bool hadDirtyMetalState = metalStateHostDirty;
    if (ensureMetalResidentState()) {
      if (applyMetalResidentTwoQubitGate(matrix.data(), controls.data(),
                                         controls.size(), targets.data())) {
        metalStateHostDirty = true;
        return;
      }
      if (hadDirtyMetalState) {
        const auto error =
            fmt::format(MKLQ_SIMULATOR_DIAGNOSTIC_PREFIX
                        " failed to apply resident Metal two-qubit gate: {}",
                        metalExecutor.lastError());
        markMetalResidentStatePoisoned(error);
        throw std::runtime_error(error);
      }
    }
    synchronizeHostStateFromMetal();
    invalidateMetalResidentState();
#endif

    if (isBuiltInOperation && operationName == "swap") {
      applySwapGate(controls, targets);
      return;
    }

    if (applyRowSparseTwoQubitGate(matrix, controls, targets))
      return;

    const std::array<std::size_t, 2> targetMasks{
        qubitMask(targets[0]),
        qubitMask(targets[1]),
    };
    const auto m00 = matrix[0];
    const auto m01 = matrix[1];
    const auto m02 = matrix[2];
    const auto m03 = matrix[3];
    const auto m10 = matrix[4];
    const auto m11 = matrix[5];
    const auto m12 = matrix[6];
    const auto m13 = matrix[7];
    const auto m20 = matrix[8];
    const auto m21 = matrix[9];
    const auto m22 = matrix[10];
    const auto m23 = matrix[11];
    const auto m30 = matrix[12];
    const auto m31 = matrix[13];
    const auto m32 = matrix[14];
    const auto m33 = matrix[15];

    const auto blockCount = stateDimension >> 2;
    const auto indexMasks = twoZeroBitIndexMasks(targets[0], targets[1]);
    const auto controlBits = controlMask(controls);
#if defined(_OPENMP)
    const auto threadCount = parallelThreadCount();
#pragma omp parallel for num_threads(                                          \
        threadCount) if (threadCount > 1 &&                                    \
                             stateDimension >= parallelStateThreshold)
#endif
    for (std::size_t block = 0; block < blockCount; ++block) {
      const auto base = indexWithTwoZeroBits(block, indexMasks);
      if (!controlsSatisfied(base, controlBits))
        continue;

      const auto index0 = base;
      const auto index1 = indexWithTwoTargetBits(base, 1, targetMasks);
      const auto index2 = indexWithTwoTargetBits(base, 2, targetMasks);
      const auto index3 = indexWithTwoTargetBits(base, 3, targetMasks);

      const auto amplitude0 = state[index0];
      const auto amplitude1 = state[index1];
      const auto amplitude2 = state[index2];
      const auto amplitude3 = state[index3];

      state[index0] = m00 * amplitude0 + m01 * amplitude1 +
                      m02 * amplitude2 + m03 * amplitude3;
      state[index1] = m10 * amplitude0 + m11 * amplitude1 +
                      m12 * amplitude2 + m13 * amplitude3;
      state[index2] = m20 * amplitude0 + m21 * amplitude1 +
                      m22 * amplitude2 + m23 * amplitude3;
      state[index3] = m30 * amplitude0 + m31 * amplitude1 +
                      m32 * amplitude2 + m33 * amplitude3;
    }

#if defined(MKLQ_ENABLE_TEST_ACCESSORS)
    ++twoQubitBlockApplications;
#endif
  }

  bool applyRowSparseTwoQubitGate(const std::vector<complexd> &matrix,
                                  const std::vector<std::size_t> &controls,
                                  const std::vector<std::size_t> &targets) {
    static constexpr std::size_t subspaceDim = 4;

    std::array<std::size_t, subspaceDim> sourceColumns{};
    std::array<complexd, subspaceDim> coefficients{};
    std::array<bool, subspaceDim> hasCoefficient{};
    for (std::size_t row = 0; row < subspaceDim; ++row) {
      for (std::size_t column = 0; column < subspaceDim; ++column) {
        const auto coefficient = matrix[row * subspaceDim + column];
        if (coefficient == complexd{0.0, 0.0})
          continue;
        if (hasCoefficient[row])
          return false;
        hasCoefficient[row] = true;
        sourceColumns[row] = column;
        coefficients[row] = coefficient;
      }
    }

    const std::array<std::size_t, 2> targetMasks{
        qubitMask(targets[0]),
        qubitMask(targets[1]),
    };

    const auto blockCount = stateDimension >> 2;
    const auto indexMasks = twoZeroBitIndexMasks(targets[0], targets[1]);
    const auto controlBits = controlMask(controls);
#if defined(_OPENMP)
    const auto threadCount = parallelThreadCount();
#pragma omp parallel for num_threads(                                          \
        threadCount) if (threadCount > 1 &&                                    \
                             stateDimension >= parallelStateThreshold)
#endif
    for (std::size_t block = 0; block < blockCount; ++block) {
      const auto base = indexWithTwoZeroBits(block, indexMasks);
      if (!controlsSatisfied(base, controlBits))
        continue;

      std::array<complexd, subspaceDim> amplitudes;
      for (std::size_t column = 0; column < subspaceDim; ++column)
        amplitudes[column] =
            state[indexWithTwoTargetBits(base, column, targetMasks)];

      for (std::size_t row = 0; row < subspaceDim; ++row) {
        const auto updated =
            hasCoefficient[row]
                ? coefficients[row] * amplitudes[sourceColumns[row]]
                : complexd{0.0, 0.0};
        state[indexWithTwoTargetBits(base, row, targetMasks)] = updated;
      }
    }

#if defined(MKLQ_ENABLE_TEST_ACCESSORS)
    ++twoQubitRowSparseApplications;
#endif
    return true;
  }

  void applyBitFlipGate(const std::vector<std::size_t> &controls,
                        std::size_t target) {
    if (controls.size() == 1) {
      applySingleControlSingleQubitPairs(
          controls[0], target, [&](std::size_t zeroIndex,
                                   std::size_t oneIndex) {
            std::swap(state[zeroIndex], state[oneIndex]);
          });
#if defined(MKLQ_ENABLE_TEST_ACCESSORS)
      ++bitFlipApplications;
#endif
      return;
    }

    const auto mask = qubitMask(target);
    const auto lowMask = mask - 1;
    const auto pairCount = stateDimension >> 1;
    const auto controlBits = controlMask(controls);

#if defined(_OPENMP)
    const auto threadCount = parallelThreadCount();
#pragma omp parallel for num_threads(                                          \
        threadCount) if (threadCount > 1 &&                                    \
                             stateDimension >= parallelStateThreshold)
#endif
    for (std::size_t pair = 0; pair < pairCount; ++pair) {
      const auto zeroIndex = ((pair & ~lowMask) << 1) | (pair & lowMask);
      if (!controlsSatisfied(zeroIndex, controlBits))
        continue;

      const auto oneIndex = zeroIndex | mask;
      std::swap(state[zeroIndex], state[oneIndex]);
    }

#if defined(MKLQ_ENABLE_TEST_ACCESSORS)
    ++bitFlipApplications;
#endif
  }

  void applyZPhaseGate(const std::vector<std::size_t> &controls,
                       std::size_t target) {
    const auto mask = qubitMask(target);
    if (controls.size() == 1) {
      const auto control = controls[0];
      const auto phaseMask = mask | qubitMask(control);
      const auto blockCount = stateDimension >> 2;
      const auto indexMasks = twoZeroBitIndexMasks(target, control);

#if defined(_OPENMP)
      const auto threadCount = parallelThreadCount();
#pragma omp parallel for num_threads(                                          \
        threadCount) if (threadCount > 1 &&                                    \
                             stateDimension >= parallelStateThreshold)
#endif
      for (std::size_t block = 0; block < blockCount; ++block) {
        const auto basis = indexWithTwoZeroBits(block, indexMasks) | phaseMask;
        state[basis] = -state[basis];
      }

#if defined(MKLQ_ENABLE_TEST_ACCESSORS)
      ++phaseApplications;
#endif
      return;
    }

    const auto controlBits = controlMask(controls);
#if defined(_OPENMP)
    const auto threadCount = parallelThreadCount();
#pragma omp parallel for num_threads(                                          \
        threadCount) if (threadCount > 1 &&                                    \
                             stateDimension >= parallelStateThreshold)
#endif
    for (std::size_t basis = 0; basis < stateDimension; ++basis) {
      if ((basis & mask) == 0 || !controlsSatisfied(basis, controlBits))
        continue;
      state[basis] = -state[basis];
    }

#if defined(MKLQ_ENABLE_TEST_ACCESSORS)
    ++phaseApplications;
#endif
  }

  void applySwapGate(const std::vector<std::size_t> &controls,
                     const std::vector<std::size_t> &targets) {
    std::vector<std::size_t> targetMasks;
    targetMasks.reserve(2);
    for (auto target : targets)
      targetMasks.push_back(qubitMask(target));

    const auto blockCount = stateDimension >> 2;
    const auto indexMasks = twoZeroBitIndexMasks(targets[0], targets[1]);
    const auto controlBits = controlMask(controls);
#if defined(_OPENMP)
    const auto threadCount = parallelThreadCount();
#pragma omp parallel for num_threads(                                          \
        threadCount) if (threadCount > 1 &&                                    \
                             stateDimension >= parallelStateThreshold)
#endif
    for (std::size_t block = 0; block < blockCount; ++block) {
      const auto base = indexWithTwoZeroBits(block, indexMasks);
      if (!controlsSatisfied(base, controlBits))
        continue;

      const auto index1 = indexWithTargetBits(base, 1, targetMasks);
      const auto index2 = indexWithTargetBits(base, 2, targetMasks);
      std::swap(state[index1], state[index2]);
    }

#if defined(MKLQ_ENABLE_TEST_ACCESSORS)
    ++swapApplications;
#endif
  }

  bool applyUnitInvolutionThreeQubitPermutation(
      const std::array<std::size_t, 8> &sourceColumns,
      const std::array<complexd, 8> &coefficients,
      const std::array<bool, 8> &hasCoefficient,
      const std::vector<std::size_t> &controls,
      const std::vector<std::size_t> &targets) {
    static constexpr std::size_t subspaceDim = 8;
    for (std::size_t row = 0; row < subspaceDim; ++row) {
      if (!hasCoefficient[row] || coefficients[row] != complexd{1.0, 0.0} ||
          sourceColumns[sourceColumns[row]] != row)
        return false;
    }

    const std::array<std::size_t, 3> targetMasks{
        qubitMask(targets[0]),
        qubitMask(targets[1]),
        qubitMask(targets[2]),
    };
    std::array<std::size_t, 3> sortedTargets{
        targets[0],
        targets[1],
        targets[2],
    };
    std::sort(sortedTargets.begin(), sortedTargets.end());

    const auto blockCount = stateDimension >> 3;
    const auto controlBits = controlMask(controls);
#if defined(_OPENMP)
    const auto threadCount = parallelThreadCount();
#pragma omp parallel for num_threads(                                          \
        threadCount) if (threadCount > 1 &&                                    \
                             stateDimension >= parallelStateThreshold)
#endif
    for (std::size_t block = 0; block < blockCount; ++block) {
      const auto base = indexWithThreeZeroBits(block, sortedTargets);
      if (!controlsSatisfied(base, controlBits))
        continue;

      for (std::size_t row = 0; row < subspaceDim; ++row) {
        const auto source = sourceColumns[row];
        if (row >= source)
          continue;
        std::swap(state[indexWithThreeTargetBits(base, row, targetMasks)],
                  state[indexWithThreeTargetBits(base, source, targetMasks)]);
      }
    }

#if defined(MKLQ_ENABLE_TEST_ACCESSORS)
    ++threeQubitUnitPermutationApplications;
#endif
    return true;
  }

  bool applyRowSparseThreeQubitGate(const std::vector<complexd> &matrix,
                                    const std::vector<std::size_t> &controls,
                                    const std::vector<std::size_t> &targets) {
    static constexpr std::size_t subspaceDim = 8;

    std::array<std::size_t, subspaceDim> sourceColumns{};
    std::array<complexd, subspaceDim> coefficients{};
    std::array<bool, subspaceDim> hasCoefficient{};
    for (std::size_t row = 0; row < subspaceDim; ++row) {
      for (std::size_t column = 0; column < subspaceDim; ++column) {
        const auto coefficient = matrix[row * subspaceDim + column];
        if (coefficient == complexd{0.0, 0.0})
          continue;
        if (hasCoefficient[row])
          return false;
        hasCoefficient[row] = true;
        sourceColumns[row] = column;
        coefficients[row] = coefficient;
      }
    }

    if (applyUnitInvolutionThreeQubitPermutation(
            sourceColumns, coefficients, hasCoefficient, controls, targets))
      return true;

    const std::array<std::size_t, 3> targetMasks{
        qubitMask(targets[0]),
        qubitMask(targets[1]),
        qubitMask(targets[2]),
    };
    std::array<std::size_t, 3> sortedTargets{
        targets[0],
        targets[1],
        targets[2],
    };
    std::sort(sortedTargets.begin(), sortedTargets.end());

    const auto blockCount = stateDimension >> 3;
    const auto controlBits = controlMask(controls);
#if defined(_OPENMP)
    const auto threadCount = parallelThreadCount();
#pragma omp parallel for num_threads(                                          \
        threadCount) if (threadCount > 1 &&                                    \
                             stateDimension >= parallelStateThreshold)
#endif
    for (std::size_t block = 0; block < blockCount; ++block) {
      const auto base = indexWithThreeZeroBits(block, sortedTargets);
      if (!controlsSatisfied(base, controlBits))
        continue;

      std::array<complexd, subspaceDim> amplitudes;
      for (std::size_t column = 0; column < subspaceDim; ++column)
        amplitudes[column] =
            state[indexWithThreeTargetBits(base, column, targetMasks)];

      for (std::size_t row = 0; row < subspaceDim; ++row) {
        const auto updated =
            hasCoefficient[row]
                ? coefficients[row] * amplitudes[sourceColumns[row]]
                : complexd{0.0, 0.0};
        state[indexWithThreeTargetBits(base, row, targetMasks)] = updated;
      }
    }

#if defined(MKLQ_ENABLE_TEST_ACCESSORS)
    ++threeQubitRowSparseApplications;
#endif
    return true;
  }

  void applyGate(const GateApplicationTask &task) override {
    if (state.empty())
      throw std::runtime_error(MKLQ_SIMULATOR_DIAGNOSTIC_PREFIX
                               " cannot apply a gate to empty state.");

    validateGateQubits(task);
    const auto targetCount = task.targets.size();
    const auto subspaceDim = 1ULL << targetCount;
    if (task.matrix.size() != subspaceDim * subspaceDim)
      throw std::runtime_error(
          fmt::format(MKLQ_SIMULATOR_DIAGNOSTIC_PREFIX
                      " gate '{}' matrix size {} does not match {} targets.",
                      task.operationName, task.matrix.size(), targetCount));

    if (targetCount == 1) {
      applySingleQubitGate(task.matrix, task.controls, task.targets[0],
                           task.operationName, task.isBuiltInOperation);
      return;
    }

    if (targetCount == 2) {
      applyTwoQubitGate(task.matrix, task.controls, task.targets,
                        task.operationName, task.isBuiltInOperation);
      return;
    }

#if defined(MKLQ_ENABLE_METAL_RUNTIME)
    if (targetCount == 3) {
      const bool hadDirtyMetalState = metalStateHostDirty;
      if (ensureMetalResidentState()) {
        if (applyMetalResidentThreeQubitGate(task.matrix.data(),
                                             task.controls.data(),
                                             task.controls.size(),
                                             task.targets.data())) {
          metalStateHostDirty = true;
          return;
        }
        if (hadDirtyMetalState) {
          const auto error =
              fmt::format(MKLQ_SIMULATOR_DIAGNOSTIC_PREFIX
                          " failed to apply resident Metal three-qubit gate: {}",
                          metalExecutor.lastError());
          markMetalResidentStatePoisoned(error);
          throw std::runtime_error(error);
        }
      }
    }
#if defined(MKLQ_ENABLE_TEST_ACCESSORS)
    if (metalStateHostDirty || metalExecutor.hasResidentState(state.size()))
      ++metalCpuFallbackApplications;
#endif
    synchronizeHostStateFromMetal();
    invalidateMetalResidentState();
#endif

    if (targetCount == 3 &&
        applyRowSparseThreeQubitGate(task.matrix, task.controls, task.targets))
      return;

    std::vector<std::size_t> targetMasks;
    targetMasks.reserve(targetCount);
    std::size_t targetMask = 0;
    for (auto target : task.targets) {
      auto mask = qubitMask(target);
      targetMasks.push_back(mask);
      targetMask |= mask;
    }

    auto next = state;
    const auto controlBits = controlMask(task.controls);
#if defined(_OPENMP)
    const auto threadCount = parallelThreadCount();
#pragma omp parallel num_threads(                                              \
        threadCount) if (threadCount > 1 &&                                    \
                             stateDimension >= parallelStateThreshold)
    {
      std::vector<complexd> amplitudes(subspaceDim);
#pragma omp for schedule(static)
      for (std::size_t base = 0; base < stateDimension; ++base) {
        if ((base & targetMask) != 0 || !controlsSatisfied(base, controlBits))
          continue;

        for (std::size_t column = 0; column < subspaceDim; ++column)
          amplitudes[column] =
              state[indexWithTargetBits(base, column, targetMasks)];

        for (std::size_t row = 0; row < subspaceDim; ++row) {
          complexd updated{0.0, 0.0};
          for (std::size_t column = 0; column < subspaceDim; ++column)
            updated +=
                task.matrix[row * subspaceDim + column] * amplitudes[column];
          next[indexWithTargetBits(base, row, targetMasks)] = updated;
        }
      }
    }
#else
    std::vector<complexd> amplitudes(subspaceDim);
    for (std::size_t base = 0; base < stateDimension; ++base) {
      if ((base & targetMask) != 0 || !controlsSatisfied(base, controlBits))
        continue;

      for (std::size_t column = 0; column < subspaceDim; ++column)
        amplitudes[column] =
            state[indexWithTargetBits(base, column, targetMasks)];

      for (std::size_t row = 0; row < subspaceDim; ++row) {
        complexd updated{0.0, 0.0};
        for (std::size_t column = 0; column < subspaceDim; ++column)
          updated +=
              task.matrix[row * subspaceDim + column] * amplitudes[column];
        next[indexWithTargetBits(base, row, targetMasks)] = updated;
      }
    }
#endif
    state = std::move(next);
  }

  void setToZeroState() override {
#if defined(MKLQ_ENABLE_METAL_RUNTIME)
    invalidateMetalResidentState();
#endif
    state.assign(stateDimension, complexd{0.0, 0.0});
    if (!state.empty())
      state[0] = 1.0;
  }

  bool measureQubit(const std::size_t index) override {
    if (state.empty())
      throw std::runtime_error(MKLQ_SIMULATOR_DIAGNOSTIC_PREFIX
                               " cannot measure empty state.");
    validateQubitIndex(index, "measure");

#if defined(MKLQ_ENABLE_METAL_RUNTIME)
    bool residentResult = false;
    if (tryMeasureResidentQubit(index, residentResult))
      return residentResult;

    synchronizeHostStateFromMetal();
    invalidateMetalResidentState();
#endif

    const auto mask = qubitMask(index);
    double probabilityOne = 0.0;
#if defined(_OPENMP)
    const auto threadCount = parallelThreadCount();
#pragma omp parallel for reduction(+ : probabilityOne)                         \
    num_threads(threadCount) if (threadCount > 1 &&                            \
                                     state.size() >= parallelStateThreshold)
#endif
    for (std::size_t basis = 0; basis < state.size(); ++basis)
      if (basis & mask)
        probabilityOne += std::norm(state[basis]);

    probabilityOne = std::clamp(probabilityOne, 0.0, 1.0);
    std::bernoulli_distribution distribution(probabilityOne);
    const bool result = distribution(randomEngine);
    const auto probability = result ? probabilityOne : 1.0 - probabilityOne;
    const auto norm = std::sqrt(probability);

    if (norm <= zeroTolerance)
      throw std::runtime_error(
          MKLQ_SIMULATOR_DIAGNOSTIC_PREFIX
          " sampled a zero-probability measurement branch.");

#if defined(_OPENMP)
#pragma omp parallel for num_threads(                                          \
        threadCount) if (threadCount > 1 &&                                    \
                             state.size() >= parallelStateThreshold)
#endif
    for (std::size_t basis = 0; basis < state.size(); ++basis) {
      if (((basis & mask) != 0) == result)
        state[basis] /= norm;
      else
        state[basis] = 0.0;
    }

    return result;
  }

  double calculateExpectationValue(const std::vector<std::size_t> &qubits) {
    validateSampleQubits(qubits);

#if defined(MKLQ_ENABLE_METAL_RUNTIME)
    if (metalExecutor.hasResidentState(state.size())) {
      throwIfMetalResidentStatePoisoned("cannot compute expectation from");
      if (!flushMetalResidentGateCommands()) {
        const auto error = fmt::format(
            MKLQ_SIMULATOR_DIAGNOSTIC_PREFIX
            " failed to flush Metal resident gates before computing "
            "expectation: {}",
            metalExecutor.lastError());
        markMetalResidentStatePoisoned(error);
        throw std::runtime_error(error);
      }

      double expectation = 0.0;
      if (computeMetalResidentZParityExpectation(qubits.data(), qubits.size(),
                                                 expectation))
        return expectation;

      synchronizeHostStateFromMetal();
      invalidateMetalResidentState();
    }

    if (metalStateHostDirty) {
      synchronizeHostStateFromMetal();
      invalidateMetalResidentState();
    }
#endif

    std::size_t bitmask = 0;
    for (auto qubit : qubits)
      bitmask |= qubitMask(qubit);

    double expectation = 0.0;
#if defined(_OPENMP)
    const auto threadCount = parallelThreadCount();
#pragma omp parallel for reduction(+ : expectation)                            \
    num_threads(threadCount) if (threadCount > 1 &&                            \
                                     state.size() >= parallelStateThreshold)
#endif
    for (std::size_t basis = 0; basis < state.size(); ++basis) {
      const auto evenParity = std::popcount(basis & bitmask) % 2 == 0;
      expectation += (evenParity ? 1.0 : -1.0) * std::norm(state[basis]);
    }
    return expectation;
  }

  std::string outcomeToBitString(std::size_t outcome,
                                 std::size_t bitCount) const {
#if defined(MKLQ_ENABLE_TEST_ACCESSORS)
    ++bitStringConversions;
#endif
    std::string bits;
    bits.reserve(bitCount);
    for (std::size_t bit = 0; bit < bitCount; ++bit)
      bits.push_back((outcome & (1ULL << bit)) ? '1' : '0');
    return bits;
  }

  void appendSampleOutcomeCount(cudaq::ExecutionResult &counts,
                                std::size_t outcome, std::size_t bitCount,
                                bool includeSequentialData,
                                std::size_t count) const {
    const auto bitString = outcomeToBitString(outcome, bitCount);
    if (includeSequentialData)
      counts.appendResult(bitString, count);
    else
      counts.counts[bitString] += count;
  }

  void appendSampleOutcome(cudaq::ExecutionResult &counts, std::size_t outcome,
                           std::size_t bitCount,
                           bool includeSequentialData) const {
    appendSampleOutcomeCount(counts, outcome, bitCount, includeSequentialData,
                             1);
  }

  void appendSampleOutcomeCounts(cudaq::ExecutionResult &counts,
                                 const std::vector<std::size_t> &drawCounts,
                                 std::size_t bitCount) const {
    for (std::size_t outcome = 0; outcome < drawCounts.size(); ++outcome) {
      const auto count = drawCounts[outcome];
      if (count == 0)
        continue;
      appendSampleOutcomeCount(counts, outcome, bitCount, false, count);
    }
  }

  void appendSampleOutcomeCounts(
      cudaq::ExecutionResult &counts,
      const std::unordered_map<std::size_t, std::size_t> &drawCounts,
      std::size_t bitCount) const {
    for (const auto &[outcome, count] : drawCounts)
      appendSampleOutcomeCount(counts, outcome, bitCount, false, count);
  }

  std::optional<std::size_t>
  deterministicSampleOutcome(std::span<const double> probabilities) const {
    std::optional<std::size_t> deterministicOutcome;
    for (std::size_t outcome = 0; outcome < probabilities.size(); ++outcome) {
      if (probabilities[outcome] <= zeroTolerance)
        continue;
      if (deterministicOutcome)
        return std::nullopt;
      deterministicOutcome = outcome;
    }
    return deterministicOutcome;
  }

  bool tryAppendDeterministicSampleOutcome(
      cudaq::ExecutionResult &counts, std::span<const double> probabilities,
      int shots, std::size_t bitCount, bool includeSequentialData) const {
    const auto outcome = deterministicSampleOutcome(probabilities);
    if (!outcome)
      return false;
    appendSampleOutcomeCount(counts, *outcome, bitCount, includeSequentialData,
                             static_cast<std::size_t>(shots));
    return true;
  }

  std::vector<std::size_t>
  drawDenseOutcomeCounts(const std::vector<double> &probabilities, int shots) {
#if defined(MKLQ_ENABLE_TEST_ACCESSORS)
    ++denseDrawCountBuffers;
#endif
    std::discrete_distribution<std::size_t> distribution(probabilities.begin(),
                                                         probabilities.end());
    std::vector<std::size_t> drawCounts(probabilities.size(), 0);
    for (int shot = 0; shot < shots; ++shot)
      ++drawCounts[distribution(randomEngine)];
    return drawCounts;
  }

  std::unordered_map<std::size_t, std::size_t>
  drawSparseOutcomeCounts(const std::vector<double> &probabilities, int shots) {
#if defined(MKLQ_ENABLE_TEST_ACCESSORS)
    ++sparseDrawCountMaps;
    ++sortedSparseDrawCountMaps;
#endif
    const auto totalWeight =
        std::accumulate(probabilities.begin(), probabilities.end(), 0.0);
    std::uniform_real_distribution<double> distribution(0.0, totalWeight);
    std::vector<double> draws;
    draws.reserve(static_cast<std::size_t>(shots));
    for (int shot = 0; shot < shots; ++shot)
      draws.push_back(distribution(randomEngine));
    std::sort(draws.begin(), draws.end());

    std::unordered_map<std::size_t, std::size_t> drawCounts;
    drawCounts.reserve(
        std::min(probabilities.size(), static_cast<std::size_t>(shots)));

    std::size_t drawIndex = 0;
    std::size_t lastPositiveOutcome = 0;
    double cumulative = 0.0;
    for (std::size_t outcome = 0; outcome < probabilities.size(); ++outcome) {
      const auto probability = probabilities[outcome];
      if (probability == 0.0)
        continue;
      lastPositiveOutcome = outcome;
      cumulative += probability;
      while (drawIndex < draws.size() && draws[drawIndex] < cumulative) {
        ++drawCounts[outcome];
        ++drawIndex;
      }
    }

    while (drawIndex < draws.size()) {
      ++drawCounts[lastPositiveOutcome];
      ++drawIndex;
    }
    return drawCounts;
  }

  void drawAndAppendSampleOutcomeCounts(
      cudaq::ExecutionResult &counts, const std::vector<double> &probabilities,
      int shots, std::size_t bitCount) {
#if defined(MKLQ_ENABLE_TEST_ACCESSORS)
    ScopedTestTimer sampleDrawTimer(sampleDrawAndCountSeconds);
    ++countsOnlySampleDrawBatches;
#endif
    if (probabilities.size() <= denseDrawCountOutcomeLimit) {
      appendSampleOutcomeCounts(counts,
                                drawDenseOutcomeCounts(probabilities, shots),
                                bitCount);
      return;
    }

    appendSampleOutcomeCounts(counts, drawSparseOutcomeCounts(probabilities,
                                                              shots),
                              bitCount);
  }

#if defined(MKLQ_ENABLE_METAL_RUNTIME)
  bool tryDrawAndAppendMetalSampleOutcomeCounts(
      cudaq::ExecutionResult &counts, const std::vector<double> &probabilities,
      int shots, std::size_t bitCount) {
    if (!metalExecutor.available() || probabilities.empty() ||
        probabilities.size() > denseDrawCountOutcomeLimit || shots < 1 ||
        static_cast<std::uint64_t>(shots) >
            std::numeric_limits<std::uint32_t>::max())
      return false;

    const double totalWeight =
        std::accumulate(probabilities.begin(), probabilities.end(), 0.0);
    if (!(totalWeight > 0.0))
      return false;

#if defined(MKLQ_ENABLE_TEST_ACCESSORS)
    const auto sampleDrawStart = std::chrono::steady_clock::now();
#endif
    std::vector<std::uint32_t> drawCounts(probabilities.size(), 0);
    const auto seed = static_cast<std::uint64_t>(randomEngine());
    if (!metalExecutor.accumulateGeneratedSampleCounts(
            probabilities.data(), probabilities.size(), seed,
            static_cast<std::size_t>(shots), drawCounts.data(),
            drawCounts.size()))
      return false;

    for (std::size_t outcome = 0; outcome < drawCounts.size(); ++outcome) {
      const auto count = drawCounts[outcome];
      if (count == 0)
        continue;
      appendSampleOutcomeCount(counts, outcome, bitCount, false,
                               static_cast<std::size_t>(count));
    }
#if defined(MKLQ_ENABLE_TEST_ACCESSORS)
    sampleDrawAndCountSeconds +=
        std::chrono::duration<double>(std::chrono::steady_clock::now() -
                                      sampleDrawStart)
            .count();
#endif
    return true;
  }
#endif

  void setExpectationFromSampleCounts(cudaq::ExecutionResult &counts,
                                      int shots) const {
#if defined(MKLQ_ENABLE_TEST_ACCESSORS)
    ScopedTestTimer sampleExpectationTimer(sampleExpectationReductionSeconds);
    ++sampleExpectationReductions;
#endif
    double expectation = 0.0;
    for (auto &[bits, count] : counts.counts) {
      const auto sign =
          cudaq::sample_result::has_even_parity(bits) ? 1.0 : -1.0;
      expectation += sign * (static_cast<double>(count) / shots);
    }
    counts.expectationValue = expectation;
  }

  void fillFullRegisterProbabilities(std::vector<double> &probabilities) const {
    if (probabilities.size() != state.size())
      throw std::runtime_error(
          MKLQ_SIMULATOR_DIAGNOSTIC_PREFIX
          " probability buffer does not match state size.");

#if defined(MKLQ_ENABLE_TEST_ACCESSORS)
    ++fullRegisterProbabilityFills;
#endif

#if defined(MKLQ_ENABLE_METAL_RUNTIME)
    if (metalExecutor.hasResidentState(state.size())) {
      throwIfMetalResidentStatePoisoned("cannot fill probabilities from");
    }
    if (metalExecutor.hasResidentState(state.size()) &&
        metalExecutor.fillResidentFullRegisterProbabilities(
            probabilities.data(), probabilities.size()))
      return;
    if (metalStateHostDirty) {
      auto *self = const_cast<MKLQ_SIMULATOR_CLASS *>(this);
      self->synchronizeHostStateFromMetal();
      self->invalidateMetalResidentState();
    }
    if (metalExecutor.fillFullRegisterProbabilities(state.data(), state.size(),
                                                    probabilities.data(),
                                                    probabilities.size()))
      return;
#endif

#if defined(_OPENMP)
    const auto threadCount = parallelThreadCount();
#pragma omp parallel for num_threads(                                          \
        threadCount) if (threadCount > 1 &&                                    \
                             state.size() >= parallelProbabilityThreshold)
#endif
    for (std::size_t index = 0; index < state.size(); ++index) {
      const auto real = state[index].real();
      const auto imag = state[index].imag();
      probabilities[index] = real * real + imag * imag;
    }
  }

#if defined(MKLQ_ENABLE_METAL_RUNTIME)
  bool shouldUseMetalResidentMarginalProbabilities(
      std::size_t probabilityCount) const {
    const auto groupCount =
        (state.size() + mklq::marginalProbabilityThreadsPerThreadgroup - 1) /
        mklq::marginalProbabilityThreadsPerThreadgroup;
    if (groupCount == 0)
      return false;

    // The marginal Metal kernel writes one partial-sum per outcome per
    // threadgroup. When that scratch work is no smaller than a resident
    // full-register probability fill, filling the full distribution and folding
    // on the host is the faster measured path on Apple Silicon.
    return probabilityCount < state.size() / groupCount;
  }
#endif

  void fillMarginalProbabilities(std::vector<double> &probabilities,
                                 const std::vector<std::size_t> &qubits) {
#if defined(MKLQ_ENABLE_TEST_ACCESSORS)
    ++marginalProbabilityFills;
#endif
    auto foldFullRegisterProbabilities =
        [&](const std::vector<double> &fullRegisterProbabilities) {
          std::fill(probabilities.begin(), probabilities.end(), 0.0);
          for (std::size_t basis = 0; basis < fullRegisterProbabilities.size();
               ++basis) {
            std::size_t outcome = 0;
            for (std::size_t bit = 0; bit < qubits.size(); ++bit)
              if (basis & qubitMask(qubits[bit]))
                outcome |= (1ULL << bit);
            probabilities[outcome] += fullRegisterProbabilities[basis];
          }
        };

#if defined(MKLQ_ENABLE_METAL_RUNTIME)
    if (metalExecutor.hasResidentState(state.size())) {
      throwIfMetalResidentStatePoisoned(
          "cannot fill marginal probabilities from");
      if (shouldUseMetalResidentMarginalProbabilities(probabilities.size()) &&
          fillMetalResidentMarginalProbabilities(
              qubits.data(), qubits.size(), probabilities.data(),
              probabilities.size()))
        return;

      std::vector<double> fullRegisterProbabilities(state.size(), 0.0);
      fillFullRegisterProbabilities(fullRegisterProbabilities);
      foldFullRegisterProbabilities(fullRegisterProbabilities);
      return;
    }
    if (metalStateHostDirty) {
      synchronizeHostStateFromMetal();
      invalidateMetalResidentState();
    }
#endif

    std::fill(probabilities.begin(), probabilities.end(), 0.0);
    for (std::size_t basis = 0; basis < state.size(); ++basis) {
      std::size_t outcome = 0;
      for (std::size_t bit = 0; bit < qubits.size(); ++bit)
        if (basis & qubitMask(qubits[bit]))
          outcome |= (1ULL << bit);
      probabilities[outcome] += std::norm(state[basis]);
    }
  }

  bool samplesFullRegisterInNaturalOrder(
      const std::vector<std::size_t> &qubits) const {
    if (qubits.size() != nQubitsAllocated)
      return false;
    for (std::size_t bit = 0; bit < qubits.size(); ++bit)
      if (qubits[bit] != bit)
        return false;
    return true;
  }

  bool trySampleSparseFullRegister(cudaq::ExecutionResult &counts, int shots,
                                   bool includeSequentialData) {
#if defined(MKLQ_ENABLE_TEST_ACCESSORS)
    ++sparseFullRegisterScans;
#endif
#if defined(MKLQ_ENABLE_METAL_RUNTIME)
    if (metalStateHostDirty) {
      synchronizeHostStateFromMetal();
      invalidateMetalResidentState();
    }
#endif

    std::vector<std::size_t> outcomes;
    std::vector<double> probabilities;
    outcomes.reserve(sparseSamplingOutcomeLimit);
    probabilities.reserve(sparseSamplingOutcomeLimit);

    for (std::size_t basis = 0; basis < state.size(); ++basis) {
      const auto probability = std::norm(state[basis]);
      if (probability == 0.0)
        continue;
      if (probabilities.size() == sparseSamplingOutcomeLimit) {
#if defined(MKLQ_ENABLE_TEST_ACCESSORS)
        ++sparseFullRegisterScanMisses;
#endif
        return false;
      }
      outcomes.push_back(basis);
      probabilities.push_back(probability);
    }

    if (probabilities.empty())
      throw std::runtime_error(MKLQ_SIMULATOR_DIAGNOSTIC_PREFIX
                               " cannot sample a zero-norm state.");
    validateProbabilityWeights(probabilities, "sparse sampler");

#if defined(MKLQ_ENABLE_TEST_ACCESSORS)
    ++sparseFullRegisterScanHits;
#endif

    if (outcomes.size() == 1) {
      appendSampleOutcomeCount(counts, outcomes.front(), nQubitsAllocated,
                               includeSequentialData,
                               static_cast<std::size_t>(shots));
      setExpectationFromSampleCounts(counts, shots);
      return true;
    }

#if defined(MKLQ_ENABLE_TEST_ACCESSORS)
    if (includeSequentialData)
      ++sequentialSampleDrawBatches;
    else
      ++countsOnlySampleDrawBatches;
#endif

    std::discrete_distribution<std::size_t> distribution(probabilities.begin(),
                                                         probabilities.end());
    for (int shot = 0; shot < shots; ++shot)
      appendSampleOutcome(counts, outcomes[distribution(randomEngine)],
                          nQubitsAllocated, includeSequentialData);

    setExpectationFromSampleCounts(counts, shots);
    return true;
  }

  cudaq::ExecutionResult sample(const std::vector<std::size_t> &qubits,
                                const int shots,
                                bool includeSequentialData = true) override {
    validateSampleQubits(qubits);

    if (shots < 1)
      return cudaq::ExecutionResult{{}, calculateExpectationValue(qubits)};

    if (samplesFullRegisterInNaturalOrder(qubits)) {
      cudaq::ExecutionResult counts;
#if defined(MKLQ_ENABLE_METAL_RUNTIME)
      if (!metalStateHostDirty &&
          trySampleSparseFullRegister(counts, shots, includeSequentialData))
        return counts;
#else
      if (trySampleSparseFullRegister(counts, shots, includeSequentialData))
        return counts;
#endif

      std::vector<double> probabilities(state.size(), 0.0);
      {
#if defined(MKLQ_ENABLE_TEST_ACCESSORS)
        ScopedTestTimer probabilityFillTimer(sampleProbabilityFillSeconds);
#endif
        fillFullRegisterProbabilities(probabilities);
      }
      validateProbabilityWeights(probabilities, "full-register sampler");
      if (tryAppendDeterministicSampleOutcome(
              counts, probabilities, shots, qubits.size(),
              includeSequentialData)) {
        setExpectationFromSampleCounts(counts, shots);
        return counts;
      }
      if (!includeSequentialData) {
#if defined(MKLQ_ENABLE_METAL_RUNTIME)
        if (tryDrawAndAppendMetalSampleOutcomeCounts(counts, probabilities,
                                                     shots, qubits.size())) {
          setExpectationFromSampleCounts(counts, shots);
          return counts;
        }
#endif
        drawAndAppendSampleOutcomeCounts(counts, probabilities, shots,
                                         qubits.size());
        setExpectationFromSampleCounts(counts, shots);
        return counts;
      }

      {
#if defined(MKLQ_ENABLE_TEST_ACCESSORS)
        ScopedTestTimer sampleDrawTimer(sampleDrawAndCountSeconds);
#endif
        std::discrete_distribution<std::size_t> distribution(
            probabilities.begin(), probabilities.end());
#if defined(MKLQ_ENABLE_TEST_ACCESSORS)
        ++sequentialSampleDrawBatches;
#endif
        for (int shot = 0; shot < shots; ++shot) {
          const auto outcome = distribution(randomEngine);
          appendSampleOutcome(counts, outcome, qubits.size(),
                              includeSequentialData);
        }
      }

      setExpectationFromSampleCounts(counts, shots);
      return counts;
    }

    const auto outcomeCount = 1ULL << qubits.size();
    std::vector<double> probabilities(outcomeCount, 0.0);
    {
#if defined(MKLQ_ENABLE_TEST_ACCESSORS)
      ScopedTestTimer probabilityFillTimer(sampleProbabilityFillSeconds);
#endif
      fillMarginalProbabilities(probabilities, qubits);
    }
    validateProbabilityWeights(probabilities, "marginal sampler");

    cudaq::ExecutionResult counts;
    if (tryAppendDeterministicSampleOutcome(counts, probabilities, shots,
                                            qubits.size(),
                                            includeSequentialData)) {
      setExpectationFromSampleCounts(counts, shots);
      return counts;
    }
    if (!includeSequentialData) {
#if defined(MKLQ_ENABLE_METAL_RUNTIME)
      if (tryDrawAndAppendMetalSampleOutcomeCounts(counts, probabilities,
                                                   shots, qubits.size())) {
        setExpectationFromSampleCounts(counts, shots);
        return counts;
      }
#endif
      drawAndAppendSampleOutcomeCounts(counts, probabilities, shots,
                                       qubits.size());
      setExpectationFromSampleCounts(counts, shots);
      return counts;
    }

    {
#if defined(MKLQ_ENABLE_TEST_ACCESSORS)
      ScopedTestTimer sampleDrawTimer(sampleDrawAndCountSeconds);
#endif
      std::discrete_distribution<std::size_t> distribution(
          probabilities.begin(), probabilities.end());
#if defined(MKLQ_ENABLE_TEST_ACCESSORS)
      ++sequentialSampleDrawBatches;
#endif
      for (int shot = 0; shot < shots; ++shot) {
        const auto outcome = distribution(randomEngine);
        appendSampleOutcome(counts, outcome, qubits.size(),
                            includeSequentialData);
      }
    }

    setExpectationFromSampleCounts(counts, shots);
    return counts;
  }

  std::unique_ptr<cudaq::SimulationState> getSimulationState() override {
    flushGateQueue();
#if defined(MKLQ_ENABLE_METAL_RUNTIME)
    synchronizeHostStateFromMetal();
#endif
    return std::make_unique<MklqCpuState>(state);
  }

public:
  MKLQ_SIMULATOR_CLASS() { summaryData.name = name(); }
  ~MKLQ_SIMULATOR_CLASS() override = default;

  void setRandomSeed(std::size_t seed) override { randomEngine.seed(seed); }

  void resetQubit(const std::size_t index) override {
    flushGateQueue();
    flushAnySamplingTasks();

    const bool measuredOne = measureQubit(index);
    if (!measuredOne)
      return;

#if defined(MKLQ_ENABLE_METAL_RUNTIME)
    if (metalStateHostDirty && metalExecutor.hasResidentState(state.size())) {
      throwIfMetalResidentStatePoisoned("cannot reset");
      const std::array<complexd, 4> xGate{
          complexd{0.0, 0.0}, complexd{1.0, 0.0}, complexd{1.0, 0.0},
          complexd{0.0, 0.0}};
      if (!applyMetalResidentResetGate(index, xGate.data())) {
        const auto error =
            fmt::format(MKLQ_SIMULATOR_DIAGNOSTIC_PREFIX
                        " failed to reset Metal resident qubit: {}",
                        metalExecutor.lastError());
        markMetalResidentStatePoisoned(error);
        throw std::runtime_error(error);
      }
      metalStateHostDirty = true;
      return;
    }
#endif

    const auto mask = qubitMask(index);
    std::vector<complexd> next(state.size(), complexd{0.0, 0.0});
#if defined(_OPENMP)
    const auto threadCount = parallelThreadCount();
#pragma omp parallel for num_threads(                                          \
        threadCount) if (threadCount > 1 &&                                    \
                             state.size() >= parallelStateThreshold)
#endif
    for (std::size_t basis = 0; basis < state.size(); ++basis)
      if (basis & mask)
        next[basis & ~mask] = state[basis];
    state = std::move(next);
  }

  std::unique_ptr<cudaq::SimulationState>
  createStateFromData(const cudaq::state_data &data) override {
    return std::make_unique<MklqCpuState>()->createFromData(data);
  }

  bool isStateVectorSimulator() const override { return true; }

  bool canOmitSequentialDataForNonExplicitSampling() const override {
    return true;
  }

  std::string name() const override { return MKLQ_SIMULATOR_BACKEND_NAME; }

  NVQIR_SIMULATOR_CLONE_IMPL(MKLQ_SIMULATOR_CLASS)
};

} // namespace nvqir

NVQIR_REGISTER_SIMULATOR(nvqir::MKLQ_SIMULATOR_CLASS,
                         MKLQ_SIMULATOR_PRINTED_NAME)
