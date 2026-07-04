# ============================================================================ #
# Copyright (c) 2026 Linsen Wu.                                                #
#                                                                              #
# This source code and the accompanying materials are made available under     #
# the terms of the Apache License 2.0 which accompanies this distribution.     #
# ============================================================================ #

import numpy as np
import pytest

import cudaq
from cudaq import spin
from mklq_test_utils import mklq_targets_available


pytestmark = pytest.mark.skipif(not mklq_targets_available(),
                                reason="MKL-Q targets are not available")


@pytest.fixture(autouse=True)
def reset_target_after_test():
    yield
    cudaq.reset_target()
    cudaq.__clearKernelRegistries()


def _state_for_target(target, kernel, *args):
    cudaq.set_target(target)
    try:
        return np.array(cudaq.get_state(kernel, *args), dtype=np.complex128)
    finally:
        cudaq.reset_target()


def _expectation_for_target(target, kernel, observable, *args):
    cudaq.set_target(target)
    try:
        return cudaq.observe(kernel, observable, *args).expectation()
    finally:
        cudaq.reset_target()


def _counts_for_target(target, kernel, shots):
    cudaq.set_target(target)
    try:
        if hasattr(cudaq, "set_random_seed"):
            cudaq.set_random_seed(23)
        return {bits: count for bits, count in cudaq.sample(
            kernel, shots_count=shots).items()}
    finally:
        cudaq.reset_target()


def _assert_counts_are_close_to_probabilities(counts, expected, shots):
    observed_total = sum(counts.values())
    assert observed_total == shots, (
        f"expected {shots} shots, observed {observed_total}; counts={counts}")

    unexpected = sorted(bits for bits, count in counts.items()
                        if count and expected.get(bits, 0.0) == 0.0)
    assert not unexpected, (
        f"unexpected non-zero sample outcomes {unexpected}; "
        f"expected support={sorted(expected)} counts={counts}")

    for bits, probability in expected.items():
        observed = counts.get(bits, 0)
        expected_count = shots * probability
        variance = shots * probability * (1.0 - probability)
        tolerance = max(8.0, 6.0 * np.sqrt(variance))
        assert abs(observed - expected_count) <= tolerance, (
            f"outcome {bits}: observed {observed}, expected "
            f"{expected_count:.2f} +/- {tolerance:.2f}; "
            f"probability={probability:.8f}, counts={counts}")


def _marginal_probabilities_from_state(state, measured_qubits):
    measured_qubits = tuple(sorted(measured_qubits))
    probabilities = {
        format(outcome, f"0{len(measured_qubits)}b")[::-1]: 0.0
        for outcome in range(1 << len(measured_qubits))
    }

    for basis, amplitude in enumerate(state):
        outcome = 0
        for bit, qubit in enumerate(measured_qubits):
            if basis & (1 << qubit):
                outcome |= 1 << bit
        probabilities[format(outcome, f"0{len(measured_qubits)}b")[::-1]] += (
            float(np.abs(amplitude)**2))

    return probabilities


def _assert_matches_qpp(kernel, *args, rtol=1.0e-12, atol=1.0e-12):
    reference = _state_for_target("qpp-cpu", kernel, *args)
    actual = _state_for_target("mklq-cpu", kernel, *args)

    assert np.allclose(actual, reference, rtol=rtol, atol=atol)


def _bell_kernel():
    kernel = cudaq.make_kernel()
    qubits = kernel.qalloc(2)
    kernel.h(qubits[0])
    kernel.cx(qubits[0], qubits[1])
    return kernel


def _ghz_kernel(qubit_count):
    kernel = cudaq.make_kernel()
    qubits = kernel.qalloc(qubit_count)
    kernel.h(qubits[0])
    for index in range(qubit_count - 1):
        kernel.cx(qubits[index], qubits[index + 1])
    return kernel


def _qft_like_kernel(qubit_count):
    kernel = cudaq.make_kernel()
    qubits = kernel.qalloc(qubit_count)
    kernel.x(qubits[0])
    kernel.x(qubits[qubit_count - 1])

    for target in range(qubit_count):
        kernel.h(qubits[target])
        for control in range(target + 1, qubit_count):
            angle = np.pi / float(1 << (control - target + 1))
            kernel.crz(angle, qubits[control], qubits[target])

    for index in range(qubit_count // 2):
        kernel.swap(qubits[index], qubits[qubit_count - index - 1])

    return kernel


def _deterministic_clifford_kernel():
    kernel = cudaq.make_kernel()
    qubits = kernel.qalloc(5)

    kernel.h(qubits[0])
    kernel.s(qubits[0])
    kernel.x(qubits[3])
    kernel.h(qubits[2])
    kernel.cx(qubits[0], qubits[1])
    kernel.cz(qubits[2], qubits[3])
    kernel.h(qubits[4])
    kernel.s(qubits[4])
    kernel.cx(qubits[4], qubits[2])
    kernel.sdg(qubits[0])
    kernel.z(qubits[1])
    kernel.swap(qubits[1], qubits[3])
    kernel.cx(qubits[3], qubits[4])
    kernel.h(qubits[1])
    kernel.s(qubits[2])
    kernel.cz(qubits[0], qubits[4])

    return kernel


def _seeded_clifford_kernel(qubit_count, seed):
    kernel = cudaq.make_kernel()
    qubits = kernel.qalloc(qubit_count)

    for layer in range(3 * qubit_count):
        target = (seed + 2 * layer) % qubit_count
        selector = (seed + 5 * layer) % 6
        if selector == 0:
            kernel.h(qubits[target])
        elif selector == 1:
            kernel.s(qubits[target])
        elif selector == 2:
            kernel.sdg(qubits[target])
        elif selector == 3:
            kernel.x(qubits[target])
        elif selector == 4:
            kernel.y(qubits[target])
        else:
            kernel.z(qubits[target])

        control = (target + 1 + layer) % qubit_count
        if control == target:
            control = (control + 1) % qubit_count
        other = (target + 2 + seed + layer) % qubit_count
        while other in {target, control}:
            other = (other + 1) % qubit_count

        if layer % 3 == 0:
            kernel.cx(qubits[control], qubits[target])
        elif layer % 3 == 1:
            kernel.cz(qubits[control], qubits[target])
        else:
            kernel.swap(qubits[target], qubits[other])

    return kernel


def _parameterized_fixture_kernel():
    kernel, theta, phi = cudaq.make_kernel(float, float)
    qubits = kernel.qalloc(4)

    kernel.ry(theta, qubits[0])
    kernel.rx(phi, qubits[1])
    kernel.h(qubits[2])
    kernel.cx(qubits[0], qubits[2])
    kernel.rz(theta, qubits[2])
    kernel.ry(phi, qubits[3])
    kernel.cx(qubits[3], qubits[1])
    kernel.cz(qubits[2], qubits[3])

    return kernel


def _hardware_efficient_ansatz_kernel(qubit_count, layers):
    kernel = cudaq.make_kernel()
    qubits = kernel.qalloc(qubit_count)

    for layer in range(layers):
        for index in range(qubit_count):
            theta = 0.037 * (layer + 1) + 0.011 * (index + 1)
            phi = -0.021 * (layer + 2) + 0.007 * index
            lam = 0.019 * (index + 2) - 0.005 * layer
            kernel.ry(theta, qubits[index])
            kernel.rz(phi, qubits[index])
            kernel.rx(lam, qubits[index])

        for index in range(0, qubit_count - 1, 2):
            kernel.cx(qubits[index], qubits[index + 1])
            kernel.crz(0.043 * (layer + 1) + 0.003 * index, qubits[index],
                       qubits[index + 1])

        for index in range(1, qubit_count - 1, 2):
            kernel.cz(qubits[index], qubits[index + 1])
            kernel.crx(-0.029 * (layer + 1) + 0.002 * index, qubits[index],
                       qubits[index + 1])

        if qubit_count > 3:
            kernel.swap(qubits[layer % qubit_count],
                        qubits[(layer + 3) % qubit_count])

    return kernel


def _phase_family_fixture_kernel():
    kernel = cudaq.make_kernel()
    qubits = kernel.qalloc(5)

    for index in range(5):
        kernel.h(qubits[index])

    kernel.s(qubits[0])
    kernel.t(qubits[1])
    kernel.sdg(qubits[2])
    kernel.tdg(qubits[3])
    kernel.r1(0.17, qubits[4])

    kernel.cs(qubits[0], qubits[1])
    kernel.ct(qubits[1], qubits[2])
    kernel.cr1(-0.29, qubits[2], qubits[3])
    kernel.crz(0.43, qubits[3], qubits[4])
    kernel.cz(qubits[4], qubits[0])

    kernel.cx(qubits[0], qubits[3])
    kernel.rz(-0.11, qubits[3])
    kernel.cs(qubits[3], qubits[0])
    kernel.ct(qubits[4], qubits[2])

    return kernel


def _partial_register_bit_order_kernel(measurement_order):
    kernel = cudaq.make_kernel()
    qubits = kernel.qalloc(4)
    kernel.x(qubits[0])
    kernel.x(qubits[2])
    for index in measurement_order:
        kernel.mz(qubits[index])
    return kernel


def _apply_sampling_distribution_circuit(kernel, qubits):
    kernel.h(qubits[0])
    kernel.ry(0.73, qubits[1])
    kernel.cx(qubits[0], qubits[2])
    kernel.x(qubits[3])
    kernel.ry(-0.41, qubits[4])
    kernel.crz(0.19, qubits[1], qubits[4])


def _sampling_distribution_state_kernel():
    kernel = cudaq.make_kernel()
    qubits = kernel.qalloc(5)
    _apply_sampling_distribution_circuit(kernel, qubits)

    return kernel


def _sampling_distribution_sample_kernel(measurement_order):
    kernel = cudaq.make_kernel()
    qubits = kernel.qalloc(5)
    _apply_sampling_distribution_circuit(kernel, qubits)
    for index in measurement_order:
        kernel.mz(qubits[index])
    return kernel


def test_mklq_cpu_bell_state_matches_analytic_fixture():
    state = _state_for_target("mklq-cpu", _bell_kernel())
    expected = np.array([1.0 / np.sqrt(2.0), 0.0, 0.0,
                         1.0 / np.sqrt(2.0)],
                        dtype=np.complex128)

    assert np.allclose(state, expected, rtol=1.0e-12, atol=1.0e-12)


def test_mklq_cpu_ghz_state_matches_analytic_fixture():
    qubit_count = 5
    state = _state_for_target("mklq-cpu", _ghz_kernel(qubit_count))
    expected = np.zeros(1 << qubit_count, dtype=np.complex128)
    expected[0] = 1.0 / np.sqrt(2.0)
    expected[-1] = 1.0 / np.sqrt(2.0)

    assert np.allclose(state, expected, rtol=1.0e-12, atol=1.0e-12)


def test_mklq_cpu_ghz_sampling_has_entangled_support():
    kernel = _ghz_kernel(5)
    counts = _counts_for_target("mklq-cpu", kernel, shots=256)
    observed = {bits for bits, count in counts.items() if count}

    assert observed <= {"00000", "11111"}
    assert observed == {"00000", "11111"}


@pytest.mark.parametrize("qubit_count", [4, 5])
def test_mklq_cpu_qft_like_fixture_matches_qpp(qubit_count):
    _assert_matches_qpp(_qft_like_kernel(qubit_count))


def test_mklq_cpu_deterministic_clifford_fixture_matches_qpp():
    _assert_matches_qpp(_deterministic_clifford_kernel())


@pytest.mark.parametrize("seed", [7, 19, 31])
def test_mklq_cpu_seeded_clifford_fixture_matches_qpp(seed):
    _assert_matches_qpp(_seeded_clifford_kernel(6, seed))


def test_mklq_cpu_parameterized_fixture_matches_qpp_state():
    _assert_matches_qpp(_parameterized_fixture_kernel(), 0.37, -0.21)


def test_mklq_cpu_parameterized_fixture_matches_qpp_observable():
    kernel = _parameterized_fixture_kernel()
    observable = (0.25 * spin.z(0) - 0.5 * spin.x(1) +
                  0.75 * spin.z(2) * spin.z(3) +
                  0.125 * spin.y(0) * spin.y(3))

    reference = _expectation_for_target("qpp-cpu", kernel, observable, -0.43,
                                        0.19)
    actual = _expectation_for_target("mklq-cpu", kernel, observable, -0.43,
                                     0.19)

    assert np.isclose(actual, reference, rtol=1.0e-12, atol=1.0e-12)


def test_mklq_cpu_hardware_efficient_ansatz_matches_qpp_state():
    _assert_matches_qpp(_hardware_efficient_ansatz_kernel(7, 3))


def test_mklq_cpu_hardware_efficient_ansatz_matches_qpp_observable():
    kernel = _hardware_efficient_ansatz_kernel(7, 3)
    observable = (0.125 * spin.z(0) * spin.z(6) -
                  0.375 * spin.x(1) * spin.x(4) +
                  0.5 * spin.y(2) * spin.z(5) +
                  0.25 * spin.z(3))

    reference = _expectation_for_target("qpp-cpu", kernel, observable)
    actual = _expectation_for_target("mklq-cpu", kernel, observable)

    assert np.isclose(actual, reference, rtol=1.0e-12, atol=1.0e-12)


def test_mklq_cpu_phase_family_fixture_matches_qpp_state():
    _assert_matches_qpp(_phase_family_fixture_kernel())


@pytest.mark.parametrize("measurement_order",
                         ([0, 2, 3], [3, 2, 0], [2, 0, 3]))
def test_mklq_cpu_partial_register_sampling_uses_natural_bit_order(
        measurement_order):
    counts = _counts_for_target(
        "mklq-cpu", _partial_register_bit_order_kernel(measurement_order),
        shots=64)

    assert counts == {"110": 64}


def test_mklq_cpu_partial_register_sampling_matches_qpp_marginal_oracle():
    measured_qubits = (4, 0, 2)
    shots = 4096
    state = _state_for_target("qpp-cpu", _sampling_distribution_state_kernel())
    expected = _marginal_probabilities_from_state(state, measured_qubits)
    counts = _counts_for_target(
        "mklq-cpu", _sampling_distribution_sample_kernel(measured_qubits),
        shots=shots)

    _assert_counts_are_close_to_probabilities(counts, expected, shots)
