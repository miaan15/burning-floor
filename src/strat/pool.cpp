module;

export module pool;

import common;

export template <typename T>
requires std::is_trivially_copyable_v<T>
struct Pool {
    // Packed layout: data -> free_ids -> deleted
    T *data = nullptr;
    size_t *free_ids = nullptr;
    void *deleted = nullptr;
    size_t next_data_index = 0;
    size_t free_id_len = 0;
    size_t count = 0;
    size_t cap = 0;

    void init(size_t cap) {
        size_t total_size = cap * sizeof(T);
        total_size = (total_size + alignof(size_t) - 1) & ~(alignof(size_t) - 1);
        size_t free_ids_offset = total_size;
        total_size += cap * sizeof(size_t);
        size_t deleted_offset = total_size;
        total_size += (cap + 7) / 8;

        data = (T *)std::malloc(total_size);
        free_ids = (size_t *)((u8 *)data + free_ids_offset);
        deleted = (void *)((u8 *)data + deleted_offset);
        next_data_index = free_id_len = count = 0;
        this->cap = cap;
    }

    void destroy() {
        if (data) std::free(data);
        data = nullptr;
        free_ids = nullptr;
        deleted = nullptr;
        next_data_index = free_id_len = count = cap = 0;
    }

    void reserve(size_t new_cap) {
        if (new_cap <= cap) return;

        size_t total_size = new_cap * sizeof(T);
        total_size = (total_size + alignof(size_t) - 1) & ~(alignof(size_t) - 1);
        size_t free_ids_offset = total_size;
        total_size += new_cap * sizeof(size_t);
        size_t deleted_offset = total_size;
        total_size += (new_cap + 7) / 8;

        T *new_data = (T *)std::malloc(total_size);
        size_t *new_free_ids = (size_t *)((u8 *)new_data + free_ids_offset);
        void *new_deleted = (void *)((u8 *)new_data + deleted_offset);

        if (data) {
            std::memcpy(new_data, data, next_data_index * sizeof(T));
            std::memcpy(new_free_ids, free_ids, free_id_len * sizeof(size_t));
            std::memcpy(new_deleted, deleted,
                        ((next_data_index + 7) / 8) * sizeof(bool));

            std::free(data);
        }

        data = new_data;
        free_ids = new_free_ids;
        deleted = new_deleted;
        cap = new_cap;
    }

    size_t add(const T& val) {
        if (free_id_len == 0) {
            if (next_data_index >= cap) return (size_t)-1;

            data[next_data_index] = val;
            set_deleted(next_data_index, false);
            ++count;

            return next_data_index++;
        }

        size_t index = free_ids[--free_id_len];
        data[index] = val;
        set_deleted(index, false);
        ++count;

        return index;
    }

    void remove(size_t index) {
        if (index >= next_data_index || is_deleted(index)) return;

        free_ids[free_id_len++] = index;

        set_deleted(index, true);
        --count;
    }

    [[nodiscard]] T get(size_t index) const {
        if (index >= next_data_index || is_deleted(index)) return T{};
        return data[index];
    }

    [[nodiscard]] T *get_ptr(size_t index) {
        if (index >= next_data_index || is_deleted(index)) return nullptr;
        return &data[index];
    }
    [[nodiscard]] const T *get_ptr(size_t index) const {
        if (index >= next_data_index || is_deleted(index)) return nullptr;
        return &data[index];
    }

    void clear() {
        next_data_index = 0;
        free_id_len = 0;
        count = 0;
    }

private:
    inline void set_deleted(size_t index, bool v) {
        u8 *bitset = (u8 *)(deleted);
        size_t byte_idx = index / 8;
        size_t bit_idx = index % 8;

        bitset[byte_idx] =
            (bitset[byte_idx] & ~(1 << bit_idx)) | (v << bit_idx);
    }
    inline bool is_deleted(size_t index) const {
        const u8 *bitset = (const u8 *)(deleted);
        return (bitset[index / 8] >> (index % 8)) & 1;
    }
};

template <typename T>
requires std::is_trivially_copyable_v<T>
[[nodiscard]] Pool<T> Pool_make(size_t cap) {
    size_t total_size = cap * sizeof(T);
    total_size = (total_size + alignof(size_t) - 1) & ~(alignof(size_t) - 1);
    size_t free_ids_offset = total_size;
    total_size += cap * sizeof(size_t);
    size_t deleted_offset = total_size;
    total_size += (cap + 7) / 8;

    T* data = (T *)std::malloc(total_size);

    return Pool<T>{.data = data,
                   .free_ids = (size_t *)((u8 *)data + free_ids_offset),
                   .deleted = (void *)((u8 *)data + deleted_offset),
                   .next_data_index = 0,
                   .free_id_len = 0,
                   .count = 0,
                   .cap = cap};
}
