# MKL-Q Apple Silicon CI

This page defines the self-hosted Apple Silicon correctness gate. The expensive
self-hosted job is manual by default, does not run on pull requests, and is not
release certification. This page is not release certification.

## Scope

The public `main` branch keeps required GitHub Actions lightweight. The required
`MKL-Q public hygiene` workflow checks source-only repository hygiene, public
metadata, public claim boundaries, benchmark summary parsing, and helper script
syntax. It does not build CUDA-Q and does not run Apple Silicon backend
correctness tests.

The `.github/workflows/mklq-apple-silicon-ci.yml` workflow covers local
simulator behavior for `mklq-cpu` and the experimental `mklq-metal` target on a
private macOS ARM64 runner. The full self-hosted job is available only through
`workflow_dispatch`, defaults `run_full_gate` to `skip`, and runs the full gate
only when a maintainer explicitly starts it with `run_full_gate=confirm`.

It must remain source-only: no tags, no GitHub Releases, no wheels, no
installers, and no signed artifacts are produced by this job.

## Runner Requirements

Use a private self-hosted Apple Silicon runner with labels that make the target
platform explicit:

```text
self-hosted
macOS
ARM64
mklq-apple-silicon
```

The runner must provide:

- Apple Silicon hardware with a working Metal runtime.
- A supported macOS ARM64 developer environment.
- CMake, Ninja, Python 3, a compiler toolchain, and CUDA-Q build prerequisites.
- Enough local disk space for `build-python` and the install prefix.
- No persistent credentials, tokens, private keys, or `.env` files in the
  checkout, build directory, install prefix, or log output.

## Workflow Policy

The reviewed workflow set is:

```text
.github/workflows/mklq-public-hygiene.yml
.github/workflows/mklq-apple-silicon-ci.yml
```

Do not add pull-request triggers to the Apple Silicon workflow until a reviewed
activation PR assigns a maintenance owner, confirms runner availability, and
updates branch protection expectations. A broad push trigger on `main` is
allowed only for the lightweight `Dispatch guard`, which keeps pushed-readiness
audits tied to the latest `main` commit. The full self-hosted job remains
manual with `workflow_dispatch`, `run_full_gate`, and default skip activation.

The future workflow should use read-only repository permissions:

```yaml
permissions: contents: read
```

It also sets explicit `timeout-minutes` and `concurrency` values so a stuck
build cannot consume the runner indefinitely and superseded runs cancel cleanly.
The source checkout uses a checkout timeout and a manual sparse checkout instead
of `actions/checkout`: workspace cleanup is limited to `${GITHUB_WORKSPACE}`,
then the job runs `git init`, configures `git sparse-checkout set` for the
build, MKL-Q docs, benchmark harness, `docs/sphinx/examples/mklq`, and
`docs/sphinx/targets`, then fetches `origin/main` with
`http.version=HTTP/1.1`, `--depth=1`, and `--filter=blob:none` using the explicit
`+refs/heads/main:refs/remotes/origin/main` refspec.
The later remote-normalization step restores full-history evidence with retried
fetch commands using `http.version=HTTP/1.1`, `--unshallow`, and
`--filter=blob:none` with explicit `origin/main` and `upstream/main` refspecs
before the public healthcheck verifies that the checkout is no longer shallow.
Submodules are bootstrapped later by the reviewed retrying step.
For non-dispatch runs on `main`, only a small unconditional `Dispatch guard`
job runs on `ubuntu-latest`; the self-hosted Apple Silicon job still requires
`workflow_dispatch` with `run_full_gate=confirm`.

The job should not use secrets. If a future step needs credentials, split that
work into a separate reviewed release or deployment plan.

## Validation Command

The manual self-hosted correctness job builds from source and then runs the full
local public healthcheck:

```bash
python_bin="${MKLQ_PYTHON:-$(command -v python3)}"
install_prefix="${RUNNER_TEMP:-${HOME}}/cudaq-mklq-install"

retry_git() {
  local label="$1"
  shift
  local attempt=1
  local max_attempts=3
  local delay_seconds=10
  while true; do
    echo "Running ${label} (attempt ${attempt}/${max_attempts})."
    if "$@"; then
      return 0
    fi
    local status="$?"
    if [ "${attempt}" -ge "${max_attempts}" ]; then
      echo "${label} failed after ${attempt} attempts (exit ${status})." >&2
      return "${status}"
    fi
    echo "${label} failed with exit ${status}; retrying in ${delay_seconds}s." >&2
    sleep "${delay_seconds}"
    attempt=$((attempt + 1))
    delay_seconds=$((delay_seconds * 2))
  done
}

workspace="${GITHUB_WORKSPACE:?}"
if [ "${workspace}" = "/" ]; then
  echo "Refusing to clean filesystem root." >&2
  exit 1
fi
mkdir -p "${workspace}"
find "${workspace}" -mindepth 1 -maxdepth 1 -exec rm -rf {} +

git init "${workspace}"
cd "${workspace}"
git remote add origin https://github.com/wuls968/MKL-Q.git
git remote add upstream https://github.com/NVIDIA/cuda-quantum.git
git sparse-checkout init --cone
git sparse-checkout set \
  .github \
  benchmarks \
  cmake \
  cudaq \
  docs/mklq \
  docs/sphinx/examples/mklq \
  docs/sphinx/targets \
  python \
  runtime \
  scripts \
  targettests \
  tpls \
  unittests \
  utils
retry_git "origin main sparse fetch" \
  git -c http.version=HTTP/1.1 -c protocol.version=2 fetch \
  --no-tags --filter=blob:none --depth=1 \
  origin +refs/heads/main:refs/remotes/origin/main
retry_git "origin main sparse checkout" \
  git -c http.version=HTTP/1.1 -c protocol.version=2 checkout \
  --force -B main origin/main

git remote set-url origin https://github.com/wuls968/MKL-Q.git
git remote remove upstream 2>/dev/null || true
git remote add upstream https://github.com/NVIDIA/cuda-quantum.git
if [ "$(git rev-parse --is-shallow-repository)" = "true" ]; then
  retry_git "origin main unshallow" \
    git -c http.version=HTTP/1.1 -c protocol.version=2 fetch \
    --no-tags --filter=blob:none --unshallow \
    origin +refs/heads/main:refs/remotes/origin/main
else
  retry_git "origin main fetch" \
    git -c http.version=HTTP/1.1 -c protocol.version=2 fetch \
    --no-tags --filter=blob:none \
    origin +refs/heads/main:refs/remotes/origin/main
fi
retry_git "upstream main fetch" \
  git -c http.version=HTTP/1.1 -c protocol.version=2 fetch \
  --no-tags --filter=blob:none \
  upstream +refs/heads/main:refs/remotes/upstream/main

retry_git "submodule sync" git submodule sync --recursive
retry_git "non-LLVM submodule bootstrap" \
  git -c submodule.tpls/llvm.update=none submodule update \
  --init \
  --depth 1 \
  --jobs 6 \
  tpls/Stim \
  tpls/armadillo \
  tpls/cpr \
  tpls/eigen \
  tpls/ensmallen \
  tpls/fmt \
  tpls/googletest-src \
  tpls/nanobind \
  tpls/qpp \
  tpls/spdlog \
  tpls/xtensor \
  tpls/xtl
retry_git "nanobind robin_map submodule bootstrap" \
  git -C tpls/nanobind submodule update \
  --init \
  --depth 1 \
  ext/robin_map
rm -rf build-python "${install_prefix}"
cmake -S . -B build-python -G Ninja \
  -D CUDAQ_ENABLE_MKLQ_BACKEND=ON \
  -D CUDAQ_ENABLE_PROJECTS=python \
  -D GIT_SUBMODULE=OFF \
  -D CMAKE_INSTALL_PREFIX="${install_prefix}" \
  -D Python_EXECUTABLE="${python_bin}" \
  -D Python3_EXECUTABLE="${python_bin}"
"${python_bin}" benchmarks/mklq/run_public_healthcheck.py \
  --full \
  --require-clean \
  --focused-install-build \
  --install-prefix "${install_prefix}" \
  --python-executable "${python_bin}" \
  --pythonpath "${install_prefix}" \
  --nvqpp "${install_prefix}/bin/nvq++" \
  --build-dir build-python
```

The tracked workflow invokes the same gate after preparing a manual sparse
checkout with an explicit checkout timeout, selecting the effective Python,
normalizing `origin` and `upstream`, unshallowing `origin/main` with
`filter=blob:none`, `http.version=HTTP/1.1`, and an explicit main refspec,
fetching `upstream/main` with an explicit main refspec,
bootstrapping the non-LLVM source submodules with shallow fetches, retrying
transient submodule network failures up to three times, clearing the persistent
runner build/install directories, and configuring a fresh `build-python` tree
with `CUDAQ_ENABLE_PROJECTS=python` and `GIT_SUBMODULE=OFF`.
The `--focused-install-build` option builds the reviewed installable MKL-Q
targets and then runs `cmake --install`, avoiding unrelated upstream test
binaries that are not part of this source-only Apple Silicon gate.

```bash
"${MKLQ_EFFECTIVE_PYTHON}" benchmarks/mklq/run_public_healthcheck.py \
  --full \
  --require-clean \
  --focused-install-build \
  --install-prefix "${MKLQ_INSTALL_PREFIX}" \
  --python-executable "${MKLQ_EFFECTIVE_PYTHON}" \
  --pythonpath "${MKLQ_INSTALL_PREFIX}" \
  --nvqpp "${MKLQ_INSTALL_PREFIX}/bin/nvq++" \
  --build-dir "${MKLQ_BUILD_DIR}" \
  --jobs 6 \
  --timeout-seconds 1800
```

For focused debugging, the same runner may run the one-command correctness
gate directly:

```bash
python3 benchmarks/mklq/run_correctness_gate.py \
  --install-prefix "${HOME}/.cudaq-mklq" \
  --build-dir build-python
```

Raw JSON produced by these commands belongs under ignored
`benchmarks/mklq/results/`. Do not commit those payloads.

## Activation Checklist

Before making the full self-hosted job automatic or branch-protected:

- Confirm the runner labels include `self-hosted`, `macOS`, `ARM64`, and
  `mklq-apple-silicon`.
- Confirm the live GitHub runner inventory has an online runner with the
  required labels before dispatching the full gate:

  ```bash
  python3 benchmarks/mklq/run_self_hosted_ci_audit.py \
    --check-runners \
    --repo wuls968/MKL-Q
  ```

  This uses the GitHub `actions/runners` API and is expected to fail when no
  private self-hosted runner is registered.
- Confirm manual `workflow_dispatch` runs with `run_full_gate=confirm` complete
  successfully on the intended runner.
- Confirm the runner has no secrets, signing assets, or personal credentials in
  the working directory.
- Confirm the job uses `permissions: contents: read`.
- Confirm the job has explicit `timeout-minutes` and `concurrency` settings.
- Confirm checkout uses manual sparse checkout with a checkout timeout, cleans
  only `${GITHUB_WORKSPACE}`, does not use `actions/checkout`, keeps credentials
  out of the working tree, and restores full-history evidence later with
  retried `http.version=HTTP/1.1` unshallow fetches using `filter=blob:none`.
- Confirm source submodule bootstrapping is a visible step before CMake
  configure, uses limited retry for transient submodule network failures, and
  CMake uses `GIT_SUBMODULE=OFF`.
- Confirm the command is
  `run_public_healthcheck.py --full --require-clean --focused-install-build` or
  a stricter reviewed replacement.
- Confirm the workflow produces no release artifacts.
- Confirm branch protection expectations are updated only after the new job is
  stable.

## Failure Handling

Treat a failing self-hosted correctness run as backend evidence, not as a
release-blocking package failure. First identify whether the failure is:

- build environment drift;
- missing dependency;
- `mklq-cpu` correctness regression;
- `mklq-metal` experimental runtime or Metal device issue;
- upstream CUDA-Q compatibility change;
- stale docs, counters, or public healthcheck expectations.

Keep raw logs and JSON local unless a sanitized summary is needed for a public
issue or pull request.

## Security Boundary

The Apple Silicon runner should be treated as a build worker, not as a release
or deployment host. It must not publish packages, create tags, create GitHub
Releases, upload wheels, sign binaries, or access private services.

This plan deliberately says not enable the heavy job by default. The
tracked audit script `benchmarks/mklq/run_self_hosted_ci_audit.py` checks this
document and confirms the Apple Silicon workflow remains read-only,
source-only, free of pull-request triggers, and limited to the reviewed
main-branch broad push `Dispatch guard` path while the full self-hosted job
stays manual until a future activation PR changes the policy.
