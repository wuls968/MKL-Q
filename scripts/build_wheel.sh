#!/bin/bash

# ============================================================================ #
# Copyright (c) 2022 - 2026 NVIDIA Corporation & Affiliates.                   #
# All rights reserved.                                                         #
#                                                                              #
# This source code and the accompanying materials are made available under     #
# the terms of the Apache License 2.0 which accompanies this distribution.     #
# ============================================================================ #

# Unified wheel build script for Linux and macOS.
#
# Usage:
#   bash scripts/build_wheel.sh              # macOS only (CPU only)
#   bash scripts/build_wheel.sh -c 12        # Linux: build cu12 wheel
#   bash scripts/build_wheel.sh -c 13        # Linux: build cu13 wheel
#
# Options:
#   -c <cuda_version>: CUDA variant, 12 or 13 (Linux only)
#   -o <output_dir>: Output directory (default: dist; its parent must already exist outside the repository for -m)
#   -a <assets_dir>: Directory containing external simulator assets (default: assets)
#   -t: Run validation tests after build
#   -q: Quick test mode (only run core tests, implies -t)
#   -p: Install prerequisites before building
#   -T <toolchain>: Toolchain to use with prerequisites (e.g., gcc12, llvm)
#   -i: Incremental build (reuse existing build artifacts)
#   -m: Build the macOS ARM64 MKL-Q release wheel
#   -v: Verbose output
#
# Environment variables:
#   PYTHON: Python interpreter to use (default: python3)
#   MKLQ_VERSION: Required PEP 440 version for -m release wheels
#   CUDA_QUANTUM_VERSION: Version string for the wheel (default: 0.0.0)
#   CUDACXX: Path to nvcc compiler
#   CUDAHOSTCXX: Host compiler for CUDA

set -euo pipefail

# Run from repo root
this_file_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root=$(cd "$this_file_dir" && git rev-parse --show-toplevel)
cd "$repo_root"
repo_root_real=$(pwd -P)

# Detect platform
platform=$(uname)
arch=$(uname -m)

# Default values
cuda_variant=""
output_dir="dist"
assets_dir="assets"
external_assets_requested=false
run_tests=false
quick_test=false
install_prereqs=false
install_toolchain=""
incremental=false
mklq_release=false
verbose=false

# Parse command line arguments
__optind__=$OPTIND
OPTIND=1
while getopts ":c:o:a:tqpT:ivm" opt; do
    case $opt in
    c)
        cuda_variant="$OPTARG"
        ;;
    o)
        output_dir="$OPTARG"
        ;;
    a)
        assets_dir="$OPTARG"
        external_assets_requested=true
        ;;
    t)
        run_tests=true
        ;;
    q)
        quick_test=true
        run_tests=true
        ;;
    p)
        install_prereqs=true
        ;;
    T)
        install_prereqs=true
        install_toolchain="$OPTARG"
        ;;
    i)
        incremental=true
        ;;
    m)
        mklq_release=true
        ;;
    v)
        verbose=true
        ;;
    \?)
        echo "Invalid command line option -$OPTARG" >&2
        exit 1
        ;;
    esac
done
OPTIND=$__optind__

if $verbose; then
    echo "Verbose mode enabled"
    echo "Platform: $platform ($arch)"
fi

# Install prerequisites (opt-in with -p or -T)
# When installing prerequisites, we also set default install prefix env vars
# so CMake knows where to find them. Without -p/-T, CMake uses standard discovery.
if $install_prereqs; then
    # Set defaults for where prerequisites will be installed
    source "$this_file_dir/set_env_defaults.sh"

    echo "Installing prerequisites..."
    # Save and clear positional parameters to avoid passing them to sourced script
    saved_args=("$@")
    if [ -n "$install_toolchain" ]; then
        set -- -t "$install_toolchain"
    else
        set --
    fi
    if $verbose; then
        source "$this_file_dir/install_prerequisites.sh" "$@"
        prereq_exit=$?
    else
        source "$this_file_dir/install_prerequisites.sh" "$@" 2>&1 | tail -5
        prereq_exit=$?
    fi
    # Restore positional parameters
    set -- "${saved_args[@]}"
    if [ $prereq_exit -ne 0 ]; then
        echo "Error: Failed to install prerequisites" >&2
        exit 1
    fi
fi

# Determine CUDA variant
if $mklq_release; then
    if [ "$platform" != "Darwin" ] || [ "$arch" != "arm64" ]; then
        echo "MKL-Q release wheels require macOS arm64 (found $platform $arch)." >&2
        exit 1
    fi
    if $external_assets_requested; then
        echo "MKL-Q release wheels do not accept -a external simulator assets." >&2
        exit 1
    fi
    case "$output_dir" in
    /*)
        ;;
    *)
        echo "MKL-Q release wheels must use an absolute output directory outside the repository." >&2
        exit 1
        ;;
    esac
    case "$output_dir" in
    "$repo_root_real"|"$repo_root_real"/*)
        echo "MKL-Q release output directory must be outside the repository." >&2
        exit 1
        ;;
    esac
    output_parent=$(dirname "$output_dir")
    if [ ! -d "$output_parent" ]; then
        echo "MKL-Q release output parent must already exist and be outside the repository." >&2
        exit 1
    fi
    output_parent=$(cd "$output_parent" && pwd -P)
    case "$output_parent" in
    "$repo_root_real"|"$repo_root_real"/*)
        echo "MKL-Q release output parent resolves inside the repository." >&2
        exit 1
        ;;
    esac
    output_dir="$output_parent/$(basename "$output_dir")"
    if [ -e "$output_dir" ]; then
        output_dir=$(cd "$output_dir" && pwd -P)
    fi
    case "$output_dir" in
    "$repo_root_real"|"$repo_root_real"/*)
        echo "MKL-Q release output directory resolves inside the repository." >&2
        exit 1
        ;;
    esac
    cuda_variant="13"
    echo "MKL-Q release: building macOS arm64 Python runtime wheel"
elif [ "$platform" = "Darwin" ]; then
    # macOS: CPU-only build. Uses cu13 pyproject but CUDA deps are excluded
    # via sys_platform markers in pyproject.toml.cu13. The cu13 variant is
    # the default fallback when no CUDA is detected (applies to Linux too).
    cuda_variant="13"
    echo "macOS: building cu$cuda_variant wheel (CPU-only)"
else
    # Linux: require explicit -c option
    if [ -z "$cuda_variant" ]; then
        echo "Error: CUDA variant required. Use -c 12 or -c 13" >&2
        exit 1
    fi
    if [ "$cuda_variant" != "12" ] && [ "$cuda_variant" != "13" ]; then
        echo "Error: CUDA variant must be 12 or 13, got: $cuda_variant" >&2
        exit 1
    fi
    echo "Linux: building cu$cuda_variant wheel"
fi

# Set up Python
python="${PYTHON:-python3}"
if ! command -v "$python" &>/dev/null; then
    echo "Error: $python not found" >&2
    exit 1
fi
echo "Using Python: $($python --version)"

if $mklq_release; then
    python_machine=$("$python" -c 'import platform; print(platform.machine())')
    python_platform=$("$python" -c 'import sysconfig; print(sysconfig.get_platform())')
    if [ "$python_machine" != "arm64" ] || [[ "$python_platform" != *arm64* ]]; then
        echo "MKL-Q release wheels require an arm64 Python interpreter (machine=$python_machine, platform=$python_platform)." >&2
        exit 1
    fi
    if [ -z "${MKLQ_VERSION:-}" ]; then
        echo "MKLQ_VERSION is required for an MKL-Q release wheel." >&2
        exit 1
    fi
    if ! "$python" benchmarks/mklq/package_release.py validate-version \
        "$MKLQ_VERSION"; then
        echo "MKLQ_VERSION is not a supported release version." >&2
        exit 1
    fi
    submodule_status="$(git submodule status --recursive)" || {
        echo "MKL-Q release wheels could not inspect submodule status." >&2
        exit 1
    }
    if printf '%s\n' "$submodule_status" | grep -Eq '^[+U-]'; then
        echo "MKL-Q release wheels require initialized submodules at exact submodule gitlink revisions." >&2
        echo "Run: git submodule update --init --recursive" >&2
        exit 1
    fi
fi

# Copy appropriate pyproject.toml
if $mklq_release; then
    echo "Using MKL-Q root pyproject.toml without rewriting the source tree"
else
    pyproject_src="pyproject.toml.cu${cuda_variant}"
    if [ ! -f "$pyproject_src" ]; then
        echo "Error: $pyproject_src not found" >&2
        exit 1
    fi
    echo "Using pyproject: $pyproject_src"
    cp -f "$pyproject_src" pyproject.toml 2>/dev/null || true
fi

# Generate README.md from template
if ! $mklq_release && [ -f "python/README.md.in" ]; then
  echo "Generating README from template..."
  cp python/README.md.in python/README.md
  
  # Set template variables (matching original Dockerfile logic)
  # CUDA_VERSION is the full version (e.g., "12.6"), cuda_variant is major only (e.g., "12")
  package_name="cuda-quantum-cu${cuda_variant}"
  cuda_version_full="${CUDA_VERSION:-${cuda_variant}.0}"
  cuda_version_requirement=">= ${cuda_version_full}"
  cuda_version_conda="${cuda_version_full}.0"
  # Map conda version 13.0.0 -> 13.0.2 (conda channel doesn't have 13.0.0)
  cuda_version_conda="${cuda_version_conda/13.0.0/13.0.2}"
  deprecation_notice=""  # No deprecation notice by default
  
  # Perform substitutions
  # The template uses ${{ variable }} syntax - we use .{{ to match any char before {{
  for variable in package_name cuda_version_requirement cuda_version_conda deprecation_notice; do
    value="${!variable}"
    # Escape special characters in value for sed replacement
    escaped_value=$(printf '%s\n' "$value" | sed 's/[&/\]/\\&/g')
    if [ "$platform" = "Darwin" ]; then
      sed -i '' "s/.{{[ ]*${variable}[ ]*}}/${escaped_value}/g" python/README.md
    else
      sed -i "s/.{{[ ]*${variable}[ ]*}}/${escaped_value}/g" python/README.md
    fi
  done
  
  # Verify all substitutions were made (use .{{ to match ${{ or any prefix)
  if grep -q '.{{.*}}' python/README.md; then
    echo "Error: Incomplete template substitutions in README.md" >&2
    grep '.{{.*}}' python/README.md >&2
    exit 1
  fi
fi

# Set up library path environment variable
if [ "$platform" = "Darwin" ]; then
    lib_path_var="DYLD_LIBRARY_PATH"
    lib_ext="dylib"
else
    lib_path_var="LD_LIBRARY_PATH"
    lib_ext="so"
fi

# Find external NVQIR simulator assets. Package releases intentionally never
# consume this escape hatch: the manifest must contain only MKL-Q backends.
if $mklq_release; then
    unset CUDAQ_EXTERNAL_NVQIR_SIMS
else
    export CUDAQ_EXTERNAL_NVQIR_SIMS=$(bash scripts/find_wheel_assets.sh "$assets_dir")
    if [ -n "$CUDAQ_EXTERNAL_NVQIR_SIMS" ]; then
        echo "Found external simulator assets: $CUDAQ_EXTERNAL_NVQIR_SIMS"
        eval "export $lib_path_var=\"\${$lib_path_var:+\$$lib_path_var:}$(pwd)/$assets_dir\""
    fi
fi

# Set version
if $mklq_release; then
    export SETUPTOOLS_SCM_PRETEND_VERSION="$MKLQ_VERSION"
else
    export SETUPTOOLS_SCM_PRETEND_VERSION=${CUDA_QUANTUM_VERSION:-0.0.0}
fi
echo "Building wheel version: $SETUPTOOLS_SCM_PRETEND_VERSION"

# Set CUDA compiler if available (Linux only)
if [ "$platform" != "Darwin" ]; then
    if [ -n "${CUDACXX:-}" ]; then
        export CUDACXX
    elif [ -f "${CUDA_HOME:-/usr/local/cuda}/bin/nvcc" ]; then
        export CUDACXX="${CUDA_HOME:-/usr/local/cuda}/bin/nvcc"
    fi
    if [ -n "${CUDAHOSTCXX:-}" ]; then
        export CUDAHOSTCXX
    elif [ -n "${CXX:-}" ]; then
        export CUDAHOSTCXX="$CXX"
    fi
fi

# Clean previous build artifacts (unless incremental). The MKL-Q release
# path uses a private temporary build directory and never rewrites this tree.
if $mklq_release; then
    mkdir -p "$output_dir"
else
    if $incremental; then
        echo "Incremental build: reusing existing build artifacts"
        rm -rf dist/*.whl "$output_dir"/*.whl 2>/dev/null || true
    else
        rm -rf _skbuild dist/*.whl "$output_dir"/*.whl 2>/dev/null || true
    fi
    mkdir -p "$output_dir"
fi

# Configure OpenMP for parallel execution performance
# Uses same logic as build_cudaq.sh, with Homebrew fallback for macOS wheel builds

# Try LLVM_INSTALL_PREFIX first (same as build_cudaq.sh)
OpenMP_libomp_LIBRARY_PATH=""
OpenMP_SEARCH_PREFIX=""
if [ -n "${LLVM_INSTALL_PREFIX:-}" ]; then
    OpenMP_libomp_LIBRARY_PATH=$(find "$LLVM_INSTALL_PREFIX" \
        \( -name 'libomp.so' -o -name 'libomp.dylib' \) -print -quit 2>/dev/null)
    if [ -n "$OpenMP_libomp_LIBRARY_PATH" ]; then
        OpenMP_SEARCH_PREFIX="$LLVM_INSTALL_PREFIX"
    fi
fi

# Fallback to Homebrew on macOS if not found in LLVM_INSTALL_PREFIX
if [ -z "$OpenMP_libomp_LIBRARY_PATH" ] && [ "$platform" = "Darwin" ]; then
    for brew_path in /opt/homebrew/opt/libomp /usr/local/opt/libomp; do
        if [ -d "$brew_path" ]; then
            OpenMP_libomp_LIBRARY_PATH=$(find "$brew_path" -name 'libomp.dylib' \
                -print -quit 2>/dev/null)
            if [ -n "$OpenMP_libomp_LIBRARY_PATH" ]; then
                OpenMP_SEARCH_PREFIX="$brew_path"
                break
            fi
        fi
    done
fi
if [ -n "$OpenMP_libomp_LIBRARY_PATH" ]; then
    omp_header_dir=$(find "$OpenMP_SEARCH_PREFIX" -name 'omp.h' -print -quit 2>/dev/null | xargs dirname)
    # Use -idirafter so omp.h is searched after system headers (avoids a
    # conflict with clang's stdint.h on macOS).
    OpenMP_FLAGS="${OpenMP_FLAGS:--fopenmp -idirafter $omp_header_dir}"
    echo "OpenMP found: $OpenMP_libomp_LIBRARY_PATH"
else
    echo "OpenMP not found - wheel will be built without OpenMP parallelization"
    if [ "$platform" = "Darwin" ]; then
        echo "  Option 1: Rebuild with -p flag (includes OpenMP by default on macOS)"
        echo "  Option 2: brew install libomp"
    else
        echo "  Option 1: Set LLVM_PROJECTS to include openmp and rebuild with -p flag:"
        echo "            export LLVM_PROJECTS='clang;lld;mlir;python-bindings;openmp'"
        echo "  Option 2: apt install libomp-dev  # or: yum install libomp-devel"
    fi
fi

# Build CMAKE_ARGS for OpenMP (same variables as build_cudaq.sh lines 249-253)
CMAKE_ARGS="${OpenMP_libomp_LIBRARY_PATH:+-DOpenMP_C_LIB_NAMES=omp}"
CMAKE_ARGS="$CMAKE_ARGS ${OpenMP_libomp_LIBRARY_PATH:+-DOpenMP_CXX_LIB_NAMES=omp}"
CMAKE_ARGS="$CMAKE_ARGS ${OpenMP_libomp_LIBRARY_PATH:+-DOpenMP_omp_LIBRARY=$OpenMP_libomp_LIBRARY_PATH}"
CMAKE_ARGS="$CMAKE_ARGS ${OpenMP_FLAGS:+-DOpenMP_C_FLAGS=\"$OpenMP_FLAGS\"}"
CMAKE_ARGS="$CMAKE_ARGS ${OpenMP_FLAGS:+-DOpenMP_CXX_FLAGS=\"$OpenMP_FLAGS\"}"
if $verbose && [ -n "$OpenMP_libomp_LIBRARY_PATH" ]; then
    echo "OpenMP CMAKE_ARGS: $CMAKE_ARGS"
fi
# Check for ccache and add compiler launcher to CMAKE_ARGS
if [ -x "$(command -v ccache)" ]; then
    echo "ccache detected enabling in cmake"
    CMAKE_ARGS="$CMAKE_ARGS -DCMAKE_C_COMPILER_LAUNCHER=ccache"
    CMAKE_ARGS="$CMAKE_ARGS -DCMAKE_CXX_COMPILER_LAUNCHER=ccache"
    if [ -n "${CUDACXX:-}" ]; then
        CMAKE_ARGS="$CMAKE_ARGS -DCMAKE_CUDA_COMPILER_LAUNCHER=ccache"
    fi
fi
if $mklq_release; then
    # macOS SDKs do not ship static zlib archives. The release wheel is repaired
    # with delocate below, so it must link dynamic third-party dependencies.
    CMAKE_ARGS="$CMAKE_ARGS -DCMAKE_OSX_ARCHITECTURES=arm64 -DCUDAQ_ENABLE_MKLQ_BACKEND=ON -DCUDAQ_MKLQ_PACKAGE_ONLY=ON -DCUDAQ_ENABLE_ALL_BACKEND=OFF -DCUDAQ_BUILD_TESTS=FALSE -DCUDAQ_STATIC_DEPS=OFF -DGIT_SUBMODULE=OFF"
fi

export CMAKE_ARGS

# Build the wheel
echo "Building wheel..."
if $mklq_release; then
    release_tmp=$(mktemp -d "${TMPDIR:-/tmp}/mklq-wheel.XXXXXX")
    trap 'rm -rf "${release_tmp:-}"' EXIT
    wheel_stage="$release_tmp/wheel"
    mkdir -p "$wheel_stage"
    build_command=("$python" -m build --wheel --outdir "$wheel_stage"
        "-Cbuild-dir=$release_tmp/build")
elif $verbose; then
    echo "  Command: $python -m build --wheel"
    echo "  SETUPTOOLS_SCM_PRETEND_VERSION=$SETUPTOOLS_SCM_PRETEND_VERSION"
    if [ -n "$CUDAQ_EXTERNAL_NVQIR_SIMS" ]; then
        echo "  CUDAQ_EXTERNAL_NVQIR_SIMS=$CUDAQ_EXTERNAL_NVQIR_SIMS"
    fi
    echo ""
    $python -m build --wheel -v
else
    $python -m build --wheel 2>&1 | tail -20
fi

if $mklq_release; then
    if $verbose; then
        printf '  Command:'
        printf ' %q' "${build_command[@]}"
        printf '\n'
        "${build_command[@]}"
    else
        "${build_command[@]}" 2>&1 | tail -20
    fi
fi

# Find the built wheel
if $mklq_release; then
    wheel_file=$(find "$wheel_stage" -maxdepth 1 -type f -name 'mklq-*.whl' \
        -print -quit)
else
    wheel_file=$(find dist -maxdepth 1 -type f -name 'cuda_quantum*.whl' \
        -print -quit 2>/dev/null)
fi
if [ -z "$wheel_file" ]; then
    echo "Error: No wheel file found in dist/" >&2
    exit 1
fi
echo "Built wheel: $wheel_file"

# Repair the wheel (bundle dependencies)
echo "Repairing wheel..."
if $verbose; then
    echo "  Input wheel: $wheel_file"
fi

if [ "$platform" = "Darwin" ]; then
    # macOS: use delocate
    if ! command -v delocate-wheel &>/dev/null; then
        echo "Error: delocate not found. Install with: pip install -r requirements-dev.txt" >&2
        exit 1
    fi

    # delocate repairs the wheel and copies it to wheelhouse/.
    # With @loader_path rpaths, delocate can resolve inter-library
    # references.
    if $mklq_release; then
        wheelhouse="$release_tmp/wheelhouse"
    else
        wheelhouse="wheelhouse"
    fi
    mkdir -p "$wheelhouse"
    if $verbose; then
        echo "  Command: delocate-wheel -v -w $wheelhouse $wheel_file"
        delocate-wheel -v -w "$wheelhouse" "$wheel_file"
    else
        delocate-wheel -w "$wheelhouse" "$wheel_file"
    fi

    # Move repaired wheel to output
    if $mklq_release; then
        repaired_wheel=$(find "$wheelhouse" -maxdepth 1 -type f -name 'mklq-*.whl' \
            -print -quit)
    else
        repaired_wheel=$(find "$wheelhouse" -maxdepth 1 -type f \
            -name 'cuda_quantum*.whl' -print -quit 2>/dev/null)
    fi
    if [ -n "$repaired_wheel" ]; then
        mv "$repaired_wheel" "$output_dir/"
        echo "Repaired wheel: $output_dir/$(basename "$repaired_wheel")"
    else
        # If delocate didn't produce output, use original
        mv "$wheel_file" "$output_dir/"
        echo "Wheel (no repair needed): $output_dir/$(basename "$wheel_file")"
    fi
    if ! $mklq_release; then
        rm -rf wheelhouse
    fi
else
    # Linux: use auditwheel
    if ! command -v auditwheel &>/dev/null; then
        echo "Error: auditwheel not found. Install with: pip install -r requirements-dev.txt" >&2
        exit 1
    fi

    # Determine CUDA library exclusions
    cuda_major="$cuda_variant"
    cudart_libsuffix=$([ "$cuda_major" = "11" ] && echo "11.0" || echo "12")

    # Add build lib to library path for auditwheel
    eval "export $lib_path_var=\"\${$lib_path_var:+\$$lib_path_var:}$(pwd)/_skbuild/lib\""

    # Use temp directory that won't conflict with output_dir
    auditwheel_tmp="_auditwheel_tmp"
    rm -rf "${auditwheel_tmp:?}"
    mkdir -p "$auditwheel_tmp"
    auditwheel_args="repair $wheel_file -w $auditwheel_tmp"
    auditwheel_args="$auditwheel_args --exclude libcustatevec.so.1"
    auditwheel_args="$auditwheel_args --exclude libcutensornet.so.2"
    auditwheel_args="$auditwheel_args --exclude libcudensitymat.so.0"
    auditwheel_args="$auditwheel_args --exclude libcublas.so.$cuda_major"
    auditwheel_args="$auditwheel_args --exclude libcublasLt.so.$cuda_major"
    auditwheel_args="$auditwheel_args --exclude libcurand.so.10"
    auditwheel_args="$auditwheel_args --exclude libcusolver.so.11"
    auditwheel_args="$auditwheel_args --exclude libcusparse.so.$cuda_major"
    auditwheel_args="$auditwheel_args --exclude libcutensor.so.2"
    auditwheel_args="$auditwheel_args --exclude libnvToolsExt.so.1"
    auditwheel_args="$auditwheel_args --exclude libcudart.so.$cudart_libsuffix"
    auditwheel_args="$auditwheel_args --exclude libnvidia-ml.so.1"
    auditwheel_args="$auditwheel_args --exclude libcuda.so.1"

    if $verbose; then
        echo "  Command: auditwheel $auditwheel_args"
        auditwheel -v $auditwheel_args
    else
        auditwheel $auditwheel_args
    fi

    # Move repaired wheel to output
    repaired_wheel=$(find "${auditwheel_tmp:?}" -maxdepth 1 -type f \
        -name '*manylinux*.whl' -print -quit 2>/dev/null)
    if [ "$(uname -m)" = "x86_64" ] && [ -n "$repaired_wheel" ]; then
        if ! unzip -l "$repaired_wheel" 2>/dev/null | grep -q 'libqrmi'; then
            echo "WARNING: libqrmi.so not bundled in x86_64 wheel"
        else
            echo "Verified libqrmi.so is bundled in x86_64 wheel"
        fi
    fi
    if [ -n "$repaired_wheel" ]; then
        mv "$repaired_wheel" "$output_dir/"
        echo "Repaired wheel: $output_dir/$(basename "$repaired_wheel")"
    else
        mv "$wheel_file" "$output_dir/"
        echo "Wheel: $output_dir/$(basename "$wheel_file")"
    fi
    rm -rf "${auditwheel_tmp:?}"
fi

if $mklq_release; then
    manifest_path="$output_dir/$(basename "${repaired_wheel:-$wheel_file}" .whl).manifest.json"
    "$python" benchmarks/mklq/package_release.py inspect-wheel \
        "$output_dir/$(basename "${repaired_wheel:-$wheel_file}")" \
        --expected-version "$MKLQ_VERSION" \
        --output "$manifest_path"
fi

echo "Done! Wheel available in $output_dir/"

# Run validation tests if requested
if $run_tests; then
    echo ""
    echo "Running validation tests..."

    # Build validation script arguments (auto-detects test files from repo)
    validate_args="-v $SETUPTOOLS_SCM_PRETEND_VERSION -i $output_dir"

    if $quick_test; then
        validate_args="$validate_args -q"
    fi

    # Add CUDA version for Linux
    if [ "$platform" != "Darwin" ]; then
        # Determine full CUDA version for conda (e.g., 12.6.0)
        if [ "$cuda_variant" = "12" ]; then
            cuda_version_conda="${CUDA_VERSION_CONDA:-12.6.0}"
        else
            cuda_version_conda="${CUDA_VERSION_CONDA:-13.0.0}"
        fi
        validate_args="$validate_args -c $cuda_version_conda"
    fi

    # Run validation (will auto-detect test files from repo)
    if $verbose; then
        echo "  Command: bash $this_file_dir/validate_pycudaq.sh $validate_args"
    fi
    bash "$this_file_dir/validate_pycudaq.sh" $validate_args
    if [ $? -ne 0 ]; then
        echo "Validation failed!" >&2
        exit 1
    fi
    echo "Validation passed!"
fi
