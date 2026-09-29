# Release process

llama.cpp uses [semantic versioning](https://semver.org) (`MAJOR.MINOR.PATCH`).

## Version bump guidelines

| Change type | Version component |
|---|---|
| Breaking change to the public C API (`include/llama.h`)         | `MAJOR` |
| Backward-compatible features, model support, or API addition    | `MINOR` |
| Bug fix with no API change                                      | `PATCH` |

The version is set in the three variables at the top of the root `CMakeLists.txt`:

```cmake
set(LLAMA_VERSION_MAJOR 0)
set(LLAMA_VERSION_MINOR 1)
set(LLAMA_VERSION_PATCH 0)
```

_A version bump should be included in the PR that introduces the change, or in a
dedicated bump commit merged before the release is cut._

_TODO: add PR labels (`semver: patch`, `semver: minor`, `semver: major`) to help
identify which PRs require a version bump before cutting a release._

## Making a release

Fork releases are created by running the [`Unified build artifacts`](../.github/workflows/890m-build-artifacts.yml)
manual workflow after the version tag is present on the Fork `master`.

The workflow builds the supported Linux and Windows ROCm/Vulkan packages,
attaches the performance report, and publishes the GitHub Release for the tag.

The version tag is created from the merge commit on the Fork `master` before the
workflow is started.

## Building a release

By default, `LLAMA_BUILD_IS_DEV=ON` which appends a `-dev` suffix to `LLAMA_VERSION`,
marking the build as a nightly/development build. Distributors building from a
release tag must pass `-DLLAMA_BUILD_IS_DEV=OFF` to produce a clean version string
(e.g. `0.1.0` instead of `0.1.0-dev`).

## How releases reach users
Users access a published version through the following channels:

- **GitHub Releases** — download the packages and read the performance report.
- **Package managers**  — consume the git tag directly.
- **Build from source** — users clone the repo and check out the tag.

## Fork release procedure

The Fork release sequence is:

1. verify the release changes on a dedicated `release/unified-*` branch;
2. merge that branch into the Fork `master`;
3. create the version tag from the resulting `master` commit;
4. run `Unified build artifacts` for that tag with the performance report;
5. publish the release page from that report.

The release page contains the supported model settings, measurement conditions, performance results, comparison target, and applicable constraints.
