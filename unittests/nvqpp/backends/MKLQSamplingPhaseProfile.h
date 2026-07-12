/*******************************************************************************
 * Copyright (c) 2026 Linsen Wu.                                                *
 *                                                                             *
 * This source code and the accompanying materials are made available under    *
 * the terms of the Apache License 2.0 which accompanies this distribution.     *
 ******************************************************************************/

#pragma once

#include <charconv>
#include <cstdlib>
#include <iomanip>
#include <limits>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace mklq_test {

struct SamplingPhaseProfileConfig {
  std::size_t qubitCount;
  std::size_t measuredQubitCount;
  int shots;
};

inline std::optional<std::string_view>
environmentValue(const char *name) {
  const auto *value = std::getenv(name);
  if (!value || !*value)
    return std::nullopt;
  return value;
}

inline std::size_t parsePositiveSize(std::string_view value,
                                     std::string_view name) {
  std::size_t parsed = 0;
  const auto [end, error] =
      std::from_chars(value.data(), value.data() + value.size(), parsed);
  if (error != std::errc{} || end != value.data() + value.size() ||
      parsed == 0)
    throw std::runtime_error("invalid positive integer in " +
                             std::string(name));
  return parsed;
}

inline std::string formatPhaseSeconds(double seconds) {
  std::ostringstream output;
  output << std::setprecision(17) << seconds;
  return output.str();
}

inline std::optional<SamplingPhaseProfileConfig>
samplingPhaseProfileConfigFromEnvironment() {
  if (!environmentValue("MKLQ_ENABLE_SAMPLING_PHASE_PROFILE"))
    return std::nullopt;

  const auto qubitCount = parsePositiveSize(
      environmentValue("MKLQ_SAMPLING_PHASE_PROFILE_QUBITS")
          .value_or("20"),
      "MKLQ_SAMPLING_PHASE_PROFILE_QUBITS");
  const auto measuredQubitCount = parsePositiveSize(
      environmentValue("MKLQ_SAMPLING_PHASE_PROFILE_MEASURED_QUBITS")
          .value_or("12"),
      "MKLQ_SAMPLING_PHASE_PROFILE_MEASURED_QUBITS");
  const auto shots = parsePositiveSize(
      environmentValue("MKLQ_SAMPLING_PHASE_PROFILE_SHOTS").value_or(
          "65536"),
      "MKLQ_SAMPLING_PHASE_PROFILE_SHOTS");

  if (qubitCount > 24)
    throw std::runtime_error(
        "MKLQ_SAMPLING_PHASE_PROFILE_QUBITS must be at most 24");
  if (measuredQubitCount > qubitCount)
    throw std::runtime_error(
        "MKLQ_SAMPLING_PHASE_PROFILE_MEASURED_QUBITS exceeds qubit count");
  if (shots > static_cast<std::size_t>(std::numeric_limits<int>::max()))
    throw std::runtime_error(
        "MKLQ_SAMPLING_PHASE_PROFILE_SHOTS exceeds the simulator shot range");

  return SamplingPhaseProfileConfig{
      qubitCount, measuredQubitCount, static_cast<int>(shots)};
}

} // namespace mklq_test
