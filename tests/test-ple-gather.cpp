#include "../src/models/models.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>

int main() {
    constexpr int64_t width = 160;
    constexpr int64_t count = 64;
    std::vector<float> values(width * count);
    for (size_t i = 0; i < values.size(); ++i) {
        values[i] = std::sin((float) i * 0.17f);
    }
    for (const auto type : {GGML_TYPE_F32, GGML_TYPE_Q5_1}) {
        ggml_init_params params = {1024 * 1024, nullptr, false};
        ggml_context * ctx = ggml_init(params);
        GGML_ASSERT(ctx);
        {
            llama_model_qwen4exp model(llama_model_default_params());
            model.per_layer_tok_embd = ggml_new_tensor_2d(ctx, type, width, count);
            if (type == GGML_TYPE_F32) {
                std::memcpy(model.per_layer_tok_embd->data, values.data(), values.size() * sizeof(float));
            } else {
                const size_t size = ggml_quantize_chunk(type, values.data(), model.per_layer_tok_embd->data, 0, count, width, nullptr);
                GGML_ASSERT(size == ggml_nbytes(model.per_layer_tok_embd));
            }
            for (size_t n : {size_t(0), size_t(1), size_t(31), size_t(512)}) {
                std::vector<int32_t> rows(n);
                for (size_t i = 0; i < n; ++i) {
                    rows[i] = (int32_t) ((i * 17) % count);
                }
                std::vector<float> serial(n * width), parallel(n * width);
                model.gather_ple_rows(rows.data(), n, serial.data(), 1);
                for (int threads : {2, 10, 32}) {
                    model.gather_ple_rows(rows.data(), n, parallel.data(), threads);
                    GGML_ASSERT(serial.empty() || std::memcmp(serial.data(), parallel.data(), serial.size() * sizeof(float)) == 0);
                }
            }
        }
        ggml_free(ctx);
    }
    std::puts("parallel PLE gather matches serial F32/Q5_1 output: OK");
}
