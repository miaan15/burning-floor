module;

export module arena;

import common;

export struct Arena {
    u8 *buffer = nullptr;
    size_t offset = 0;
    size_t cap = 0;

    void init(size_t bytes) {
        buffer = (u8 *)std::malloc(bytes);
        offset = 0;
        cap = bytes;
    }

    void destroy() {
        if (buffer) std::free(buffer);
        buffer = nullptr;
        offset = cap = 0;
    }

    void reset() {
        offset = 0;
    }

    [[nodiscard]] void *alloc(size_t bytes, size_t align) {
        size_t aligned = (offset + align - 1) & ~(align - 1);
        if (aligned + bytes > cap) return nullptr;
        offset = aligned + bytes;
        return buffer + aligned;
    }

    [[nodiscard]] void *alloc_zero(size_t bytes, size_t align) {
        void *ptr = alloc(bytes, align);
        if (ptr) std::memset(ptr, 0, bytes);
        return ptr;
    }
};

[[nodiscard]] Arena Arena_make(size_t bytes) {
    return Arena{.buffer = (u8 *)std::malloc(bytes),
                 .offset = 0,
                 .cap = bytes};
}
