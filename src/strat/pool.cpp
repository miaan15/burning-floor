module;

export module pool;

import common;

export template <typename T>
requires std::is_trivially_copyable_v<T>
struct Pool {
    // Packed layout: data -> free_ids -> deleted
    union Slot {
        T val;
        size_t next;
    } *data = nullptr;
    void *deleted = nullptr;
    size_t next_index = 0;
    size_t bound_index = 0;
    size_t count = 0;
    size_t cap = 0;

    void init(size_t cap) {
        size_t total_size = cap * sizeof(Slot);
        size_t deleted_offset = total_size;
        total_size += (cap + 7) / 8;

        data = (Slot *)std::malloc(total_size);
        deleted = (void *)((u8 *)data + deleted_offset);
        next_index = bound_index = count = 0;
        this->cap = cap;
    }

    void destroy() {
        if (data) std::free(data);
        data = nullptr;
        deleted = nullptr;
        next_index = bound_index = count = cap = 0;
    }

    void reserve(size_t new_cap) {
        if (new_cap <= cap) return;

        size_t total_size = new_cap * sizeof(T);
        size_t deleted_offset = total_size;
        total_size += (new_cap + 7) / 8;

        Slot *new_data = (Slot *)std::malloc(total_size);
        void *new_deleted = (void *)((u8 *)new_data + deleted_offset);

        if (data) {
            std::memcpy(new_data, data, bound_index * sizeof(T));
            std::memcpy(new_deleted, deleted,
                        ((bound_index + 7) / 8) * sizeof(bool));

            std::free(data);
        }

        data = new_data;
        deleted = new_deleted;
        cap = new_cap;
    }

    size_t add(const T& val) {
        size_t index = next_index;

        if (next_index == bound_index) {
            ++next_index;
            ++bound_index;
        } else {
            next_index = data[next_index].next;
        }

        data[index].val = val; 

        _set_deleted(index, false);
        ++count;

        return index;
    }

    void remove(size_t index) {
        if (index >= bound_index || _is_deleted(index)) return;

        data[index].next = next_index;

        _set_deleted(index, true);
        --count;
    }

    [[nodiscard]] T get(size_t index) const {
        if (index >= bound_index || _is_deleted(index)) return T{};
        return data[index];
    }

    [[nodiscard]] T *get_ptr(size_t index) {
        if (index >= bound_index || _is_deleted(index)) return nullptr;
        return &data[index];
    }
    [[nodiscard]] const T *get_ptr(size_t index) const {
        if (index >= bound_index || _is_deleted(index)) return nullptr;
        return &data[index];
    }

    void clear() {
        bound_index = 0;
        next_index = 0;
        count = 0;
    }

    inline void _set_deleted(size_t index, bool v) {
        u8 *bitset = (u8 *)(deleted);
        size_t byte_idx = index / 8;
        size_t bit_idx = index % 8;

        bitset[byte_idx] =
            (bitset[byte_idx] & ~(1 << bit_idx)) | (v << bit_idx);
    }
    inline bool _is_deleted(size_t index) const {
        const u8 *bitset = (const u8 *)(deleted);
        return (bitset[index / 8] >> (index % 8)) & 1;
    }
};

template <typename T>
requires std::is_trivially_copyable_v<T>
[[nodiscard]] Pool<T> Pool_make(size_t cap) {
    size_t total_size = cap * sizeof(T);
    size_t deleted_offset = total_size;
    total_size += (cap + 7) / 8;

    T* data = (T *)std::malloc(total_size);

    return Pool<T>{.data = data,
                   .deleted = (void *)((u8 *)data + deleted_offset),
                   .next_index = 0,
                   .bound_index = 0,
                   .count = 0,
                   .cap = cap};
}
