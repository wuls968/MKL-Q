"""Release-package contracts that do not require a compiled CUDA-Q runtime."""

from __future__ import annotations

import importlib.util
import os
import subprocess
import tomllib
import zipfile
from pathlib import Path

import pytest


REPO_ROOT = Path(__file__).resolve().parents[3]


def load_distribution_guard():
    path = REPO_ROOT / "python" / "cudaq" / "_mklq_distribution.py"
    assert path.is_file(), "MKL-Q distribution guard module is missing"
    spec = importlib.util.spec_from_file_location("mklq_distribution_guard", path)
    assert spec is not None
    assert spec.loader is not None
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def load_package_release_module():
    path = REPO_ROOT / "benchmarks" / "mklq" / "package_release.py"
    assert path.is_file(), "MKL-Q package release helper is missing"
    spec = importlib.util.spec_from_file_location("mklq_package_release", path)
    assert spec is not None
    assert spec.loader is not None
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def load_package_release_audit_module():
    path = REPO_ROOT / "benchmarks" / "mklq" / "run_package_release_audit.py"
    assert path.is_file(), "MKL-Q package release audit is missing"
    spec = importlib.util.spec_from_file_location("mklq_package_release_audit", path)
    assert spec is not None
    assert spec.loader is not None
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def load_public_healthcheck_module():
    path = REPO_ROOT / "benchmarks" / "mklq" / "run_public_healthcheck.py"
    assert path.is_file()
    spec = importlib.util.spec_from_file_location("mklq_public_healthcheck", path)
    assert spec is not None
    assert spec.loader is not None
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def load_public_readiness_audit_module():
    path = REPO_ROOT / "benchmarks" / "mklq" / "run_public_readiness_audit.py"
    assert path.is_file()
    spec = importlib.util.spec_from_file_location("mklq_public_readiness", path)
    assert spec is not None
    assert spec.loader is not None
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def load_self_hosted_ci_audit_module():
    path = REPO_ROOT / "benchmarks" / "mklq" / "run_self_hosted_ci_audit.py"
    assert path.is_file()
    spec = importlib.util.spec_from_file_location("mklq_self_hosted_ci", path)
    assert spec is not None
    assert spec.loader is not None
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def load_preflight_audit_module():
    path = REPO_ROOT / "benchmarks" / "mklq" / "run_preflight_audit.py"
    assert path.is_file()
    spec = importlib.util.spec_from_file_location("mklq_preflight", path)
    assert spec is not None
    assert spec.loader is not None
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def test_root_wheel_metadata_identifies_mklq_as_macos_only_distribution():
    with (REPO_ROOT / "pyproject.toml").open("rb") as handle:
        metadata = tomllib.load(handle)

    project = metadata["project"]
    assert project["name"] == "mklq"
    assert project["requires-python"] == ">=3.11,<3.15"
    assert project["urls"]["Repository"] == "https://github.com/wuls968/MKL-Q"
    assert "macos" in " ".join(project["classifiers"]).lower()
    assert not any("nvidia-" in dependency for dependency in project["dependencies"])


def test_public_readmes_and_changelog_describe_the_package_release_boundary():
    readme = (REPO_ROOT / "README.md").read_text(encoding="utf-8")
    package_readme = (REPO_ROOT / "python" / "README.md.in").read_text(
        encoding="utf-8")
    changelog = (REPO_ROOT / "CHANGELOG.md").read_text(encoding="utf-8")

    assert "python -m pip install mklq" in readme
    assert "This repository is source-only for the first public version" not in readme
    assert "macOS ARM64" in package_readme
    assert "pip install mklq" in package_readme
    assert "${{" not in package_readme
    assert "mklq-v0.1.0" in changelog
    assert "Source-Only Tag" not in changelog


def test_release_build_uses_mklq_version_and_mklq_repository_provenance():
    cmake = (REPO_ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
    version_source = (REPO_ROOT / "runtime" / "common" / "Version.cpp.in").read_text(
        encoding="utf-8")

    assert "ENV{MKLQ_VERSION}" in cmake
    assert "MKLQ_VERSION must be a release PEP 440 version" in cmake
    assert "https://github.com/wuls968/MKL-Q" in version_source


def test_package_only_cmake_requires_an_explicit_valid_mklq_version():
    cmake = (REPO_ROOT / "CMakeLists.txt").read_text(encoding="utf-8")

    assert (
        "if(CUDAQ_MKLQ_PACKAGE_ONLY)\n"
        "  if(NOT DEFINED ENV{MKLQ_VERSION})" in cmake)
    assert "CUDAQ_MKLQ_PACKAGE_ONLY=ON requires ENV{MKLQ_VERSION}" in cmake
    assert "MKLQ_VERSION must be a release PEP 440 version" in cmake
    assert (
        "if(CUDAQ_MKLQ_PACKAGE_ONLY)\n"
        "  set(CUDA_QUANTUM_VERSION \"${MKLQ_RELEASE_VERSION}\")" in cmake)
    assert "elseif (DEFINED ENV{CUDA_QUANTUM_VERSION})" in cmake


def test_wheel_asset_helper_installs_macos_dylib_backends():
    helper = (REPO_ROOT / "cmake" / "modules" / "BuildHelpers.cmake").read_text(
        encoding="utf-8")

    assert '"${FILE_EXTENSION}" STREQUAL ".dylib"' in helper


def test_mklq_release_wheel_script_keeps_build_products_outside_worktree():
    script = (REPO_ROOT / "scripts" / "build_wheel.sh").read_text(encoding="utf-8")

    assert "set -euo pipefail" in script
    assert "parent must already exist outside the repository for -m" in script
    assert 'release_tmp=$(mktemp -d "${TMPDIR:-/tmp}/mklq-wheel.XXXXXX")' in script
    assert '"-Cbuild-dir=$release_tmp/build"' in script
    assert "must use an absolute output directory outside the repository" in script
    assert "release output directory resolves inside the repository" in script
    assert 'if $mklq_release; then\n    mkdir -p "$output_dir"\nelse\n    if $incremental;' in script
    assert 'submodule_status="$(git submodule status --recursive)"' in script
    assert 'printf \'%s\\n\' "$submodule_status" | grep -Eq \'^[+U-]\'' in script
    assert "could not inspect submodule status" in script
    assert "-DGIT_SUBMODULE=OFF" in script
    assert "-DCUDAQ_MKLQ_PACKAGE_ONLY=ON" in script
    assert "-DCUDAQ_ENABLE_ALL_BACKEND=OFF" in script
    assert "-DCUDAQ_BUILD_TESTS=FALSE" in script
    assert "-DCUDAQ_STATIC_DEPS=OFF" in script


def test_cmake_can_build_an_mklq_only_python_runtime_wheel():
    root_cmake = (REPO_ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
    nvqir_cmake = (REPO_ROOT / "runtime" / "nvqir" /
                   "CMakeLists.txt").read_text(encoding="utf-8")

    assert "CUDAQ_MKLQ_PACKAGE_ONLY" in root_cmake
    assert "if (NOT CUDAQ_MKLQ_PACKAGE_ONLY)" in nvqir_cmake
    assert "add_dependencies(nvqir nvqir-mklq_cpu)" in nvqir_cmake


def test_general_nvqir_export_does_not_link_the_unexported_qpp_plugin():
    nvqir_cmake = (REPO_ROOT / "runtime" / "nvqir" /
                   "CMakeLists.txt").read_text(encoding="utf-8")

    assert "target_link_libraries(${LIBRARY_NAME} PRIVATE nvqir-qpp)" not in nvqir_cmake
    assert "add_subdirectory(qpp)" in nvqir_cmake


def test_package_only_holder_defaults_to_mklq_cpu_via_compile_definition():
    holder = (REPO_ROOT / "python" / "utils" / "LinkedLibraryHolder.cpp").read_text(
        encoding="utf-8")
    python_utils = (REPO_ROOT / "python" / "utils" / "CMakeLists.txt").read_text(
        encoding="utf-8")
    python_cmake = (REPO_ROOT / "python" / "CMakeLists.txt").read_text(
        encoding="utf-8")

    assert "#if defined(CUDAQ_MKLQ_PACKAGE_ONLY)" in holder
    assert 'DEFAULT_TARGET[] = "mklq-cpu"' in holder
    assert 'DEFAULT_TARGET[] = "qpp-cpu"' in holder
    assert "defaultTarget = DEFAULT_TARGET;" in holder
    assert "std::string resolved = DEFAULT_TARGET;" in holder
    assert "if (defaultTarget != DEFAULT_TARGET)" in holder
    assert "target_compile_definitions(cudaq-py-utils PRIVATE CUDAQ_MKLQ_PACKAGE_ONLY)" in python_utils
    assert "add_compile_definitions(CUDAQ_MKLQ_PACKAGE_ONLY)" in python_cmake


def test_package_only_cmake_excludes_non_mklq_runtime_assets():
    root_cmake = (REPO_ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
    runtime_cmake = (REPO_ROOT / "runtime" / "cudaq" / "CMakeLists.txt").read_text(
        encoding="utf-8")
    platform_cmake = (REPO_ROOT / "runtime" / "cudaq" / "platform" /
                      "CMakeLists.txt").read_text(encoding="utf-8")
    default_platform_cmake = (REPO_ROOT / "runtime" / "cudaq" / "platform" /
                              "default" / "CMakeLists.txt").read_text(encoding="utf-8")
    python_cmake = (REPO_ROOT / "python" / "CMakeLists.txt").read_text(
        encoding="utf-8")

    for setting in (
            "CUDAQ_ENABLE_REST OFF",
            "CUDAQ_ENABLE_NLOPT ON",
            "CUDAQ_ENABLE_ENSMALLEN ON",
            "CUDAQ_SKIP_MPI ON",
            "CUDAQ_DISABLE_CPP_FRONTEND ON",
            "CUDAQ_DISABLE_TOOLS ON",
    ):
        assert setting in root_cmake
    assert (
        "if (NOT CUDAQ_MKLQ_PACKAGE_ONLY)\n"
        "  if (NOT CUDAQ_DISABLE_RUNTIME)\n"
        "    add_subdirectory(tpls/qpp" in root_cmake)
    assert "if(CUDAQ_MKLQ_PACKAGE_ONLY)\n  add_subdirectory(qis/managers/default)" in runtime_cmake
    assert "if(CUDAQ_MKLQ_PACKAGE_ONLY)\n  add_subdirectory(default)" in platform_cmake
    assert "if (NOT CUDAQ_MKLQ_PACKAGE_ONLY)\n  add_target_config(opt-test)" in default_platform_cmake
    assert "if (NOT CUDAQ_MKLQ_PACKAGE_ONLY)\n  add_subdirectory(runtime/cudaq/domains/plugins)" in python_cmake


def test_package_only_runtime_does_not_link_the_remote_rest_client():
    common_cmake = (REPO_ROOT / "runtime" / "common" / "CMakeLists.txt").read_text(
        encoding="utf-8")
    rest_client = (REPO_ROOT / "runtime" / "common" / "RestClient.cpp").read_text(
        encoding="utf-8")

    assert "target_sources(${LIBRARY_NAME} PRIVATE RestClient.cpp)" in common_cmake
    assert "if(OPENSSL_FOUND AND CUDAQ_ENABLE_REST)" in common_cmake
    assert rest_client.index('#include "nlohmann/json.hpp"') < rest_client.index(
        "#ifdef CUDAQ_RESTCLIENT_AVAILABLE")


def test_package_only_python_extension_excludes_remote_and_photonics_code():
    extension_cmake = (REPO_ROOT / "python" / "extension" / "CMakeLists.txt").read_text(
        encoding="utf-8")
    extension_cpp = (REPO_ROOT / "python" / "extension" /
                     "CUDAQuantumExtension.cpp").read_text(encoding="utf-8")
    cudaq_init = (REPO_ROOT / "python" / "cudaq" / "__init__.py").read_text(
        encoding="utf-8")

    assert "set(_cudaq_python_remote_extension_sources" in extension_cmake
    assert "if(CUDAQ_MKLQ_PACKAGE_ONLY)\n  set(_cudaq_python_remote_extension_sources \"\")" in extension_cmake
    assert "${_cudaq_python_remote_extension_sources}" in extension_cmake
    assert "if (NOT CUDAQ_MKLQ_PACKAGE_ONLY)\n  list(APPEND _cudaq_python_runtime_link_libs cudaq-em-photonics)" in extension_cmake
    assert "#if !defined(CUDAQ_MKLQ_PACKAGE_ONLY)" in extension_cpp
    assert "auto orcaSubmodule" in extension_cpp
    assert "auto photonicsSubmodule" in extension_cpp
    assert 'if hasattr(cudaq_runtime, "orca"):' in cudaq_init


def test_package_only_runtime_exposes_the_pep440_distribution_version():
    extension_cpp = (REPO_ROOT / "python" / "extension" /
                     "CUDAQuantumExtension.cpp").read_text(encoding="utf-8")

    assert "#if defined(CUDAQ_MKLQ_PACKAGE_ONLY)" in extension_cpp
    assert 'cudaqRuntime.attr("__version__") = getVersion();' in extension_cpp
    assert "#else\n  std::stringstream ss;" in extension_cpp
    assert 'ss << "CUDA-Q Version " << getVersion()' in extension_cpp
    assert 'cudaqRuntime.attr("__version__") = ss.str();' in extension_cpp


def test_mklq_release_wheel_script_pins_arm64_and_rejects_external_assets():
    script = (REPO_ROOT / "scripts" / "build_wheel.sh").read_text(encoding="utf-8")

    assert "external_assets_requested=false" in script
    assert "external_assets_requested=true" in script
    assert "MKL-Q release wheels do not accept -a external simulator assets." in script
    assert "-DCMAKE_OSX_ARCHITECTURES=arm64" in script
    assert "if $mklq_release; then\n    unset CUDAQ_EXTERNAL_NVQIR_SIMS" in script
    assert "platform.machine()" in script
    assert "sysconfig.get_platform()" in script
    assert "require an arm64 Python interpreter" in script


def test_mklq_release_wheel_script_requires_exact_submodule_gitlinks():
    script = (REPO_ROOT / "scripts" / "build_wheel.sh").read_text(encoding="utf-8")

    assert "grep -Eq '^[+U-]'" in script
    assert "exact submodule gitlink revisions" in script


def test_mklq_release_wheel_script_rejects_relative_output_before_build(tmp_path):
    environment = os.environ.copy()
    environment["MKLQ_VERSION"] = "0.1.0"

    result = subprocess.run(
        ["bash", "scripts/build_wheel.sh", "-m", "-o", "release-assets"],
        cwd=REPO_ROOT,
        env=environment,
        capture_output=True,
        text=True,
    )

    assert result.returncode != 0
    assert "absolute output directory outside the repository" in (
        result.stdout + result.stderr)


def test_mklq_release_wheel_script_rejects_worktree_output_before_build():
    environment = os.environ.copy()
    environment["MKLQ_VERSION"] = "0.1.0"
    worktree_output = REPO_ROOT / "release-assets"

    result = subprocess.run(
        ["bash", "scripts/build_wheel.sh", "-m", "-o", str(worktree_output)],
        cwd=REPO_ROOT,
        env=environment,
        capture_output=True,
        text=True,
    )

    assert result.returncode != 0
    assert "output directory must be outside the repository" in (
        result.stdout + result.stderr)
    assert not worktree_output.exists()


def test_mklq_release_wheel_script_rejects_new_nested_worktree_output_without_creating_it(
        tmp_path):
    environment = os.environ.copy()
    environment["MKLQ_VERSION"] = "0.1.0"
    worktree_parent = REPO_ROOT / f".mklq-release-output-{tmp_path.name}"
    worktree_output = worktree_parent / "nested" / "out"
    assert not worktree_parent.exists()

    result = subprocess.run(
        ["bash", "scripts/build_wheel.sh", "-m", "-o", str(worktree_output)],
        cwd=REPO_ROOT,
        env=environment,
        capture_output=True,
        text=True,
    )

    assert result.returncode != 0
    assert "must be outside the repository" in result.stdout + result.stderr
    assert not worktree_parent.exists()


def test_mklq_release_wheel_script_rejects_symlinked_nested_parent_without_writing_worktree(
        tmp_path):
    environment = os.environ.copy()
    environment["MKLQ_VERSION"] = "0.1.0"
    link_to_repo = tmp_path / "link-to-repo"
    link_to_repo.symlink_to(REPO_ROOT, target_is_directory=True)
    worktree_parent = REPO_ROOT / f".mklq-symlink-output-{tmp_path.name}"
    worktree_output = link_to_repo / worktree_parent.name / "out"
    assert not worktree_parent.exists()

    result = subprocess.run(
        ["bash", "scripts/build_wheel.sh", "-m", "-o", str(worktree_output)],
        cwd=REPO_ROOT,
        env=environment,
        capture_output=True,
        text=True,
    )

    assert result.returncode != 0
    assert "output parent must already exist" in result.stdout + result.stderr
    assert not worktree_parent.exists()


def test_mklq_release_wheel_script_rejects_existing_symlink_parent_to_worktree(
        tmp_path):
    environment = os.environ.copy()
    environment["MKLQ_VERSION"] = "0.1.0"
    link_to_repo = tmp_path / "link-to-repo"
    link_to_repo.symlink_to(REPO_ROOT, target_is_directory=True)
    worktree_output = REPO_ROOT / f".mklq-symlink-child-{tmp_path.name}"

    result = subprocess.run(
        ["bash", "scripts/build_wheel.sh", "-m", "-o",
         str(link_to_repo / worktree_output.name)],
        cwd=REPO_ROOT,
        env=environment,
        capture_output=True,
        text=True,
    )

    assert result.returncode != 0
    assert "output parent resolves inside the repository" in (
        result.stdout + result.stderr)
    assert not worktree_output.exists()


@pytest.mark.parametrize("submodule_status", ["+", "-", "U"])
def test_mklq_release_wheel_script_rejects_submodule_drift(
        tmp_path, submodule_status):
    fake_git = tmp_path / "git"
    fake_git.write_text(
        "#!/bin/bash\n"
        "if [ \"$1\" = \"rev-parse\" ]; then\n"
        "  printf '%s\\n' \"$MKLQ_FAKE_REPO_ROOT\"\n"
        "elif [ \"$1\" = \"submodule\" ]; then\n"
        "  printf '%s fixture\\n' \"$MKLQ_SUBMODULE_STATUS\"\n"
        "else\n"
        "  exit 64\n"
        "fi\n",
        encoding="utf-8",
    )
    fake_git.chmod(0o755)
    environment = os.environ.copy()
    environment.update({
        "MKLQ_VERSION": "0.1.0",
        "PYTHON": "/opt/anaconda3/bin/python3",
        "MKLQ_FAKE_REPO_ROOT": str(REPO_ROOT),
        "MKLQ_SUBMODULE_STATUS": submodule_status,
        "PATH": f"{tmp_path}:{environment['PATH']}",
    })

    result = subprocess.run(
        ["bash", "scripts/build_wheel.sh", "-m", "-o", str(tmp_path / "out")],
        cwd=REPO_ROOT,
        env=environment,
        capture_output=True,
        text=True,
    )

    assert result.returncode != 0
    assert "exact submodule gitlink revisions" in result.stdout + result.stderr
    assert not (tmp_path / "out").exists()


def test_mklq_release_wheel_script_fails_when_submodule_status_cannot_be_read(
        tmp_path):
    fake_git = tmp_path / "git"
    fake_git.write_text(
        "#!/bin/bash\n"
        "if [ \"$1\" = \"rev-parse\" ]; then\n"
        "  printf '%s\\n' \"$MKLQ_FAKE_REPO_ROOT\"\n"
        "elif [ \"$1\" = \"submodule\" ]; then\n"
        "  exit 65\n"
        "else\n"
        "  exit 64\n"
        "fi\n",
        encoding="utf-8",
    )
    fake_git.chmod(0o755)
    environment = os.environ.copy()
    environment.update({
        "MKLQ_VERSION": "0.1.0",
        "PYTHON": "/opt/anaconda3/bin/python3",
        "MKLQ_FAKE_REPO_ROOT": str(REPO_ROOT),
        "PATH": f"{tmp_path}:{environment['PATH']}",
    })

    result = subprocess.run(
        ["bash", "scripts/build_wheel.sh", "-m", "-o", str(tmp_path / "out")],
        cwd=REPO_ROOT,
        env=environment,
        capture_output=True,
        text=True,
    )

    assert result.returncode != 0
    assert "could not inspect submodule status" in result.stdout + result.stderr
    assert not (tmp_path / "out").exists()


def test_distribution_guard_allows_an_isolated_mklq_installation():
    guard = load_distribution_guard()

    conflicts = guard.conflicting_distribution_names(
        {"cudaq": ["mklq"]}, current_distribution="mklq")

    assert conflicts == []


def test_distribution_guard_rejects_official_cudaq_coinstallation():
    guard = load_distribution_guard()

    with pytest.raises(ImportError, match="isolated virtual environment"):
        guard.ensure_mklq_distribution_ownership(
            {"cudaq": ["mklq", "cudaq"]},
            installed_distributions={"mklq", "cudaq"},
        )


@pytest.mark.parametrize("conflicting_distribution", [
    "cudaq",
    "cuda-quantum",
    "cuda-quantum-cu13",
])
def test_distribution_guard_rejects_cudaq_family_even_without_cudaq_owner_metadata(
        conflicting_distribution):
    guard = load_distribution_guard()

    with pytest.raises(ImportError, match="isolated virtual environment"):
        guard.ensure_mklq_distribution_ownership(
            {"cudaq": ["mklq"]},
            installed_distributions={"mklq", conflicting_distribution},
        )


@pytest.mark.parametrize("version", ["0.1.0", "0.1.0rc1", "0.1.0.post1"])
def test_package_release_helper_accepts_supported_pep440_release_versions(version):
    helper = load_package_release_module()

    assert helper.validate_release_version(version) == version


@pytest.mark.parametrize("version", ["", "v0.1.0", "0.1", "0.1.0-dev"])
def test_package_release_helper_rejects_non_release_versions(version):
    helper = load_package_release_module()

    with pytest.raises(ValueError, match="PEP 440"):
        helper.validate_release_version(version)


def test_package_release_helper_inspects_required_mklq_assets(tmp_path):
    helper = load_package_release_module()
    wheel = tmp_path / "mklq-0.1.0-cp312-cp312-macosx_11_0_arm64.whl"
    with zipfile.ZipFile(wheel, "w") as archive:
        archive.writestr("cudaq/__init__.py", "")
        archive.writestr("cudaq/lib/libnvqir-mklq_cpu.dylib", "cpu")
        archive.writestr("cudaq/lib/libnvqir-mklq_metal.dylib", "metal")
        archive.writestr("cudaq/targets/mklq-cpu.yml", "target: mklq-cpu\n")
        archive.writestr("cudaq/targets/mklq-metal.yml", "target: mklq-metal\n")

    report = helper.inspect_wheel(wheel, expected_version="0.1.0")

    assert report["summary"] == {"status": "passed", "failed": 0}
    assert report["wheel"]["filename"] == wheel.name
    assert report["targets"] == ["mklq-cpu", "mklq-metal"]


def test_package_release_helper_allows_only_declared_mklq_runtime_native_assets(
        tmp_path):
    helper = load_package_release_module()
    wheel = tmp_path / "mklq-0.1.0-cp312-cp312-macosx_11_0_arm64.whl"
    with zipfile.ZipFile(wheel, "w") as archive:
        for library in (
                "libnvqir.dylib",
                "libnvqir-mklq_cpu.dylib",
                "libnvqir-mklq_metal.dylib",
                "libcudaq.dylib",
                "libcudaq-common.dylib",
                "libcudaq-logger.dylib",
                "libcudaq-mlir-runtime.dylib",
                "libcudaq-operator.dylib",
                "libcudaq-builder.dylib",
                "libcudaq-em-default.dylib",
                "libcudaq-ensmallen.dylib",
                "libcudaq-platform-default.dylib",
                "libcudaq-device-call-runtime.dylib",
                "libcudaq-nlopt.dylib",
                "libcudaq-py-utils.dylib",
                "libcudaq-python-interop.dylib",
                "libCUDAQuantumPythonCAPI.dylib",
                "libMLIRPythonSupport-cudaq.dylib",
                "libnanobind-cudaq.dylib",
                "libomp.dylib",
        ):
            archive.writestr(f"cudaq/lib/{library}", "runtime")
        for extension in (
                "_mlir.cpython-312-darwin.so",
                "_mlirAsyncPasses.cpython-312-darwin.so",
                "_mlirDialectsAMDGPU.cpython-312-darwin.so",
                "_mlirDialectsGPU.cpython-312-darwin.so",
                "_mlirDialectsIRDL.cpython-312-darwin.so",
                "_mlirDialectsLLVM.cpython-312-darwin.so",
                "_mlirDialectsLinalg.cpython-312-darwin.so",
                "_mlirDialectsNVGPU.cpython-312-darwin.so",
                "_mlirDialectsPDL.cpython-312-darwin.so",
                "_mlirDialectsQuant.cpython-312-darwin.so",
                "_mlirDialectsSMT.cpython-312-darwin.so",
                "_mlirDialectsSparseTensor.cpython-312-darwin.so",
                "_mlirDialectsTransform.cpython-312-darwin.so",
                "_mlirExecutionEngine.cpython-312-darwin.so",
                "_mlirGPUPasses.cpython-312-darwin.so",
                "_mlirLinalgPasses.cpython-312-darwin.so",
                "_mlirRegisterEverything.cpython-312-darwin.so",
                "_mlirSparseTensorPasses.cpython-312-darwin.so",
                "_mlirTransformInterpreter.cpython-312-darwin.so",
                "_quakeDialects.cpython-312-darwin.so",
        ):
            archive.writestr(f"cudaq/mlir/_mlir_libs/{extension}", "extension")
        archive.writestr("cudaq/targets/mklq-cpu.yml", "target: mklq-cpu\n")
        archive.writestr("cudaq/targets/mklq-metal.yml", "target: mklq-metal\n")

    assert helper.inspect_wheel(wheel, expected_version="0.1.0")["summary"]["status"] == "passed"


def test_package_release_helper_rejects_wheel_missing_cpu_target_assets(tmp_path):
    helper = load_package_release_module()
    wheel = tmp_path / "mklq-0.1.0-cp312-cp312-macosx_11_0_arm64.whl"
    with zipfile.ZipFile(wheel, "w") as archive:
        archive.writestr("cudaq/__init__.py", "")

    with pytest.raises(ValueError, match="mklq-cpu"):
        helper.inspect_wheel(wheel, expected_version="0.1.0")


@pytest.mark.parametrize(
    ("extra_name", "error"),
    [
        ("cudaq/targets/qpp-cpu.yml", "unsupported target configuration"),
        ("cudaq/lib/libnvqir-qpp.dylib", "forbidden backend library"),
        ("cudaq/lib/libnvqir-unknown.dylib", "forbidden backend library"),
        ("cudaq/lib/libnvqir-mklq_cpu_extra.dylib", "forbidden backend library"),
        ("cudaq/lib/libcudaq-em-photonics.dylib", "forbidden backend library"),
        ("cudaq/lib/libcudaq-rest-qpu.dylib", "forbidden backend library"),
        ("cudaq/lib/libcudaq-dynamics.dylib", "unsupported native library"),
        ("cudaq/lib/libcudaq-foo.dylib", "unsupported native library"),
        ("cudaq/lib/libcustatevec.dylib", "unsupported native library"),
        ("cudaq/lib/libunexpected.dylib", "unsupported native library"),
        ("cudaq/mlir/_mlir_libs/_evil.cpython-312-darwin.so",
         "unsupported native library"),
        ("cudaq/mlir/_mlir_libs/_mlirDialectsUnapproved.cpython-312-darwin.so",
         "unsupported native library"),
    ],
)
def test_package_release_helper_rejects_non_mklq_backend_assets(
        tmp_path, extra_name, error):
    helper = load_package_release_module()
    wheel = tmp_path / "mklq-0.1.0-cp312-cp312-macosx_11_0_arm64.whl"
    with zipfile.ZipFile(wheel, "w") as archive:
        archive.writestr("cudaq/lib/libnvqir-mklq_cpu.dylib", "cpu")
        archive.writestr("cudaq/lib/libnvqir-mklq_metal.dylib", "metal")
        archive.writestr("cudaq/targets/mklq-cpu.yml", "target: mklq-cpu\n")
        archive.writestr("cudaq/targets/mklq-metal.yml", "target: mklq-metal\n")
        archive.writestr(extra_name, "not allowed")

    with pytest.raises(ValueError, match=error):
        helper.inspect_wheel(wheel, expected_version="0.1.0")


def test_package_release_helper_rejects_non_macos_or_cli_wheels(tmp_path):
    helper = load_package_release_module()
    wheel = tmp_path / "mklq-0.1.0-cp312-cp312-manylinux_2_28_aarch64.whl"
    with zipfile.ZipFile(wheel, "w") as archive:
        archive.writestr("cudaq/lib/libnvqir-mklq_cpu.dylib", "cpu")
        archive.writestr("cudaq/lib/libnvqir-mklq_metal.dylib", "metal")
        archive.writestr("cudaq/targets/mklq-cpu.yml", "target: mklq-cpu\n")
        archive.writestr("cudaq/targets/mklq-metal.yml", "target: mklq-metal\n")
        archive.writestr("bin/nvq++", "not allowed")

    with pytest.raises(ValueError, match="macOS arm64"):
        helper.inspect_wheel(wheel, expected_version="0.1.0")


@pytest.mark.parametrize("version", [None, "v0.1.0"])
def test_mklq_release_wheel_script_rejects_invalid_version_before_build(
        tmp_path, version):
    environment = os.environ.copy()
    environment.pop("MKLQ_VERSION", None)
    if version is not None:
        environment["MKLQ_VERSION"] = version

    result = subprocess.run(
        ["bash", "scripts/build_wheel.sh", "-m", "-o", str(tmp_path / "out")],
        cwd=REPO_ROOT,
        env=environment,
        capture_output=True,
        text=True,
    )

    assert result.returncode != 0
    assert "MKLQ_VERSION" in result.stdout + result.stderr
    assert not (tmp_path / "out").exists()


def test_package_release_audit_accepts_the_v010_release_contract(tmp_path):
    audit = load_package_release_audit_module()
    (tmp_path / "docs" / "mklq").mkdir(parents=True)
    (tmp_path / ".github" / "workflows").mkdir(parents=True)
    (tmp_path / "pyproject.toml").write_text(
        "[project]\nname = \"mklq\"\nrequires-python = \">=3.11,<3.15\"\n",
        encoding="utf-8",
    )
    (tmp_path / "docs" / "mklq" / "release-policy.md").write_text(
        "mklq-vX.Y.Z TestPyPI PyPI mklq-metal experimental no .pkg\n",
        encoding="utf-8",
    )
    (tmp_path / "docs" / "mklq" / "release-notes-v0.1.0.md").write_text(
        "mklq-v0.1.0 mklq-cpu mklq-metal experimental TestPyPI PyPI\n",
        encoding="utf-8",
    )
    (tmp_path / "SECURITY.md").write_text(
        "Use GitHub private vulnerability reporting.\n", encoding="utf-8")
    (tmp_path / ".github" / "workflows" / "mklq-package-release.yml").write_text(
        "workflow_dispatch\nid-token: write\ntestpypi\npypi\n"
        "attest-build-provenance\npython: \"3.11\"\npython: \"3.12\"\n"
        "python: \"3.13\"\npython: \"3.14\"\n"
        "Verify trusted runner Python provisioning\nlipo -archs\n"
        "Reinstall and validate the published package\n"
        "https://test.pypi.org/simple\nhttps://pypi.org/simple\n"
        "mklq-v${{ inputs.version }}rc1\n"
        "MKLQ_ASSET_ROOT=${RUNNER_TEMP}/mklq-release-assets-\n"
        "git status --porcelain --untracked-files=all\n",
        encoding="utf-8",
    )

    report = audit.build_report(tmp_path, version="0.1.0", docs_only=True)
    rc_report = audit.build_report(tmp_path, version="0.1.0rc1", docs_only=True)

    assert report["summary"] == {"status": "passed", "passed": 5, "failed": 0}
    assert rc_report["summary"] == {"status": "passed", "passed": 5, "failed": 0}
    assert audit.validate_release_tag("mklq-v0.1.0", "0.1.0") is None
    assert audit.validate_release_tag("mklq-v0.1.0.post1", "0.1.0.post1") is None


def test_package_release_audit_requires_matrix_and_post_publish_validation(tmp_path):
    audit = load_package_release_audit_module()
    (tmp_path / "docs" / "mklq").mkdir(parents=True)
    (tmp_path / ".github" / "workflows").mkdir(parents=True)
    (tmp_path / "pyproject.toml").write_text(
        "[project]\nname = \"mklq\"\nrequires-python = \">=3.11,<3.15\"\n",
        encoding="utf-8",
    )
    (tmp_path / "docs" / "mklq" / "release-policy.md").write_text(
        "mklq-vX.Y.Z TestPyPI PyPI mklq-metal experimental no .pkg\n",
        encoding="utf-8",
    )
    (tmp_path / "docs" / "mklq" / "release-notes-v0.1.0.md").write_text(
        "mklq-v0.1.0 mklq-cpu mklq-metal experimental TestPyPI PyPI\n",
        encoding="utf-8",
    )
    (tmp_path / "SECURITY.md").write_text(
        "Use GitHub private vulnerability reporting.\n", encoding="utf-8")
    (tmp_path / ".github" / "workflows" / "mklq-package-release.yml").write_text(
        "workflow_dispatch\nid-token: write\ntestpypi\npypi\nattest-build-provenance\n",
        encoding="utf-8",
    )

    report = audit.build_report(tmp_path, version="0.1.0", docs_only=True)
    workflow_check = next(
        check for check in report["checks"]
        if check["name"] == ".github/workflows/mklq-package-release.yml")

    assert workflow_check["status"] == "failed"
    assert 'python: "3.11"' in workflow_check["details"]["missing"]
    assert "Reinstall and validate the published package" in workflow_check[
        "details"]["missing"]


def test_package_release_audit_requires_rc_lineage_and_external_build_assets(
        tmp_path):
    audit = load_package_release_audit_module()
    (tmp_path / "docs" / "mklq").mkdir(parents=True)
    (tmp_path / ".github" / "workflows").mkdir(parents=True)
    (tmp_path / "pyproject.toml").write_text(
        "[project]\nname = \"mklq\"\nrequires-python = \">=3.11,<3.15\"\n",
        encoding="utf-8",
    )
    (tmp_path / "docs" / "mklq" / "release-policy.md").write_text(
        "mklq-vX.Y.Z TestPyPI PyPI mklq-metal experimental no .pkg\n",
        encoding="utf-8",
    )
    (tmp_path / "docs" / "mklq" / "release-notes-v0.1.0.md").write_text(
        "mklq-v0.1.0 mklq-cpu mklq-metal experimental TestPyPI PyPI\n",
        encoding="utf-8",
    )
    (tmp_path / "SECURITY.md").write_text(
        "Use GitHub private vulnerability reporting.\n", encoding="utf-8")
    (tmp_path / ".github" / "workflows" / "mklq-package-release.yml").write_text(
        "workflow_dispatch\nid-token: write\ntestpypi\npypi\n"
        "attest-build-provenance\npython: \"3.11\"\npython: \"3.12\"\n"
        "python: \"3.13\"\npython: \"3.14\"\n"
        "Verify trusted runner Python provisioning\nlipo -archs\n"
        "Reinstall and validate the published package\n"
        "https://test.pypi.org/simple\nhttps://pypi.org/simple\n",
        encoding="utf-8",
    )

    report = audit.build_report(tmp_path, version="0.1.0", docs_only=True)
    workflow_check = next(
        check for check in report["checks"]
        if check["name"] == ".github/workflows/mklq-package-release.yml")

    assert workflow_check["status"] == "failed"
    assert "mklq-v${{ inputs.version }}rc1" in workflow_check["details"]["missing"]
    assert "MKLQ_ASSET_ROOT=${RUNNER_TEMP}/mklq-release-assets-" in workflow_check[
        "details"]["missing"]


def test_public_hygiene_and_branch_protection_use_package_release_terms():
    workflow = (REPO_ROOT / ".github" / "workflows" /
                "mklq-public-hygiene.yml").read_text(encoding="utf-8")
    protection = (REPO_ROOT / ".github" /
                  "branch-protection-main.json").read_text(encoding="utf-8")

    assert "name: MKL-Q repository checks" in workflow
    assert "run_package_release_audit.py --version 0.1.0 --docs-only" in workflow
    assert '"MKL-Q repository checks"' in protection


def test_package_release_workflow_uploads_only_wheels_to_pypi():
    workflow = (REPO_ROOT / ".github" / "workflows" /
                "mklq-package-release.yml").read_text(encoding="utf-8")

    assert 'echo "MKLQ_ASSET_ROOT=${RUNNER_TEMP}/mklq-release-assets-' in workflow
    assert "MKLQ_ASSET_ROOT: ${{ runner.temp }}" not in workflow
    assert 'asset_root="${MKLQ_ASSET_ROOT}"' in workflow
    assert 'bash scripts/build_wheel.sh -m -o "${asset_root}"' in workflow
    assert "git diff --exit-code" in workflow
    assert "git status --porcelain --untracked-files=all" in workflow
    assert 'mkdir -p "${asset_root}/pypi"' in workflow
    assert 'mv "${asset_root}"/mklq-*.whl "${asset_root}/pypi/"' in workflow
    assert "pattern: mklq-${{ inputs.version }}-py*-macos-arm64" in workflow
    assert "merge-multiple: false" in workflow
    assert "packages-dir: release-assets/pypi" in workflow
    assert "subject-path: ${{ env.MKLQ_ASSET_ROOT }}/pypi/*.whl" in workflow
    assert '$(git rev-parse origin/main)' in workflow
    assert 'repos/${{ github.repository }}/private-vulnerability-reporting' in workflow
    assert "--jq '.enabled'" in workflow
    assert "private_vulnerability_reporting.status" not in workflow
    assert "gh run list" in workflow
    assert "shasum -a 256 -c ../SHA256SUMS" in workflow
    assert '(cd "${asset_root}/pypi" && shasum -a 256 -c ../SHA256SUMS)' in workflow
    assert "(cd release-assets/pypi && shasum -a 256 -c ../SHA256SUMS)" in workflow
    assert '(cd "${asset_root}" && shasum -a 256 -c SHA256SUMS)' not in workflow
    assert "(cd release-assets && shasum -a 256 -c SHA256SUMS)" not in workflow
    assert "otool -L" in workflow
    assert "otool -L \"${native_file}\" | sed '1d' >> \"${asset_root}/otool-L.txt\"" in workflow
    assert "lipo -archs" in workflow
    assert "pip check" in workflow
    assert "strategy:" in workflow
    for version in ("3.11", "3.12", "3.13", "3.14"):
        assert f'python: "{version}"' in workflow
        assert f"python{version}" in workflow
    assert "Verify trusted runner Python provisioning" in workflow
    assert "actions/setup-python" not in workflow
    assert "platform.machine()" in workflow
    assert "sysconfig.get_platform()" in workflow
    assert "Reinstall and validate the published package" in workflow
    assert 'cat > "${smoke_script}" <<\'PY\'' in workflow
    assert '"${smoke_root}/venv/bin/python" "${smoke_script}"' in workflow
    assert '"${PYTHON}" "${smoke_script}"' in workflow
    assert "https://test.pypi.org/simple" in workflow
    assert "https://pypi.org/simple" in workflow
    assert "cudaq.__version__" in workflow
    assert "SHA256SUMS" in workflow
    assert "git rev-parse \"${{ inputs.tag }}^{commit}\"" in workflow


def test_package_release_workflow_requires_rc1_for_pure_final_same_commit():
    workflow = (REPO_ROOT / ".github" / "workflows" /
                "mklq-package-release.yml").read_text(encoding="utf-8")

    assert 'rc_tag="mklq-v${{ inputs.version }}rc1"' in workflow
    assert 'git rev-parse "${rc_tag}^{commit}"' in workflow
    assert '"${{ inputs.version }}" != *.post*' in workflow


def test_package_public_claim_boundary_check_passes():
    result = subprocess.run(
        [
            os.environ.get("PYTHON", "python3"),
            "benchmarks/mklq/check_public_claims.py",
            "--root",
            str(REPO_ROOT),
        ],
        cwd=REPO_ROOT,
        capture_output=True,
        text=True,
    )

    assert result.returncode == 0, result.stdout + result.stderr


def test_release_facing_docs_make_package_audits_current_and_source_notes_historical():
    branch_policy = (REPO_ROOT / "docs" / "mklq" /
                     "branch-protection.md").read_text(encoding="utf-8")
    testing_matrix = (REPO_ROOT / "docs" / "mklq" /
                      "testing-matrix.md").read_text(encoding="utf-8")
    developer_workflow = (REPO_ROOT / "docs" / "mklq" /
                          "developer-workflow.md").read_text(encoding="utf-8")
    benchmark_readme = (REPO_ROOT / "benchmarks" / "mklq" /
                        "README.md").read_text(encoding="utf-8")
    maintainer_runbook = (REPO_ROOT / "docs" / "mklq" /
                          "maintainer-runbook.md").read_text(encoding="utf-8")

    assert "MKL-Q repository checks" in branch_policy
    assert "package-release-aware" in branch_policy
    assert "Package release audit" in testing_matrix
    assert "run_package_release_audit.py" in testing_matrix
    assert "run_package_release_audit.py" in developer_workflow
    assert "historical source-only" in developer_workflow
    assert "run_package_release_audit.py" in benchmark_readme
    assert "historical source-only" in benchmark_readme
    assert "MKL-Q repository checks" in maintainer_runbook
    assert "package-release project" in maintainer_runbook


def test_public_healthcheck_selects_mklq_package_metadata_contract(tmp_path):
    healthcheck = load_public_healthcheck_module()
    (tmp_path / "pyproject.toml").write_text(
        "[project]\nname = \"mklq\"\n", encoding="utf-8")

    requirements = healthcheck.public_metadata_requirements_for(tmp_path)

    assert healthcheck.is_package_release(tmp_path) is True
    assert ("README.md", "python -m pip install mklq") in requirements
    assert ("README.md", "source-only-rc-v0.1.md") not in requirements


def test_package_aware_audits_select_the_mklq_release_contract(tmp_path):
    (tmp_path / "pyproject.toml").write_text(
        "[project]\nname = \"mklq\"\n", encoding="utf-8")
    (tmp_path / "SECURITY.md").write_text(
        "GitHub private vulnerability reporting is enabled.\n", encoding="utf-8")

    readiness = load_public_readiness_audit_module()
    self_hosted = load_self_hosted_ci_audit_module()
    preflight = load_preflight_audit_module()

    assert readiness.required_status_check_for(tmp_path) == "MKL-Q repository checks"
    assert readiness.expected_workflows_for(tmp_path) == [
        ".github/workflows/mklq-apple-silicon-ci.yml",
        ".github/workflows/mklq-package-release.yml",
        ".github/workflows/mklq-public-hygiene.yml",
    ]
    assert readiness.PACKAGE_RELEASE_TAG_PATTERN.fullmatch(
        "mklq-v0.1.0.post1")
    assert self_hosted.expected_workflows_for(tmp_path) == [
        ".github/workflows/mklq-apple-silicon-ci.yml",
        ".github/workflows/mklq-package-release.yml",
        ".github/workflows/mklq-public-hygiene.yml",
    ]
    assert preflight.required_status_check_for(tmp_path) == "MKL-Q repository checks"
    config = preflight.PreflightConfig(
        repo_root=tmp_path,
        repo="wuls968/MKL-Q",
        output=tmp_path / "preflight.json",
        require_clean=True,
        check_github=False,
    )
    assert preflight.check_security_reporting(config)["status"] == "passed"
