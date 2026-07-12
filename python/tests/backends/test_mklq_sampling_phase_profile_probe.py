# ============================================================================ #
# Copyright (c) 2026 Linsen Wu.                                                #
#                                                                              #
# This source code and the accompanying materials are made available under     #
# the terms of the Apache License 2.0 which accompanies this distribution.     #
# ============================================================================ #

import importlib.util

import pytest


def _load_module():
    from pathlib import Path

    root = Path(__file__).resolve().parents[3]
    path = root / "benchmarks" / "mklq" / "run_sampling_phase_profile_probe.py"
    spec = importlib.util.spec_from_file_location("mklq_sampling_phase_probe",
                                                  path)
    module = importlib.util.module_from_spec(spec)
    assert spec.loader is not None
    spec.loader.exec_module(module)
    return module


def _profile_xml(module, target="mklq-metal", skipped=False):
    properties = {
        "mklq_sampling_phase_profile": "true",
        "mklq_sampling_phase_profile_target": target,
        "mklq_sampling_phase_profile_qubits": "20",
        "mklq_sampling_phase_profile_measured_qubits": "12",
        "mklq_sampling_phase_profile_shots": "65536",
        "mklq_sampling_phase_profile_probability_fill_seconds": "0.0042",
        "mklq_sampling_phase_profile_draw_and_count_seconds": "0.0008",
        "mklq_sampling_phase_profile_expectation_reduction_seconds": "0.0001",
        "mklq_sampling_phase_profile_host_fold_seconds": "0.0005",
        "mklq_sampling_phase_profile_metal_probability_buffer_preparation_seconds": "0.0007",
        "mklq_sampling_phase_profile_metal_probability_dispatch_seconds": "0.002",
        "mklq_sampling_phase_profile_metal_probability_host_conversion_seconds": "0.001",
        "mklq_sampling_phase_profile_metal_probability_fill_applications": "1",
        "mklq_sampling_phase_profile_metal_marginal_probability_applications": "0",
        "mklq_sampling_phase_profile_metal_generated_count_accumulations": "1",
    }
    property_xml = "".join(
        f'<property name="{name}" value="{value}"/>'
        for name, value in properties.items())
    skipped_xml = "<skipped/>" if skipped else ""
    return f"""<testsuites><testsuite><testcase name=
    \"{module.PROFILE_TEST_SUFFIX}\"><properties>{property_xml}</properties>
    {skipped_xml}</testcase></testsuite></testsuites>"""


def test_parse_profile_xml_reads_native_phase_properties(tmp_path):
    module = _load_module()
    xml_path = tmp_path / "profile.xml"
    xml_path.write_text(_profile_xml(module), encoding="utf-8")

    profile = module.parse_profile_xml(xml_path, "mklq-metal")

    assert profile["qubits"] == 20
    assert profile["measured_qubits"] == 12
    assert profile["shots"] == 65536
    assert profile["phase_seconds"]["probability_fill_seconds"] == 0.0042
    assert profile["subphase_seconds"]["host_fold_seconds"] == 0.0005
    assert profile["subphase_seconds"][
        "metal_probability_buffer_preparation_seconds"] == 0.0007
    assert profile["subphase_seconds"][
        "metal_probability_dispatch_seconds"] == 0.002
    assert profile["metal_probability_fill_applications"] == 1
    assert profile["metal_marginal_probability_applications"] == 0
    assert profile["metal_generated_count_accumulations"] == 1


def test_parse_profile_xml_rejects_skipped_testcase(tmp_path):
    module = _load_module()
    xml_path = tmp_path / "profile.xml"
    xml_path.write_text(_profile_xml(module, skipped=True), encoding="utf-8")

    with pytest.raises(ValueError, match="skipped"):
        module.parse_profile_xml(xml_path, "mklq-metal")


def test_validate_profile_config_rejects_mismatched_environment():
    module = _load_module()

    with pytest.raises(ValueError, match="measured_qubits"):
        module.validate_profile_config({
            "qubits": 20,
            "measured_qubits": 11,
            "shots": 65536,
        }, 20, 12, 65536)


def test_build_report_rejects_invalid_profile_shape(tmp_path):
    module = _load_module()

    with pytest.raises(ValueError, match="measured-qubits"):
        module.build_report(tmp_path, tmp_path, "mklq-cpu", 20, 21, 32, 1)
