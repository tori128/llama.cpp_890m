## Benchmark environment

| Item | Value |
| --- | --- |
| CPU | AMD Ryzen AI 9 HX 370 |
| GPU | AMD Radeon 890M (gfx1150) |
| Memory | DDR5-6000 |
| Input | 32,768-token programming prompt |
| Output measurement | One cold 32K -> 4K run followed by two prompt-cache 32K -> 4K runs |
| Cache | Enabled after the cold run for the two repeated TG measurements |
| Comparison | Current unified build vs. the pre-change e0a0d2d build with identical model, token array, seed, and runtime parameters |

## Performance results

The three decode values in each cell are cold / cache 1 / cache 2. Positive deltas are faster.

| Model | Backend | Draft method | Unified prefill | Previous prefill | Prefill delta | Unified decode (4K) | Previous decode (4K) | Decode delta |
| --- | --- | --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Qwen3.6 35B-A3B | ROCm 7.13 | MTP, n=2 | 360.12 tok/s | 361.36 tok/s | -0.34% | 23.87 / 21.79 / 22.72 tok/s | 24.39 / 19.01 / 20.43 tok/s | -2.13% / +14.61% / +11.21% |
| Qwen3.8 27B | ROCm 7.14 | DFlash2 Q8_0, n=4 | 102.77 tok/s | 102.90 tok/s | -0.12% | 7.91 / 7.24 / 7.80 tok/s | 8.66 / 6.74 / 8.19 tok/s | -8.58% / +7.35% / -4.82% |

The Qwen3.6 token array SHA-256 was `2e10e433bdb384ff1716e8c411ef8cfc9d6f2892b1cbc348e0dbac296d0605d4`. The Qwen3.8 token array SHA-256 was `da077f0287ce2f288c79e3295ccc2e2dd40f9efba1fa3d156aded42a68418b34`.

Each measured process had a 0 KiB VMSwap limit. H2D staging reduced the Qwen3.6 cold-run GTT peak from about 896 MiB to about 781 MiB, and the Qwen3.8 cold-run GTT peak from about 856 MiB to about 739 MiB.

## ROCm startup compatibility

The previous ROCm configuration did not reach server-ready state while loading the Laguna and Ling-3.0 shortcut-equivalent configurations. With HIP iGPU mmap-prefetch suppression, both configurations reached server-ready state.

| Model | Backend | Previous result | Current result | Server-ready time |
| --- | --- | --- | --- | ---: |
| Laguna | ROCm | Did not reach server-ready state | Ready | 20.10 s |
| Ling-3.0 | ROCm | Did not reach server-ready state | Ready | 14.93 s |

The times use the prefetch-only result because H2D staging does not determine startup compatibility. ROCm decode performance was not preferable for either model, so their desktop shortcuts continue to use Vulkan.

## Change from previous release

| Model | Backend | Current prefill | Previous-release prefill | Prefill delta | Current decode (4K) | Previous-release decode (4K) | Decode delta |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Qwen3.6 35B-A3B | ROCm | 360.12 tok/s | unavailable | -- | 23.87 / 21.79 / 22.72 tok/s | unavailable | -- |
| Qwen3.8 27B | ROCm | 102.77 tok/s | unavailable | -- | 7.91 / 7.24 / 7.80 tok/s | unavailable | -- |

The prior public release used different runtime configurations and did not contain the same three-interval continuous measurement. Its values are therefore not directly comparable. The Performance results table gives the exact pre-change comparison for this release.
