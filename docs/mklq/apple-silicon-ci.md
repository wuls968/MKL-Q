# MKL-Q Apple Silicon CI

This page defines the manual self-hosted Apple Silicon correctness gate. It is
not enabled by default, does not run on pull requests, and is not release
certification. This page is not release certification.

## Scope

The public `main` branch keeps required GitHub Actions lightweight. The required
`MKL-Q public hygiene` workflow checks source-only repository hygiene, public
metadata, public claim boundaries, benchmark summary parsing, and helper script
syntax. It does not build CUDA-Q and does not run Apple Silicon backend
correctness tests.

The manual `.github/workflows/mklq-apple-silicon-ci.yml` workflow covers local
simulator behavior for `mklq-cpu` and the experimental `mklq-metal` target on a
private macOS ARM64 runner. It is available only through `workflow_dispatch`,
defaults `run_full_gate` to `skip`, and runs the full gate only when a
maintainer explicitly starts it with `run_full_gate=confirm`.

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

Do not add pull-request triggers or broad push triggers to the Apple Silicon
workflow until a reviewed activation PR assigns a maintenance owner, confirms
runner availability, and updates branch protection expectations. Until then it
remains a manual workflow with `workflow_dispatch`, `run_full_gate`, and default
skip activation. The only push trigger is limited to `main` changes of
`.github/workflows/mklq-apple-silicon-ci.yml`, where it runs the lightweight
`Dispatch guard` instead of the self-hosted correctness job.

The future workflow should use read-only repository permissions:

```yaml
permissions: contents: read
```

It also sets explicit `timeout-minutes` and `concurrency` values so a stuck
build cannot consume the runner indefinitely and superseded runs cancel cleanly.
If GitHub creates a non-dispatch validation run for the workflow file, only a
small unconditional `Dispatch guard` job runs on `ubuntu-latest`; the
self-hosted Apple Silicon job still requires `workflow_dispatch` with
`run_full_gate=confirm`.

The job should not use secrets. If a future step needs credentials, split that
work into a separate reviewed release or deployment plan.

## Validation Command

The manual self-hosted correctness job builds from source and then runs the full
local public healthcheck:

```bash
cmake -S . -B build-python -D CUDAQ_ENABLE_MKLQ_BACKEND=ON \
  -D CMAKE_INSTALL_PREFIX="${HOME}/.cudaq-mklq"
cmake --build build-python --target install -j 6
python3 benchmarks/mklq/run_public_healthcheck.py --full --require-clean
```

The tracked workflow invokes the same gate through:

```bash
python3 benchmarks/mklq/run_public_healthcheck.py \
  --full \
  --require-clean \
  --install-prefix "${MKLQ_INSTALL_PREFIX}" \
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

Before making the workflow automatic or branch-protected:

- Confirm the runner labels include `self-hosted`, `macOS`, `ARM64`, and
  `mklq-apple-silicon`.
- Confirm manual `workflow_dispatch` runs with `run_full_gate=confirm` complete
  successfully on the intended runner.
- Confirm the runner has no secrets, signing assets, or personal credentials in
  the working directory.
- Confirm the job uses `permissions: contents: read`.
- Confirm the job has explicit `timeout-minutes` and `concurrency` settings.
- Confirm the command is `run_public_healthcheck.py --full --require-clean` or
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

This plan deliberately says not enable the heavy workflow by default. The
tracked audit script `benchmarks/mklq/run_self_hosted_ci_audit.py` checks this
document and confirms the Apple Silicon workflow remains manual, read-only,
source-only, free of pull-request triggers, and limited to the reviewed
main-branch workflow-file `Dispatch guard` push path until a future activation
PR changes the policy.
