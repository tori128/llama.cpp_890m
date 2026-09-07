# llama.cpp

![llama](https://raw.githubusercontent.com/ggml-org/llama.brand/refs/heads/master/cover/llama-cpp/cover-llama-cpp-dark.svg)

<div align="center">

<b>LLM inference in C/C++</b>

[![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg)](https://opensource.org/licenses/MIT)
[![Release](https://img.shields.io/github/v/release/ggml-org/llama.cpp)](https://github.com/ggml-org/llama.cpp/releases)
[![Server](https://github.com/ggml-org/llama.cpp/actions/workflows/server.yml/badge.svg)](https://github.com/ggml-org/llama.cpp/actions/workflows/server.yml)
[![Docker](https://github.com/ggml-org/llama.cpp/actions/workflows/docker.yml/badge.svg)](https://github.com/ggml-org/llama.cpp/actions/workflows/docker.yml)
[![Winget](https://github.com/ggml-org/llama.cpp/actions/workflows/winget.yml/badge.svg)](https://github.com/ggml-org/llama.cpp/actions/workflows/winget.yml)

[manifesto](https://github.com/ggml-org/llama.cpp/discussions/205) / [ggml](https://github.com/ggml-org/ggml) / [ops](https://github.com/ggml-org/llama.cpp/blob/master/docs/ops.md) / [maintainer PRs](https://github.com/ggml-org/llama.cpp/issues?q=is%3Apr%20is%3Aopen%20draft%3AFalse%20(author%3Argerganov%20OR%20author%3AKitaitiMakoto%20OR%20author%3Adanbev%20OR%20author%3Aaldehir%20OR%20author%3Amax-krasnyansky%20OR%20author%3ACISC%20OR%20author%3Aggerganov%20OR%20author%3Aam17an%20OR%20author%3Abartowski1182%20OR%20author%3Ahipudding%20OR%20author%3AServeurpersoCom%20OR%20author%3Apwilkin%20OR%20author%3Areeselevine%20OR%20author%3Angxson%20OR%20author%3Ajeffbolznv%20OR%20author%3A0cc4m%20OR%20author%3Aangt%20OR%20author%3AIMbackK%20OR%20author%3Aarthw%20OR%20author%3AJohannesGaessler%20OR%20author%3AORippler%20OR%20author%3Aruixiang63%20OR%20author%3Axctan%20OR%20author%3Aallozaur%20OR%20author%3Ayomaytk%20OR%20author%3Aaendk%20OR%20author%3Agaugarg-nv%20OR%20author%3Ataronaeo%20OR%20author%3Aforforever73%20OR%20author%3Alhez%20OR%20author%3Anetrunnereve%20OR%20author%3Afairydreaming)%20sort%3Aupdated-desc) / [compile times](https://github.com/ggml-org/llama.cpp-dev/blob/master/README-compile-times.md) / [lib llama API](https://github.com/ggml-org/llama.cpp/issues/9289) / [llama-server REST API](https://github.com/ggml-org/llama.cpp/issues/9291)

</div>

## Quick start

A few options to get `llama.cpp` installed on your machine:

- Visit https://llama.app and follow the instructions
- Run with Docker - see our [Docker documentation](docs/docker.md)
- Download pre-built binaries from the [releases page](https://github.com/ggml-org/llama.cpp/releases)
- Build from source by cloning this repository - check out [our build guide](docs/build.md)

Once installed:

```sh
# Download and run a model directly from Hugging Face
llama cli -hf ggml-org/Qwen3.5-0.8B-GGUF

# Launch OpenAI-compatible API server
llama serve -hf ggml-org/Qwen3.5-0.8B-GGUF
```

<table align="center">
    <tr>
        <td align="center" width=50%>
            <img width="1310" height="888" alt="VLM session with `llama cli`" src="https://github.com/user-attachments/assets/88726b48-1713-48aa-a525-95a02e78afc4" />
            <i>VLM session with <b>llama cli</b></i>
        </td>
        <td align="center">
            <img width="1392" height="958" alt="Built-in web UI against `llama serve` running Qwen 3.6" src="https://github.com/user-attachments/assets/b402f972-2e32-4def-8771-8d849f08cf2e" />
            <i>Built-in web UI against <b>llama serve</b></i>
        </td>
    </tr>
<table>

## Flash-Next Vulkan profile (this fork)

This profile is for Qwen3.8 Flash-Next IQ4_XS-M64 and a matching Q4_K_M MTP
draft. Download the [unified release](https://github.com/tori128/llama.cpp_890m/releases/tag/unified-ece817752), keep all 28 target GGUF shards in one directory, and pass
`...-00001-of-00028.gguf` to `--model`.

The values below reproduce the Linux Vulkan shortcut used for the release
benchmark on an AMD Ryzen AI 9 HX 370. `--threads`, `--threads-batch`, and
`LLAMA_QSA_GATHER=16384` are hardware-specific tuning values; measure them
before using them on another system.

### Linux Vulkan

Run this from the unpacked Linux Vulkan release directory. Replace the three
path variables with local paths.

```sh
release_dir=/path/to/llama-unified-linux-vulkan
model=/path/to/Qwen3.8-Flash-Next-AD-3.84bpw-IQ4_XS-M64-00001-of-00028.gguf
draft=/path/to/Qwen3.8-Flash-Next-MTP-Q4_K_M.gguf

export LD_LIBRARY_PATH="$release_dir"
export LLAMA_MMAP_RANDOM=1
export LLAMA_PLE_HOST_GATHER=1
export LLAMA_ATTN_ROT_DISABLE=1
export LLAMA_QSA_GATHER=16384

"$release_dir/llama-server" \
  --model "$model" \
  --gpu-layers 99 --n-cpu-moe 0 --flash-attn on \
  --load-mode mmap --no-host --no-repack --fit off \
  --ctx-size 100000 --parallel 1 --threads 10 --threads-batch 10 \
  --cache-prompt --cache-ram 2048 --ctx-checkpoints 4 \
  --checkpoint-min-step 8192 --cache-reuse 0 \
  --batch-size 2048 --ubatch-size 512 \
  --cache-type-k q8_0 --cache-type-v q8_0 --kv-unified \
  --jinja --reasoning on --reasoning-effort low --reasoning-preserve \
  --temp 1.0 --top-p 0.95 --top-k 20 --min-p 0.0 \
  --presence-penalty 0.0 --repeat-penalty 1.0 \
  --alias qwen3.8-flash-next --metrics --host 0.0.0.0 --port 1234 \
  --spec-type draft-mtp,ngram-mod --spec-draft-model "$draft" \
  --spec-draft-n-max 3 --spec-draft-n-min 0 --spec-draft-p-min 0.75 \
  --spec-draft-ngl all --spec-draft-type-k q8_0 --spec-draft-type-v q8_0
```

### Windows Vulkan (PowerShell)

Extract the Windows Vulkan archive and retain its DLLs beside
`llama-server.exe`. `LD_LIBRARY_PATH` is Linux-only.

```powershell
$releaseDir = 'C:\path\to\llama-unified-windows-vulkan'
$model = 'C:\path\to\Qwen3.8-Flash-Next-AD-3.84bpw-IQ4_XS-M64-00001-of-00028.gguf'
$draft = 'C:\path\to\Qwen3.8-Flash-Next-MTP-Q4_K_M.gguf'

$env:LLAMA_MMAP_RANDOM = '1'
$env:LLAMA_PLE_HOST_GATHER = '1'
$env:LLAMA_ATTN_ROT_DISABLE = '1'
$env:LLAMA_QSA_GATHER = '16384'

& "$releaseDir\llama-server.exe" `
  --model "$model" `
  --gpu-layers 99 --n-cpu-moe 0 --flash-attn on `
  --load-mode mmap --no-host --no-repack --fit off `
  --ctx-size 100000 --parallel 1 --threads 10 --threads-batch 10 `
  --cache-prompt --cache-ram 2048 --ctx-checkpoints 4 `
  --checkpoint-min-step 8192 --cache-reuse 0 `
  --batch-size 2048 --ubatch-size 512 `
  --cache-type-k q8_0 --cache-type-v q8_0 --kv-unified `
  --jinja --reasoning on --reasoning-effort low --reasoning-preserve `
  --temp 1.0 --top-p 0.95 --top-k 20 --min-p 0.0 `
  --presence-penalty 0.0 --repeat-penalty 1.0 `
  --alias qwen3.8-flash-next --metrics --host 0.0.0.0 --port 1234 `
  --spec-type draft-mtp,ngram-mod --spec-draft-model "$draft" `
  --spec-draft-n-max 3 --spec-draft-n-min 0 --spec-draft-p-min 0.75 `
  --spec-draft-ngl all --spec-draft-type-k q8_0 --spec-draft-type-v q8_0
```

> [!WARNING]
> Windows supports this Flash-Next MTP and `ngram-mod` profile, but
> `--load-mode mmap` is memory-mapped I/O, not direct SSD I/O. This fork does
> not explicitly release GPU-uploaded source-mapping pages or accessed PLE
> pages on Windows, so system-memory use can grow during inference.
> `--no-host` bypasses an additional GPU host buffer; it does not limit PLE
> residency. Leave sufficient system and shared-GPU memory headroom for long
> contexts.

On Linux Vulkan integrated GPUs, this fork explicitly releases source-mapping
pages after GPU upload. `LLAMA_MMAP_RANDOM=1` handles sparse PLE access and row
prefetch; it does not provide a Windows memory-residency limit.

## Description

The main goal of `llama.cpp` is to enable LLM (and VLM) inference with minimal setup and state-of-the-art performance on
a wide range of hardware - locally and in the cloud.

- Plain C/C++ implementation without any dependencies
- Apple silicon is a first-class citizen - optimized via ARM NEON, Accelerate and Metal frameworks
- AVX, AVX2, AVX512 and AMX support for x86 architectures
- RVV, ZVFH, ZFH, ZICBOP and ZIHINTPAUSE support for RISC-V architectures
- 1.5-bit, 2-bit, 3-bit, 4-bit, 5-bit, 6-bit, and 8-bit integer quantization for faster inference and reduced memory use
- Custom CUDA kernels for running LLMs on NVIDIA GPUs (support for AMD GPUs via HIP and Moore Threads GPUs via MUSA)
- Vulkan and SYCL backend support
- CPU+GPU hybrid inference to partially accelerate models larger than the total VRAM capacity

The `llama.cpp` project is build on top of the [ggml](https://github.com/ggml-org/ggml) library.

## Supported backends

| Backend | Target devices |
| --- | --- |
| [BLAS](docs/build.md#blas-build) | All |
| [BLIS](docs/backend/BLIS.md) | All |
| [CANN](docs/build.md#cann) | Ascend NPU |
| [CUDA](docs/build.md#cuda) | Nvidia GPU |
| [HIP](docs/build.md#hip) | AMD GPU |
| [Hexagon [In Progress]](docs/backend/snapdragon/README.md) | Snapdragon |
| [IBM zDNN](docs/backend/zDNN.md) | IBM Z & LinuxONE |
| [MUSA](docs/build.md#musa) | Moore Threads GPU |
| [Metal](docs/build.md#metal-build) | Apple Silicon |
| [OpenCL](docs/backend/OPENCL.md) | Adreno GPU |
| [OpenVINO [In Progress]](docs/backend/OPENVINO.md) | Intel CPUs, GPUs, and NPUs |
| [RPC](https://github.com/ggml-org/llama.cpp/tree/master/tools/rpc) | All |
| [SYCL](docs/backend/SYCL.md) | Intel GPU |
| [VirtGPU](docs/backend/VirtGPU.md) | VirtGPU APIR |
| [Vulkan](docs/build.md#vulkan) | GPU |
| [WebGPU](docs/build.md#webgpu) | All |
| [ZenDNN](docs/build.md#zendnn) | AMD CPU |

## Documentation

#### Tools

- [cli](tools/cli/README.md)
- [completion](tools/completion/README.md)
- [server](tools/server/README.md)
- [GBNF grammars](grammars/README.md)

#### Development

- [How to build](docs/build.md)
- [Running on Docker](docs/docker.md)
- [Build on Android](docs/android.md)
- [Multi-GPU usage](docs/multi-gpu.md)
- [Performance troubleshooting](docs/development/token_generation_performance_tips.md)
- [GGML tips & tricks](https://github.com/ggml-org/llama.cpp/wiki/GGML-Tips-&-Tricks)
- [XCFramework](docs/xcframework.md)
- [Completions](docs/completions.md)
- [Models](docs/models.md)
- [Release process](docs/release.md)

## Contributing

- Contributors can open PRs
- Collaborators will be invited based on contributions
- Maintainers can push to branches in the `llama.cpp` repo and merge PRs into the `master` branch
- Any help with managing issues, PRs and projects is very appreciated!
- Read the [CONTRIBUTING.md](CONTRIBUTING.md) for more information

## Acknowledgements

- [yhirose/cpp-httplib](https://github.com/yhirose/cpp-httplib) - Single-header HTTP server, used by `llama-server` - MIT license
- [stb-image](https://github.com/nothings/stb) - Single-header image format decoder, used by multimodal subsystem - Public domain
- [nlohmann/json](https://github.com/nlohmann/json) - Single-header JSON library, used by various tools/examples - MIT License
- [miniaudio.h](https://github.com/mackron/miniaudio) - Single-header audio format decoder, used by multimodal subsystem - Public domain
- [subprocess.h](https://github.com/sheredom/subprocess.h) - Single-header process launching solution for C and C++ - Public domain
