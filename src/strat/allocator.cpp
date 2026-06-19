module;

export module allocator;

import common;

export template <typename T>
concept AllocConcept =
    requires(T instance, size_t size, size_t align, void* ptr) {
        { instance.alloc(size, align) } -> std::same_as<void*>;
        { instance.free(ptr) } -> std::same_as<void>;
    };

export struct CAlloc {
    [[nodiscard]] void *alloc(size_t size,
                              size_t align = alignof(std::max_align_t)) {
        return std::malloc(size);
    }

    [[nodiscard]] void *alloc_zero(size_t size,
                                   size_t align = alignof(std::max_align_t)) {
        void *ptr = std::malloc(size);
        std::memset(ptr, 0, size);
        return ptr;
    }

    void free(void *ptr) {
        std::free(ptr);
    }
};
export CAlloc c_allocator;

export struct Arena {
    u8 *buffer = nullptr;
    size_t offset = 0;
    size_t cap = 0;

    void init(size_t size) {
        buffer = (u8 *)std::malloc(size);
        offset = 0;
        cap = size;
    }

    void destroy() {
        if (buffer) std::free(buffer);
        buffer = nullptr;
        offset = cap = 0;
    }

    void reset() {
        offset = 0;
    }

    [[nodiscard]] void *alloc(size_t size,
                              size_t align = alignof(std::max_align_t)) {
        size_t aligned = (offset + align - 1) & ~(align - 1);
        if (aligned + size > cap) return nullptr;
        offset = aligned + size;
        return buffer + aligned;
    }

    [[nodiscard]] void *alloc_zero(size_t size,
                                   size_t align = alignof(std::max_align_t)) {
        void *ptr = alloc(size, align);
        if (ptr) std::memset(ptr, 0, size);
        return ptr;
    }

    void free(void *ptr) {}
};
[[nodiscard]] Arena Arena_make(size_t size) {
    return Arena{.buffer = (u8 *)std::malloc(size),
                 .offset = 0,
                 .cap = size};
}
