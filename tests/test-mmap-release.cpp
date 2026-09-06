#include "../src/llama-mmap.h"
#include "ggml.h"

#include <cstdio>
#include <cstring>
#include <limits>
#include <memory>
#include <vector>

int main() {
    auto close_file = [](FILE * file) { std::fclose(file); };
    std::unique_ptr<FILE, decltype(close_file)> stream(std::tmpfile(), close_file);
    GGML_ASSERT(stream);
    std::vector<unsigned char> expected(1024 * 1024 + 13);
    for (size_t i = 0; i < expected.size(); ++i) {
        expected[i] = (unsigned char) (i * 37);
    }
    GGML_ASSERT(std::fwrite(expected.data(), 1, expected.size(), stream.get()) == expected.size());
    GGML_ASSERT(std::fflush(stream.get()) == 0);
    llama_file file(stream.get());
    llama_mmap mapping(&file, 0);
    GGML_ASSERT(std::memcmp(mapping.addr(), expected.data(), expected.size()) == 0);
    for (const auto & range : std::vector<std::pair<size_t, size_t>>{
            {1, 13}, {17, 128 * 1024}, {0, expected.size()},
            {expected.size() - 2, std::numeric_limits<size_t>::max()},
            {expected.size(), 4096}, {0, 0}}) {
        mapping.release_range(range.first, range.second);
        GGML_ASSERT(std::memcmp(mapping.addr(), expected.data(), expected.size()) == 0);
    }
    std::puts("mmap release preserves file contents and mapping access: OK");
}
