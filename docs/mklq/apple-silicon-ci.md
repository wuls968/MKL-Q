# MKL-Q Apple Silicon CI

This page defines the readiness plan for a future self-hosted Apple Silicon
correctness gate. It is not enabled by default and is not release certification.

## Scope

The public `main` branch currently keeps GitHub Actions lightweight. The
enabled workflow checks source-only repository hygiene, public metadata, public
claim boundaries, benchmark summary parsing, and helper script syntax. It does
not build CUDA-Q and does not run Apple Silicon backend correctness tests.

The future Apple Silicon correctness job should cover local simulator behavior
for `mklq-cpu` and the experimental `mklq-metal` target on macOS ARM64. It must
remain source-only: no tags, no GitHub Releases, no wheels, no installers, and
no signed artifacts are produced by this job.

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

Do not enable a heavy Apple Silicon workflow until a reviewed activation PR
assigns a maintenance owner and confirms runner availability. Until then,
`.github/workflows/` must contain only the lightweight
`mklq-public-hygiene.yml` workflow.

The future workflow should use read-only repository permissions:

```yaml
permissions: contents: read
```

It should also set explicit `timeout-minutes` and `concurrency` values so a
stuck build cannot consume the runner indefinitely and superseded runs cancel
cleanly.

The job should not use secrets. If a future step needs credentials, split that
work into a separate reviewed release or deployment plan.

## Validation Command

The intended self-hosted correctness job should build from source and then run
the full local public healthcheck:

```bash
cmake -S . -B build-python -D CUDAQ_ENABLE_MKLQ_BACKEND=ON \
  -D CMAKE_INSTALL_PREFIX="${HOME}/.cudaq-mklq"
cmake --build build-python --target install -j 6
python3 benchmarks/mklq/run_public_healthcheck.py --full --require-clean
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

Before enabling the workflow:

- Confirm the runner labels include `self-hosted`, `macOS`, `ARM64`, and
  `mklq-apple-silicon`.
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
document and confirms the enabled workflow set remains lightweight until a
future activation PR changes the policy.
