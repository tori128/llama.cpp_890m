## Benchmark environment

| Item | Value |
| --- | --- |
| CPU | AMD Ryzen AI 9 HX 370 |
| GPU | AMD Radeon 890M (gfx1150) |
| Memory | DDR5-6000 |
| Input | 32,768-token programming prompt |
| Output measurement | From generation start to natural EOS or 4,096 tokens; cold 32K -> 4K, then two cached 32K -> 4K runs |
| Cache / memory | Prompt cache enabled after the cold run; 0 KiB VMSwap limit |
| Comparison | Fork vs. upstream master with matched original runtime; current H2D vs. pre-H2D |

## Performance results

### Fork vs. upstream master

Existing cache-disabled 32K -> 4K measurements: unified `02a6b452a2b4` vs. upstream `67a17c17caa9`.

| Model | Backend | Draft method | Unified prefill | Upstream prefill | Prefill delta | Unified decode (4K) | Upstream decode (4K) | Decode delta |
| --- | --- | --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Qwen3.6 35B-A3B | ROCm 7.14 | MTP, n=2 | 368.62 tok/s | 356.92 tok/s | +3.28% | 20.55 tok/s | 16.06 tok/s | +27.95% |
| Qwen3.6 35B-A3B | Vulkan | MTP, n=2 | 351.38 tok/s | 262.31 tok/s | +33.96% | 19.31 tok/s | 15.73 tok/s | +22.75% |
| Qwen3.8 27B | Vulkan | DFlash2 Q8_0, n=4 | 86.23 tok/s | 75.58 tok/s | +14.09% | 6.72 tok/s | 6.55 tok/s | +2.45% |
| Qwen3.8 Flash-Next | Vulkan | MTP Q4_K_M, n=4 | 108.52 tok/s* | unavailable | -- | 6.66 tok/s* | unavailable | -- |

* Qwen3.8 Flash-Next was measured twice. The difference in 4K decode speed was 0.30%.

### Current H2D validation

`a0550d11b` vs. pre-H2D `e0a0d2d`; decode values are cold / cache 1 / cache 2.

| Model | Backend | Draft method | Unified prefill | Previous prefill | Prefill delta | Unified decode (4K) | Previous decode (4K) | Decode delta |
| --- | --- | --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Qwen3.6 35B-A3B | ROCm 7.13 | MTP, n=2 | 360.12 tok/s | 361.36 tok/s | -0.34% | 23.87 / 21.79 / 22.72 tok/s | 24.39 / 19.01 / 20.43 tok/s | -2.13% / +14.61% / +11.21% |
| Qwen3.8 27B | ROCm 7.14 | DFlash2 Q8_0, n=4 | 102.77 tok/s | 102.90 tok/s | -0.12% | 7.91 / 7.24 / 7.80 tok/s | 8.66 / 6.74 / 8.19 tok/s | -8.58% / +7.35% / -4.82% |

Cold-run GTT: Qwen3.6 896 -> 781 MiB; Qwen3.8 856 -> 739 MiB.

### ROCm startup compatibility

| Model | Previous ROCm | Current ROCm | Server-ready time |
| --- | --- | --- | ---: |
| Laguna | Not ready | Ready | 20.10 s |
| Ling-3.0 | Not ready | Ready | 14.93 s |

HIP iGPU mmap-prefetch suppression enables both models. Vulkan remains selected for decode speed.

## Change from previous release

| Model | Backend | Current prefill | Previous-release prefill | Prefill delta | Current decode (4K) | Previous-release decode (4K) | Decode delta |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Qwen3.6 35B-A3B | ROCm | 360.12 tok/s | unavailable | -- | 23.87 / 21.79 / 22.72 tok/s | unavailable | -- |
| Qwen3.8 27B | ROCm | 102.77 tok/s | unavailable | -- | 7.91 / 7.24 / 7.80 tok/s | unavailable | -- |

The previous release used a different configuration and measurement method; directly comparable values are unavailable.
