# MKL-Q Public Readiness

This page records the public repository readiness snapshot for MKL-Q. It is a
source-only repository audit, not a release certification, package
certification, Apple Silicon CI replacement, or performance certification.

Snapshot date: 2026-07-06.

## Scope

This readiness snapshot covers:

- public GitHub repository identity;
- upstream fork relationship;
- source-only artifact boundary;
- GitHub workflow and branch protection configuration;
- public documentation coverage;
- current local validation evidence;
- current caveats before any tag, package, or release artifact is created.

It does not certify:

- wheel, PyPI, Homebrew, installer, or binary artifact readiness;
- full CUDA-Q target parity;
- full Metal-native execution;
- cross-machine performance claims;
- hosted Apple Silicon correctness CI.

## Repository Identity

The intended public repository is `wuls968/MKL-Q`:

- URL: <https://github.com/wuls968/MKL-Q>
- Default branch: `main`
- Parent fork: `NVIDIA/cuda-quantum`
- License: Apache-2.0
- Description: `CUDA-Q-compatible Apple Silicon simulator fork with MKL-Q targets`
- Topics: `accelerate`, `apple-silicon`, `cuda-quantum`, `metal`, `mklq`,
  `quantum-computing`

MKL-Q keeps CUDA-Q public API compatibility for the first public source phase:
Python users still import `cudaq`, and C++ users still compile with `nvq++`.

## Git And Remotes

The current public branch model is:

- `origin` points to `https://github.com/wuls968/MKL-Q.git`.
- `upstream` points to `https://github.com/NVIDIA/cuda-quantum.git`.
- The repository is not shallow.
- The public branch keeps CUDA-Q upstream history plus MKL-Q commits on top.
- Sparse checkout may be used locally, but `.github` must be visible before
  public hygiene work.

Use:

```bash
git status --short --branch
git remote -v
git rev-parse --is-shallow-repository
git sparse-checkout list
git log --oneline --decorate -5
```

Expected result: `main` is clean before collecting clean evidence, the repo is
not shallow, and `origin/main` matches the intended public commit.

## Public Documentation

The public MKL-Q support boundary is documented in:

- [`architecture.md`](architecture.md)
- [`validation.md`](validation.md)
- [`testing-matrix.md`](testing-matrix.md)
- [`benchmark-evidence.md`](benchmark-evidence.md)
- [`cpu-gate-counters.md`](cpu-gate-counters.md)
- [`cpu-sampling-counters.md`](cpu-sampling-counters.md)
- [`metal-runtime-counters.md`](metal-runtime-counters.md)
- [`known-limitations.md`](known-limitations.md)
- [`roadmap.md`](roadmap.md)
- [`upstream-sync.md`](upstream-sync.md)
- [`release-policy.md`](release-policy.md)
- [`source-only-rc-v0.1.md`](source-only-rc-v0.1.md)
- [`release-notes-v0.1.0-source.md`](release-notes-v0.1.0-source.md)
- [`public-release-checklist.md`](public-release-checklist.md)
- [`developer-workflow.md`](developer-workflow.md)
- [`maintainer-runbook.md`](maintainer-runbook.md)
- [`issue-labels.md`](issue-labels.md)
- [`branch-protection.md`](branch-protection.md)

These documents intentionally describe the current source-only state. They must
not imply that MKL-Q publishes wheels, PyPI packages, installers, release tags,
or GitHub Releases.

## GitHub Configuration

The public GitHub configuration is intentionally lightweight:

- `.github/workflows/mklq-public-hygiene.yml` is the required lightweight
  workflow for pushes and pull requests.
- `.github/workflows/mklq-apple-silicon-ci.yml` is a manual
`workflow_dispatch` workflow for a private self-hosted Apple Silicon runner;
  it defaults `run_full_gate` to `skip`, has no pull-request trigger, and is
  not part of branch protection.
- `.github/ISSUE_TEMPLATE/bug_report.yaml` and
  `.github/ISSUE_TEMPLATE/feature_request.yaml` are the only issue templates.
- `.github/pull_request_template.md` records compatibility, validation,
  benchmark evidence, and public hygiene checks.
- `.github/labels.yml` records the public triage label taxonomy.
- `.github/branch-protection-main.json` records the intended `main` protection
  API payload.

The lightweight workflow checks source-only repository hygiene, public metadata,
tracked benchmark summary parseability, bounded CPU gate fast-path, CPU
sampling/probability, and Metal runtime counter probe parseability with
complete expected counter-test coverage, and benchmark helper syntax. It does
not build CUDA-Q or run Apple Silicon backend correctness tests.

The manual Apple Silicon workflow runs the full local public healthcheck only
when a maintainer explicitly dispatches it with `run_full_gate=confirm`. It is
source-only and must not create tags, GitHub Releases, wheels, installers, or
signed artifacts. Non-dispatch validation runs are limited to the lightweight
`Dispatch guard` job and do not consume a self-hosted Apple Silicon runner.

The pushed-public readiness audit is handled by
`benchmarks/mklq/run_public_readiness_audit.py`. In addition to repository
identity and latest workflow status, it checks that the public issue template
set is intentional, every issue-template label is declared in
`.github/labels.yml`, live GitHub label metadata matches the tracked label
taxonomy, live `main` branch protection matches
`.github/branch-protection-main.json`, the latest pushed commit has a
successful `MKL-Q public hygiene` run, the latest pushed commit has a
successful `MKL-Q Apple Silicon correctness` run, and the public
claim-boundary guard passes.

The documented source-only readiness baseline was refreshed on 2026-07-07 as a
historical evidence snapshot from protected `main` at
`2f0a72443dc8f8a01e801d6954d69ea2b063f83b`. The
`MKL-Q public hygiene` run
<https://github.com/wuls968/MKL-Q/actions/runs/28834453115> and the manual
`MKL-Q Apple Silicon correctness` full gate
<https://github.com/wuls968/MKL-Q/actions/runs/28834480337> both completed
successfully for that head. The ignored local readiness payload
`/tmp/mklq-public-readiness-audit-final-rc-v0.1-2026-07-06.json` recorded 13/13 checks
passed, and `/tmp/mklq-source-release-tag-post-refresh-baseline-2026-07-07.json`
recorded 9/9 source tag preflight checks passed, including repository identity,
issue templates, labels, branch protection, latest pushed workflow status,
public claim boundaries, the manual Apple Silicon full gate, and the
source-only no-tags/no-releases boundary. Use the readiness commands below for
the exact latest commit and workflow run IDs; this tracked page records the
stable source-only readiness boundary, not a moving run log.

## Branch Protection

The public `main` branch is intended to be protected with:

- required status check: `Source-only repository checks`;
- strict status checks enabled;
- force pushes disabled;
- branch deletion disabled;
- administrator enforcement enabled, so maintainers use the same required
  status check path as external contributors;
- no required pull-request review policy yet;
- no branch push restrictions yet.

The branch protection policy is documented in
[`branch-protection.md`](branch-protection.md), and the machine-readable API
payload is `.github/branch-protection-main.json`. The public readiness audit
compares the live core protection fields against that JSON reference.

## Validation Snapshot

The latest public local validation evidence is recorded in
[`validation.md`](validation.md), with the default public healthcheck refreshed
on 2026-07-05, the source-only readiness baseline refreshed on 2026-07-06, and
focused CRZ distance-sweep evidence retained from 2026-07-01:

- latest correctness refresh date: 2026-07-05;
- source state: the ignored raw healthcheck JSON records the exact local Git
  state for the latest public healthcheck gate, and the ignored correctness
  gate JSON records the latest correctness-gate state;
- install-prefix build: passed;
- default public healthcheck: passed with 32/32 steps passed;
- full public healthcheck: passed with 36/36 steps passed for the expanded
  source-only tag-audit gate, including the install-prefix build, local macOS
  signature repair for 71 loadables, correctness gate, benchmark harness tests,
  and public example smoke gate;
- source-only release-candidate dry run: passed the local full public
  healthcheck, public release checklist audit, preflight audit, and public
  readiness audit on 2026-07-05; the tracked source-only RC v0.1 entry point is
  [`source-only-rc-v0.1.md`](source-only-rc-v0.1.md);
- live self-hosted runner inventory check: refreshed on 2026-07-06 and
  reported one online runner named `mklq-apple-silicon-a0000` with the
  required `self-hosted`, `macOS`, `ARM64`, and `mklq-apple-silicon` labels;
- tracked source-only RC manual self-hosted Apple Silicon full gate: passed on
  2026-07-07 for `2f0a72443dc8f8a01e801d6954d69ea2b063f83b` in
  <https://github.com/wuls968/MKL-Q/actions/runs/28834480337>, with the full
  public healthcheck reporting 36/36 steps passed, the correctness gate ctest
  subset reporting 104/104 tests passed, the Metal runtime counter probe
  reporting 50/50 selected counter tests passed, the public example smoke gate
  reporting 30/30 checks passed, and the benchmark harness reporting 228
  passed;
- public readiness audit: passed with 13/13 checks passed against the
  then-current protected `main` branch;
- one-command correctness gate: passed with 4/4 steps passed, including
  `metal_runtime_counter_probe`;
- public example smoke gate: passed with 30/30 steps passed;
- current benchmark harness tests: `231 passed`;
- current `cpu_sampling_counter_probe_parse`: 3 bounded reports, 21 expected,
  21 selected, 0 missing, and 0 failures, including full-register and marginal
  probability-fill counter ctests;
- current `cpu_gate_counter_probe_parse`: 4 bounded reports, 53 expected, 53
  selected, 0 missing, and 0 failures, including single-control X/CNOT,
  per-gate single-control H/Y/Rx/Ry direct pair fixtures, single-control Rz
  direct phase fast-path fixture, generic two-qubit 4x4 block update,
  dedicated SWAP, and hardware-efficient ansatz composite fast-path fixtures;
- standalone install-prefix Python subset: `37 passed`;
- `python_target_smoke`: `70 passed`;
- `nvqpp_smoke`: `2 passed`;
- current build-tree `ctest` subset: `104/104 passed`, including the
  hardware-efficient ansatz composite CPU fast-path counter fixture and the
  resident full-register Metal sampling telemetry plus selected sample-count
  accumulation fixtures;
- current tracked `metal_runtime_counter_probe`: 13 bounded reports, 535
  expected, 535 selected, 0 missing, and 0 failures, including resident gate,
  probability/sampling, deterministic sampling bypass, direct and simulator
  coverage for selected full-register and partial-register counts-only Metal
  generated-draw/count accumulation, the uniform-probability generated-count
  fast path, remaining sequential host-side sampling telemetry, native sampling
  phase timing,
  measurement/reset, fallback, and error-boundary fixtures.
- clean CPU benchmark gate: passed with 32 q20 `qpp-cpu`/`mklq-cpu` rows,
  including `two-qubit-state`, `three-qubit-state`, `qft-like-state`,
  `seeded-clifford-state`, and `hardware-efficient-ansatz-state`, with 32 rows
  reporting `status == "ok"` against
  `dbebe3744f826ba4cbeed2b99708a2bdab03b11e`.
- CRZ distance-sweep CPU evidence: passed with 38 q20 `qpp-cpu`/`mklq-cpu`
  rows covering distances 1 through 19, with all rows reporting
  `status == "ok"` and a minimum local median elapsed ratio of `68.56x`
  against `a311c8749bbf5edfa553f64eb71a79faeafdd803`.
- hardware-efficient ansatz CPU scaling evidence: passed with 6 q18/q20/q22
  `qpp-cpu`/`mklq-cpu` rows, with 6 rows reporting `status == "ok"` and local
  median elapsed ratios of `26.84x`, `52.94x`, and `81.37x` against
  `f2d87a4bf1e0d0163481a560df868292715a660a`.
- two/three-qubit CPU scaling evidence: passed with 12 q18/q20/q22
  `qpp-cpu`/`mklq-cpu` rows for `two-qubit-state` and `three-qubit-state`, with
  12 rows reporting `status == "ok"` and local median elapsed ratios of
  `47.20x`, `131.99x`, `163.42x`, `24.54x`, `87.34x`, and `90.91x` against
  `cb688b20c825a970965ffe41ca84757287abf847`.
- Metal stochastic sampling boundary evidence: passed with tracked q20, q22,
  and q24
  `mklq-metal` full-register and partial-register counts-only rows at 256,
  1024, 8192, and 65536 shots. The latest q24 summary records 8 rows with
  `status == "ok"` against
  `b859075217a92c0a68b34605aaf9c78b0dfc1efa`; the high-shot versus low-shot
  median elapsed ratios were 0.984x for full-register sampling and 0.941x for
  partial-register sampling.

This evidence is local/self-hosted Apple Silicon evidence. It is useful for
source bootstrap confidence, but it is not GitHub-hosted CI, release
certification, or cross-machine performance certification.

The current public healthcheck also includes the static
`check_metal_evidence.py` guard for tracked `mklq-metal` summaries. That guard
checks local tuning provenance, ignored raw payload paths, successful Metal
rows, and wording that keeps the experimental mixed-path/host boundary clear.
It also includes `check_metal_sampling_boundary_evidence.py` for the tracked
q20/q22/q24 stochastic `mklq-metal` sampling summaries, requiring historical
host-side draw/count wording or selected Metal sample-count accumulation with
host-generated or device-generated draws, while rejecting broad device-side
sampler claims in those static summaries.
It also includes `check_metal_uniform_sampling_evidence.py` for the tracked
q20/q22/q24 uniform-probability partial-register sampling summary, requiring
12 measured qubits, 4096 marginal outcomes, the generated-count fast-path
label, and local-only/non-release wording.
It also includes `check_cpu_gate_counter_docs.py` and
`check_metal_runtime_counter_docs.py`, which fail if the public CPU gate
fast-path or Metal runtime counter summaries drift from the tracked bounded
counter reports. The preflight audit checks concrete
`benchmarks/mklq/reports/*.json` references in public docs and workflows,
including the lightweight GitHub workflow. The public readiness audit
additionally runs the static `check_public_claims.py` guard before accepting
the pushed repository state.

## No Tags Or Releases

The current public state is source-only:

- no public version tags;
- no GitHub Releases;
- no PyPI packages;
- no wheels;
- no installers;
- no Homebrew formula;
- no signed artifacts;
- no raw local benchmark payloads tracked.

Do not change this boundary without updating [`release-policy.md`](release-policy.md)
and running the release gates described there.

## Benchmark Evidence Boundary

Sanitized benchmark summaries may be tracked under `benchmarks/mklq/reports/`.
Raw local benchmark JSON under `benchmarks/mklq/results/` is intentionally
ignored.

Benchmark summaries must be interpreted through their `evidence_kind`,
`machine`, and `interpretation` fields. They are not cross-machine performance
certification.

## Current Caveats

- `mklq-cpu` is the stable local Apple Silicon target.
- `mklq-metal` is experimental and must not be described as default-ready.
- Public branch protection currently requires source hygiene only.
- Backend correctness is covered by local/self-hosted Apple Silicon validation,
  not by GitHub-hosted CI.
- The manual self-hosted Apple Silicon full gate has an online runner and a
  latest successful dispatch, but it remains manual, not branch-protected, and
  not release evidence.
- No package manager or binary artifact support is published.
- Upstream CUDA-Q syncs must follow [`upstream-sync.md`](upstream-sync.md).

## Readiness Commands

Use these commands for the public repository readiness audit:

```bash
python3 benchmarks/mklq/run_public_readiness_audit.py
git status --short --branch
git rev-parse --is-shallow-repository
git sparse-checkout list
git ls-files .github | sort
git ls-files docs/mklq | sort
git ls-remote --tags origin 'refs/tags/*'
gh repo view wuls968/MKL-Q \
  --json nameWithOwner,isFork,parent,defaultBranchRef,url,description,repositoryTopics,licenseInfo
gh api repos/wuls968/MKL-Q/branches/main --jq '{name,protected,commit:.commit.sha}'
gh run list --repo wuls968/MKL-Q --branch main --workflow 'MKL-Q public hygiene' --limit 1
gh run list --repo wuls968/MKL-Q --branch main --workflow 'MKL-Q Apple Silicon correctness' --limit 1
gh release list --repo wuls968/MKL-Q --limit 20
```

Expected result:

- the repository remains a fork of `NVIDIA/cuda-quantum`;
- `main` is the default branch;
- `main` is protected;
- the latest pushed commit has a successful `MKL-Q public hygiene` run;
- live issue labels match `.github/labels.yml`;
- live branch protection matches `.github/branch-protection-main.json`;
- the public claim-boundary guard passes;
- the latest pushed commit has a successful manual
  `MKL-Q Apple Silicon correctness` run before using self-hosted correctness
  evidence in public readiness claims;
- no release tags or GitHub Releases exist in the current source-only phase;
- only intentional MKL-Q public docs, issue templates, branch protection config,
  and the lightweight workflow are tracked.

## Stop Conditions

Do not describe the public repository as ready if any of these are true:

- the worktree is dirty before collecting clean evidence;
- `origin/main` does not match the intended public commit;
- the latest GitHub Actions result is failing, missing, or still pending;
- branch protection is missing, no longer requires `Source-only repository
  checks`, or no longer enforces administrators;
- raw benchmark JSON, build output, caches, `.DS_Store`, signing artifacts,
  tokens, secrets, or `docs/superpowers/` are tracked;
- release tags, GitHub Releases, wheels, installers, or package artifacts were
  created without a reviewed release plan;
- `mklq-metal` is presented as default-ready or full Metal-native without a
  separate readiness plan.
