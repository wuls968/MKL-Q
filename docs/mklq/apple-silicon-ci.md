# MKL-Q Apple Silicon CI

This page defines the self-hosted Apple Silicon correctness gate. The expensive
self-hosted job is manual by default, does not run on pull requests, and is not
release certification. This page is not release certification.

## Scope

The public `main` branch keeps required GitHub Actions lightweight. The required
`MKL-Q repository checks` workflow checks package-aware repository hygiene,
public metadata, public claim boundaries, benchmark summary parsing, and helper
script syntax. It does not build CUDA-Q and does not run Apple Silicon backend
correctness tests.

The `.github/workflows/mklq-apple-silicon-ci.yml` workflow covers local
simulator behavior for `mklq-cpu` and the experimental `mklq-metal` target on a
private macOS ARM64 runner. The full self-hosted job is available only through
`workflow_dispatch`, defaults `run_full_gate` to `skip`, and runs the full gate
only when a maintainer explicitly starts it with `run_full_gate=confirm`.

It must remain correctness-only: no tags, no GitHub Releases, no wheels, no
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
- CMake, Ninja, a compiler toolchain, and CUDA-Q build prerequisites.
- For package-release jobs, native ARM64 interpreters at
  `/opt/homebrew/opt/python@3.11/bin/python3.11`,
  `/opt/homebrew/opt/python@3.12/bin/python3.12`,
  `/opt/homebrew/opt/python@3.13/bin/python3.13`, and
  `/opt/homebrew/opt/python@3.14/bin/python3.14`. The release workflow checks
  each interpreter's version, `platform.machine()`, and
  `sysconfig.get_platform()`; an x86_64/Rosetta interpreter is a hard failure.
- Enough local disk space for `build-python` and the install prefix.
- No persistent credentials, tokens, private keys, or `.env` files in the
  checkout, build directory, install prefix, or log output.

## Workflow Policy

The reviewed workflow set is:

```text
.github/workflows/mklq-public-hygiene.yml
.github/workflows/mklq-apple-silicon-ci.yml
.github/workflows/mklq-package-release.yml
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
of `actions/checkout`: workspace cleanup is limited to `${GITHUB_WORKSPACE}`.
For a manual run, it validates the current workflow_dispatch ref from
`GITHUB_REF_NAME` with `git check-ref-format --branch`, stores it as
`MKLQ_CHECKOUT_REF`, and checks out that branch with a read-only explicit
origin refspec. This lets a maintainer manually validate a PR branch without
adding an automatic `pull_request` trigger. The job then configures
`git sparse-checkout set` for the build, MKL-Q docs, benchmark harness,
`docs/sphinx/examples/mklq`, and `docs/sphinx/targets`.
When available, a runner-owned Git object cache at
`${RUNNER_TOOL_CACHE}/mklq-git-cache/mklq-origin.git` is attached through
`.git/objects/info/alternates` before the first fetch. When its local `main`
ref is available, the checkout creates `refs/mklq-cache/main` only inside the
ephemeral job repository so the later full-history fetch can negotiate against
the cached ancestry. The cache contains Git objects only, is optional, and
avoids repeatedly transferring already-known history; it does not replace the
origin or upstream remotes, create a public branch, or contain credentials.
If cache ref/object verification or local attachment fails, the job removes the
attempted local cache state and fetches normally from the remote.
The later remote-normalization step restores full-history evidence with retried
fetch commands using `http.version=HTTP/1.1`, `--unshallow`, and
`--filter=blob:none` for both `origin/main` and the checked-out dispatch ref,
plus the explicit `upstream/main` refspec. Before the public healthcheck, it
verifies that `HEAD` still equals `origin/${MKLQ_CHECKOUT_REF}` and that the
checkout is no longer shallow.
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
cache_root="${RUNNER_TOOL_CACHE:-${RUNNER_TEMP:?}}/mklq-git-cache"
origin_object_cache="${cache_root}/mklq-origin.git"
if [ -d "${origin_object_cache}/objects" ] && \
  cache_main="$(git -C "${origin_object_cache}" rev-parse \
    --verify refs/heads/main 2>/dev/null)" && \
  git -C "${origin_object_cache}" cat-file -e "${cache_main}^{commit}" 2>/dev/null && \
  printf '%s\n' "${origin_object_cache}/objects" > .git/objects/info/alternates
  git update-ref refs/mklq-cache/main "${cache_main}"; then
  echo "Using the runner Git object cache for full-history negotiation."
else
  rm -f .git/objects/info/alternates
  git update-ref -d refs/mklq-cache/main 2>/dev/null || true
  echo "Runner Git object cache is unavailable; fetching history normally."
fi
git remote add origin https://github.com/wuls968/MKL-Q.git
git remote add upstream https://github.com/NVIDIA/cuda-quantum.git
checkout_ref="${GITHUB_REF_NAME:?}"
git check-ref-format --branch "${checkout_ref}"
export MKLQ_CHECKOUT_REF="${checkout_ref}"
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
retry_git "origin ${checkout_ref} sparse fetch" \
  git -c http.version=HTTP/1.1 -c protocol.version=2 fetch \
  --no-tags --filter=blob:none --depth=1 \
  origin "+refs/heads/${checkout_ref}:refs/remotes/origin/${checkout_ref}"
retry_git "origin ${checkout_ref} sparse checkout" \
  git -c http.version=HTTP/1.1 -c protocol.version=2 checkout \
  --force -B "${checkout_ref}" "origin/${checkout_ref}"

git remote set-url origin https://github.com/wuls968/MKL-Q.git
git remote remove upstream 2>/dev/null || true
git remote add upstream https://github.com/NVIDIA/cuda-quantum.git
main_refspec="+refs/heads/main:refs/remotes/origin/main"
checkout_refspec="+refs/heads/${checkout_ref}:refs/remotes/origin/${checkout_ref}"
if [ "${checkout_ref}" = "main" ]; then
  origin_refspecs=("${main_refspec}")
else
  origin_refspecs=("${main_refspec}" "${checkout_refspec}")
fi
if [ "$(git rev-parse --is-shallow-repository)" = "true" ]; then
  retry_git "origin main and ${checkout_ref} unshallow" \
    git -c http.version=HTTP/1.1 -c protocol.version=2 fetch \
    --no-tags --filter=blob:none --unshallow \
    origin "${origin_refspecs[@]}"
else
  retry_git "origin main and ${checkout_ref} fetch" \
    git -c http.version=HTTP/1.1 -c protocol.version=2 fetch \
    --no-tags --filter=blob:none \
    origin "${origin_refspecs[@]}"
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
checkout with an explicit checkout timeout, validating the current
`workflow_dispatch` branch from `GITHUB_REF_NAME`, selecting the effective
Python, optionally attaching a runner-owned Git object cache from
`RUNNER_TOOL_CACHE`, normalizing `origin` and `upstream`, unshallowing both
`origin/main` and `MKLQ_CHECKOUT_REF` with `filter=blob:none`,
`http.version=HTTP/1.1`, and explicit origin refspecs, fetching `upstream/main`
with an explicit main refspec,
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
  out of the working tree, uses the optional Git object cache only through
  `.git/objects/info/alternates`, and restores full-history evidence later with
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
stays manual until a future activation PR changes the policy. Manual runs may
select a reviewed repository branch, including a PR branch, but retain no
secrets, read-only permissions, and no automatic pull-request trigger.
