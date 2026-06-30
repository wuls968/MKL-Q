# MKL-Q Public Readiness

This page records the public repository readiness snapshot for MKL-Q. It is a
source-only repository audit, not a release certification, package
certification, Apple Silicon CI replacement, or performance certification.

Snapshot date: 2026-06-30.

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
- [`metal-runtime-counters.md`](metal-runtime-counters.md)
- [`known-limitations.md`](known-limitations.md)
- [`roadmap.md`](roadmap.md)
- [`upstream-sync.md`](upstream-sync.md)
- [`release-policy.md`](release-policy.md)
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

- `.github/workflows/mklq-public-hygiene.yml` is the only tracked workflow.
- `.github/ISSUE_TEMPLATE/bug_report.yaml` and
  `.github/ISSUE_TEMPLATE/feature_request.yaml` are the only issue templates.
- `.github/pull_request_template.md` records compatibility, validation,
  benchmark evidence, and public hygiene checks.
- `.github/labels.yml` records the public triage label taxonomy.
- `.github/branch-protection-main.json` records the intended `main` protection
  API payload.

The lightweight workflow checks source-only repository hygiene, public metadata,
tracked benchmark summary parseability, bounded CPU sampling/probability counter
and Metal runtime counter probe parseability with complete expected counter-test
coverage, and benchmark helper syntax. It does not build CUDA-Q or run Apple
Silicon backend correctness tests.

The pushed-public readiness audit is handled by
`benchmarks/mklq/run_public_readiness_audit.py`. In addition to repository
identity and latest workflow status, it checks that the public issue template
set is intentional, every issue-template label is declared in
`.github/labels.yml`, live GitHub label metadata matches the tracked label
taxonomy, live `main` branch protection matches
`.github/branch-protection-main.json`, and the public claim-boundary guard
passes.

The pushed readiness audit was validated on 2026-06-30 after the protected
`main` branch completed the `MKL-Q public hygiene` workflow successfully. Use
the readiness commands below for the exact latest commit and workflow run IDs;
this tracked page records the stable source-only readiness boundary, not a
moving run log.

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
[`validation.md`](validation.md), with a correctness refresh on 2026-06-28 and
a public metadata refresh on 2026-06-30:

- latest correctness refresh date: 2026-06-28;
- source state: the ignored raw healthcheck JSON records the exact local Git
  state for the latest public healthcheck gate, and the ignored correctness
  gate JSON records the latest correctness-gate state;
- install-prefix build: passed;
- default public healthcheck: passed with 26/26 steps passed;
- full public healthcheck: passed with 29/29 steps passed;
- one-command correctness gate: passed with 4/4 steps passed, including
  `metal_runtime_counter_probe`;
- public example smoke gate: passed with 30/30 steps passed;
- current benchmark harness tests: `155 passed`;
- current `cpu_sampling_counter_probe_parse`: 2 bounded reports, 14 expected,
  14 selected, 0 missing, and 0 failures, including full-register and marginal
  probability-fill counter ctests;
- standalone install-prefix Python subset: `37 passed`;
- `python_target_smoke`: `61 passed`;
- `nvqpp_smoke`: `2 passed`;
- current full `target_config_ctest`: `89/89 passed`, including the
  hardware-efficient ansatz composite CPU fast-path counter fixture;
- current tracked `metal_runtime_counter_probe`: 2 bounded reports, 40
  expected, 40 selected, 0 missing, and 0 failures, including the direct
  resident three-target runtime fixture, resident built-in Rx/Ry/Rz,
  controlled-Rx/Ry/Rz, phase-family S/T/Sdg/Tdg, and multi-control
  single-qubit fixtures, plus the simulator resident three-target gate fixture
  and unsupported gate fallback/reupload boundary fixture.
- clean CPU benchmark gate: passed with 20 q20 `qpp-cpu`/`mklq-cpu` rows,
  including `cz-state`, `qft-like-state`, `seeded-clifford-state`, and
  `hardware-efficient-ansatz-state`, with 20 rows reporting `status == "ok"`
  against `23d34ab226c3e4d7a47f15af3292bf81ce25987b`.
- hardware-efficient ansatz CPU scaling evidence: passed with 6 q18/q20/q22
  `qpp-cpu`/`mklq-cpu` rows, with 6 rows reporting `status == "ok"` and local
  median elapsed ratios of `26.84x`, `52.94x`, and `81.37x` against
  `f2d87a4bf1e0d0163481a560df868292715a660a`.

This evidence is local Apple Silicon evidence. It is useful for source bootstrap
confidence, but it is not hosted CI, release certification, or cross-machine
performance certification.

The current public healthcheck also includes the static
`check_metal_evidence.py` guard for tracked `mklq-metal` summaries. That guard
checks local tuning provenance, ignored raw payload paths, successful Metal
rows, and wording that keeps the experimental mixed-path/host boundary clear.
It also includes `check_metal_runtime_counter_docs.py`, which fails if the
public Metal runtime counter summary drifts from the tracked bounded counter
reports. The preflight audit checks concrete `benchmarks/mklq/reports/*.json`
references in public docs and workflows, including the lightweight GitHub
workflow. The public readiness audit additionally runs the static
`check_public_claims.py` guard before accepting the pushed repository state.

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
- Public GitHub Actions currently run source hygiene only.
- Backend correctness still depends on local Apple Silicon validation.
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
