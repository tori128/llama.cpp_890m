## Benchmark environment

| Item | Value |
| --- | --- |
| CPU | AMD Ryzen AI 9 HX 370 |
| GPU | AMD Radeon 890M (gfx1150) |
| Memory | DDR5-6000 |
| Build toolchain | TheRock gfx1150 7.13 |
| Runtime | ROCm 7.14, identical for every compared server |
| Input | Model-specific 32,768-token programming prompt; token IDs and seed are identical within each comparison |
| Output measurement | 4,096 generated tokens; one `cold` 32K -> 4K run followed by two prompt-cache 32K -> 4K runs in the same server process |
| Cache / swap | Prompt cache enabled after `cold`; `MemorySwapMax=0`; server VmSwap was 0 KiB in every reported segment |
| GPU memory | GTT peak 720 MiB in every reported segment; GPU peak temperature range 78–95 °C |
| Comparison | H2D-enabled `a0550d11b` versus upstream master `4d9176092d`, H2D-before `e0a0d2d`, and previous-release `c21d8a925` with identical shortcut parameters |

## Performance results

TG cells use the order `cold / cache 1 / cache 2`.

| Model | Backend | Draft method | `a0550d11b` prefill | master prefill | Prefill delta | `a0550d11b` TG (4K) | master TG (4K) | TG delta |
| --- | --- | --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Qwen3.6 35B-A3B | ROCm | MTP, n=2 | 358.51 tok/s | 282.76 tok/s | +26.79% | 21.34 / 16.67 / 23.47 tok/s | 24.84 / 24.83 / 24.82 tok/s | -14.10% / -32.87% / -5.44% |
| Qwen3.8 27B | ROCm | DFlash2 Q8_0, n=4 | 101.52 tok/s | 88.86 tok/s | +14.25% | 7.66 / 6.94 / 7.32 tok/s | 6.06 / 6.22 / 5.97 tok/s | +26.26% / +11.57% / +22.47% |

## Change from previous release

The H2D validation and previous-release comparison use the same measurement conditions. `e0a0d2d` is the parent before CPU memory-to-GPU memory transfer (H2D) staging; `c21d8a925` is the previous release.

| Model | Reference | `a0550d11b` prefill | Reference prefill | Prefill delta | `a0550d11b` TG (4K) | Reference TG (4K) | TG delta |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Qwen3.6 35B-A3B | H2D-before `e0a0d2d` | 358.51 tok/s | 359.98 tok/s | -0.41% | 21.34 / 16.67 / 23.47 tok/s | 22.37 / 20.47 / 18.62 tok/s | -4.62% / -18.56% / +26.06% |
| Qwen3.6 35B-A3B | Previous release `c21d8a925` | 358.51 tok/s | 360.47 tok/s | -0.54% | 21.34 / 16.67 / 23.47 tok/s | 25.60 / 23.90 / 18.12 tok/s | -16.65% / -30.24% / +29.51% |
| Qwen3.8 27B | H2D-before `e0a0d2d` | 101.52 tok/s | 101.66 tok/s | -0.14% | 7.66 / 6.94 / 7.32 tok/s | 7.67 / 6.58 / 7.56 tok/s | -0.24% / +5.51% / -3.23% |
| Qwen3.8 27B | Previous release `c21d8a925` | 101.52 tok/s | 101.40 tok/s | +0.12% | 7.66 / 6.94 / 7.32 tok/s | 7.26 / 6.25 / 7.29 tok/s | +5.41% / +11.03% / +0.31% |

The H2D comparison has both positive and negative TG deltas; neither model has all reported metrics below its H2D-before reference.

## ROCm startup compatibility

| Model | H2D-before `e0a0d2d` | H2D-enabled `a0550d11b` | Server-ready time |
| --- | --- | --- | ---: |
| Laguna | Server not ready | Ready | 20.10 s |
| Ling-3.0 | Server not ready | Ready | 14.93 s |

HIP iGPU mmap-prefetch suppression enables both startup paths. Vulkan remains the selected decode backend for Laguna and Ling-3.0 because their ROCm decode speed was lower.
