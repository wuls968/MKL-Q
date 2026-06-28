# MKL-Q Benchmark Evidence

This file is generated from sanitized benchmark summaries under `benchmarks/mklq/reports/`.

Caveat: these entries are local benchmark evidence from development or release-prep runs. Interpret each entry through its `evidence_kind` and `interpretation` fields; none is a cross-machine performance certification.

## Evidence Inventory

| Summary ID | Kind | Machine | Targets | Cases | Qubits | Run shape | Rows | Raw evidence |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| local-clean-cpu-q20-2026-06-21 | clean_local_benchmark_evidence | Apple M5, 10 logical cores, 16 GiB RAM, macOS 26.5.1 | qpp-cpu, mklq-cpu | y-state, cy-state, cz-state, qft-like-state, seeded-clifford-state, sample-full-register, sample-partial-register | 20 | shot_counts=1024, 65536; repeats=2; warmups=1; layers=8; isolate_rows=true | ok=18 | benchmarks/mklq/results/local-clean-cpu-gate-y-cy-cz-q20-2026-06-21.json sha256=2b438094b63b; benchmarks/mklq/results/local-clean-cpu-composite-qft-like-seeded-clifford-q20-2026-06-21.json sha256=b07b3ba92b83; benchmarks/mklq/results/local-clean-cpu-sampling-q20-2026-06-21.json sha256=167b5c4adef8 |
| local-clean-cpu-q20-2026-06-28 | clean_local_benchmark_evidence | Apple M5, 10 logical cores, 16 GiB RAM, macOS 26.5.1 | qpp-cpu, mklq-cpu | y-state, cy-state, cz-state, qft-like-state, seeded-clifford-state, hardware-efficient-ansatz-state, sample-full-register, sample-partial-register | 20 | shot_counts=1024, 65536; repeats=2; warmups=1; layers=8; isolate_rows=true | ok=20 | benchmarks/mklq/results/local-clean-cpu-gate-y-cy-cz-q20-2026-06-28.json sha256=fd46266986bd; benchmarks/mklq/results/local-clean-cpu-composite-qft-like-seeded-clifford-hardware-efficient-ansatz-q20-2026-06-28.json sha256=3fa15408e149; benchmarks/mklq/results/local-clean-cpu-sampling-q20-2026-06-28.json sha256=c17703381afc |
| local-counts-only-sampling-shot-scaling-q20-2026-06-19 | local_tuning_evidence | Apple M5, 10 logical cores, 16 GiB RAM, macOS 26.5.1 | qpp-cpu, mklq-cpu, mklq-metal | sample-full-register, sample-partial-register | 20 | shot_counts=256, 1024, 8192, 65536; repeats=2; warmups=1; layers=8; isolate_rows=true | ok=24 | benchmarks/mklq/results/local-counts-only-sampling-shot-scaling-q20-2026-06-19.json sha256=ef9846673b46 |
| local-crz-distance-sweep-cpu-q20-2026-06-22 | clean_local_benchmark_evidence | Apple M5, 10 logical cores, 16 GiB RAM, macOS 26.5.1 | qpp-cpu, mklq-cpu | crz-distance-sweep-state | 20 | shot_counts=1024; repeats=2; warmups=1; layers=8; isolate_rows=true | ok=38 | benchmarks/mklq/results/local-clean-cpu-crz-distance-sweep-q20-2026-06-22.json sha256=64f9d0b4f709 |
| local-current-sampling-fullprob-gated-q20-2026-06-19 | local_tuning_evidence | Apple M5, 10 logical cores, 16 GiB RAM, macOS 26.5.1 | qpp-cpu, mklq-cpu, mklq-metal | sample-full-register, sample-partial-register | 20 | shots=1024; repeats=2; warmups=1; layers=4; isolate_rows=true | ok=6 | benchmarks/mklq/results/local-current-sampling-fullprob-gated-q20-2026-06-19.json sha256=8ca6a4f7a7ae; benchmarks/mklq/results/local-current-sampling-shot-scaling-q20-2026-06-19.json sha256=9c15c0c1d566 |
| local-metal-composite-mixed-path-q20-2026-06-21 | local_tuning_evidence | Apple M5, 10 logical cores, 16 GiB RAM, macOS 26.5.1 | qpp-cpu, mklq-cpu, mklq-metal | qft-like-state, seeded-clifford-state | 20 | shot_counts=1024; repeats=2; warmups=1; layers=8; isolate_rows=true | ok=6 | benchmarks/mklq/results/local-metal-composite-mixed-path-q20-2026-06-21.json sha256=ef58b5922221 |
| local-metal-path-labels-q20-2026-06-22 | local_tuning_evidence | Apple M5, 10 logical cores, 16 GiB RAM, macOS 26.5.1 | mklq-metal | y-state, cy-state, qft-like-state, seeded-clifford-state, sample-full-register, sample-partial-register | 20 | shot_counts=1024, 65536; repeats=2; warmups=1; layers=8; isolate_rows=true | ok=8 | benchmarks/mklq/results/local-metal-path-labels-state-q20-2026-06-22.json sha256=5c44e7772c48; benchmarks/mklq/results/local-metal-path-labels-sampling-q20-2026-06-22.json sha256=0087e0be2ca9 |
| local-metal-three-qubit-resident-q20-2026-06-22 | local_tuning_evidence | Apple M5, 10 logical cores, 16 GiB RAM, macOS 26.5.1 | qpp-cpu, mklq-cpu, mklq-metal | three-qubit-state | 20 | shot_counts=1024; repeats=2; warmups=1; layers=8; isolate_rows=true | ok=3 | benchmarks/mklq/results/local-metal-three-qubit-resident-q20-2026-06-22.json sha256=daed4c1deb2d |
| local-metal-y-cy-resident-isolated-q20-2026-06-19 | local_tuning_evidence | Apple M5, 10 logical cores, 16 GiB RAM, macOS 26.5.1 | qpp-cpu, mklq-cpu, mklq-metal | y-state, cy-state | 20 | shots=1024; repeats=2; warmups=1; layers=8; isolate_rows=true | ok=6 | benchmarks/mklq/results/local-metal-y-cy-resident-isolated-q20-2026-06-19.json sha256=84891e8f907c |
| local-multi-control-cpu-q20-2026-06-22 | clean_local_benchmark_evidence | Apple M5, 10 logical cores, 16 GiB RAM, macOS 26.5.1 | qpp-cpu, mklq-cpu | multi-control-state | 20 | shot_counts=1024; repeats=5; warmups=2; layers=8; isolate_rows=true | ok=2 | benchmarks/mklq/results/local-clean-cpu-multi-control-q20-2026-06-22.json sha256=6c483e023c90 |
| local-sampling-profile-breakdown-cpu-q20-q22-2026-06-23 | local_tuning_evidence | Apple M5, 10 logical cores, 16 GiB RAM, macOS 26.5.1 | mklq-cpu | sample-full-register, sample-partial-register | 20, 22 | shot_counts=1024, 65536; repeats=2; warmups=1; layers=8; isolate_rows=true; profile_sampling_breakdown=true | ok=8 | benchmarks/mklq/results/local-sampling-profile-breakdown-cpu-q20-q22-2026-06-23.json sha256=9821493c7e24 |
| local-sampling-scaling-cpu-q18-q22-2026-06-23 | clean_local_benchmark_evidence | Apple M5, 10 logical cores, 16 GiB RAM, macOS 26.5.1 | qpp-cpu, mklq-cpu | sample-full-register, sample-partial-register | 18, 20, 22 | shot_counts=1024, 65536; repeats=2; warmups=1; layers=8; isolate_rows=true | ok=24 | benchmarks/mklq/results/local-sampling-scaling-cpu-q18-q22-2026-06-23.json sha256=d0fc30326ee6 |
| local-scaling-cpu-multi-control-q18-q22-2026-06-22 | clean_local_benchmark_evidence | Apple M5, 10 logical cores, 16 GiB RAM, macOS 26.5.1 | qpp-cpu, mklq-cpu | multi-control-state | 18, 20, 22 | shot_counts=1024; repeats=3; warmups=1; layers=8; isolate_rows=true | ok=6 | benchmarks/mklq/results/local-scaling-cpu-multi-control-q18-q22-2026-06-22.json sha256=be97c9f00a75 |
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
| local-current-sampling-fullprob-gated-q20-2026-06-19 | `pre_gate_probe.mklq_metal_sample_full_register_q20_seconds` | 0.0313161 s |
| local-current-sampling-fullprob-gated-q20-2026-06-19 | `pre_gate_probe.mklq_metal_sample_partial_register_q20_seconds` | 0.255697 s |
| local-current-sampling-fullprob-gated-q20-2026-06-19 | `pre_gate_probe.repeats` | 1 |
| local-current-sampling-fullprob-gated-q20-2026-06-19 | `pre_gate_probe.shots` | 1024 |
| local-current-sampling-fullprob-gated-q20-2026-06-19 | `same_day_tuning_ratio.mklq_metal_sample_partial_register_pre_gate_over_cost_gate` | 11.62x |
| local-metal-composite-mixed-path-q20-2026-06-21 | `mklq_metal_elapsed_seconds_median.qft_like_state_q20` | 1.21681 s |
| local-metal-composite-mixed-path-q20-2026-06-21 | `mklq_metal_elapsed_seconds_median.seeded_clifford_state_q20` | 0.189254 s |
| local-metal-composite-mixed-path-q20-2026-06-21 | `same_day_cross_target_ratio.qpp_cpu_over_mklq_metal_qft_like_state_q20` | 56.00x |
| local-metal-composite-mixed-path-q20-2026-06-21 | `same_day_cross_target_ratio.qpp_cpu_over_mklq_metal_seeded_clifford_state_q20` | 70.50x |
| local-metal-path-labels-q20-2026-06-22 | `mklq_metal_elapsed_seconds_median.cy_state_q20` | 0.195022 s |
| local-metal-path-labels-q20-2026-06-22 | `mklq_metal_elapsed_seconds_median.qft_like_state_q20` | 1.33702 s |
| local-metal-path-labels-q20-2026-06-22 | `mklq_metal_elapsed_seconds_median.sample_full_register_q20_1024_shots` | 0.0299139 s |
| local-metal-path-labels-q20-2026-06-22 | `mklq_metal_elapsed_seconds_median.sample_full_register_q20_65536_shots` | 0.0317822 s |
| local-metal-path-labels-q20-2026-06-22 | `mklq_metal_elapsed_seconds_median.sample_partial_register_q20_1024_shots` | 0.059596 s |
| local-metal-path-labels-q20-2026-06-22 | `mklq_metal_elapsed_seconds_median.sample_partial_register_q20_65536_shots` | 0.0324173 s |
| local-metal-path-labels-q20-2026-06-22 | `mklq_metal_elapsed_seconds_median.seeded_clifford_state_q20` | 0.164919 s |
| local-metal-path-labels-q20-2026-06-22 | `mklq_metal_elapsed_seconds_median.y_state_q20` | 0.14844 s |
| local-metal-three-qubit-resident-q20-2026-06-22 | `mklq_metal_elapsed_seconds_median.three_qubit_state_q20` | 0.155606 s |
| local-metal-three-qubit-resident-q20-2026-06-22 | `same_day_cross_target_ratio.qpp_cpu_over_mklq_metal_three_qubit_state_q20` | 54.67x |
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
| local-scaling-cpu-multi-control-q18-q22-2026-06-22 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_multi_control_state_q18` | 11.66x |
| local-scaling-cpu-multi-control-q18-q22-2026-06-22 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_multi_control_state_q20` | 28.00x |
| local-scaling-cpu-multi-control-q18-q22-2026-06-22 | `clean_worktree_cross_target_ratio.qpp_cpu_over_mklq_cpu_multi_control_state_q22` | 72.93x |
| local-scaling-cpu-multi-control-q18-q22-2026-06-22 | `mklq_cpu_elapsed_seconds_median.multi_control_state_q18` | 0.163252 s |
| local-scaling-cpu-multi-control-q18-q22-2026-06-22 | `mklq_cpu_elapsed_seconds_median.multi_control_state_q20` | 0.277172 s |
| local-scaling-cpu-multi-control-q18-q22-2026-06-22 | `mklq_cpu_elapsed_seconds_median.multi_control_state_q22` | 0.771644 s |
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

| Summary ID | Case | Shots | Label | Scope | Source |
| --- | --- | --- | --- | --- | --- |
| local-metal-path-labels-q20-2026-06-22 | y-state | 1024 | `mklq_metal_resident_single_gate_state_host_readback` | resident fp32 Metal single-target gate update followed by host readback for cudaq.get_state | benchmark_harness_static_case_map |
| local-metal-path-labels-q20-2026-06-22 | cy-state | 1024 | `mklq_metal_resident_controlled_gate_state_host_readback` | resident fp32 Metal controlled gate update followed by host readback for cudaq.get_state | benchmark_harness_static_case_map |
| local-metal-path-labels-q20-2026-06-22 | qft-like-state | 1024 | `mklq_metal_mixed_composite_state_host_readback` | experimental mklq-metal mixed-path composite state-vector update followed by host readback for cudaq.get_state | benchmark_harness_static_case_map |
| local-metal-path-labels-q20-2026-06-22 | seeded-clifford-state | 1024 | `mklq_metal_mixed_composite_state_host_readback` | experimental mklq-metal mixed-path composite state-vector update followed by host readback for cudaq.get_state | benchmark_harness_static_case_map |
| local-metal-path-labels-q20-2026-06-22 | sample-full-register | 1024 | `mklq_metal_mixed_sampling_host_counts` | mixed-path Metal probability fill with host-side sample draw/count accumulation | benchmark_harness_static_case_map |
| local-metal-path-labels-q20-2026-06-22 | sample-full-register | 65536 | `mklq_metal_mixed_sampling_host_counts` | mixed-path Metal probability fill with host-side sample draw/count accumulation | benchmark_harness_static_case_map |
| local-metal-path-labels-q20-2026-06-22 | sample-partial-register | 1024 | `mklq_metal_mixed_sampling_host_counts` | mixed-path Metal probability fill with host-side sample draw/count accumulation | benchmark_harness_static_case_map |
| local-metal-path-labels-q20-2026-06-22 | sample-partial-register | 65536 | `mklq_metal_mixed_sampling_host_counts` | mixed-path Metal probability fill with host-side sample draw/count accumulation | benchmark_harness_static_case_map |
| local-metal-three-qubit-resident-q20-2026-06-22 | three-qubit-state | 1024 | `mklq_metal_resident_three_gate_state_host_readback` | resident fp32 Metal three-target gate update followed by host readback for cudaq.get_state | benchmark_harness_static_case_map |

Regenerate with:

```bash
python3 benchmarks/mklq/summarize_reports.py \
  --reports benchmarks/mklq/reports \
  --format markdown \
  --output docs/mklq/benchmark-evidence.md
```
