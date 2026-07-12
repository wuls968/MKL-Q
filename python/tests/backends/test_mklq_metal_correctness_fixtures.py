# ============================================================================ #
# Copyright (c) 2026 Linsen Wu.                                                #
#                                                                              #
# This source code and the accompanying materials are made available under     #
# the terms of the Apache License 2.0 which accompanies this distribution.     #
# ============================================================================ #

import numpy as np
import pytest

import cudaq
from mklq_test_utils import mklq_targets_available


pytestmark = pytest.mark.skipif(not mklq_targets_available(),
                                reason="MKL-Q targets are not available")

METAL_RTOL = 1.0e-5
METAL_ATOL = 1.0e-5


@pytest.fixture(autouse=True)
def reset_target_after_test():
    yield
    cudaq.reset_target()
    cudaq.__clearKernelRegistries()


def _state_for_target(target, kernel):
    cudaq.set_target(target)
    try:
        return np.array(cudaq.get_state(kernel), dtype=np.complex128)
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


def _assert_metal_matches_qpp(kernel):
    reference = _state_for_target("qpp-cpu", kernel)
    actual = _state_for_target("mklq-metal", kernel)

    assert np.allclose(actual, reference, rtol=METAL_RTOL, atol=METAL_ATOL)


def _single_target_resident_kernel():
    kernel = cudaq.make_kernel()
    qubits = kernel.qalloc(5)

    for index in range(5):
        theta = 0.071 + 0.013 * index
        kernel.h(qubits[index])
        kernel.rx(theta, qubits[index])
        kernel.ry(-0.5 * theta, qubits[index])
        kernel.rz(theta, qubits[index])

    kernel.x(qubits[1])
    kernel.y(qubits[3])
    kernel.z(qubits[4])

    return kernel


def _controlled_resident_kernel():
    kernel = cudaq.make_kernel()
    qubits = kernel.qalloc(5)

    kernel.h(qubits[0])
    kernel.h(qubits[2])
    kernel.x(qubits[4])
    kernel.cx(qubits[0], qubits[1])
    kernel.cy(qubits[2], qubits[3])
    kernel.cz(qubits[4], qubits[0])
    kernel.crx(0.19, qubits[0], qubits[2])
    kernel.cry(-0.23, qubits[2], qubits[4])
    kernel.crz(0.31, qubits[1], qubits[3])

    return kernel


def _two_target_resident_kernel():
    kernel = cudaq.make_kernel()
    qubits = kernel.qalloc(4)

    kernel.h(qubits[0])
    kernel.x(qubits[2])
    kernel.swap(qubits[0], qubits[3])
    kernel.swap(qubits[1], qubits[2])
    kernel.cx(qubits[3], qubits[1])

    return kernel


def _controlled_swap_resident_kernel():
    kernel = cudaq.make_kernel()
    qubits = kernel.qalloc(5)

    kernel.h(qubits[0])
    kernel.x(qubits[1])
    kernel.ry(0.19, qubits[2])
    kernel.rz(-0.27, qubits[3])
    kernel.cswap(qubits[0], qubits[1], qubits[2])
    kernel.cswap(qubits[3], qubits[2], qubits[4])
    kernel.cx(qubits[4], qubits[1])

    return kernel


def _custom_three_target_resident_kernel():
    flip_all = np.fliplr(np.eye(8, dtype=np.complex128))
    cudaq.register_operation("mklq_custom_flip_all_3", flip_all)

    kernel = cudaq.make_kernel()
    qubits = kernel.qalloc(3)

    kernel.h(qubits[0])
    kernel.ry(0.17, qubits[1])
    kernel.mklq_custom_flip_all_3(qubits[0], qubits[1], qubits[2])
    kernel.rz(-0.29, qubits[0])
    kernel.x(qubits[2])

    return kernel


def _custom_four_target_resident_kernel():
    flip_all = np.fliplr(np.eye(16, dtype=np.complex128))
    cudaq.register_operation("mklq_custom_flip_all_4", flip_all)

    kernel = cudaq.make_kernel()
    qubits = kernel.qalloc(4)

    kernel.h(qubits[0])
    kernel.ry(0.17, qubits[1])
    kernel.mklq_custom_flip_all_4(qubits[0], qubits[1], qubits[2], qubits[3])
    kernel.rz(-0.29, qubits[0])
    kernel.x(qubits[3])

    return kernel


def _dense_four_target_resident_kernel():
    hadamard = np.array([[1.0, 1.0], [1.0, -1.0]], dtype=np.complex128)
    dense_tensor = hadamard
    for _ in range(3):
        dense_tensor = np.kron(dense_tensor, hadamard)
    cudaq.register_operation("mklq_custom_dense_h4", dense_tensor / 4.0)

    kernel = cudaq.make_kernel()
    qubits = kernel.qalloc(4)

    kernel.ry(0.17, qubits[0])
    kernel.rz(-0.29, qubits[1])
    kernel.x(qubits[3])
    kernel.mklq_custom_dense_h4(qubits[0], qubits[1], qubits[2], qubits[3])
    kernel.ry(0.11, qubits[2])

    return kernel


def _complex_dense_four_target_resident_kernel():
    hadamard = np.array([[1.0, 1.0], [1.0, -1.0]], dtype=np.complex128)
    dense_tensor = hadamard
    for _ in range(3):
        dense_tensor = np.kron(dense_tensor, hadamard)
    phases = np.exp(1j * 0.17 * np.arange(16, dtype=np.float64))
    cudaq.register_operation("mklq_custom_complex_dense_h4",
                             phases[:, np.newaxis] * dense_tensor / 4.0)

    kernel = cudaq.make_kernel()
    qubits = kernel.qalloc(4)
    kernel.ry(0.17, qubits[0])
    kernel.rz(-0.29, qubits[1])
    kernel.x(qubits[3])
    kernel.mklq_custom_complex_dense_h4(qubits[0], qubits[1], qubits[2],
                                        qubits[3])
    kernel.ry(0.11, qubits[2])

    return kernel


def _controlled_four_target_resident_kernel():
    flip_all = np.fliplr(np.eye(16, dtype=np.complex128))
    cudaq.register_operation("mklq_custom_controlled_flip_all_4", flip_all)

    four_gate, first, second, third, fourth = cudaq.make_kernel(
        cudaq.qubit, cudaq.qubit, cudaq.qubit, cudaq.qubit)
    four_gate.mklq_custom_controlled_flip_all_4(first, second, third, fourth)

    kernel = cudaq.make_kernel()
    qubits = kernel.qalloc(5)
    kernel.h(qubits[0])
    kernel.ry(0.17, qubits[1])
    kernel.rz(-0.29, qubits[3])
    kernel.control(four_gate, qubits[0], qubits[4], qubits[1], qubits[3],
                   qubits[2])
    kernel.x(qubits[4])

    return kernel


def _qft_like_resident_kernel(qubit_count):
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


def _seeded_clifford_resident_kernel(qubit_count, seed):
    kernel = cudaq.make_kernel()
    qubits = kernel.qalloc(qubit_count)

    for layer in range(2 * qubit_count + 1):
        target = (seed + 3 * layer) % qubit_count
        selector = (seed + 7 * layer) % 5
        if selector == 0:
            kernel.h(qubits[target])
        elif selector == 1:
            kernel.x(qubits[target])
        elif selector == 2:
            kernel.y(qubits[target])
        elif selector == 3:
            kernel.z(qubits[target])
        else:
            kernel.s(qubits[target])

        control = (target + layer + 1) % qubit_count
        if control == target:
            control = (control + 1) % qubit_count
        other = (target + seed + layer + 2) % qubit_count
        while other in {target, control}:
            other = (other + 1) % qubit_count

        if layer % 4 == 0:
            kernel.cx(qubits[control], qubits[target])
        elif layer % 4 == 1:
            kernel.cy(qubits[control], qubits[target])
        elif layer % 4 == 2:
            kernel.cz(qubits[control], qubits[target])
        else:
            kernel.swap(qubits[target], qubits[other])

    return kernel


def _deterministic_sample_kernel():
    kernel = cudaq.make_kernel()
    qubits = kernel.qalloc(3)
    kernel.x(qubits[0])
    kernel.h(qubits[1])
    kernel.z(qubits[1])
    kernel.h(qubits[1])
    kernel.cx(qubits[0], qubits[2])
    kernel.mz(qubits)
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


@cudaq.kernel
def _resident_measurement_feedback() -> bool:
    qubits = cudaq.qvector(2)
    h(qubits[0])
    x.ctrl(qubits[0], qubits[1])
    if mz(qubits[0]):
        x(qubits[1])
    return mz(qubits[1])


@cudaq.kernel
def _resident_reset_after_measurement() -> bool:
    qubit = cudaq.qubit()
    h(qubit)
    mz(qubit)
    reset(qubit)
    x(qubit)
    return mz(qubit)


def test_mklq_metal_single_target_resident_fixture_matches_qpp():
    _assert_metal_matches_qpp(_single_target_resident_kernel())


def test_mklq_metal_controlled_resident_fixture_matches_qpp():
    _assert_metal_matches_qpp(_controlled_resident_kernel())


def test_mklq_metal_two_target_resident_fixture_matches_qpp():
    _assert_metal_matches_qpp(_two_target_resident_kernel())


def test_mklq_metal_controlled_swap_resident_fixture_matches_qpp():
    _assert_metal_matches_qpp(_controlled_swap_resident_kernel())


def test_mklq_metal_custom_three_target_resident_fixture_matches_qpp():
    _assert_metal_matches_qpp(_custom_three_target_resident_kernel())


def test_mklq_metal_custom_four_target_resident_fixture_matches_qpp():
    _assert_metal_matches_qpp(_custom_four_target_resident_kernel())


def test_mklq_metal_dense_four_target_resident_fixture_matches_qpp():
    _assert_metal_matches_qpp(_dense_four_target_resident_kernel())


def test_mklq_metal_complex_dense_four_target_resident_fixture_matches_qpp():
    _assert_metal_matches_qpp(_complex_dense_four_target_resident_kernel())


def test_mklq_metal_controlled_four_target_resident_fixture_matches_qpp():
    _assert_metal_matches_qpp(_controlled_four_target_resident_kernel())


@pytest.mark.parametrize("qubit_count", [4, 5])
def test_mklq_metal_qft_like_fixture_matches_qpp(qubit_count):
    _assert_metal_matches_qpp(_qft_like_resident_kernel(qubit_count))


@pytest.mark.parametrize("seed", [5, 17])
def test_mklq_metal_seeded_clifford_fixture_matches_qpp(seed):
    _assert_metal_matches_qpp(_seeded_clifford_resident_kernel(5, seed))


def test_mklq_metal_deterministic_sampling_uses_supported_gate_set():
    cudaq.set_target("mklq-metal")
    try:
        counts = cudaq.sample(_deterministic_sample_kernel(), shots_count=64)
    finally:
        cudaq.reset_target()

    assert dict(counts.items()) == {"111": 64}


@pytest.mark.parametrize("measurement_order",
                         ([0, 2, 3], [3, 2, 0], [2, 0, 3]))
def test_mklq_metal_partial_register_sampling_uses_natural_bit_order(
        measurement_order):
    counts = _counts_for_target(
        "mklq-metal", _partial_register_bit_order_kernel(measurement_order),
        shots=64)

    assert counts == {"110": 64}


def test_mklq_metal_partial_register_sampling_matches_qpp_marginal_oracle():
    measured_qubits = (4, 0, 2)
    shots = 4096
    state = _state_for_target("qpp-cpu", _sampling_distribution_state_kernel())
    expected = _marginal_probabilities_from_state(state, measured_qubits)
    counts = _counts_for_target(
        "mklq-metal", _sampling_distribution_sample_kernel(measured_qubits),
        shots=shots)

    _assert_counts_are_close_to_probabilities(counts, expected, shots)


def test_mklq_metal_resident_measurement_feedback_collapses_state():
    cudaq.set_target("mklq-metal")
    try:
        results = cudaq.run(_resident_measurement_feedback, shots_count=32)
    finally:
        cudaq.reset_target()

    assert len(results) == 32
    assert not any(results)


def test_mklq_metal_resident_reset_after_measurement_is_reusable():
    cudaq.set_target("mklq-metal")
    try:
        results = cudaq.run(_resident_reset_after_measurement, shots_count=32)
    finally:
        cudaq.reset_target()

    assert len(results) == 32
    assert all(results)
