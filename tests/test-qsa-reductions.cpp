#include "ggml.h"
#include "ggml-backend.h"
#include "ggml-cpp.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <vector>

int main(int argc, char ** argv) {
    ggml_backend_load_all();
    ggml_backend_ptr backend(ggml_backend_init_by_name(argc > 1 ? argv[1] : "CPU", nullptr));
    GGML_ASSERT(backend);

    for (bool head_sum : {false, true}) {
        for (int64_t blocks : {1, 8192, 25024}) {
            for (int64_t tokens : {1, 5, 512}) {
                if (!head_sum && tokens != 1) {
                    continue;
                }
                ggml_init_params params = {ggml_tensor_overhead() * 64 + ggml_graph_overhead(), nullptr, true};
                ggml_context_ptr ctx(ggml_init(params));
                GGML_ASSERT(ctx);
                auto * input = ggml_new_tensor_3d(ctx.get(), GGML_TYPE_F32,
                        head_sum ? blocks : 128, 4, head_sum ? tokens : blocks);
                auto * values = head_sum ? ggml_relu(ctx.get(), input) : input;
                ggml_tensor * before = nullptr;
                if (head_sum) {
                    before = ggml_sum_rows(ctx.get(), ggml_cont(ctx.get(), ggml_permute(ctx.get(), values, 1, 0, 2, 3)));
                } else {
                    for (int64_t i = 0; i < 4; ++i) {
                        auto * slice = ggml_cont(ctx.get(), ggml_view_2d(ctx.get(), values,
                                128, blocks, values->nb[2], i * values->nb[1]));
                        before = before ? ggml_add(ctx.get(), before, slice) : slice;
                    }
                    before = ggml_scale(ctx.get(), before, 0.25f);
                }
                auto * after = ggml_pool_2d(ctx.get(), values, GGML_OP_POOL_AVG, 1, 4, 1, 4, 0, 0);
                if (head_sum) {
                    after = ggml_scale(ctx.get(), after, 4.0f);
                }
                auto * graph = ggml_new_graph(ctx.get());
                ggml_build_forward_expand(graph, before);
                ggml_build_forward_expand(graph, after);
                ggml_backend_buffer_ptr buffer(ggml_backend_alloc_ctx_tensors(ctx.get(), backend.get()));
                GGML_ASSERT(buffer);
                std::vector<float> data(ggml_nelements(input));
                for (size_t i = 0; i < data.size(); ++i) {
                    data[i] = ((int) (i % 257) - 128) / 127.0f;
                }
                ggml_backend_tensor_set(input, data.data(), 0, ggml_nbytes(input));
                GGML_ASSERT(ggml_backend_graph_compute(backend.get(), graph) == GGML_STATUS_SUCCESS);
                GGML_ASSERT(ggml_nelements(before) == ggml_nelements(after));
                std::vector<float> old_values(ggml_nelements(before)), new_values(ggml_nelements(after));
                ggml_backend_tensor_get(before, old_values.data(), 0, ggml_nbytes(before));
                ggml_backend_tensor_get(after, new_values.data(), 0, ggml_nbytes(after));
                float max_error = 0.0f;
                for (size_t i = 0; i < old_values.size(); ++i) {
                    GGML_ASSERT(std::isfinite(old_values[i]) && std::isfinite(new_values[i]));
                    max_error = std::max(max_error, std::fabs(old_values[i] - new_values[i]));
                }
                GGML_ASSERT(max_error <= 2e-6f);
                std::printf("%s blocks=%lld tokens=%lld max_abs=%.9g\n", head_sum ? "head_sum" : "block_mean",
                        (long long) blocks, (long long) tokens, max_error);
            }
        }
    }
}
