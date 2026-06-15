module;

export module hash_map;

import common;

export template <typename K, typename V,
                 typename Hash = std::hash<K>,
                 typename KeyEqual = std::equal_to<K>>
requires std::is_trivially_copyable_v<K> && std::is_trivially_copyable_v<V>
struct HashMap {
    struct Slot { K key; V value; };
    enum SlotState : char { EMPTY = 0, TAKEN, DELETED };

    Slot *slots = nullptr;
    SlotState *states = nullptr;
    size_t len = 0;
    size_t cap = 0;
    size_t deleted_count = 0;

    void destroy() {
        if (slots) {
            for (size_t i = 0; i < cap; ++i) {
                if (states[i] == TAKEN) slots[i].~Slot();
            }
            std::free(slots);
            std::free(states);
        }
        slots = nullptr;
        states = nullptr;
        len = cap = deleted_count = 0;
    }

    void rehash(size_t new_cap) {
        Slot *new_slots = (Slot *)std::malloc(new_cap * sizeof(Slot));
        SlotState *new_states =
            (SlotState *)std::malloc(new_cap * sizeof(SlotState));
        std::memset(new_states, 0, new_cap * sizeof(SlotState));

        if (slots) {
            for (size_t i = 0; i < cap; ++i) {
                if (states[i] != TAKEN) continue;

                size_t hash = Hash{}(slots[i].key);
                size_t new_index = hash & (new_cap - 1);

                while (new_states[new_index] != EMPTY)
                    new_index = (new_index + 1) & (new_cap - 1);

                std::memcpy(&new_slots[new_index], &slots[i], sizeof(Slot));
                new_states[new_index] = TAKEN;
            }
            std::free(slots);
            std::free(states);
        }

        slots = new_slots;
        states = new_states;
        cap = new_cap;
        deleted_count = 0;
    }

    bool add(const K &key, const V &val) {
        if ((len + deleted_count) * 10 >= cap * 7) // 70%
            rehash(cap < 8 ? 8 : cap * 2);

        constexpr auto FALLBACK = (size_t)(-1);
        size_t hash = Hash{}(key);
        size_t index = hash & (cap - 1);
        size_t first_deleted = FALLBACK;
        while (states[index] != EMPTY) {
            if (states[index] == TAKEN && KeyEqual{}(slots[index].key, key)) {
                return false;
            }
            if (states[index] == DELETED && first_deleted == FALLBACK) {
                first_deleted = index;
            }
            index = (index + 1) & (cap - 1);
        }

        size_t target =
            first_deleted != FALLBACK ? first_deleted : index;
        slots[target] = Slot{key, val};

        ++len;
        if (target == first_deleted) --deleted_count;

        return true;
    }

    bool set(const K &key, const V &val) {
        if ((len + deleted_count) * 10 >= cap * 7)
            rehash(cap < 8 ? 8 : cap * 2);

        size_t hash = Hash{}(key);
        size_t index = hash & (cap - 1);
        while (states[index] != EMPTY) {
            if (states[index] == TAKEN && KeyEqual{}(slots[index].key, key)) {
                slots[index].val = val;
                return true;
            }
        }
        return false;
    }

    void add_or_set(const K &key, const V &val) {
        if ((len + deleted_count) * 10 >= cap * 7)
            rehash(cap < 8 ? 8 : cap * 2);

        constexpr auto FALLBACK = (size_t)(-1);
        size_t hash = Hash{}(key);
        size_t index = hash & (cap - 1);
        size_t first_deleted = FALLBACK;
        while (states[index] != EMPTY) {
            if (states[index] == TAKEN && KeyEqual{}(slots[index].key, key)) {
                slots[index].val = val;
                return;
            }
            if (states[index] == DELETED && first_deleted == FALLBACK) {
                first_deleted = index;
            }
            index = (index + 1) & (cap - 1);
        }

        size_t target =
            first_deleted != FALLBACK ? first_deleted : index;
        slots[target] = Slot{key, val};

        ++len;
        if (target == first_deleted) --deleted_count;
    }

    bool remove(const K &key) {
        if (cap < 8) return false;

        size_t hash = Hash{}(key);
        size_t index = hash & (cap - 1);
        while (states[index] != EMPTY) {
            if (states[index] == TAKEN && KeyEqual{}(slots[index].key, key)) {
                states[index] = DELETED;
                --len;
                ++deleted_count;

                return true;
            }
            index = (index + 1) & (cap - 1);
        }
        return false;
    }

    [[nodiscard]] V get(const K &key) const {
        if (cap < 8) return V{};

        size_t hash = Hash{}(key);
        size_t index = hash & (cap - 1);
        while (states[index] != EMPTY) {
            if (states[index] == TAKEN && KeyEqual{}(slots[index].key, key)) {
                return slots[index].val;
            }
            index = (index + 1) & (cap - 1);
        }
        return V{};
    }

    [[nodiscard]] V *get_ptr(const K &key) {
        if (cap < 8) return nullptr;

        size_t hash = Hash{}(key);
        size_t index = hash & (cap - 1);
        while (states[index] != EMPTY) {
            if (states[index] == TAKEN && KeyEqual{}(slots[index].key, key)) {
                return &slots[index].val;
            }
            index = (index + 1) & (cap - 1);
        }
        return nullptr;
    }
    [[nodiscard]] const V *get_ptr(const K &key) const {
        if (cap < 8) return nullptr;

        size_t hash = Hash{}(key);
        size_t index = hash & (cap - 1);
        while (states[index] != EMPTY) {
            if (states[index] == TAKEN && KeyEqual{}(slots[index].key, key)) {
                return &slots[index].val;
            }
            index = (index + 1) & (cap - 1);
        }
        return nullptr;
    }

    [[nodiscard]] V *get_or_add(const K &key) {
        if ((len + deleted_count) * 10 >= cap * 7)
            rehash(cap < 8 ? 8 : cap * 2);

        constexpr auto FALLBACK = (size_t)(-1);
        size_t hash = Hash{}(key);
        size_t index = hash & (cap - 1);
        size_t first_deleted = FALLBACK;
        while (states[index] != EMPTY) {
            if (states[index] == TAKEN && KeyEqual{}(slots[index].key, key)) {
                return &slots[index].val;
            }
            if (states[index] == DELETED && first_deleted == FALLBACK) {
                first_deleted = index;
            }
            index = (index + 1) & (cap - 1);
        }

        size_t target =
            first_deleted != FALLBACK ? first_deleted : index;
        slots[target] = Slot{key, V{}};

        ++len;
        if (target == first_deleted) --deleted_count;

        return &slots[target].val;
    }

    [[nodiscard]] bool has(const K &key) const {
        if (cap < 8) return false;

        size_t hash = Hash{}(key);
        size_t index = hash & (cap - 1);
        while (states[index] != EMPTY) {
            if (states[index] == TAKEN && KeyEqual{}(slots[index].key, key)) {
                return true;
            }
            index = (index + 1) & (cap - 1);
        }
        return false;
    }

    void clear() {
        if (!slots) return;
        std::memset(states, 0, cap * sizeof(SlotState));
        len = deleted_count = 0;
    }

    template <bool IsConst> struct IteratorImpl {
        using map_type = std::conditional_t<IsConst, const HashMap, HashMap>;
        using map_ptr = map_type *;
        using slot_type = std::conditional_t<IsConst, const Slot, Slot>;

        using iterator_category = std::forward_iterator_tag;
        using value_type = Slot;
        using difference_type = std::ptrdiff_t;
        using pointer = slot_type *;
        using reference = slot_type &;

        map_ptr map;
        size_t index;

        IteratorImpl(map_ptr m, size_t i) : map(m), index(i) {
            advance_to_valid();
        }

        void advance_to_valid() {
            while (index < map->cap && map->states[index] != TAKEN) ++index;
        }

        [[nodiscard]] reference operator*() const {
            return map->slots[index];
        }
        [[nodiscard]] pointer operator->() const {
            return &map->slots[index];
        }

        IteratorImpl & operator++() {
            ++index;
            advance_to_valid();
            return *this;
        }
        IteratorImpl operator++(int) {
            IteratorImpl tmp = *this;
            ++(*this);
            return tmp;
        }

        friend bool operator==(const IteratorImpl &a, const IteratorImpl &b) {
            return a.index == b.index;
        }

        friend bool operator!=(const IteratorImpl &a, const IteratorImpl &b) {
            return a.index != b.index;
        }
    };

    using iterator = IteratorImpl<false>;
    using const_iterator = IteratorImpl<true>;

    [[nodiscard]] iterator begin() { return iterator(this, 0); }
    [[nodiscard]] iterator end() { return iterator(this, cap); }
    [[nodiscard]] const_iterator begin() const { return const_iterator(this, 0); }
    [[nodiscard]] const_iterator end() const { return const_iterator(this, cap); }
    [[nodiscard]] const_iterator cbegin() const { return begin(); }
    [[nodiscard]] const_iterator cend() const { return end(); }
};
