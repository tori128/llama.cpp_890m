#include "../src/llama-batch.h"
#include "../src/llama-io.h"
#include "../src/llama-kv-cache.h"
#include "../src/llama-model.h"

#include <cstring>
#include <memory>
#include <vector>

struct state_buffer : llama_io_write_i, llama_io_read_i {
    std::vector<uint8_t> data;
    size_t pos = 0;

    void write(const void * src, size_t size) override {
        const auto * bytes = static_cast<const uint8_t *>(src);
        data.insert(data.end(), bytes, bytes + size);
        pos += size;
    }

    void write_tensor(ggml_tensor * tensor, size_t offset, size_t size) override {
        const size_t begin = data.size();
        data.resize(begin + size);
        ggml_backend_tensor_get(tensor, data.data() + begin, offset, size);
        pos += size;
    }

    void read(void * dst, size_t size) override {
        GGML_ASSERT(pos + size <= data.size());
        std::memcpy(dst, data.data() + pos, size);
        pos += size;
    }

    void read_tensor(ggml_tensor * tensor, size_t offset, size_t size) override {
        GGML_ASSERT(pos + size <= data.size());
        ggml_backend_tensor_set(tensor, data.data() + pos, offset, size);
        pos += size;
    }

    size_t n_bytes() override { return pos; }
};

static size_t cache_bytes(const llama_kv_cache & cache) {
    size_t result = 0;
    for (const auto & item : cache.memory_breakdown()) {
        result += item.second;
    }
    return result;
}

static void test_cache(bool unified, bool v_trans) {
    std::unique_ptr<llama_model> model(llama_model_create(LLM_ARCH_LLAMA, llama_model_default_params()));
    auto & hp = model->hparams;
    hp.n_layer_all = 2;
    hp.n_embd = 256;
    hp.n_embd_head_k_full = 128;
    hp.n_embd_head_v_full = 256;
    hp.n_head_arr.fill(2);
    hp.n_head_kv_arr.fill(1);
    const auto type_v = v_trans ? GGML_TYPE_F32 : GGML_TYPE_Q8_0;
    auto make_cache = [&](bool key_only) {
        return std::make_unique<llama_kv_cache>(
            *model, hp, GGML_TYPE_Q8_0, type_v, v_trans, false, unified,
            16, 2, 1, 0, LLAMA_SWA_TYPE_NONE, nullptr, nullptr, nullptr, nullptr, "idx_", key_only);
    };
    auto keys = make_cache(true);
    auto full = make_cache(false);
    const size_t expected_v = 2 * ggml_row_size(type_v, 256) * 16 * keys->get_n_stream();
    GGML_ASSERT(cache_bytes(*full) - cache_bytes(*keys) == expected_v);

    llama_pos positions[] = { 0, 1, 2, 3 };
    llama_seq_id seq = 0;
    llama_seq_id * seq_ids[] = { &seq, &seq, &seq, &seq };
    int32_t n_seq_ids[] = { 1, 1, 1, 1 };
    llama_ubatch batch = {};
    batch.b_equal_seqs = 1;
    batch.n_tokens = batch.n_seq_tokens = 4;
    batch.n_seqs = batch.n_seqs_unq = batch.n_pos = 1;
    batch.pos = positions;
    batch.n_seq_id = n_seq_ids;
    batch.seq_id = seq_ids;
    batch.seq_id_unq = &seq;
    const auto slot = keys->find_slot(batch, true);
    GGML_ASSERT(!slot.empty());
    keys->apply_ubatch(slot, batch);
    GGML_ASSERT(keys->seq_pos_max(0) == 3);

    std::vector<std::vector<uint8_t>> expected;
    for (const auto il : keys->get_layer_ids()) {
        auto * tensor = keys->get_k_storage(il);
        expected.emplace_back(4 * tensor->nb[1]);
        for (size_t i = 0; i < expected.back().size(); ++i) {
            expected.back()[i] = (i + 17 * il) % 251;
        }
        ggml_backend_tensor_set(tensor, expected.back().data(), 0, expected.back().size());
    }
    state_buffer state;
    keys->state_write(state, 0);
    keys->clear(true);
    state.pos = 0;
    keys->state_read(state, 0);
    GGML_ASSERT(state.pos == state.data.size());
    GGML_ASSERT(keys->seq_pos_min(0) == 0 && keys->seq_pos_max(0) == 3);
    for (const auto il : keys->get_layer_ids()) {
        std::vector<uint8_t> actual(expected[il].size());
        ggml_backend_tensor_get(keys->get_k_storage(il), actual.data(), 0, actual.size());
        GGML_ASSERT(actual == expected[il]);
    }
    if (unified) {
        keys->seq_cp(0, 1, 0, -1);
        GGML_ASSERT(keys->seq_pos_max(1) == 3);
        keys->seq_keep(1);
        GGML_ASSERT(keys->seq_pos_max(0) == -1);
        GGML_ASSERT(keys->seq_rm(1, 2, -1));
        GGML_ASSERT(keys->seq_pos_max(1) == 1);
    }
}

int main() {
    ggml_backend_load_all();
    for (const bool unified : { false, true }) {
        for (const bool v_trans : { false, true }) {
            test_cache(unified, v_trans);
        }
    }
    return 0;
}
