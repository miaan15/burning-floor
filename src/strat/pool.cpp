module;

export module pool;

import common;

// TODO: improve data layout: pack all three data, free_ids, deleted in to a single data buffer since this is static
export template <typename T>
requires std::is_trivially_copyable_v<T>
struct Pool {
    T *data = nullptr;
    size_t *free_ids = nullptr;
    bool *deleted = nullptr;
    size_t next_data_index = 0;
    size_t free_id_len = 0;
    size_t count = 0;
    size_t cap = 0;

    void init(size_t cap) {
        data = (T *)std::malloc(cap * sizeof(T));
        free_ids = (size_t *)std::malloc(cap * sizeof(size_t));
        deleted = (bool *)std::malloc(cap * sizeof(bool));
        next_data_index = free_id_len = count = 0;
        this->cap = cap;
    }

    void destroy() {
        if (data) {
            std::free(data);
            std::free(free_ids);
            std::free(deleted);
        }
        next_data_index = free_id_len = count = cap = 0;
    }

    void reserve(size_t new_cap) {
        if (new_cap <= cap) return;

        T *new_data = (T *)std::malloc(new_cap * sizeof(T));
        size_t *new_free_ids = (size_t *)std::malloc(new_cap * sizeof(size_t));
        bool *new_deleted = (bool *)std::malloc(new_cap * sizeof(bool));

        if (data) {
            std::memcpy(new_data, data, next_data_index * sizeof(T));
            std::free(data);
        }
        if (deleted) {
            std::memcpy(new_deleted, deleted, next_data_index * sizeof(bool));
            std::free(deleted);
        }
        if (free_ids) {
            std::memcpy(new_free_ids, free_ids, free_id_len * sizeof(size_t));
            std::free(free_ids);
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
            deleted[next_data_index] = false;
            ++count;

            return next_data_index++;
        }

        size_t index = free_ids[--free_id_len];
        data[index] = val;
        deleted[index] = false;
        ++count;

        return index;
    }

    void remove(size_t index) {
        if (index >= next_data_index || deleted[index]) return;

        free_ids[free_id_len++] = index;

        deleted[index] = true;
        --count;
    }

    [[nodiscard]] T get(size_t index) const {
        if (index >= next_data_index || deleted[index]) return T{};
        return data[index];
    }

    [[nodiscard]] T *get_ptr(size_t index) {
        if (index >= next_data_index || deleted[index]) return nullptr;
        return &data[index];
    }
    [[nodiscard]] const T *get_ptr(size_t index) const {
        if (index >= next_data_index || deleted[index]) return nullptr;
        return &data[index];
    }

    void clear() {
        next_data_index = 0;
        free_id_len = 0;
        count = 0;
    }
};

template <typename T>
requires std::is_trivially_copyable_v<T>
[[nodiscard]] Pool<T> Pool_make(size_t cap) {
    return Pool<T>{.data = (T *)std::malloc(cap * sizeof(T)),
                   .free_ids = (size_t *)std::malloc(cap * sizeof(size_t)),
                   .deleted = (bool *)std::malloc(cap * sizeof(bool)),
                   .next_data_index = 0,
                   .free_id_len = 0,
                   .count = 0,
                   .cap = cap};
}
