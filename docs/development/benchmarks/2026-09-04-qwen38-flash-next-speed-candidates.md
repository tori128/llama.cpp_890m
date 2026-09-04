# Qwen3.8 Flash-Next: speed candidate report

Date: 2026-09-04

## Scope

- Baseline: commit `61448a641`.
- Backend: Vulkan.
- Runtime configuration: the active Qwen3.8 Flash-Next configuration with MTP plus n-gram draft decoding (`nmax=4`), B2048/U512, and Q8 KV cache.
- Input: a tokenized programming task of exactly 32,768 tokens.
- Generation: one continuous 12,288-token response per build, with checkpoints at 4,096, 8,192, and 12,288 generated tokens.
- Execution order: baseline, then candidate.
- Controls: identical input token IDs and seed; prompt-cache reuse and allocation disabled; a new server process for each build.

Each build performs one 32K prefill.  The three generation rates below are derived from cumulative server timing at each checkpoint, so they represent consecutive 4K windows of the same response.

## Validation

Every reported baseline and candidate run reached the 12,288-token generation limit.  For each candidate, the generated token IDs, text output, and MTP draft counters were identical to the baseline.  The output-quality audit found no repeated long paragraphs.

## Measurements

Negative rate deltas mean the candidate is slower.  Positive elapsed-time deltas mean the candidate takes longer.

| Candidate | Prefill: baseline -> candidate tok/s | Prefill delta | TG 0-4K delta | TG 4-8K delta | TG 8-12K delta | Elapsed-time delta |
|---|---:|---:|---:|---:|---:|---:|
| [PR #27909](https://github.com/ggml-org/llama.cpp/pull/27909), `1037c022c` | 105.50 -> 100.41 | -4.83% | -2.66% | -1.43% | -0.89% | +2.13% |
| KV-cell bitmap, `e5fe69efb` | 105.85 -> 96.86 | -8.49% | -3.52% | -1.74% | -0.34% | +2.87% |
| Qwen4 indexer query cont removal, `7b19501a6` | 107.20 -> 97.12 | -9.40% | -3.94% | -1.01% | -1.35% | +3.19% |

## Earlier screened candidates

- PR #28023: three paired 32K plus 4K runs had unstable results.  The paired median prefill delta was -0.79%, and the candidate did not meet the prefill criterion.
- PR #28040: three paired 32K plus 4K runs had a prefill median delta of -1.06%, a generation median delta of -1.53%, and an elapsed-time median delta of +1.39%.
- PR #28190: the current source already contains an equivalent or more comprehensive GQA Flash Attention implementation.
- PR #28032 and PR #28123 were rejected by earlier local benchmarks and do not provide a positive speed result for this configuration.

## Decision

**Do not integrate any tested speed candidate.**

The three candidates measured with the continuous-generation method were slower for 32K prefill, every 4K generation window, and end-to-end elapsed time.  The active executable and shortcut remain unchanged.

## Publication hygiene

This report contains aggregate measurements only.  It excludes prompts, responses, commands, model paths, host identifiers, network addresses, timestamps, and raw logs.
