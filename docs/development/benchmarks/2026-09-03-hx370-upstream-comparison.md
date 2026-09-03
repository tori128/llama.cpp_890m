# Ryzen AI 9 HX 370: llama.cpp 本家版との速度比較

測定日: 2026-09-03

## 実行環境と方法

- CPU: AMD Ryzen AI 9 HX 370
- GPU: AMD Radeon 890M（gfx1150）
- メモリ転送レート: DDR5-6000 MT/s
- 現行統合版の実行形式: `02a6b452a2b41d57f93db6900e11ebecd7163b25`
- 本家版の実行形式: [llama.cpp `67a17c17caa95742186f8b1ecadd1b5abd6d5ebb`（2026-09-03）](https://github.com/ggml-org/llama.cpp/commit/67a17c17caa95742186f8b1ecadd1b5abd6d5ebb)
- 入力: チャットテンプレート適用後に 32,768 トークンとなる公開可能なPythonプログラミング課題
- 出力: 生成開始から自然EOS到達時または4,096トークン到達時まで。表の4K生成は4,096トークン到達時の累積値
- 実行: 実装ごとに別プロセス、同一seed、prompt cache無効、`cache_n=0`

## モデル別の起動条件

| モデル | バックエンド | 主モデル量子化 | speculative decoding | KV cache | context / batch / u-batch |
| --- | --- | --- | --- | --- | --- |
| Qwen3.6 35B-A3B | ROCm 7.14 | UD-Q4_K_M | MTP n=2 | q8_0 / q8_0 unified | 100,000 / 2048 / 512 |
| Qwen3.6 35B-A3B | Vulkan | UD-Q4_K_M | MTP n=2 | q8_0 / q8_0 unified | 100,000 / 2048 / 512 |
| Qwen3.8 27B | Vulkan | UD-Q4_K_M | DFlash2 Q8_0, n=4 | q8_0 / q8_0 unified | 100,000 / 2048 / 512 |
| Qwen3.8 Flash-Next | Vulkan | IQ4_XS-M64 | MTP + n-gram, n=4, p=0.75, Q4_K_M draft | q8_0 / q8_0 unified | 100,000 / 2048 / 512 |

Qwen3.6 ROCmでは `MMQ_ID_J=32` を使用した。Qwen3.8 Flash-Nextでは `LLAMA_MMAP_RANDOM=1`、`LLAMA_PLE_HOST_GATHER=1`、`LLAMA_ATTN_ROT_DISABLE=1`、`LLAMA_QSA_GATHER=16384` を使用した。全プロファイルでflash attentionを有効化した。

## 結果

| モデル / バックエンド | 現行統合版 prefill tok/s | 本家版 prefill tok/s | prefill差 | 現行統合版 4K生成 tok/s | 本家版 4K生成 tok/s | 4K生成差 | 状態 |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | --- |
| Qwen3.6 / ROCm | 368.62 | 356.92 | +3.28% | 20.55 | 16.06 | +27.95% | 両方4K到達 |
| Qwen3.6 / Vulkan | 351.38 | 262.31 | +33.96% | 19.31 | 15.73 | +22.75% | 両方4K到達 |
| Qwen3.8 27B / Vulkan | 86.23 | 75.58 | +14.09% | 6.72 | 6.55 | +2.45% | 両方4K到達 |
| Qwen3.8 Flash-Next / Vulkan | 108.52 | — | — | 6.66 | — | — | 本家版がMTP Q4_K_M draftを読み込めず |

Qwen3.8 Flash-Next の現行統合版は2回とも4Kへ到達した。prefillは 110.58 / 106.46 tok/s、4K生成は 6.65 / 6.67 tok/s、draft受理率は両試行とも 76.34% だった。4K生成速度の試行間差は 0.30% である。本家版は draftロード時に必要な `hc_attn_norm` テンソルを検出できず、同一の正規ショートカット構成では起動しなかった。このためFlash-Nextの本家版との差は算出していない。

この結果は記載したソフトウェア版、量子化、起動引数、HX 370の実行環境における測定値である。
