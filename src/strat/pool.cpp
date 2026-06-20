module;

export module pool;

import common;
export import allocator;

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
        std::memset(deleted, 0, (cap + 7) / 8);
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

        size_t total_size = new_cap * sizeof(Slot);
        size_t deleted_offset = total_size;
        total_size += (new_cap + 7) / 8;

        Slot *new_data = (Slot *)std::malloc(total_size);
        void *new_deleted = (void *)((u8 *)new_data + deleted_offset);
        std::memset(new_deleted, 0, (new_cap + 7) / 8);

        if (data) {
            std::memcpy(new_data, data, bound_index * sizeof(Slot));
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
        next_index = index;

        _set_deleted(index, true);
        --count;
    }

    [[nodiscard]] T get(size_t index) const {
        if (index >= bound_index || _is_deleted(index)) return T{};
        return data[index].val;
    }

    [[nodiscard]] T *get_ptr(size_t index) {
        if (index >= bound_index || _is_deleted(index)) return nullptr;
        return &data[index].val;
    }
    [[nodiscard]] const T *get_ptr(size_t index) const {
        if (index >= bound_index || _is_deleted(index)) return nullptr;
        return &data[index].val;
    }

    void clear() {
        bound_index = 0;
        next_index = 0;
        count = 0;
    }

    [[nodiscard]] size_t mem_size() {
        size_t total_size = cap * sizeof(Slot);
        size_t deleted_offset = total_size;
        total_size += (cap + 7) / 8;
        return total_size;
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

    template <bool Const>
    struct _BaseIterator {
        using PoolPtr = std::conditional_t<Const, const Pool*, Pool*>;
        using ValueRef = std::conditional_t<Const, const T&, T&>;
        using ValuePtr = std::conditional_t<Const, const T*, T*>;

        PoolPtr pool;
        size_t index;

        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = ValuePtr;
        using reference = ValueRef;

        _BaseIterator(PoolPtr p, size_t idx) : pool(p), index(idx) {
            skip_deleted();
        }

        void skip_deleted() {
            while (index < pool->bound_index && pool->_is_deleted(index)) {
                ++index;
            }
        }

        _BaseIterator& operator++() {
            ++index;
            skip_deleted();
            return *this;
        }
        _BaseIterator operator++(int) {
            _BaseIterator tmp = *this;
            ++(*this);
            return tmp;
        }

        ValueRef operator*() const { return pool->data[index].val; }
        ValuePtr operator->() const { return &pool->data[index].val; }

        bool operator==(const _BaseIterator& other) const {
            return index == other.index;
        }
        bool operator!=(const _BaseIterator& other) const {
            return index != other.index;
        }
    };
    using iterator = _BaseIterator<false>;
    using const_iterator = _BaseIterator<true>;

    iterator begin() { return iterator(this, 0); }
    iterator end() { return iterator(this, bound_index); }
    const_iterator begin() const { return const_iterator(this, 0); }
    const_iterator end() const { return const_iterator(this, bound_index); }
};

template <typename T>
requires std::is_trivially_copyable_v<T>
[[nodiscard]] Pool<T> Pool_make(size_t cap) {
    using slot_t = typename Pool<T>::Slot;
    size_t total_size = cap * sizeof(slot_t);
    size_t deleted_offset = total_size;
    total_size += (cap + 7) / 8;

    slot_t *data = (slot_t *)std::malloc(total_size);
    std::memset((u8 *)data + deleted_offset, 0, (cap + 7) / 8);

    return Pool<T>{.data = data,
                   .deleted = (void *)((u8 *)data + deleted_offset),
                   .next_index = 0,
                   .bound_index = 0,
                   .count = 0,
                   .cap = cap};
}

// WITH ALLOC
// =============================================================================
export template <typename T, AllocConcept Alloc>
requires std::is_trivially_copyable_v<T>
struct PoolWAlloc {
    Alloc *alloc = nullptr;

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

    void init(size_t cap, Alloc *alloc) {
        size_t total_size = cap * sizeof(Slot);
        size_t deleted_offset = total_size;
        total_size += (cap + 7) / 8;

        this->alloc = alloc;
        data = (Slot *)alloc->alloc(total_size);
        deleted = (void *)((u8 *)data + deleted_offset);
        std::memset(deleted, 0, (cap + 7) / 8);
        next_index = bound_index = count = 0;
        this->cap = cap;
    }

    void destroy() {
        if (alloc && data) alloc->free(data);
        data = nullptr;
        deleted = nullptr;
        next_index = bound_index = count = cap = 0;
    }

    void reserve(size_t new_cap) {
        if (!alloc) return;
        if (new_cap <= cap) return;

        size_t total_size = new_cap * sizeof(Slot);
        size_t deleted_offset = total_size;
        total_size += (new_cap + 7) / 8;

        Slot *new_data = (Slot *)alloc->alloc(total_size);
        void *new_deleted = (void *)((u8 *)new_data + deleted_offset);
        std::memset(new_deleted, 0, (new_cap + 7) / 8);

        if (data) {
            std::memcpy(new_data, data, bound_index * sizeof(Slot));
            std::memcpy(new_deleted, deleted,
                        ((bound_index + 7) / 8) * sizeof(bool));

            alloc->free(data);
        }

        data = new_data;
        deleted = new_deleted;
        cap = new_cap;
    }

    size_t add(const T& val) {
        if (next_index >= cap) return 0;

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
        next_index = index;

        _set_deleted(index, true);
        --count;
    }

    [[nodiscard]] T get(size_t index) const {
        if (index >= bound_index || _is_deleted(index)) return T{};
        return data[index].val;
    }

    [[nodiscard]] T *get_ptr(size_t index) {
        if (index >= bound_index || _is_deleted(index)) return nullptr;
        return &data[index].val;
    }
    [[nodiscard]] const T *get_ptr(size_t index) const {
        if (index >= bound_index || _is_deleted(index)) return nullptr;
        return &data[index].val;
    }

    [[nodiscard]] bool check(size_t index) const {
        return index < bound_index && !_is_deleted(index);
    }

    void clear() {
        bound_index = 0;
        next_index = 0;
        count = 0;
    }

    [[nodiscard]] size_t mem_size() {
        size_t total_size = cap * sizeof(Slot);
        size_t deleted_offset = total_size;
        total_size += (cap + 7) / 8;
        return total_size;
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

    template <bool Const>
    struct _BaseIterator {
        using PoolPtr = std::conditional_t<Const, const PoolWAlloc*, PoolWAlloc*>;
        using ValueRef = std::conditional_t<Const, const T&, T&>;
        using ValuePtr = std::conditional_t<Const, const T*, T*>;

        PoolPtr pool;
        size_t index;

        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = ValuePtr;
        using reference = ValueRef;

        _BaseIterator(PoolPtr p, size_t idx) : pool(p), index(idx) {
            skip_deleted();
        }

        void skip_deleted() {
            while (index < pool->bound_index && pool->_is_deleted(index)) {
                ++index;
            }
        }

        _BaseIterator& operator++() {
            ++index;
            skip_deleted();
            return *this;
        }
        _BaseIterator operator++(int) {
            _BaseIterator tmp = *this;
            ++(*this);
            return tmp;
        }

        ValueRef operator*() const { return pool->data[index].val; }
        ValuePtr operator->() const { return &pool->data[index].val; }

        bool operator==(const _BaseIterator& other) const {
            return index == other.index;
        }
        bool operator!=(const _BaseIterator& other) const {
            return index != other.index;
        }
    };
    using iterator = _BaseIterator<false>;
    using const_iterator = _BaseIterator<true>;

    iterator begin() { return iterator(this, 0); }
    iterator end() { return iterator(this, bound_index); }
    const_iterator begin() const { return const_iterator(this, 0); }
    const_iterator end() const { return const_iterator(this, bound_index); }
};

template <typename T, AllocConcept Alloc>
requires std::is_trivially_copyable_v<T>
[[nodiscard]] PoolWAlloc<T, Alloc> PoolWAlloc_make(size_t cap, Alloc *alloc) {
    using slot_t = typename PoolWAlloc<T, Alloc>::Slot;
    size_t total_size = cap * sizeof(slot_t);
    size_t deleted_offset = total_size;
    total_size += (cap + 7) / 8;

    slot_t *data = (slot_t *)alloc->alloc(total_size);
    std::memset((u8 *)data + deleted_offset, 0, (cap + 7) / 8);

    return PoolWAlloc<T, Alloc>{
        .alloc = alloc,
        .data = data,
        .deleted = (void *)((u8 *)data + deleted_offset),
        .next_index = 0,
        .bound_index = 0,
        .count = 0,
        .cap = cap};
}
