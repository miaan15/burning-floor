module;

export module dyn_array;

import common;

export template <typename T>
requires std::is_trivially_copyable_v<T>
struct DynArray {
    T *data = nullptr;
    size_t len = 0;
    size_t cap = 0;

    void destroy() {
        if (data) std::free(data);
        data = nullptr;
        len = cap = 0;
    }

    void reserve(size_t new_cap) {
        if (new_cap <= cap) return;

        T* new_data = (T *)std::malloc(new_cap * sizeof(T));

        if (data) {
            std::memcpy(new_data, data, len * sizeof(T));
            std::free(data);
        }

        data = new_data;
        cap = new_cap;
    }

    void grow(size_t min_cap) {
        size_t new_cap = cap < 4 ? 4 : (cap * 2);
        if (new_cap < min_cap) new_cap = min_cap;
        reserve(new_cap);
    }

    void append(const T& val) {
        if (len == cap) grow(len + 1);
        data[len++] = val;
    }

    void append_range(T *range_begin, size_t range_len) {
        if (len + range_len >= cap) grow(len + range_len);
        std::memcpy(data + len, range_begin, range_len * sizeof(T));
        len += range_len;
    }

    void pop() {
        if (len > 0) --len;
    }

    void set(size_t index, const T& val) {
        if (index < len) data[index] = val;
    }

    [[nodiscard]] T get(size_t index) const {
        if (index >= len) return T{};
        return data[index];
    }

    [[nodiscard]] T *get_ptr(size_t index) {
        if (index >= len) return nullptr;
        return &data[index];
    }
    [[nodiscard]] const T *get_ptr(size_t index) const {
        if (index >= len) return nullptr;
        return &data[index];
    }

    void clear() { len = 0; }

    using iterator = T *;
    using const_iterator = const T *;

    iterator begin()  { return data; }
    iterator end() { return data + len; }
    const_iterator begin() const { return data; }
    const_iterator end() const { return data + len; }
    const_iterator cbegin() const { return data; }
    const_iterator cend() const { return data + len; }
};
