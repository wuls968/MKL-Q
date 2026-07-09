# MKL-Q Benchmark Evidence

This file is generated from sanitized benchmark summaries under `benchmarks/mklq/reports/`.

Caveat: these entries are local benchmark evidence from development or release-prep runs. Interpret each entry through its `evidence_kind` and `interpretation` fields; none is a cross-machine performance certification.

## Evidence Inventory

| Summary ID | Kind | Machine | Targets | Cases | Qubits | Run shape | Rows | Raw evidence |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| local-clean-cpu-q20-2026-06-21 | clean_local_benchmark_evidence | Apple M5, 10 logical cores, 16 GiB RAM, macOS 26.5.1 | qpp-cpu, mklq-cpu | y-state, cy-state, cz-state, qft-like-state, seeded-clifford-state, sample-full-register, sample-partial-register | 20 | shot_counts=1024, 65536; repeats=2; warmups=1; layers=8; isolate_rows=true | ok=18 | benchmarks/mklq/results/local-clean-cpu-gate-y-cy-cz-q20-2026-06-21.json sha256=2b438094b63b; benchmarks/mklq/results/local-clean-cpu-composite-qft-like-seeded-clifford-q20-2026-06-21.json sha256=b07b3ba92b83; benchmarks/mklq/results/local-clean-cpu-sampling-q20-2026-06-21.json sha256=167b5c4adef8 |
| local-clean-cpu-q20-2026-06-28 | clean_local_benchmark_evidence | Apple M5, 10 logical cores, 16 GiB RAM, macOS 26.5.1 | qpp-cpu, mklq-cpu | y-state, cy-state, cz-state, qft-like-state, seeded-clifford-state, hardware-efficient-ansatz-state, sample-full-register, sample-partial-register | 20 | shot_counts=1024, 65536; repeats=2; warmups=1; layers=8; isolate_rows=true | ok=20 | benchmarks/mklq/results/local-clean-cpu-gate-y-cy-cz-q20-2026-06-28.json sha256=fd46266986bd; benchmarks/mklq/results/local-clean-cpu-composite-qft-like-seeded-clifford-hardware-efficient-ansatz-q20-2026-06-28.json sha256=3fa15408e149; benchmarks/mklq/results/local-clean-cpu-sampling-q20-2026-06-28.json sha256=c17703381afc |
| local-clean-cpu-q20-2026-06-30 | clean_local_benchmark_evidence | Apple M5, 10 logical cores, 16 GiB RAM, macOS 26.5.1 | qpp-cpu, mklq-cpu | y-state, cy-state, cz-state, qft-like-state, seeded-clifford-state, hardware-efficient-ansatz-state, sample-full-register, sample-partial-register | 20 | shot_counts=1024, 65536; repeats=2; warmups=1; layers=8; isolate_rows=true | ok=20 | benchmarks/mklq/results/local-clean-cpu-gate-y-cy-cz-q20-2026-06-30.json sha256=2cacb592d4e3; benchmarks/mklq/results/local-clean-cpu-composite-qft-like-seeded-clifford-hardware-efficient-ansatz-q20-2026-06-30.json sha256=b79f9b2ef836; benchmarks/mklq/results/local-clean-cpu-sampling-q20-2026-06-30.json sha256=219bb960f0e0 |
| local-clean-cpu-q20-2026-07-03 | clean_local_benchmark_evidence | Apple M5, 10 logical cores, 16 GiB RAM, macOS 26.5.1 | qpp-cpu, mklq-cpu | y-state, ch-state, cy-state, crx-state, cry-state, crz-state, cz-state, qft-like-state, seeded-clifford-state, hardware-efficient-ansatz-state, sample-full-register, sample-partial-register | 20 | shot_counts=1024, 65536; repeats=2; warmups=1; layers=8; isolate_rows=true | ok=28 | benchmarks/mklq/results/local-clean-cpu-gate-y-ch-cy-crx-cry-crz-cz-q20-2026-07-03.json sha256=d116aec76b8f; benchmarks/mklq/results/local-clean-cpu-composite-qft-like-seeded-clifford-hardware-efficient-ansatz-q20-2026-07-03.json sha256=88d0a2b69681; benchmarks/mklq/results/local-clean-cpu-sampling-q20-2026-07-03.json sha256=9e5f0b0a94ad |
| local-clean-cpu-q20-2026-07-03-two-three | clean_local_benchmark_evidence | Apple M5, 10 logical cores, 16 GiB RAM, macOS 26.5.1 | qpp-cpu, mklq-cpu | y-state, ch-state, cy-state, crx-state, cry-state, crz-state, cz-state, two-qubit-state, three-qubit-state, qft-like-state, seeded-clifford-state, hardware-efficient-ansatz-state, sample-full-register, sample-partial-register | 20 | shot_counts=1024, 65536; repeats=2; warmups=1; layers=8; isolate_rows=true | ok=32 | benchmarks/mklq/results/local-clean-cpu-gate-y-ch-cy-crx-cry-crz-cz-two-qubit-three-qubit-q20-2026-07-03-two-three.json sha256=e45243bbdaba; benchmarks/mklq/results/local-clean-cpu-composite-qft-like-seeded-clifford-hardware-efficient-ansatz-q20-2026-07-03-two-three.json sha256=d57ba2e9a520; benchmarks/mklq/results/local-clean-cpu-sampling-q20-2026-07-03-two-three.json sha256=7a66431362fd |
| local-clean-cpu-q20-2026-07-08-diagonal-phase | clean_local_benchmark_evidence | Apple M5, 10 logical cores, 16 GiB RAM, macOS 26.5.1 | qpp-cpu, mklq-cpu | y-state, diagonal-phase-state, ch-state, cy-state, crx-state, cry-state, crz-state, cz-state, two-qubit-state, custom-two-qubit-state, dense-two-qubit-state, controlled-dense-two-qubit-state, three-qubit-state, qft-like-state, seeded-clifford-state, hardware-efficient-ansatz-state, sample-full-register, sample-partial-register | 20 | shot_counts=1024, 65536; repeats=2; warmups=1; layers=8; isolate_rows=true | ok=40 | benchmarks/mklq/results/local-clean-cpu-gate-y-diagonal-phase-ch-cy-crx-cry-crz-cz-two-qubit-custom-two-qubit-dense-two-qubit-controlled-dense-two-qubit-three-qubit-q20-2026-07-08-diagonal-phase.json sha256=98d5bec556b3; benchmarks/mklq/results/local-clean-cpu-composite-qft-like-seeded-clifford-hardware-efficient-ansatz-q20-2026-07-08-diagonal-phase.json sha256=c26e751c3250; benchmarks/mklq/results/local-clean-cpu-sampling-q20-2026-07-08-diagonal-phase.json sha256=cc4878b63be9 |
| local-counts-only-sampling-shot-scaling-q20-2026-06-19 | local_tuning_evidence | Apple M5, 10 logical cores, 16 GiB RAM, macOS 26.5.1 | qpp-cpu, mklq-cpu, mklq-metal | sample-full-register, sample-partial-register | 20 | shot_counts=256, 1024, 8192, 65536; repeats=2; warmups=1; layers=8; isolate_rows=true | ok=24 | benchmarks/mklq/results/local-counts-only-sampling-shot-scaling-q20-2026-06-19.json sha256=ef9846673b46 |
| local-crz-distance-sweep-cpu-q20-2026-06-22 | clean_local_benchmark_evidence | Apple M5, 10 logical cores, 16 GiB RAM, macOS 26.5.1 | qpp-cpu, mklq-cpu | crz-distance-sweep-state | 20 | shot_counts=1024; repeats=2; warmups=1; layers=8; isolate_rows=true | ok=38 | benchmarks/mklq/results/local-clean-cpu-crz-distance-sweep-q20-2026-06-22.json sha256=64f9d0b4f709 |
| local-crz-distance-sweep-cpu-q20-2026-07-01 | clean_local_benchmark_evidence | Apple M5, 10 logical cores, 16 GiB RAM, macOS 26.5.1 | qpp-cpu, mklq-cpu | crz-distance-sweep-state | 20 | shot_counts=1024; repeats=2; warmups=1; layers=8; isolate_rows=true | ok=38 | benchmarks/mklq/results/local-clean-cpu-crz-distance-sweep-q20-2026-07-01.json sha256=e502854a8ca2 |
| local-current-sampling-fullprob-gated-q20-2026-06-19 | local_tuning_evidence | Apple M5, 10 logical cores, 16 GiB RAM, macOS 26.5.1 | qpp-cpu, mklq-cpu, mklq-metal | sample-full-register, sample-partial-register | 20 | shots=1024; repeats=2; warmups=1; layers=4; isolate_rows=true | ok=6 | benchmarks/mklq/results/local-current-sampling-fullprob-gated-q20-2026-06-19.json sha256=8ca6a4f7a7ae; benchmarks/mklq/results/local-current-sampling-shot-scaling-q20-2026-06-19.json sha256=9c15c0c1d566 |
| local-metal-composite-mixed-path-q20-2026-06-21 | local_tuning_evidence | Apple M5, 10 logical cores, 16 GiB RAM, macOS 26.5.1 | qpp-cpu, mklq-cpu, mklq-metal | qft-like-state, seeded-clifford-state | 20 | shot_counts=1024; repeats=2; warmups=1; layers=8; isolate_rows=true | ok=6 | benchmarks/mklq/results/local-metal-composite-mixed-path-q20-2026-06-21.json sha256=ef58b5922221 |
| local-metal-count-accumulation-sampling-q20-2026-07-04 | local_tuning_evidence | Apple M5, 10 logical cores, 16 GiB RAM, macOS 26.5.1 | mklq-metal | sample-full-register, sample-partial-register | 20 | shot_counts=256, 1024, 8192, 65536; repeats=2; warmups=1; layers=8; isolate_rows=true | ok=8 | benchmarks/mklq/results/local-metal-count-accumulation-sampling-q20-2026-07-04.json sha256=751c59993fc4 |
| local-metal-count-accumulation-sampling-q22-2026-07-04 | local_tuning_evidence | Apple M5, 10 logical cores, 16 GiB RAM, macOS 26.5.1 | mklq-metal | sample-full-register, sample-partial-register | 22 | shot_counts=256, 1024, 8192, 65536; repeats=2; warmups=1; layers=8; isolate_rows=true | ok=8 | benchmarks/mklq/results/local-metal-count-accumulation-sampling-q22-2026-07-04.json sha256=0a270eda1a97 |
| local-metal-diagonal-phase-q20-2026-07-08 | local_tuning_evidence | Apple M5, 10 logical cores, 16 GiB RAM, macOS 26.5.1 | qpp-cpu, mklq-cpu, mklq-metal | diagonal-phase-state | 20 | shot_counts=1024; repeats=2; warmups=1; layers=8; isolate_rows=true | ok=3 | benchmarks/mklq/results/local-metal-diagonal-phase-q20-2026-07-08.json sha256=c7b8fbbaafc9 |
| local-metal-partial-count-accumulation-sampling-q20-2026-07-05 | local_tuning_evidence | Apple M5, 10 logical cores, 16 GiB RAM, macOS 26.5.1 | mklq-metal | sample-full-register, sample-partial-register | 20 | shot_counts=256, 1024, 8192, 65536; repeats=2; warmups=1; layers=8; isolate_rows=true | ok=8 | benchmarks/mklq/results/local-metal-partial-count-accumulation-sampling-q20-2026-07-05.json sha256=e96c244a6ce3 |
| local-metal-partial-count-accumulation-sampling-q22-2026-07-05 | local_tuning_evidence | Apple M5, 10 logical cores, 16 GiB RAM, macOS 26.5.1 | mklq-metal | sample-full-register, sample-partial-register | 22 | shot_counts=256, 1024, 8192, 65536; repeats=2; warmups=1; layers=8; isolate_rows=true | ok=8 | benchmarks/mklq/results/local-metal-partial-count-accumulation-sampling-q22-2026-07-05.json sha256=ad5e8ca80911 |
| local-metal-partial-count-accumulation-sampling-q24-2026-07-05 | local_tuning_evidence | Apple M5, 10 logical cores, 16 GiB RAM, macOS 26.5.1 | mklq-metal | sample-full-register, sample-partial-register | 24 | shot_counts=256, 1024, 8192, 65536; repeats=2; warmups=1; layers=8; isolate_rows=true | ok=8 | benchmarks/mklq/results/local-metal-partial-count-accumulation-sampling-q24-2026-07-05.json sha256=be9062049a05 |
| local-metal-path-labels-q20-2026-06-22 | local_tuning_evidence | Apple M5, 10 logical cores, 16 GiB RAM, macOS 26.5.1 | mklq-metal | y-state, cy-state, qft-like-state, seeded-clifford-state, sample-full-register, sample-partial-register | 20 | shot_counts=1024, 65536; repeats=2; warmups=1; layers=8; isolate_rows=true | ok=8 | benchmarks/mklq/results/local-metal-path-labels-state-q20-2026-06-22.json sha256=5c44e7772c48; benchmarks/mklq/results/local-metal-path-labels-sampling-q20-2026-06-22.json sha256=0087e0be2ca9 |
| local-metal-sampling-boundary-q22-2026-07-04 | local_tuning_evidence | Apple M5, 10 logical cores, 16 GiB RAM, macOS 26.5.1 | mklq-metal | sample-full-register, sample-partial-register | 22 | shot_counts=256, 1024, 8192, 65536; repeats=2; warmups=1; layers=8; isolate_rows=true | ok=8 | benchmarks/mklq/results/local-metal-sampling-boundary-q22-2026-07-04.json sha256=c351ec6c2b3e |
| local-metal-three-qubit-resident-q20-2026-06-22 | local_tuning_evidence | Apple M5, 10 logical cores, 16 GiB RAM, macOS 26.5.1 | qpp-cpu, mklq-cpu, mklq-metal | three-qubit-state | 20 | shot_counts=1024; repeats=2; warmups=1; layers=8; isolate_rows=true | ok=3 | benchmarks/mklq/results/local-metal-three-qubit-resident-q20-2026-06-22.json sha256=daed4c1deb2d |
| local-metal-uniform-partial-sampling-q20-q24-2026-07-05 | local_tuning_evidence | Apple M5, 10 logical cores, 16 GiB RAM, macOS 26.5.1 | mklq-metal | sample-uniform-partial-register | 20, 22, 24 | shot_counts=256, 1024, 8192, 65536; repeats=2; warmups=1; layers=8; isolate_rows=true | ok=12 | benchmarks/mklq/results/local-metal-uniform-partial-sampling-q20-q24-2026-07-05.json sha256=a9505bdef581 |
| local-metal-y-cy-resident-isolated-q20-2026-06-19 | local_tuning_evidence | Apple M5, 10 logical cores, 16 GiB RAM, macOS 26.5.1 | qpp-cpu, mklq-cpu, mklq-metal | y-state, cy-state | 20 | shots=1024; repeats=2; warmups=1; layers=8; isolate_rows=true | ok=6 | benchmarks/mklq/results/local-metal-y-cy-resident-isolated-q20-2026-06-19.json sha256=84891e8f907c |
| local-multi-control-cpu-q20-2026-06-22 | clean_local_benchmark_evidence | Apple M5, 10 logical cores, 16 GiB RAM, macOS 26.5.1 | qpp-cpu, mklq-cpu | multi-control-state | 20 | shot_counts=1024; repeats=5; warmups=2; layers=8; isolate_rows=true | ok=2 | benchmarks/mklq/results/local-clean-cpu-multi-control-q20-2026-06-22.json sha256=6c483e023c90 |
| local-sampling-profile-breakdown-cpu-q20-q22-2026-06-23 | local_tuning_evidence | Apple M5, 10 logical cores, 16 GiB RAM, macOS 26.5.1 | mklq-cpu | sample-full-register, sample-partial-register | 20, 22 | shot_counts=1024, 65536; repeats=2; warmups=1; layers=8; isolate_rows=true; profile_sampling_breakdown=true | ok=8 | benchmarks/mklq/results/local-sampling-profile-breakdown-cpu-q20-q22-2026-06-23.json sha256=9821493c7e24 |
| local-sampling-scaling-cpu-q18-q22-2026-06-23 | clean_local_benchmark_evidence | Apple M5, 10 logical cores, 16 GiB RAM, macOS 26.5.1 | qpp-cpu, mklq-cpu | sample-full-register, sample-partial-register | 18, 20, 22 | shot_counts=1024, 65536; repeats=2; warmups=1; layers=8; isolate_rows=true | ok=24 | benchmarks/mklq/results/local-sampling-scaling-cpu-q18-q22-2026-06-23.json sha256=d0fc30326ee6 |
| local-scaling-cpu-controlled-dense-two-qubit-q18-q22-2026-07-08-controlled-dense-two-qubit-scaling | clean_local_benchmark_evidence | Apple M5, 10 logical cores, 16 GiB RAM, macOS 26.5.1 | qpp-cpu, mklq-cpu | controlled-dense-two-qubit-state | 18, 20, 22 | shot_counts=1024; repeats=3; warmups=1; layers=8; isolate_rows=true | ok=6 | benchmarks/mklq/results/local-scaling-cpu-controlled-dense-two-qubit-q18-q22-2026-07-08-controlled-dense-two-qubit-scaling.json sha256=5247b33453cc |
| local-scaling-cpu-controlled-swap-q18-q22-2026-07-09-controlled-swap-scaling | clean_local_benchmark_evidence | Apple M5, 10 logical cores, 16 GiB RAM, macOS 26.5.2 | qpp-cpu, mklq-cpu | controlled-swap-state | 18, 20, 22 | shot_counts=1024; repeats=3; warmups=1; layers=8; isolate_rows=true | ok=6 | benchmarks/mklq/results/local-scaling-cpu-controlled-swap-q18-q22-2026-07-09-controlled-swap-scaling.json sha256=a44c5599a9d0 |
| local-scaling-cpu-dense-two-qubit-q18-q22-2026-07-08-dense-two-qubit-scaling | clean_local_benchmark_evidence | Apple M5, 10 logical cores, 16 GiB RAM, macOS 26.5.1 | qpp-cpu, mklq-cpu | dense-two-qubit-state | 18, 20, 22 | shot_counts=1024; repeats=3; warmups=1; layers=8; isolate_rows=true | ok=6 | benchmarks/mklq/results/local-scaling-cpu-dense-two-qubit-q18-q22-2026-07-08-dense-two-qubit-scaling.json sha256=337e4dc6de41 |
| local-scaling-cpu-hardware-efficient-ansatz-q18-q22-2026-06-30 | clean_local_benchmark_evidence | Apple M5, 10 logical cores, 16 GiB RAM, macOS 26.5.1 | qpp-cpu, mklq-cpu | hardware-efficient-ansatz-state | 18, 20, 22 | shot_counts=1024; repeats=3; warmups=1; layers=8; isolate_rows=true | ok=6 | benchmarks/mklq/results/local-scaling-cpu-hardware-efficient-ansatz-q18-q22-2026-06-30.json sha256=26721c3b56f9 |
| local-scaling-cpu-multi-control-q18-q22-2026-06-22 | clean_local_benchmark_evidence | Apple M5, 10 logical cores, 16 GiB RAM, macOS 26.5.1 | qpp-cpu, mklq-cpu | multi-control-state | 18, 20, 22 | shot_counts=1024; repeats=3; warmups=1; layers=8; isolate_rows=true | ok=6 | benchmarks/mklq/results/local-scaling-cpu-multi-control-q18-q22-2026-06-22.json sha256=be97c9f00a75 |
| local-scaling-cpu-two-qubit-custom-two-qubit-three-qubit-q18-q22-2026-07-08-custom-two-qubit-scaling-main | clean_local_benchmark_evidence | Apple M5, 10 logical cores, 16 GiB RAM, macOS 26.5.1 | qpp-cpu, mklq-cpu | two-qubit-state, custom-two-qubit-state, three-qubit-state | 18, 20, 22 | shot_counts=1024; repeats=3; warmups=1; layers=8; isolate_rows=true | ok=18 | benchmarks/mklq/results/local-scaling-cpu-two-qubit-custom-two-qubit-three-qubit-q18-q22-2026-07-08-custom-two-qubit-scaling-main.json sha256=4eb1c5478b3b |
| local-scaling-cpu-two-qubit-custom-two-qubit-three-qubit-q18-q22-2026-07-08-row-sparse-two-qubit | clean_local_benchmark_evidence | Apple M5, 10 logical cores, 16 GiB RAM, macOS 26.5.1 | qpp-cpu, mklq-cpu | two-qubit-state, custom-two-qubit-state, three-qubit-state | 18, 20, 22 | shot_counts=1024; repeats=3; warmups=1; layers=8; isolate_rows=true | ok=18 | benchmarks/mklq/results/local-scaling-cpu-two-qubit-custom-two-qubit-three-qubit-q18-q22-2026-07-08-row-sparse-two-qubit.json sha256=f330ab779803 |
| local-scaling-cpu-two-qubit-three-qubit-q18-q22-2026-07-03-two-three-scaling | clean_local_benchmark_evidence | Apple M5, 10 logical cores, 16 GiB RAM, macOS 26.5.1 | qpp-cpu, mklq-cpu | two-qubit-state, three-qubit-state | 18, 20, 22 | shot_counts=1024; repeats=3; warmups=1; layers=8; isolate_rows=true | ok=12 | benchmarks/mklq/results/local-scaling-cpu-two-qubit-three-qubit-q18-q22-2026-07-03-two-three-scaling.json sha256=95dacd993ab7 |
| local-y-cy-fastpath-isolated-q20-2026-06-19 | local_tuning_evidence | Apple M5, 10 logical cores, 16 GiB RAM, macOS 26.5.1 | qpp-cpu, mklq-cpu | y-state, cy-state | 20 | shots=1024; repeats=2; warmups=1; layers=8; isolate_rows=true | ok=4 | benchmarks/mklq/results/local-y-cy-fastpath-isolated-q20-2026-06-19.json sha256=93bce3b77fcc |

## Comparison Signals

The values below are copied from each summary's bounded `comparison` object. Keep their original local-run context when citing them.

| Summary ID | Metric | Value |
| --- | --- | --- |
| local-clean-cpu-q20-2026-06-21 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_cy_state_q20` | 99.08x |
| local-clean-cpu-q20-2026-06-21 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_cz_state_q20` | 121.47x |
| local-clean-cpu-q20-2026-06-21 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_qft_like_state_q20` | 54.63x |
| local-clean-cpu-q20-2026-06-21 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_sample_full_register_q20_1024_shots` | 139.93x |
| local-clean-cpu-q20-2026-06-21 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_sample_full_register_q20_65536_shots` | 106.66x |
| local-clean-cpu-q20-2026-06-21 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_sample_partial_register_q20_1024_shots` | 166.86x |
| local-clean-cpu-q20-2026-06-21 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_sample_partial_register_q20_65536_shots` | 120.60x |
| local-clean-cpu-q20-2026-06-21 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_seeded_clifford_state_q20` | 97.56x |
| local-clean-cpu-q20-2026-06-21 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_y_state_q20` | 120.44x |
| local-clean-cpu-q20-2026-06-21 | `mklq_cpu_elapsed_seconds_median.cy_state_q20` | 0.0560914 s |
| local-clean-cpu-q20-2026-06-21 | `mklq_cpu_elapsed_seconds_median.cz_state_q20` | 0.0410443 s |
| local-clean-cpu-q20-2026-06-21 | `mklq_cpu_elapsed_seconds_median.qft_like_state_q20` | 1.33673 s |
| local-clean-cpu-q20-2026-06-21 | `mklq_cpu_elapsed_seconds_median.sample_full_register_q20_1024_shots` | 0.0128129 s |
| local-clean-cpu-q20-2026-06-21 | `mklq_cpu_elapsed_seconds_median.sample_full_register_q20_65536_shots` | 0.0157315 s |
| local-clean-cpu-q20-2026-06-21 | `mklq_cpu_elapsed_seconds_median.sample_partial_register_q20_1024_shots` | 0.010787 s |
| local-clean-cpu-q20-2026-06-21 | `mklq_cpu_elapsed_seconds_median.sample_partial_register_q20_65536_shots` | 0.0133672 s |
| local-clean-cpu-q20-2026-06-21 | `mklq_cpu_elapsed_seconds_median.seeded_clifford_state_q20` | 0.132161 s |
| local-clean-cpu-q20-2026-06-21 | `mklq_cpu_elapsed_seconds_median.y_state_q20` | 0.0446229 s |
| local-clean-cpu-q20-2026-06-28 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_cy_state_q20` | 110.99x |
| local-clean-cpu-q20-2026-06-28 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_cz_state_q20` | 96.82x |
| local-clean-cpu-q20-2026-06-28 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_hardware_efficient_ansatz_state_q20` | 87.95x |
| local-clean-cpu-q20-2026-06-28 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_qft_like_state_q20` | 55.74x |
| local-clean-cpu-q20-2026-06-28 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_sample_full_register_q20_1024_shots` | 81.41x |
| local-clean-cpu-q20-2026-06-28 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_sample_full_register_q20_65536_shots` | 84.25x |
| local-clean-cpu-q20-2026-06-28 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_sample_partial_register_q20_1024_shots` | 102.36x |
| local-clean-cpu-q20-2026-06-28 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_sample_partial_register_q20_65536_shots` | 90.39x |
| local-clean-cpu-q20-2026-06-28 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_seeded_clifford_state_q20` | 107.64x |
| local-clean-cpu-q20-2026-06-28 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_y_state_q20` | 86.24x |
| local-clean-cpu-q20-2026-06-28 | `mklq_cpu_elapsed_seconds_median.cy_state_q20` | 0.0672632 s |
| local-clean-cpu-q20-2026-06-28 | `mklq_cpu_elapsed_seconds_median.cz_state_q20` | 0.0564232 s |
| local-clean-cpu-q20-2026-06-28 | `mklq_cpu_elapsed_seconds_median.hardware_efficient_ansatz_state_q20` | 0.267873 s |
| local-clean-cpu-q20-2026-06-28 | `mklq_cpu_elapsed_seconds_median.qft_like_state_q20` | 0.988832 s |
| local-clean-cpu-q20-2026-06-28 | `mklq_cpu_elapsed_seconds_median.sample_full_register_q20_1024_shots` | 0.0154057 s |
| local-clean-cpu-q20-2026-06-28 | `mklq_cpu_elapsed_seconds_median.sample_full_register_q20_65536_shots` | 0.0150148 s |
| local-clean-cpu-q20-2026-06-28 | `mklq_cpu_elapsed_seconds_median.sample_partial_register_q20_1024_shots` | 0.0144241 s |
| local-clean-cpu-q20-2026-06-28 | `mklq_cpu_elapsed_seconds_median.sample_partial_register_q20_65536_shots` | 0.0174023 s |
| local-clean-cpu-q20-2026-06-28 | `mklq_cpu_elapsed_seconds_median.seeded_clifford_state_q20` | 0.0957006 s |
| local-clean-cpu-q20-2026-06-28 | `mklq_cpu_elapsed_seconds_median.y_state_q20` | 0.0661458 s |
| local-clean-cpu-q20-2026-06-30 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_cy_state_q20` | 116.93x |
| local-clean-cpu-q20-2026-06-30 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_cz_state_q20` | 136.95x |
| local-clean-cpu-q20-2026-06-30 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_hardware_efficient_ansatz_state_q20` | 100.99x |
| local-clean-cpu-q20-2026-06-30 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_qft_like_state_q20` | 80.09x |
| local-clean-cpu-q20-2026-06-30 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_sample_full_register_q20_1024_shots` | 93.85x |
| local-clean-cpu-q20-2026-06-30 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_sample_full_register_q20_65536_shots` | 122.15x |
| local-clean-cpu-q20-2026-06-30 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_sample_partial_register_q20_1024_shots` | 138.15x |
| local-clean-cpu-q20-2026-06-30 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_sample_partial_register_q20_65536_shots` | 126.38x |
| local-clean-cpu-q20-2026-06-30 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_seeded_clifford_state_q20` | 155.55x |
| local-clean-cpu-q20-2026-06-30 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_y_state_q20` | 123.52x |
| local-clean-cpu-q20-2026-06-30 | `mklq_cpu_elapsed_seconds_median.cy_state_q20` | 0.0454563 s |
| local-clean-cpu-q20-2026-06-30 | `mklq_cpu_elapsed_seconds_median.cz_state_q20` | 0.0361126 s |
| local-clean-cpu-q20-2026-06-30 | `mklq_cpu_elapsed_seconds_median.hardware_efficient_ansatz_state_q20` | 0.34967 s |
| local-clean-cpu-q20-2026-06-30 | `mklq_cpu_elapsed_seconds_median.qft_like_state_q20` | 1.06543 s |
| local-clean-cpu-q20-2026-06-30 | `mklq_cpu_elapsed_seconds_median.sample_full_register_q20_1024_shots` | 0.0156096 s |
| local-clean-cpu-q20-2026-06-30 | `mklq_cpu_elapsed_seconds_median.sample_full_register_q20_65536_shots` | 0.0122021 s |
| local-clean-cpu-q20-2026-06-30 | `mklq_cpu_elapsed_seconds_median.sample_partial_register_q20_1024_shots` | 0.0102048 s |
| local-clean-cpu-q20-2026-06-30 | `mklq_cpu_elapsed_seconds_median.sample_partial_register_q20_65536_shots` | 0.0110367 s |
| local-clean-cpu-q20-2026-06-30 | `mklq_cpu_elapsed_seconds_median.seeded_clifford_state_q20` | 0.119174 s |
| local-clean-cpu-q20-2026-06-30 | `mklq_cpu_elapsed_seconds_median.y_state_q20` | 0.0425469 s |
| local-clean-cpu-q20-2026-07-03 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_ch_state_q20` | 36.07x |
| local-clean-cpu-q20-2026-07-03 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_crx_state_q20` | 76.43x |
| local-clean-cpu-q20-2026-07-03 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_cry_state_q20` | 78.64x |
| local-clean-cpu-q20-2026-07-03 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_crz_state_q20` | 96.49x |
| local-clean-cpu-q20-2026-07-03 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_cy_state_q20` | 122.20x |
| local-clean-cpu-q20-2026-07-03 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_cz_state_q20` | 183.33x |
| local-clean-cpu-q20-2026-07-03 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_hardware_efficient_ansatz_state_q20` | 77.80x |
| local-clean-cpu-q20-2026-07-03 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_qft_like_state_q20` | 54.08x |
| local-clean-cpu-q20-2026-07-03 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_sample_full_register_q20_1024_shots` | 81.62x |
| local-clean-cpu-q20-2026-07-03 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_sample_full_register_q20_65536_shots` | 97.38x |
| local-clean-cpu-q20-2026-07-03 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_sample_partial_register_q20_1024_shots` | 85.98x |
| local-clean-cpu-q20-2026-07-03 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_sample_partial_register_q20_65536_shots` | 124.96x |
| local-clean-cpu-q20-2026-07-03 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_seeded_clifford_state_q20` | 116.73x |
| local-clean-cpu-q20-2026-07-03 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_y_state_q20` | 44.90x |
| local-clean-cpu-q20-2026-07-03 | `mklq_cpu_elapsed_seconds_median.ch_state_q20` | 0.245537 s |
| local-clean-cpu-q20-2026-07-03 | `mklq_cpu_elapsed_seconds_median.crx_state_q20` | 0.116326 s |
| local-clean-cpu-q20-2026-07-03 | `mklq_cpu_elapsed_seconds_median.cry_state_q20` | 0.167777 s |
| local-clean-cpu-q20-2026-07-03 | `mklq_cpu_elapsed_seconds_median.crz_state_q20` | 0.121356 s |
| local-clean-cpu-q20-2026-07-03 | `mklq_cpu_elapsed_seconds_median.cy_state_q20` | 0.0848989 s |
| local-clean-cpu-q20-2026-07-03 | `mklq_cpu_elapsed_seconds_median.cz_state_q20` | 0.058331 s |
| local-clean-cpu-q20-2026-07-03 | `mklq_cpu_elapsed_seconds_median.hardware_efficient_ansatz_state_q20` | 0.414149 s |
| local-clean-cpu-q20-2026-07-03 | `mklq_cpu_elapsed_seconds_median.qft_like_state_q20` | 1.45527 s |
| local-clean-cpu-q20-2026-07-03 | `mklq_cpu_elapsed_seconds_median.sample_full_register_q20_1024_shots` | 0.0213506 s |
| local-clean-cpu-q20-2026-07-03 | `mklq_cpu_elapsed_seconds_median.sample_full_register_q20_65536_shots` | 0.0269914 s |
| local-clean-cpu-q20-2026-07-03 | `mklq_cpu_elapsed_seconds_median.sample_partial_register_q20_1024_shots` | 0.0246332 s |
| local-clean-cpu-q20-2026-07-03 | `mklq_cpu_elapsed_seconds_median.sample_partial_register_q20_65536_shots` | 0.0198829 s |
| local-clean-cpu-q20-2026-07-03 | `mklq_cpu_elapsed_seconds_median.seeded_clifford_state_q20` | 0.117576 s |
| local-clean-cpu-q20-2026-07-03 | `mklq_cpu_elapsed_seconds_median.y_state_q20` | 0.236777 s |
| local-clean-cpu-q20-2026-07-03-two-three | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_ch_state_q20` | 34.21x |
| local-clean-cpu-q20-2026-07-03-two-three | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_crx_state_q20` | 23.71x |
| local-clean-cpu-q20-2026-07-03-two-three | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_cry_state_q20` | 50.17x |
| local-clean-cpu-q20-2026-07-03-two-three | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_crz_state_q20` | 49.13x |
| local-clean-cpu-q20-2026-07-03-two-three | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_cy_state_q20` | 28.18x |
| local-clean-cpu-q20-2026-07-03-two-three | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_cz_state_q20` | 59.77x |
| local-clean-cpu-q20-2026-07-03-two-three | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_hardware_efficient_ansatz_state_q20` | 80.59x |
| local-clean-cpu-q20-2026-07-03-two-three | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_qft_like_state_q20` | 78.77x |
| local-clean-cpu-q20-2026-07-03-two-three | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_sample_full_register_q20_1024_shots` | 76.65x |
| local-clean-cpu-q20-2026-07-03-two-three | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_sample_full_register_q20_65536_shots` | 126.82x |
| local-clean-cpu-q20-2026-07-03-two-three | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_sample_partial_register_q20_1024_shots` | 90.40x |
| local-clean-cpu-q20-2026-07-03-two-three | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_sample_partial_register_q20_65536_shots` | 61.80x |
| local-clean-cpu-q20-2026-07-03-two-three | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_seeded_clifford_state_q20` | 98.46x |
| local-clean-cpu-q20-2026-07-03-two-three | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_three_qubit_state_q20` | 41.92x |
| local-clean-cpu-q20-2026-07-03-two-three | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_two_qubit_state_q20` | 56.82x |
| local-clean-cpu-q20-2026-07-03-two-three | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_y_state_q20` | 25.42x |
| local-clean-cpu-q20-2026-07-03-two-three | `mklq_cpu_elapsed_seconds_median.ch_state_q20` | 0.464325 s |
| local-clean-cpu-q20-2026-07-03-two-three | `mklq_cpu_elapsed_seconds_median.crx_state_q20` | 0.660293 s |
| local-clean-cpu-q20-2026-07-03-two-three | `mklq_cpu_elapsed_seconds_median.cry_state_q20` | 0.311901 s |
| local-clean-cpu-q20-2026-07-03-two-three | `mklq_cpu_elapsed_seconds_median.crz_state_q20` | 0.331593 s |
| local-clean-cpu-q20-2026-07-03-two-three | `mklq_cpu_elapsed_seconds_median.cy_state_q20` | 0.553516 s |
| local-clean-cpu-q20-2026-07-03-two-three | `mklq_cpu_elapsed_seconds_median.cz_state_q20` | 0.235804 s |
| local-clean-cpu-q20-2026-07-03-two-three | `mklq_cpu_elapsed_seconds_median.hardware_efficient_ansatz_state_q20` | 0.470984 s |
| local-clean-cpu-q20-2026-07-03-two-three | `mklq_cpu_elapsed_seconds_median.qft_like_state_q20` | 1.25039 s |
| local-clean-cpu-q20-2026-07-03-two-three | `mklq_cpu_elapsed_seconds_median.sample_full_register_q20_1024_shots` | 0.0242865 s |
| local-clean-cpu-q20-2026-07-03-two-three | `mklq_cpu_elapsed_seconds_median.sample_full_register_q20_65536_shots` | 0.0192056 s |
| local-clean-cpu-q20-2026-07-03-two-three | `mklq_cpu_elapsed_seconds_median.sample_partial_register_q20_1024_shots` | 0.0209785 s |
| local-clean-cpu-q20-2026-07-03-two-three | `mklq_cpu_elapsed_seconds_median.sample_partial_register_q20_65536_shots` | 0.0408418 s |
| local-clean-cpu-q20-2026-07-03-two-three | `mklq_cpu_elapsed_seconds_median.seeded_clifford_state_q20` | 0.163067 s |
| local-clean-cpu-q20-2026-07-03-two-three | `mklq_cpu_elapsed_seconds_median.three_qubit_state_q20` | 0.70659 s |
| local-clean-cpu-q20-2026-07-03-two-three | `mklq_cpu_elapsed_seconds_median.two_qubit_state_q20` | 0.350092 s |
| local-clean-cpu-q20-2026-07-03-two-three | `mklq_cpu_elapsed_seconds_median.y_state_q20` | 0.772671 s |
| local-clean-cpu-q20-2026-07-08-diagonal-phase | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_ch_state_q20` | 119.76x |
| local-clean-cpu-q20-2026-07-08-diagonal-phase | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_controlled_dense_two_qubit_state_q20` | 47.87x |
| local-clean-cpu-q20-2026-07-08-diagonal-phase | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_crx_state_q20` | 130.41x |
| local-clean-cpu-q20-2026-07-08-diagonal-phase | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_cry_state_q20` | 112.74x |
| local-clean-cpu-q20-2026-07-08-diagonal-phase | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_crz_state_q20` | 80.03x |
| local-clean-cpu-q20-2026-07-08-diagonal-phase | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_custom_two_qubit_state_q20` | 73.77x |
| local-clean-cpu-q20-2026-07-08-diagonal-phase | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_cy_state_q20` | 160.39x |
| local-clean-cpu-q20-2026-07-08-diagonal-phase | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_cz_state_q20` | 140.55x |
| local-clean-cpu-q20-2026-07-08-diagonal-phase | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_dense_two_qubit_state_q20` | 37.20x |
| local-clean-cpu-q20-2026-07-08-diagonal-phase | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_diagonal_phase_state_q20` | 90.93x |
| local-clean-cpu-q20-2026-07-08-diagonal-phase | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_hardware_efficient_ansatz_state_q20` | 78.16x |
| local-clean-cpu-q20-2026-07-08-diagonal-phase | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_qft_like_state_q20` | 68.04x |
| local-clean-cpu-q20-2026-07-08-diagonal-phase | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_sample_full_register_q20_1024_shots` | 108.49x |
| local-clean-cpu-q20-2026-07-08-diagonal-phase | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_sample_full_register_q20_65536_shots` | 90.11x |
| local-clean-cpu-q20-2026-07-08-diagonal-phase | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_sample_partial_register_q20_1024_shots` | 116.52x |
| local-clean-cpu-q20-2026-07-08-diagonal-phase | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_sample_partial_register_q20_65536_shots` | 96.67x |
| local-clean-cpu-q20-2026-07-08-diagonal-phase | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_seeded_clifford_state_q20` | 181.62x |
| local-clean-cpu-q20-2026-07-08-diagonal-phase | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_three_qubit_state_q20` | 65.28x |
| local-clean-cpu-q20-2026-07-08-diagonal-phase | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_two_qubit_state_q20` | 197.40x |
| local-clean-cpu-q20-2026-07-08-diagonal-phase | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_y_state_q20` | 112.12x |
| local-clean-cpu-q20-2026-07-08-diagonal-phase | `mklq_cpu_elapsed_seconds_median.ch_state_q20` | 0.0701289 s |
| local-clean-cpu-q20-2026-07-08-diagonal-phase | `mklq_cpu_elapsed_seconds_median.controlled_dense_two_qubit_state_q20` | 0.199748 s |
| local-clean-cpu-q20-2026-07-08-diagonal-phase | `mklq_cpu_elapsed_seconds_median.crx_state_q20` | 0.070338 s |
| local-clean-cpu-q20-2026-07-08-diagonal-phase | `mklq_cpu_elapsed_seconds_median.cry_state_q20` | 0.0753376 s |
| local-clean-cpu-q20-2026-07-08-diagonal-phase | `mklq_cpu_elapsed_seconds_median.crz_state_q20` | 0.103228 s |
| local-clean-cpu-q20-2026-07-08-diagonal-phase | `mklq_cpu_elapsed_seconds_median.custom_two_qubit_state_q20` | 0.139011 s |
| local-clean-cpu-q20-2026-07-08-diagonal-phase | `mklq_cpu_elapsed_seconds_median.cy_state_q20` | 0.0609493 s |
| local-clean-cpu-q20-2026-07-08-diagonal-phase | `mklq_cpu_elapsed_seconds_median.cz_state_q20` | 0.0523486 s |
| local-clean-cpu-q20-2026-07-08-diagonal-phase | `mklq_cpu_elapsed_seconds_median.dense_two_qubit_state_q20` | 0.274282 s |
| local-clean-cpu-q20-2026-07-08-diagonal-phase | `mklq_cpu_elapsed_seconds_median.diagonal_phase_state_q20` | 0.0868624 s |
| local-clean-cpu-q20-2026-07-08-diagonal-phase | `mklq_cpu_elapsed_seconds_median.hardware_efficient_ansatz_state_q20` | 0.502892 s |
| local-clean-cpu-q20-2026-07-08-diagonal-phase | `mklq_cpu_elapsed_seconds_median.qft_like_state_q20` | 1.52084 s |
| local-clean-cpu-q20-2026-07-08-diagonal-phase | `mklq_cpu_elapsed_seconds_median.sample_full_register_q20_1024_shots` | 0.0170533 s |
| local-clean-cpu-q20-2026-07-08-diagonal-phase | `mklq_cpu_elapsed_seconds_median.sample_full_register_q20_65536_shots` | 0.0201992 s |
| local-clean-cpu-q20-2026-07-08-diagonal-phase | `mklq_cpu_elapsed_seconds_median.sample_partial_register_q20_1024_shots` | 0.0199689 s |
| local-clean-cpu-q20-2026-07-08-diagonal-phase | `mklq_cpu_elapsed_seconds_median.sample_partial_register_q20_65536_shots` | 0.0239938 s |
| local-clean-cpu-q20-2026-07-08-diagonal-phase | `mklq_cpu_elapsed_seconds_median.seeded_clifford_state_q20` | 0.115497 s |
| local-clean-cpu-q20-2026-07-08-diagonal-phase | `mklq_cpu_elapsed_seconds_median.three_qubit_state_q20` | 0.227342 s |
| local-clean-cpu-q20-2026-07-08-diagonal-phase | `mklq_cpu_elapsed_seconds_median.two_qubit_state_q20` | 0.05031 s |
| local-clean-cpu-q20-2026-07-08-diagonal-phase | `mklq_cpu_elapsed_seconds_median.y_state_q20` | 0.0562494 s |
| local-counts-only-sampling-shot-scaling-q20-2026-06-19 | `q20_65536_shot_elapsed_ratio_qpp_cpu_over_mklq_cpu.sample_full_register` | 58.54x |
| local-counts-only-sampling-shot-scaling-q20-2026-06-19 | `q20_65536_shot_elapsed_ratio_qpp_cpu_over_mklq_cpu.sample_partial_register` | 83.64x |
| local-counts-only-sampling-shot-scaling-q20-2026-06-19 | `q20_65536_shot_elapsed_ratio_qpp_cpu_over_mklq_metal.sample_full_register` | 27.94x |
| local-counts-only-sampling-shot-scaling-q20-2026-06-19 | `q20_65536_shot_elapsed_ratio_qpp_cpu_over_mklq_metal.sample_partial_register` | 38.01x |
| local-crz-distance-sweep-cpu-q20-2026-06-22 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_crz_distance_sweep_state_q20_distance_1` | 65.93x |
| local-crz-distance-sweep-cpu-q20-2026-06-22 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_crz_distance_sweep_state_q20_distance_2` | 65.23x |
| local-crz-distance-sweep-cpu-q20-2026-06-22 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_crz_distance_sweep_state_q20_distance_3` | 65.65x |
| local-crz-distance-sweep-cpu-q20-2026-06-22 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_crz_distance_sweep_state_q20_distance_4` | 65.60x |
| local-crz-distance-sweep-cpu-q20-2026-06-22 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_crz_distance_sweep_state_q20_distance_5` | 66.49x |
| local-crz-distance-sweep-cpu-q20-2026-06-22 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_crz_distance_sweep_state_q20_distance_6` | 68.96x |
| local-crz-distance-sweep-cpu-q20-2026-06-22 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_crz_distance_sweep_state_q20_distance_7` | 67.14x |
| local-crz-distance-sweep-cpu-q20-2026-06-22 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_crz_distance_sweep_state_q20_distance_8` | 69.03x |
| local-crz-distance-sweep-cpu-q20-2026-06-22 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_crz_distance_sweep_state_q20_distance_9` | 70.04x |
| local-crz-distance-sweep-cpu-q20-2026-06-22 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_crz_distance_sweep_state_q20_distance_10` | 71.75x |
| local-crz-distance-sweep-cpu-q20-2026-06-22 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_crz_distance_sweep_state_q20_distance_11` | 76.41x |
| local-crz-distance-sweep-cpu-q20-2026-06-22 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_crz_distance_sweep_state_q20_distance_12` | 69.08x |
| local-crz-distance-sweep-cpu-q20-2026-06-22 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_crz_distance_sweep_state_q20_distance_13` | 78.59x |
| local-crz-distance-sweep-cpu-q20-2026-06-22 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_crz_distance_sweep_state_q20_distance_14` | 77.75x |
| local-crz-distance-sweep-cpu-q20-2026-06-22 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_crz_distance_sweep_state_q20_distance_15` | 77.30x |
| local-crz-distance-sweep-cpu-q20-2026-06-22 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_crz_distance_sweep_state_q20_distance_16` | 78.32x |
| local-crz-distance-sweep-cpu-q20-2026-06-22 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_crz_distance_sweep_state_q20_distance_17` | 71.83x |
| local-crz-distance-sweep-cpu-q20-2026-06-22 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_crz_distance_sweep_state_q20_distance_18` | 73.04x |
| local-crz-distance-sweep-cpu-q20-2026-06-22 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_crz_distance_sweep_state_q20_distance_19` | 68.00x |
| local-crz-distance-sweep-cpu-q20-2026-06-22 | `mklq_cpu_elapsed_seconds_median.crz_distance_sweep_state_q20_distance_1` | 0.0742748 s |
| local-crz-distance-sweep-cpu-q20-2026-06-22 | `mklq_cpu_elapsed_seconds_median.crz_distance_sweep_state_q20_distance_2` | 0.0706062 s |
| local-crz-distance-sweep-cpu-q20-2026-06-22 | `mklq_cpu_elapsed_seconds_median.crz_distance_sweep_state_q20_distance_3` | 0.0697185 s |
| local-crz-distance-sweep-cpu-q20-2026-06-22 | `mklq_cpu_elapsed_seconds_median.crz_distance_sweep_state_q20_distance_4` | 0.0656998 s |
| local-crz-distance-sweep-cpu-q20-2026-06-22 | `mklq_cpu_elapsed_seconds_median.crz_distance_sweep_state_q20_distance_5` | 0.0631273 s |
| local-crz-distance-sweep-cpu-q20-2026-06-22 | `mklq_cpu_elapsed_seconds_median.crz_distance_sweep_state_q20_distance_6` | 0.0600961 s |
| local-crz-distance-sweep-cpu-q20-2026-06-22 | `mklq_cpu_elapsed_seconds_median.crz_distance_sweep_state_q20_distance_7` | 0.0566415 s |
| local-crz-distance-sweep-cpu-q20-2026-06-22 | `mklq_cpu_elapsed_seconds_median.crz_distance_sweep_state_q20_distance_8` | 0.053566 s |
| local-crz-distance-sweep-cpu-q20-2026-06-22 | `mklq_cpu_elapsed_seconds_median.crz_distance_sweep_state_q20_distance_9` | 0.0509577 s |
| local-crz-distance-sweep-cpu-q20-2026-06-22 | `mklq_cpu_elapsed_seconds_median.crz_distance_sweep_state_q20_distance_10` | 0.0468659 s |
| local-crz-distance-sweep-cpu-q20-2026-06-22 | `mklq_cpu_elapsed_seconds_median.crz_distance_sweep_state_q20_distance_11` | 0.0438885 s |
| local-crz-distance-sweep-cpu-q20-2026-06-22 | `mklq_cpu_elapsed_seconds_median.crz_distance_sweep_state_q20_distance_12` | 0.0448066 s |
| local-crz-distance-sweep-cpu-q20-2026-06-22 | `mklq_cpu_elapsed_seconds_median.crz_distance_sweep_state_q20_distance_13` | 0.0387231 s |
| local-crz-distance-sweep-cpu-q20-2026-06-22 | `mklq_cpu_elapsed_seconds_median.crz_distance_sweep_state_q20_distance_14` | 0.0357577 s |
| local-crz-distance-sweep-cpu-q20-2026-06-22 | `mklq_cpu_elapsed_seconds_median.crz_distance_sweep_state_q20_distance_15` | 0.0334365 s |
| local-crz-distance-sweep-cpu-q20-2026-06-22 | `mklq_cpu_elapsed_seconds_median.crz_distance_sweep_state_q20_distance_16` | 0.0308791 s |
| local-crz-distance-sweep-cpu-q20-2026-06-22 | `mklq_cpu_elapsed_seconds_median.crz_distance_sweep_state_q20_distance_17` | 0.0294991 s |
| local-crz-distance-sweep-cpu-q20-2026-06-22 | `mklq_cpu_elapsed_seconds_median.crz_distance_sweep_state_q20_distance_18` | 0.0246605 s |
| local-crz-distance-sweep-cpu-q20-2026-06-22 | `mklq_cpu_elapsed_seconds_median.crz_distance_sweep_state_q20_distance_19` | 0.0223731 s |
| local-crz-distance-sweep-cpu-q20-2026-07-01 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_crz_distance_sweep_state_q20_distance_1` | 76.31x |
| local-crz-distance-sweep-cpu-q20-2026-07-01 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_crz_distance_sweep_state_q20_distance_2` | 86.92x |
| local-crz-distance-sweep-cpu-q20-2026-07-01 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_crz_distance_sweep_state_q20_distance_3` | 78.95x |
| local-crz-distance-sweep-cpu-q20-2026-07-01 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_crz_distance_sweep_state_q20_distance_4` | 69.82x |
| local-crz-distance-sweep-cpu-q20-2026-07-01 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_crz_distance_sweep_state_q20_distance_5` | 82.12x |
| local-crz-distance-sweep-cpu-q20-2026-07-01 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_crz_distance_sweep_state_q20_distance_6` | 82.16x |
| local-crz-distance-sweep-cpu-q20-2026-07-01 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_crz_distance_sweep_state_q20_distance_7` | 97.41x |
| local-crz-distance-sweep-cpu-q20-2026-07-01 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_crz_distance_sweep_state_q20_distance_8` | 81.56x |
| local-crz-distance-sweep-cpu-q20-2026-07-01 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_crz_distance_sweep_state_q20_distance_9` | 168.76x |
| local-crz-distance-sweep-cpu-q20-2026-07-01 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_crz_distance_sweep_state_q20_distance_10` | 79.23x |
| local-crz-distance-sweep-cpu-q20-2026-07-01 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_crz_distance_sweep_state_q20_distance_11` | 105.45x |
| local-crz-distance-sweep-cpu-q20-2026-07-01 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_crz_distance_sweep_state_q20_distance_12` | 110.93x |
| local-crz-distance-sweep-cpu-q20-2026-07-01 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_crz_distance_sweep_state_q20_distance_13` | 124.85x |
| local-crz-distance-sweep-cpu-q20-2026-07-01 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_crz_distance_sweep_state_q20_distance_14` | 81.38x |
| local-crz-distance-sweep-cpu-q20-2026-07-01 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_crz_distance_sweep_state_q20_distance_15` | 83.96x |
| local-crz-distance-sweep-cpu-q20-2026-07-01 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_crz_distance_sweep_state_q20_distance_16` | 97.90x |
| local-crz-distance-sweep-cpu-q20-2026-07-01 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_crz_distance_sweep_state_q20_distance_17` | 72.56x |
| local-crz-distance-sweep-cpu-q20-2026-07-01 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_crz_distance_sweep_state_q20_distance_18` | 70.92x |
| local-crz-distance-sweep-cpu-q20-2026-07-01 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_crz_distance_sweep_state_q20_distance_19` | 68.56x |
| local-crz-distance-sweep-cpu-q20-2026-07-01 | `mklq_cpu_elapsed_seconds_median.crz_distance_sweep_state_q20_distance_1` | 0.0748332 s |
| local-crz-distance-sweep-cpu-q20-2026-07-01 | `mklq_cpu_elapsed_seconds_median.crz_distance_sweep_state_q20_distance_2` | 0.0645978 s |
| local-crz-distance-sweep-cpu-q20-2026-07-01 | `mklq_cpu_elapsed_seconds_median.crz_distance_sweep_state_q20_distance_3` | 0.062464 s |
| local-crz-distance-sweep-cpu-q20-2026-07-01 | `mklq_cpu_elapsed_seconds_median.crz_distance_sweep_state_q20_distance_4` | 0.0648671 s |
| local-crz-distance-sweep-cpu-q20-2026-07-01 | `mklq_cpu_elapsed_seconds_median.crz_distance_sweep_state_q20_distance_5` | 0.0616692 s |
| local-crz-distance-sweep-cpu-q20-2026-07-01 | `mklq_cpu_elapsed_seconds_median.crz_distance_sweep_state_q20_distance_6` | 0.0567954 s |
| local-crz-distance-sweep-cpu-q20-2026-07-01 | `mklq_cpu_elapsed_seconds_median.crz_distance_sweep_state_q20_distance_7` | 0.0515437 s |
| local-crz-distance-sweep-cpu-q20-2026-07-01 | `mklq_cpu_elapsed_seconds_median.crz_distance_sweep_state_q20_distance_8` | 0.0518899 s |
| local-crz-distance-sweep-cpu-q20-2026-07-01 | `mklq_cpu_elapsed_seconds_median.crz_distance_sweep_state_q20_distance_9` | 0.0495797 s |
| local-crz-distance-sweep-cpu-q20-2026-07-01 | `mklq_cpu_elapsed_seconds_median.crz_distance_sweep_state_q20_distance_10` | 0.0454354 s |
| local-crz-distance-sweep-cpu-q20-2026-07-01 | `mklq_cpu_elapsed_seconds_median.crz_distance_sweep_state_q20_distance_11` | 0.0455507 s |
| local-crz-distance-sweep-cpu-q20-2026-07-01 | `mklq_cpu_elapsed_seconds_median.crz_distance_sweep_state_q20_distance_12` | 0.0398609 s |
| local-crz-distance-sweep-cpu-q20-2026-07-01 | `mklq_cpu_elapsed_seconds_median.crz_distance_sweep_state_q20_distance_13` | 0.036468 s |
| local-crz-distance-sweep-cpu-q20-2026-07-01 | `mklq_cpu_elapsed_seconds_median.crz_distance_sweep_state_q20_distance_14` | 0.0341299 s |
| local-crz-distance-sweep-cpu-q20-2026-07-01 | `mklq_cpu_elapsed_seconds_median.crz_distance_sweep_state_q20_distance_15` | 0.0315363 s |
| local-crz-distance-sweep-cpu-q20-2026-07-01 | `mklq_cpu_elapsed_seconds_median.crz_distance_sweep_state_q20_distance_16` | 0.0298987 s |
| local-crz-distance-sweep-cpu-q20-2026-07-01 | `mklq_cpu_elapsed_seconds_median.crz_distance_sweep_state_q20_distance_17` | 0.0262286 s |
| local-crz-distance-sweep-cpu-q20-2026-07-01 | `mklq_cpu_elapsed_seconds_median.crz_distance_sweep_state_q20_distance_18` | 0.0233933 s |
| local-crz-distance-sweep-cpu-q20-2026-07-01 | `mklq_cpu_elapsed_seconds_median.crz_distance_sweep_state_q20_distance_19` | 0.0216201 s |
| local-current-sampling-fullprob-gated-q20-2026-06-19 | `pre_gate_probe.mklq_metal_sample_full_register_q20_seconds` | 0.0313161 s |
| local-current-sampling-fullprob-gated-q20-2026-06-19 | `pre_gate_probe.mklq_metal_sample_partial_register_q20_seconds` | 0.255697 s |
| local-current-sampling-fullprob-gated-q20-2026-06-19 | `pre_gate_probe.repeats` | 1 |
| local-current-sampling-fullprob-gated-q20-2026-06-19 | `pre_gate_probe.shots` | 1024 |
| local-current-sampling-fullprob-gated-q20-2026-06-19 | `same_day_tuning_ratio.mklq_metal_sample_partial_register_pre_gate_over_cost_gate` | 11.62x |
| local-metal-composite-mixed-path-q20-2026-06-21 | `mklq_metal_elapsed_seconds_median.qft_like_state_q20` | 1.21681 s |
| local-metal-composite-mixed-path-q20-2026-06-21 | `mklq_metal_elapsed_seconds_median.seeded_clifford_state_q20` | 0.189254 s |
| local-metal-composite-mixed-path-q20-2026-06-21 | `same_day_cross_target_ratio.qpp_cpu_over_mklq_metal_qft_like_state_q20` | 56.00x |
| local-metal-composite-mixed-path-q20-2026-06-21 | `same_day_cross_target_ratio.qpp_cpu_over_mklq_metal_seeded_clifford_state_q20` | 70.50x |
| local-metal-count-accumulation-sampling-q20-2026-07-04 | `mklq_metal_elapsed_seconds_median.sample_full_register_q20_1024_shots` | 0.0213063 s |
| local-metal-count-accumulation-sampling-q20-2026-07-04 | `mklq_metal_elapsed_seconds_median.sample_full_register_q20_256_shots` | 0.042652 s |
| local-metal-count-accumulation-sampling-q20-2026-07-04 | `mklq_metal_elapsed_seconds_median.sample_full_register_q20_65536_shots` | 0.042121 s |
| local-metal-count-accumulation-sampling-q20-2026-07-04 | `mklq_metal_elapsed_seconds_median.sample_full_register_q20_8192_shots` | 0.0215093 s |
| local-metal-count-accumulation-sampling-q20-2026-07-04 | `mklq_metal_elapsed_seconds_median.sample_partial_register_q20_1024_shots` | 0.0321124 s |
| local-metal-count-accumulation-sampling-q20-2026-07-04 | `mklq_metal_elapsed_seconds_median.sample_partial_register_q20_256_shots` | 0.0381636 s |
| local-metal-count-accumulation-sampling-q20-2026-07-04 | `mklq_metal_elapsed_seconds_median.sample_partial_register_q20_65536_shots` | 0.0213736 s |
| local-metal-count-accumulation-sampling-q20-2026-07-04 | `mklq_metal_elapsed_seconds_median.sample_partial_register_q20_8192_shots` | 0.0227139 s |
| local-metal-count-accumulation-sampling-q22-2026-07-04 | `mklq_metal_elapsed_seconds_median.sample_full_register_q22_1024_shots` | 0.0705315 s |
| local-metal-count-accumulation-sampling-q22-2026-07-04 | `mklq_metal_elapsed_seconds_median.sample_full_register_q22_256_shots` | 0.0713257 s |
| local-metal-count-accumulation-sampling-q22-2026-07-04 | `mklq_metal_elapsed_seconds_median.sample_full_register_q22_65536_shots` | 0.0651732 s |
| local-metal-count-accumulation-sampling-q22-2026-07-04 | `mklq_metal_elapsed_seconds_median.sample_full_register_q22_8192_shots` | 0.0666397 s |
| local-metal-count-accumulation-sampling-q22-2026-07-04 | `mklq_metal_elapsed_seconds_median.sample_partial_register_q22_1024_shots` | 0.0689797 s |
| local-metal-count-accumulation-sampling-q22-2026-07-04 | `mklq_metal_elapsed_seconds_median.sample_partial_register_q22_256_shots` | 0.0753249 s |
| local-metal-count-accumulation-sampling-q22-2026-07-04 | `mklq_metal_elapsed_seconds_median.sample_partial_register_q22_65536_shots` | 0.071028 s |
| local-metal-count-accumulation-sampling-q22-2026-07-04 | `mklq_metal_elapsed_seconds_median.sample_partial_register_q22_8192_shots` | 0.0736341 s |
| local-metal-diagonal-phase-q20-2026-07-08 | `mklq_metal_elapsed_seconds_median.diagonal_phase_state_q20` | 0.135114 s |
| local-metal-diagonal-phase-q20-2026-07-08 | `same_day_cross_target_ratio.qpp_cpu_over_mklq_metal_diagonal_phase_state_q20` | 69.99x |
| local-metal-partial-count-accumulation-sampling-q20-2026-07-05 | `mklq_metal_elapsed_seconds_median.sample_full_register_q20_1024_shots` | 0.0213005 s |
| local-metal-partial-count-accumulation-sampling-q20-2026-07-05 | `mklq_metal_elapsed_seconds_median.sample_full_register_q20_256_shots` | 0.0203252 s |
| local-metal-partial-count-accumulation-sampling-q20-2026-07-05 | `mklq_metal_elapsed_seconds_median.sample_full_register_q20_65536_shots` | 0.0261075 s |
| local-metal-partial-count-accumulation-sampling-q20-2026-07-05 | `mklq_metal_elapsed_seconds_median.sample_full_register_q20_8192_shots` | 0.0221756 s |
| local-metal-partial-count-accumulation-sampling-q20-2026-07-05 | `mklq_metal_elapsed_seconds_median.sample_partial_register_q20_1024_shots` | 0.021208 s |
| local-metal-partial-count-accumulation-sampling-q20-2026-07-05 | `mklq_metal_elapsed_seconds_median.sample_partial_register_q20_256_shots` | 0.0286338 s |
| local-metal-partial-count-accumulation-sampling-q20-2026-07-05 | `mklq_metal_elapsed_seconds_median.sample_partial_register_q20_65536_shots` | 0.023933 s |
| local-metal-partial-count-accumulation-sampling-q20-2026-07-05 | `mklq_metal_elapsed_seconds_median.sample_partial_register_q20_8192_shots` | 0.0412857 s |
| local-metal-partial-count-accumulation-sampling-q22-2026-07-05 | `mklq_metal_elapsed_seconds_median.sample_full_register_q22_1024_shots` | 0.0761623 s |
| local-metal-partial-count-accumulation-sampling-q22-2026-07-05 | `mklq_metal_elapsed_seconds_median.sample_full_register_q22_256_shots` | 0.0955889 s |
| local-metal-partial-count-accumulation-sampling-q22-2026-07-05 | `mklq_metal_elapsed_seconds_median.sample_full_register_q22_65536_shots` | 0.0912466 s |
| local-metal-partial-count-accumulation-sampling-q22-2026-07-05 | `mklq_metal_elapsed_seconds_median.sample_full_register_q22_8192_shots` | 0.0957031 s |
| local-metal-partial-count-accumulation-sampling-q22-2026-07-05 | `mklq_metal_elapsed_seconds_median.sample_partial_register_q22_1024_shots` | 0.0807732 s |
| local-metal-partial-count-accumulation-sampling-q22-2026-07-05 | `mklq_metal_elapsed_seconds_median.sample_partial_register_q22_256_shots` | 0.0734609 s |
| local-metal-partial-count-accumulation-sampling-q22-2026-07-05 | `mklq_metal_elapsed_seconds_median.sample_partial_register_q22_65536_shots` | 0.084595 s |
| local-metal-partial-count-accumulation-sampling-q22-2026-07-05 | `mklq_metal_elapsed_seconds_median.sample_partial_register_q22_8192_shots` | 0.0888247 s |
| local-metal-partial-count-accumulation-sampling-q24-2026-07-05 | `mklq_metal_elapsed_seconds_median.sample_full_register_q24_1024_shots` | 0.225761 s |
| local-metal-partial-count-accumulation-sampling-q24-2026-07-05 | `mklq_metal_elapsed_seconds_median.sample_full_register_q24_256_shots` | 0.22206 s |
| local-metal-partial-count-accumulation-sampling-q24-2026-07-05 | `mklq_metal_elapsed_seconds_median.sample_full_register_q24_65536_shots` | 0.218408 s |
| local-metal-partial-count-accumulation-sampling-q24-2026-07-05 | `mklq_metal_elapsed_seconds_median.sample_full_register_q24_8192_shots` | 0.222875 s |
| local-metal-partial-count-accumulation-sampling-q24-2026-07-05 | `mklq_metal_elapsed_seconds_median.sample_partial_register_q24_1024_shots` | 0.242955 s |
| local-metal-partial-count-accumulation-sampling-q24-2026-07-05 | `mklq_metal_elapsed_seconds_median.sample_partial_register_q24_256_shots` | 0.243337 s |
| local-metal-partial-count-accumulation-sampling-q24-2026-07-05 | `mklq_metal_elapsed_seconds_median.sample_partial_register_q24_65536_shots` | 0.22909 s |
| local-metal-partial-count-accumulation-sampling-q24-2026-07-05 | `mklq_metal_elapsed_seconds_median.sample_partial_register_q24_8192_shots` | 0.224184 s |
| local-metal-path-labels-q20-2026-06-22 | `mklq_metal_elapsed_seconds_median.cy_state_q20` | 0.195022 s |
| local-metal-path-labels-q20-2026-06-22 | `mklq_metal_elapsed_seconds_median.qft_like_state_q20` | 1.33702 s |
| local-metal-path-labels-q20-2026-06-22 | `mklq_metal_elapsed_seconds_median.sample_full_register_q20_1024_shots` | 0.0299139 s |
| local-metal-path-labels-q20-2026-06-22 | `mklq_metal_elapsed_seconds_median.sample_full_register_q20_65536_shots` | 0.0317822 s |
| local-metal-path-labels-q20-2026-06-22 | `mklq_metal_elapsed_seconds_median.sample_partial_register_q20_1024_shots` | 0.059596 s |
| local-metal-path-labels-q20-2026-06-22 | `mklq_metal_elapsed_seconds_median.sample_partial_register_q20_65536_shots` | 0.0324173 s |
| local-metal-path-labels-q20-2026-06-22 | `mklq_metal_elapsed_seconds_median.seeded_clifford_state_q20` | 0.164919 s |
| local-metal-path-labels-q20-2026-06-22 | `mklq_metal_elapsed_seconds_median.y_state_q20` | 0.14844 s |
| local-metal-sampling-boundary-q22-2026-07-04 | `mklq_metal_elapsed_seconds_median.sample_full_register_q22_1024_shots` | 0.066725 s |
| local-metal-sampling-boundary-q22-2026-07-04 | `mklq_metal_elapsed_seconds_median.sample_full_register_q22_256_shots` | 0.0612515 s |
| local-metal-sampling-boundary-q22-2026-07-04 | `mklq_metal_elapsed_seconds_median.sample_full_register_q22_65536_shots` | 0.0972842 s |
| local-metal-sampling-boundary-q22-2026-07-04 | `mklq_metal_elapsed_seconds_median.sample_full_register_q22_8192_shots` | 0.058947 s |
| local-metal-sampling-boundary-q22-2026-07-04 | `mklq_metal_elapsed_seconds_median.sample_partial_register_q22_1024_shots` | 0.0884355 s |
| local-metal-sampling-boundary-q22-2026-07-04 | `mklq_metal_elapsed_seconds_median.sample_partial_register_q22_256_shots` | 0.0727415 s |
| local-metal-sampling-boundary-q22-2026-07-04 | `mklq_metal_elapsed_seconds_median.sample_partial_register_q22_65536_shots` | 0.0809605 s |
| local-metal-sampling-boundary-q22-2026-07-04 | `mklq_metal_elapsed_seconds_median.sample_partial_register_q22_8192_shots` | 0.077946 s |
| local-metal-three-qubit-resident-q20-2026-06-22 | `mklq_metal_elapsed_seconds_median.three_qubit_state_q20` | 0.155606 s |
| local-metal-three-qubit-resident-q20-2026-06-22 | `same_day_cross_target_ratio.qpp_cpu_over_mklq_metal_three_qubit_state_q20` | 54.67x |
| local-metal-uniform-partial-sampling-q20-q24-2026-07-05 | `mklq_metal_elapsed_seconds_median.sample_uniform_partial_register_q20_1024_shots` | 0.0159298 s |
| local-metal-uniform-partial-sampling-q20-q24-2026-07-05 | `mklq_metal_elapsed_seconds_median.sample_uniform_partial_register_q20_256_shots` | 0.0143657 s |
| local-metal-uniform-partial-sampling-q20-q24-2026-07-05 | `mklq_metal_elapsed_seconds_median.sample_uniform_partial_register_q20_65536_shots` | 0.0184548 s |
| local-metal-uniform-partial-sampling-q20-q24-2026-07-05 | `mklq_metal_elapsed_seconds_median.sample_uniform_partial_register_q20_8192_shots` | 0.0168338 s |
| local-metal-uniform-partial-sampling-q20-q24-2026-07-05 | `mklq_metal_elapsed_seconds_median.sample_uniform_partial_register_q22_1024_shots` | 0.0753138 s |
| local-metal-uniform-partial-sampling-q20-q24-2026-07-05 | `mklq_metal_elapsed_seconds_median.sample_uniform_partial_register_q22_256_shots` | 0.0473506 s |
| local-metal-uniform-partial-sampling-q20-q24-2026-07-05 | `mklq_metal_elapsed_seconds_median.sample_uniform_partial_register_q22_65536_shots` | 0.0422257 s |
| local-metal-uniform-partial-sampling-q20-q24-2026-07-05 | `mklq_metal_elapsed_seconds_median.sample_uniform_partial_register_q22_8192_shots` | 0.0484554 s |
| local-metal-uniform-partial-sampling-q20-q24-2026-07-05 | `mklq_metal_elapsed_seconds_median.sample_uniform_partial_register_q24_1024_shots` | 0.161996 s |
| local-metal-uniform-partial-sampling-q20-q24-2026-07-05 | `mklq_metal_elapsed_seconds_median.sample_uniform_partial_register_q24_256_shots` | 0.135619 s |
| local-metal-uniform-partial-sampling-q20-q24-2026-07-05 | `mklq_metal_elapsed_seconds_median.sample_uniform_partial_register_q24_65536_shots` | 0.150551 s |
| local-metal-uniform-partial-sampling-q20-q24-2026-07-05 | `mklq_metal_elapsed_seconds_median.sample_uniform_partial_register_q24_8192_shots` | 0.144362 s |
| local-metal-y-cy-resident-isolated-q20-2026-06-19 | `same_day_cross_target_ratio.mklq_metal_over_mklq_cpu_cy_state_q20` | 1.74x |
| local-metal-y-cy-resident-isolated-q20-2026-06-19 | `same_day_cross_target_ratio.mklq_metal_over_mklq_cpu_y_state_q20` | 2.13x |
| local-metal-y-cy-resident-isolated-q20-2026-06-19 | `same_day_cross_target_ratio.qpp_cpu_over_mklq_metal_cy_state_q20` | 53.20x |
| local-metal-y-cy-resident-isolated-q20-2026-06-19 | `same_day_cross_target_ratio.qpp_cpu_over_mklq_metal_y_state_q20` | 56.16x |
| local-multi-control-cpu-q20-2026-06-22 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_multi_control_state_q20` | 45.09x |
| local-multi-control-cpu-q20-2026-06-22 | `mklq_cpu_elapsed_seconds_median.multi_control_state_q20` | 0.174905 s |
| local-sampling-profile-breakdown-cpu-q20-q22-2026-06-23 | `mklq_cpu_elapsed_seconds_median.sample_full_register_q20_1024_shots` | 0.0282634 s |
| local-sampling-profile-breakdown-cpu-q20-q22-2026-06-23 | `mklq_cpu_elapsed_seconds_median.sample_full_register_q20_65536_shots` | 0.0203374 s |
| local-sampling-profile-breakdown-cpu-q20-q22-2026-06-23 | `mklq_cpu_elapsed_seconds_median.sample_full_register_q22_1024_shots` | 0.0832497 s |
| local-sampling-profile-breakdown-cpu-q20-q22-2026-06-23 | `mklq_cpu_elapsed_seconds_median.sample_full_register_q22_65536_shots` | 0.0846335 s |
| local-sampling-profile-breakdown-cpu-q20-q22-2026-06-23 | `mklq_cpu_elapsed_seconds_median.sample_partial_register_q20_1024_shots` | 0.0128818 s |
| local-sampling-profile-breakdown-cpu-q20-q22-2026-06-23 | `mklq_cpu_elapsed_seconds_median.sample_partial_register_q20_65536_shots` | 0.0141688 s |
| local-sampling-profile-breakdown-cpu-q20-q22-2026-06-23 | `mklq_cpu_elapsed_seconds_median.sample_partial_register_q22_1024_shots` | 0.0914932 s |
| local-sampling-profile-breakdown-cpu-q20-q22-2026-06-23 | `mklq_cpu_elapsed_seconds_median.sample_partial_register_q22_65536_shots` | 0.0758121 s |
| local-sampling-scaling-cpu-q18-q22-2026-06-23 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_sample_full_register_q18_1024_shots` | 78.52x |
| local-sampling-scaling-cpu-q18-q22-2026-06-23 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_sample_full_register_q18_65536_shots` | 45.24x |
| local-sampling-scaling-cpu-q18-q22-2026-06-23 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_sample_full_register_q20_1024_shots` | 81.56x |
| local-sampling-scaling-cpu-q18-q22-2026-06-23 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_sample_full_register_q20_65536_shots` | 54.22x |
| local-sampling-scaling-cpu-q18-q22-2026-06-23 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_sample_full_register_q22_1024_shots` | 66.32x |
| local-sampling-scaling-cpu-q18-q22-2026-06-23 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_sample_full_register_q22_65536_shots` | 55.63x |
| local-sampling-scaling-cpu-q18-q22-2026-06-23 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_sample_partial_register_q18_1024_shots` | 109.56x |
| local-sampling-scaling-cpu-q18-q22-2026-06-23 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_sample_partial_register_q18_65536_shots` | 77.84x |
| local-sampling-scaling-cpu-q18-q22-2026-06-23 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_sample_partial_register_q20_1024_shots` | 82.19x |
| local-sampling-scaling-cpu-q18-q22-2026-06-23 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_sample_partial_register_q20_65536_shots` | 93.82x |
| local-sampling-scaling-cpu-q18-q22-2026-06-23 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_sample_partial_register_q22_1024_shots` | 81.32x |
| local-sampling-scaling-cpu-q18-q22-2026-06-23 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_sample_partial_register_q22_65536_shots` | 80.44x |
| local-sampling-scaling-cpu-q18-q22-2026-06-23 | `mklq_cpu_elapsed_seconds_median.sample_full_register_q18_1024_shots` | 0.00287288 s |
| local-sampling-scaling-cpu-q18-q22-2026-06-23 | `mklq_cpu_elapsed_seconds_median.sample_full_register_q18_65536_shots` | 0.0054851 s |
| local-sampling-scaling-cpu-q18-q22-2026-06-23 | `mklq_cpu_elapsed_seconds_median.sample_full_register_q20_1024_shots` | 0.0129733 s |
| local-sampling-scaling-cpu-q18-q22-2026-06-23 | `mklq_cpu_elapsed_seconds_median.sample_full_register_q20_65536_shots` | 0.0189641 s |
| local-sampling-scaling-cpu-q18-q22-2026-06-23 | `mklq_cpu_elapsed_seconds_median.sample_full_register_q22_1024_shots` | 0.0731874 s |
| local-sampling-scaling-cpu-q18-q22-2026-06-23 | `mklq_cpu_elapsed_seconds_median.sample_full_register_q22_65536_shots` | 0.0879145 s |
| local-sampling-scaling-cpu-q18-q22-2026-06-23 | `mklq_cpu_elapsed_seconds_median.sample_partial_register_q18_1024_shots` | 0.002284 s |
| local-sampling-scaling-cpu-q18-q22-2026-06-23 | `mklq_cpu_elapsed_seconds_median.sample_partial_register_q18_65536_shots` | 0.00329994 s |
| local-sampling-scaling-cpu-q18-q22-2026-06-23 | `mklq_cpu_elapsed_seconds_median.sample_partial_register_q20_1024_shots` | 0.0138417 s |
| local-sampling-scaling-cpu-q18-q22-2026-06-23 | `mklq_cpu_elapsed_seconds_median.sample_partial_register_q20_65536_shots` | 0.0120873 s |
| local-sampling-scaling-cpu-q18-q22-2026-06-23 | `mklq_cpu_elapsed_seconds_median.sample_partial_register_q22_1024_shots` | 0.0686197 s |
| local-sampling-scaling-cpu-q18-q22-2026-06-23 | `mklq_cpu_elapsed_seconds_median.sample_partial_register_q22_65536_shots` | 0.0757497 s |
| local-scaling-cpu-controlled-dense-two-qubit-q18-q22-2026-07-08-controlled-dense-two-qubit-scaling | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_controlled_dense_two_qubit_state_q18` | 39.60x |
| local-scaling-cpu-controlled-dense-two-qubit-q18-q22-2026-07-08-controlled-dense-two-qubit-scaling | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_controlled_dense_two_qubit_state_q20` | 39.72x |
| local-scaling-cpu-controlled-dense-two-qubit-q18-q22-2026-07-08-controlled-dense-two-qubit-scaling | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_controlled_dense_two_qubit_state_q22` | 65.53x |
| local-scaling-cpu-controlled-dense-two-qubit-q18-q22-2026-07-08-controlled-dense-two-qubit-scaling | `mklq_cpu_elapsed_seconds_median.controlled_dense_two_qubit_state_q18` | 0.102496 s |
| local-scaling-cpu-controlled-dense-two-qubit-q18-q22-2026-07-08-controlled-dense-two-qubit-scaling | `mklq_cpu_elapsed_seconds_median.controlled_dense_two_qubit_state_q20` | 0.35266 s |
| local-scaling-cpu-controlled-dense-two-qubit-q18-q22-2026-07-08-controlled-dense-two-qubit-scaling | `mklq_cpu_elapsed_seconds_median.controlled_dense_two_qubit_state_q22` | 0.953518 s |
| local-scaling-cpu-controlled-swap-q18-q22-2026-07-09-controlled-swap-scaling | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_controlled_swap_state_q18` | 36.36x |
| local-scaling-cpu-controlled-swap-q18-q22-2026-07-09-controlled-swap-scaling | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_controlled_swap_state_q20` | 125.50x |
| local-scaling-cpu-controlled-swap-q18-q22-2026-07-09-controlled-swap-scaling | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_controlled_swap_state_q22` | 147.44x |
| local-scaling-cpu-controlled-swap-q18-q22-2026-07-09-controlled-swap-scaling | `mklq_cpu_elapsed_seconds_median.controlled_swap_state_q18` | 0.0389757 s |
| local-scaling-cpu-controlled-swap-q18-q22-2026-07-09-controlled-swap-scaling | `mklq_cpu_elapsed_seconds_median.controlled_swap_state_q20` | 0.052201 s |
| local-scaling-cpu-controlled-swap-q18-q22-2026-07-09-controlled-swap-scaling | `mklq_cpu_elapsed_seconds_median.controlled_swap_state_q22` | 0.280982 s |
| local-scaling-cpu-dense-two-qubit-q18-q22-2026-07-08-dense-two-qubit-scaling | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_dense_two_qubit_state_q18` | 17.23x |
| local-scaling-cpu-dense-two-qubit-q18-q22-2026-07-08-dense-two-qubit-scaling | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_dense_two_qubit_state_q20` | 24.23x |
| local-scaling-cpu-dense-two-qubit-q18-q22-2026-07-08-dense-two-qubit-scaling | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_dense_two_qubit_state_q22` | 31.95x |
| local-scaling-cpu-dense-two-qubit-q18-q22-2026-07-08-dense-two-qubit-scaling | `mklq_cpu_elapsed_seconds_median.dense_two_qubit_state_q18` | 0.14287 s |
| local-scaling-cpu-dense-two-qubit-q18-q22-2026-07-08-dense-two-qubit-scaling | `mklq_cpu_elapsed_seconds_median.dense_two_qubit_state_q20` | 0.449856 s |
| local-scaling-cpu-dense-two-qubit-q18-q22-2026-07-08-dense-two-qubit-scaling | `mklq_cpu_elapsed_seconds_median.dense_two_qubit_state_q22` | 2.05452 s |
| local-scaling-cpu-hardware-efficient-ansatz-q18-q22-2026-06-30 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_hardware_efficient_ansatz_state_q18` | 26.84x |
| local-scaling-cpu-hardware-efficient-ansatz-q18-q22-2026-06-30 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_hardware_efficient_ansatz_state_q20` | 52.94x |
| local-scaling-cpu-hardware-efficient-ansatz-q18-q22-2026-06-30 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_hardware_efficient_ansatz_state_q22` | 81.37x |
| local-scaling-cpu-hardware-efficient-ansatz-q18-q22-2026-06-30 | `mklq_cpu_elapsed_seconds_median.hardware_efficient_ansatz_state_q18` | 0.188219 s |
| local-scaling-cpu-hardware-efficient-ansatz-q18-q22-2026-06-30 | `mklq_cpu_elapsed_seconds_median.hardware_efficient_ansatz_state_q20` | 0.423815 s |
| local-scaling-cpu-hardware-efficient-ansatz-q18-q22-2026-06-30 | `mklq_cpu_elapsed_seconds_median.hardware_efficient_ansatz_state_q22` | 1.60115 s |
| local-scaling-cpu-multi-control-q18-q22-2026-06-22 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_multi_control_state_q18` | 11.66x |
| local-scaling-cpu-multi-control-q18-q22-2026-06-22 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_multi_control_state_q20` | 28.00x |
| local-scaling-cpu-multi-control-q18-q22-2026-06-22 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_multi_control_state_q22` | 72.93x |
| local-scaling-cpu-multi-control-q18-q22-2026-06-22 | `mklq_cpu_elapsed_seconds_median.multi_control_state_q18` | 0.163252 s |
| local-scaling-cpu-multi-control-q18-q22-2026-06-22 | `mklq_cpu_elapsed_seconds_median.multi_control_state_q20` | 0.277172 s |
| local-scaling-cpu-multi-control-q18-q22-2026-06-22 | `mklq_cpu_elapsed_seconds_median.multi_control_state_q22` | 0.771644 s |
| local-scaling-cpu-two-qubit-custom-two-qubit-three-qubit-q18-q22-2026-07-08-custom-two-qubit-scaling-main | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_custom_two_qubit_state_q18` | 20.62x |
| local-scaling-cpu-two-qubit-custom-two-qubit-three-qubit-q18-q22-2026-07-08-custom-two-qubit-scaling-main | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_custom_two_qubit_state_q20` | 31.07x |
| local-scaling-cpu-two-qubit-custom-two-qubit-three-qubit-q18-q22-2026-07-08-custom-two-qubit-scaling-main | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_custom_two_qubit_state_q22` | 60.98x |
| local-scaling-cpu-two-qubit-custom-two-qubit-three-qubit-q18-q22-2026-07-08-custom-two-qubit-scaling-main | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_three_qubit_state_q18` | 31.78x |
| local-scaling-cpu-two-qubit-custom-two-qubit-three-qubit-q18-q22-2026-07-08-custom-two-qubit-scaling-main | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_three_qubit_state_q20` | 68.35x |
| local-scaling-cpu-two-qubit-custom-two-qubit-three-qubit-q18-q22-2026-07-08-custom-two-qubit-scaling-main | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_three_qubit_state_q22` | 86.10x |
| local-scaling-cpu-two-qubit-custom-two-qubit-three-qubit-q18-q22-2026-07-08-custom-two-qubit-scaling-main | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_two_qubit_state_q18` | 45.23x |
| local-scaling-cpu-two-qubit-custom-two-qubit-three-qubit-q18-q22-2026-07-08-custom-two-qubit-scaling-main | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_two_qubit_state_q20` | 139.92x |
| local-scaling-cpu-two-qubit-custom-two-qubit-three-qubit-q18-q22-2026-07-08-custom-two-qubit-scaling-main | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_two_qubit_state_q22` | 171.88x |
| local-scaling-cpu-two-qubit-custom-two-qubit-three-qubit-q18-q22-2026-07-08-custom-two-qubit-scaling-main | `mklq_cpu_elapsed_seconds_median.custom_two_qubit_state_q18` | 0.108013 s |
| local-scaling-cpu-two-qubit-custom-two-qubit-three-qubit-q18-q22-2026-07-08-custom-two-qubit-scaling-main | `mklq_cpu_elapsed_seconds_median.custom_two_qubit_state_q20` | 0.318189 s |
| local-scaling-cpu-two-qubit-custom-two-qubit-three-qubit-q18-q22-2026-07-08-custom-two-qubit-scaling-main | `mklq_cpu_elapsed_seconds_median.custom_two_qubit_state_q22` | 1.04109 s |
| local-scaling-cpu-two-qubit-custom-two-qubit-three-qubit-q18-q22-2026-07-08-custom-two-qubit-scaling-main | `mklq_cpu_elapsed_seconds_median.three_qubit_state_q18` | 0.159559 s |
| local-scaling-cpu-two-qubit-custom-two-qubit-three-qubit-q18-q22-2026-07-08-custom-two-qubit-scaling-main | `mklq_cpu_elapsed_seconds_median.three_qubit_state_q20` | 0.238634 s |
| local-scaling-cpu-two-qubit-custom-two-qubit-three-qubit-q18-q22-2026-07-08-custom-two-qubit-scaling-main | `mklq_cpu_elapsed_seconds_median.three_qubit_state_q22` | 0.78728 s |
| local-scaling-cpu-two-qubit-custom-two-qubit-three-qubit-q18-q22-2026-07-08-custom-two-qubit-scaling-main | `mklq_cpu_elapsed_seconds_median.two_qubit_state_q18` | 0.0361282 s |
| local-scaling-cpu-two-qubit-custom-two-qubit-three-qubit-q18-q22-2026-07-08-custom-two-qubit-scaling-main | `mklq_cpu_elapsed_seconds_median.two_qubit_state_q20` | 0.0570591 s |
| local-scaling-cpu-two-qubit-custom-two-qubit-three-qubit-q18-q22-2026-07-08-custom-two-qubit-scaling-main | `mklq_cpu_elapsed_seconds_median.two_qubit_state_q22` | 0.259211 s |
| local-scaling-cpu-two-qubit-custom-two-qubit-three-qubit-q18-q22-2026-07-08-row-sparse-two-qubit | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_custom_two_qubit_state_q18` | 39.11x |
| local-scaling-cpu-two-qubit-custom-two-qubit-three-qubit-q18-q22-2026-07-08-row-sparse-two-qubit | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_custom_two_qubit_state_q20` | 90.81x |
| local-scaling-cpu-two-qubit-custom-two-qubit-three-qubit-q18-q22-2026-07-08-row-sparse-two-qubit | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_custom_two_qubit_state_q22` | 61.34x |
| local-scaling-cpu-two-qubit-custom-two-qubit-three-qubit-q18-q22-2026-07-08-row-sparse-two-qubit | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_three_qubit_state_q18` | 22.43x |
| local-scaling-cpu-two-qubit-custom-two-qubit-three-qubit-q18-q22-2026-07-08-row-sparse-two-qubit | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_three_qubit_state_q20` | 60.54x |
| local-scaling-cpu-two-qubit-custom-two-qubit-three-qubit-q18-q22-2026-07-08-row-sparse-two-qubit | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_three_qubit_state_q22` | 83.70x |
| local-scaling-cpu-two-qubit-custom-two-qubit-three-qubit-q18-q22-2026-07-08-row-sparse-two-qubit | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_two_qubit_state_q18` | 88.03x |
| local-scaling-cpu-two-qubit-custom-two-qubit-three-qubit-q18-q22-2026-07-08-row-sparse-two-qubit | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_two_qubit_state_q20` | 217.61x |
| local-scaling-cpu-two-qubit-custom-two-qubit-three-qubit-q18-q22-2026-07-08-row-sparse-two-qubit | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_two_qubit_state_q22` | 176.87x |
| local-scaling-cpu-two-qubit-custom-two-qubit-three-qubit-q18-q22-2026-07-08-row-sparse-two-qubit | `mklq_cpu_elapsed_seconds_median.custom_two_qubit_state_q18` | 0.0954476 s |
| local-scaling-cpu-two-qubit-custom-two-qubit-three-qubit-q18-q22-2026-07-08-row-sparse-two-qubit | `mklq_cpu_elapsed_seconds_median.custom_two_qubit_state_q20` | 0.149173 s |
| local-scaling-cpu-two-qubit-custom-two-qubit-three-qubit-q18-q22-2026-07-08-row-sparse-two-qubit | `mklq_cpu_elapsed_seconds_median.custom_two_qubit_state_q22` | 0.852549 s |
| local-scaling-cpu-two-qubit-custom-two-qubit-three-qubit-q18-q22-2026-07-08-row-sparse-two-qubit | `mklq_cpu_elapsed_seconds_median.three_qubit_state_q18` | 0.155295 s |
| local-scaling-cpu-two-qubit-custom-two-qubit-three-qubit-q18-q22-2026-07-08-row-sparse-two-qubit | `mklq_cpu_elapsed_seconds_median.three_qubit_state_q20` | 0.242565 s |
| local-scaling-cpu-two-qubit-custom-two-qubit-three-qubit-q18-q22-2026-07-08-row-sparse-two-qubit | `mklq_cpu_elapsed_seconds_median.three_qubit_state_q22` | 0.927065 s |
| local-scaling-cpu-two-qubit-custom-two-qubit-three-qubit-q18-q22-2026-07-08-row-sparse-two-qubit | `mklq_cpu_elapsed_seconds_median.two_qubit_state_q18` | 0.0364757 s |
| local-scaling-cpu-two-qubit-custom-two-qubit-three-qubit-q18-q22-2026-07-08-row-sparse-two-qubit | `mklq_cpu_elapsed_seconds_median.two_qubit_state_q20` | 0.0571493 s |
| local-scaling-cpu-two-qubit-custom-two-qubit-three-qubit-q18-q22-2026-07-08-row-sparse-two-qubit | `mklq_cpu_elapsed_seconds_median.two_qubit_state_q22` | 0.363025 s |
| local-scaling-cpu-two-qubit-three-qubit-q18-q22-2026-07-03-two-three-scaling | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_three_qubit_state_q18` | 24.54x |
| local-scaling-cpu-two-qubit-three-qubit-q18-q22-2026-07-03-two-three-scaling | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_three_qubit_state_q20` | 87.34x |
| local-scaling-cpu-two-qubit-three-qubit-q18-q22-2026-07-03-two-three-scaling | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_three_qubit_state_q22` | 90.91x |
| local-scaling-cpu-two-qubit-three-qubit-q18-q22-2026-07-03-two-three-scaling | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_two_qubit_state_q18` | 47.20x |
| local-scaling-cpu-two-qubit-three-qubit-q18-q22-2026-07-03-two-three-scaling | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_two_qubit_state_q20` | 131.99x |
| local-scaling-cpu-two-qubit-three-qubit-q18-q22-2026-07-03-two-three-scaling | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_two_qubit_state_q22` | 163.42x |
| local-scaling-cpu-two-qubit-three-qubit-q18-q22-2026-07-03-two-three-scaling | `mklq_cpu_elapsed_seconds_median.three_qubit_state_q18` | 0.211712 s |
| local-scaling-cpu-two-qubit-three-qubit-q18-q22-2026-07-03-two-three-scaling | `mklq_cpu_elapsed_seconds_median.three_qubit_state_q20` | 0.255251 s |
| local-scaling-cpu-two-qubit-three-qubit-q18-q22-2026-07-03-two-three-scaling | `mklq_cpu_elapsed_seconds_median.three_qubit_state_q22` | 0.942877 s |
| local-scaling-cpu-two-qubit-three-qubit-q18-q22-2026-07-03-two-three-scaling | `mklq_cpu_elapsed_seconds_median.two_qubit_state_q18` | 0.0456607 s |
| local-scaling-cpu-two-qubit-three-qubit-q18-q22-2026-07-03-two-three-scaling | `mklq_cpu_elapsed_seconds_median.two_qubit_state_q20` | 0.1019 s |
| local-scaling-cpu-two-qubit-three-qubit-q18-q22-2026-07-03-two-three-scaling | `mklq_cpu_elapsed_seconds_median.two_qubit_state_q22` | 0.351995 s |
| local-y-cy-fastpath-isolated-q20-2026-06-19 | `same_day_cross_target_ratio.qpp_cpu_over_mklq_cpu_cy_state_q20` | 103.85x |
| local-y-cy-fastpath-isolated-q20-2026-06-19 | `same_day_cross_target_ratio.qpp_cpu_over_mklq_cpu_y_state_q20` | 167.38x |

## Sampling Profile Signals

These rows expose benchmark-harness diagnostic timings around `cudaq.sample`. They are not native backend internal phase counters.

| Summary ID | Target | Case | Qubits | Shots | Elapsed | Kernel build | Sample call | Counts materialization | Boundary |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| local-sampling-profile-breakdown-cpu-q20-q22-2026-06-23 | mklq-cpu | sample-full-register | 20 | 1024 | 0.0282634 s | 0.000992167 s | 0.0282634 s | 1.01455e-05 s | Additional benchmark harness timings for kernel construction, cudaq.sample calls, and result count-map materialization; not native backend internal phase counters such as probability fill, draw, or counts aggregation. |
| local-sampling-profile-breakdown-cpu-q20-q22-2026-06-23 | mklq-cpu | sample-full-register | 20 | 65536 | 0.0203374 s | 0.000781354 s | 0.0203374 s | 1.1563e-05 s | Additional benchmark harness timings for kernel construction, cudaq.sample calls, and result count-map materialization; not native backend internal phase counters such as probability fill, draw, or counts aggregation. |
| local-sampling-profile-breakdown-cpu-q20-q22-2026-06-23 | mklq-cpu | sample-full-register | 22 | 1024 | 0.0832497 s | 0.000854646 s | 0.0832497 s | 7.875e-06 s | Additional benchmark harness timings for kernel construction, cudaq.sample calls, and result count-map materialization; not native backend internal phase counters such as probability fill, draw, or counts aggregation. |
| local-sampling-profile-breakdown-cpu-q20-q22-2026-06-23 | mklq-cpu | sample-full-register | 22 | 65536 | 0.0846335 s | 0.00101577 s | 0.0846335 s | 1.44165e-05 s | Additional benchmark harness timings for kernel construction, cudaq.sample calls, and result count-map materialization; not native backend internal phase counters such as probability fill, draw, or counts aggregation. |
| local-sampling-profile-breakdown-cpu-q20-q22-2026-06-23 | mklq-cpu | sample-partial-register | 20 | 1024 | 0.0128818 s | 0.000869979 s | 0.0128818 s | 7.896e-06 s | Additional benchmark harness timings for kernel construction, cudaq.sample calls, and result count-map materialization; not native backend internal phase counters such as probability fill, draw, or counts aggregation. |
| local-sampling-profile-breakdown-cpu-q20-q22-2026-06-23 | mklq-cpu | sample-partial-register | 20 | 65536 | 0.0141688 s | 0.00102417 s | 0.0141688 s | 9.29201e-06 s | Additional benchmark harness timings for kernel construction, cudaq.sample calls, and result count-map materialization; not native backend internal phase counters such as probability fill, draw, or counts aggregation. |
| local-sampling-profile-breakdown-cpu-q20-q22-2026-06-23 | mklq-cpu | sample-partial-register | 22 | 1024 | 0.0914932 s | 0.00123794 s | 0.0914932 s | 8.12502e-06 s | Additional benchmark harness timings for kernel construction, cudaq.sample calls, and result count-map materialization; not native backend internal phase counters such as probability fill, draw, or counts aggregation. |
| local-sampling-profile-breakdown-cpu-q20-q22-2026-06-23 | mklq-cpu | sample-partial-register | 22 | 65536 | 0.0758121 s | 0.00114002 s | 0.0758121 s | 1.12915e-05 s | Additional benchmark harness timings for kernel construction, cudaq.sample calls, and result count-map materialization; not native backend internal phase counters such as probability fill, draw, or counts aggregation. |

## Metal Path Labels

The rows below expose static benchmark case-map labels for `mklq-metal`. They are not runtime counters or proof that every operation stayed on Metal.

| Summary ID | Case | Qubits | Shots | Label | Scope | Source |
| --- | --- | --- | --- | --- | --- | --- |
| local-metal-count-accumulation-sampling-q20-2026-07-04 | sample-full-register | 20 | 256 | `mklq_metal_full_register_sample_count_accumulation` | mixed-path Metal probability fill with selected full-register counts-only Metal sample-count accumulation after host-generated draws | benchmark_harness_static_case_map |
| local-metal-count-accumulation-sampling-q20-2026-07-04 | sample-full-register | 20 | 1024 | `mklq_metal_full_register_sample_count_accumulation` | mixed-path Metal probability fill with selected full-register counts-only Metal sample-count accumulation after host-generated draws | benchmark_harness_static_case_map |
| local-metal-count-accumulation-sampling-q20-2026-07-04 | sample-full-register | 20 | 8192 | `mklq_metal_full_register_sample_count_accumulation` | mixed-path Metal probability fill with selected full-register counts-only Metal sample-count accumulation after host-generated draws | benchmark_harness_static_case_map |
| local-metal-count-accumulation-sampling-q20-2026-07-04 | sample-full-register | 20 | 65536 | `mklq_metal_full_register_sample_count_accumulation` | mixed-path Metal probability fill with selected full-register counts-only Metal sample-count accumulation after host-generated draws | benchmark_harness_static_case_map |
| local-metal-count-accumulation-sampling-q20-2026-07-04 | sample-partial-register | 20 | 256 | `mklq_metal_partial_register_host_counts` | mixed-path Metal probability fill with partial-register host-side sample draw/count accumulation | benchmark_harness_static_case_map |
| local-metal-count-accumulation-sampling-q20-2026-07-04 | sample-partial-register | 20 | 1024 | `mklq_metal_partial_register_host_counts` | mixed-path Metal probability fill with partial-register host-side sample draw/count accumulation | benchmark_harness_static_case_map |
| local-metal-count-accumulation-sampling-q20-2026-07-04 | sample-partial-register | 20 | 8192 | `mklq_metal_partial_register_host_counts` | mixed-path Metal probability fill with partial-register host-side sample draw/count accumulation | benchmark_harness_static_case_map |
| local-metal-count-accumulation-sampling-q20-2026-07-04 | sample-partial-register | 20 | 65536 | `mklq_metal_partial_register_host_counts` | mixed-path Metal probability fill with partial-register host-side sample draw/count accumulation | benchmark_harness_static_case_map |
| local-metal-count-accumulation-sampling-q22-2026-07-04 | sample-full-register | 22 | 256 | `mklq_metal_full_register_sample_count_accumulation` | mixed-path Metal probability fill with selected full-register counts-only Metal sample-count accumulation after host-generated draws | benchmark_harness_static_case_map |
| local-metal-count-accumulation-sampling-q22-2026-07-04 | sample-full-register | 22 | 1024 | `mklq_metal_full_register_sample_count_accumulation` | mixed-path Metal probability fill with selected full-register counts-only Metal sample-count accumulation after host-generated draws | benchmark_harness_static_case_map |
| local-metal-count-accumulation-sampling-q22-2026-07-04 | sample-full-register | 22 | 8192 | `mklq_metal_full_register_sample_count_accumulation` | mixed-path Metal probability fill with selected full-register counts-only Metal sample-count accumulation after host-generated draws | benchmark_harness_static_case_map |
| local-metal-count-accumulation-sampling-q22-2026-07-04 | sample-full-register | 22 | 65536 | `mklq_metal_full_register_sample_count_accumulation` | mixed-path Metal probability fill with selected full-register counts-only Metal sample-count accumulation after host-generated draws | benchmark_harness_static_case_map |
| local-metal-count-accumulation-sampling-q22-2026-07-04 | sample-partial-register | 22 | 256 | `mklq_metal_partial_register_host_counts` | mixed-path Metal probability fill with partial-register host-side sample draw/count accumulation | benchmark_harness_static_case_map |
| local-metal-count-accumulation-sampling-q22-2026-07-04 | sample-partial-register | 22 | 1024 | `mklq_metal_partial_register_host_counts` | mixed-path Metal probability fill with partial-register host-side sample draw/count accumulation | benchmark_harness_static_case_map |
| local-metal-count-accumulation-sampling-q22-2026-07-04 | sample-partial-register | 22 | 8192 | `mklq_metal_partial_register_host_counts` | mixed-path Metal probability fill with partial-register host-side sample draw/count accumulation | benchmark_harness_static_case_map |
| local-metal-count-accumulation-sampling-q22-2026-07-04 | sample-partial-register | 22 | 65536 | `mklq_metal_partial_register_host_counts` | mixed-path Metal probability fill with partial-register host-side sample draw/count accumulation | benchmark_harness_static_case_map |
| local-metal-diagonal-phase-q20-2026-07-08 | diagonal-phase-state | 20 | 1024 | `mklq_metal_resident_single_gate_state_host_readback` | resident fp32 Metal single-target gate update followed by host readback for cudaq.get_state | benchmark_harness_static_case_map |
| local-metal-partial-count-accumulation-sampling-q20-2026-07-05 | sample-full-register | 20 | 256 | `mklq_metal_full_register_sample_count_accumulation` | mixed-path Metal probability fill with selected full-register counts-only Metal sample-count accumulation after host-generated draws | benchmark_harness_static_case_map |
| local-metal-partial-count-accumulation-sampling-q20-2026-07-05 | sample-full-register | 20 | 1024 | `mklq_metal_full_register_sample_count_accumulation` | mixed-path Metal probability fill with selected full-register counts-only Metal sample-count accumulation after host-generated draws | benchmark_harness_static_case_map |
| local-metal-partial-count-accumulation-sampling-q20-2026-07-05 | sample-full-register | 20 | 8192 | `mklq_metal_full_register_sample_count_accumulation` | mixed-path Metal probability fill with selected full-register counts-only Metal sample-count accumulation after host-generated draws | benchmark_harness_static_case_map |
| local-metal-partial-count-accumulation-sampling-q20-2026-07-05 | sample-full-register | 20 | 65536 | `mklq_metal_full_register_sample_count_accumulation` | mixed-path Metal probability fill with selected full-register counts-only Metal sample-count accumulation after host-generated draws | benchmark_harness_static_case_map |
| local-metal-partial-count-accumulation-sampling-q20-2026-07-05 | sample-partial-register | 20 | 256 | `mklq_metal_partial_register_sample_count_accumulation` | mixed-path Metal probability fill with selected partial-register counts-only Metal sample-count accumulation after host-generated draws | benchmark_harness_static_case_map |
| local-metal-partial-count-accumulation-sampling-q20-2026-07-05 | sample-partial-register | 20 | 1024 | `mklq_metal_partial_register_sample_count_accumulation` | mixed-path Metal probability fill with selected partial-register counts-only Metal sample-count accumulation after host-generated draws | benchmark_harness_static_case_map |
| local-metal-partial-count-accumulation-sampling-q20-2026-07-05 | sample-partial-register | 20 | 8192 | `mklq_metal_partial_register_sample_count_accumulation` | mixed-path Metal probability fill with selected partial-register counts-only Metal sample-count accumulation after host-generated draws | benchmark_harness_static_case_map |
| local-metal-partial-count-accumulation-sampling-q20-2026-07-05 | sample-partial-register | 20 | 65536 | `mklq_metal_partial_register_sample_count_accumulation` | mixed-path Metal probability fill with selected partial-register counts-only Metal sample-count accumulation after host-generated draws | benchmark_harness_static_case_map |
| local-metal-partial-count-accumulation-sampling-q22-2026-07-05 | sample-full-register | 22 | 256 | `mklq_metal_full_register_sample_count_accumulation` | mixed-path Metal probability fill with selected full-register counts-only Metal sample-count accumulation after host-generated draws | benchmark_harness_static_case_map |
| local-metal-partial-count-accumulation-sampling-q22-2026-07-05 | sample-full-register | 22 | 1024 | `mklq_metal_full_register_sample_count_accumulation` | mixed-path Metal probability fill with selected full-register counts-only Metal sample-count accumulation after host-generated draws | benchmark_harness_static_case_map |
| local-metal-partial-count-accumulation-sampling-q22-2026-07-05 | sample-full-register | 22 | 8192 | `mklq_metal_full_register_sample_count_accumulation` | mixed-path Metal probability fill with selected full-register counts-only Metal sample-count accumulation after host-generated draws | benchmark_harness_static_case_map |
| local-metal-partial-count-accumulation-sampling-q22-2026-07-05 | sample-full-register | 22 | 65536 | `mklq_metal_full_register_sample_count_accumulation` | mixed-path Metal probability fill with selected full-register counts-only Metal sample-count accumulation after host-generated draws | benchmark_harness_static_case_map |
| local-metal-partial-count-accumulation-sampling-q22-2026-07-05 | sample-partial-register | 22 | 256 | `mklq_metal_partial_register_sample_count_accumulation` | mixed-path Metal probability fill with selected partial-register counts-only Metal sample-count accumulation after host-generated draws | benchmark_harness_static_case_map |
| local-metal-partial-count-accumulation-sampling-q22-2026-07-05 | sample-partial-register | 22 | 1024 | `mklq_metal_partial_register_sample_count_accumulation` | mixed-path Metal probability fill with selected partial-register counts-only Metal sample-count accumulation after host-generated draws | benchmark_harness_static_case_map |
| local-metal-partial-count-accumulation-sampling-q22-2026-07-05 | sample-partial-register | 22 | 8192 | `mklq_metal_partial_register_sample_count_accumulation` | mixed-path Metal probability fill with selected partial-register counts-only Metal sample-count accumulation after host-generated draws | benchmark_harness_static_case_map |
| local-metal-partial-count-accumulation-sampling-q22-2026-07-05 | sample-partial-register | 22 | 65536 | `mklq_metal_partial_register_sample_count_accumulation` | mixed-path Metal probability fill with selected partial-register counts-only Metal sample-count accumulation after host-generated draws | benchmark_harness_static_case_map |
| local-metal-partial-count-accumulation-sampling-q24-2026-07-05 | sample-full-register | 24 | 256 | `mklq_metal_full_register_sample_count_accumulation` | mixed-path Metal probability fill with selected full-register counts-only Metal sample-count accumulation after host-generated draws | benchmark_harness_static_case_map |
| local-metal-partial-count-accumulation-sampling-q24-2026-07-05 | sample-full-register | 24 | 1024 | `mklq_metal_full_register_sample_count_accumulation` | mixed-path Metal probability fill with selected full-register counts-only Metal sample-count accumulation after host-generated draws | benchmark_harness_static_case_map |
| local-metal-partial-count-accumulation-sampling-q24-2026-07-05 | sample-full-register | 24 | 8192 | `mklq_metal_full_register_sample_count_accumulation` | mixed-path Metal probability fill with selected full-register counts-only Metal sample-count accumulation after host-generated draws | benchmark_harness_static_case_map |
| local-metal-partial-count-accumulation-sampling-q24-2026-07-05 | sample-full-register | 24 | 65536 | `mklq_metal_full_register_sample_count_accumulation` | mixed-path Metal probability fill with selected full-register counts-only Metal sample-count accumulation after host-generated draws | benchmark_harness_static_case_map |
| local-metal-partial-count-accumulation-sampling-q24-2026-07-05 | sample-partial-register | 24 | 256 | `mklq_metal_partial_register_sample_count_accumulation` | mixed-path Metal probability fill with selected partial-register counts-only Metal sample-count accumulation after host-generated draws | benchmark_harness_static_case_map |
| local-metal-partial-count-accumulation-sampling-q24-2026-07-05 | sample-partial-register | 24 | 1024 | `mklq_metal_partial_register_sample_count_accumulation` | mixed-path Metal probability fill with selected partial-register counts-only Metal sample-count accumulation after host-generated draws | benchmark_harness_static_case_map |
| local-metal-partial-count-accumulation-sampling-q24-2026-07-05 | sample-partial-register | 24 | 8192 | `mklq_metal_partial_register_sample_count_accumulation` | mixed-path Metal probability fill with selected partial-register counts-only Metal sample-count accumulation after host-generated draws | benchmark_harness_static_case_map |
| local-metal-partial-count-accumulation-sampling-q24-2026-07-05 | sample-partial-register | 24 | 65536 | `mklq_metal_partial_register_sample_count_accumulation` | mixed-path Metal probability fill with selected partial-register counts-only Metal sample-count accumulation after host-generated draws | benchmark_harness_static_case_map |
| local-metal-path-labels-q20-2026-06-22 | y-state | 20 | 1024 | `mklq_metal_resident_single_gate_state_host_readback` | resident fp32 Metal single-target gate update followed by host readback for cudaq.get_state | benchmark_harness_static_case_map |
| local-metal-path-labels-q20-2026-06-22 | cy-state | 20 | 1024 | `mklq_metal_resident_controlled_gate_state_host_readback` | resident fp32 Metal controlled gate update followed by host readback for cudaq.get_state | benchmark_harness_static_case_map |
| local-metal-path-labels-q20-2026-06-22 | qft-like-state | 20 | 1024 | `mklq_metal_mixed_composite_state_host_readback` | experimental mklq-metal mixed-path composite state-vector update followed by host readback for cudaq.get_state | benchmark_harness_static_case_map |
| local-metal-path-labels-q20-2026-06-22 | seeded-clifford-state | 20 | 1024 | `mklq_metal_mixed_composite_state_host_readback` | experimental mklq-metal mixed-path composite state-vector update followed by host readback for cudaq.get_state | benchmark_harness_static_case_map |
| local-metal-path-labels-q20-2026-06-22 | sample-full-register | 20 | 1024 | `mklq_metal_mixed_sampling_host_counts` | mixed-path Metal probability fill with host-side sample draw/count accumulation | benchmark_harness_static_case_map |
| local-metal-path-labels-q20-2026-06-22 | sample-full-register | 20 | 65536 | `mklq_metal_mixed_sampling_host_counts` | mixed-path Metal probability fill with host-side sample draw/count accumulation | benchmark_harness_static_case_map |
| local-metal-path-labels-q20-2026-06-22 | sample-partial-register | 20 | 1024 | `mklq_metal_mixed_sampling_host_counts` | mixed-path Metal probability fill with host-side sample draw/count accumulation | benchmark_harness_static_case_map |
| local-metal-path-labels-q20-2026-06-22 | sample-partial-register | 20 | 65536 | `mklq_metal_mixed_sampling_host_counts` | mixed-path Metal probability fill with host-side sample draw/count accumulation | benchmark_harness_static_case_map |
| local-metal-sampling-boundary-q22-2026-07-04 | sample-full-register | 22 | 256 | `mklq_metal_mixed_sampling_host_counts` | mixed-path Metal probability fill with host-side sample draw/count accumulation | benchmark_harness_static_case_map |
| local-metal-sampling-boundary-q22-2026-07-04 | sample-full-register | 22 | 1024 | `mklq_metal_mixed_sampling_host_counts` | mixed-path Metal probability fill with host-side sample draw/count accumulation | benchmark_harness_static_case_map |
| local-metal-sampling-boundary-q22-2026-07-04 | sample-full-register | 22 | 8192 | `mklq_metal_mixed_sampling_host_counts` | mixed-path Metal probability fill with host-side sample draw/count accumulation | benchmark_harness_static_case_map |
| local-metal-sampling-boundary-q22-2026-07-04 | sample-full-register | 22 | 65536 | `mklq_metal_mixed_sampling_host_counts` | mixed-path Metal probability fill with host-side sample draw/count accumulation | benchmark_harness_static_case_map |
| local-metal-sampling-boundary-q22-2026-07-04 | sample-partial-register | 22 | 256 | `mklq_metal_mixed_sampling_host_counts` | mixed-path Metal probability fill with host-side sample draw/count accumulation | benchmark_harness_static_case_map |
| local-metal-sampling-boundary-q22-2026-07-04 | sample-partial-register | 22 | 1024 | `mklq_metal_mixed_sampling_host_counts` | mixed-path Metal probability fill with host-side sample draw/count accumulation | benchmark_harness_static_case_map |
| local-metal-sampling-boundary-q22-2026-07-04 | sample-partial-register | 22 | 8192 | `mklq_metal_mixed_sampling_host_counts` | mixed-path Metal probability fill with host-side sample draw/count accumulation | benchmark_harness_static_case_map |
| local-metal-sampling-boundary-q22-2026-07-04 | sample-partial-register | 22 | 65536 | `mklq_metal_mixed_sampling_host_counts` | mixed-path Metal probability fill with host-side sample draw/count accumulation | benchmark_harness_static_case_map |
| local-metal-three-qubit-resident-q20-2026-06-22 | three-qubit-state | 20 | 1024 | `mklq_metal_resident_three_gate_state_host_readback` | resident fp32 Metal three-target gate update followed by host readback for cudaq.get_state | benchmark_harness_static_case_map |
| local-metal-uniform-partial-sampling-q20-q24-2026-07-05 | sample-uniform-partial-register | 20 | 256 | `mklq_metal_uniform_partial_register_sample_count_accumulation` | mixed-path Metal marginal probability fill with uniform-probability generated-count fast path for counts-only partial-register sample-count accumulation | benchmark_harness_static_case_map |
| local-metal-uniform-partial-sampling-q20-q24-2026-07-05 | sample-uniform-partial-register | 20 | 1024 | `mklq_metal_uniform_partial_register_sample_count_accumulation` | mixed-path Metal marginal probability fill with uniform-probability generated-count fast path for counts-only partial-register sample-count accumulation | benchmark_harness_static_case_map |
| local-metal-uniform-partial-sampling-q20-q24-2026-07-05 | sample-uniform-partial-register | 20 | 8192 | `mklq_metal_uniform_partial_register_sample_count_accumulation` | mixed-path Metal marginal probability fill with uniform-probability generated-count fast path for counts-only partial-register sample-count accumulation | benchmark_harness_static_case_map |
| local-metal-uniform-partial-sampling-q20-q24-2026-07-05 | sample-uniform-partial-register | 20 | 65536 | `mklq_metal_uniform_partial_register_sample_count_accumulation` | mixed-path Metal marginal probability fill with uniform-probability generated-count fast path for counts-only partial-register sample-count accumulation | benchmark_harness_static_case_map |
| local-metal-uniform-partial-sampling-q20-q24-2026-07-05 | sample-uniform-partial-register | 22 | 256 | `mklq_metal_uniform_partial_register_sample_count_accumulation` | mixed-path Metal marginal probability fill with uniform-probability generated-count fast path for counts-only partial-register sample-count accumulation | benchmark_harness_static_case_map |
| local-metal-uniform-partial-sampling-q20-q24-2026-07-05 | sample-uniform-partial-register | 22 | 1024 | `mklq_metal_uniform_partial_register_sample_count_accumulation` | mixed-path Metal marginal probability fill with uniform-probability generated-count fast path for counts-only partial-register sample-count accumulation | benchmark_harness_static_case_map |
| local-metal-uniform-partial-sampling-q20-q24-2026-07-05 | sample-uniform-partial-register | 22 | 8192 | `mklq_metal_uniform_partial_register_sample_count_accumulation` | mixed-path Metal marginal probability fill with uniform-probability generated-count fast path for counts-only partial-register sample-count accumulation | benchmark_harness_static_case_map |
| local-metal-uniform-partial-sampling-q20-q24-2026-07-05 | sample-uniform-partial-register | 22 | 65536 | `mklq_metal_uniform_partial_register_sample_count_accumulation` | mixed-path Metal marginal probability fill with uniform-probability generated-count fast path for counts-only partial-register sample-count accumulation | benchmark_harness_static_case_map |
| local-metal-uniform-partial-sampling-q20-q24-2026-07-05 | sample-uniform-partial-register | 24 | 256 | `mklq_metal_uniform_partial_register_sample_count_accumulation` | mixed-path Metal marginal probability fill with uniform-probability generated-count fast path for counts-only partial-register sample-count accumulation | benchmark_harness_static_case_map |
| local-metal-uniform-partial-sampling-q20-q24-2026-07-05 | sample-uniform-partial-register | 24 | 1024 | `mklq_metal_uniform_partial_register_sample_count_accumulation` | mixed-path Metal marginal probability fill with uniform-probability generated-count fast path for counts-only partial-register sample-count accumulation | benchmark_harness_static_case_map |
| local-metal-uniform-partial-sampling-q20-q24-2026-07-05 | sample-uniform-partial-register | 24 | 8192 | `mklq_metal_uniform_partial_register_sample_count_accumulation` | mixed-path Metal marginal probability fill with uniform-probability generated-count fast path for counts-only partial-register sample-count accumulation | benchmark_harness_static_case_map |
| local-metal-uniform-partial-sampling-q20-q24-2026-07-05 | sample-uniform-partial-register | 24 | 65536 | `mklq_metal_uniform_partial_register_sample_count_accumulation` | mixed-path Metal marginal probability fill with uniform-probability generated-count fast path for counts-only partial-register sample-count accumulation | benchmark_harness_static_case_map |

Regenerate with:

```bash
python3 benchmarks/mklq/summarize_reports.py \
  --reports benchmarks/mklq/reports \
  --format markdown \
  --output docs/mklq/benchmark-evidence.md
```
